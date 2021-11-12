#include "type-checker.h"

#define type_dup(type, type_name)                 \
	Type* type_name = cnew_s(Type, type_name, 1); \
	type_name->mods = type->mods;                 \
	type_name->repr = _strdup(type->repr)

#define type_dup_no_alloc(type, type_name)   \
	type_name = cnew_s(Type, type_name, 1);  \
	type_name->mods = type->mods;            \
	type_name->repr = _strdup(type->repr)

Type* get_expr_type(Expr* expr, Table* table)
{
#define set_and_return_type(type, to_node) \
	if (!type) return NULL;                \
	type_dup_no_alloc(type, type_new);     \
	to_node->type = type_new;              \
	return type_new

	Type* type = NULL;
	Type* type_new = NULL;

	if (!expr)
		return NULL;

	switch (expr->kind)
	{
	case EXPR_IDNT:
		type = get_idnt_type(expr->idnt, table);
		set_and_return_type(type, expr->idnt);
	case EXPR_CONST:
		type = get_const_type(expr->cnst);
		set_and_return_type(type, expr->cnst);
	case EXPR_STRING:
		type = get_string_type(expr->str);
		set_and_return_type(type, expr->str);
	case EXPR_UNARY_EXPR:
		type = get_unary_expr_type(expr->unary_expr, table);
		set_and_return_type(type, expr->unary_expr);
	case EXPR_BINARY_EXPR:
		type = get_binary_expr_type(expr->unary_expr, table);
		set_and_return_type(type, expr->binary_expr);
	case EXPR_TERNARY_EXPR:
		type = get_ternary_expr_type(expr->ternary_expr, table);
		set_and_return_type(type, expr->ternary_expr);
	default:
		report_error(frmt("Unknown expression kind met: %d",
			expr->kind), NULL);
	}
	return NULL;
}

