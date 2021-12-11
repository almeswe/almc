#include "type.h"

/*uint8_t is_predefined_type(char* type)
{
	char* types[] = {
		VOID_TYPE, CHAR_TYPE, I8_TYPE, U8_TYPE, I16_TYPE, U16_TYPE,
		I32_TYPE, U32_TYPE, I64_TYPE, U64_TYPE, F32_TYPE, F64_TYPE
	};

	for (size_t i = 0; i < sizeof(types) / sizeof(char*); i++)
		if (strcmp(type, types[i]) == 0)
			return 1;
	return 0;
}

Type* unknown_type_new()
{
	Type* type = cnew_s(Type, type, 1);
	type->repr = "unknown";
	type->spec.is_unknown = 1;
	return type;
}

Type* type_new(const char* repr, SrcArea* area)
{
	Type* type = cnew_s(Type, type, 1);
	type->repr = repr;
	type->is_origin = 0;
	type->spec.is_predefined = 
		is_predefined_type(repr);
	type->area = area;
	return type;
}

Type* type_new2(const char* repr, TypeSpec spec,
	SrcArea* area)
{
	Type* type = type_new(repr, area);
	uint8_t is_predefined_already =
		type->spec.is_predefined;
	type->spec = spec;
	if (is_predefined_already)
		type->spec.is_predefined = 1;
	return type;
}

Type* type_dup(Type* type)
{
	TypeSpec spec = type->spec;
	SrcArea* area = type->area ?
		src_area_new(type->area->begins, 
			type->area->ends) : NULL;
	Type* dup = type_new2(type->repr, spec, area);
	dup->is_origin = 0;
	dup->members = type->members;
	dup->dimensions = type->dimensions;
	return dup->size = type->size, dup;
}

Type* type_address(Type* type)
{
	type->spec.ptr_rank += 1;
	return type;
}

Type* type_dereference(Type* type)
{
	type->spec.ptr_rank -= type->spec.ptr_rank > 0 ?
		1 : 0;
	type->spec.array_rank -= type->spec.array_rank > 0 ?
		1 : 0;
	return type;
}*/

Type* type_new(const char* repr)
{
	Type* type = cnew_s(Type, type, 1);
	type->repr = repr;
	//type->can_be_freed = true;
	type->kind = TYPE_INCOMPLETE;
	return type;
}

Type* array_type_new(Type* base, Expr* index)
{
	Type* array_type = cnew_s(Type, array_type, 1);
	array_type->kind = TYPE_ARRAY;
	//array_type->can_be_freed = true;
	//array_type->is_origin_array = true;
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
	//pointer_type->can_be_freed = true;
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
	// todo: error here because in other case 
	// we cannot dereference type
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
	return false;
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
		//todo: issue with freeing area!
		//free(type->area);
		free(type);
	}
}