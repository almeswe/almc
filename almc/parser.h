#ifndef ALMC_PARSER_H
#define ALMC_PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct Parser
{
	Token* tokens;
	uint32_t token_index;
} Parser;

Parser* parser_new(Lexer* lexer);

AstRoot* parse(Parser* parser);
Expr* parse_expr(Parser* parser);
Expr* parse_paren_expr(Parser* parser);
Expr* parse_additive_expr(Parser* parser);
Expr* parse_multiplicative_expr(Parser* parser);

Expr* parse_unary_expr(Parser* parser);
Expr* parse_unary_cast_expr(Parser* parser);
Expr* parse_mul_arith_expr(Parser* parser);
Expr* parse_add_arith_expr(Parser* parser);
Expr* parse_primary_expr(Parser* parser);

#endif