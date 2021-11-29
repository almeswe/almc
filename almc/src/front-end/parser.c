#include "parser.h"

//todo: think about how i can access struct members (also if struct is pointer) in initializer

#define matcht(parser, t) (get_curr_token(parser)->type == (t))
#define expect_with_skip(parser, type, str) expect(parser, type, str), get_next_token(parser)
#define token_index_fits(parser) (parser->token_index >= 0 && parser->token_index < sbuffer_len(parser->tokens))

#define context_starts(parser, var_name) \
	SrcContext* var_name = get_curr_token(parser)->context

#define context_ends(parser, starts, node) \
	new__chk(node);                        \
	unget_curr_token(parser);              \
	node->area = src_area_new(starts,      \
		get_curr_token(parser)->context);  \
	get_next_token(parser)					

#define TOKEN_PREDEFINED_TYPE     \
	     TOKEN_KEYWORD_VOID:	  \
	case TOKEN_KEYWORD_CHAR:      \
	case TOKEN_KEYWORD_INT8:      \
	case TOKEN_KEYWORD_INT16:	  \
	case TOKEN_KEYWORD_INT32:	  \
	case TOKEN_KEYWORD_INT64:	  \
	case TOKEN_KEYWORD_UINT8:	  \
	case TOKEN_KEYWORD_UINT16:	  \
	case TOKEN_KEYWORD_UINT32:	  \
	case TOKEN_KEYWORD_UINT64:	  \
	case TOKEN_KEYWORD_FLOAT32:	  \
	case TOKEN_KEYWORD_FLOAT64:   \
	case TOKEN_KEYWORD_STRING      

#define TOKEN_KEYWORD_USER_TYPEDECL \
		 TOKEN_KEYWORD_ENUM:	    \
	case TOKEN_KEYWORD_UNION:       \
	case TOKEN_KEYWORD_STRUCT

#define TOKEN_KEYWORD_LOOP   \
		 TOKEN_KEYWORD_DO:   \
	case TOKEN_KEYWORD_FOR:  \
	case TOKEN_KEYWORD_WHILE

#define TOKEN_KEYWORD_JUMP     \
		 TOKEN_KEYWORD_GOTO:   \
	case TOKEN_KEYWORD_BREAK:  \
	case TOKEN_KEYWORD_RETURN: \
	case TOKEN_KEYWORD_CONTINUE

#define TOKEN_PATH_DESC			\
		 TOKEN_IDNT:			\
	case TOKEN_SLASH:			\
	case TOKEN_STRING:			\
	case TOKEN_NAV_CURR_DIR:	\
	case TOKEN_NAV_PREV_DIR		\

static char** imported_modules = NULL;

Parser* parser_new(char* file, Token** tokens)
{
	Parser* p = new_s(Parser, p);
	p->file = file;
	p->token_index = 0;
	p->tokens = tokens;
	return p;
}

void parser_free(Parser* parser)
{
	if (parser)
	{
		for (int i = 0; i < sbuffer_len(parser->tokens); i++)
			token_free(parser->tokens[i]);
		sbuffer_free(parser->tokens);
		free(parser);
	}
}

Token* get_next_token(Parser* parser)
{
	if (!parser->tokens)
		report_error("Cannot get next token, because parser->tokens is NULL!", NULL);
	else
		return token_index_fits(parser) ?
			parser->tokens[++parser->token_index] : parser->tokens[parser->token_index];
	return NULL;
}

Token* get_curr_token(Parser* parser)
{
	if (!parser->tokens)
		report_error("Cannot get current token, because parser->tokens is NULL!", NULL);
	else
		return token_index_fits(parser) ?
			parser->tokens[parser->token_index] : parser->tokens[0];
	return NULL;
}

void unget_curr_token(Parser* parser)
{
	if (parser->token_index > 0)
		parser->token_index--;
}

void expect(Parser* parser, TokenKind type, const char* token_value)
{
	if (!matcht(parser, type))
	{
		Token* token = get_curr_token(parser);
		report_error(frmt(
			"Expected \'%s\', but met: %s",
			token_value,
			token_type_tostr(token->type)),
			token->context
		);
	}
}

AstRoot* parse(Parser* parser)
{
	AstRoot* ast = new_s(AstRoot, ast);
	ast->stmts = NULL;
	while (!matcht(parser, TOKEN_EOF))
		sbuffer_add(ast->stmts, parse_stmt(parser));
	return ast;
}

Expr* parse_expr(Parser* parser)
{
	return parse_comma_expr(parser);
}

Type* parse_abstract_declarator(Parser* parser, Type* type)
{
	/*
	<abstract-declarator> ::= <pointer>
					| <pointer> <direct-abstract-declarator>
					| <direct-abstract-declarator>

	<direct-abstract-declarator> ::=  ( <abstract-declarator> )
			| {<direct-abstract-declarator>}? [ {<constant-expression>}? ]
			| {<direct-abstract-declarator>}? ( {<parameter-type-list>}? )
	*/

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_ASTERISK:
		type->mods.is_ptr++;
		get_next_token(parser);
		return parse_abstract_declarator(parser, type);
	/*case TOKEN_OP_BRACKET:
		type->mods.is_array++;
		get_next_token(parser);
		sbuffer_add(type->info.arr_dim_sizes,
			parse_expr(parser));
		expect_with_skip(parser, TOKEN_CL_BRACKET, "]");
		return parse_abstract_declarator(parser, type);*/
	default:
		return type;
	}
}

