#ifndef ALMC_TYPES_MACROSES_H
#define ALMC_TYPES_MACROSES_H

#define I8_TYPE			"i8"
#define I16_TYPE		"i16"
#define I32_TYPE		"i32"
#define I64_TYPE		"i64"
#define U8_TYPE			"u8"
#define U16_TYPE		"u16"
#define U32_TYPE		"u32"
#define U64_TYPE		"u64"
#define F32_TYPE		"f32"
#define F64_TYPE		"f64"
#define CHAR_TYPE		"char"
#define VOID_TYPE		"void"
#define STRING_TYPE		"str"

#define IS_SPECIFIED_TYPE(type, type_str) \
	(type && strcmp(type->repr, type_str) == 0)
#define IS_SPECIFIED_TYPE_POINTER(type) \
	(type && type->spec.ptr_rank)
#define IS_SPECIFIED_TYPE_NON_POINTER(type, type_str) \
	(IS_SPECIFIED_TYPE(type, type_str) && !IS_SPECIFIED_TYPE_POINTER(type))

#define IS_POINTER_TYPE(type) \
	(IS_SPECIFIED_TYPE_POINTER(type))
#define IS_REAL_TYPE(type) \
	(IS_SPECIFIED_TYPE_NON_POINTER(type, F32_TYPE) || \
	 IS_SPECIFIED_TYPE_NON_POINTER(type, F64_TYPE))
#define IS_INTEGRAL_TYPE(type) \
    (IS_SPECIFIED_TYPE_NON_POINTER(type, I8_TYPE)  ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, I16_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, I32_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, I64_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, U8_TYPE)  ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, U16_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, U32_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, U64_TYPE) ||  \
     IS_SPECIFIED_TYPE_NON_POINTER(type, CHAR_TYPE))
#define IS_NUMERIC_TYPE(type) \
	(IS_REAL_TYPE(type) || IS_INTEGRAL_TYPE(type))

#define IS_POINTER_RANK(rank, type) \
	(type && type->spec.ptr_rank == (rank))
#define IS_CHAR_POINTER_TYPE(type) \
	(IS_SPECIFIED_TYPE(type, CHAR_TYPE) && IS_POINTER_TYPE(type))

#define IS_I8_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, I8_TYPE)
#define IS_I16_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, I16_TYPE)
#define IS_I32_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, I32_TYPE)
#define IS_I64_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, I64_TYPE)
#define IS_U8_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, U8_TYPE)
#define IS_U16_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, U16_TYPE)
#define IS_U32_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, U32_TYPE)
#define IS_U64_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, U64_TYPE)
#define IS_F32_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, F32_TYPE)
#define IS_F64_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, F64_TYPE)
#define IS_STRING_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, STRING_TYPE)
#define IS_VOID_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, VOID_TYPE)
#define IS_CHAR_TYPE(type) \
	IS_SPECIFIED_TYPE_NON_POINTER(type, CHAR_TYPE)

#endif