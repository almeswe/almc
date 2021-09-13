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
	return parse_constant_expr(parser);
}

Type* parse_type(Parser* parser)
{
	//todo: add modifiers
	//todo: recode this type_new
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

Expr* postfix_expr(Parser* parser)
{

}

Expr* parse_unary_expr(Parser* parser)
{
	/*
		<unary-expression> ::= <postfix-expression>
							 | ++ <unary-expression>
							 | -- <unary-expression>
							 | <unary-operator> <cast-expression>
							 | sizeof <unary-expression>
							 | sizeof <type-name>

		<unary-operator> ::= &
				| *
				| +
				| -
				| ~
				| !

	*/

	//todo: remake this for switch (now occurrs bug)
	assert(0);
	Expr* unary_expr = NULL;
	while (matcht(parser, TOKEN_INC)
		|| matcht(parser, TOKEN_DEC)
		|| matcht(parser, TOKEN_PLUS)
		|| matcht(parser, TOKEN_DASH)
		|| matcht(parser, TOKEN_TILDE)
		|| matcht(parser, TOKEN_ASTERISK)
		|| matcht(parser, TOKEN_AMPERSAND)
		|| matcht(parser, TOKEN_EXCL_MARK))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_INC:
			type = UNARY_INC; break;
		case TOKEN_DEC:
			type = UNARY_DEC; break;
		case TOKEN_PLUS:
			type = UNARY_PLUS; break;
		case TOKEN_DASH:
			type = UNARY_MINUS; break;
		case TOKEN_TILDE:
			type = UNARY_BW_NOT; break;
		case TOKEN_EXCL_MARK:
			type = UNARY_LG_NOT; break;
		case TOKEN_AMPERSAND:
			type = UNARY_ADDRESS; break;
		case TOKEN_ASTERISK:
			type = UNARY_DEREFERENCE; break;
		}
		get_next_token(parser);
		unary_expr = expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(type, parse_unary_expr(parser)));
	}
	return unary_expr ? unary_expr : parse_primary_expr(parser);

	// todo: for now this three unaries ignored
	/*case TOKEN_KEYWORD_CAST:
		return parse_unary_cast_expr(parser);
	case TOKEN_KEYWORD_DATASIZE:
		return parse_unary_datasize_expr(parser);
	case TOKEN_KEYWORD_TYPESIZE:
		return parse_unary_typesize_expr(parser);*/
}

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

Expr* parse_unary_datasize_expr(Parser* parser)
{
	expect_with_skip(parser, TOKEN_KEYWORD_DATASIZE, "datasize");
	return expr_new(EXPR_UNARY_EXPR,
		unary_expr_new(UNARY_DATASIZE, parse_unary_expr(parser)));
}

Expr* parse_unary_typesize_expr(Parser* parser)
{
	expect_with_skip(parser, TOKEN_KEYWORD_TYPESIZE, "typesize");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	Type* type = parse_type(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	Expr* typesize_expr = expr_new(EXPR_UNARY_EXPR,
		unary_expr_new(UNARY_TYPESIZE, NULL));
	typesize_expr->unary_expr->cast_type = type;
	return typesize_expr;
}

Expr* parse_mul_arith_expr(Parser* parser)
{
	/*
	<multiplicative-expression> ::= <cast-expression>
								  | <multiplicative-expression> * <cast-expression>
								  | <multiplicative-expression> / <cast-expression>
								  | <multiplicative-expression> % <cast-expression>
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
	<additive-expression> ::= <multiplicative-expression>
							| <additive-expression> + <multiplicative-expression>
							| <additive-expression> - <multiplicative-expression>
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

Expr* parse_sft_expr(Parser* parser)
{
	/*
	<shift-expression> ::= <additive-expression>
                     | <shift-expression> << <additive-expression>
                     | <shift-expression> >> <additive-expression>
	*/

	Expr* sft_expr = NULL;
	Expr* add_expr = parse_add_arith_expr(parser);
	while (matcht(parser, TOKEN_LSHIFT)
		|| matcht(parser, TOKEN_RSHIFT))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_LSHIFT:
			type = BINARY_LSHIFT; break;
		case TOKEN_RSHIFT:
			type = BINARY_RSHIFT; break;
		}
		get_next_token(parser);
		sft_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (sft_expr) ? sft_expr : add_expr, parse_add_arith_expr(parser)));
	}
	return (sft_expr) ? sft_expr : add_expr;
}