Type* parse_type_name(Parser* parser)
{
	/*
	<type-name> ::= {<specifier-qualifier>}+ {<abstract-declarator>}?

	<specifier-qualifier> ::= <type-specifier>
						| <type-qualifier>

	<type-qualifier> ::= const
				   | volatile

	<type-specifier> ::= char
				   | void
				   | int8
				   | int16
				   | int32
				   | uint8
				   | uint16
				   | uint32
				   | uint64
				   | float32
				   | float64
				   | string
				   | idnt
	*/

	context_starts(parser, context);
	Type* type = cnew_s(Type, type, 1);
	Token* token = get_curr_token(parser);

	switch (token->type)
	{
	case TOKEN_IDNT:
	case TOKEN_PREDEFINED_TYPE:
		type->repr = token->svalue;
		type->mods.is_predefined = (token->type != TOKEN_IDNT);
		type->mods.is_void = (token->type == TOKEN_KEYWORD_VOID);
		get_next_token(parser);
		type = parse_abstract_declarator(parser, type);
		context_ends(parser, context, type);
		return type;
	default:
		report_error(frmt("Type expected, but met: %s",
			token_type_tostr(token->type)), token->context);
	}
	return 0;
}

Expr* parse_paren_expr(Parser* parser)
{
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	Expr* expr = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	return expr;
}

Expr* parse_func_call_expr(Parser* parser)
{
	Expr** func_args = NULL;
	const char* func_name = 
		get_curr_token(parser)->svalue;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_IDNT, "func name");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	if (!matcht(parser, TOKEN_CL_PAREN))
	{
		do
		{
			if (sbuffer_len(func_args) && matcht(parser, TOKEN_COMMA))
				get_next_token(parser);
			// assignment expression here needed to avoid recognition of ',' as operator (its separator actually)
			sbuffer_add(func_args, parse_assignment_expr(parser));
		} while (matcht(parser, TOKEN_COMMA));
	}
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	Expr* expr = expr_new(EXPR_FUNC_CALL,
		func_call_new(func_name, func_args));
	context_ends(parser, context, expr->func_call);
	return expr;
}

Expr* parse_idnt_ambigulty(Parser* parser)
{
	get_next_token(parser);
	if (matcht(parser, TOKEN_OP_PAREN))
	{
		unget_curr_token(parser);
		return parse_func_call_expr(parser);
	}
	else
	{
		unget_curr_token(parser);
		return expr_new(EXPR_IDNT,
			idnt_new(get_curr_token(parser)->svalue,
				get_curr_token(parser)->context));
	}
}

Expr* parse_primary_expr(Parser* parser)
{
	Expr* expr = NULL;
	Token* token = get_curr_token(parser);
	switch (token->type)
	{
	case TOKEN_INT_CONST:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_INT, token->svalue, token->context));
		break;
	case TOKEN_UINT_CONST:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_UINT, token->svalue, token->context));
		break;
	case TOKEN_FLOAT_CONST:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_FLOAT, token->svalue, token->context));
		break;
	case TOKEN_IDNT:
		expr = parse_idnt_ambigulty(parser);
		if (expr->kind == EXPR_FUNC_CALL)
			return expr;
		break;
	case TOKEN_STRING:
		expr = expr_new(EXPR_STRING,
			str_new(token->svalue, token->context));
		break;
	case TOKEN_CHARACTER:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_UINT, frmt("%u", (uint32_t)token->cvalue), token->context));
		break;
	case TOKEN_OP_PAREN:
		return parse_paren_expr(parser);
	case TOKEN_OP_BRACE:
		return parse_initializer_expr(parser);
	case TOKEN_KEYWORD_TRUE:
	case TOKEN_KEYWORD_FALSE:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_INT, (int64_t)(token->type == TOKEN_KEYWORD_TRUE ? "1" : "0"), token->context));
		break;
	default:
		report_error(frmt("Primary expression token expected, but met: %s",
			token_type_tostr(token->type)), token->context);
	}
	get_next_token(parser);
	return expr;
}

Expr* parse_postfix_xxcrement_expr(Parser* parser, Expr* expr, TokenKind xxcrmt_type)
{
	Expr* uexpr = NULL;
	context_starts(parser, context);
	Token* token = get_curr_token(parser);

	switch (token->type)
	{
	case TOKEN_INC:
	case TOKEN_DEC:
		uexpr = expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(xxcrmt_type == TOKEN_INC ? 
				UNARY_POSTFIX_INC : UNARY_POSTFIX_DEC, expr));
		get_next_token(parser);
		context_ends(parser, context, uexpr->unary_expr);
		return uexpr;
	default:
		report_error(frmt("Expected postfix (in/de)crement, but met: %s",
			token_type_tostr(token->type)), token->context);
	}
}

Expr* parse_array_accessor_expr(Parser* parser, Expr* rexpr)
{
	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_OP_BRACKET, "[");
	Expr* expr = expr_new(EXPR_BINARY_EXPR,
		binary_expr_new(BINARY_ARR_MEMBER_ACCESSOR, rexpr, parse_expr(parser)));
	expect_with_skip(parser, TOKEN_CL_BRACKET, "]");
	context_ends(parser, context, expr->binary_expr);
	return expr;
}

Expr* parse_member_accessor_expr(Parser* parser, Expr* rexpr, TokenKind accessor_type)
{
	Expr* member_accessor = NULL;
	context_starts(parser, context);
	Token* token = get_curr_token(parser);

	switch (accessor_type)
	{
	case TOKEN_DOT:
	case TOKEN_ARROW:
		//todo: check for func call after referencing
		token = get_next_token(parser);
		if (!matcht(parser, TOKEN_IDNT))
			report_error(frmt("Expected identifier as acessed member, but met: %s",
				token_type_tostr(token->type)), token->context);
		get_next_token(parser);
		member_accessor = expr_new(EXPR_BINARY_EXPR, 
			binary_expr_new(accessor_type == TOKEN_DOT ? BINARY_MEMBER_ACCESSOR : BINARY_PTR_MEMBER_ACCESSOR,
				rexpr, expr_new(EXPR_IDNT, idnt_new(token->svalue, token->context))));
		context_ends(parser, context, member_accessor->binary_expr);
		return member_accessor;
	default:
		report_error(frmt("Expected member accessor, but met: %s",
			token_type_tostr(token->type)), token->context);
	}
}

