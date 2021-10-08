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

#define zeroLL 48LL

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

char* ext_chars[] = {
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

	"false",
	"f32",
	"f64",
	"goto",
	"if",
	"i8",
	"i16",
	"i32",
	"i64",
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
	"void",
	"volatile",
	"while",
	"do",
	"else",
};

Lexer* lexer_new(char* input, StreamType input_type)
{
	Lexer* l = new_s(Lexer, l);
	if (input_type == FROM_CHAR_PTR)
	{
		l->stream_origin = input;
		l->stream_size = strlen(l->stream_origin);
		l->curr_file = "undefined";
	}
	else
	{
		FILE* file;
		fopen_s(&file, input, "r");
		if (!file)
			report_error(frmt("Cannot open file: \'%s\'", input), NULL);
		else
		{
			l->curr_file = input;
			fseek(file, 0L, SEEK_END);
			l->stream_size = ftell(file);
			rewind(file);
			l->stream_origin = newc_s(char, l->stream_origin, l->stream_size);
			fread(l->stream_origin, sizeof(char), l->stream_size, file);
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
//		else if (issquote(curr_char))
//			sbuffer_add(tokens, get_char_token2(lexer));
//		else if (isdquote(curr_char))
//			sbuffer_add(tokens, get_string_token2(lexer));
//		else if (isidnt(curr_char))
//			sbuffer_add(tokens, get_idnt_token2(lexer));
//		else if (isknch(curr_char) >= 0)
//			sbuffer_add(tokens, get_keychar_token2(lexer, isknch(curr_char)));
		else
			if (!isspace(curr_char) && !isescape(curr_char))
				report_error(frmt("Unknown char met (code: %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
					src_context_new(lexer->curr_file, lexer->curr_line_offset, 1, lexer->curr_line));
		get_next_char(lexer);
	}
	sbuffer_add(tokens, get_eof_token(lexer, tokens));
	return tokens;
}

int32_t get_next_char(Lexer* lexer)
{
	if (eos(lexer)) 
		return EOF;
	else
	{
		int32_t ch;
		lexer->stream += 1;
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
		default:
			lexer->curr_line_offset++;
			break;
		}
		return ch;
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
		// todo: probably not the proper way to return this char
		return EOF;
		//return *lexer->stream_origin;
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
			//todo: solve this macro
			//macro here because of bug with 0 repr in decimal
			unget_curr_char(lex);
			return FORMAT_DEC;
		}
	}
	return FORMAT_DEC;
}

Token* get_eof_token(Lexer* lexer, Token** tokens)
{
	size_t len = sbuffer_len(tokens);
	SrcContext* prev_context = len > 1 ? tokens[len - 1]->context :
		src_context_new(lexer->curr_file, 0, 1, 1);
	SrcContext* new_context = src_context_new(
		prev_context->file, prev_context->start + prev_context->size, 1, prev_context->line);
	Token* token = token_new(TOKEN_EOF, new_context);
	token->svalue = "EOF";
	return token;
}

#define NUM_BUILDER_BUFFER 128

#define num_builder_create_buffer(buffer) \
	char* buffer = newc_s(char, buffer, NUM_BUILDER_BUFFER)

//todo: fix realloc warning
#define num_builder_reduce_buffer(buffer, size) \
	buffer = rnew_s(char, buffer, size+1);      \
	buffer[size] = '\0';

#define num_builder_err										\
	report_error(frmt("Number size should be less than %d", \
		NUM_BUILDER_BUFFER), NULL)

#define num_builder_add(buffer, digit, index)			 \
	((index)+1 > NUM_BUILDER_BUFFER) ? num_builder_err : \
		(buffer[index] = digit)

Token* get_num_token(Lexer* lexer)
{
	int format = get_tokens_format(lexer);
	switch (format)
	{
//	case FORMAT_BIN:
//		return get_bin_num_token2(lexer);
	//case FORMAT_OCT:
//		return get_oct_num_token2(lexer);
//	case FORMAT_HEX:
//		return get_hex_num_token2(lexer);
	case FORMAT_DEC:
		return get_dec_num_token(lexer);
	}
	report_error(frmt("Unknown format for number met (code: %d).", format), NULL);
	return NULL;
}

Token* get_dec_expnum_token(Lexer* lexer, char* buffer, uint32_t size, char is_float)
{
	if (tolower(get_curr_char(lexer)) != 'e')
		report_error(frmt("Expected exponent sign!"), NULL);
	num_builder_add(buffer, get_curr_char(lexer), size); size++;
	int32_t ch = get_next_char(lexer);
	if (ch != '+' && ch != '-')
		report_error(frmt("Unknown char for decimal exponent form. Expected \'-\' or \'+\'!"), NULL);
	num_builder_add(buffer, get_curr_char(lexer), size); size++;
	ch = get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9'))
		report_error(frmt("Expected at least one digit after \'e\'!"), NULL);
	num_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit(get_next_char(lexer)))
	{
		num_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	num_builder_reduce_buffer(buffer, size);
	Token* token = token_new(is_float? TOKEN_FNUM : TOKEN_INUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_dec_num_token(Lexer* lexer)
{
	// size is 1 because i already met digit
	uint32_t size = 1;
	num_builder_create_buffer(buffer);
	num_builder_add(buffer, get_curr_char(lexer), 0);

	while (isdigit_ext(get_next_char(lexer)))
	{
		if (matchc(lexer, '.'))
			return get_dec_fnum_token(lexer, buffer, size);
		if (matchc(lexer, 'e') || matchc(lexer, 'E'))
			return get_dec_expnum_token(lexer, buffer, size, 0);
		num_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	num_builder_reduce_buffer(buffer, size);
	Token* token = token_new(TOKEN_INUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

Token* get_dec_fnum_token(Lexer* lexer, char* buffer, uint32_t size)
{
	if (!matchc(lexer, '.'))
		report_error(frmt("Expected dot character for float number representation!"), NULL);
	num_builder_add(buffer, get_curr_char(lexer), size); size++;
	get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9'))
		report_error(frmt("Expected at least one digit after \'.\'!"), NULL);
	num_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit_fext(get_next_char(lexer)))
	{
		if (matchc(lexer, 'e') || matchc(lexer, 'E'))
			return get_dec_expnum_token(lexer, buffer, size, 1);
		num_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	num_builder_reduce_buffer(buffer, size);
	Token* token = token_new(TOKEN_FNUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = buffer;
	return token;
}

inline int isknch(const char ch)
{
	for (int i = 0; i < CHARS; i++)
		if (ch == chars[i])
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