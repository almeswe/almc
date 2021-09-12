#include "ast.h"


Expr* expr_new(ExprType type, void* expr_value_ptr)
{
	#define expr_set_value(type, field) e->field = (type*)expr_value_ptr
	Expr* e = new_s(Expr, e);
	switch (e->type = type)
	{
	case EXPR_IDNT:
		expr_set_value(Idnt, idnt);
		break;
	case EXPR_CONST:
		expr_set_value(Const, cnst);
		break;
	case EXPR_UNARY_EXPR:
		expr_set_value(UnaryExpr, unary_expr);
		break;
	case EXPR_BINARY_EXPR:
		expr_set_value(BinaryExpr, binary_expr);
		break;
	}
	return e;
}

Idnt* idnt_new(const char* idnt, SrcContext* context)
{
	Idnt* i = new_s(Idnt, i);
	i->svalue = idnt;
	i->context = context;
	return i;
}

Const* const_new(ConstType type, double value, SrcContext* context)
{
	Const* c = new_s(Const, c);
	c->context = context;
	switch (c->type = type)
	{
	case CONST_INT:
		c->ivalue = (int64_t)value;
		break;
	case CONST_UINT:
		c->uvalue = (uint64_t)value;
		break;
	case CONST_FLOAT:
		c->fvalue = value;
		break;
	}
	return c;
}

UnaryExpr* unary_expr_new(UnaryExprType type, Expr* expr)
{
	UnaryExpr* ue = new_s(UnaryExpr, ue);
	ue->type = type;
	ue->expr = expr;
	return ue;
}

BinaryExpr* binary_expr_new(BinaryExprType type, Expr* lexpr, Expr* rexpr)
{
	BinaryExpr* be = new_s(BinaryExpr, be);
	be->type = type;
	be->lexpr = lexpr;
	be->rexpr = rexpr;
	return be;
}

void print_ast(AstRoot* ast)
{
	printf("ast-root\n");
	for (int i = 0; i < sbuffer_len(ast->exprs); i++)
		print_expr(ast->exprs[i], "");
}

void print_expr(Expr* expr, const char* indent)
{
	// |——
	char* new_indent = frmt("%s   ", indent);
	switch (expr->type)
	{
	case EXPR_IDNT:
		print_idnt(expr->idnt, new_indent);
		break;
	case EXPR_CONST:
		print_const(expr->cnst, new_indent);
		break;
	case EXPR_UNARY_EXPR:
		print_unary_expr(expr->unary_expr, new_indent);
		break;
	case EXPR_BINARY_EXPR:
		print_binary_expr(expr->binary_expr, new_indent);
		break;
	}
}

void print_idnt(Idnt* idnt, const char* indent)
{
	printf(BOLDWHITE);
	printf("%sidnt: %s\n", indent, idnt->svalue);
	printf(RESET);
}

void print_const(Const* cnst, const char* indent)
{
	printf(BOLDWHITE);
	switch (cnst->type)
	{
	case CONST_INT:
		printf("%sint-const: %lld\n", indent, cnst->ivalue);
		break;
	case CONST_UINT:
		printf("%suint-const: %I64u\n", indent, cnst->uvalue);
		break;
	case CONST_FLOAT:
		printf("%sfloat-const: %f\n", indent, cnst->fvalue);
		break;
	}
	printf(RESET);
}
void print_unary_expr(UnaryExpr* expr, const char* indent)
{
	printf(YELLOW);
	const char* unary_expr_type_str[] = {
		"unary-plus: +",
		"unary-minus: -",
		"unary-lg-not: !",
		"unary-bw-not: ~",

		"unary-cast:",
		"unary-sizeof: sizeof",
	};
	if (expr->type == UNARY_CAST)
		printf("%s%s (%s)\n", indent, unary_expr_type_str[expr->type], expr->cast_type);
	else
		printf("%s%s\n", indent, unary_expr_type_str[expr->type]);
	printf(RESET);
	print_expr(expr->expr, indent);
}

void print_binary_expr(BinaryExpr* expr, const char* indent)
{
	printf(BOLDYELLOW);
	const char* binary_expr_type_str[] = {
		"binary-add: +",
		"binary-sub: -",
		"binary-div: /",
		"binary-mod: %",
		"binary-mult: *",

		"binary-lg-or: ||",
		"binary-lg-and: &&",
		"binary-lg-eq: ==",
		"binary-lg-neq: !=",

		"binary-bw-or: |",
		"binary-bw-and: &",
		"binary-bw-xor: ^",
	};
	printf("%s%s\n", indent, binary_expr_type_str[expr->type]);
	printf(RESET);
	print_expr(expr->lexpr, indent);
	print_expr(expr->rexpr, indent);
}