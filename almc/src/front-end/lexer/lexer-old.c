#include "lexer.h"

//todo: rewrite lexer?

//TODO: lexer backup
//TODO: add boolean

#define get__next_char_cstream(lexer) (*(++lexer->char_stream))
#define get__next_char_fstream(lexer) (fgetc(lexer->file_stream))
#define get__next_char(lexer) ((lexer->stream_type == STREAM_FILE) ? get__next_char_fstream(lexer) : get__next_char_cstream(lexer))

#define get__curr_char_cstream(lexer) (*lexer->char_stream)
#define get__curr_char_fstream(lexer) (fpeek(lexer->file_stream))

#define unget__curr_char_fstream(lexer) (ungetc(get__curr_char_fstream(lexer), lexer->file_stream))
#define unget__curr_char_cstream(lexer) (--lexer->char_stream)
#define unget__curr_char(lexer) ((lexer->stream_type == STREAM_FILE) ? unget__curr_char_fstream(lexer) : unget__curr_char_cstream(lexer))

#define get_curr_char(lexer) ((lexer->stream_type == STREAM_FILE) ? get__curr_char_fstream(lexer) : get__curr_char_cstream(lexer))

#define check_stream(lexer) ((lexer->stream_type == STREAM_FILE) ? (feof(lexer->file_stream)) : (*lexer->char_stream == '\0'))
#define close_stream(lexer) ((lexer->stream_type == STREAM_FILE) ? (fclose(lexer->file_stream)) : 0)

#define matchc(lexer, c) (get_curr_char(lexer) == c)
#define matchc_in(lexer, c1, c2) ((get_curr_char(lexer)) >= (c1) && (get_curr_char(lexer)) <= (c2))

// Initializes all needed variables for sft_with_overflow macro 
//TODO: REFACTOR THIS MACROSES
#define sft_init_vars(max_sfts, format) char shifts = 0; char max_shifts = max_sfts; const char base = format; char met = 0
#define sft_zero_check(c) (!met && c == '0' ? max_shifts++ : met++)
#define sft_with_overflow(a) shifts++, ((shifts <= max_shifts) ? a = a * base : report_error("Integer size is too large.", NULL))
#define add_init_vars(a) uint64_t prev_value = a
#define add_with_overflow(a, b) ((a <= (ULLONG_MAX - (uint64_t)(b)) && (a >= prev_value)) ? (a = a + (uint64_t)(b), prev_value = a) : report_error("Integer size is too large.", NULL))