Type* get_const_type(Const* cnst)
{
#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

	Type* type = cnew_s(Type, type, 1);
	type->mods.is_predefined = 1;

	switch (cnst->kind)
	{
	case CONST_INT:
		if (IN_BOUNDS_OF(INT8_MAX, INT8_MIN, cnst->ivalue))
			type->repr = "i8";
		else if (IN_BOUNDS_OF(INT16_MAX, INT16_MIN, cnst->ivalue))
			type->repr = "i16";
		else if (IN_BOUNDS_OF(INT32_MAX, INT32_MIN, cnst->ivalue))
			type->repr = "i32";
		else
			type->repr = "i64";
		break;
	case CONST_UINT:
		if (IN_BOUNDS_OF(UINT8_MAX, 0, cnst->uvalue))
			type->repr = "u8";
		else if (IN_BOUNDS_OF(UINT16_MAX, 0, cnst->uvalue))
			type->repr = "u16";
		else if (IN_BOUNDS_OF(UINT32_MAX, 0, cnst->uvalue))
			type->repr = "u32";
		else
			type->repr = "u64";
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
	type->repr = "str";
	return type;
}

Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table)
{
	Type* type = get_expr_type(unary_expr->expr, table);

	// this type needed to handle sizeof(idnt) ambiguity
	Type* szof_type = NULL;
	// this type needed to handle dereference, address exprs etc.
	Type* type_new = NULL;

	switch (unary_expr->kind)
	{
	//-----------------------------
	// operators that can be applied with numeric types & pointers
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
		if (IS_NUMERIC_TYPE(type) || IS_POINTER_TYPE(type))
			return type;
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);
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
		report_error2("Cannot use this operator with this operand type.",
			unary_expr->area);
	//-----------------------------

	//-----------------------------
	// addresible cases
	case UNARY_ADDRESS:
		if (!type)
			report_error2("Cannot determine the type of unary expression while taking an address.",
				unary_expr->area);
		else
		{
			// setting the new type based on extracted type, with is_ptr changed
			type_dup_no_alloc(type, type_new);
			type_new->mods.is_ptr += 1;
			return type_new;
		}
	case UNARY_DEREFERENCE:
		if (!type)
			report_error2("Cannot determine the type of unary expression while dereferencing it.",
				unary_expr->area);
		if (!IS_POINTER_TYPE(type))
			report_error2(frmt("Cannot dereference value of type %s.",
				type_tostr_plain(type)), unary_expr->area);
		else
		{
			type_dup_no_alloc(type, type_new);
			type_new->mods.is_ptr -= 1;
			return type_new;
		}
	//-----------------------------

	case UNARY_CAST:
		if (unary_expr->expr->kind == EXPR_CONST)
			cast_explicitly_when_const(unary_expr->cast_type, type);
		return cast_explicitly(unary_expr->cast_type, type);
	case UNARY_SIZEOF:
		if (!unary_expr->cast_type)
		{
			// handling type of expr of sizeof
			if (unary_expr->expr && type)
				if (!IS_NUMERIC_TYPE(type) && !IS_POINTER_TYPE(type))
					report_error2(frmt("sizeof accepts only numeric or pointer type of expession, met %s.",
						type_tostr_plain(type)), unary_expr->area);

			// if sizeof has idnt in expr, its can be simple variable or user-type, need to check it
			if (unary_expr->expr && unary_expr->expr->kind == EXPR_IDNT)
				if (get_struct(unary_expr->expr->idnt->svalue, table) ||
					get_union(unary_expr->expr->idnt->svalue, table))
				{
					szof_type = cnew_s(Type, szof_type, 1);
					szof_type->repr = unary_expr->expr->idnt->svalue;
					unary_expr->cast_type = szof_type;
				}
		}

		// this case appears when sizeof(type)
		// in this case expression in null -> expression type is null
		// need to allocate it
		type_new = cnew_s(Type, type_new, 1);
		type_new->repr = "u32";
		return type_new;
	default:
		report_error(frmt("Unknown unary expression kind met: %d.",
			unary_expr->type), NULL);
	}
	return type;
}

Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table)
{
	Type* ltype = get_expr_type(
		binary_expr->lexpr, table);
	Type* rtype = get_expr_type(
		binary_expr->rexpr, table);

	// type needed to handle the array member accessor expression
	Type* type_new = NULL;

	// variable needed for containing possible value of struct or union (see accessor's part)
	void* type_decl_stmt = NULL; 

	switch (binary_expr->kind)
	{
	//-----------------------------
	// assign operators need only one check for implicit cast (to left assignable type) 

	// in case of common assign, any type can be
	case BINARY_ASSIGN:
	case BINARY_ADD_ASSIGN:
		return cast_implicitly_when_assign(ltype, rtype);

	// assign operators that needs numeric or pointer types
	case BINARY_SUB_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_MUL_ASSIGN:
		if ((IS_NUMERIC_TYPE(ltype) || IS_POINTER_TYPE(ltype)) &&
			(IS_NUMERIC_TYPE(rtype) || IS_POINTER_TYPE(rtype)))
				return cast_implicitly_when_assign(ltype, rtype);
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
				return cast_implicitly_when_assign(ltype, rtype);
		report_error2("Cannot use this operator with this operand types.", 
			binary_expr->area);
	//-----------------------------


	//-----------------------------
	// operators that can be applied with strings
	case BINARY_ADD:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
		return can_cast_implicitly(rtype, ltype) ?
			cast_implicitly(rtype, ltype) : cast_implicitly(ltype, rtype);
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);
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
		report_error2("Cannot use this operator with this operand types.", 
			binary_expr->area);
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
		report_error2("Cannot use this operator with this operand types.",
			binary_expr->area);
	//-----------------------------

	//------------------------------
	// accessors
	case BINARY_ARR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))			
			report_error2(frmt("Cannot access array element value of type %s, pointer type expected.",
				type_tostr_plain(ltype)), binary_expr->area);
		// index of an array should be an integral type
		if (!IS_INTEGRAL_TYPE(rtype))
			report_error2(frmt("Index should be value of integral type. Type %s met.",
				type_tostr_plain(rtype)), binary_expr->area);
		type_dup_no_alloc(ltype, type_new);
		type_new->mods.is_ptr -= 1;
		return type_new;

	case BINARY_PTR_MEMBER_ACCESSOR:
		if (!IS_POINTER_TYPE(ltype))
			report_error2(frmt("Cannot access member of non-pointer type %s.",
				type_tostr_plain(ltype)), binary_expr->area);
		// logic of pointer and common member accessor are the same except this primary condition
		goto find_matching_member_type;

	case BINARY_MEMBER_ACCESSOR:
		if (IS_POINTER_TYPE(ltype))
			report_error2(frmt("Cannot access member of pointer type %s.",
				type_tostr_plain(ltype)), binary_expr->area);
		//~~~~~~~~~~~~~~~~~~~~~~~
		find_matching_member_type:
		//~~~~~~~~~~~~~~~~~~~~~~~

		// condition for both common and pointer accessors
		// checks if the left expression's type is not predefined simple type
		if (ltype->mods.is_predefined)
			report_error2(frmt("Cannot access member of non-user defined type %s.",
				type_tostr_plain(ltype)), binary_expr->area);

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

		report_error2(frmt("Cannot find any member with name %s in type %s.",
			binary_expr->rexpr->idnt->svalue, type_tostr_plain(ltype)), binary_expr->area);
		break;
	//------------------------------

	//------------------------------
	// comma expr, just return right expr's type for any size of comma expr (specific parser property)
	case BINARY_COMMA:
		return rtype;
	//------------------------------


	default:
		report_error(frmt("Unknown binary expression kind met: %d",
			binary_expr->type), NULL);
	}
	// useless, only to avoid warning
	return ltype;
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
	return NULL;
}

Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table)
{
	Type* ltype = get_expr_type(ternary_expr->lexpr, table);
	Type* rtype = get_expr_type(ternary_expr->rexpr, table);
	Type* ctype = get_expr_type(ternary_expr->cond, table);

	if (!IS_NUMERIC_TYPE(ctype) && !IS_POINTER_TYPE(ctype))
		if (!ctype)
			report_error2("Cannot determine the type of condition in ternary expression.", 
				get_expr_area(ternary_expr->cond));
		else 
			report_error2(frmt("Expected numeric or pointer type in condition of ternary expression, type met: %s",
				type_tostr_plain(ctype)), get_expr_area(ternary_expr->cond));

	if (can_cast_implicitly(ltype, rtype))
		return cast_implicitly(ltype, rtype);
	else if (can_cast_implicitly(rtype, ltype))
		return cast_implicitly(rtype, ltype);
	else
		report_error2("Left and right expressions in ternary expression should be the same or implicitly equal", 
			ternary_expr->area);
	return NULL;
}

