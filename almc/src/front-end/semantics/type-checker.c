#include "type-checker.h"

#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

Type* get_expr_type(Expr* expr, Table* table)
{
	if (!expr)
		return &unknown_type;

	switch (expr->kind)
	{
	case EXPR_IDNT:
		return get_idnt_type(expr->idnt, table, table);
	case EXPR_CONST:
		return get_const_type(expr->cnst);
	case EXPR_STRING:
		return get_string_type(expr->str);
	case EXPR_FUNC_CALL:
		return get_func_call_type(expr->func_call, table);
	case EXPR_UNARY_EXPR:
		return get_unary_expr_type(expr->unary_expr, table);
	case EXPR_BINARY_EXPR:
		return get_binary_expr_type(expr->binary_expr, table);
	case EXPR_TERNARY_EXPR:
		return get_ternary_expr_type(expr->ternary_expr, table);
	default:
		report_error(frmt("Unknown expression kind met in get_expr_type(): %d",
			expr->kind), NULL);
	}
	return &unknown_type;
}

Type* get_const_type(Const* cnst)
{
	switch (cnst->kind)
	{
	case CONST_INT:
		return IN_BOUNDS_OF(INT32_MAX, INT32_MIN, cnst->ivalue) ?
			&i32_type : &i64_type;
	case CONST_UINT:
		return IN_BOUNDS_OF(UINT32_MAX, 0, cnst->uvalue) ?
			&u32_type : &u64_type;
	case CONST_FLOAT:
		return IN_BOUNDS_OF(FLT_MAX, FLT_MIN, cnst->fvalue) ?
			&f32_type : &f64_type;
	case CONST_CHAR:
		return &char_type;
	}
	return &unknown_type;
}

Type* get_ivalue_type(int64_t value)
{
	if (IN_BOUNDS_OF(INT8_MAX, INT8_MIN, value))
		return &i8_type;
	else if (IN_BOUNDS_OF(UINT8_MAX, 0, value))
		return &u8_type;
	else if (IN_BOUNDS_OF(INT16_MAX, INT16_MIN, value))
		return &i16_type;
	else if (IN_BOUNDS_OF(UINT16_MAX, 0, value))
		return &u16_type;
	else if (IN_BOUNDS_OF(INT32_MAX, INT32_MIN, value))
		return &i32_type;
	else if (IN_BOUNDS_OF(UINT32_MAX, 0, value))
		return &u32_type;
	else if (IN_BOUNDS_OF(INT64_MAX, INT64_MIN, value))
		return &i64_type;
	else
		return &u64_type;
}

Type* get_fvalue_type(double value)
{
	return IN_BOUNDS_OF(FLT_MAX, FLT_MIN, value) ?
		&f32_type : &f64_type;
}

Type* get_idnt_type(Idnt* idnt, Table* table)
{
	if (is_enum_member(idnt->svalue, table))
		return get_enum_member_type(idnt->svalue, table);
	if (!is_variable_declared(idnt->svalue, table) &&
		!is_function_param_passed(idnt->svalue, table))
			return &unknown_type;

	Type* type = is_function_param_passed(idnt->svalue, table) ? 
		get_function_param(idnt->svalue, table)->type : 
			get_variable(idnt->svalue, table)->type_var->type;
	return type;
}

Type* get_string_type(Str* str)
{
	//todo: change for str type, char* for now
	return pointer_type_new(&char_type);
}

