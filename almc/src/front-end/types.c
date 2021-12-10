#include "types.h"

uint8_t is_predefined_type(char* type)
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
}