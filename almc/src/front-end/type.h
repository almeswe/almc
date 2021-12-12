#ifndef ALMC_TYPES_H
#define ALMC_TYPES_H

#include <string.h>
#include <stdbool.h>

#include "..\utils\common.h"
#include "..\utils\context.h"

#define I8_TYPE		 "i8"
#define I16_TYPE	 "i16"
#define I32_TYPE	 "i32"
#define I64_TYPE	 "i64"
#define U8_TYPE		 "u8"
#define U16_TYPE	 "u16"
#define U32_TYPE	 "u32"
#define U64_TYPE	 "u64"
#define F32_TYPE	 "f32"
#define F64_TYPE	 "f64"
#define CHAR_TYPE	 "char"
#define VOID_TYPE	 "void"
#define STRING_TYPE	 "str"
#define UNKNOWN_TYPE "unknown"

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

#define IS_TYPE(type, str)      (type && (strcmp(type->repr, str) == 0))

#define IS_ARRAY_TYPE(type)     (type && (type->kind == TYPE_ARRAY))
#define IS_POINTER_TYPE(type)   (type && (type->kind == TYPE_POINTER))
#define IS_PRIMITIVE_TYPE(type) (type && (type->kind == TYPE_PRIMITIVE))

#define IS_UNKNOWN_TYPE(type)	 (type && (type->kind == TYPE_UNKNOWN))
#define IS_INCOMPLETE_TYPE(type) (type && (type->kind == TYPE_INCOMPLETE))

#define IS_ENUM_TYPE(type)		(type && (type->kind == TYPE_ENUM))
#define IS_UNION_TYPE(type)		(type && (type->kind == TYPE_UNION))
#define IS_STRUCT_TYPE(type)	(type && (type->kind == TYPE_STRUCT))
#define IS_AGGREGATE_TYPE(type) (IS_UNION_TYPE(type) || IS_STRUCT_TYPE(type))

#define IS_I8_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, I8_TYPE))
#define IS_I16_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, I16_TYPE))
#define IS_I32_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, I32_TYPE))
#define IS_I64_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, I64_TYPE))
#define IS_U8_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, U8_TYPE))
#define IS_U16_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, U16_TYPE))
#define IS_U32_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, U32_TYPE))
#define IS_U64_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, U64_TYPE))
#define IS_F32_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, F32_TYPE))
#define IS_F64_TYPE(type)		(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, F64_TYPE))
#define IS_CHAR_TYPE(type)	    (IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, CHAR_TYPE))
#define IS_STRING_TYPE(type)	(IS_PRIMITIVE_TYPE(type) && IS_TYPE(type, STRING_TYPE))
#define IS_VOID_TYPE(type)	    (type && (type->kind == TYPE_VOID))

#define IS_CHAR_POINTER_TYPE(type) (type && (IS_CHAR_TYPE(type->base) && IS_POINTER_TYPE(type)))

typedef struct Expr Expr;
typedef struct Member Member;

extern void expr_free(Expr* expr);

typedef enum TypeKind 
{
	TYPE_ARRAY,
	TYPE_POINTER,
	TYPE_PRIMITIVE,

	TYPE_ENUM,
	TYPE_UNION,
	TYPE_STRUCT,

	TYPE_INCOMPLETE,
	TYPE_VOID,
	TYPE_UNKNOWN
} TypeKind;

typedef struct Type
{
	uint32_t size;
	TypeKind kind;
	SrcArea* area;
	const char* repr;

	// Used by pointer and array types
	struct Type* base;
	union
	{
		struct _array_kind_data
		{
			// can be accessed when type is TYPE_ARRAY
			Expr* dimension;
		};
		struct _aggregate_kind_data
		{
			// can be accessed when type is aggregate (struct or union)
			Member** members;
		};
	};
} Type;

/* Initialization of primitive types */
static Type i8_type = { I8_SIZE, TYPE_PRIMITIVE, NULL, I8_TYPE };
static Type u8_type = { U8_SIZE, TYPE_PRIMITIVE, NULL, U8_TYPE };
static Type char_type = { CHAR_SIZE, TYPE_PRIMITIVE, NULL, CHAR_TYPE };

static Type i16_type = { I16_SIZE, TYPE_PRIMITIVE, NULL, I16_TYPE };
static Type u16_type = { U16_SIZE, TYPE_PRIMITIVE, NULL, U16_TYPE };

static Type i32_type = { I32_SIZE, TYPE_PRIMITIVE, NULL, I32_TYPE };
static Type u32_type = { U32_SIZE, TYPE_PRIMITIVE, NULL, U32_TYPE };
static Type f32_type = { F32_SIZE, TYPE_PRIMITIVE, NULL, F32_TYPE };

static Type i64_type = { I64_SIZE, TYPE_PRIMITIVE, NULL, I64_TYPE };
static Type u64_type = { U64_SIZE, TYPE_PRIMITIVE, NULL, U64_TYPE };
static Type f64_type = { F64_SIZE, TYPE_PRIMITIVE, NULL, F64_TYPE };

/* Initialization of some other supply types */
static Type void_type = { 0, TYPE_VOID, NULL, VOID_TYPE };
static Type unknown_type = { 0, TYPE_UNKNOWN, NULL, UNKNOWN_TYPE };

Type* type_new(const char* repr);

Type* array_type_new(Type* base, Expr* index);
Type* pointer_type_new(Type* base);
Type* dereference_type(Type* type);
Type* address_type(Type* type);

char* type_tostr_plain(Type* type);

bool is_real_type(Type* type);
bool is_numeric_type(Type* type);
bool is_integral_type(Type* type);
bool is_integral_smaller_than_pointer_type(Type* type);

bool is_pointer_like_type(Type* type);
bool is_not_aggregate_type(Type* type);
bool is_both_primitive(Type* type1, Type* type2);
bool is_both_are_equal_user_defined(Type* type1, Type* type2);

Type* get_base_type(Type* type);
uint32_t get_pointer_rank(Type* type);

void type_free(Type* type);

#endif