Type* get_func_call_type(FuncCall* func_call, Table* table)
{
	// also checking type of each function's argument with type of passed value
	FuncDecl* origin = get_function(func_call->name, table);
	for (size_t i = 0; i < sbuffer_len(origin->params); i++)
		cast_implicitly(origin->params[i]->type, get_expr_type(func_call->args[i], table),
			get_expr_area(func_call->args[i]));
	return origin->type;
}

Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table)
{
	Type* type = get_and_set_expr_type(unary_expr->expr, table);

	switch (unary_expr->kind)
	{
	//-----------------------------
	// operators that can be applied with numeric types & pointers
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
		if (is_numeric_type(type))
			return type;
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);

	//-----------------------------
	// operators that can be applied only with integral types & pointers
	case UNARY_BW_NOT:
	case UNARY_PREFIX_INC:
	case UNARY_PREFIX_DEC:
	case UNARY_POSTFIX_INC:
	case UNARY_POSTFIX_DEC:
		if (is_integral_type(type))
			return type;
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);

	//-----------------------------
	// addresible cases
	case UNARY_ADDRESS:
		if (type)
			return address_type(type);
		else
			report_error2("Cannot determine the type of unary expression " 
				"while taking an address.", unary_expr->area);
	case UNARY_DEREFERENCE:
		if (is_pointer_like_type(type))
			return dereference_type(type);
		else
			report_error2(frmt("Cannot dereference value of type \'%s\'.",
				type_tostr_plain(type)), unary_expr->area);
	//-----------------------------

	case UNARY_CAST:
		if (is_const_expr(unary_expr->expr))
			return cast_explicitly_when_const_expr(unary_expr->expr,
				unary_expr->cast_type, type);
		return cast_explicitly(unary_expr->cast_type, type);
	case UNARY_SIZEOF:
	case UNARY_LENGTHOF:
		// use u16 instead of u32 because it will be easier to put it in signed expression
		// (no neeed to cast)
		return &u16_type; 
	default:
		report_error(frmt("Unknown unary expression kind met: %d.",
			unary_expr->type), NULL);
	}
	// to avoid warning
	return type;
}

Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table)
{
	Type* ltype = get_and_set_expr_type(
		binary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(
		binary_expr->rexpr, table);

	switch (binary_expr->kind)
	{
	//-----------------------------
	// cases with relative operators, which must return i32 type in any situation

	// with any type
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
		if (can_cast_implicitly(ltype, rtype) ||
			can_cast_implicitly(rtype, ltype))
				return &i32_type;
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	// with numeric type
	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		if ((is_numeric_type(ltype) || is_pointer_like_type(ltype)) &&
			(is_numeric_type(rtype) || is_pointer_like_type(rtype)))
				if (can_cast_implicitly(ltype, rtype) ||
					can_cast_implicitly(rtype, ltype))
						return &i32_type;
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	//-----------------------------
	// assign operators need only one check for implicit cast (to left assignable type) 
	// in case of common assign, any type can be
	case BINARY_ASSIGN:
	case BINARY_ADD_ASSIGN:
		return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);

	// assign operators that needs numeric or pointer types
	case BINARY_SUB_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_MUL_ASSIGN:
		if (is_numeric_type(ltype) &&
			is_numeric_type(rtype))
				return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.", 
			binary_expr->area);

	// assign operators that needs integral or pointer type
	case BINARY_MOD_ASSIGN:
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
		if (is_integral_type(ltype) &&
		    is_integral_type(rtype))
				return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.", 
			binary_expr->area);

	//-----------------------------
	// operator that can be applied with strings
	case BINARY_ADD:
		return can_cast_implicitly(rtype, ltype) ?
			cast_implicitly(rtype, ltype, binary_expr->area) : 
				cast_implicitly(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	//-----------------------------
	// operators that can be applied with numeric type & pointers
	case BINARY_SUB:
	case BINARY_DIV:
	case BINARY_MULT:
	case BINARY_LG_OR:
	case BINARY_LG_AND:
		if (is_numeric_type(ltype) &&
			is_numeric_type(rtype))
				return can_cast_implicitly(rtype, ltype) ? 
					cast_implicitly(rtype, ltype, binary_expr->area) : 
						cast_implicitly(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.", 
			binary_expr->area);
	
	//-----------------------------
	// operators that can be applied only with integral types
	case BINARY_MOD:
	case BINARY_BW_OR:
	case BINARY_BW_AND:
	case BINARY_BW_XOR:
	case BINARY_LSHIFT:
	case BINARY_RSHIFT:
		if (is_integral_type(ltype) &&
			is_integral_type(rtype))
				return can_cast_implicitly(rtype, ltype) ?
					cast_implicitly(rtype, ltype, binary_expr->area) :
						cast_implicitly(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	//------------------------------
	// accessors
	case BINARY_ARR_MEMBER_ACCESSOR:
		if (!is_pointer_like_type(ltype))			
			report_error2(frmt("Cannot access array element value of type \'%s\', pointer type expected.",
				type_tostr_plain(ltype)), binary_expr->area);
		// index of an array should be an integral type
		if (!is_integral_type(rtype))
			report_error2(frmt("Index should be value of integral type. Type \'%s\' met.",
				type_tostr_plain(rtype)), binary_expr->area);
		return dereference_type(ltype);

	case BINARY_PTR_MEMBER_ACCESSOR:
		if (!is_pointer_like_type(ltype))
			report_error2(frmt("Cannot access member of non pointer-like type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);
		// logic of pointer and common member accessor are the same except this primary condition
		ltype = ltype->base;

	case BINARY_MEMBER_ACCESSOR:
		if (is_pointer_like_type(ltype))
			report_error2(frmt("Cannot access member of pointer-like type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);

		// condition for both common and pointer accessors
		// checks if the left expression's type is not predefined simple type
		if (IS_PRIMITIVE_TYPE(ltype))
			report_error2(frmt("Cannot access member of primitive type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);

		// iterating through all struct and through all struct's members, trying to find matching
		if (IS_STRUCT_TYPE(ltype) || IS_UNION_TYPE(ltype))
			for (size_t i = 0; i < sbuffer_len(ltype->members); i++)
				if (strcmp(ltype->members[i]->name, get_member_name(binary_expr->rexpr)) == 0)
					return ltype->members[i]->type;

		report_error2(frmt("Cannot find any member with name \'%s\' in type \'%s\'.",
			binary_expr->rexpr->idnt->svalue, ltype->repr), binary_expr->area);

	//------------------------------
	// comma expr, just return right expr's type for any size of comma expr (specific parser property)
	case BINARY_COMMA:
		return rtype;
	//------------------------------

	default:
		report_error(frmt("Unknown binary expression kind met in get_binary_expr_type(): %d",
			binary_expr->type), NULL);
	}
	// useless, only to avoid warning
	return ltype;
}

Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table)
{
	Type* ltype = get_and_set_expr_type(ternary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(ternary_expr->rexpr, table);
	Type* ctype = get_and_set_expr_type(ternary_expr->cond, table);

	if (!is_numeric_type(ctype) && !IS_POINTER_TYPE(ctype))
			report_error2("Cannot determine the type of condition in ternary expression.", 
				get_expr_area(ternary_expr->cond));
		else 
			report_error2(frmt("Expected numeric or pointer type in "
				"condition of ternary expression, type met: \'%s\'",
					type_tostr_plain(ctype)), get_expr_area(ternary_expr->cond));

	if (can_cast_implicitly(ltype, rtype))
		return cast_implicitly(ltype, rtype, ternary_expr->area);
	else if (can_cast_implicitly(rtype, ltype))
		return cast_implicitly(rtype, ltype, ternary_expr->area);
	else
		report_error2("Left and right expressions in ternary expression "
				"should be the same or implicitly equal.", ternary_expr->area);
	return &unknown_type;
}

Type* get_and_set_expr_type(Expr* expr, Table* table)
{
	if (!expr)
		return &unknown_type;

	Type* type = get_expr_type(expr, table);

	switch (expr->kind)
	{
	case EXPR_IDNT:
		return expr->idnt->type = type;
	case EXPR_CONST:
		return expr->cnst->type = type;
	case EXPR_STRING:
		return expr->str->type = type;
	case EXPR_FUNC_CALL:
		return expr->func_call->type = type;
	case EXPR_UNARY_EXPR:
		return expr->unary_expr->type = type;
	case EXPR_BINARY_EXPR:
		return expr->binary_expr->type = type;
	case EXPR_TERNARY_EXPR:
		return expr->ternary_expr->type = type;
	}
	return &unknown_type;
}

uint32_t get_type_priority(Type* type)
{
	if (IS_U8_TYPE(type))
		return U8;
	if (IS_I8_TYPE(type))
		return I8;
	if (IS_CHAR_TYPE(type))
		return CHAR;
	if (IS_U16_TYPE(type))
		return U16;
	if (IS_I16_TYPE(type))
		return I16;
	if (IS_U32_TYPE(type))
		return U32;
	if (IS_I32_TYPE(type))
		return I32;
	if (IS_U64_TYPE(type))
		return U64;
	if (IS_I64_TYPE(type))
		return I64;
	if (IS_F32_TYPE(type))
		return F32;
	if (IS_F64_TYPE(type))
		return F64;
	if (IS_STRING_TYPE(type))
		return STR;
	if (IS_VOID_TYPE(type))
		return VOID;
	if (type && !IS_PRIMITIVE_TYPE(type))
		return I32;
	return 0x0;
}

Type* cast_explicitly(Type* to, Type* type)
{
	if (!to || !type)
		report_error2("Cannot determine at least one type when "
			"trying to convert explicitly.", NULL);
	else
	{
		if (IS_VOID_TYPE(to))
			report_error2("Explicit conversion to void is not allowed.", to->area);
		if (IS_VOID_TYPE(type))
			report_error2("Explicit conversion of void is not allowed.", to->area);
		if (IS_STRING_TYPE(type) && !IS_STRING_TYPE(to) && !IS_CHAR_POINTER_TYPE(to))
			report_error2(frmt("Cannot explicitly convert type \'%s\' to \'%s\'.",
				type_tostr_plain(type), type_tostr_plain(to)), to->area);
		if (is_integral_type(to) && is_real_type(type))
			report_warning2("Converting real type to integral type may occur data losses.",
				to->area);
	}
	return to;
}

Type* cast_explicitly_when_const_expr(Expr* const_expr, Type* to, Type* const_expr_type)
{
	if (!to || !const_expr_type)
		report_error2("Cannot determine at least one type "
			"when trying to convert explicitly.", NULL);
	else
	{
		if (IS_VOID_TYPE(to))
			report_error2("Explicit conversion to void is not allowed.", to->area);
		if (!IS_PRIMITIVE_TYPE(get_base_type(to)) && !IS_ENUM_TYPE(get_base_type(to)))
			report_error2(frmt("Cannot explicitly convert constant expression to type \'%s\'.",
				type_tostr_plain(to)), to->area);

		//---------------------------------------
		// determining the type of evaluated constant
		double value = 0.0;
		Type* const_expr_type_new = NULL;
		if (is_integral_type(const_expr_type) || is_pointer_like_type(const_expr_type))
				const_expr_type_new = get_ivalue_type(
					value = evaluate_expr_itype(const_expr));
		else if (is_real_type(const_expr_type))
			const_expr_type_new = get_fvalue_type(
				value = evaluate_expr_ftype(const_expr));
		else
			report_error2("Cannot evaluate constant expression for explicit cast.",
				get_expr_area(const_expr));

		if (const_expr_type_new)
			if (to->size < const_expr_type_new->size)
				report_error2(frmt("Cannot explicitly convert constant value of type \'%s\' to \'%s\' (value: %f).",
					type_tostr_plain(to), type_tostr_plain(const_expr_type_new), value), to->area);
		return cast_explicitly(to, const_expr_type);
	}
}

Type* cast_implicitly(Type* to, Type* type, SrcArea* area)
{
	if (can_cast_implicitly(to, type))
	{
		// if both are pointer-like
		if (IS_POINTER_TYPE(to) && is_pointer_like_type(type))
			return to;
		if (is_pointer_like_type(to) && IS_POINTER_TYPE(type))
			return type;
		// if one is enum and second is integral
		if (IS_ENUM_TYPE(to) && is_integral_type(type))
			return type;
		if (IS_ENUM_TYPE(type) && is_integral_type(to))
			return to;

		// if one is pointer-like and second is primitive
		if (is_pointer_like_type(to) && (get_type_priority(type) > I32))
			return type;
		if (is_pointer_like_type(to) && (get_type_priority(type) <= I32))
			return to;
		if ((get_type_priority(to) > I32) && is_pointer_like_type(type))
			return to;
		if ((get_type_priority(to) <= I32) && is_pointer_like_type(type))
			return type;

		// if both are primitive types
		if (is_both_primitive(to, type))
			return get_type_priority(to) >= get_type_priority(type) ? to : type;
	}
	else
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(to), type_tostr_plain(type)), area);
}

Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area)
{
	if (can_cast_implicitly(to, type))
		return to;
	else
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(to), type_tostr_plain(type)), area);
}

bool can_cast_implicitly(Type* to, Type* type)
{
	if (IS_VOID_TYPE(to) || 
		IS_VOID_TYPE(type))
		return false;

	if (IS_UNKNOWN_TYPE(to) ||
		IS_UNKNOWN_TYPE(type))
		return false;

	if (IS_INCOMPLETE_TYPE(to) ||
		IS_INCOMPLETE_TYPE(type))
		return false;

	// if only one type is string
	/*if ((IS_STRING_TYPE(to) && !IS_STRING_TYPE(type)) ||
		(!IS_STRING_TYPE(to) && IS_STRING_TYPE(type)))
			return false;
	*/

	// case of two pointer-like types of same rank
	if ((is_pointer_like_type(to) && is_pointer_like_type(type)))
	{
		if (get_pointer_rank(to) == get_pointer_rank(type))
			// and also if their base types can be casted implicitly
			if (can_cast_implicitly(get_base_type(to), get_base_type(type)))
				return true;
	}

	// case when trying to cast max 4 byte integral type to enum type
	else if (IS_ENUM_TYPE(to) && is_integral_smaller_than_pointer_type(type))
		return true;

	else if (!is_integral_smaller_than_pointer_type(to) &&
		is_pointer_like_type(type))
		return true;

	else if (is_pointer_like_type(to) &&
		is_integral_smaller_than_pointer_type(type))
		return true;

	else if (is_both_primitive(to, type))
	{
		// types that can be casted to u8 and char
		if ((IS_U8_TYPE(to) || IS_CHAR_TYPE(to)) &&
			(IS_U8_TYPE(type) || IS_CHAR_TYPE(type)))
			return true;

		// types that can be casted to i8
		if (IS_I8_TYPE(to) &&
			IS_I8_TYPE(type))
			return true;

		// types that can be casted to u16
		else if (IS_U16_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_U16_TYPE(type)))
			return true;

		// types that can be casted to i16
		else if (IS_I16_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type)))
			return true;

		// types that can be casted to u32
		else if (IS_U32_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_U32_TYPE(type)))
			return true;

		// types that can be casted to i32
		else if (IS_I32_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_I32_TYPE(type)))
			return true;

		// types that can be casted to u64
		else if (IS_U64_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_I32_TYPE(type) || IS_U32_TYPE(type) ||
				IS_U64_TYPE(type)))
			return true;

		// types that can be casted to i64
		else if (IS_I64_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_I32_TYPE(type) || IS_U32_TYPE(type) ||
				IS_I64_TYPE(type)))
			return true;

		// types that can be casted to f32
		else if (IS_F32_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_I32_TYPE(type) || IS_U32_TYPE(type)))
			return true;

		// types that can be casted to f64
		else if (IS_F64_TYPE(to) &&
			(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
				IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
				IS_I32_TYPE(type) || IS_U32_TYPE(type) ||
				IS_I64_TYPE(type) || IS_U64_TYPE(type) || IS_F32_TYPE(type)))
			return true;
	}
	else if (is_both_are_equal_user_defined(to, type))
		return true;
	return false;
}

