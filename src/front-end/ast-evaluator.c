#include "ast-evaluator.h"

#define static_eval_cast(value, type) 	\
	if (is_char_type(type)) {         	\
		return (char)value;			  	\
	}        							\
	else if (is_i8_type(type)) {       	\
		return (int8_t)value;			\
	}									\
	else if (is_i16_type(type)) {		\
		return (int16_t)value;       	\
	}  									\
	else if (is_i32_type(type)) {     	\
		return (int32_t)value;   		\
	}    								\
	else if (is_i64_type(type)) {      	\
		return (int64_t)value;   		\
	}    								\
	else if (is_u8_type(type)) {       	\
		return (uint8_t)value;  		\
	}     								\
	else if (is_u16_type(type)) {      	\
		return (uint16_t)value;  		\
	}    								\
	else if (is_u32_type(type)) {      	\
		return (uint32_t)value;  		\
	}    								\
	else if (is_u64_type(type)) {      	\
		return (uint64_t)value;  		\
	}    								\
	else if (is_f32_type(type)) {      	\
		return (float)value;     		\
	}    								\
	else if (is_f64_type(type)) {      	\
		return (double)value;			\
	}

double evaluate_expr(Expr* expr) {
	if (is_real_type(retrieve_expr_type(expr))) {
		return evaluate_expr_ftype(expr);
	}
	return (double)evaluate_expr_itype(expr);
}

int64_t evaluate_const_itype(Const* cnst) {
	switch (cnst->kind) {
		case CONST_INT:
		case CONST_UINT:
		case CONST_CHAR:
			return cnst->ivalue;
	}
	report_error(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), cnst->context);
}

int64_t evaluate_ternary_expr_itype(TernaryExpr* ternary_expr) {
	return (int64_t)evaluate_expr(ternary_expr->cond) ?
		(int64_t)evaluate_expr(ternary_expr->lexpr) : (int64_t)evaluate_expr(ternary_expr->rexpr);
}