Expr* parse_postfix_expr(Parser* parser)
{
	/*
	<postfix-expression> ::= <primary-expression>
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++
                       | <postfix-expression> --
	*/

	Expr* postfix_expr = NULL;
	Expr* primary_expr = parse_primary_expr(parser);

	while (matcht(parser, TOKEN_INC)
		|| matcht(parser, TOKEN_DEC)
		|| matcht(parser, TOKEN_DOT)
		|| matcht(parser, TOKEN_ARROW)
		|| matcht(parser, TOKEN_OP_BRACKET))
	{
		Expr* rexpr = postfix_expr ?
			postfix_expr : primary_expr;

		switch (get_curr_token(parser)->type)
		{
		case TOKEN_INC:
		case TOKEN_DEC:
			postfix_expr = parse_postfix_xxcrement_expr(parser,
				rexpr, get_curr_token(parser)->type);
			break;
		case TOKEN_DOT:
		case TOKEN_ARROW:
			postfix_expr = parse_member_accessor_expr(parser,
				rexpr, get_curr_token(parser)->type);
			break;
		case TOKEN_OP_BRACKET:
			postfix_expr = parse_array_accessor_expr(parser,
				rexpr, get_curr_token(parser)->type);
			break;
		}
	}
	return (postfix_expr) ? postfix_expr : primary_expr;
}

Expr* parse_unary_expr(Parser* parser)
{
	/*
		<unary-expression> ::= <postfix-expression>
							 | ++ <unary-expression>
							 | -- <unary-expression>
							 | <unary-operator> <cast-expression>
							 | <sizeof-expression>

		<unary-operator> ::= &
				| *
				| +
				| -
				| ~
				| !

	*/

	Expr* expr = NULL;
	context_starts(parser, context);

#define unary_cast_case(parser, type)                   \
	get_next_token(parser);			                    \
    expr = expr_new(EXPR_UNARY_EXPR,                    \
		unary_expr_new(type, parse_cast_expr(parser))); \
	context_ends(parser, context, expr->unary_expr);    \

#define unary_unary_case(parser, type)                   \
	get_next_token(parser);			                     \
    expr = expr_new(EXPR_UNARY_EXPR,                     \
		unary_expr_new(type, parse_unary_expr(parser))); \
	context_ends(parser, context, expr->unary_expr);     \

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_INC:
		unary_unary_case(parser, UNARY_PREFIX_INC);
		return expr;
	case TOKEN_DEC:
		unary_unary_case(parser, UNARY_PREFIX_DEC);
		return expr;
	case TOKEN_PLUS:
		unary_cast_case(parser, UNARY_PLUS);
		return expr;
	case TOKEN_DASH:
		unary_cast_case(parser, UNARY_MINUS);
		return expr;
	case TOKEN_TILDE:
		unary_cast_case(parser, UNARY_BW_NOT);
		return expr;
	case TOKEN_ASTERISK:
		unary_cast_case(parser, UNARY_DEREFERENCE);
		return expr;
	case TOKEN_AMPERSAND:
		unary_cast_case(parser, UNARY_ADDRESS);
		return expr;
	case TOKEN_EXCL_MARK:
		unary_cast_case(parser, UNARY_LG_NOT);
		return expr;
	case TOKEN_KEYWORD_SIZEOF:
		return parse_sizeof_expr(parser);
	default:
		return parse_postfix_expr(parser);
	}
}

Type* try_to_get_type(Parser* parser)
{
	Type* type = NULL;

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_OP_PAREN:
		get_next_token(parser);
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_IDNT:
			type = parse_type_name(parser);
			if (matcht(parser, TOKEN_CL_PAREN) && type->mods.is_ptr)
				goto type_declaration;
			else
				// <= here because we also need to unget idnt token
				for (int i = 0; i <= type->mods.is_ptr; i++)
					unget_curr_token(parser);
			type = NULL;
			break;
		case TOKEN_PREDEFINED_TYPE:
			type = parse_type_name(parser);
			type_declaration:
			expect_with_skip(parser, TOKEN_CL_PAREN, ")");
			return type;
		}
		unget_curr_token(parser);
	default:
		return type;
	}
}

Expr* parse_cast_expr(Parser* parser)
{
	/*
	<cast-expression> ::= <unary-expression>
						| ( <type-name> ) <cast-expression>
	*/

	Type* type = NULL;
	Expr* expr = NULL;

	if (!(type = try_to_get_type(parser)))
		return parse_unary_expr(parser);
	else
	{
		context_starts(parser, context);
		expr = expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(UNARY_CAST, parse_cast_expr(parser)));
		expr->unary_expr->cast_type = type;
		context_ends(parser, context, expr->unary_expr);
		return expr;
	}
}

