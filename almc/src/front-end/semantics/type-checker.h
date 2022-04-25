#ifndef _ALMC_FRONT_END_TYPE_CHECKER_H
#define _ALMC_FRONT_END_TYPE_CHECKER_H

#include "table.h"
#include "..\ast\ast-evaluator.h"

/* Type priorities */
enum _type_priority_table
{
	VOIDp = 0x1,
	STRp  = 0x2,

	U8p,  I8p, CHARp,
	U16p, I16p,
	U32p, I32p,
	U64p, I64p,
	F32p, F64p
};

extern bool is_const_expr(Expr* expr);

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
Type* get_spec_binary_type(BinaryExpr* expr);

Type* cast_explicitly(Type* to, Type* type);
Type* cast_explicitly_when_const_expr(Expr* const_expr, Type* to, Type* const_expr_type);

Type* cast_implicitly(Type* to, Type* type, SrcArea* area);
Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area);

bool can_cast_implicitly(Type* to, Type* type);

const char* get_member_name(Expr* expr);
Idnt* get_member_idnt(Expr* expr);
SrcArea* get_expr_area(Expr* expr);

#endif // _ALMC_FRONT_END_TYPE_CHECKER_H