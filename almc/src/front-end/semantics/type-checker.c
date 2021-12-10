#include "type-checker.h"

#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

Type* get_expr_type(Expr* expr, Table* table)
{
	if (!expr)
		return unknown_type_new();

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
	return unknown_type_new();
}

Type* get_const_type(Const* cnst)
{
	switch (cnst->kind)
	{
	case CONST_INT:
		return type_new(IN_BOUNDS_OF(INT32_MAX, INT32_MIN, cnst->ivalue) ?
			I32_TYPE : I64_TYPE, NULL);
	case CONST_UINT:
		return type_new(IN_BOUNDS_OF(UINT32_MAX, 0, cnst->uvalue) ?
			U32_TYPE : U64_TYPE, NULL);
	case CONST_FLOAT:
		return type_new(IN_BOUNDS_OF(FLT_MAX, FLT_MIN, cnst->fvalue) ?
			F32_TYPE : F64_TYPE, NULL);
	case CONST_CHAR:
		return type_new(CHAR_TYPE, NULL);
	}
	return unknown_type_new();
}

Type* get_ivalue_type(int64_t value)
{
	if (IN_BOUNDS_OF(INT8_MAX, INT8_MIN, value))
		return type_new(I8_TYPE, NULL);
	else if (IN_BOUNDS_OF(UINT8_MAX, 0, value))
		return type_new(U8_TYPE, NULL);
	else if (IN_BOUNDS_OF(INT16_MAX, INT16_MIN, value))
		return type_new(I16_TYPE, NULL);
	else if (IN_BOUNDS_OF(UINT16_MAX, 0, value))
		return type_new(U16_TYPE, NULL);
	else if (IN_BOUNDS_OF(INT32_MAX, INT32_MIN, value))
		return type_new(I32_TYPE, NULL);
	else if (IN_BOUNDS_OF(UINT32_MAX, 0, value))
		return type_new(U32_TYPE, NULL);
	else if (IN_BOUNDS_OF(INT64_MAX, INT64_MIN, value))
		return type_new(I64_TYPE, NULL);
	else
		return type_new(U64_TYPE, NULL);
}

Type* get_fvalue_type(double value)
{
	return type_new(IN_BOUNDS_OF(FLT_MAX, FLT_MIN, value) ?
		F32_TYPE : F64_TYPE, NULL);
}

Type* get_idnt_type(Idnt* idnt, Table* table)
{
	if (is_enum_member(idnt->svalue, table))
		return get_enum_member_type(idnt->svalue, table);
	if (!is_variable_declared(idnt->svalue, table) &&
		!is_function_param_passed(idnt->svalue, table))
			return unknown_type_new();

	Type* type = is_function_param_passed(idnt->svalue, table) ? 
		get_function_param(idnt->svalue, table)->type : 
			get_variable(idnt->svalue, table)->type_var->type;
	return type_dup(type);
}

Type* get_string_type(Str* str)
{
	return type_new(
		STRING_TYPE, NULL);
}

