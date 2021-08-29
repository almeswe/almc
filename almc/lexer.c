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
			FILE* file = fopen(src, "rt");
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

Token* lexer_get_tokens(Lexer* lex)
{
	char curr_char;
	Token* tokens = NULL;
	Token* current = NULL;
	while (!is_stream_empty(lex))
	{
		curr_char = get_curr_char(lex);
		if (isdigit(curr_char))
			current = get_num_token(lex);
		else if (isidnt(curr_char))
			current = get_idnt_token(lex);
		else
		{
			//todo: isreserved
			get_next_char(lex);
			continue;
		}
		sbuffer_add(tokens, *current);
	}
	return tokens;
}

//lexer->init -> tokens
//					|
//				get_token

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

	SrcContext* cntxt = src_context_new("undefined", '?', size, '?');
	Token* token = token_new(TOKEN_NUM, cntxt);
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

	SrcContext* cntxt = src_context_new("undefined", '?', size, '?');
	Token* token = token_new(TOKEN_IDNT, cntxt);
	token->idnt_value = value;
	return token;
}
