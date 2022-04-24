#include "parser.h"

//todo: add error recovering.

#define matcht(parser, t) (get_curr_token(parser)->type == (t))
#define expect_with_skip(parser, type, str) expect(parser, type, str), get_next_token(parser)
#define token_index_fits(parser) (parser->token_index >= 0 && parser->token_index < sbuffer_len(parser->tokens))

#define context_starts(parser, var_name) \
	SrcContext* var_name = get_curr_token(parser)->context

#define context_ends(parser, starts, node) \
	if (node == NULL)					   \
		printerr("context_ends: node");    \
	unget_curr_token(parser);              \
	node->area = src_area_new(starts,      \
		get_curr_token(parser)->context);  \
	get_next_token(parser)					

// c# analog of ?? operator
#define NULL_COLEASING(expr1, expr2) \
	((expr1) != NULL ? expr1 : expr2)

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
	Parser* p = new(Parser);
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
		clear_imported_modules();
		free(parser);
	}
}

void parser_free_safe(Parser* parser)
{
	// needed for parse_module function, because origin parser_free frees the contexts
	if (parser)
	{
		for (int i = 0; i < sbuffer_len(parser->tokens); i++)
			free(parser->tokens[i]);
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
	AstRoot* ast = new(AstRoot);
	ast->stmts = NULL;
	while (!matcht(parser, TOKEN_EOF))
		sbuffer_add(ast->stmts, parse_stmt(parser));
	return ast;
}

Expr* parse_expr(Parser* parser)
{
	return parse_comma_expr(parser);
}

Type* parse_type_declarator(Parser* parser, Type* type)
{
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_ASTERISK:
		if (type->kind == TYPE_ARRAY)
			report_error("Cannot create type with this sequence of type declarators.",
				get_curr_token(parser)->context);
		get_next_token(parser);
		type = pointer_type_new(type);
		return parse_type_declarator(parser, type);
	case TOKEN_OP_BRACKET:
		get_next_token(parser);
		type = array_type_new(type, parse_expr(parser));
		expect_with_skip(parser, TOKEN_CL_BRACKET, "]");
		return parse_type_declarator(parser, type);
	default:
		return type;
	}
}

Type* parse_type(Parser* parser)
{
	#define ASSIGN_TYPE(primitive) \
		type = &primitive; break

	Type* type = NULL;
	Token* token = get_curr_token(parser);
	context_starts(parser, context);

	switch (token->type)
	{
	case TOKEN_KEYWORD_VOID:
		ASSIGN_TYPE(void_type);
	case TOKEN_KEYWORD_CHAR:   
		ASSIGN_TYPE(char_type);
	case TOKEN_KEYWORD_INT8: 
		ASSIGN_TYPE(i8_type);
	case TOKEN_KEYWORD_INT16:	  
		ASSIGN_TYPE(i16_type);
	case TOKEN_KEYWORD_INT32:	  
		ASSIGN_TYPE(i32_type);
	case TOKEN_KEYWORD_INT64:
		ASSIGN_TYPE(i64_type);
	case TOKEN_KEYWORD_UINT8:
		ASSIGN_TYPE(u8_type);
	case TOKEN_KEYWORD_UINT16:
		ASSIGN_TYPE(u16_type);
	case TOKEN_KEYWORD_UINT32:
		ASSIGN_TYPE(u32_type);
	case TOKEN_KEYWORD_UINT64:
		ASSIGN_TYPE(u64_type);
	case TOKEN_KEYWORD_FLOAT32:
		ASSIGN_TYPE(f32_type);
	case TOKEN_KEYWORD_FLOAT64:
		ASSIGN_TYPE(f64_type);
	//case TOKEN_KEYWORD_STRING:
	case TOKEN_IDNT:
		type = type_new(token->lexeme);
		break;
	default:
		report_error(frmt("Type expected, but met: %s",
			token_type_tostr(token->type)), token->context);
	}

	get_next_token(parser);
	type = parse_type_declarator(parser, type);
	context_ends(parser, context, type);
	return type;

	#undef ASSIGN_TYPE
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
		get_curr_token(parser)->lexeme;

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

Expr* parse_idnt_ambiguity(Parser* parser)
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
			idnt_new(get_curr_token(parser)->lexeme,
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
			const_new(CONST_INT, token->lexeme, token->context));
		break;
	case TOKEN_UINT_CONST:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_UINT, token->lexeme, token->context));
		break;
	case TOKEN_FLOAT_CONST:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_FLOAT, token->lexeme, token->context));
		break;
	case TOKEN_IDNT:
		expr = parse_idnt_ambiguity(parser);
		if (expr->kind == EXPR_FUNC_CALL)
			return expr;
		break;
	case TOKEN_STRING:
		expr = expr_new(EXPR_STRING,
			str_new(token->lexeme, token->context));
		break;
	case TOKEN_CHARACTER:
		expr = expr_new(EXPR_CONST,
			const_new(CONST_CHAR, frmt("%u", 
				(uint32_t)token->lexeme[0]), token->context));
		break;
	case TOKEN_OP_PAREN:
		return parse_paren_expr(parser);
	//case TOKEN_OP_BRACE:
	//	return parse_initializer_expr(parser);
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
				rexpr, expr_new(EXPR_IDNT, idnt_new(token->lexeme, token->context))));
		context_ends(parser, context, member_accessor->binary_expr);
		return member_accessor;
	default:
		report_error(frmt("Expected member accessor, but met: %s",
			token_type_tostr(token->type)), token->context);
	}
}

