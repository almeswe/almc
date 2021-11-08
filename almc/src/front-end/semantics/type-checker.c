#include "type-checker.h"

#define type_dup(type, type_name)            \
	Type* type_name = cnew_s(Type, type, 1); \
	type_name->mods = type->mods;            \
	type_name->info = type->info;            \
	type_name->repr = _strdup(type->repr);

Type* get_expr_type(Expr* expr, Table* table)
{
	switch (expr->type)
	{
	case EXPR_IDNT:
		return get_idnt_type(expr->idnt, table);
	case EXPR_CONST:
		return get_const_type(expr->cnst);
	case EXPR_STRING:
		return get_string_type(expr->str);
	case EXPR_UNARY_EXPR:
		return get_unary_expr_type(expr->unary_expr, table);
	case EXPR_BINARY_EXPR:
		return get_binary_expr_type(expr->unary_expr, table);
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

Type* get_idnt_type(Idnt* idnt, Table* table)
{
	VarDecl* var = get_variable(idnt->svalue, table);
	return var ? var->type_var->type : NULL;
}

Type* get_string_type(Str* str)
{
	Type* type = cnew_s(Type, type, 1);
	type->mods.is_predefined = 1;
	type->mods.is_ptr = 1;
	type->repr = "str";
	return type;
}

Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table)
{
	Type* type = get_expr_type(unary_expr->expr, table);
	switch (unary_expr->type)
	{
	//-----------------------------
	// operators that can be applied with numeric types & pointers
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
		if (IS_NUMERIC_TYPE(type) || IS_POINTER_TYPE(type))
			return type;
		report_error("Cannot use this operator with this operand type.", NULL);
	//-----------------------------

	//-----------------------------
	// operators that can be applied only with integral types & pointers
	case UNARY_BW_NOT:
	case UNARY_PREFIX_INC:
	case UNARY_PREFIX_DEC:
	case UNARY_POSTFIX_INC:
	case UNARY_POSTFIX_DEC:
		if (IS_INTEGRAL_TYPE(type) || IS_POINTER_TYPE(type))
			return type;
		report_error("Cannot use this operator with this operand type.", NULL);
	//-----------------------------

	//-----------------------------
	// addresible cases
	case UNARY_ADDRESS:
		type->mods.is_ptr += 1;
		return type;
	case UNARY_DEREFERENCE:
		if (!IS_POINTER_TYPE(type))
			report_error(frmt("Cannot dereference value type of [%s].",
				type->repr), NULL);
		type->mods.is_ptr -= 1;
		return type;
	//-----------------------------

	case UNARY_CAST:
		return unary_expr->cast_type;
	case UNARY_SIZEOF:
		// todo: checking if variable in sizeof is type name (not local/global variable)
		if (unary_expr->expr->type == EXPR_IDNT)
		{
		}
		// todo: create i32 type variable
		break;
	default:
		assert(0);
	}
	return type;
}

Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table)
{
	Type* ltype = get_expr_type(
		binary_expr->lexpr, table);
	Type* rtype = get_expr_type(
		binary_expr->rexpr, table);

	// variable needed for containing possible value of struct or union (see accessor's part)
	void* type_decl_stmt = NULL; 

	switch (binary_expr->type)
	{
	//-----------------------------
	// assign operators need only one check for implicit cast (to left assignable type) 

	// in case of common assign, any type can be
	case BINARY_ASSIGN:
		return cast_implicitly(ltype, rtype);

	// assign operators that needs numeric or pointer types
	case BINARY_SUB_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_MUL_ASSIGN:
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return cast_implicitly(ltype, rtype);
		report_error("Cannot use this operator with this operand types.", NULL);

	// assign operators that needs integral or pointer type
	case BINARY_MOD_ASSIGN:
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
		if ((IS_INTEGRAL_TYPE(ltype) || IS_POINTER_TYPE(ltype)) && 
		    (IS_INTEGRAL_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return cast_implicitly(ltype, rtype);
		report_error("Cannot use this operator with this operand types.", NULL);
	//-----------------------------


	//-----------------------------
	// operators that can be applied with strings
	case BINARY_ADD:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
	case BINARY_ADD_ASSIGN:
		return can_cast_implicitly(rtype, ltype) ?
			cast_implicitly(rtype, ltype) : cast_implicitly(ltype, rtype);
		report_error("Cannot use this operator with this operand types.", NULL);
	//-----------------------------

	//-----------------------------
	// operators that can be applied with numeric type & pointers
	case BINARY_SUB:
	case BINARY_DIV:
	case BINARY_MULT:
	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return can_cast_implicitly(rtype, ltype) ? 
					cast_implicitly(rtype, ltype) : cast_implicitly(ltype, rtype);
		report_error("Cannot use this operator with this operand types.", NULL);
	//-----------------------------
	
	//-----------------------------
	// operators that can be applied only with integral types
	case BINARY_MOD:
	case BINARY_BW_OR:
	case BINARY_BW_AND:
	case BINARY_BW_XOR:
	case BINARY_LSHIFT:
	case BINARY_RSHIFT:
		if ((IS_INTEGRAL_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_INTEGRAL_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return can_cast_implicitly(rtype, ltype) ?
					cast_implicitly(rtype, ltype) : cast_implicitly(ltype, rtype);
		report_error("Cannot use this operator with this operand types.", NULL);
	//-----------------------------

	//------------------------------
	// accessors
	case BINARY_ARR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))			
			report_error(frmt("Cannot access array element value type of [%s], pointer type expected.",
				ltype->repr), NULL);
		// index of an array should be an integral type
		if (!IS_INTEGRAL_TYPE(rtype))
			report_error(frmt("Index should be value of integral type. Type [%s] met.",
				rtype->repr), NULL);
		ltype->mods.is_ptr -= 1;
		return ltype;

	case BINARY_PTR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))
			report_error(frmt("Cannot access member of non-pointer type [%s].",
				ltype->repr), NULL);
		// logic of pointer and common member accessor are the same except this primary condition
		goto find_matching_member_type;

	case BINARY_MEMBER_ACCESSOR:
		if (IS_POINTER_TYPE(ltype))
			report_error(frmt("Cannot access member of pointer type [%s].",
				ltype->repr), NULL);
		//~~~~~~~~~~~~~~~~~~~~~~~
		find_matching_member_type:
		//~~~~~~~~~~~~~~~~~~~~~~~

		// condition for both common and pointer accessors
		// checks if the left expression's type is not predefined simple type
		if (ltype->mods.is_predefined)
			report_error(frmt("Cannot access member of non user defined type [%s].",
				ltype->repr), NULL);

		// iterating through all struct and through all struct's members, trying to find matching
		if (type_decl_stmt = get_struct(ltype->repr, table))
			for (size_t i = 0; i < sbuffer_len(((StructDecl*)type_decl_stmt)->struct_mmbrs); i++)
				if (strcmp(((StructDecl*)type_decl_stmt)->struct_mmbrs[i]->var, get_member_name(binary_expr->rexpr)) == 0)
					return ((StructDecl*)type_decl_stmt)->struct_mmbrs[i]->type;

		// same logic as struct's was
		if (type_decl_stmt = get_union(ltype->repr, table))
			for (size_t i = 0; i < sbuffer_len(((UnionDecl*)type_decl_stmt)->union_mmbrs); i++)
				if (strcmp(((UnionDecl*)type_decl_stmt)->union_mmbrs[i]->var, get_member_name(binary_expr->rexpr)) == 0)
					return ((UnionDecl*)type_decl_stmt)->union_mmbrs[i]->type;

		report_error(frmt("Cannot find any member with name [%s] in type [%s].",
			binary_expr->rexpr->idnt->svalue, ltype->repr), NULL);
		break;
		//------------------------------

	default:
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
	return -0xA;
}

Type* cast_implicitly(Type* to, Type* type)
{
	if (can_cast_implicitly(to, type))
	{
		if (IS_POINTER_TYPE(to) && IS_POINTER_TYPE(type))
			return to;
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) >= U32))
			return type;
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) < U32))
			return to;
		if ((get_type_priority(to) >= U32) && IS_POINTER_TYPE(type))
			return to;
		if ((get_type_priority(to) < U32) && IS_POINTER_TYPE(type))
			return type;
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

	// types that can be casted to i8, u8, chr
	if ((IS_U8(to)   || IS_I8(to)   || IS_CHAR(to)) &&
	    (IS_U8(type) || IS_I8(type) || IS_CHAR(type)))
			return 1;

	// types that can be casted to i16, u16
	else if ((IS_U16(to) || IS_I16(to))   &&
	    (IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
	     IS_I16(type) || IS_U16(type)))
			return 1;

	// types that can be casted to i32, u32
	else if ((IS_U32(to) || IS_I32(to))   &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)))
			return 1;

	// types that can be casted to i64, u64
	else if ((IS_U64(to) || IS_I64(to))   &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)  ||
		 IS_I64(type) || IS_U64(type)))
			return 1;

	// types that can be casted to f32
	else if (IS_F32(to) &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)))
			return 1;

	// types that can be casted to f64
	else if (IS_F64(to) &&
	   (IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		IS_I16(type) || IS_U16(type)  ||
		IS_I32(type) || IS_U32(type)  ||
		IS_I64(type) || IS_U64(type)  || IS_F32(type)))
			return 1;
	return 0;
}

char* get_member_name(Expr* expr)
{
	// function is needed espesially for recognizing the member's name in accessor expression
	// i mean that we are not fully knew that member will be represented as Idnt
	// it can be ...->a[i], ...->a++/--, ...->a.a-> ...
	switch (expr->type)
	{
	case EXPR_IDNT:
		return expr->idnt->svalue;
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->type)
		{
		case UNARY_POSTFIX_DEC:
		case UNARY_POSTFIX_INC:
			return get_member_name(expr->unary_expr->expr);
		default:
			report_error("Cannot get member name.", NULL);
			break;
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->type)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
		case BINARY_ARR_MEMBER_ACCESSOR:
			return get_member_name(expr->binary_expr->lexpr);
		default:
			report_error("Cannot get member name.", NULL);
			break;
		}
		break;
	default:
		report_error("Cannot get member name.", NULL);
		break;
	}
}