const char chars[] = {
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
const char* ext_chars[] = {
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

const char* keywords[] = {
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
	"let",
	"label",
	"void",
	"volatile",
	"while",
	"do",
	"else",
};
const char* tokens_str[] = {
	"TOKEN_PLUS",
	"TOKEN_DASH",
	"TOKEN_ASTERISK",
	"TOKEN_SLASH,",
	"TOKEN_MODULUS",
	"TOKEN_BAR",
	"TOKEN_TILDE",
	"TOKEN_CARET",
	"TOKEN_S_QUOTE",
	"TOKEN_D_QUOTE",
	"TOKEN_EXCL_MARK",
	"TOKEN_COMMA",
	"TOKEN_DOT",
	"TOKEN_COLON",
	"TOKEN_SEMICOLON",
	"TOKEN_ASSIGN",
	"TOKEN_QUESTION",
	"TOKEN_AMPERSAND",
	"TOKEN_CL_PAREN",
	"TOKEN_OP_PAREN",
	"TOKEN_CL_BRACKET",
	"TOKEN_OP_BRACKET",
	"TOKEN_CL_BRACE",
	"TOKEN_OP_BRACE",
	"TOKEN_LEFT_ANGLE",
	"TOKEN_RIGHT_ANGLE",

	"TOKEN_ADD_ASSIGN",
	"TOKEN_SUB_ASSIGN",
	"TOKEN_MUL_ASSIGN",
	"TOKEN_DIV_ASSIGN",
	"TOKEN_MOD_ASSIGN",
	"TOKEN_LSHIFT_ASSIGN",
	"TOKEN_RSHIFT_ASSIGN",

	"TOKEN_BW_OR_ASSIGN",
	"TOKEN_BW_AND_ASSIGN",
	"TOKEN_BW_XOR_ASSIGN",
	"TOKEN_BW_NOT_ASSIGN",

	"TOKEN_LG_OR",
	"TOKEN_LG_EQ",
	"TOKEN_LG_NEQ",
	"TOKEN_LG_AND",

	"TOKEN_LSHIFT",
	"TOKEN_RSHIFT",
	"TOKEN_LESS_EQ_THAN",
	"TOKEN_GREATER_EQ_THAN",
	"TOKEN_INC",
	"TOKEN_DEC",
	"TOKEN_ARROW",

	"TOKEN_STRING",
	"TOKEN_CHARACTER",
	"TOKEN_INUM",
	"TOKEN_FNUM",
	"TOKEN_IDNT",

	"TOKEN_KEYWORD_AUTO",
	"TOKEN_KEYWORD_BREAK",
	"TOKEN_KEYWORD_CASE",
	"TOKEN_KEYWORD_CHAR",
	"TOKEN_KEYWORD_CONST",
	"TOKEN_KEYWORD_CONTINUE",
	"TOKEN_KEYWORD_DEFAULT",
	"TOKEN_KEYWORD_ENUM",
	"TOKEN_KEYWORD_ELIF",
	"TOKEN_KEYWORD_EXTERN",
	"TOKEN_KEYWORD_FOR",
	"TOKEN_KEYWORD_FUNC",
	"TOKEN_KEYWORD_FALSE",
	"TOKEN_KEYWORD_FLOAT32",
	"TOKEN_KEYWORD_FLOAT64",
	"TOKEN_KEYWORD_GOTO",
	"TOKEN_KEYWORD_IF",
	"TOKEN_KEYWORD_INT8",
	"TOKEN_KEYWORD_INT16",
	"TOKEN_KEYWORD_INT32",
	"TOKEN_KEYWORD_INT64",
	"TOKEN_KEYWORD_REGISTER",
	"TOKEN_KEYWORD_RETURN",
	"TOKEN_KEYWORD_STRING",
	"TOKEN_KEYWORD_STATIC",
	"TOKEN_KEYWORD_STRUCT",
	"TOKEN_KEYWORD_SWITCH",
	"TOKEN_KEYWORD_SIZEOF",
	"TOKEN_KEYWORD_TRUE",
	"TOKEN_KEYWORD_TYPEOF",
	"TOKEN_KEYWORD_UNION",
	"TOKEN_KEYWORD_UINT8",
	"TOKEN_KEYWORD_UINT16",
	"TOKEN_KEYWORD_UINT32",
	"TOKEN_KEYWORD_UINT64",
	"TOKEN_KEYWORD_LET",
	"TOKEN_KEYWORD_LABEL",
	"TOKEN_KEYWORD_VOID",
	"TOKEN_KEYWORD_VOLATILE",
	"TOKEN_KEYWORD_WHILE",
	"TOKEN_KEYWORD_DO",
	"TOKEN_KEYWORD_ELSE",
	"TOKEN_EOF",
};

Lexer* lexer_new(const char* src, InputStreamType type)
{
	Lexer* lex = new_s(Lexer, lex);
	switch (type)
	{
		case STREAM_FILE:
		{
			FILE* file;
			fopen_s(&file, src, "rb+");
			if (!file)
				report_error(frmt("Cannot open file: \'%s\'", src), NULL);
			lex->curr_file = src;
			lex->file_stream = file;
			break;
		}
		case STREAM_CHAR_PTR:
		{
			lex->curr_file = "undefined";
			lex->char_stream = src;
			break;
		}
	}
	lex->stream_type = type;
	lex->curr_line = 1;
	lex->curr_line_offset = 1;
	return lex;
}

Token* token_new(TokenType type, SrcContext* context)
{
	Token* t = new_s(Token, t);
	t->type = type;
	t->context = context;
	return t;
}

void lexer_free(Lexer* lexer)
{
	//todo: need properly free tokens 
	//for (int i = 0; i < sbuffer_len(lexer->tokens); i++)
	//	token_free(&lexer->tokens[i]);
	sbuffer_free(lexer->tokens);
	free(lexer);
}

void token_free(Token* token)
{
	src_context_free(token->context);
	free(token);
}

char* token_tostr(Token* token)
{
	char* str;
	if (token->type == TOKEN_FNUM)
		str = frmt("%s: %f", token_type_tostr(token->type), token->fvalue);
	else if (token->type == TOKEN_INUM)
		str = frmt("%s: %u", token_type_tostr(token->type), token->ivalue);
	else if (token->type == TOKEN_CHARACTER ||
		    (token->type >= TOKEN_PLUS && token->type <= TOKEN_RIGHT_ANGLE))
		str = frmt("%s: %c", token_type_tostr(token->type), token->char_value);
	else
		str = frmt("%s: %s", token_type_tostr(token->type), token->svalue);
	return frmt("%s %s", str, src_context_tostr(token->context));
}

char* token_type_tostr(TokenType type)
{
	return (type >= 0 && type < TOKEN_EOF) ?
		tokens_str[type] : tokens_str[0];
}

void unget_curr_char(Lexer* lex)
{
	if (!check_stream(lex))
	{
		unget__curr_char(lex);
		switch (get_curr_char(lex))
		{
		case '\n':
			lex->curr_line -= 1;
			break;
		case '\r':
			lex->curr_line_offset = lex->backup.prev_line_offset;
			break;
		//case '\v':
		//case '\t':
		//	break;
		default:
			lex->curr_line_offset--;
			break;
		}
	}
}
char get_next_char(Lexer* lex)
{
	if (check_stream(lex))
		return '\0';
	else
	{
		char ch;
		switch (ch = get_curr_char(lex))
		{
		case '\n':
			lex->curr_line += 1;
			break;
		case '\r':
			lex->backup.prev_line_offset = lex->curr_line_offset;
			lex->curr_line_offset = 1;
			break;
		//case '\v': 
		//case '\t':
		//	break;
		//todo: deal with comments (mb remove them out)
		/*case '/':
		{
			ch = get__next_char(lex);
			switch (ch)
			{
			case '/':
				scomment(lex);
				break;
			case '*':
				mcomment(lex);
				break;
			default:
				unget__curr_char(lex);
				lex->curr_line_offset += 2;
				break;
			}
			break;
		}*/
		default:
			lex->curr_line_offset++;
			break;
		}
	}
	return get__next_char(lex);
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
			unget__curr_char(lex);
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

Token* lex(Lexer* lexer)
{
	char curr_char;
	lexer->tokens = NULL;
	while (!check_stream(lexer))
	{
		curr_char = get_curr_char(lexer);
		if (isdigit(curr_char))
			sbuffer_add(lexer->tokens, *get_num_token(lexer));
		else if (issquote(curr_char))
			sbuffer_add(lexer->tokens, *get_char_token(lexer));
		else if (isdquote(curr_char))
			sbuffer_add(lexer->tokens, *get_string_token(lexer));
		else if (isidnt(curr_char))
			sbuffer_add(lexer->tokens, *get_idnt_token(lexer));
		else if (isknch(curr_char) >= 0)
			sbuffer_add(lexer->tokens, *get_keychar_token(lexer, isknch(curr_char)));
		else
			if (!isspace(curr_char) && !isescape(curr_char))
				report_error(frmt("Unknown char met (code: %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
					src_context_new(lexer->curr_file, lexer->curr_line_offset, 1, lexer->curr_line));
		get_next_char(lexer);
	}
	close_stream(lexer);
	sbuffer_add(lexer->tokens, *get_eof_token(lexer));
	return lexer->tokens;
}

Token* get_eof_token(Lexer* lexer)
{
	size_t len = sbuffer_len(lexer->tokens);
	SrcContext* prev_context = len > 1 ? lexer->tokens[len - 1].context :
		src_context_new(lexer->curr_file, 0, 1, 1);
	SrcContext* new_context  = src_context_new(
		prev_context->file, prev_context->start + prev_context->size, 1, prev_context->line);
	Token* token = token_new(TOKEN_EOF, new_context);
	token->svalue = "EOF";
	return token;
}

Token* get_num_token(Lexer* lexer)
{
	int format = get_tokens_format(lexer);
	switch (format)
	{
	case FORMAT_BIN:
		return get_bin_num_token(lexer);
	case FORMAT_OCT:
		return get_oct_num_token(lexer);
	case FORMAT_HEX:
		return get_hex_num_token(lexer);
	case FORMAT_DEC:
		return get_dec_num_token(lexer);
	}
	report_error(frmt("Unknown format for number met (code: %d).", format), NULL);
	return NULL;
}

Token* get_hex_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	uint64_t value = 0;
	add_init_vars(value);
	sft_init_vars(16, 16);

	while (isdigit_hex(get_next_char(lexer)))
	{
		sft_zero_check(get_curr_char(lexer));
		sft_with_overflow(value);
		if (matchc_in(lexer, '0', '9'))
			add_with_overflow(value, get_curr_char(lexer) - '0');
		else if (matchc_in(lexer, 'a', 'f'))
			add_with_overflow(value, get_curr_char(lexer) - 'a' + 10);
		else if (matchc_in(lexer, 'A', 'F'))
			add_with_overflow(value, get_curr_char(lexer) - 'A' + 10);
		size++;
	}
	unget_curr_char(lexer);

	Token* token = token_new(TOKEN_INUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->ivalue = value;
	return token;
}

Token* get_oct_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	uint64_t value = 0;
	add_init_vars(value);
	sft_init_vars(22, 8);

	while (isdigit_oct(get_next_char(lexer)))
	{
		sft_zero_check(get_curr_char(lexer));
		sft_with_overflow(value);
		add_with_overflow(value, get_curr_char(lexer) - '0');
		size++;
	}
	unget_curr_char(lexer);
	if (shifts == max_shifts)
		report_warning("Integer overflow may occure. Check the max 64bit value for octal number.",
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));

	Token* token = token_new(TOKEN_INUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->ivalue = value;
	return token;
}

Token* get_bin_num_token(Lexer* lexer)
{
	uint32_t size = 2;
	uint64_t value = 0;
	add_init_vars(value);
	sft_init_vars(64, 2);

	while (isdigit_bin(get_next_char(lexer)))
	{
		sft_zero_check(get_curr_char(lexer));
		sft_with_overflow(value);
		add_with_overflow(value, get_curr_char(lexer) - '0');
		size++;
	}
	unget_curr_char(lexer);

	Token* token = token_new(TOKEN_INUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->ivalue = value;
	return token;
}

Token* get_dec_num_token(Lexer* lexer)
{
	uint32_t size = 1; 
	uint64_t value = get_curr_char(lexer) - '0';
	add_init_vars(value);
	sft_init_vars(19, 10);
	sft_zero_check(get_curr_char(lexer));

	while (isdigit_ext(get_next_char(lexer)))
	{
		if (matchc(lexer, '.'))
			return get_dec_fnum_token(lexer, value, size);
		sft_zero_check(get_curr_char(lexer));
		sft_with_overflow(value);
		add_with_overflow(value, get_curr_char(lexer) - '0');
		size++;
	}
	unget_curr_char(lexer);
	if (shifts == max_shifts)
		report_warning("Integer overflow may occure. Check the max 64bit value for decimal number.", 
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));

	Token* token = token_new(TOKEN_INUM, 
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->ivalue = value;
	return token;
}

Token* get_dec_fnum_token(Lexer* lexer, uint64_t base_inum, uint32_t size)
{
	double scalar = 0.1;
	double float_value = (double)base_inum;

	while (isdigit(get_next_char(lexer)))
	{
		float_value += (scalar * (get_curr_char(lexer) - '0'));
		scalar /= 10;
		size++;
	}
	unget_curr_char(lexer);
	
	Token* token = token_new(TOKEN_FNUM,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->fvalue = float_value;
	return token;
}

Token* get_idnt_token(Lexer* lexer)
{
	uint32_t size = 1;
	char* value = NULL;
	sbuffer_add(value, get_curr_char(lexer));

	while (isidnt_ext(get_next_char(lexer)))
	{
		sbuffer_add(value, get_curr_char(lexer));
		size++;
	}
	sbuffer_add(value, '\0');
	unget_curr_char(lexer);

	int order = iskeyword(value);
	Token* token = (order >= 0) ? get_keyword_token(lexer, order) :
		 token_new(TOKEN_IDNT, src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = value;
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
	token->char_value = character;
	return token;
}

Token* get_string_token(Lexer* lexer)
{
	char curr_char;
	char is_escape;
	char* str = NULL;
	uint32_t size = 2;

	while (!check_stream(lexer) && isstrc(get_next_char(lexer)))
	{
		curr_char = ((is_escape = is_escape_sequence(lexer)) >= 0) ?
			is_escape : get_curr_char(lexer);
		sbuffer_add(str, curr_char);
		size += (is_escape > 0) ? 2 : 1;
	}
	if (!matchc(lexer, '\"'))
		report_error(frmt("Expected double quote, but met (code %d): [%c]", (int)get_curr_char(lexer), get_curr_char(lexer)),
			src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	sbuffer_add(str, '\0');

	Token* token = token_new(TOKEN_STRING,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, size, lexer->curr_line));
	token->svalue = str;
	return token;
}

Token* get_keychar_token(Lexer* lexer, int order)
{
	//todo: refactor this
	#define appendc(c) index++, str[index-1] = c, str[index] = '\0'
	#define popc() index--, appendc('\0')  
	#define schar() (strlen(str) == 1)

	char found = 0;
	int32_t index = 0;
	int32_t type = -1;
	char* str = newc_s(char, str, 5);
	appendc(chars[order]);

	do
	{
		appendc(get_next_char(lexer));
		for (int i = 0; i < EXT_CHARS; i++)
		{
			found = 0;
			if (strcmp(ext_chars[i], str) == 0)
			{
				type = i + EXT_CHARS_IN_TOKEN_ENUM_OFFSET;
				found = 1;
				break;
			}
		}
	} while (found && !check_stream(lexer));
	popc();
	unget_curr_char(lexer);

	Token* token = token_new(schar() ? (order + CHARS_IN_TOKEN_ENUM_OFFSET) : type,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, strlen(str), lexer->curr_line));
	if (schar())
		token->char_value = str[0];
	else
		token->svalue = str;
	return token;
}

Token* get_keyword_token(Lexer* lexer, int order)
{
	const char* keyword = keywords[order];
	Token* token = token_new(order + KEYWORD_IN_TOKEN_ENUM_OFFSET,
		src_context_new(lexer->curr_file, lexer->curr_line_offset, strlen(keyword), lexer->curr_line));
	token->svalue = keyword;
	return token;
}

void mcomment(Lexer* lexer)
{
	char curr = get_next_char(lexer);
	while (!check_stream(lexer))
	{
		if (matchc(lexer, '*'))
		{
			curr = get_next_char(lexer);
			if (matchc(lexer, '/'))
				break;
			else
				unget_curr_char(lexer);
		}
		else
			curr = get_next_char(lexer);
	}
}

void scomment(Lexer* lexer)
{
	char curr = get_next_char(lexer);
	while (!check_stream(lexer) && !matchc(lexer, '\n'))
		curr = get_next_char(lexer);
	if (matchc(lexer, '\n'))
		get_next_char(lexer);
}

int fpeek(FILE* file)
{
	fseek(file, -1, SEEK_CUR);
	return fgetc(file);
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