Expr* parse_postfix_expr(Parser* parser)
{
	Expr* postfix_expr = NULL;
	Expr* primary_expr = parse_primary_expr(parser);

	while (matcht(parser, TOKEN_DOT)
		|| matcht(parser, TOKEN_ARROW)
		|| matcht(parser, TOKEN_OP_BRACKET))
	{
		Expr* rexpr = postfix_expr ?
			postfix_expr : primary_expr;

		switch (get_curr_token(parser)->type)
		{
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
	return NULL_COLEASING(postfix_expr, primary_expr);
}

Expr* parse_unary_expr(Parser* parser)
{
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
	case TOKEN_KEYWORD_LENGTHOF:
		return parse_lengthof_expr(parser);
	default:
		return parse_postfix_expr(parser);
	}
}

Expr* parse_cast_expr(Parser* parser)
{
	Type* type = NULL;
	Expr* expr = NULL;

	if (!matcht(parser, TOKEN_KEYWORD_CAST))
		return parse_unary_expr(parser);
	else
	{
		expect_with_skip(parser, TOKEN_KEYWORD_CAST, "cast");
		expect_with_skip(parser, TOKEN_OP_PAREN, "(");
		type = parse_type(parser);
		expect_with_skip(parser, TOKEN_CL_PAREN, ")");
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
	<sizeof-expression> ::= sizeof ( <type-name> )
	*/

	Type* type = NULL;
	Expr* expr = NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_KEYWORD_SIZEOF, "sizeof");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	type = parse_type(parser);
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	expr = expr_new(EXPR_UNARY_EXPR,
		unary_expr_new(UNARY_SIZEOF, NULL));
	expr->unary_expr->cast_type = type;
	context_ends(parser, context, expr->unary_expr);
	return expr;
}

Expr* parse_lengthof_expr(Parser* parser)
{
	Type* type = NULL;
	Expr* expr = NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_KEYWORD_LENGTHOF, "lengthof");
	expr = expr_new(EXPR_UNARY_EXPR,
		unary_expr_new(UNARY_LENGTHOF, parse_unary_expr(parser)));
	context_ends(parser, context, expr->unary_expr);
	return expr;
}

Expr* parse_mul_arith_expr(Parser* parser)
{
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
			binary_expr_new(type, NULL_COLEASING(mul_expr, unary_expr),
				parse_cast_expr(parser)));
		context_ends(parser, context, mul_expr->binary_expr);
	}
	return NULL_COLEASING(mul_expr, unary_expr);
}

Expr* parse_add_arith_expr(Parser* parser)
{
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
			binary_expr_new(type, NULL_COLEASING(add_expr, mul_expr),
				parse_mul_arith_expr(parser)));
		context_ends(parser, context, add_expr->binary_expr);
	}
	return NULL_COLEASING(add_expr, mul_expr);
}

Expr* parse_sft_expr(Parser* parser)
{
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
			binary_expr_new(type, NULL_COLEASING(sft_expr, add_expr),
				parse_add_arith_expr(parser)));
		context_ends(parser, context, sft_expr->binary_expr);
	}
	return NULL_COLEASING(sft_expr, add_expr);
}

