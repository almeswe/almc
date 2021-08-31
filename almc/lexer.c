#include "lexer.h"
#include <assert.h>

//TODO: add match (macro or function) for chars & tokens (for future)
//TODO: add mergede operators support

const char chars[] = {
	'+',
	'-',
	'*',
	'\\',
	'%',
	'|',
	'~',
	'^',
	'\'',
	'"',
	'!',
	',',
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
	'>',
	'<'
};
const char* ext_chars[] = {
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"<<=",
	">>=",
	"~=",
	"|=",
	"&=",
	"^=",
	"||",
	"!=",
	"==",
	"&&",
	"<<",
	">>",
	"++",
	"--",
};
const char* keywords[] = {
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"double",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typeof",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",
	"do",
	"else",
};

//char* curr_lexer_file = NULL;

Lexer* lexer_new(const char* src, InputStreamType type)
{
	Lexer* lex = malloc(sizeof(Lexer));
	switch (type)
	{
		case STREAM_FILE:
		{
			FILE* file = fopen(src, "rb+");
			lex->file_stream = file;
			break;
		}
		case STREAM_CHAR_PTR:
		{
			lex->char_stream = src;
			break;
		}
	}
	lex->type = type;
	lex->curr_line = 1;
	lex->curr_line_offset = 1;
	return lex;
}
Token* token_new(TokenType type, SrcContext* context)
{
	Token* t = malloc(sizeof(Token));
	t->type = type;
	t->context = context;
	return t;
}
SrcContext* src_context_new(const char* file, size_t start, size_t size, size_t line)
{
	SrcContext* c = malloc(sizeof(SrcContext));
	c->size = size;
	c->file = file;
	c->line = line;
	c->start = start;
	return c;
}

Token* lexer_get_tokens(Lexer* lex)
{
	char curr_char;
	Token* tokens = NULL;
	while (!is_stream_empty(lex))
	{
		curr_char = get_curr_char(lex);
		if (isdigit(curr_char))
			sbuffer_add(tokens, *get_inum_token(lex));
		else if (isidnt(curr_char))
			sbuffer_add(tokens, *get_idnt_token(lex));
		else if (isknch(curr_char) >= 0)
			sbuffer_add(tokens, *get_char_token(lex, isknch(curr_char)));
		else
			if (!isescape(curr_char) && !isspace(curr_char))
				printf("This character [{'%c'} line: %d, pos: %d] is not supported by lexer!\n", get_curr_char(lex), lex->curr_line, lex->curr_line_offset);
		get_next_char(lex);
	}	
	return lex->tokens = tokens;
}

char get_next_char(Lexer* lex)
{
	if (is_stream_empty(lex))
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
			lex->curr_line_offset = 1;
			break;
		case '\v': //???
		case '\t':
			break;
		//todo: add escape chars support
		//todo: comment logic occures bug with 
		//todo: add preprocessor instead? 
		case '/':
		{
			//todo: may be usafe using get__next_char()
			ch = get__next_char(lex);
			switch (ch)
			{
			case '/':
				single_line_comment(lex);
				break;
			case '*':
				multi_line_comment(lex);
				break;
			//todo: escape char can be next -> bug, the offset will be greater by 1 
			default:
				lex->curr_line_offset += 2;
				break;
			}
			break;
		}
		default:
			lex->curr_line_offset++;
			break;
		}
	}
	return get__next_char(lex);
}

Token* get_next_token()
{
	return NULL;
}

Token* get_inum_token(Lexer* lex)
{
	size_t size = 1; 
	uint64_t int_value = get_curr_char(lex) - '0';
	uint64_t prev_int_value = int_value;

	while (isdigit_ext(get_next_char(lex)))
	{
		//todo: int32 and int64 token support
		//todo: hex, binary?
		//todo: extra chars for types (f.ex. L)??
		if (matchc(lex, '.'))
			return get_fnum_token(lex, int_value, size);
		int_value *= 10;
		int_value += get_curr_char(lex) - '0';
		if (prev_int_value > int_value)
			//todo: create smth normal for raising error
			assert(!"int64 overflow ocurred!");
		prev_int_value = int_value;
		size++;
	}

	Token* token = token_new(TOKEN_INUM, 
		src_context_new("undefined", lex->curr_line_offset, size, lex->curr_line));
	token->int_value = int_value;
	return token;
}

