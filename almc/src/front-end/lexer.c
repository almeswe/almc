#include "lexer.h"

// beginning of stream
#define bos(lexer) \
	((lexer->stream - lexer->stream_origin) <= 0)

// end of stream
#define eos(lexer) \
	((uint32_t)(lexer->stream - lexer->stream_origin) >= lexer->stream_size)

#define curr_char_fits(lexer) \
	((uint32_t)((lexer->stream - lexer->stream_origin) >= 0) && (!eos(lexer)))

#define matchc(lexer, c) (get_curr_char(lexer) == c)
#define matchc_in(lexer, c1, c2) ((get_curr_char(lexer)) >= (c1) && (get_curr_char(lexer)) <= (c2))

#define STR_BUILDER_BUFFER_SIZE 512

#define str_builder_create_buffer(buffer) \
	char* buffer = newc_s(char, buffer, STR_BUILDER_BUFFER_SIZE)

#define str_builder_reduce_buffer(temp, buffer, size) \
	char* temp = buffer;							  \
	if (!(buffer = rnew(char, size + 1, buffer)))     \
		 buffer = temp;								  \
	buffer[size] = '\0';

#define str_builder_err										\
	report_error(frmt("Number size should be less than %d", \
		STR_BUILDER_BUFFER_SIZE), NULL)

#define str_builder_add(buffer, digit, index)			      \
	((index)+1 > STR_BUILDER_BUFFER_SIZE) ? str_builder_err : \
		(buffer[index] = digit)

char chars[] = {
	'+',
	'-',
	'*',
	'/',
	'%',
	'|',
	'~',
	'^',
	'\'',
	'"',
	'!',
	',',
	'.',
	':',
	';',
	'=',
	'?',
	'&',
	')',
	'(',
	']',
	'[',
	'}',
	'{',
	'<',
	'>'
};
char* keychars[] = {
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"<<=",
	">>=",
	"|=",
	"&=",
	"^=",
	"~=",
	"||",
	"==",
	"!=",
	"&&",
	"<<",
	">>",
	"<=",
	">=",
	"++",
	"--",
	"->",
	"..",
	"...",
	"./",	//additional lexemes especially for path
	"../",
};
char* keywords[] = {
	"auto",
	"break",
	"case",
	"chr",
	"const",
	"continue",
	"default",
	"enum",
	"elif",
	"extern",
	"for",
	"fnc",
	"from",
	"false",
	"f32",
	"f64",
	"forward",
	"goto",
	"if",
	"i8",
	"i16",
	"i32",
	"i64",
	"import",
	"intrinsic",
	"register",
	"return",
	"str",
	"static",
	"struct",
	"switch",
	"sizeof",
	"true",
	"typeof",
	"union",
	"u8",
	"u16",
	"u32",
	"u64",
	"let",
	"label",
	"void",
	"volatile",
	"while",
	"do",
	"else",
};

Lexer* lexer_new(const char* input, StreamType input_type)
{
	Lexer* l = new_s(Lexer, l);
	if ((l->input = input_type) == FROM_CHAR_PTR)
	{
		l->stream_origin = input;
		l->stream_size = strlen(l->stream_origin);
		l->curr_file = "undefined";
	}
	else
	{
		FILE* file;
		fopen_s(&file, input, "rb");
		if (!file)
			report_error(frmt("Cannot open file: \'%s\'", input), NULL);
		else
		{
			l->curr_file = input;
			fseek(file, 0L, SEEK_END);
			l->stream_size = ftell(file);
			rewind(file);
			l->stream_origin = newc_s(char, l->stream_origin, l->stream_size + 1);
			fread(l->stream_origin, sizeof(char), l->stream_size, file);
			l->stream_origin[l->stream_size] = '\0';
			fclose(file);
		}
	}

	l->stream = l->stream_origin;
	l->curr_line = l->curr_line_offset = 1;
	return l;
}