Expr* parse_sizeof_expr(Parser* parser)
{
	/*
	<sizeof-expression> ::= sizeof <unary-expression> 
						 || sizeof ( <type-name> )
	*/
	Type* type = NULL;
	Expr* expr = NULL;
	context_starts(parser, context);

	expect_with_skip(parser, TOKEN_KEYWORD_SIZEOF, "sizeof");
	if (!(type = try_to_get_type(parser)))
		expr = expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(UNARY_SIZEOF, parse_unary_expr(parser)));
	else
	{
		expr = expr_new(EXPR_UNARY_EXPR,
			unary_expr_new(UNARY_SIZEOF, NULL));
		expr->unary_expr->cast_type = type;
	}
	context_ends(parser, context, expr->unary_expr);
	return expr;
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
	context_starts(parser, context);
	Expr* unary_expr = parse_cast_expr(parser);

	while (matcht(parser, TOKEN_SLASH)
		|| matcht(parser, TOKEN_MODULUS)
		|| matcht(parser, TOKEN_ASTERISK))
	{
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
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
			binary_expr_new(type, (mul_expr) ? mul_expr : unary_expr, parse_cast_expr(parser)));
		context_ends(parser, context, mul_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* mul_expr = parse_mul_arith_expr(parser);

	while (matcht(parser, TOKEN_PLUS)
		|| matcht(parser, TOKEN_DASH))
	{
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_PLUS:
			type = BINARY_ADD; break;
		case TOKEN_DASH:
			type = BINARY_SUB; break;
		}
		get_next_token(parser);
		add_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (add_expr) ? add_expr : mul_expr, parse_mul_arith_expr(parser)));
		context_ends(parser, context, add_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* add_expr = parse_add_arith_expr(parser);

	while (matcht(parser, TOKEN_LSHIFT)
		|| matcht(parser, TOKEN_RSHIFT))
	{
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_LSHIFT:
			type = BINARY_LSHIFT; break;
		case TOKEN_RSHIFT:
			type = BINARY_RSHIFT; break;
		}
		get_next_token(parser);
		sft_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (sft_expr) ? sft_expr : add_expr, parse_add_arith_expr(parser)));
		context_ends(parser, context, sft_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* sft_expr = parse_sft_expr(parser);

	while (matcht(parser, TOKEN_LEFT_ANGLE)
		|| matcht(parser, TOKEN_RIGHT_ANGLE)
		|| matcht(parser, TOKEN_LESS_EQ_THAN)
		|| matcht(parser, TOKEN_GREATER_EQ_THAN))
	{
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
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
		context_ends(parser, context, rel_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* rel_expr = parse_rel_expr(parser);

	while (matcht(parser, TOKEN_LG_EQ)
		|| matcht(parser, TOKEN_LG_NEQ))
	{
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_LG_EQ:
			type = BINARY_LG_EQ; break;
		case TOKEN_LG_NEQ:
			type = BINARY_LG_NEQ; break;
		}
		get_next_token(parser);
		equ_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (equ_expr) ? equ_expr : rel_expr, parse_rel_expr(parser)));
		context_ends(parser, context, equ_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* equ_expr = parse_equ_expr(parser);

	while (matcht(parser, TOKEN_AMPERSAND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_AND, (and_expr) ? and_expr : equ_expr, parse_equ_expr(parser)));
		context_ends(parser, context, and_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* and_expr = parse_and_bw_expr(parser);

	while (matcht(parser, TOKEN_CARET))
	{
		get_next_token(parser);
		xor_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_XOR, (xor_expr) ? xor_expr : and_expr, parse_and_bw_expr(parser)));
		context_ends(parser, context, xor_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* xor_expr = parse_xor_bw_expr(parser);

	while (matcht(parser, TOKEN_BAR))
	{
		get_next_token(parser);
		or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_OR, (or_expr) ? or_expr : xor_expr, parse_xor_bw_expr(parser)));
		context_ends(parser, context, or_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* or_expr = parse_or_bw_expr(parser);

	while (matcht(parser, TOKEN_LG_AND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_AND, (and_expr) ? and_expr : or_expr, parse_or_bw_expr(parser)));
		context_ends(parser, context, and_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* and_expr = parse_and_lg_expr(parser);

	while (matcht(parser, TOKEN_LG_OR))
	{
		get_next_token(parser);
		lg_or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_OR, (lg_or_expr) ? lg_or_expr : and_expr, parse_and_lg_expr(parser)));
		context_ends(parser, context, lg_or_expr->binary_expr);
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
	context_starts(parser, context);
	Expr* or_expr = parse_or_lg_expr(parser);

	while (matcht(parser, TOKEN_QUESTION))
	{
		get_next_token(parser);
		Expr* lexpr = parse_expr(parser);
		expect_with_skip(parser, TOKEN_COLON, ":");
		cond_expr = expr_new(EXPR_TERNARY_EXPR,
			ternary_expr_new((cond_expr) ? cond_expr : or_expr, lexpr, parse_conditional_expr(parser)));
		context_ends(parser, context, cond_expr->ternary_expr);
	}
	return (cond_expr) ? cond_expr : or_expr;
}

Expr* parse_constant_expr(Parser* parser)
{
	return parse_conditional_expr(parser);
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
	context_starts(parser, context);
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
		BinaryExprKind type = 0;
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_ASSIGN:
			type = BINARY_ASSIGN; break;
		case TOKEN_ADD_ASSIGN:
			type = BINARY_ADD_ASSIGN; break;
		case TOKEN_SUB_ASSIGN:
			type = BINARY_SUB_ASSIGN; break;
		case TOKEN_MUL_ASSIGN:
			type = BINARY_MUL_ASSIGN; break;
		case TOKEN_DIV_ASSIGN:
			type = BINARY_DIV_ASSIGN; break;
		case TOKEN_MOD_ASSIGN:
			type = BINARY_MOD_ASSIGN; break;
		case TOKEN_LSHIFT_ASSIGN:
			type = BINARY_LSHIFT_ASSIGN; break;
		case TOKEN_RSHIFT_ASSIGN:
			type = BINARY_RSHIFT_ASSIGN; break;
		case TOKEN_BW_NOT_ASSIGN:
			type = BINARY_BW_NOT_ASSIGN; break;
		case TOKEN_BW_OR_ASSIGN:
			type = BINARY_BW_OR_ASSIGN; break;
		case TOKEN_BW_AND_ASSIGN:
			type = BINARY_BW_AND_ASSIGN; break;
		case TOKEN_BW_XOR_ASSIGN:
			type = BINARY_BW_XOR_ASSIGN; break;
		}
		get_next_token(parser);
		Expr* init;
		if (matcht(parser, TOKEN_OP_BRACE))
			return expr_new(EXPR_BINARY_EXPR,
				binary_expr_new(type, (assign_expr) ? assign_expr : cond_expr, parse_initializer_expr(parser)));
		assign_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, (assign_expr) ? assign_expr : cond_expr, parse_assignment_expr(parser)));
		context_ends(parser, context, assign_expr->binary_expr);
	}
	return (assign_expr) ? assign_expr : cond_expr;
}

Expr* parse_comma_expr(Parser* parser)
{
	Expr* comma_expr = NULL;
	context_starts(parser, context);
	Expr* assign_expr = parse_assignment_expr(parser);

	while (matcht(parser, TOKEN_COMMA))
	{
		get_next_token(parser);
		comma_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_COMMA, comma_expr ? comma_expr : assign_expr, parse_assignment_expr(parser)));
		context_ends(parser, context, comma_expr->binary_expr);
	}

	return comma_expr ? comma_expr : assign_expr;
}