SrcArea* get_expr_area(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_IDNT:
		return src_area_new(expr->idnt->context, NULL);
	case EXPR_CONST:
		return src_area_new(expr->cnst->context, NULL);
	case EXPR_STRING:
		return src_area_new(expr->str->context, NULL);
	case EXPR_FUNC_CALL:
		return expr->func_call->area;
	case EXPR_INITIALIZER:
		return expr->initializer->area;
	case EXPR_UNARY_EXPR:
		return expr->unary_expr->area;
	case EXPR_BINARY_EXPR:
		return expr->binary_expr->area;
	case EXPR_TERNARY_EXPR:
		return expr->ternary_expr->area;
	default:
		report_error("Unknown expression kind for selecting any context.", NULL);
	}
	return &unknown_type;
}

char* get_member_name(Expr* expr)
{
	// function is needed espesially for recognizing the member's name in accessor expression
	// i mean that we are not fully knew that member will be represented as Idnt
	// it can be ...->a[i], ...->a++/--, ...->a.a-> ...
	switch (expr->kind)
	{
	case EXPR_IDNT:
		return expr->idnt->svalue;
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_POSTFIX_DEC:
		case UNARY_POSTFIX_INC:
			return get_member_name(expr->unary_expr->expr);
		default:
			report_error2("Cannot get member name from unary expression.", 
				expr->unary_expr->area);
			break;
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
		case BINARY_ARR_MEMBER_ACCESSOR:
			return get_member_name(expr->binary_expr->lexpr);
		default:
			report_error2("Cannot get member name from binary expression.", 
				expr->binary_expr->area);
			break;
		}
		break;
	default:
		report_error("Cannot get member name from expression.", NULL);
		break;
	}
	return &unknown_type;
}