Expr* parse_rel_expr(Parser* parser)
{
	/*
	<relational-expression> ::= <shift-expression>
                          | <relational-expression> < <shift-expression>
                          | <relational-expression> > <shift-expression>
                          | <relational-expression> <= <shift-expression>
                          | <relational-expression> >= <shift-expression>
	*/

	Expr* rel_expr = NULL;
	Expr* sft_expr = parse_sft_expr(parser);
	while (matcht(parser, TOKEN_LEFT_ANGLE)
		|| matcht(parser, TOKEN_RIGHT_ANGLE)
		|| matcht(parser, TOKEN_LESS_EQ_THAN)
		|| matcht(parser, TOKEN_GREATER_EQ_THAN))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_LEFT_ANGLE:
			type = BINARY_LESS_THAN; break;
		case TOKEN_RIGHT_ANGLE:
			type = BINARY_GREATER_THAN; break;
		case TOKEN_LESS_EQ_THAN:
			type = BINARY_LESS_EQ_THAN; break;
		case TOKEN_GREATER_EQ_THAN:
			type = BINARY_GREATER_EQ_THAN; break;
		}
		get_next_token(parser);
		rel_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (rel_expr) ? rel_expr : sft_expr, parse_sft_expr(parser)));
	}
	return (rel_expr) ? rel_expr : sft_expr;
}

Expr* parse_equ_expr(Parser* parser)
{
	/*
	<equality-expression> ::= <relational-expression>
                        | <equality-expression> == <relational-expression>
                        | <equality-expression> != <relational-expression>
	*/

	Expr* equ_expr = NULL;
	Expr* rel_expr = parse_rel_expr(parser);
	while (matcht(parser, TOKEN_LG_EQ)
		|| matcht(parser, TOKEN_LG_NEQ))
	{
		BinaryExprType type = 0;
		switch (get_curr_token(parser).type)
		{
		case TOKEN_LG_EQ:
			type = BINARY_LG_EQ; break;
		case TOKEN_LG_NEQ:
			type = BINARY_LG_NEQ; break;
		}
		get_next_token(parser);
		equ_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (equ_expr) ? equ_expr : rel_expr, parse_rel_expr(parser)));
	}
	return (equ_expr) ? equ_expr : rel_expr;
}

Expr* parse_and_bw_expr(Parser* parser)
{
	/*
	<and-expression> ::= <equality-expression>
                   | <and-expression> & <equality-expression>
	*/

	Expr* and_expr = NULL;
	Expr* equ_expr = parse_equ_expr(parser);
	while (matcht(parser, TOKEN_AMPERSAND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_AND, (and_expr) ? and_expr : equ_expr, parse_equ_expr(parser)));
	}
	return (and_expr) ? and_expr : equ_expr;
}

Expr* parse_xor_bw_expr(Parser* parser)
{
	/*
	<exclusive-or-expression> ::= <and-expression>
                            | <exclusive-or-expression> ^ <and-expression>
	
	*/

	Expr* xor_expr = NULL;
	Expr* and_expr = parse_and_bw_expr(parser);
	while (matcht(parser, TOKEN_CARET))
	{
		get_next_token(parser);
		xor_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_XOR, (xor_expr) ? xor_expr : and_expr, parse_and_bw_expr(parser)));
	}
	return (xor_expr) ? xor_expr : and_expr;
}

Expr* parse_or_bw_expr(Parser* parser)
{
	/*
	<inclusive-or-expression> ::= <exclusive-or-expression>
                            | <inclusive-or-expression> | <exclusive-or-expression>
	*/

	Expr* or_expr = NULL;
	Expr* xor_expr = parse_xor_bw_expr(parser);
	while (matcht(parser, TOKEN_BAR))
	{
		get_next_token(parser);
		or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_OR, (or_expr) ? or_expr : xor_expr, parse_xor_bw_expr(parser)));
	}
	return (or_expr) ? or_expr : xor_expr;
}

