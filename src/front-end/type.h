#ifndef _ALMC_TYPES_H
#define _ALMC_TYPES_H

#include <string.h>

#include "../utils/common.h"
#include "../utils/context.h"
#include "../utils/data-structures/sbuffer.h"
#include "../utils/data-structures/str-builder.h"

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

#define PTR_SIZE				 sizeof(void*)
#define MACHINE_WORD			 sizeof(void*)
#define STRUCT_DEFAULT_ALIGNMENT 0x4

typedef struct Expr Expr;
typedef struct Member Member;

extern void expr_free(Expr* expr);

typedef enum TypeKind {
	TYPE_ARRAY,
	TYPE_POINTER,
	TYPE_PRIMITIVE,

	TYPE_ENUM,
	TYPE_UNION,
	TYPE_STRUCT,
	TYPE_FUNCTION,

	TYPE_INCOMPLETE,
	TYPE_VOID,
	TYPE_UNKNOWN
} TypeKind;

typedef struct Type {
	size_t size;
	TypeKind kind;
	SrcArea* area;
	const char* repr;

	// Used by pointer and array types
	struct Type* base;
	bool is_alias;
	union _type_attributes {
		struct _array_kind_data {
			// can be accessed when type is TYPE_ARRAY
			Expr* dimension;
			// max capacity of current dimension
			size_t capacity;
		} arr;
		struct _function_kind_data {
			struct Type* ret;
			struct Type** params;
		} func;
		struct _struct_or_union_kind_data {
			// can be accessed when type is struct or union
			Member** members;
		} cmpd;
	} attrs;
} Type;

/* Initialization of primitive types */
static Type unknown_type = { 0, TYPE_UNKNOWN, NULL, UNKNOWN_TYPE, false };

static Type i8_type = { sizeof(int8_t), TYPE_PRIMITIVE, NULL, I8_TYPE, &unknown_type, false };
static Type u8_type = { sizeof(uint8_t), TYPE_PRIMITIVE, NULL, U8_TYPE, &unknown_type, false };
static Type char_type = { sizeof(char), TYPE_PRIMITIVE, NULL, CHAR_TYPE, &unknown_type, false };

static Type i16_type = { sizeof(int16_t), TYPE_PRIMITIVE, NULL, I16_TYPE, &unknown_type, false };
static Type u16_type = { sizeof(uint16_t), TYPE_PRIMITIVE, NULL, U16_TYPE, &unknown_type, false };

static Type i32_type = { sizeof(int32_t), TYPE_PRIMITIVE, NULL, I32_TYPE, &unknown_type, false };
static Type u32_type = { sizeof(uint32_t), TYPE_PRIMITIVE, NULL, U32_TYPE, &unknown_type, false };
static Type f32_type = { sizeof(float), TYPE_PRIMITIVE, NULL, F32_TYPE, &unknown_type, false };

static Type i64_type = { sizeof(int64_t), TYPE_PRIMITIVE, NULL, I64_TYPE, &unknown_type, false };
static Type u64_type = { sizeof(uint64_t), TYPE_PRIMITIVE, NULL, U64_TYPE, &unknown_type, false };
static Type f64_type = { sizeof(double), TYPE_PRIMITIVE, NULL, F64_TYPE, &unknown_type, false };

/* Initialization of some other supply types */
static Type void_type = { sizeof(void), TYPE_VOID, NULL, VOID_TYPE, false };

Type* type_new(const char* repr);

Type* alias_type_new(const char* alias, Type* base);
Type* array_type_new(Type* base, Expr* index);
Type* pointer_type_new(Type* base);
Type* dereference_type(Type* type);
Type* address_type(Type* type);
Type* function_type_new(Type* type, Type** params);

const char* type_tostr_plain(Type* type);

bool is_u8_type(Type* type);
bool is_i8_type(Type* type);
bool is_u16_type(Type* type);
bool is_i16_type(Type* type);
bool is_u32_type(Type* type);
bool is_i32_type(Type* type);
bool is_u64_type(Type* type);
bool is_i64_type(Type* type);
bool is_f32_type(Type* type);
bool is_f64_type(Type* type);
bool is_char_type(Type* type);
bool is_void_type(Type* type);
bool is_enum_type(Type* type);
bool is_union_type(Type* type);
bool is_struct_type(Type* type);
bool is_alias_type(Type* type);
bool is_array_type(Type* type);
bool is_pointer_type(Type* type);
bool is_primitive_type(Type* type);
bool is_function_type(Type* type);
bool is_pointer_like_type(Type* type);
bool is_incomplete_type(Type* type);

bool is_real_type(Type* type);
bool is_numeric_type(Type* type);
bool is_integral_type(Type* type);
bool is_struct_or_union_type(Type* type);

bool is_aggregate_type(Type* type);
bool is_user_defined_type(Type* type);

bool is_signed_type(Type* type);
bool is_unsigned_type(Type* type);

bool is_one(Type* type1, Type* type2, TypeKind kind);
bool is_one_action(Type* type1, Type* type2, bool (action_func)(Type*));
bool is_both(Type* type1, Type* type2, TypeKind kind);
bool is_both_action(Type* type1, Type* type2, bool (action_func)(Type*));

Type* get_base_type(Type* type);
Type* get_array_base_type(Type* type);
uint32_t get_pointer_rank(Type* type);
uint32_t get_array_dimensions(Type* type);
Expr* get_array_dimension(Type* type, uint32_t dimension);

void type_free(Type* type);

#endif // _ALMC_TYPES_H