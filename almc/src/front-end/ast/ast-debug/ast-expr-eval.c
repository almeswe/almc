#include "ast-expr-eval.h"

//todo: rewrite for new lexer

int32_t eval_ast(AstRoot* ast)
{
	int res = 0;
	for (uint32_t i = 0; i < sbuffer_len(ast->exprs); i++)
		res += eval_expr(ast->exprs[i]);
	return res;
}

int32_t eval_const(Const* cnst)
{
	return cnst->ivalue;
}

int32_t eval_uexpr(UnaryExpr* uexpr)
{
	switch (uexpr->type)
	{
	case UNARY_PLUS:
		return eval_expr(uexpr->expr);
	case UNARY_MINUS:
		return -eval_expr(uexpr->expr);
	case UNARY_BW_NOT:
		return ~eval_expr(uexpr->expr);
	case UNARY_LG_NOT:
		return !eval_expr(uexpr->expr);
	default:
		assert(0);
	}
}

int32_t eval_bexpr(BinaryExpr* bexpr)
{
	switch (bexpr->type)
	{
	case BINARY_ADD:
		return (eval_expr(bexpr->lexpr)
			+ eval_expr(bexpr->rexpr));
	case BINARY_SUB:
		return (eval_expr(bexpr->lexpr)
			- eval_expr(bexpr->rexpr));
	case BINARY_MULT:
		return (eval_expr(bexpr->lexpr)
			* eval_expr(bexpr->rexpr));
	case BINARY_DIV:
		return (eval_expr(bexpr->lexpr)
			/ eval_expr(bexpr->rexpr));
	case BINARY_MOD:
		return (eval_expr(bexpr->lexpr)
			% eval_expr(bexpr->rexpr));
	case BINARY_BW_AND:
		return (eval_expr(bexpr->lexpr)
			& eval_expr(bexpr->rexpr));
	case BINARY_BW_OR:
		return (eval_expr(bexpr->lexpr)
			| eval_expr(bexpr->rexpr));
	case BINARY_BW_XOR:
		return (eval_expr(bexpr->lexpr)
			^ eval_expr(bexpr->rexpr));
	case BINARY_LG_AND:
		return (eval_expr(bexpr->lexpr)
			&& eval_expr(bexpr->rexpr));
	case BINARY_LG_OR:
		return (eval_expr(bexpr->lexpr)
			|| eval_expr(bexpr->rexpr));
	case BINARY_LG_EQ:
		return (eval_expr(bexpr->lexpr)
			== eval_expr(bexpr->rexpr));
	case BINARY_LG_NEQ:
		return (eval_expr(bexpr->lexpr)
			!= eval_expr(bexpr->rexpr));
	case BINARY_LSHIFT:
		return (eval_expr(bexpr->lexpr)
			<< eval_expr(bexpr->rexpr));
	case BINARY_RSHIFT:
		return (eval_expr(bexpr->lexpr)
			>> eval_expr(bexpr->rexpr));
	case BINARY_LESS_THAN:
		return (eval_expr(bexpr->lexpr)
			< eval_expr(bexpr->rexpr));
	case BINARY_GREATER_THAN:
		return (eval_expr(bexpr->lexpr)
			> eval_expr(bexpr->rexpr));
	case BINARY_LESS_EQ_THAN:
		return (eval_expr(bexpr->lexpr)
			<= eval_expr(bexpr->rexpr));
	case BINARY_GREATER_EQ_THAN:
		return (eval_expr(bexpr->lexpr)
			>= eval_expr(bexpr->rexpr));
	default:
		assert(0);
	}
	return 0;
}

int32_t eval_texpr(TernaryExpr* texpr)
{
	return eval_expr(texpr->cond) ?
		eval_expr(texpr->lexpr) : eval_expr(texpr->rexpr);
}

int32_t eval_expr(Expr* expr)
{
	switch (expr->type)
	{
	case EXPR_CONST:
		return eval_const(expr->cnst);
	case EXPR_UNARY_EXPR:
		return eval_uexpr(expr->unary_expr);
	case EXPR_BINARY_EXPR:
		return eval_bexpr(expr->binary_expr);
	case EXPR_TERNARY_EXPR:
		return eval_texpr(expr->ternary_expr);
	}
	return 0;
}