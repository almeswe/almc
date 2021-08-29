#include "lexer.h"
#include <assert.h>

Lexer* lexer_new(const char* src, InputStreamType type)
{
	Lexer* lex = calloc(1, sizeof(Lexer));
	switch (type)
	{
		case STREAM_FILE:
		{
			//todo: correct file mode?
			FILE* file = fopen(src, "rb+");
			assert(file);
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
			sbuffer_add(tokens, *get_num_token(lex));
		else if (isidnt(curr_char))
			sbuffer_add(tokens, *get_idnt_token(lex));
		//todo: else if reserved
		else if (isspace(curr_char))
			get_next_char(lex);
		else
		{
			printf("This character [{'%c'} line: %d, pos: %d] is not supported by lexer!\n", get_curr_char(lex), lex->curr_line, lex->curr_line_offset);
			get_next_char(lex);
		}
	}	
	return tokens;
}

char get_next_char(Lexer* lex)
{
	char ch;
	if (is_stream_empty(lex))
		return '\0';
	else
	{
		//todo: add full escape sequence table??
		//todo: first char is skipped, fix this
		switch (ch = get__next_char(lex))
		{
		case '\n':
			lex->curr_line++;
			break;
		case '\r':
			lex->curr_line_offset = 0;
			break;
		case '\v': //???
		case '\t':
			break;
		//todo: add comment support
		//todo: add preprocessor instead? 
		case '/':
		{
			//todo: + escape chars here
			ch = get_next_char(lex);
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
		}
		default:
			lex->curr_line_offset++;
			break;
		}
	}
	return ch;
}

Token* get_next_token()
{
	return NULL;
}

Token* get_num_token(Lexer* lex)
{
	size_t size = 1; 
	uint64_t int_value = get_curr_char(lex) - '0';
	uint64_t prev_int_value = int_value;

	while (isdigit(get_next_char(lex)))
	{
		//todo: float point support
		//todo: int32 and int64 token support
		int_value *= 10;
		int_value += get_curr_char(lex) - '0';
		if (prev_int_value > int_value)
			//todo: create smth normal for raising error
			assert(0);
		prev_int_value = int_value;
		size++;
	}

	Token* token = token_new(
		TOKEN_NUM,
		src_context_new(
			"undefined",
			lex->curr_line_offset,
			size,
			lex->curr_line
		)
	);
	token->int_value = int_value;
	return token;
}

Token* get_idnt_token(Lexer* lex)
{
	size_t size = 1;
	char* value = NULL;
	sbuffer_add(value, get_curr_char(lex));

	while (isidnt_ext(get_next_char(lex)))
	{
		sbuffer_add(value, get_curr_char(lex));
		size++;
	}
	sbuffer_add(value, '\0');

	Token* token = token_new(
		TOKEN_IDNT,
		src_context_new(
			"undefined",
			lex->curr_line_offset,
			size,
			lex->curr_line
		)
	);
	token->idnt_value = sbuffer_rdc(value, sbuffer_len(value));
	return token;
}

void multi_line_comment(Lexer* lex)
{
	char curr = get__next_char(lex);
	while (!is_stream_empty(lex))
	{
		if (curr == '*')
		{
			curr = get__next_char(lex);
			if (curr == '/')
				break;
		}
		else
			curr = get__next_char(lex);
	}
}

void single_line_comment(Lexer* lex)
{
	char curr = get__next_char(lex);
	while (!is_stream_empty(lex) && curr != '\n')
		curr = get__next_char(lex);
}

int fgetc_ext(FILE* file)
{
	return fseek(file, 0, SEEK_CUR);
}

inline int isidnt(char ch)
{
	return isalpha(ch) || ch == '_';
}
inline int isidnt_ext(char ch)
{
	return isidnt(ch) || isdigit(ch);
}
inline int isreserved(char ch)
{

}