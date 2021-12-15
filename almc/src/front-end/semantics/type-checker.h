#ifndef ALMC_FRONT_END_TYPE_CHECKER_H
#define ALMC_FRONT_END_TYPE_CHECKER_H

#include "table.h"
#include "..\ast\ast-evaluator.h"

/* Type priorities */
#define U8		0x3
#define I8		0x4
#define CHAR	0x5

#define U16		0x6
#define I16		0x7

#define U32		0x8
#define I32		0x9

#define U64		0xA
#define I64		0xB

#define F32		0xC
#define F64		0xD

#define STR		0x2
#define VOID	0x1

Type* get_string_type(Str* str);
Type* get_const_type(Const* cnst);
Type* get_fvalue_type(double value);
Type* get_ivalue_type(int64_t value);
Type* get_idnt_type(Idnt* idnt, Table* table);
Type* get_func_call_type(FuncCall* func_call, Table* table);

Type* retrieve_expr_type(Expr* expr);

Type* get_expr_type(Expr* expr, Table* table);
Type* get_unary_expr_type(UnaryExpr* unary_expr, Table* table);
Type* get_binary_expr_type(BinaryExpr* binary_expr, Table* table);
Type* get_ternary_expr_type(TernaryExpr* ternary_expr, Table* table);
Type* get_and_set_expr_type(Expr* expr, Table* table);

uint32_t get_type_priority(Type* type);

Type* cast_explicitly(Type* to, Type* type);
Type* cast_explicitly_when_const_expr(Expr* const_expr, Type* to, Type* const_expr_type);

Type* cast_implicitly(Type* to, Type* type, SrcArea* area);
Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area);

bool can_cast_implicitly(Type* to, Type* type);

SrcArea* get_expr_area(Expr* expr);
char* get_member_name(Expr* expr);

// todo: replace methods to visitor.h
bool is_const_expr(Expr* expr, Table* table);
bool is_enum_member(const char* var, Table* table);
int is_addressable_value(Expr* expr, Table* table);

Type* get_enum_member_type(const char* member, Table* table);

#endif