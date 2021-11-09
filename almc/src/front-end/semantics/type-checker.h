#ifndef ALMC_FRONT_END_TYPE_CHECKER_H
#define ALMC_FRONT_END_TYPE_CHECKER_H

#include <limits.h>

#include "table.h"
#include "..\ast\ast.h"

//----------------------------------------
#define CHAR 0x0
#define U8   0x1
#define I8   0x2

#define U16 0x3
#define I16 0x4

#define U32 0x5
#define I32 0x6

#define U64 0x7
#define I64 0x8

#define F32 0x9
#define F64 0xA

#define STR  -0x1
#define VOID -0x2
//----------------------------------------

#define IS_POINTER_TYPE(type) \
	(type && type->mods.is_ptr)

#define IS_REAL_TYPE(type)             \
   (type && (						   \
    strcmp(type->repr, "f32")  == 0 || \
	strcmp(type->repr, "f64")  == 0))

#define IS_STRING_TYPE(type)									  \
    (type && (													  \
    (strcmp(type->repr, "chr") == 0 && type->mods.is_ptr == 1) || \
     strcmp(type->repr, "str") == 0))

#define IS_INTEGRAL_TYPE(type)        \
   (type && (					      \
    strcmp(type->repr, "i8")  == 0 || \
	strcmp(type->repr, "i16") == 0 || \
	strcmp(type->repr, "i32") == 0 || \
	strcmp(type->repr, "i64") == 0 || \
	strcmp(type->repr, "u8")  == 0 || \
	strcmp(type->repr, "u16") == 0 || \
	strcmp(type->repr, "u32") == 0 || \
	strcmp(type->repr, "u64") == 0 || \
	strcmp(type->repr, "chr") == 0 ))

#define IS_NUMERIC_TYPE(type) \
	(IS_REAL_TYPE(type) || IS_INTEGRAL_TYPE(type))

#define IS_CHAR(type) (type && strcmp(type->repr, "chr") == 0)
#define IS_STRING(type) (type && strcmp(type->repr, "str") == 0)

#define IS_I8(type)  (type && strcmp(type->repr, "i8") == 0)
#define IS_I16(type) (type && strcmp(type->repr, "i16") == 0)
#define IS_I32(type) (type && strcmp(type->repr, "i32") == 0)
#define IS_I64(type) (type && strcmp(type->repr, "i64") == 0)

#define IS_U8(type)  (type && strcmp(type->repr, "u8") == 0)
#define IS_U16(type) (type && strcmp(type->repr, "u16") == 0)
#define IS_U32(type) (type && strcmp(type->repr, "u32") == 0)
#define IS_U64(type) (type && strcmp(type->repr, "u64") == 0)

#define IS_F32(type) (type && strcmp(type->repr, "f32") == 0)
#define IS_F64(type) (type && strcmp(type->repr, "f64") == 0)

#define IS_VOID(type) (type && type->mods.is_void)

Type* get_string_type(Str* str);
Type* get_const_type(Const* cnst);
Type* get_idnt_type(Idnt* idnt, Table* table);

Type* get_expr_type(Expr* expr, Table* table);
Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table);
Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table);
Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table);

uint32_t get_type_priority(Type* type);

Type* cast_implicitly(Type* to, Type* type);
uint32_t can_cast_implicitly(Type* to, Type* type);

char* get_member_name(Expr* expr);

#endif