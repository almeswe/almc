#ifndef ALMC_TYPES_H
#define ALMC_TYPES_H

#include <string.h>

#include "type-macro.h"
#include "..\utils\common.h"
#include "..\utils\context.h"

#define MACHINE_WORD 0x4

#define I8_SIZE		sizeof(int8_t)
#define U8_SIZE		sizeof(uint8_t)
#define CHAR_SIZE	sizeof(int8_t)

#define I16_SIZE	sizeof(int16_t)
#define U16_SIZE	sizeof(uint16_t)
#define I32_SIZE	sizeof(int32_t)
#define U32_SIZE	sizeof(uint32_t)

#define I64_SIZE	sizeof(int64_t)
#define U64_SIZE	sizeof(uint64_t)

#define F32_SIZE	sizeof(float)
#define F64_SIZE	sizeof(double)

typedef struct Expr Expr;
typedef struct Member Member;

typedef struct TypeSpec
{
	int16_t ptr_rank;   
	int16_t array_rank;
	
	uint8_t is_void;
	uint8_t is_unknown;

	uint8_t is_union;
	uint8_t is_struct;
	uint8_t is_predefined;
} TypeSpec;

/*
	todo: add recursive type structure!
	todo: fix the error with accessing the struct's member
	typedef struct d {
		int a;
	} d_t;

	(*d)->a;
*/

typedef struct Type
{
	uint32_t size;
	TypeSpec spec;
	SrcArea* area;
	const char* repr;

	struct _array_kind_data 
	{
		// left-right order
		// flag that specifies the possibility
		// for type_free to free dimensions
		char is_origin;
		Expr** dimensions;
	};
	struct _aggregate_kind_data 
	{
		Member** members;
	};
} Type;

Type* unknown_type_new();

Type* type_new(const char* repr, SrcArea* area);
Type* type_new2(const char* repr, TypeSpec spec, 
	SrcArea* area);
Type* type_dup(Type* type);

Type* type_address(Type* type);
Type* type_dereference(Type* type);

void type_free(Type* type);

#endif