int is_const_expr(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_CONST:
		return 1;
	case EXPR_UNARY_EXPR:
		return is_const_expr(expr->unary_expr->expr);
	case EXPR_BINARY_EXPR:
		return is_const_expr(expr->binary_expr->lexpr) &&
			   is_const_expr(expr->binary_expr->rexpr);
	case EXPR_TERNARY_EXPR:
		return is_const_expr(expr->ternary_expr->lexpr) &&
			   is_const_expr(expr->ternary_expr->rexpr) &&
			   is_const_expr(expr->ternary_expr->cond);
	}
	return 0;
}

int is_simple_const_expr(Expr* expr)
{
	// the diffrence with is_const_expr 
	// that here i removed logic with unary expressions
	switch (expr->kind)
	{
	case EXPR_CONST:
		return 1;
	case EXPR_BINARY_EXPR:
		return is_simple_const_expr(expr->binary_expr->lexpr) &&
			is_simple_const_expr(expr->binary_expr->rexpr);
	case EXPR_TERNARY_EXPR:
		return is_simple_const_expr(expr->ternary_expr->lexpr) &&
			is_simple_const_expr(expr->ternary_expr->rexpr) &&
			is_simple_const_expr(expr->ternary_expr->cond);
	}
	return 0;
}

int is_enum_member(const char* var, Table* table)
{
	for (Table* parent = table; parent; parent = parent->parent)
		for (size_t i = 0; i < sbuffer_len(parent->enums); i++)
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->members); j++)
				if (strcmp(var, parent->enums[i]->members[j]->name) == 0)
					return 1;
	return 0;
}