Expr* parse_initializer_expr(Parser* parser)
{
	Expr** values =	NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		// same thing why i put parse_assignment_expr, but not parse_expr (same with parse_func_call_expr)
		sbuffer_add(values, parse_assignment_expr(parser));
		if (!matcht(parser, TOKEN_CL_BRACE))
			expect_with_skip(parser, TOKEN_COMMA, ",");
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	Expr* expr = expr_new(EXPR_INITIALIZER, 
		initializer_new(values));
	context_ends(parser, context, expr->initializer);
	return expr;
}

Stmt* parse_stmt(Parser* parser)
{
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_IDNT:
		return parse_expr_stmt(parser);
	case TOKEN_OP_BRACE:
		return parse_block(parser);
	case TOKEN_SEMICOLON:
		return parse_empty_stmt(parser);
	case TOKEN_KEYWORD_IF:
		return parse_if_stmt(parser);
	case TOKEN_KEYWORD_LET:
		return parse_var_decl_stmt(parser);
	case TOKEN_KEYWORD_LOOP:
		return parse_loop_stmt(parser);
	case TOKEN_KEYWORD_JUMP:
		return parse_jump_stmt(parser);
	case TOKEN_KEYWORD_FUNC:
		return parse_func_decl_stmt(parser);
	case TOKEN_KEYWORD_FROM:
		return parse_from_import_stmt(parser);
	case TOKEN_KEYWORD_LABEL:
		return parse_label_decl_stmt(parser);
	case TOKEN_KEYWORD_SWITCH:
		return parse_switch_stmt(parser);
	case TOKEN_KEYWORD_IMPORT:
		return parse_import_stmt(parser);
	case TOKEN_KEYWORD_USER_TYPEDECL:
		return parse_type_decl_stmt(parser);
	default:
		return parse_expr_stmt(parser);
	}
}

Stmt* parse_type_decl_stmt(Parser* parser)
{
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_ENUM:
		return parse_enum_decl_stmt(parser);
	case TOKEN_KEYWORD_UNION:
		return parse_union_decl_stmt(parser);
	case TOKEN_KEYWORD_STRUCT:
		return parse_struct_decl_stmt(parser);
	default:
		report_error(frmt("Expected keyword (enum, struct or union), but met: %s",
			token_type_tostr(get_curr_token(parser)->type)), get_curr_token(parser)->context);
	}
}

Stmt* parse_enum_decl_stmt(Parser* parser)
{
	char* enum_name = "";
	Idnt** enum_idnts = NULL;
	Expr* enum_idnt_value = NULL;
	Expr** enum_idnt_values = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_ENUM, "enum");
	// enum can be no-name
	if (matcht(parser, TOKEN_IDNT))
		enum_name = get_curr_token(parser)->svalue,
			expect_with_skip(parser, TOKEN_IDNT, "enum name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		// here i check if the prev-value is not null, if not, add 1 to the previous value
		// and set it as actual value, either create 0 const
		if (!enum_idnt_value)
			enum_idnt_value = expr_new(EXPR_CONST, const_new(CONST_INT, "0", NULL));
		else
			enum_idnt_value = enum_idnt_value->kind == EXPR_CONST ?
			expr_new(EXPR_CONST, const_new(CONST_INT, frmt("%d", enum_idnt_value->cnst->ivalue + 1), NULL)) :
			expr_new(EXPR_BINARY_EXPR, binary_expr_new(BINARY_ADD, enum_idnt_value,
				expr_new(EXPR_CONST, const_new(CONST_INT, "1", NULL))));

		sbuffer_add(enum_idnts, idnt_new(get_curr_token(parser)->svalue,
			get_curr_token(parser)->context));
		expect_with_skip(parser, TOKEN_IDNT, "enum's identifier");
		if (matcht(parser, TOKEN_ASSIGN))
		{
			get_next_token(parser);
			enum_idnt_value = parse_constant_expr(parser);
		}
		sbuffer_add(enum_idnt_values, enum_idnt_value);
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL,
		type_decl_new(TYPE_DECL_ENUM, 
			enum_decl_new(enum_idnts, enum_idnt_values, enum_name)));
}

Stmt* parse_union_decl_stmt(Parser* parser)
{
	char* union_name = "";
	TypeVar** union_mmbrs = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_UNION, "union");
	union_name = get_curr_token(parser)->svalue;
	expect_with_skip(parser, TOKEN_IDNT, "union name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		sbuffer_add(union_mmbrs, parse_type_var(parser));
		expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL,
		type_decl_new(TYPE_DECL_UNION, union_decl_new(union_mmbrs, union_name)));
}

Stmt* parse_struct_decl_stmt(Parser* parser)
{
	char* struct_name = "";
	TypeVar** struct_mmbrs = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_STRUCT, "struct");
	struct_name = get_curr_token(parser)->svalue;
	expect_with_skip(parser, TOKEN_IDNT, "struct name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		sbuffer_add(struct_mmbrs, parse_type_var(parser));
		expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL,
		type_decl_new(TYPE_DECL_STRUCT, struct_decl_new(struct_mmbrs, struct_name)));
}

Stmt* parse_block(Parser* parser)
{
	Stmt** stmts = NULL;
	char is_simple = matcht(parser, TOKEN_OP_BRACE);
	//todo: func decl is illegal
	if (!is_simple)
		sbuffer_add(stmts, parse_stmt(parser));
	else
	{
		expect_with_skip(parser, TOKEN_OP_BRACE, "{");
		while (!matcht(parser, TOKEN_CL_BRACE))
			sbuffer_add(stmts, parse_stmt(parser));
		expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	}
	return stmt_new(STMT_BLOCK, block_new(stmts));
}

Stmt* parse_expr_stmt(Parser* parser)
{
	Expr* expr = parse_expr(parser);

	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_EXPR, expr_stmt_new(expr));
}

Stmt* parse_empty_stmt(Parser* parser)
{
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_EMPTY,
		empty_stmt_new());
}