Expr* parse_rel_expr(Parser* parser)
{
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
			binary_expr_new(type, NULL_COLEASING(rel_expr, sft_expr),
				parse_sft_expr(parser)));
		context_ends(parser, context, rel_expr->binary_expr);
	}
	return NULL_COLEASING(rel_expr, sft_expr);
}

Expr* parse_equ_expr(Parser* parser)
{
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
			binary_expr_new(type, NULL_COLEASING(equ_expr, rel_expr),
				parse_rel_expr(parser)));
		context_ends(parser, context, equ_expr->binary_expr);
	}
	return NULL_COLEASING(equ_expr, rel_expr);
}

Expr* parse_and_bw_expr(Parser* parser)
{
	Expr* and_expr = NULL;
	context_starts(parser, context);
	Expr* equ_expr = parse_equ_expr(parser);

	while (matcht(parser, TOKEN_AMPERSAND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_AND, NULL_COLEASING(and_expr, equ_expr),
				parse_equ_expr(parser)));
		context_ends(parser, context, and_expr->binary_expr);
	}
	return NULL_COLEASING(and_expr, equ_expr);
}

Expr* parse_xor_bw_expr(Parser* parser)
{
	Expr* xor_expr = NULL;
	context_starts(parser, context);
	Expr* and_expr = parse_and_bw_expr(parser);

	while (matcht(parser, TOKEN_CARET))
	{
		get_next_token(parser);
		xor_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_XOR, NULL_COLEASING(xor_expr, and_expr),
				parse_and_bw_expr(parser)));
		context_ends(parser, context, xor_expr->binary_expr);
	}
	return NULL_COLEASING(xor_expr, and_expr);
}

Expr* parse_or_bw_expr(Parser* parser)
{
	Expr* or_expr = NULL;
	context_starts(parser, context);
	Expr* xor_expr = parse_xor_bw_expr(parser);

	while (matcht(parser, TOKEN_BAR))
	{
		get_next_token(parser);
		or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_BW_OR, NULL_COLEASING(or_expr, xor_expr),
				parse_xor_bw_expr(parser)));
		context_ends(parser, context, or_expr->binary_expr);
	}
	return NULL_COLEASING(or_expr, xor_expr);
}

Expr* parse_and_lg_expr(Parser* parser)
{
	Expr* and_expr = NULL;
	context_starts(parser, context);
	Expr* or_expr = parse_or_bw_expr(parser);

	while (matcht(parser, TOKEN_LG_AND))
	{
		get_next_token(parser);
		and_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_AND, NULL_COLEASING(and_expr, or_expr),
				parse_or_bw_expr(parser)));
		context_ends(parser, context, and_expr->binary_expr);
	}
	return NULL_COLEASING(and_expr, or_expr);
}

Expr* parse_or_lg_expr(Parser* parser)
{
	Expr* lg_or_expr = NULL;
	context_starts(parser, context);
	Expr* and_expr = parse_and_lg_expr(parser);

	while (matcht(parser, TOKEN_LG_OR))
	{
		get_next_token(parser);
		lg_or_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_LG_OR, NULL_COLEASING(lg_or_expr, and_expr),
				parse_and_lg_expr(parser)));
		context_ends(parser, context, lg_or_expr->binary_expr);
	}
	return NULL_COLEASING(lg_or_expr, and_expr);
}

Expr* parse_conditional_expr(Parser* parser)
{
	Expr* cond_expr = NULL;
	context_starts(parser, context);
	Expr* or_expr = parse_or_lg_expr(parser);

	if (matcht(parser, TOKEN_QUESTION))
	{
		get_next_token(parser);
		Expr* lexpr = parse_expr(parser);
		expect_with_skip(parser, TOKEN_COLON, ":");
		cond_expr = expr_new(EXPR_TERNARY_EXPR,
			ternary_expr_new(or_expr, lexpr, parse_conditional_expr(parser)));
		context_ends(parser, context, cond_expr->ternary_expr);
	}
	return NULL_COLEASING(cond_expr, or_expr);
}

Expr* parse_constant_expr(Parser* parser)
{
	return parse_conditional_expr(parser);
}

