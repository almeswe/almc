#include "type-checker.h"

Type* get_expr_type(Expr* expr)
{
	switch (expr->type)
	{
	case EXPR_CONST:
		return get_const_type(expr->cnst);
	case EXPR_STRING:
		return get_string_type(expr->str);
	case EXPR_UNARY_EXPR:
		return get_unary_expr_type(expr->unary_expr);
	case EXPR_BINARY_EXPR:
		return get_binary_expr_type(expr->unary_expr);
	default:
		assert(0);
	}
}

Type* get_const_type(Const* cnst)
{
	Type* type = cnew_s(Type, type, 1);
	type->mods.is_predefined = 1;

	switch (cnst->type)
	{
	case CONST_INT:
		type->repr = cnst->ivalue <= INT32_MAX &&
			cnst->ivalue >= INT32_MIN ? "i32" : "i64";
		break;
	case CONST_UINT:
		type->repr = cnst->ivalue <= UINT32_MAX ?
			"u32" : "u64";
		break;
	case CONST_FLOAT:
		type->repr = (int64_t)cnst->fvalue <= INT32_MAX &&
			(int64_t)cnst->fvalue >= INT32_MIN ? "f32" : "f64";
		break;
	}
	return type;
}

Type* get_string_type(Str* str)
{
	Type* type = cnew_s(Type, type, 1);
	type->mods.is_predefined = 1;
	type->mods.is_ptr = 1;
	type->repr = "str";
	return type;
}

Type* get_unary_expr_type(UnaryExpr* unary_expr)
{
	Type* type = NULL;
	switch (unary_expr->type)
	{
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
	case UNARY_BW_NOT:
		type = get_expr_type(unary_expr->expr);
		break;
	case UNARY_ADDRESS:
		type = get_expr_type(unary_expr->expr);
		type->mods.is_ptr += 1;
		return type;
	case UNARY_DEREFERENCE:
		type = get_expr_type(unary_expr->expr);
		if (IS_POINTER_TYPE(type))
			type->mods.is_ptr -= 1;
		else
			report_error(frmt("Cannot dereference value type of [%s], use pointer type.", 
				type->repr), NULL);
		return type;
	case UNARY_CAST:
		return unary_expr->cast_type;
	default:
		assert(0);
	}
	return type;
}

Type* get_binary_expr_type(BinaryExpr* binary_expr)
{
	Type* ltype = get_expr_type(
		binary_expr->lexpr);
	Type* rtype = get_expr_type(
		binary_expr->rexpr);

	switch (binary_expr->type)
	{
	//-----------------------------
	// operators that can be applied with strings
	case BINARY_ADD:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
	case BINARY_ASSIGN:
	case BINARY_ADD_ASSIGN:
		return can_cast_implicitly(rtype, ltype) ?
			cast_emplicitly(rtype, ltype) : cast_emplicitly(ltype, rtype);
	//-----------------------------

	//-----------------------------
	// operators that can be applied with numeric type & pointers
	case BINARY_SUB:
	case BINARY_DIV:
	case BINARY_MULT:
	case BINARY_SUB_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_MUL_ASSIGN:
	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return can_cast_implicitly(rtype, ltype) ? 
					cast_emplicitly(rtype, ltype) : cast_emplicitly(ltype, rtype);
	//-----------------------------
	
	//-----------------------------
	// operators that can be applied only with integral types
	case BINARY_MOD:
	case BINARY_BW_OR:
	case BINARY_BW_AND:
	case BINARY_BW_XOR:
	case BINARY_LSHIFT:
	case BINARY_RSHIFT:
	case BINARY_MOD_ASSIGN:
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
		if (IS_INTEGRAL_TYPE(ltype) && 
			IS_INTEGRAL_TYPE(rtype))
				return can_cast_implicitly(rtype, ltype) ?
					cast_emplicitly(rtype, ltype) : cast_emplicitly(ltype, rtype);
	//-----------------------------

	default:
		report_error("Cannot use this operator with this operand types.", NULL);
		assert(0);
	}
}

uint32_t get_type_priority(Type* type)
{
	if (IS_U8(type))
		return U8;
	if (IS_I8(type))
		return I8;
	if (IS_CHAR(type))
		return CHAR;
	if (IS_U16(type))
		return U16;
	if (IS_I16(type))
		return I16;
	if (IS_U32(type))
		return U32;
	if (IS_I32(type))
		return I32;
	if (IS_U64(type))
		return U64;
	if (IS_I64(type))
		return I64;
	if (IS_F32(type))
		return F32;
	if (IS_F64(type))
		return F64;
	if (IS_STRING(type))
		return STR;
	if (IS_VOID(type))
		return VOID;
	assert(0);
}

Type* cast_implicitly(Type* to, Type* type)
{
	if (can_cast_implicitly(to, type))
	{
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) >= U32))
			return type;
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) < U32))
			return to;
		return get_type_priority(to) >= get_type_priority(type) ?
			to : type;
	}
	else
		report_error(frmt("Cannot implicitly cast type %s to %s",
			to->repr, type->repr), NULL);
}

uint32_t can_cast_implicitly(Type* to, Type* type)
{
	if (to->mods.is_void || type->mods.is_void)
		return 0;

	// if one type is string, except second type
	if ((IS_STRING(to) && !IS_STRING(type)) ||
		(!IS_STRING(to) && IS_STRING(type)))
			return 0;

	// case when types are equal
	if (strcmp(to->repr, type->repr) == 0 &&
		!IS_POINTER_TYPE(to) && !IS_POINTER_TYPE(type))
			return 1;

	// case of two pointers of same rank
	if (to->mods.is_ptr && (to->mods.is_ptr == type->mods.is_ptr))
		return 1;

	// case of pointer && integral (not pointer)
	if (to->mods.is_ptr && IS_INTEGRAL_TYPE(type) && !IS_POINTER_TYPE(type))
		return 1;

	// case of integral (not pointer) && pointer
	if (type->mods.is_ptr && IS_INTEGRAL_TYPE(to) && !IS_POINTER_TYPE(to))
		return 1;

	if ((IS_U8(to)   || IS_I8(to)   || IS_CHAR(to)) &&
	    (IS_U8(type) || IS_I8(type) || IS_CHAR(type)))
			return 1;

	else if ((IS_U16(to) || IS_I16(to))   &&
	    (IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
	     IS_I16(type) || IS_U16(type)))
			return 1;

	else if ((IS_U32(to) || IS_I32(to))   &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)))
			return 1;

	else if ((IS_U64(to) || IS_I64(to))   &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)  ||
		 IS_I64(type) || IS_U64(type)))
			return 1;

	else if (IS_F32(to) &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)))
			return 1;

	else if (IS_F64(to) &&
	   (IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		IS_I16(type) || IS_U16(type)  ||
		IS_I32(type) || IS_U32(type)  ||
		IS_I64(type) || IS_U64(type)  || IS_F32(type)))
			return 1;
	return 0;
}