Stmt* parse_var_decl_stmt(Parser* parser)
{
	Expr* var_init = NULL;
	TypeVar* type_var = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_LET, "let");
	type_var = parse_type_var(parser);
	if (matcht(parser, TOKEN_ASSIGN))
	{
		get_next_token(parser);
		var_init = parse_expr(parser);
	}
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_VAR_DECL,
		var_decl_new(type_var, var_init));
}

FuncSpecifiers* parse_func_specifiers(Parser* parser)
{
	FuncSpecifiers* func_spec = cnew_s(FuncSpecifiers, func_spec, 1);
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_ENTRY:
		get_next_token(parser);
		func_spec->is_entry = 1;
		break;
	case TOKEN_KEYWORD_EXTERN:
		get_next_token(parser);
		func_spec->is_external = 1;
		break;
	case TOKEN_KEYWORD_FORWARD:
		get_next_token(parser);
		func_spec->is_forward = 1;
		break;
	case TOKEN_KEYWORD_INTRINSIC:
		get_next_token(parser);
		func_spec->is_intrinsic = 1;
		break;
	}
	return func_spec;
}

Stmt* parse_func_decl_stmt(Parser* parser)
{
	char* func_name = NULL;
	Type* func_type = NULL;
	Block* func_body = NULL;
	TypeVar** func_params = NULL;
	FuncSpecifiers* func_spec = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_FUNC, "fnc");
	func_spec = parse_func_specifiers(parser);
	func_name = get_curr_token(parser)->svalue;
	expect_with_skip(parser, TOKEN_IDNT, "func name");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	while (!matcht(parser, TOKEN_CL_PAREN))
	{
		sbuffer_add(func_params, parse_type_var(parser));
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
	}
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	expect_with_skip(parser, TOKEN_COLON, ":");
	func_type = parse_type_name(parser);
	
	// checking if function does not have any block
	if (matcht(parser, TOKEN_SEMICOLON))
		get_next_token(parser);
	else
	{
		if (!matcht(parser, TOKEN_OP_BRACE))
			expect_with_skip(parser, TOKEN_OP_BRACE, "{");
		func_body = parse_block(parser)->block;
	}
	return stmt_new(STMT_FUNC_DECL, 
		func_decl_new(func_name, func_params, func_type, func_body, *func_spec));
}

Stmt* parse_label_decl_stmt(Parser* parser)
{
	Idnt* label  = cnew_s(Idnt, label, 1);

	expect_with_skip(parser, TOKEN_KEYWORD_LABEL, "label");
	label->svalue = get_curr_token(parser)->svalue;
	label->context = get_curr_token(parser)->context;
	expect_with_skip(parser, TOKEN_IDNT, "label name");
	expect_with_skip(parser, TOKEN_COLON, ":");
	return stmt_new(STMT_LABEL_DECL, label_decl_new(label));
}

Stmt* parse_loop_stmt(Parser* parser)
{
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_DO:
		return parse_do_loop_stmt(parser);
	case TOKEN_KEYWORD_FOR:
		return parse_for_loop_stmt(parser);
	case TOKEN_KEYWORD_WHILE:
		return parse_while_loop_stmt(parser);
	default:
		report_error(frmt("Expected keyword (do, while or for), but met: %s",
			token_type_tostr(get_curr_token(parser)->type)), get_curr_token(parser)->context);
	}
}

Stmt* parse_do_loop_stmt(Parser* parser)
{
	Expr* do_cond = NULL;
	Block* do_body = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_DO, "do");
	do_body = parse_block(parser)->block;
	expect_with_skip(parser, TOKEN_KEYWORD_WHILE, "while");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	do_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_LOOP,
		loop_stmt_new(LOOP_DO, do_loop_new(do_cond, do_body)));
}

Stmt* parse_for_loop_stmt(Parser* parser)
{
	Expr* for_cond = NULL;
	Expr* for_step = NULL;
	Block* for_body = NULL;
	VarDecl* for_init = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_FOR, "for");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	if (!matcht(parser, TOKEN_SEMICOLON))
		for_init = parse_var_decl_stmt(parser)->var_decl;
	else
		expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	if (!matcht(parser, TOKEN_SEMICOLON))
		for_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	if (!matcht(parser, TOKEN_CL_PAREN))
		for_step = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	for_body = parse_block(parser)->block;
	return stmt_new(STMT_LOOP, 
		loop_stmt_new(LOOP_FOR, for_loop_new(for_init, for_cond, for_step, for_body)));
}

Stmt* parse_while_loop_stmt(Parser* parser)
{
	Expr* while_cond = NULL;
	Block* while_body = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_WHILE, "while");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	while_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	while_body = parse_block(parser)->block;
	return stmt_new(STMT_LOOP, 
		loop_stmt_new(LOOP_WHILE, while_loop_new(while_cond, while_body)));
}

Stmt* parse_if_stmt(Parser* parser)
{
	Expr* if_cond = NULL;
	Block* if_body = NULL;
	ElseIf** elifs = NULL;
	Block* else_body = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_IF, "if");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	if_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	if_body = parse_block(parser)->block;
	while (matcht(parser, TOKEN_KEYWORD_ELIF))
		sbuffer_add(elifs, parse_elif_stmt(parser));
	if (matcht(parser, TOKEN_KEYWORD_ELSE))
	{
		expect_with_skip(parser, TOKEN_KEYWORD_ELSE, "else");
		else_body = parse_block(parser)->block;
	}
	return stmt_new(STMT_IF, if_stmt_new(if_cond,
		if_body, elifs, else_body));
}

ElseIf* parse_elif_stmt(Parser* parser)
{
	Expr* elif_cond = NULL;
	Block* elif_body = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_ELIF, "elif");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	elif_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	elif_body = parse_block(parser)->block;
	return elif_stmt_new(elif_cond, elif_body);
}

