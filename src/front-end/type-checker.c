#include "type-checker.h"

Type* retrieve_expr_type(Expr* expr) {
	if (expr == NULL) {
		return &unknown_type;
	}
	switch (expr->kind) {
		case EXPR_IDNT:
			return expr->idnt->type;
		case EXPR_CONST:
			return expr->cnst->type;
		case EXPR_STRING:
			return expr->str->type;
		case EXPR_FUNC_CALL:
			return expr->func_call->type;
		case EXPR_FUNC_CALL2:
			return expr->func_call2->type;
		case EXPR_UNARY_EXPR:
			return expr->unary_expr->type;
		case EXPR_BINARY_EXPR:
			return expr->binary_expr->type;
		case EXPR_TERNARY_EXPR:
			return expr->ternary_expr->type;
		default:
			report_error(frmt("Unknown expression kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
	return &unknown_type;
}

Type* get_expr_type(Expr* expr, Table* table) {
	if (expr == NULL) {
		return &unknown_type;
	}
	switch (expr->kind) {
		case EXPR_IDNT:
			return get_idnt_type(expr->idnt, table);
		case EXPR_CONST:
			return get_const_type(expr->cnst);
		case EXPR_STRING:
			return get_string_type(expr->str);
		case EXPR_FUNC_CALL:
			return get_func_call_type(expr->func_call, table);
		case EXPR_FUNC_CALL2:
			return get_func_call2_type(expr->func_call2, table);
		case EXPR_UNARY_EXPR:
			return get_unary_expr_type(expr->unary_expr, table);
		case EXPR_BINARY_EXPR:
			return get_binary_expr_type(expr->binary_expr, table);
		case EXPR_TERNARY_EXPR:
			return get_ternary_expr_type(expr->ternary_expr, table);
		default:
			report_error(frmt("Unknown expression kind met",
				" in function: %s", __FUNCTION__), NULL);
	}
	return &unknown_type;
}

Type* get_const_type(Const* cnst) {
	switch (cnst->kind) {
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

Type* get_ivalue_type(int64_t value) {
	if (IN_BOUNDS_OF(INT8_MAX, INT8_MIN, value)) {
		return &i8_type;
	}
	else if (IN_BOUNDS_OF(UINT8_MAX, 0, value)) {
		return &u8_type;
	}
	else if (IN_BOUNDS_OF(INT16_MAX, INT16_MIN, value)) {
		return &i16_type;
	}
	else if (IN_BOUNDS_OF(UINT16_MAX, 0, value)) {
		return &u16_type;
	}
	else if (IN_BOUNDS_OF(INT32_MAX, INT32_MIN, value)) {
		return &i32_type;
	}
	else if (IN_BOUNDS_OF(UINT32_MAX, 0, value)) {
		return &u32_type;
	}
	else if (IN_BOUNDS_OF(INT64_MAX, INT64_MIN, value)) {
		return &i64_type;
	}
	else {
		return &u64_type;
	}
}

Type* get_fvalue_type(double value) {
	return IN_BOUNDS_OF(FLT_MAX, FLT_MIN, value) ?
		&f32_type : &f64_type;
}

Type* get_idnt_type(Idnt* idnt, Table* table) {
	if (idnt->attrs.is_enum_member) {
		return retrieve_expr_type(idnt->attrs.enum_member->value);
	}
	TableEntity* varent = get_variable(idnt->svalue, table);
	TableEntity* parament = get_parameter(idnt->svalue, table);
	if (!varent && !parament) {
		TableEntity* funcent = get_function(idnt->svalue, table);
		if (funcent != NULL) {
			return funcent->value.function->type;
		}
		return &unknown_type;
	}
	if (varent && parament) {
		report_error(frmt("Bug with type identification met"
			" in function: %s.", __FUNCTION__), NULL);
	}
	
	return varent ? varent->value.local->type_var->type :
		parament->value.parameter->type;
}

Type* get_string_type(Str* str) {
	return pointer_type_new(&char_type);
}

Type* get_func_call_type(FuncCall* func_call, Table* table) {
	// also checking type of each function's argument with type of passed value
	FuncDecl* origin = func_call->decl;
	for (size_t i = 0; i < sbuffer_len(origin->params); i++) {
		cast_implicitly(origin->params[i]->type, 
			get_expr_type(func_call->args[i], table),
				get_expr_area(func_call->args[i]));
	}
	return origin->type->attrs.func.ret;
}

Type* get_func_call2_type(FuncCall2* func_call2, Table* table) {
	Type* rexpr_type = get_expr_type(func_call2->rexpr, table);
	if (!is_function_type(rexpr_type)) {
		report_error2("Cannot call expression of non-function type.", 
			get_expr_area(func_call2->rexpr));
	}
	Type** params = rexpr_type->attrs.func.params;
	for (size_t i = 0; i < sbuffer_len(params); i++) {
		cast_implicitly(params[i], 
			get_expr_type(func_call2->args[i], table),
				get_expr_area(func_call2->args[i]));
	}
	return rexpr_type->attrs.func.ret;
}

Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table) {
	Type* type = get_and_set_expr_type(unary_expr->expr, table);

	switch (unary_expr->kind) {
		//-----------------------------
		// operators that can be applied with numeric types & pointers
		case UNARY_PLUS:
		case UNARY_MINUS:
		case UNARY_LG_NOT:
			if (is_numeric_type(type)) {
				return type;
			}
			report_error2("Cannot use this operator with this operand type.",
				unary_expr->area);

		//-----------------------------
		// operators that can be applied only with integral types & pointers
		case UNARY_BW_NOT:
			if (is_integral_type(type)) {
				return type;
			}
			report_error2("Cannot use this operator with this operand type.",
				unary_expr->area);

		//-----------------------------
		// addresible cases
		case UNARY_ADDRESS:
			if (type) {
				return address_type(type);
			}
			report_error2("Cannot determine the type of unary expression " 
				"while taking an address.", unary_expr->area);
		case UNARY_DEREFERENCE:
			if (is_pointer_like_type(type)) {
				return dereference_type(type);
			}
			report_error2(frmt("Cannot dereference value of type \'%s\'.",
				type_tostr_plain(type)), unary_expr->area);
		//-----------------------------

		case UNARY_CAST:
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

Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table) {
	Type* ltype = get_and_set_expr_type(
		binary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(
		binary_expr->rexpr, table);

	switch (binary_expr->kind) {
		//-----------------------------
		// cases with relative operators, which must return i32 type in any situation

		// with any type
		case BINARY_LG_EQ:
		case BINARY_LG_NEQ:
			if (can_cast_implicitly(ltype, rtype) ||
				can_cast_implicitly(rtype, ltype)) {
				return &i32_type;
			}
			report_error2("Cannot use this operator with these operand types.",
				binary_expr->area);

		// with numeric type
		case BINARY_LESS_THAN:
		case BINARY_GREATER_THAN:
		case BINARY_LESS_EQ_THAN:
		case BINARY_GREATER_EQ_THAN:
			if (is_one_action(ltype, rtype, is_numeric_type) ||
				is_one_action(ltype, rtype, is_pointer_like_type)) {
				if (can_cast_implicitly(ltype, rtype) ||
					can_cast_implicitly(rtype, ltype)) {
					return &i32_type;
				}
			}
			report_error2("Cannot use this operator with these operand types.",
				binary_expr->area);

		//-----------------------------
		// assign operators need only one check for implicit cast (to left assignable type) 
		// in case of common assign, any type can be
		case BINARY_ASSIGN:
			return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);

		// assign operators that needs numeric or pointer types
		case BINARY_ADD_ASSIGN:
		case BINARY_SUB_ASSIGN:
		case BINARY_DIV_ASSIGN:
		case BINARY_MUL_ASSIGN:
			if (is_both_action(ltype, rtype, is_numeric_type)) {
				return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);
			}
			report_error2("Cannot use this operator with these operand types.", 
				binary_expr->area);

		// assign operators that needs integral or pointer type
		case BINARY_MOD_ASSIGN:
		case BINARY_BW_OR_ASSIGN:
		case BINARY_BW_AND_ASSIGN:
		case BINARY_BW_XOR_ASSIGN:
		case BINARY_LSHIFT_ASSIGN:
		case BINARY_RSHIFT_ASSIGN:		
			if (is_both_action(ltype, rtype, is_integral_type)) {
				return cast_implicitly_when_assign(ltype, rtype, binary_expr->area);
			}
			report_error2("Cannot use this operator with these operand types.", 
				binary_expr->area);

		//-----------------------------
		// operator that can be applied with strings
		case BINARY_ADD:
			return can_cast_implicitly(rtype, ltype) ?
				cast_implicitly(rtype, ltype, binary_expr->area) : 
				cast_implicitly(ltype, rtype, binary_expr->area);
			report_error2("Cannot use this operator with these operand types.",
				binary_expr->area);

		//-----------------------------
		// operators that can be applied with numeric type & pointers
		case BINARY_SUB:
		case BINARY_DIV:
		case BINARY_MULT:
		case BINARY_LG_OR:
		case BINARY_LG_AND:
			if (is_both_action(ltype, rtype, is_numeric_type)) {
				return can_cast_implicitly(rtype, ltype) ? 
					cast_implicitly(rtype, ltype, binary_expr->area) : 
					cast_implicitly(ltype, rtype, binary_expr->area);
			}
			report_error2("Cannot use this operator with these operand types.", 
				binary_expr->area);
		
		//-----------------------------
		// operators that can be applied only with integral types
		case BINARY_MOD:
		case BINARY_BW_OR:
		case BINARY_BW_AND:
		case BINARY_BW_XOR:
		case BINARY_LSHIFT:
		case BINARY_RSHIFT:
			if (is_both_action(ltype, rtype, is_integral_type)) {
				return can_cast_implicitly(rtype, ltype) ?
					cast_implicitly(rtype, ltype, binary_expr->area) :
					cast_implicitly(ltype, rtype, binary_expr->area);
			}
			report_error2("Cannot use this operator with these operand types.",
				binary_expr->area);

		//------------------------------
		// accessors
		case BINARY_ARR_MEMBER_ACCESSOR:
			if (!is_pointer_like_type(ltype)) {
				report_error2(frmt("Cannot access element of base type\'%s\' like array element.",
					type_tostr_plain(ltype)), binary_expr->area);
			}
			// index of an array should be an integral type
			if (!is_integral_type(rtype)) {
				report_error2(frmt("Index should be value of integral type, met: \'%s\'",
					type_tostr_plain(rtype)), binary_expr->area);
			}
			return dereference_type(ltype);

		case BINARY_PTR_MEMBER_ACCESSOR:
			if (!is_pointer_like_type(ltype)) {
				report_error2(frmt("Cannot access member of non pointer type \'%s\'.",
					type_tostr_plain(ltype)), binary_expr->area);
			}
			// logic of pointer and common member accessor are the same except this primary condition
			ltype = ltype->base;

		case BINARY_MEMBER_ACCESSOR:
			if (is_pointer_like_type(ltype)) {
				report_error2(frmt("Cannot access member of pointer-like type \'%s\'.",
					type_tostr_plain(ltype)), binary_expr->area);
			}
			// condition for both common and pointer accessors
			// checks if the left expression's type is not predefined simple type
			if (is_primitive_type(ltype)) {
				report_error2(frmt("Cannot access member of primitive type \'%s\'.",
					type_tostr_plain(ltype)), binary_expr->area);
			}

			// iterating through all struct's members, trying to find matching
			if (is_struct_or_union_type(ltype)) {
				Member** members = ltype->attrs.cmpd.members;
				for (size_t i = 0; i < sbuffer_len(members); i++) {
					Idnt* member = get_member_idnt(binary_expr->rexpr);
					if (str_eq(members[i]->name, member->svalue)) {
						return member->type = members[i]->type;
					}
				}
			}

			report_error2(frmt("Cannot find any member with name \'%s\' in type \'%s\'.",
				binary_expr->rexpr->idnt->svalue, ltype->repr), binary_expr->area);

		//------------------------------
		// comma expr, just return right expr's type for any size of comma expr (specific parser property)
		case BINARY_COMMA:
			return rtype;
		//------------------------------

		default:
			report_error(frmt("Unknown binary expression kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
	// useless, only to avoid warning
	return ltype;
}

Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table) {
	Type* ltype = get_and_set_expr_type(ternary_expr->lexpr, table);
	Type* rtype = get_and_set_expr_type(ternary_expr->rexpr, table);
	Type* ctype = get_and_set_expr_type(ternary_expr->cond, table);

	if (!is_numeric_type(ctype) && !is_pointer_like_type(ctype)) {
		report_error2(frmt("Expected numeric or pointer-like type in "
			"condition of ternary expression, type met: \'%s\'",
				type_tostr_plain(ctype)), get_expr_area(ternary_expr->cond));
	}
	if (can_cast_implicitly(ltype, rtype)) {
		return cast_implicitly(ltype, rtype, ternary_expr->area);
	}
	if (can_cast_implicitly(rtype, ltype)) {
		return cast_implicitly(rtype, ltype, ternary_expr->area);
	}
	report_error2("Left and right expressions in ternary expression "
		"should be the same or implicitly equal.", ternary_expr->area);
	return &unknown_type;
}

Type* get_and_set_expr_type(Expr* expr, Table* table) {
	if (expr == NULL) {
		return &unknown_type;
	}
	Type* type = get_expr_type(expr, table);

	switch (expr->kind) {
		case EXPR_IDNT:
			return expr->idnt->type = type;
		case EXPR_CONST:
			return expr->cnst->type = type;
		case EXPR_STRING:
			return expr->str->type = type;
		case EXPR_FUNC_CALL:
			return expr->func_call->type = type;
		case EXPR_FUNC_CALL2:
			return expr->func_call2->type = type;
		case EXPR_UNARY_EXPR:
			return expr->unary_expr->type = type;
		case EXPR_BINARY_EXPR:
			return expr->binary_expr->type = type;
		case EXPR_TERNARY_EXPR:
			return expr->ternary_expr->type = type;
		default:
			report_error(frmt("Unknown expression kind met, "
				"in function %s", __FUNCTION__), NULL);
	}
	return &unknown_type;
}

Type* get_spec_binary_type(BinaryExpr* expr) {
	Type* lexpr = retrieve_expr_type(expr->lexpr);
	Type* rexpr = retrieve_expr_type(expr->rexpr);
	return can_cast_implicitly(lexpr, rexpr) ?
		cast_implicitly(lexpr, rexpr, NULL) : 
		cast_implicitly(rexpr, lexpr, NULL);
}

uint32_t get_type_priority(Type* type) {
	if (is_u8_type(type)) {
		return U8_TYPE_PRIORITY;
	}
	if (is_i8_type(type)) {
		return I8_TYPE_PRIORITY;
	}
	if (is_char_type(type)) {
		return CHAR_TYPE_PRIORITY;
	}
	if (is_u16_type(type)) {
		return U16_TYPE_PRIORITY;
	}
	if (is_i16_type(type)) {
		return I16_TYPE_PRIORITY;
	}
	if (is_u32_type(type)) {
		return U32_TYPE_PRIORITY;
	}
	if (is_i32_type(type)) {
		return I32_TYPE_PRIORITY;
	}
	if (is_u64_type(type)) {
		return U64_TYPE_PRIORITY;
	}
	if (is_i64_type(type)) {
		return I64_TYPE_PRIORITY;
	}
	if (is_f32_type(type)) {
		return F32_TYPE_PRIORITY;
	}
	if (is_f64_type(type)) {
		return F64_TYPE_PRIORITY;
	}
	if (is_void_type(type)) {
		return VOID_TYPE_PRIORITY;
	}
	if (type && !is_primitive_type(type)) {
		return I32_TYPE_PRIORITY;
	}
	return 0x0;
}

Type* cast_explicitly(Type* to, Type* type) {
	if (is_void_type(to)) {
		report_error2("Cannot cast to void type.", to->area);
	}
	if (is_void_type(type)) {
		report_error2("Cannot cast expression of void type.", to->area);
	}
	return to;
}

Type* cast_implicitly(Type* to, Type* type, SrcArea* area) {
	if (!can_cast_implicitly(to, type)) {
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(type), type_tostr_plain(to)), area);
	}
	else {
		// if both are pointer-like
		if (is_one_action(to, type, is_pointer_type) &&
			is_one_action(to, type, is_array_type)) {
				return is_pointer_type(to) ? to : type;
		}
		// if one is enum and second is integral
		if (is_one_action(to, type, is_enum_type) &&
			is_one_action(to, type, is_integral_type)) {
				return is_integral_type(to) ? to : type;
		}

		if (is_one_action(to, type, is_integral_type) ||
			is_one_action(to, type, is_pointer_like_type)) {
			if (is_pointer_like_type(to)) {
				return type->size > i32_type.size ? type : to;
			}
			if (is_pointer_like_type(type)) {
				return to->size > i32_type.size ? to : type;
			}
		}

		// if both are function types
		if (is_both_action(to, type, is_function_type)) {
			return type;
		}

		// if both are primitive types
		if (is_both(to, type, TYPE_PRIMITIVE)) {
			return get_type_priority(to) >= get_type_priority(type) ? to : type;
		}
	}
	return NULL;
}

Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area) {
	if (can_cast_implicitly(to, type)) {
		return to;
	}
	else {
		report_error2(frmt("Cannot implicitly convert type \'%s\' to \'%s\'",
			type_tostr_plain(to), type_tostr_plain(type)), area);
	}
}

bool can_cast_enum_type(Type* to, Type* type) {
	if (is_both(to, type, TYPE_ENUM)) {
		return true;
	}
	if (is_enum_type(to) && is_integral_type(type)) {
		if (type->size <= i32_type.size) {
			return true;
		}
	}
	return false;
}

bool can_cast_intptr_types(Type* to, Type* type) {
	if (is_integral_type(to)) {
		if (to->size >= i32_type.size) {
			return true;
		}
	}
	if (is_pointer_like_type(to)) {
		if (type->size <= 4) {
			return true;
		}
	}
	return false;
}

bool can_cast_pointer_types(Type* to, Type* type) {
	if (get_pointer_rank(to) == get_pointer_rank(type)) {
		// if we trying to cast pointer type with same rank 
		// to void pointer, return true, because void pointer is
		// kind-of generic type for pointers
		if (get_base_type(to)->kind == TYPE_VOID) {
			return true;
		}
		// and also if their base types can be casted implicitly
		if (can_cast_implicitly(get_base_type(to), get_base_type(type))) {
			return true;
		}
	}
	return false;
}

bool can_cast_function_types(Type* to, Type* type) {
	if (sbuffer_len(to->attrs.func.params) != sbuffer_len(type->attrs.func.params)) {
		return false;
	}
	// try cast their return types
	if (!can_cast_implicitly(to->attrs.func.ret, type->attrs.func.ret)) {
		return false;
	}
	// try to cast their each parameter's type
	for (size_t i = 0; i < sbuffer_len(to->attrs.func.params); i++) {
		Type* targ_type = to->attrs.func.params[i];
		Type* cast_type = type->attrs.func.params[i];
		if (!can_cast_implicitly(targ_type, cast_type)) {
			return false;
		}
	}
	return true;
}

bool can_cast_implicitly(Type* to, Type* type) {
	// trying to check if at least one of passed types 
	// is incomplete, pure void, or unknown.
	// In this case cast is impossible.
	if (is_one(to, type, TYPE_VOID)    ||
		is_one(to, type, TYPE_UNKNOWN) ||
		is_one(to, type, TYPE_INCOMPLETE)) {
			return false;
	}
	// trying to cast if both passed types are pointer-like types
	//		- pointers, arrays, [function types ?]
	if (is_both_action(to, type, is_pointer_like_type)) {
		return can_cast_pointer_types(to, type);
	}
	// trying to cast if at least one type is enum type
	// it needs separate processing because it behaves like simple
	// integral type
	if (is_one(to, type, TYPE_ENUM)) {
		return can_cast_enum_type(to, type);
	}
	// trying to cast if one type is integral type,
	// and second type is pointer
	if (is_one_action(to, type, is_integral_type) &&
		is_one_action(to, type, is_pointer_like_type)) {
			return can_cast_intptr_types(to, type);
	}

	// trying to cast if both types are func types
	if (is_both_action(to, type, is_function_type)) {
		return can_cast_function_types(to, type);
	}

	// trying to cast if both types are just primitive
	if (is_both(to, type, TYPE_PRIMITIVE)) {
		// types that can be casted to u8 and char
		if ((is_u8_type(to)   || is_char_type(to)) &&
			(is_u8_type(type) || is_char_type(type))) {
				return true;
		}

		// types that can be casted to i8
		if (is_i8_type(to) && is_i8_type(type)) {
			return true;
		}

		// types that can be casted to u16
		else if (is_u16_type(to) &&
			(is_i8_type(type) || is_char_type(type) ||
			 is_u8_type(type) || is_u16_type(type))) {
				return true;
		}

		// types that can be casted to i16
		else if (is_i16_type(to) &&
			(is_i8_type(type) || is_char_type(type) || 
			 is_u8_type(type) || is_i16_type(type))) {
				return true;
		}

		// types that can be casted to u32
		else if (is_u32_type(to) &&
			(is_i8_type(type)  || is_char_type(type) || is_u8_type(type) ||
			 is_i16_type(type) || is_u16_type(type)  || is_u32_type(type))) {
				return true;
		}

		// types that can be casted to i32
		else if (is_i32_type(to) &&
			(is_i8_type(type)  || is_char_type(type) || is_u8_type(type) ||
		     is_i16_type(type) || is_u16_type(type)  || is_i32_type(type))) {
				return true;
		}

		// types that can be casted to u64
		else if (is_u64_type(to) &&
			(is_i8_type(type)  || is_char_type(type) || is_u8_type(type) ||
			 is_i16_type(type) || is_u16_type(type)  ||
			 is_i32_type(type) || is_u32_type(type)  || is_u64_type(type))) {
				return true;
		}

		// types that can be casted to i64
		else if (is_i64_type(to) &&
			(is_i8_type(type)  || is_char_type(type) || is_u8_type(type)  ||
			 is_i16_type(type) || is_u16_type(type)  || is_i32_type(type) || 
			 is_u32_type(type) || is_i64_type(type))) {
				return true;
		}

		// types that can be casted to f32
		else if (is_f32_type(to) &&
			(is_i8_type(type) || is_char_type(type) || is_u8_type(type) ||
			is_i16_type(type) || is_u16_type(type)  ||
			is_i32_type(type) || is_u32_type(type)  || is_f32_type(type))) {
				return true;
		}

		// types that can be casted to f64
		else if (is_f64_type(to) &&
			(is_i8_type(type) || is_char_type(type) || is_u8_type(type) ||
			is_i16_type(type) || is_u16_type(type) ||
			is_i32_type(type) || is_u32_type(type) ||
			is_i64_type(type) || is_u64_type(type) || is_f32_type(type))) {
				return true;
		}
	}
	// if both types are equal, no need for 
	// any additional processing.
	if (str_eq(to->repr, type->repr)) {
		return true;
	}

	return false;
}

SrcArea* get_expr_area(Expr* expr) {
	switch (expr->kind) {
		case EXPR_IDNT:
			return src_area_new(expr->idnt->context, NULL);
		case EXPR_CONST:
			return src_area_new(expr->cnst->context, NULL);
		case EXPR_STRING:
			return src_area_new(expr->str->context, NULL);
		case EXPR_FUNC_CALL:
			return expr->func_call->area;
		case EXPR_FUNC_CALL2:
			return expr->func_call2->area;
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
	return NULL;
}

const char* get_member_name(Expr* expr) {
	Idnt* idnt = get_member_idnt(expr);
	return idnt ? idnt->svalue : NULL;
}

Idnt* get_member_idnt(Expr* expr) {
	// function is needed espesially for recognizing the member's name in accessor expression
	// i mean that we are not fully knew that member will be represented as Idnt
	// it can be ...->a[i], ...->a.a-> ...
	switch (expr->kind) {
		case EXPR_IDNT:
			return expr->idnt;
		case EXPR_BINARY_EXPR:
			switch (expr->binary_expr->kind) {
				case BINARY_MEMBER_ACCESSOR:
				case BINARY_PTR_MEMBER_ACCESSOR:
				case BINARY_ARR_MEMBER_ACCESSOR:
					return get_member_idnt(expr->binary_expr->lexpr);
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
	return NULL;
}