Expr* parse_and_lg_expr(Parser* parser)
{
	/*
	<logical-and-expression> ::= <inclusive-or-expression>
                           | <logical-and-expression> && <inclusive-or-expression>
	*/

	Expr* and_expr = NULL;
	Expr* or_expr = parse_or_bw_expr(parser);
	while (matcht(parser, TOKEN_LG_AND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_AND, (and_expr) ? and_expr : or_expr, parse_or_bw_expr(parser)));
	}
	return (and_expr) ? and_expr : or_expr;
}

Expr* parse_or_lg_expr(Parser* parser)
{
	/*
	<logical-or-expression> ::= <logical-and-expression>
                          | <logical-or-expression> || <logical-and-expression>
	*/
	Expr* lg_or_expr = NULL;
	Expr* and_expr = parse_and_lg_expr(parser);
	while (matcht(parser, TOKEN_LG_OR))
	{
		get_next_token(parser);
		lg_or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_OR, (lg_or_expr) ? lg_or_expr : and_expr, parse_and_lg_expr(parser)));
	}
	return (lg_or_expr) ? lg_or_expr : and_expr;
}

Expr* parse_conditional_expr(Parser* parser)
{
	/*
	<conditional-expression> ::= <logical-or-expression>
                           | <logical-or-expression> ? <expression> : <conditional-expression>
	*/

	Expr* cond_expr = NULL;
	Expr* or_expr = parse_or_lg_expr(parser);
	while (matcht(parser, TOKEN_QUESTION))
	{
		get_next_token(parser);
		Expr* lexpr = parse_expr(parser);
		expect_with_skip(parser, TOKEN_COLON, ":");
		cond_expr = expr_new(EXPR_TERNARY_EXPR,
			ternary_expr_new((cond_expr) ? cond_expr : or_expr, lexpr, parse_conditional_expr(parser)));
	}
	return (cond_expr) ? cond_expr : or_expr;
}

Expr* parse_constant_expr(Parser* parser)
{
	return parse_assignment_expr(parser);
}

Expr* parse_assignment_expr(Parser* parser)
{
	/*
	<assignment-expression> ::= <conditional-expression>
                          | <unary-expression> <assignment-operator> <assignment-expression>

	<assignment-operator> ::= =
							| *=
							| /=
							| %=
							| +=
							| -=
							| <<=
							| >>=
							| &=
							| ^=
							| |=
	*/

	Expr* assign_expr = NULL;
	Expr* cond_expr = parse_conditional_expr(parser);
	while (matcht(parser, TOKEN_ASSIGN)
		|| matcht(parser, TOKEN_ADD_ASSIGN)
		|| matcht(parser, TOKEN_SUB_ASSIGN)
		|| matcht(parser, TOKEN_MUL_ASSIGN)
		|| matcht(parser, TOKEN_DIV_ASSIGN)
		|| matcht(parser, TOKEN_MOD_ASSIGN)
		|| matcht(parser, TOKEN_LSHIFT_ASSIGN)
		|| matcht(parser, TOKEN_RSHIFT_ASSIGN)
		|| matcht(parser, TOKEN_BW_NOT_ASSIGN)
		|| matcht(parser, TOKEN_BW_OR_ASSIGN)
		|| matcht(parser, TOKEN_BW_AND_ASSIGN)
		|| matcht(parser, TOKEN_BW_XOR_ASSIGN))
	{
		TokenType ttype = get_curr_token(parser).type;
		BinaryExprType type = ttype == TOKEN_ASSIGN ?
			BINARY_ASSIGN : ttype - 6; // token type enum - binary type enum 
		
		get_next_token(parser);
		assign_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (assign_expr) ? assign_expr : cond_expr, parse_assignment_expr(parser)));
	}
	return (assign_expr) ? assign_expr : cond_expr;
}