Case* parse_case_stmt(Parser* parser)
{
	Expr* case_cond = NULL;
	Block* case_body = NULL;
	uint32_t is_conjucted = 0;

	expect_with_skip(parser, TOKEN_KEYWORD_CASE, "case");
	case_cond = parse_expr(parser);
	if (case_cond->kind != EXPR_IDNT &&
		case_cond->kind != EXPR_CONST)
		report_error2("Constant or identifier expression expected in case condition.",
			get_expr_area(case_cond));
	expect_with_skip(parser, TOKEN_COLON, ":");
	// if we face the case keyword next after ':', it means that this case is 
	// conjucted, rather parse block
	matcht(parser, TOKEN_KEYWORD_CASE) ?
		is_conjucted = 1 : parse_case_block(parser);
	return case_stmt_new(case_cond, case_body, is_conjucted);
}

Block* parse_case_block(Parser* parser)
{
	Stmt** stmts = NULL;

	while (!matcht(parser, TOKEN_KEYWORD_BREAK))
		sbuffer_add(stmts, parse_stmt(parser));
	expect_with_skip(parser, TOKEN_KEYWORD_BREAK, "break");
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	sbuffer_add(stmts, stmt_new(STMT_JUMP, jump_stmt_new(JUMP_BREAK, NULL)));
	return block_new(stmts);
}

Stmt* parse_switch_stmt(Parser* parser)
{
	Expr* switch_cond = NULL;
	Case** switch_cases = NULL;
	Block* switch_default = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_SWITCH, "switch");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	switch_cond = parse_expr(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		if (!matcht(parser, TOKEN_KEYWORD_DEFAULT))
			sbuffer_add(switch_cases, parse_case_stmt(parser));
		else
		{
			expect_with_skip(parser, TOKEN_KEYWORD_DEFAULT, "default");
			expect_with_skip(parser, TOKEN_COLON, ":");
			switch_default = parse_case_block(parser);
			break;
		}
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_SWITCH, 
		switch_stmt_new(switch_cond, switch_cases, switch_default));
}

void clear_imported_modules()
{
	sbuffer_free(imported_modules);
	imported_modules = NULL;
}

char is_module_imported(const char* module)
{
	for (int i = 0; i < sbuffer_len(imported_modules); i++)
		if (strcmp(module, imported_modules[i]) == 0)
			return 1;
	return 0;
}

AstRoot* parse_module(const char* module_path)
{
	Lexer* lexer = lexer_new(module_path, FROM_FILE);
	Parser* parser = parser_new(module_path, lex(lexer));
	AstRoot* module = parse(parser);
	lexer_free(lexer);
	parser_free(parser);
	return module;
}

char is_stmt_for_import(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_TYPE_DECL:
	case STMT_VAR_DECL:
	case STMT_FUNC_DECL:
		return 1;
	}
	return 0;
}

char* get_stmt_for_import_name(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_TYPE_DECL:
		switch (stmt->type_decl->kind)
		{
		case TYPE_DECL_ENUM:
			return stmt->type_decl->enum_decl->enum_name;
		case TYPE_DECL_UNION:
			return stmt->type_decl->union_decl->union_name;
		case TYPE_DECL_STRUCT:
			return stmt->type_decl->struct_decl->struct_name;
		}
		break;
	case STMT_VAR_DECL:
		return stmt->var_decl->type_var->var;
	case STMT_FUNC_DECL:
		return stmt->func_decl->func_name;
	}
	return NULL;
}

Stmt* parse_import_stmt(Parser* parser)
{
	char* module_path = NULL;
	AstRoot* imported_module = NULL;
	AstRoot* module = cnew_s(AstRoot, module, 1);
	
	expect_with_skip(parser, TOKEN_KEYWORD_IMPORT, "import");
	do
	{
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
		switch (get_curr_token(parser)->type)
		{
		case TOKEN_PATH_DESC:
			module_path = parse_import_path_desc(parser);
			imported_module = parse_module(module_path);
			for (int i = 0; i < sbuffer_len(imported_module->stmts); i++)
			{
				// todo: issue with statements, 
				// that will be imported in any case (is_stmt_for_import function return 0)
				if (!is_stmt_for_import(imported_module->stmts[i]))
					sbuffer_add(module->stmts, imported_module->stmts[i]);
				else
				{
					char* module_path_with_member = frmt("%s?%s",
						module_path, get_stmt_for_import_name(imported_module->stmts[i]));
					if (is_module_imported(module_path_with_member))
					{
						free(module_path_with_member);
						stmt_free(imported_module->stmts[i]);
					}
					else
					{
						sbuffer_add(module->stmts, imported_module->stmts[i]);
						sbuffer_add(imported_modules, module_path_with_member);
					}
				}
			}
			free(imported_module);
			break;
		default:
			report_error(frmt("Expected \'./\', \'../\', \'/\', relative or absolute path, but met: %s",
				token_type_tostr(get_curr_token(parser)->type)), get_curr_token(parser)->context);
		}
	} while (matcht(parser, TOKEN_COMMA));

	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_IMPORT, import_stmt_new(module));
}

