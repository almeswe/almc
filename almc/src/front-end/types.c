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
	type->mods.is_unknown = 1;
	return type;
}

Type* type_new(const char* repr, SrcArea* area)
{
	Type* type = cnew_s(Type, type, 1);
	type->repr = repr;
	type->mods.is_predefined = 
		is_predefined_type(repr);
	type->area = area;
	return type;
}

Type* type_new2(const char* repr, TypeMods mods,
	SrcArea* area)
{
	Type* type = type_new(repr, area);
	uint8_t is_predefined_already =
		type->mods.is_predefined;
	type->mods = mods;
	if (is_predefined_already)
		type->mods.is_predefined = 1;
	return type;
}

Type* type_new3(const char* repr, TypeInfo info,
	TypeMods mods, SrcArea* area)
{
	Type* type = type_new2(repr, mods, area);
	type->info = info;
	return type;
}

Type* type_dup(Type* type)
{
	//todo: probably add deep copy for expressions in expr->info.arr_dimensions
	TypeMods mods = type->mods;
	TypeInfo info = type->info;
	info.arr_dimensions = NULL;
	SrcArea* area = type->area ?
		src_area_new(type->area->begins, 
			type->area->ends) : NULL;
	return type_new3(type->repr, 
		info, mods, area);
}

Type* type_address(Type* type)
{
	type->mods.ptr_rank += 1;
	return type;
}

Type* type_dereference(Type* type)
{
	type->mods.ptr_rank -= type->mods.ptr_rank > 0 ?
		1 : 0;
	type->mods.array_rank -= type->mods.array_rank > 0 ?
		1 : 0;
	return type;
}