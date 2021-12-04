#ifndef ALMC_TYPES_H
#define ALMC_TYPES_H

#include <string.h>

#include "type-macro.h"
#include "..\utils\common.h"
#include "..\utils\context.h"

typedef struct Expr Expr;

typedef struct TypeMods
{
	uint16_t is_ptr;   // ptr rank
	uint16_t is_array; // arr rank
	
	uint8_t is_void;
	uint8_t is_unknown;
	uint8_t is_predefined;
} TypeMods;

typedef struct TypeInfo
{
	uint8_t is_fixed_array;
	uint8_t is_dynamic_array;

	uint64_t arr_dim_count; // dimension count
	Expr** arr_dimensions;  // value of each dimension
} TypeInfo;

typedef struct Type
{
	TypeMods mods;
	TypeInfo info;
	SrcArea* area;
	const char* repr;
} Type;

Type* unknown_type_new();

Type* type_new(const char* repr, SrcArea* area);
Type* type_new2(const char* repr, TypeMods mods, 
	SrcArea* area);
Type* type_new3(const char* repr, TypeInfo info, 
	TypeMods mods, SrcArea* area);

Type* type_dup(Type* type);

#endif