int is_addressable_value(Expr* expr, Table* table)
{
	if (!expr)
		return 0;
	switch (expr->kind)
	{
	case EXPR_IDNT:
		return !is_enum_member(
			expr->idnt->svalue, table);
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_PREFIX_INC:
		case UNARY_PREFIX_DEC:
		case UNARY_POSTFIX_INC:
		case UNARY_POSTFIX_DEC:
			return is_addressable_value(expr->unary_expr->expr, table);

		case UNARY_DEREFERENCE:
			switch (expr->unary_expr->expr->kind)
			{
			case EXPR_IDNT:
				return 1;
			case EXPR_UNARY_EXPR:
				return is_addressable_value(expr->unary_expr->expr, table);
			case EXPR_BINARY_EXPR:
				return is_addressable_value(expr->unary_expr->expr->binary_expr->lexpr, table) ||
					is_addressable_value(expr->unary_expr->expr->binary_expr->rexpr, table);
			default:
				return 0;
			}
			break;
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
			return is_addressable_value(expr->binary_expr->lexpr, table) &&
				is_addressable_value(expr->binary_expr->rexpr, table);
		case BINARY_ARR_MEMBER_ACCESSOR:
			return is_addressable_value(expr->binary_expr->lexpr, table);
		default:
			return 0;
		}
		break;
		/*case EXPR_TERNARY_EXPR:
			return is_addressable_value(expr->ternary_expr->rexpr, table);*/
	}
	return 0;
}

Type* get_enum_member_type(const char* member, Table* table)
{
	for (Table* parent = table; parent; parent = parent->parent)
		for (size_t i = 0; i < sbuffer_len(parent->enums); i++)
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->members); j++)
				if (strcmp(member, parent->enums[i]->members[j]->name) == 0)
					return get_expr_type(parent->enums[i]->members[j]->value, table);
	return &unknown_type;
}