Expr* parse_assignment_expr(Parser* parser)
{
	Expr* assign_expr = NULL;
	context_starts(parser, context);
	Expr* cond_expr = parse_conditional_expr(parser);

	if (matcht(parser, TOKEN_ASSIGN)
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
		case TOKEN_BW_OR_ASSIGN:
			type = BINARY_BW_OR_ASSIGN; break;
		case TOKEN_BW_AND_ASSIGN:
			type = BINARY_BW_AND_ASSIGN; break;
		case TOKEN_BW_XOR_ASSIGN:
			type = BINARY_BW_XOR_ASSIGN; break;
		}
		get_next_token(parser);
		//Expr* init;
		// todo: rewrite this part with initializer
		//if (matcht(parser, TOKEN_OP_BRACE))
		//	return expr_new(EXPR_BINARY_EXPR,
		//		binary_expr_new(type, (assign_expr) ? assign_expr : cond_expr, parse_initializer_expr(parser)));
		assign_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(type, cond_expr, parse_assignment_expr(parser)));
		context_ends(parser, context, assign_expr->binary_expr);
	}
	return NULL_COLEASING(assign_expr, cond_expr);
}

Expr* parse_comma_expr(Parser* parser)
{
	Expr* comma_expr = NULL;
	context_starts(parser, context);
	Expr* assign_expr = parse_assignment_expr(parser);

	if (matcht(parser, TOKEN_COMMA))
	{
		get_next_token(parser);
		comma_expr = expr_new(EXPR_BINARY_EXPR,
			binary_expr_new(BINARY_COMMA, assign_expr,
				parse_comma_expr(parser)));
		context_ends(parser, context, comma_expr->binary_expr);
	}

	return NULL_COLEASING(comma_expr, assign_expr);
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
	case TOKEN_KEYWORD_VAR:
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
	char* member_name = "";
	Expr* member_value = NULL;
	SrcContext* member_context = NULL;

	char* name = "";
	EnumMember** members = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_ENUM, "enum");
	// enum can be no-name
	if (matcht(parser, TOKEN_IDNT))
		name = get_curr_token(parser)->lexeme,
			expect_with_skip(parser, TOKEN_IDNT, "enum name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		member_name = get_curr_token(parser)->lexeme;
		member_context = get_curr_token(parser)->context;
		expect_with_skip(parser, TOKEN_IDNT, "enum's member's name");

		// if we assign value explicitly, then just set value from parse_constant_expr
		if (matcht(parser, TOKEN_ASSIGN))
			get_next_token(parser),
				member_value = parse_constant_expr(parser);
		else
		{
			// either do following:
			//	if there are no any assigned value yet, create expr->const->0
			//	otherwise create expr based on this value, and add 1

			if (!member_value)
				member_value = expr_new(EXPR_CONST, 
					const_new(CONST_INT, "0", NULL));
			else
			{
				if (member_value->kind == EXPR_CONST)
					member_value = expr_new(EXPR_CONST, const_new(
						/*free this new created with frmt string?*/
						CONST_INT, frmt("%d", member_value->cnst->ivalue + 1), NULL));
				else
					member_value = expr_new(EXPR_BINARY_EXPR, binary_expr_new(BINARY_ADD, member_value,
						expr_new(EXPR_CONST, const_new(CONST_INT, "1", NULL))));
			}
		}
		sbuffer_add(members, enum_member_new(member_name, 
			member_value, member_context));
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL, type_decl_new(TYPE_DECL_ENUM,
		enum_decl_new(members, name)));
}

Stmt* parse_union_decl_stmt(Parser* parser)
{
	char* name = "";
	Member** members = NULL;
	TypeVar* typevar = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_UNION, "union");
	name = get_curr_token(parser)->lexeme;
	expect_with_skip(parser, TOKEN_IDNT, "union name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		typevar = parse_type_var(parser);
		sbuffer_add(members, member_new(typevar->var,
			typevar->type, typevar->area));
		expect_with_skip(parser, TOKEN_SEMICOLON, ";");
		free(typevar);
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL, type_decl_new(TYPE_DECL_UNION, 
		union_decl_new(members, name)));
}

