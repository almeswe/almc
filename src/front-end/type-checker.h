#ifndef _ALMC_FRONT_END_TYPE_CHECKER_H
#define _ALMC_FRONT_END_TYPE_CHECKER_H

#include "table.h"
#include "..\ast\ast-evaluator.h"

/* Type priorities */
enum _type_priority_table
{
	VOID_TYPE_PRIORITY = 0x1,
	STR_TYPE_PRIORITY,

	U8_TYPE_PRIORITY,  
	I8_TYPE_PRIORITY, 
	CHAR_TYPE_PRIORITY,
	
	U16_TYPE_PRIORITY, 
	I16_TYPE_PRIORITY,
	
	U32_TYPE_PRIORITY, 
	I32_TYPE_PRIORITY,
	
	U64_TYPE_PRIORITY, 
	I64_TYPE_PRIORITY,
	F32_TYPE_PRIORITY, 
	F64_TYPE_PRIORITY
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

Type* cast_implicitly(Type* to, Type* type, SrcArea* area);
Type* cast_implicitly_when_assign(Type* to, Type* type, SrcArea* area);

bool can_cast_implicitly(Type* to, Type* type);

const char* get_member_name(Expr* expr);
Idnt* get_member_idnt(Expr* expr);
SrcArea* get_expr_area(Expr* expr);

#endif // _ALMC_FRONT_END_TYPE_CHECKER_H