int64_t evaluate_binary_expr_itype(BinaryExpr* binary_expr) {
	switch (binary_expr->kind) {
		case BINARY_ADD:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				+ (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_SUB:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				- (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_MULT:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				* (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_DIV:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				/ (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_MOD:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				% (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_BW_AND:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				& (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_BW_OR:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				| (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_BW_XOR:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				^ (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LSHIFT:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				<< (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_RSHIFT:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				>> (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_AND:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				&& (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_OR:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				|| (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_EQ:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				== (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_NEQ:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				!= (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LESS_THAN:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				< (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_GREATER_THAN:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				> (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_LESS_EQ_THAN:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				<= (int64_t)evaluate_expr(binary_expr->rexpr));
		case BINARY_GREATER_EQ_THAN:
			return ((int64_t)evaluate_expr(binary_expr->lexpr)
				>= (int64_t)evaluate_expr(binary_expr->rexpr));
	}
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(binary_expr->lexpr));
}

int64_t evaluate_unary_expr_itype(UnaryExpr* unary_expr) {
	switch (unary_expr->kind) {
		case UNARY_PLUS:
			return (int64_t)evaluate_expr(unary_expr->expr);
		case UNARY_MINUS:
			return -(int64_t)evaluate_expr(unary_expr->expr);
		case UNARY_BW_NOT:
			return ~(int64_t)evaluate_expr(unary_expr->expr);
		case UNARY_LG_NOT:
			return !(int64_t)evaluate_expr(unary_expr->expr);
		case UNARY_SIZEOF:
			return (int64_t)unary_expr->cast_type->size;
		case UNARY_LENGTHOF:
			return (int64_t)retrieve_expr_type(unary_expr->expr)->size;
		case UNARY_CAST:
			static_eval_cast(evaluate_expr(unary_expr->expr),
				unary_expr->cast_type);
	}
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(unary_expr->expr));
}

int64_t evaluate_expr_itype(Expr* expr) {
	switch (expr->kind) {
		case EXPR_IDNT:
			if (expr->idnt->attrs.is_enum_member)
				return evaluate_expr_itype(
					expr->idnt->attrs.enum_member->value);
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
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(expr));
}

//-------------------------------------------------

double evaluate_const_ftype(Const* cnst) {
	switch (cnst->kind) {
		case CONST_INT:
		case CONST_UINT:
		case CONST_CHAR:
			return (double)cnst->ivalue;
		case CONST_FLOAT:
			return cnst->fvalue;
	}
	report_error(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), cnst->context);
}

double evaluate_ternary_expr_ftype(TernaryExpr* ternary_expr)
{
	return evaluate_expr(ternary_expr->cond) ?
		evaluate_expr(ternary_expr->lexpr) : evaluate_expr(ternary_expr->rexpr);
}

double evaluate_binary_expr_ftype(BinaryExpr* binary_expr) {
	switch (binary_expr->kind) {
		case BINARY_ADD:
			return (evaluate_expr(binary_expr->lexpr)
				+ evaluate_expr(binary_expr->rexpr));
		case BINARY_SUB:
			return (evaluate_expr(binary_expr->lexpr)
				- evaluate_expr(binary_expr->rexpr));
		case BINARY_MULT:
			return (evaluate_expr(binary_expr->lexpr)
				* evaluate_expr(binary_expr->rexpr));
		case BINARY_DIV:
			return (evaluate_expr(binary_expr->lexpr)
				/ evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_AND:
			return (evaluate_expr(binary_expr->lexpr)
				&& evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_OR:
			return (evaluate_expr(binary_expr->lexpr)
				|| evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_EQ:
			return (evaluate_expr(binary_expr->lexpr)
				== evaluate_expr(binary_expr->rexpr));
		case BINARY_LG_NEQ:
			return (evaluate_expr(binary_expr->lexpr)
				!= evaluate_expr(binary_expr->rexpr));
		case BINARY_LESS_THAN:
			return (evaluate_expr(binary_expr->lexpr)
				< evaluate_expr(binary_expr->rexpr));
		case BINARY_GREATER_THAN:
			return (evaluate_expr(binary_expr->lexpr)
				> evaluate_expr(binary_expr->rexpr));
		case BINARY_LESS_EQ_THAN:
			return (evaluate_expr(binary_expr->lexpr)
				<= evaluate_expr(binary_expr->rexpr));
		case BINARY_GREATER_EQ_THAN:
			return (evaluate_expr(binary_expr->lexpr)
				>= evaluate_expr(binary_expr->rexpr));
	}
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(binary_expr->rexpr));
}

double evaluate_unary_expr_ftype(UnaryExpr* unary_expr) {
	switch (unary_expr->kind) {
		case UNARY_PLUS:
			return evaluate_expr(unary_expr->expr);
		case UNARY_MINUS:
			return -evaluate_expr(unary_expr->expr);
		case UNARY_LG_NOT:
			return !evaluate_expr(unary_expr->expr);
		case UNARY_SIZEOF:
			return unary_expr->cast_type->size;
		case UNARY_LENGTHOF:
			return (double)retrieve_expr_type(unary_expr->expr)->size;
		case UNARY_CAST:
			static_eval_cast(evaluate_expr(unary_expr->expr),
				unary_expr->cast_type);
	}
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(unary_expr->expr));
}

double evaluate_expr_ftype(Expr* expr) {
	switch (expr->kind) {
		case EXPR_IDNT:
			if (expr->idnt->attrs.is_enum_member)
				return evaluate_expr_ftype(
					expr->idnt->attrs.enum_member->value);
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
	report_error2(frmt("Cannot evaluate expression"
		" in function: %s", __FUNCTION__), get_expr_area(expr));
}

bool value_in_bounds_of_type(Type* type, double value) {
	if (is_integral_type(type) || is_pointer_like_type(type)) {
		int64_t ivalue = (int64_t)value;
		uint64_t uvalue = (uint64_t)value;

		if (is_i8_type(type)) {
			return IN_BOUNDS_OF(INT8_MAX, INT8_MIN, ivalue);
		}
		if (is_u8_type(type)) {
			return IN_BOUNDS_OF(UINT8_MAX, 0, uvalue);
		}
		if (is_i16_type(type)) {
			return IN_BOUNDS_OF(INT16_MAX, INT16_MIN, ivalue);
		}
		if (is_u16_type(type)) {
			return IN_BOUNDS_OF(UINT16_MAX, 0, uvalue);
		}
		if (is_i32_type(type) || is_enum_type(type)) {
			return IN_BOUNDS_OF(INT32_MAX, INT32_MIN, ivalue);
		}
		if (is_u32_type(type) || is_enum_type(type) || is_pointer_like_type(type)) {
			return IN_BOUNDS_OF(UINT32_MAX, 0, uvalue);
		}
		if (is_i64_type(type)) {
			return IN_BOUNDS_OF(INT64_MAX, INT64_MIN, ivalue);
		}
		return IN_BOUNDS_OF(UINT64_MAX, 0, uvalue);
	}
	else if (is_real_type(type)) {
		if (is_f32_type(type)) {
			return IN_BOUNDS_OF(FLT_MAX, FLT_MIN, value);
		}
		if (is_f64_type(type)) {
			return IN_BOUNDS_OF(DBL_MAX, DBL_MIN, value);
		}
	}
	return false;
}