Stmt* parse_struct_decl_stmt(Parser* parser)
{
	char* name = "";
	TypeVar* typevar = NULL;
	Member** members = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_STRUCT, "struct");
	name = get_curr_token(parser)->lexeme;
	expect_with_skip(parser, TOKEN_IDNT, "struct name");
	expect_with_skip(parser, TOKEN_OP_BRACE, "{");
	while (!matcht(parser, TOKEN_CL_BRACE))
	{
		typevar = parse_type_var(parser);
		sbuffer_add(members, member_new(typevar->var,
			typevar->type, typevar->area));
		expect_with_skip(parser, TOKEN_SEMICOLON, ";");
		free(typevar);
	}
	expect_with_skip(parser, TOKEN_CL_BRACE, "}");
	return stmt_new(STMT_TYPE_DECL, type_decl_new(TYPE_DECL_STRUCT,
		struct_decl_new(members, name)));
}

Stmt* parse_block(Parser* parser)
{
	Stmt** stmts = NULL;
	bool is_simple = matcht(parser, TOKEN_OP_BRACE);

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

	if (matcht(parser, TOKEN_KEYWORD_VAR))
		return parse_auto_var_decl_stmt(parser);
	expect_with_skip(parser, TOKEN_KEYWORD_LET, "let");
	type_var = parse_type_var(parser);
	if (matcht(parser, TOKEN_ASSIGN))
	{
		get_next_token(parser);
		var_init = matcht(parser, TOKEN_OP_BRACE) ? 
			parse_initializer_expr(parser) : parse_expr(parser);
	}
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_VAR_DECL,
		var_decl_new(false, type_var, var_init));
}

Stmt* parse_auto_var_decl_stmt(Parser* parser)
{
	Expr* var_init = NULL;
	TypeVar* type_var = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_VAR, "var");
	type_var = parse_auto_type_var(parser);
	expect_with_skip(parser, TOKEN_ASSIGN, "=");
	var_init = matcht(parser, TOKEN_OP_BRACE) ?
		parse_initializer_expr(parser) : parse_expr(parser);
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
	return stmt_new(STMT_VAR_DECL,
		var_decl_new(true, type_var, var_init));
}

CallConv* calling_convention_new()
{
	CallConv* convention = cnew(CallConv, 1);
	convention->repr = "c";
	convention->kind = CALL_CONV_CDECL;
	return convention;
}

CallConv* parse_func_calling_convention(Parser* parser)
{
	// cdecl will be set by default
	CallConv* convention = 
		calling_convention_new();

	char* token_str = get_curr_token(parser)->lexeme;

	if (strcmp(token_str, "cdecl") == 0)
		get_next_token(parser);
	else if (strcmp(token_str, "stdcall") == 0)
	{
		get_next_token(parser);
		convention->kind = CALL_CONV_STDCALL,
			convention->repr = token_str;
	}
	return convention;
}

ExternalFuncSpec* parse_func_proto_spec(Parser* parser)
{
	ExternalFuncSpec* proto = cnew(ExternalFuncSpec, 1);
	expect_with_skip(parser, TOKEN_KEYWORD_FROM, "from keyword");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	proto->lib = get_curr_token(parser)->lexeme;
	expect_with_skip(parser, TOKEN_STRING, "lib name");
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	return proto;
}

FuncSpec* parse_func_specifiers(Parser* parser)
{
	FuncSpec* spec = cnew(FuncSpec, 1);
	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_FROM:
		spec->proto = 
			parse_func_proto_spec(parser);
		spec->is_external = true;
		break;
	case TOKEN_KEYWORD_ENTRY:
		get_next_token(parser);
		spec->is_entry = true;
		break;
	}
	return spec;
}