char* parse_import_path_desc(Parser* parser)
{
#define assign_new_path(new_path) \
	path_dup = path;			  \
	path = new_path;			  \
	free(path_dup)

	char* filename = NULL;
	// path's duplicate, needed for free the previous assigned path, 
	// using assign_new_path macro
	char* path_dup = NULL;
	char* path = get_dir_parent(parser->file);

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_IDNT:
	continue_building_relative_path:
		filename = get_curr_token(parser)->svalue;
		expect_with_skip(parser, TOKEN_IDNT, "module name");
		assign_new_path(path_combine(path, filename));
		while (matcht(parser, TOKEN_SLASH))
		{
			expect_with_skip(parser, TOKEN_SLASH, "path separator");
			filename = get_curr_token(parser)->svalue;
			assign_new_path(path_combine(path, filename));
			expect_with_skip(parser, TOKEN_IDNT, "module name");
		}
		// adding extension to file
		assign_new_path(frmt("%s%s", path, ALMC_FILE_EXTENSION));
		break;
	case TOKEN_SLASH:
		get_next_token(parser);
		assign_new_path(get_root(parser->file));
		// if we met slash, so that should be the case of 
		// identifier building part
		goto continue_building_relative_path;
		break;
	case TOKEN_STRING:
		assign_new_path(get_curr_token(parser)->svalue);
		expect_with_skip(parser, TOKEN_STRING, "absolute path");
		break;
	case TOKEN_NAV_CURR_DIR:
	case TOKEN_NAV_PREV_DIR:
		while (matcht(parser, TOKEN_NAV_CURR_DIR) ||
			matcht(parser, TOKEN_NAV_PREV_DIR))
		{
			switch (get_curr_token(parser)->type)
			{
			case TOKEN_NAV_CURR_DIR:
				break;
			case TOKEN_NAV_PREV_DIR:
				assign_new_path(get_dir_parent(path));
				break;
			}
			get_next_token(parser);
		}
		// it means that if we found any of them: TOKEN_NAV_CURR_DIR | TOKEN_NAV_PREV_DIR
		// next token in import's path description should be identifier, so here we just jumping 
		// to part that describes case of identifier
		goto continue_building_relative_path;
		break;
	}

	if (strcmp(path, parser->file) == 0)
		report_error(frmt("Cannot import module: \'%s\', attempt to import module from itself.",
			path), get_curr_token(parser)->context);
	else if (file_exists(path))
		return path;
	else
		report_error(frmt("Cannot import module: \'%s\', no such module found.",
			path), get_curr_token(parser)->context);
#undef assign_new_path
}

Stmt* parse_from_import_stmt(Parser* parser)
{
	char* module_path = NULL;
	AstRoot* from_module = NULL;   // module that will represent extracted statements from module
	AstRoot* import_module = NULL; // whole module imported via import

	expect_with_skip(parser, TOKEN_KEYWORD_FROM, "from");
	module_path = parse_import_path_desc(parser);
	import_module = parse_module(module_path);
	expect_with_skip(parser, TOKEN_KEYWORD_IMPORT, "import");
	from_module = cnew_s(AstRoot, from_module, 1);
	do
	{
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
		// concatenating module_path with name of member (separated by '?')
		char* module_path_with_member = frmt("%s?%s", 
			module_path, get_curr_token(parser)->svalue);
		if (is_module_imported(module_path_with_member))
		{
			// if this member already imported, free this new string, and skip member name token
			free(module_path_with_member);
			expect_with_skip(parser, TOKEN_IDNT, "member name");
		}
		else
		{
			// otherwise add to imported modules, and get this member from module
			sbuffer_add(imported_modules, module_path_with_member);
			sbuffer_add(from_module->stmts, parse_from_import_member_stmt(parser, import_module));
		}
	} while (matcht(parser, TOKEN_COMMA));
		
	// finding and releasing all non-used in from-module members
	for (int i = 0; i < sbuffer_len(import_module->stmts); i++)
	{
		char in_use = 0;
		for (int j = 0; j < sbuffer_len(from_module->stmts); j++)
			if (from_module->stmts[j] == import_module->stmts[i])
				in_use = 1;
		if (!in_use)
			stmt_free(import_module->stmts[i]);
	}
	sbuffer_free(import_module->stmts);
	free(import_module);

	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_IMPORT, import_stmt_new(from_module));
}

Stmt* parse_from_import_member_stmt(Parser* parser, AstRoot* import_module)
{
	Stmt* wanted = NULL;
	Token* token = get_curr_token(parser);
	char* member_name = token->svalue;

	expect_with_skip(parser, TOKEN_IDNT, "member name");
	for (int i = 0; i < sbuffer_len(import_module->stmts); i++)
		if (is_stmt_for_import(import_module->stmts[i]))
			if (strcmp(get_stmt_for_import_name(import_module->stmts[i]), member_name) == 0)
				wanted = import_module->stmts[i];
	if (!wanted)
		report_error(frmt("Cannot import member \'%s\', no such member found in module.",
			member_name), token->context);
	return wanted;
}

Stmt* parse_jump_stmt(Parser* parser)
{
	JumpStmtKind type = -1;
	context_starts(parser, context);
	Expr* additional_expr = NULL;
	Idnt* goto_label = cnew_s(Idnt, goto_label, 1);

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_GOTO:
		type = JUMP_GOTO;
		expect_with_skip(parser, TOKEN_KEYWORD_GOTO, "goto");
		goto_label->svalue = get_curr_token(parser)->svalue;
		expect_with_skip(parser, TOKEN_IDNT, "goto label");
		additional_expr = expr_new(EXPR_IDNT, goto_label);
		break;
	case TOKEN_KEYWORD_BREAK:
		type = JUMP_BREAK;
		expect_with_skip(parser, TOKEN_KEYWORD_BREAK, "break");
		break;
	case TOKEN_KEYWORD_CONTINUE:
		type = JUMP_CONTINUE;
		expect_with_skip(parser, TOKEN_KEYWORD_CONTINUE, "continue");
		break;
	case TOKEN_KEYWORD_RETURN:
		type = JUMP_RETURN;
		expect_with_skip(parser, TOKEN_KEYWORD_RETURN, "return");
		if (!matcht(parser, TOKEN_SEMICOLON))
			additional_expr = parse_expr(parser);
		break;
	default:
		report_error(frmt("Expected keyword (return, break or continue), but met: %s",
			token_type_tostr(get_curr_token(parser)->type)), get_curr_token(parser)->context);
	}
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	Stmt* stmt = stmt_new(STMT_JUMP, jump_stmt_new(type, 
		additional_expr));
	context_ends(parser, context, stmt->jump_stmt);
	return stmt;
}

TypeVar* parse_type_var(Parser* parser)
{
	Type* type = NULL;
	const char* var = 
		get_curr_token(parser)->svalue;
	TypeVar* type_var = NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_IDNT, "variable's name");
	expect_with_skip(parser, TOKEN_COLON, ":");
	type = parse_type_name(parser);
	type_var = type_var_new(type, var);
	context_ends(parser, context, type_var);
	return type_var;
}