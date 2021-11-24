#include "ast-evaluator.h"

int64_t evaluate_const_itype(Const* cnst)
{
	switch (cnst->kind)
	{
	case CONST_INT:
	case CONST_UINT:
		return cnst->ivalue;
	}
	return 1;
}

int64_t evaluate_ternary_expr_itype(TernaryExpr* ternary_expr)
{
	return evaluate_expr_itype(ternary_expr->cond) ?
		evaluate_expr_itype(ternary_expr->lexpr) : evaluate_expr_itype(ternary_expr->rexpr);
}

int64_t evaluate_binary_expr_itype(BinaryExpr* binary_expr)
{
	switch (binary_expr->kind)
	{
	case BINARY_ADD:
		return (evaluate_expr_itype(binary_expr->lexpr)
			+ evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_SUB:
		return (evaluate_expr_itype(binary_expr->lexpr)
			- evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_MULT:
		return (evaluate_expr_itype(binary_expr->lexpr)
			* evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_DIV:
		return (evaluate_expr_itype(binary_expr->lexpr)
			/ evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_MOD:
		return (evaluate_expr_itype(binary_expr->lexpr)
			% evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_BW_AND:
		return (evaluate_expr_itype(binary_expr->lexpr)
			& evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_BW_OR:
		return (evaluate_expr_itype(binary_expr->lexpr)
			| evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_BW_XOR:
		return (evaluate_expr_itype(binary_expr->lexpr)
			^ evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LSHIFT:
		return (evaluate_expr_itype(binary_expr->lexpr)
			<< evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_RSHIFT:
		return (evaluate_expr_itype(binary_expr->lexpr)
			>> evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LG_AND:
		return (evaluate_expr_itype(binary_expr->lexpr)
			&& evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LG_OR:
		return (evaluate_expr_itype(binary_expr->lexpr)
			|| evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LG_EQ:
		return (evaluate_expr_itype(binary_expr->lexpr)
			== evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LG_NEQ:
		return (evaluate_expr_itype(binary_expr->lexpr)
			!= evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LESS_THAN:
		return (evaluate_expr_itype(binary_expr->lexpr)
			< evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_GREATER_THAN:
		return (evaluate_expr_itype(binary_expr->lexpr)
			> evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_LESS_EQ_THAN:
		return (evaluate_expr_itype(binary_expr->lexpr)
			<= evaluate_expr_itype(binary_expr->rexpr));
	case BINARY_GREATER_EQ_THAN:
		return (evaluate_expr_itype(binary_expr->lexpr)
			>= evaluate_expr_itype(binary_expr->rexpr));
	}
	return 1;
}

int64_t evaluate_unary_expr_itype(UnaryExpr* unary_expr)
{
	int64_t sizeof_value;
	switch (unary_expr->kind)
	{
	case UNARY_PLUS:
		return evaluate_expr_itype(unary_expr->expr);
	case UNARY_MINUS:
		return -evaluate_expr_itype(unary_expr->expr);
	case UNARY_BW_NOT:
		return ~evaluate_expr_itype(unary_expr->expr);
	case UNARY_LG_NOT:
		return !evaluate_expr_itype(unary_expr->expr);
	case UNARY_SIZEOF:
		sizeof_value = evaluate_expr_itype(unary_expr->expr);
		return (int64_t)(sizeof(sizeof_value));
	case UNARY_CAST:
		return evaluate_expr_itype(unary_expr->expr);
	}
	return 1;
}

int64_t evaluate_expr_itype(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_CONST:
		return evaluate_const_itype(expr->cnst);
	case EXPR_UNARY_EXPR:
		return evaluate_unary_expr_itype(expr->unary_expr);
	case EXPR_BINARY_EXPR:
		return evaluate_binary_expr_itype(expr->binary_expr);
	case EXPR_TERNARY_EXPR:
		return evaluate_ternary_expr_itype(expr->ternary_expr);
	}
	return 1;
}

//-------------------------------------------------

double evaluate_const_ftype(Const* cnst)
{
	switch (cnst->kind)
	{
	case CONST_INT:
	case CONST_UINT:
		return cnst->ivalue;
	case CONST_FLOAT:
		return cnst->fvalue;
	}
	return 1.0f;
}

double evaluate_ternary_expr_ftype(TernaryExpr* ternary_expr)
{
	return evaluate_expr_ftype(ternary_expr->cond) ?
		evaluate_expr_ftype(ternary_expr->lexpr) : evaluate_expr_ftype(ternary_expr->rexpr);
}

double evaluate_binary_expr_ftype(BinaryExpr* binary_expr)
{
	switch (binary_expr->kind)
	{
	case BINARY_ADD:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			+ evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_SUB:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			- evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_MULT:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			* evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_DIV:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			/ evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LG_AND:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			&& evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LG_OR:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			|| evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LG_EQ:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			== evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LG_NEQ:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			!= evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LESS_THAN:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			< evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_GREATER_THAN:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			> evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_LESS_EQ_THAN:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			<= evaluate_expr_ftype(binary_expr->rexpr));
	case BINARY_GREATER_EQ_THAN:
		return (evaluate_expr_ftype(binary_expr->lexpr)
			>= evaluate_expr_ftype(binary_expr->rexpr));
	}
	return 1.0f;
}

double evaluate_unary_expr_ftype(UnaryExpr* unary_expr)
{
	double sizeof_value;
	switch (unary_expr->kind)
	{
	case UNARY_PLUS:
		return evaluate_expr_ftype(unary_expr->expr);
	case UNARY_MINUS:
		return -evaluate_expr_ftype(unary_expr->expr);
	case UNARY_LG_NOT:
		return !evaluate_expr_ftype(unary_expr->expr);
	case UNARY_SIZEOF:
		sizeof_value = evaluate_expr_ftype(unary_expr->expr);
		return (double)(sizeof(sizeof_value));
	case UNARY_CAST:
		return evaluate_expr_ftype(unary_expr->expr);
	}
	return 1.0f;
}

double evaluate_expr_ftype(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_CONST:
		return evaluate_const_ftype(expr->cnst);
	case EXPR_UNARY_EXPR:
		return evaluate_unary_expr_ftype(expr->unary_expr);
	case EXPR_BINARY_EXPR:
		return evaluate_binary_expr_ftype(expr->binary_expr);
	case EXPR_TERNARY_EXPR:
		return evaluate_ternary_expr_ftype(expr->ternary_expr);
	}
	return 1.0f;
}