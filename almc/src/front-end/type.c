#include "type.h"

Type* type_new(const char* repr)
{
	Type* type = cnew_s(Type, type, 1);
	type->repr = repr;
	type->kind = TYPE_INCOMPLETE;
	return type;
}

Type* array_type_new(Type* base, Expr* index)
{
	Type* array_type = cnew_s(Type, array_type, 1);
	array_type->kind = TYPE_ARRAY;
	array_type->dimension = index,
		array_type->base = base;
	array_type->repr = base->repr,
		array_type->area = base->area;
	return array_type;
}

Type* pointer_type_new(Type* base)
{
	Type* pointer_type = cnew_s(Type, pointer_type, 1);
	pointer_type->kind = TYPE_POINTER;
	pointer_type->base = base;
	pointer_type->repr = base->repr,
		pointer_type->area = base->area;
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

bool is_real_type(Type* type)
{
	return (IS_F32_TYPE(type) || IS_F64_TYPE(type)) ?
		true : false;
}

bool is_numeric_type(Type* type)
{
	return is_real_type(type) ||
		is_integral_type(type);
}

bool is_integral_type(Type* type)
{
	if (IS_I8_TYPE(type)  || IS_U8_TYPE(type)  || IS_CHAR_TYPE(type) ||
		IS_I16_TYPE(type) || IS_U16_TYPE(type) ||
		IS_I32_TYPE(type) || IS_U32_TYPE(type) ||
		IS_I64_TYPE(type) || IS_U64_TYPE(type))
			return true;
	return IS_ENUM_TYPE(type) || false;
}

bool is_integral_smaller_than_pointer_type(Type* type)
{
	return is_integral_type(type) && 
		(type->size <= MACHINE_WORD);
}

bool is_pointer_like_type(Type* type)
{
	return IS_ARRAY_TYPE(type) || 
		IS_POINTER_TYPE(type);
}

bool is_both_primitive(Type* type1, Type* type2)
{
	return IS_PRIMITIVE_TYPE(type1) &&
		IS_PRIMITIVE_TYPE(type2);
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

bool is_not_aggregate_type(Type* type)
{
	return IS_PRIMITIVE_TYPE(type) || 
		IS_VOID_TYPE(type);
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
		IS_AGGREGATE_TYPE(type);
}

void type_free(Type* type)
{
	if (type && can_be_freed(type))
	{
		if (IS_ARRAY_TYPE(type))
			expr_free(type->dimension);
		type_free(type->base);
		free(type);
	}
}