Type* get_func_call_type(FuncCall* func_call, Table* table)
{
	// also checking type of each function's argument with type of passed value
	FuncDecl* origin = get_function(func_call->func_name, table);
	for (size_t i = 0; i < sbuffer_len(origin->func_params); i++)
		cast_implicitly(origin->func_params[i]->type, get_expr_type(func_call->func_args[i], table),
			get_expr_area(func_call->func_args[i]));
	return type_dup(origin->func_type);
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
		if (IS_NUMERIC_TYPE(type) || IS_POINTER_TYPE(type))
			return type_dup(type);
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);

	//-----------------------------
	// operators that can be applied only with integral types & pointers
	case UNARY_BW_NOT:
	case UNARY_PREFIX_INC:
	case UNARY_PREFIX_DEC:
	case UNARY_POSTFIX_INC:
	case UNARY_POSTFIX_DEC:
		if (IS_INTEGRAL_TYPE(type) || IS_POINTER_TYPE(type))
			return type_dup(type);
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);

	//-----------------------------
	// addresible cases
	case UNARY_ADDRESS:
		if (type)
			return type_address(type_dup(type));
		else
			report_error2("Cannot determine the type of unary expression " 
				"while taking an address.", unary_expr->area);
	case UNARY_DEREFERENCE:
		if (IS_POINTER_TYPE(type))
			return type_dereference(type_dup(type));
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
		return type_new(U32_TYPE, NULL);
	default:
		report_error(frmt("Unknown unary expression kind met: %d.",
			unary_expr->type), NULL);
	}
	// to avoid warning
	return type_dup(type);
}

Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table)
{
	Type* ltype = get_and_set_expr_type(
		binary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(
		binary_expr->rexpr, table);

	// type needed to handle the array member accessor expression
	Type* new_type = NULL;

	switch (binary_expr->kind)
	{
	//-----------------------------
	// cases with relative operators, which must return i32 type in any situation

	// with any type
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
		if (can_cast_implicitly(ltype, rtype) ||
			can_cast_implicitly(rtype, ltype))
				return type_new(I32_TYPE, NULL);
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	// with numeric type
	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				if (can_cast_implicitly(ltype, rtype) ||
					can_cast_implicitly(rtype, ltype))
						return type_new(I32_TYPE, NULL);
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
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
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
		if ((IS_INTEGRAL_TYPE(ltype) || IS_POINTER_TYPE(ltype)) && 
		    (IS_INTEGRAL_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
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
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
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
		if ((IS_INTEGRAL_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_INTEGRAL_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return can_cast_implicitly(rtype, ltype) ?
					cast_implicitly(rtype, ltype, binary_expr->area) :
						cast_implicitly(ltype, rtype, binary_expr->area);
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);

	//------------------------------
	// accessors
	case BINARY_ARR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))			
			report_error2(frmt("Cannot access array element value of type \'%s\', pointer type expected.",
				type_tostr_plain(ltype)), binary_expr->area);
		// index of an array should be an integral type
		if (!IS_INTEGRAL_TYPE(rtype))
			report_error2(frmt("Index should be value of integral type. Type \'%s\' met.",
				type_tostr_plain(rtype)), binary_expr->area);
		return type_dereference(type_dup(ltype));

	case BINARY_PTR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))
			report_error2(frmt("Cannot access member of non-pointer type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);
		// logic of pointer and common member accessor are the same except this primary condition
		goto find_matching_member_type;

	case BINARY_MEMBER_ACCESSOR:
		if (IS_POINTER_TYPE(ltype))
			report_error2(frmt("Cannot access member of pointer type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);
		//~~~~~~~~~~~~~~~~~~~~~~~
		find_matching_member_type:
		//~~~~~~~~~~~~~~~~~~~~~~~

		// condition for both common and pointer accessors
		// checks if the left expression's type is not predefined simple type
		if (ltype->spec.is_predefined)
			report_error2(frmt("Cannot access member of non-user defined type \'%s\'.",
				type_tostr_plain(ltype)), binary_expr->area);

		// iterating through all struct and through all struct's members, trying to find matching
		if (ltype->spec.is_struct)
			for (size_t i = 0; i < sbuffer_len(ltype->members); i++)
				if (strcmp(ltype->members[i]->name, get_member_name(binary_expr->rexpr)) == 0)
					return type_dup(ltype->members[i]->type);

		// same logic as struct's was
		if (ltype->spec.is_union)
			for (size_t i = 0; i < sbuffer_len(ltype->members); i++)
				if (strcmp(ltype->members[i]->name, get_member_name(binary_expr->rexpr)) == 0)
					return type_dup(ltype->members[i]->type);

		report_error2(frmt("Cannot find any member with name \'%s\' in type \'%s\'.",
			binary_expr->rexpr->idnt->svalue, ltype->repr), binary_expr->area);

	//------------------------------
	// comma expr, just return right expr's type for any size of comma expr (specific parser property)
	case BINARY_COMMA:
		return type_dup(rtype);
	//------------------------------

	default:
		report_error(frmt("Unknown binary expression kind met in get_binary_expr_type(): %d",
			binary_expr->type), NULL);
	}
	// useless, only to avoid warning
	return type_dup(ltype);
}

Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table)
{
	Type* ltype = get_and_set_expr_type(ternary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(ternary_expr->rexpr, table);
	Type* ctype = get_and_set_expr_type(ternary_expr->cond, table);

	if (!IS_NUMERIC_TYPE(ctype) && !IS_POINTER_TYPE(ctype))
		if (!ctype)
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
	return unknown_type_new();
}

Type* get_and_set_expr_type(Expr* expr, Table* table)
{
	if (!expr)
		return unknown_type_new();

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
	return unknown_type_new();
}

uint32_t get_user_type_size_in_bytes(Type* type, Table* table)
{
	// todo: add alignment?
	StructDecl* type_decl = NULL;
	uint32_t size = 0, buffer = 0;
	if (type_decl = get_struct(type->repr, table))
		for (size_t i = 0; i < sbuffer_len(type_decl->struct_mmbrs); i++)
			size += get_type_size_in_bytes(
				type_decl->struct_mmbrs[i]->type, table);
	else if (is_enum_declared(type->repr, table))
		size = sizeof(int32_t);
	else if (type_decl = get_union(type->repr, table))
		for (size_t i = 0; i < sbuffer_len(type_decl->struct_mmbrs); i++)
			buffer = get_type_size_in_bytes(type_decl->struct_mmbrs[i]->type, table),
			size = max(size, buffer);
	else
		report_error(frmt("Passed type \'%s\' is not user-defined, "
			"in get_user_type_size_in_bytes()"), type_tostr_plain(type), NULL);
	return size;
}

uint32_t get_type_size_in_bytes(Type* type)
{
	assert(!type->mods.array_rank);
	if (type->mods.ptr_rank)
		return 8;

	if (!type->mods.is_predefined)
		assert(0);//return get_user_type_size_in_bytes(type, table);
	else
	{
		if (IS_U8_TYPE(type) || IS_I8_TYPE(type) || IS_CHAR_TYPE(type))
			return sizeof(int8_t);
		else if (IS_U16_TYPE(type) || IS_I16_TYPE(type))
			return sizeof(int16_t);
		else if (IS_U32_TYPE(type) || IS_I32_TYPE(type) || IS_F32_TYPE(type))
			return sizeof(int32_t);
		else if (IS_U64_TYPE(type) || IS_I64_TYPE(type) || IS_F64_TYPE(type))
			return sizeof(int64_t);
		else
			report_error(frmt("Cannot get size of \'%s\' type",
				type_tostr_plain(type)), NULL);
	}
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
	if (type && !type->spec.is_predefined)
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
		if (IS_INTEGRAL_TYPE(to) && IS_REAL_TYPE(type))
			report_warning2("Converting real type to integral type may occur data losses.",
				to->area);
	}
	return type_dup(to);
}

Type* cast_explicitly_when_const_expr(Expr* const_expr, Type* to, Type* const_expr_type)
{
	if (!to || !const_expr_type)
		report_error2("Cannot determine at least one type "
			"when trying to convert explicitly.", NULL);
	else
	{
		if (to->spec.is_void && !IS_POINTER_TYPE(to))
			report_error2("Explicit conversion to void is not allowed.", to->area);

		//---------------------------------------
		// determining the type of evaluated constant
		double value = 0.0f;
		Type* const_expr_type_new = NULL;
		if (IS_INTEGRAL_TYPE(const_expr_type) || IS_POINTER_TYPE(const_expr_type))
			const_expr_type_new = get_ivalue_type(
				(int64_t)(evaluate_expr_itype(const_expr)));
		else if (IS_REAL_TYPE(const_expr_type))
			const_expr_type_new = get_fvalue_type(
				value = evaluate_expr_ftype(const_expr));
		else
			report_error2("Cannot evaluate constant expression for explicit cast.",
				get_expr_area(const_expr));
		//---------------------------------------
		if (to->spec.is_predefined && const_expr_type_new->spec.is_predefined)
			if (to->size < get_size_of_primitive_type(const_expr_type_new))
				report_error2(frmt("Cannot explicitly convert constant value of type \'%s\' to \'%s\' (value: %f).",
					type_tostr_plain(to), type_tostr_plain(const_expr_type_new), value), to->area);
		// freeing temporary type instance (type from evaluated const expression) 
		type_free(const_expr_type_new);
		return cast_explicitly(to, const_expr_type);
	}
}

Type* cast_implicitly(Type* to, Type* type, SrcArea* area)
{
	if (can_cast_implicitly(to, type))
	{
		if (IS_POINTER_TYPE(to) && IS_POINTER_TYPE(type))
			return type_dup(to);
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) > I32))
			return type_dup(type);
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) <= I32))
			return type_dup(to);
		if ((get_type_priority(to) > I32) && IS_POINTER_TYPE(type))
			return type_dup(to);
		if ((get_type_priority(to) <= I32) && IS_POINTER_TYPE(type))
			return type_dup(type);
		
		return type_dup(get_type_priority(to) >= get_type_priority(type) ?
			to : type);
	}
	else
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(to), type_tostr_plain(type)), area);
}

Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area)
{
	if (can_cast_implicitly(to, type))
		return type_dup(to);
	else
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(to), type_tostr_plain(type)), area);
}

