#include "type.h"

// todo: replace all macroses from type.h to functions
// todo: add NULL check for all type functions

Type* type_new(const char* repr)
{
	Type* type = cnew(Type, 1);
	type->repr = repr;
	type->kind = TYPE_INCOMPLETE;
	type->base = &unknown_type;
	return type;
}

Type* array_type_new(Type* base, Expr* index)
{
	Type* array_type = cnew(Type, 1);
	array_type->base = base;
	array_type->dimension = index;
	array_type->kind = TYPE_ARRAY;
	array_type->repr = base->repr;
	array_type->area = base->area;
	return array_type;
}

Type* pointer_type_new(Type* base)
{
	Type* pointer_type = cnew(Type, 1);
	pointer_type->base = base;
	pointer_type->repr = base->repr;
	pointer_type->area = base->area;
	pointer_type->kind = TYPE_POINTER;
	pointer_type->size = MACHINE_WORD;
	return pointer_type;
}

Type* dereference_type(Type* type)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
	case TYPE_POINTER:
		return type->base;
	}
	return type;
}

Type* address_type(Type* type)
{
	return pointer_type_new(type);
}

char* type_tostr_plain(Type* type)
{
	char* str, temp;
	switch (type->kind)
	{
	case TYPE_ARRAY:
		//temp = type_tostr_plain(type->base);
		//return str = frmt("%s[]", temp), free(temp), str;
		return frmt("%s[]", type_tostr_plain(type->base));
	case TYPE_POINTER:
		//temp = type_tostr_plain(type->base);
		//return str = frmt("%s*", temp), free(temp), str;
		return frmt("%s*", type_tostr_plain(type->base));
	case TYPE_ENUM:
		return frmt("enum %s", type->repr);
	case TYPE_UNION:
		return frmt("union %s", type->repr);
	case TYPE_STRUCT:
		return frmt("struct %s", type->repr);
	default:
		return frmt(type->repr);
	}
}

bool is_u8_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, U8_TYPE);
}

bool is_i8_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, I8_TYPE);
}

bool is_u16_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, U16_TYPE);
}

bool is_i16_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, I16_TYPE);
}

bool is_u32_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, U32_TYPE);
}

bool is_i32_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, I32_TYPE);
}

bool is_u64_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, U64_TYPE);
}

bool is_i64_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, I64_TYPE);
}

bool is_f32_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, F32_TYPE);
}

bool is_f64_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, F64_TYPE);
}

bool is_char_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, CHAR_TYPE);
}

bool is_void_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE &&
		str_eq(type->repr, VOID_TYPE);
}

bool is_enum_type(Type* type)
{
	return type->kind == TYPE_ENUM;
}

bool is_union_type(Type* type)
{
	return type->kind == TYPE_UNION;
}

bool is_struct_type(Type* type)
{
	return type->kind == TYPE_STRUCT;
}

bool is_real_type(Type* type)
{
	return is_f32_type(type) || is_f64_type(type);
}

bool is_numeric_type(Type* type)
{
	return is_real_type(type) || is_integral_type(type);
}

bool is_integral_type(Type* type)
{
	if (is_i8_type(type)  || is_u8_type(type)  || is_char_type(type) ||
		is_i16_type(type) || is_u16_type(type) ||
		is_i32_type(type) || is_u32_type(type) ||
		is_i64_type(type) || is_u64_type(type)) {
			return true;
	}
	return is_enum_type(type);
}

bool is_struct_or_union_type(Type* type)
{
	return is_struct_type(type) || is_union_type(type);
}

bool is_pointer_type(Type* type)
{
	return type->kind == TYPE_POINTER;
}

bool is_primitive_type(Type* type)
{
	return type->kind == TYPE_PRIMITIVE;
}

bool is_pointer_like_type(Type* type)
{
	return type->kind == TYPE_ARRAY || 
		 type->kind == TYPE_POINTER;
}

bool is_both_are_equal_user_defined(Type* type1, Type* type2)
{
	if (IS_UNION_TYPE(type1) && IS_UNION_TYPE(type2))
		return IS_TYPE(type1, type2->repr);
	else if (IS_STRUCT_TYPE(type1) && IS_STRUCT_TYPE(type2))
		return IS_TYPE(type1, type2->repr);
	else if (IS_ENUM_TYPE(type1) && IS_ENUM_TYPE(type2))
		return IS_TYPE(type1, type2->repr);
	return false;
}

bool is_aggregate_type(Type* type)
{
	return type && (type->kind == TYPE_ARRAY ||
		type->kind == TYPE_STRUCT || type->kind == TYPE_UNION);
}

bool is_user_defined_type(Type* type) 
{
	Type* base = get_base_type(type);
	return IS_ENUM_TYPE(base) || 
		IS_STRUCT_OR_UNION_TYPE(base);
}

bool is_array_type(Type* type)
{
	return type && type->kind == TYPE_ARRAY;
}

bool is_incomplete_type(Type* type)
{
	if (type) {
		Type* base = get_base_type(type);
		return base->kind == TYPE_INCOMPLETE ||
			type->kind == TYPE_VOID;
	}
	return false;
}

bool is_signed_type(Type* type)
{
	if (is_i8_type(type)  ||
		is_i16_type(type) ||
		is_i32_type(type) ||
		is_i64_type(type))
			return true;
	return false;
}

bool is_unsigned_type(Type* type)
{
	if (is_u8_type(type)  || is_char_type(type) ||
		is_u16_type(type) ||
		is_u32_type(type) ||
		is_u64_type(type))
			return true;
	return IS_POINTER_TYPE(type);
}

bool is_one(Type* type1, Type* type2, TypeKind kind)
{
	return type1->kind == kind || type2->kind == kind;
}

bool is_onea(Type* type1, Type* type2, 
	bool (action_func)(Type*))
{
	return action_func(type1) || action_func(type2);
}

bool is_both(Type* type1, Type* type2, TypeKind kind)
{
	return type1->kind == kind && type2->kind == kind;
}

bool is_botha(Type* type1, Type* type2, 
	bool (action_func)(Type*))
{
	return action_func(type1) && action_func(type2);
}

Type* get_base_type(Type* type)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
	case TYPE_POINTER:
		return get_base_type(type->base);
	}
	return type;
}

Type* get_array_base_type(Type* type)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
		return get_array_base_type(type->base);
	}
	return type;
}

uint32_t get_pointer_rank(Type* type)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
	case TYPE_POINTER:
		return get_pointer_rank(type->base) + 1;
	}
	return 0;
}

bool can_be_freed(Type* type)
{
	return is_pointer_like_type(type) ||
		IS_STRUCT_OR_UNION_TYPE(type);
}

Expr* get_array_dimension(Type* type, uint32_t dimension)
{
	if (!IS_ARRAY_TYPE(type->base)) {
		return type->dimension;
	}
	if (dimension == 1) {
		return type->dimension;
	}
	return get_array_dimension(type->base, dimension - 1);
}

uint32_t get_array_dimensions(Type* type)
{
	return type->kind == TYPE_ARRAY ?
		(get_array_dimensions(type->base) + 1) : 0;
}

void type_free(Type* type)
{
	if (type && can_be_freed(type)) {
		if (IS_ARRAY_TYPE(type)) {
			expr_free(type->dimension);
		}
		if (IS_STRUCT_OR_UNION_TYPE(type)) {
			free(type->area);
		}
		type_free(type->base);
		free(type);
	}
}