#include "parser.h"

//todo: add eof token!!
//todo: refactor macroses

#define get_curr_token(parser)   parser->tokens[parser->token_index]
#define unget_curr_token(parser) parser->token_index > 0 ? parser->token_index-- : 0 
#define get_next_token(parser)   parser->token_index < sbuffer_len(parser->tokens) ? &parser->tokens[parser->token_index++] : NULL
#define matcht(parser, t) (get_curr_token(parser).type == (t))

#define expect(parser, etype, str) (!matcht(parser, etype) ? \
	report_error(frmt("Expected \'%s\', but met: %s", str, TOKEN_TYPE_STR(get_curr_token(parser).type)), get_curr_token(parser).context) : 0)				
#define expect_with_skip(parser, type, str) expect(parser, type, str), get_next_token(parser)

Parser* parser_new(Lexer* lexer)
{
	Parser* p = new_s(Parser, p);
	p->token_index = 0;
	p->tokens = lexer->tokens;
	return p;
}

AstRoot* parse(Parser* parser)
{
	AstRoot* ast = new_s(AstRoot, ast);
	ast->exprs = NULL;
	sbuffer_add(ast->exprs, parse_expr(parser));
}

Expr* parse_expr(Parser* parser)
{
	return parse_add_arith_expr(parser);
}

Type* parse_type(Parser* parser)
{
	//todo: add modifiers
	Type* type = type_new(get_curr_token(parser).str_value);
	get_next_token(parser);
	while (matcht(parser, TOKEN_ASTERISK))
	{
		type->mods.is_ptr++;
		get_next_token(parser);
	}
	return type;
}

Expr* parse_paren_expr(Parser* parser)
{
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	Expr* expr = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	return expr;
}

Expr* parse_primary_expr(Parser* parser)
{
	Expr* expr = NULL;
	Token token = get_curr_token(parser);
	switch (token.type)
	{
	case TOKEN_INUM:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_UINT, token.ivalue, token.context));
		break;
	case TOKEN_FNUM:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_FLOAT, token.fvalue, token.context));
		break;
	case TOKEN_IDNT:
		expr = expr_new(EXPR_IDNT,
			idnt_new(token.str_value, token.context));
		break;
	case TOKEN_OP_PAREN:
		return parse_paren_expr(parser);
	default:
		report_error(frmt("Met unexpected token: %s", TOKEN_TYPE_STR(token.type)), token.context);
	}
	get_next_token(parser);
	return expr;
}

Expr* parse_unary_expr(Parser* parser)
{
	/*
		unary-expression ::=
			  primary-expression 
			| sizeof-expression
			| '+' unary-expression
			| '-' unary-expression

	*/
	switch (get_curr_token(parser).type)
	{
		//TODO: add !, ~, casts, increments
	case TOKEN_PLUS:
		get_next_token(parser);
		return expr_new(EXPR_UNARY_EXPR, 
			unary_expr_new(UNARY_PLUS, parse_unary_expr(parser)));
	case TOKEN_DASH:
		get_next_token(parser);
		return expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(UNARY_MINUS, parse_unary_expr(parser)));
	/*case TOKEN_KEYWORD_SIZEOF:
		get_next_token(parser);
		return expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(UNARY_SIZEOF, parse_unary_expr(parser)));*/
	case TOKEN_KEYWORD_CAST:
		return parse_unary_cast_expr(parser);
	
	default:
		return parse_primary_expr(parser);
	}
}

/*Expr* parse_unary_sizeof_expr(Parser* parser)
{
	//todo: expect macro
	//todo: maybe merge cast logic to sizeof
	if (!matcht(parser, TOKEN_KEYWORD_SIZEOF))
		report_error(frmt("Expected \'sizeof\', but met: %d",
			get_curr_token(parser).type), get_curr_token(parser).context);
	get_next_token(parser);
	switch (get_curr_token(parser).type)
	{
	case TOKEN_OP_PAREN:
		get_next_token(parser);
		switch (get_curr_token(parser).type)
		{
		//todo: type parse here too
		case TOKEN_PREDEF_TYPE:
			get_next_token(parser);
			if (!matcht(parser, TOKEN_KEYWORD_SIZEOF))
				report_error(frmt("Expected \')\', but met: %d",
					get_curr_token(parser).type), get_curr_token(parser).context);
			get_next_token(parser);
			return expr_new(EXPR_UNARY_EXPR,
				unary_expr_new(UNARY_MINUS, idnt_new(get_curr_token(parser).)));
		}
	default:
		return parse_unary_expr(parser);
	}
}*/

Expr* parse_unary_cast_expr(Parser* parser)
{
	expect_with_skip(parser, TOKEN_KEYWORD_CAST, "cast");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	Type* type = parse_type(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	Expr* cast_expr = expr_new(EXPR_UNARY_EXPR,
		unary_expr_new(UNARY_CAST, parse_unary_expr(parser)));
	cast_expr->unary_expr->cast_type = type;
	return cast_expr;
}

Expr* parse_mul_arith_expr(Parser* parser)
{
	/*
	multiplicative-expression ::=
		  unary-expression
		| multiplicative-expression '*' unary-expression
		| multiplicative-expression '/' unary-expression
		| multiplicative-expression '%' unary-expression
	*/

	Expr* mul_expr = NULL;
	Expr* unary_expr = parse_unary_expr(parser);
	while (matcht(parser, TOKEN_SLASH)
		|| matcht(parser, TOKEN_MODULUS)
		|| matcht(parser, TOKEN_ASTERISK))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_SLASH:
			type = BINARY_DIV; break;
		case TOKEN_MODULUS:
			type = BINARY_MOD; break;
		case TOKEN_ASTERISK:
			type = BINARY_MULT; break;
		}
		get_next_token(parser);
		mul_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (mul_expr) ? mul_expr : unary_expr, parse_unary_expr(parser)));
	}
	return (mul_expr) ? mul_expr : unary_expr;
}

Expr* parse_add_arith_expr(Parser* parser)
{
	/*
	additive-expression ::=
		  multiplicative-expression
		| additive-expression '+' multiplicative-expression
		| additive-expression '-' multiplicative-expression
	*/

	Expr* add_expr = NULL;
	Expr* mul_expr = parse_mul_arith_expr(parser);
	while (matcht(parser, TOKEN_PLUS)
		|| matcht(parser, TOKEN_DASH))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_PLUS:
			type = BINARY_ADD; break;
		case TOKEN_DASH:
			type = BINARY_SUB; break;
		}
		get_next_token(parser);
		add_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (add_expr) ? add_expr : mul_expr, parse_mul_arith_expr(parser)));
	}
	return (add_expr) ? add_expr : mul_expr;
}