void lexer_free(Lexer* lexer)
{
	if (lexer)
	{
		if (lexer->input != FROM_CHAR_PTR)
			free(lexer->stream_origin);
		free(lexer);
	}
}

Token** lex(Lexer* lexer)
{
	Token** tokens = NULL;
	while (!eos(lexer))
	{
		char curr_char = get_curr_char(lexer);
		if (isdigit(curr_char))
			sbuffer_add(tokens, get_num_token(lexer));
		else if (issquote(curr_char))
			sbuffer_add(tokens, get_char_token(lexer));
		else if (isdquote(curr_char))
			sbuffer_add(tokens, get_string_token(lexer));
		else if (isidnt(curr_char))
			sbuffer_add(tokens, get_idnt_token(lexer));
		else if (isknch(curr_char) >= 0)
			sbuffer_add(tokens, get_keychar_token(lexer, isknch(curr_char)));
		else
			if (!isspace(curr_char) && !isescape(curr_char) && !issharp(curr_char))
				report_error(frmt("Unknown char met (code: %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
					src_context_new(lexer->curr_file, lexer->curr_line_offset, 1, lexer->curr_line));
		get_next_char(lexer);
	}
	sbuffer_add(tokens, get_eof_token(lexer));
	return tokens;
}

int check_comment(Lexer* lexer)
{
	if (!matchc(lexer, '#'))
		return 0;
	// manually skip # here
	lexer->stream++;
	lexer->curr_line_offset++;
	if (*lexer->stream != '>')
		sngl_comment(lexer);
	else
	{
		//skip ~ here
		get_next_char(lexer);
		mult_comment(lexer);
	}
	return 1;
}

void mult_comment(Lexer* lexer)
{
	while (!eos(lexer))
	{
		if (!matchc(lexer, '<'))
			get_next_char(lexer);
		else
		{
			get_next_char(lexer);
			if (matchc(lexer, '#'))
			{
				lexer->curr_line_offset++;
				break;
			}
		}
	}
}

void sngl_comment(Lexer* lexer)
{
	while (!eos(lexer))
	{
		get_next_char(lexer);
		if (matchc(lexer, '\n'))
		{
			lexer->prev_line = lexer->curr_line;
			lexer->curr_line++;
			break;
		}
	}
}

int32_t get_next_char(Lexer* lexer)
{
	if (eos(lexer)) 
		return EOF;
	else
	{
		int32_t ch;
		switch (ch = *lexer->stream) 
		{
		case '\r':
			lexer->prev_line_offset =
				lexer->curr_line_offset;
			lexer->curr_line_offset = 1;
			break;
		case '\n':
			lexer->prev_line = lexer->curr_line;
			lexer->curr_line++;
			break;
		case '#':
			if (!check_comment(lexer))
				goto default_case;
			break;
		default:
		default_case:
			lexer->curr_line_offset++;
			break;
		}
		return *(++lexer->stream);
	}
}

int32_t get_curr_char(Lexer* lexer)
{
	return curr_char_fits(lexer) ?
		*lexer->stream : EOF;
}

int32_t unget_curr_char(Lexer* lexer)
{
	if (bos(lexer))
		return EOF;
	else
	{
		int32_t ch;
		lexer->stream -= 1;
		switch (ch = *lexer->stream)
		{
		case '\r':
			lexer->curr_line_offset =
				lexer->prev_line_offset;
			break;
		case '\n':
			lexer->curr_line =
				lexer->prev_line;
			break;
		default:
			lexer->curr_line_offset--;
			break;
		}
		return ch;
	}
}

int get_tokens_format(Lexer* lex)
{
	if (matchc(lex, '0'))
	{
		switch (tolower(get_next_char(lex)))
		{
		case 'x':
			return FORMAT_HEX;
		case 'o':
			return FORMAT_OCT;
		case 'b':
			return FORMAT_BIN;
		default:
			unget_curr_char(lex);
			return FORMAT_DEC;
		}
	}
	return FORMAT_DEC;
}

char is_escape_sequence(Lexer* lex)
{
	if (matchc(lex, '\\'))
	{
		switch (get_next_char(lex))
		{
		case 'a':
			return '\a';
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';
		case '0':
			return '\0';
		case '\\':
			return '\\';
		case '\'':
			return '\'';
		case '\"':
			return '\"';
		default:
			unget_curr_char(lex);
			break;
		}
	}
	return -1;
}

Token* get_eof_token(Lexer* lexer)
{
	Token* token = token_new(TOKEN_EOF, src_context_new(
		lexer->curr_file, lexer->curr_line_offset+1, 1, lexer->curr_line));
	token->svalue = "EOF";
	return token;
}

Token* get_num_token_endian(Lexer* lexer, Token* num_token)
{
	if (num_token->type == TOKEN_FLOAT_CONST)
		return num_token;
	switch (tolower(get_next_char(lexer)))
	{
	case 'i':
		num_token->type = TOKEN_INT_CONST;
		break;
	case 'u':
		num_token->type = TOKEN_UINT_CONST;
		break;
	case 'f':
		num_token->type = TOKEN_FLOAT_CONST;
		break;
	default:
		return unget_curr_char(lexer), num_token;
	}
	//return get_next_char(lexer), num_token;
	return num_token;
}

Token* get_num_token(Lexer* lexer)
{
	int format = get_tokens_format(lexer);
	switch (format)
	{
	case FORMAT_BIN:
		return get_num_token_endian(lexer, 
			get_bin_num_token(lexer));
	case FORMAT_OCT:
		return get_num_token_endian(lexer, 
			get_oct_num_token(lexer));
	case FORMAT_HEX:
		return get_num_token_endian(lexer, 
			get_hex_num_token(lexer));
	case FORMAT_DEC:
		return get_num_token_endian(lexer,
			get_dec_num_token(lexer));
	default:
		report_error(frmt("Unknown format for number met: %d", format), NULL);
	}
	return NULL;
}

Token* get_bin_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_bin(get_next_char(lexer)))
	{
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_oct_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_oct(get_next_char(lexer)))
	{
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_hex_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_hex(get_next_char(lexer)))
	{
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_dec_num_token(Lexer* lexer)
{
	// size is 1 because i already met digit
	uint32_t size = 1;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, get_curr_char(lexer), 0);

	while (isdigit_ext(get_next_char(lexer)))
	{
		if (matchc(lexer, '.'))
			return get_dec_fnum_token(lexer, buffer, size);
		if (matchc(lexer, 'e') || matchc(lexer, 'E'))
			return get_dec_expnum_token(lexer, buffer, size, 0);
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_dec_fnum_token(Lexer* lexer, char* buffer, uint32_t size)
{
	if (!matchc(lexer, '.'))
		report_error(frmt("Expected dot character for float number representation!"), NULL);
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9'))
		report_error(frmt("Expected at least one digit after \'.\'!"), 
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit_fext(get_next_char(lexer)))
	{
		if (matchc(lexer, 'e') || matchc(lexer, 'E'))
			return get_dec_expnum_token(lexer, buffer, size, 1);
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_FLOAT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_dec_expnum_token(Lexer* lexer, char* buffer, uint32_t size, char is_float)
{
	if (tolower(get_curr_char(lexer)) != 'e')
		//context is null because this error in my case will never occure
		report_error(frmt("Expected exponent sign!"), NULL); 
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	int32_t ch = get_next_char(lexer);
	if (ch != '+' && ch != '-')
		report_error(frmt("Unknown char for decimal exponent form. Expected \'-\' or \'+\'!"), 
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	ch = get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9'))
		report_error(frmt("Expected at least one digit after exponent sign!"), 
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit(get_next_char(lexer)))
	{
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(is_float ? TOKEN_FLOAT_CONST : TOKEN_INT_CONST,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_idnt_token(Lexer* lexer)
{
	uint32_t size = 1;
	str_builder_create_buffer(idnt);
	str_builder_add(idnt, get_curr_char(lexer), 0);

	while (isidnt_ext(get_next_char(lexer)))
	{
		str_builder_add(idnt, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, idnt, size);
	int order = iskeyword(idnt);
	Token* token = (order >= 0) ? get_keyword_token(lexer, order) :
		token_new(TOKEN_IDNT, src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = idnt;
	return token;
}

Token* get_char_token(Lexer* lexer)
{
	char character;
	char is_escape;
	get_next_char(lexer);
	character = ((is_escape = is_escape_sequence(lexer)) >= 0) ?
		is_escape : get_curr_char(lexer);
	get_next_char(lexer);
	if (!matchc(lexer, '\''))
		report_error(frmt("Expected single quote, but met (code %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
			src_context_new(lexer->curr_file, lexer->curr_line_offset, is_escape > 0 ? 4 : 3, lexer->curr_line));

	Token* token = token_new(TOKEN_CHARACTER,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, is_escape > 0 ? 4 : 3, lexer->curr_line));
	token->cvalue = character;
	return token;
}

Token* get_string_token(Lexer* lexer)
{
	char curr_char;
	char is_escape;
	uint32_t size = 2;
	uint32_t index = 0;

	str_builder_create_buffer(str);

	while (!eos(lexer) && isstrc(get_next_char(lexer)))
	{
		curr_char = ((is_escape = is_escape_sequence(lexer)) >= 0) ?
			is_escape : get_curr_char(lexer);
		str_builder_add(str, curr_char, index);
		index++;
		size += (is_escape > 0) ? 2 : 1;
	}
	if (!matchc(lexer, '\"'))
		report_error(frmt("Expected double quote, but met (code %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	str_builder_reduce_buffer(temp, str, index);
	Token* token = token_new(TOKEN_STRING,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = str;
	return token;
}

Token* get_keyword_token(Lexer* lexer, int order)
{
	char* keyword = keywords[order];
	Token* token = token_new(order + KEYWORD_IN_TOKEN_ENUM_OFFSET,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, strlen(keyword), lexer->curr_line));
	token->svalue = keyword;
	return token;
}

Token* get_keychar_token(Lexer* lexer, int order)
{
	// if we found any equal string in ext_chars array
	char found = 1;
	int32_t ret = -1;
	int32_t type = -1;
	uint32_t size = 2;
	str_builder_create_buffer(str);
	str_builder_add(str, chars[order], 0);

	while (found && !eos(lexer))
	{
		found = 0;
		str_builder_add(str, get_next_char(lexer), size-1);
		str_builder_add(str, '\0', size);
		if ((ret = isextch(str)) < 0)
			size--;
		if (found = (ret >= 0))
			size++;
		type = ret >= 0 ? ret : type;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, str, size);
	Token* token = token_new(strlen(str) == 1 ? (order + CHARS_IN_TOKEN_ENUM_OFFSET) : (type + EXT_CHARS_IN_TOKEN_ENUM_OFFSET),
		src_context_new(lexer->curr_file, lexer->curr_line_offset, strlen(str), lexer->curr_line));
	token->svalue = str;
	return token;
}

inline int isknch(const char ch)
{
	for (int i = 0; i < CHARS; i++)
		if (ch == chars[i])
			return i;
	return -1;
}

inline int isextch(const char* ech)
{
	for (int i = 0; i < EXT_CHARS; i++)
		if (strcmp(ech, keychars[i]) == 0)
			return i;
	return -1;
}

//returns -1 if idnt is not the, either returns the order in keywords array
inline int iskeyword(const char* idnt)
{
	for (int i = 0; i < KEYWORDS; i++)
		if (strcmp(idnt, keywords[i]) == 0)
			return i;
	return -1;
}