uint32_t get_type_size_in_bytes(Type* type)
{
	if (type->mods.is_ptr)
		return 4;

	if (type->mods.is_predefined)
	{
		if (IS_U8(type) || IS_I8(type) || IS_CHAR(type))
			return 1;
		else if (IS_U16(type) || IS_I16(type))
			return 2;
		else if (IS_U32(type) || IS_I32(type) || IS_F32(type))
			return 4;
		else if (IS_U64(type) || IS_I64(type) || IS_F64(type))
			return 8;
		else
			report_error("Cannot get size of %s type", type_tostr_plain(type));
	}
	else
		//todo: add type size for user types
		assert(0);
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
	if (IS_STRING_TYPE(type))
		return STR;
	if (IS_VOID(type))
		return VOID;
	return -0xA;
}

Type* cast_explicitly(Type* to, Type* type)
{
	if (!to || !type)
		report_error2("Cannot determine at least one type when trying to convert explicitly.", NULL);
	else
	{
		if (to->mods.is_void && !IS_POINTER_TYPE(to))
			report_error2("Explicit conversion to void is not allowed.", to->area);
		if (IS_STRING_TYPE(type) && !IS_STRING_TYPE(to) && !IS_CHAR_POINTER_TYPE(to))
			report_error2(frmt("Cannot explicitly convert type %s to %s.",
				type_tostr_plain(type), type_tostr_plain(to)), to->area);
		if (IS_INTEGRAL_TYPE(to) && IS_REAL_TYPE(type))
			report_warning2("Converting real type to integral type may occur data losses.",
				to->area);
	}
	return to;
}

Type* cast_explicitly_when_const(Type* to, Type* const_type)
{
	if (!to || !const_type)
		report_error2("Cannot determine at least one type when trying to convert explicitly.", NULL);
	else
	{
		if (to->mods.is_void && !IS_POINTER_TYPE(to))
			report_error2("Explicit conversion to void is not allowed.", to->area);
		if (to->mods.is_predefined && const_type->mods.is_predefined)
			if (get_type_size_in_bytes(to) < get_type_size_in_bytes(const_type))
				report_error2(frmt("Cannot explicitly convert constant value of type %s to %s.",
					type_tostr_plain(to), type_tostr_plain(const_type)), to->area);
		return cast_explicitly(to, const_type);
	}
}

