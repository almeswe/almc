#include "lexer.h"

SrcContext* context_create(const char* file, size_t sym, size_t line)
{
	SrcContext* context = malloc(sizeof(SrcContext));
	context->sym = sym;
	context->file = file;
	context->line = line;
	return context;
}

Token* token_create(char* value, TokenType type, SrcContext context)
{
	Token* token = malloc(sizeof(Token));
	token->type = type;
	token->value = value;
	token->context = context;
}