uint32_t can_cast_implicitly(Type* to, Type* type)
{
	if (to->spec.is_void || type->spec.is_void)
		return 0;

	// if one type is string, except second type
	if ((IS_STRING_TYPE(to) && !IS_STRING_TYPE(type)) ||
		(!IS_STRING_TYPE(to) && IS_STRING_TYPE(type)))
			return 0;

	// case of two pointers of same rank
	if (to->spec.ptr_rank && (to->spec.ptr_rank == type->spec.ptr_rank))
		return 1;

	// case when types are equal
	if (strcmp(to->repr, type->repr) == 0 &&
		!IS_POINTER_TYPE(to) && !IS_POINTER_TYPE(type))
			return 1;

	// case of pointer && integral (not pointer), and not greater than 32 bits
	if (to->spec.ptr_rank && IS_INTEGRAL_TYPE(type) && (get_type_priority(type) <= I32) && !IS_POINTER_TYPE(type))
		return 1;

	// case of integral (not pointer) && pointer, and greater equal than 32 bits
	if (type->spec.ptr_rank && IS_INTEGRAL_TYPE(to) && (get_type_priority(to) >= U32) && !IS_POINTER_TYPE(to))
		return 1;

	// types that can be casted to u8 and char
	if ((IS_U8_TYPE(to)   || IS_CHAR_TYPE(to)) &&
	    (IS_U8_TYPE(type) || IS_CHAR_TYPE(type)))
			return 1;

	// types that can be casted to i8
	if (IS_I8_TYPE(to) &&
	    IS_I8_TYPE(type))
			return 1;

	// types that can be casted to u16
	else if (IS_U16_TYPE(to) &&
		(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
	     IS_U16_TYPE(type)))
			return 1;

	// types that can be casted to i16
	else if (IS_I16_TYPE(to) &&
		(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type)))
			return 1;

	// types that can be casted to u32
	else if (IS_U32_TYPE(to) &&
		(IS_I8_TYPE(type)  || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type) || IS_U16_TYPE(type)  ||
		 IS_U32_TYPE(type)))
			return 1;

	// types that can be casted to i32
	else if (IS_I32_TYPE(to) &&
		(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
		 IS_I32_TYPE(type)))
			return 1;

	// types that can be casted to u64
	else if (IS_U64_TYPE(to) &&
		(IS_I8_TYPE(type)  || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type) || IS_U16_TYPE(type)  ||
		 IS_I32_TYPE(type) || IS_U32_TYPE(type)  ||
		 IS_U64_TYPE(type)))
			return 1;

	// types that can be casted to i64
	else if (IS_I64_TYPE(to) &&
		(IS_I8_TYPE(type) || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
		 IS_I32_TYPE(type) || IS_U32_TYPE(type) ||
		 IS_I64_TYPE(type)))
			return 1;

	// types that can be casted to f32
	else if (IS_F32_TYPE(to) &&
		(IS_I8_TYPE(type)  || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		 IS_I16_TYPE(type) || IS_U16_TYPE(type)  ||
		 IS_I32_TYPE(type) || IS_U32_TYPE(type)))
			return 1;

	// types that can be casted to f64
	else if (IS_F64_TYPE(to) &&
	   (IS_I8_TYPE(type)  || IS_CHAR_TYPE(type) || IS_U8_TYPE(type) ||
		IS_I16_TYPE(type) || IS_U16_TYPE(type)  ||
		IS_I32_TYPE(type) || IS_U32_TYPE(type)  ||
		IS_I64_TYPE(type) || IS_U64_TYPE(type)  || IS_F32_TYPE(type)))
			return 1;
	return 0;
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
	return unknown_type_new();
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
	return unknown_type_new();
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
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->enum_idnts); j++)
				if (strcmp(var, parent->enums[i]->enum_idnts[j]->svalue) == 0)
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
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->enum_idnts); j++)
				if (strcmp(member, parent->enums[i]->enum_idnts[j]->svalue) == 0)
					return get_expr_type(parent->enums[i]->enum_idnt_values[j], table);
	return unknown_type_new();
}