Stmt* parse_func_decl_stmt(Parser* parser)
{
	Idnt* name = NULL;
	Type* type = NULL;
	Block* body = NULL;
	TypeVar** params = NULL;
	FuncSpec* spec = NULL;
	CallConv* call_conv = NULL;

	expect_with_skip(parser, TOKEN_KEYWORD_FUNC, "fnc");
	call_conv = parse_func_calling_convention(parser);
	spec = parse_func_specifiers(parser);
	name = idnt_new(get_curr_token(parser)->lexeme,
		get_curr_token(parser)->context);
	expect_with_skip(parser, TOKEN_IDNT, "func name");
	expect_with_skip(parser, TOKEN_OP_PAREN, "(");
	while (!matcht(parser, TOKEN_CL_PAREN))
	{
		//todo: may be do that more clearly
		if (matcht(parser, TOKEN_TRIPLE_DOT))
		{
			spec->is_vararg = true;
			get_next_token(parser);
			break;
		}
		sbuffer_add(params, parse_type_var(parser));
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
	}
	expect_with_skip(parser, TOKEN_CL_PAREN, ")");
	expect_with_skip(parser, TOKEN_COLON, ":");
	type = parse_type(parser);
	
	// checking if function does not have any block
	if (matcht(parser, TOKEN_SEMICOLON))
		get_next_token(parser);
	else
	{
		if (!matcht(parser, TOKEN_OP_BRACE))
			expect_with_skip(parser, TOKEN_OP_BRACE, "{");
		body = parse_block(parser)->block;
	}
	return stmt_new(STMT_FUNC_DECL, func_decl_new(name, 
		params, type, body, spec, call_conv));
}

Stmt* parse_label_decl_stmt(Parser* parser)
{
	Idnt* label = cnew(Idnt, 1);

	expect_with_skip(parser, TOKEN_KEYWORD_LABEL, "label");
	label->svalue = get_curr_token(parser)->lexeme;
	label->context = get_curr_token(parser)->context;
	expect_with_skip(parser, TOKEN_IDNT, "label name");
	expect_with_skip(parser, TOKEN_SEMICOLON, ";");
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
	parser_free_safe(parser);
	return module;
}

bool is_stmt_for_import(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_VAR_DECL:
	case STMT_TYPE_DECL:
	case STMT_FUNC_DECL:
		return true;
	}
	return false;
}

char* get_stmt_for_import_name(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_TYPE_DECL:
		switch (stmt->type_decl->kind)
		{
		case TYPE_DECL_ENUM:
			return stmt->type_decl->enum_decl->name;
		case TYPE_DECL_UNION:
			return stmt->type_decl->union_decl->name;
		case TYPE_DECL_STRUCT:
			return stmt->type_decl->struct_decl->name;
		}
		break;
	case STMT_VAR_DECL:
		return stmt->var_decl->type_var->var;
	case STMT_FUNC_DECL:
		return stmt->func_decl->name->svalue;
	}
	return NULL;
}

Stmt* parse_import_stmt(Parser* parser)
{
	char* module_path = NULL;
	AstRoot* imported_module = NULL;
	AstRoot* module = cnew(AstRoot, 1);
	
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
		filename = get_curr_token(parser)->lexeme;
		expect_with_skip(parser, TOKEN_IDNT, "module name");
		assign_new_path(path_combine(path, filename));
		while (matcht(parser, TOKEN_SLASH))
		{
			expect_with_skip(parser, TOKEN_SLASH, "path separator");
			filename = get_curr_token(parser)->lexeme;
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
		assign_new_path(get_curr_token(parser)->lexeme);
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
	from_module = cnew(AstRoot, 1);
	do
	{
		if (matcht(parser, TOKEN_COMMA))
			expect_with_skip(parser, TOKEN_COMMA, ",");
		// concatenating module_path with name of member (separated by '?')
		char* module_path_with_member = frmt("%s?%s", 
			module_path, get_curr_token(parser)->lexeme);
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
	char* member_name = token->lexeme;

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
	Idnt* goto_label = cnew(Idnt, 1);

	switch (get_curr_token(parser)->type)
	{
	case TOKEN_KEYWORD_GOTO:
		type = JUMP_GOTO;
		expect_with_skip(parser, TOKEN_KEYWORD_GOTO, "goto");
		goto_label->svalue = get_curr_token(parser)->lexeme;
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
		get_curr_token(parser)->lexeme;
	TypeVar* type_var = NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_IDNT, "variable's name");
	expect_with_skip(parser, TOKEN_COLON, ":");
	type = parse_type(parser);
	type_var = type_var_new(type, var);
	context_ends(parser, context, type_var);
	return type_var;
}

TypeVar* parse_auto_type_var(Parser* parser)
{
	const char* var = get_curr_token(parser)->lexeme;
	TypeVar* type_var = NULL;

	context_starts(parser, context);
	expect_with_skip(parser, TOKEN_IDNT, "variable's name");
	expect_with_skip(parser, TOKEN_COLON, ":");
	type_var = type_var_new(&unknown_type, var);
	context_ends(parser, context, type_var);
	return type_var;
}