Type* cast_implicitly(Type* to, Type* type)
{
	//-------------------------------
	// creating i32 type needed for certain cases
	Type* i32_type = cnew_s(Type, i32_type, 1);
	i32_type->mods.is_predefined = 1;
	i32_type->repr = "i32";
	//-------------------------------

	if (can_cast_implicitly(to, type))
	{
		if (IS_POINTER_TYPE(to) && IS_POINTER_TYPE(type))
			return i32_type;
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) >= U32))
			return type_free(i32_type), type;
		if (IS_POINTER_TYPE(to) && (get_type_priority(type) < U32))
			return i32_type;
		if ((get_type_priority(to) >= U32) && IS_POINTER_TYPE(type))
			return type_free(i32_type), to;
		if ((get_type_priority(to) < U32) && IS_POINTER_TYPE(type))
			return i32_type;

		type_free(i32_type);
		return get_type_priority(to) >= get_type_priority(type) ?
			to : type;
	}
	else
		report_error2(frmt("Cannot implicitly convert type %s to %s",
			type_tostr_plain(to), type_tostr_plain(type)), to->area);
}

Type* cast_implicitly_when_assign(Type* to, Type* type)
{
	if (can_cast_implicitly(to, type))
		return to;
	else
		report_error2(frmt("Cannot implicitly convert type %s to %s",
			type_tostr_plain(to), type_tostr_plain(type)), to->area);
}

uint32_t can_cast_implicitly(Type* to, Type* type)
{
	if (to->mods.is_void || type->mods.is_void)
		return 0;

	// if one type is string, except second type
	if ((IS_STRING_TYPE(to) && !IS_STRING_TYPE(type)) ||
		(!IS_STRING_TYPE(to) && IS_STRING_TYPE(type)))
			return 0;

	// case of two pointers of same rank
	if (to->mods.is_ptr && (to->mods.is_ptr == type->mods.is_ptr))
		return 1;

	// case when types are equal
	if (strcmp(to->repr, type->repr) == 0 &&
		!IS_POINTER_TYPE(to) && !IS_POINTER_TYPE(type))
			return 1;

	// case of pointer && integral (not pointer), and not greater than 32 bits
	if (to->mods.is_ptr && IS_INTEGRAL_TYPE(type) && (get_type_priority(type) <= I32) && !IS_POINTER_TYPE(type))
		return 1;

	// case of integral (not pointer) && pointer, and greater equal than 32 bits
	if (type->mods.is_ptr && IS_INTEGRAL_TYPE(to) && (get_type_priority(to) >= U32) && !IS_POINTER_TYPE(to))
		return 1;

	// types that can be casted to u8 and char
	if ((IS_U8(to)   || IS_CHAR(to)) &&
	    (IS_U8(type) || IS_CHAR(type)))
			return 1;

	// types that can be casted to i8
	if (IS_I8(to) &&
	    IS_I8(type))
			return 1;

	// types that can be casted to u16
	else if (IS_U16(to) &&
		(IS_I8(type) || IS_CHAR(type) || IS_U8(type) ||
	     IS_U16(type)))
			return 1;

	// types that can be casted to i16
	else if (IS_I16(to) &&
		(IS_I8(type) || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type)))
			return 1;

	// types that can be casted to u32
	else if (IS_U32(to) &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_U32(type)))
			return 1;

	// types that can be casted to i32
	else if (IS_I32(to) &&
		(IS_I8(type) || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type) ||
		 IS_I32(type)))
			return 1;

	// types that can be casted to u64
	else if (IS_U64(to) &&
		(IS_I8(type)  || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type)  ||
		 IS_I32(type) || IS_U32(type)  ||
		 IS_U64(type)))
			return 1;

	// types that can be casted to i64
	else if (IS_I64(to) &&
		(IS_I8(type) || IS_CHAR(type) || IS_U8(type) ||
		 IS_I16(type) || IS_U16(type) ||
		 IS_I32(type) || IS_U32(type) ||
		 IS_I64(type)))
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
	return NULL;
}