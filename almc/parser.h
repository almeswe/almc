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

Type* parse_type(Parser* parser);

/*Expr* parse_unary_cast_expr(Parser* parser);
Expr* parse_unary_datasize_expr(Parser* parser);
Expr* parse_unary_typesize_expr(Parser* parser);
*/

Expr* parse_primary_expr(Parser* parser);
Expr* postfix_expr(Parser* parser);
Expr* parse_unary_expr(Parser* parser);

Expr* parse_mul_arith_expr(Parser* parser);
Expr* parse_add_arith_expr(Parser* parser);
Expr* parse_sft_expr(Parser* parser);
Expr* parse_rel_expr(Parser* parser);
Expr* parse_equ_expr(Parser* parser);

Expr* parse_and_bw_expr(Parser* parser);
Expr* parse_xor_bw_expr(Parser* parser);
Expr* parse_or_bw_expr(Parser* parser);

Expr* parse_and_lg_expr(Parser* parser);
Expr* parse_or_lg_expr(Parser* parser);

Expr* parse_conditional_expr(Parser* parser);
Expr* parse_constant_expr(Parser* parser);

Expr* parse_assignment_expr(Parser* parser);

#endif