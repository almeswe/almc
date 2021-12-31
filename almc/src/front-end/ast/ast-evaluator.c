#include "ast-evaluator.h"

#define CAST(value, type)            \
	if (IS_CHAR_TYPE(type))          \
		return (char)value;          \
	else if (IS_I8_TYPE(type))       \
		return (int8_t)value;        \
	else if (IS_I16_TYPE(type))      \
		return (int16_t)value;       \
	else if (IS_I32_TYPE(type))      \
		return (int32_t)value;       \
	else if (IS_I64_TYPE(type))      \
		return (int64_t)value;       \
	else if (IS_U8_TYPE(type))       \
		return (uint8_t)value;       \
	else if (IS_U16_TYPE(type))      \
		return (uint16_t)value;      \
	else if (IS_U32_TYPE(type))      \
		return (uint32_t)value;      \
	else if (IS_U64_TYPE(type))      \
		return (uint64_t)value;      \
	else if (IS_F32_TYPE(type))      \
		return (float)value;         \
	else if (IS_F64_TYPE(type))      \
		return (double)value;

int64_t evaluate_const_itype(Const* cnst)
{
	switch (cnst->kind)
	{
	case CONST_INT:
	case CONST_UINT:
	case CONST_CHAR:
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
		return unary_expr->cast_type->size;
	case UNARY_LENGTHOF:
		sizeof_value = evaluate_expr_itype(unary_expr->expr);
		return (int64_t)(sizeof(sizeof_value));
	case UNARY_CAST:
		CAST(evaluate_expr_ftype(unary_expr->expr),
			unary_expr->cast_type);
	}
	return 1;
}

int64_t evaluate_expr_itype(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_IDNT:
		if (expr->idnt->is_enum_member)
			return evaluate_expr_itype(
				expr->idnt->enum_member_value);
		break;
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
	case CONST_CHAR:
		return (double)cnst->ivalue;
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
		return unary_expr->cast_type->size;
	case UNARY_LENGTHOF:
		sizeof_value = evaluate_expr_ftype(unary_expr->expr);
		return (double)(sizeof(sizeof_value));
	case UNARY_CAST:
		CAST(evaluate_expr_ftype(unary_expr->expr),
			unary_expr->cast_type);
	}
	return 1.0f;
}

double evaluate_expr_ftype(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_IDNT:
		if (expr->idnt->is_enum_member)
			return evaluate_expr_ftype(
				expr->idnt->enum_member_value);
		break;
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

bool value_in_bounds_of_type(Type* type, double value)
{
	if (is_integral_type(type) || is_pointer_like_type(type))
	{
		int64_t ivalue = (int64_t)value;
		uint64_t uvalue = (uint64_t)value;

		if (IS_I8_TYPE(type))
			return IN_BOUNDS_OF(INT8_MAX, INT8_MIN, ivalue);
		if (IS_U8_TYPE(type))
			return IN_BOUNDS_OF(UINT8_MAX, 0, uvalue);
		if (IS_I16_TYPE(type))
			return IN_BOUNDS_OF(INT16_MAX, INT16_MIN, ivalue);
		if (IS_U16_TYPE(type))
			return IN_BOUNDS_OF(UINT16_MAX, 0, uvalue);
		if (IS_I32_TYPE(type) || IS_ENUM_TYPE(type))
			return IN_BOUNDS_OF(INT32_MAX, INT32_MIN, ivalue);
		if (IS_U32_TYPE(type) || IS_ENUM_TYPE(type) || is_pointer_like_type(type))
			return IN_BOUNDS_OF(UINT32_MAX, 0, uvalue);
		if (IS_I64_TYPE(type))
			return IN_BOUNDS_OF(INT64_MAX, INT64_MIN, ivalue);
		return IN_BOUNDS_OF(UINT64_MAX, 0, uvalue);
	}
	else if (is_real_type(type))
	{
		if (IS_F32_TYPE(type))
			return IN_BOUNDS_OF(FLT_MAX, FLT_MIN, value);
	}
	return false;
}