Token* get_fnum_token(Lexer* lex, uint64_t base_inum, size_t size)
{
	double scalar = 0.1;
	double float_value = (double)base_inum;

	while (isdigit(get_next_char(lex)))
	{
		//todo: overflow check?
		float_value += (scalar * (get_curr_char(lex) - '0'));
		scalar /= 10;
		size++;
	}
	//it means that after dot was no any char
	//todo: resolve this, idk if its needed (i mean check for any digits after dot)
	//if (scalar == 0.1)
	//	assert(0);

	Token* token = token_new(TOKEN_FNUM,
		src_context_new("undefined", lex->curr_line_offset, size, lex->curr_line));
	token->float_value = float_value;
	return token;
}

Token* get_idnt_token(Lexer* lex)
{
	size_t size = 1;
	char* value = NULL;
	Token* token = NULL;
	sbuffer_add(value, get_curr_char(lex));

	while (isidnt_ext(get_next_char(lex)))
	{
		sbuffer_add(value, get_curr_char(lex));
		size++;
	}
	sbuffer_add(value, '\0');

	int order = iskeyword(value);
	token = (order >= 0) ? get_keyword_token(lex, order) :
		 token_new(TOKEN_IDNT, src_context_new("undefined", lex->curr_line_offset, size, lex->curr_line));
	token->idnt_value = sbuffer_rdc(value, sbuffer_len(value));
	return token;
}

Token* get_char_token(Lexer* lex, int order)
{
	#define appendc(c) index++, str[index-1] = c, str[index] = '\0'
	#define popc() index--, appendc('\0')  
	#define single_char() (strlen(str) == 1)
	//todo: doc here for all vars
	char str[5];
	int index = 0;
	int found = 0;
	int type = -1;
	appendc(chars[order]);

	do
	{
		appendc(get_next_char(lex));
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
	} while (found && !is_stream_empty(lex));
	popc();
	unget__curr_char(lex);

	Token* token = token_new(single_char() ? (order + CHARS_IN_TOKEN_ENUM_OFFSET) : type,
		src_context_new("undefined", lex->curr_line_offset, strlen(str), lex->curr_line));
	token->op_value = str;
	return token;
}

Token* get_keyword_token(Lexer* lex, int order)
{
	const char* keyword = keywords[order];
	return token_new(order + KEYWORD_IN_TOKEN_ENUM_OFFSET,
		src_context_new("undefined", lex->curr_line_offset, strlen(keyword), lex->curr_line));
}

void multi_line_comment(Lexer* lex)
{
	char curr = get_next_char(lex);
	while (!is_stream_empty(lex))
	{
		if (matchc(lex, '*'))
		{
			curr = get_next_char(lex);
			if (matchc(lex, '/'))
				break;
		}
		else
			curr = get_next_char(lex);
	}
}

void single_line_comment(Lexer* lex)
{
	char curr = get_next_char(lex);
	while (!is_stream_empty(lex) && !matchc(lex, '\n'))
		curr = get_next_char(lex);
	//todo: wrong
	if (matchc(lex, '\n'))
		get_next_char(lex);
}

int fgetc_ext(FILE* file)
{
	fseek(file, -1, SEEK_CUR);
	return fgetc(file);
}

inline int isidnt(char ch)
{
	return isalpha(ch) || ch == '_';
}
inline int isidnt_ext(char ch)
{
	return isidnt(ch) || isdigit(ch);
}
inline int isdigit_ext(char ch)
{
	return isdigit(ch) || ch == '.';
}
inline int isescape(const char ch)
{
	switch (ch)
	{
	case '\a':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return 1;
	}
	return 0;
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