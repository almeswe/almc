#ifndef ALMC_BACK_END_EXPR_GENERATOR
#define ALMC_BACK_END_EXPR_GENERATOR

#include "program.h"
#include "regtable.h"
#include "stack-frame.h"
#include "..\..\front-end\front-end.h"

typedef enum x86_ExprGeneratorResult
{
	EXPR_GEN_RES_QWORD,
	EXPR_GEN_RES_DWORD,
	EXPR_GEN_RES_WORD,
	EXPR_GEN_RES_BYTE,
} ExprGeneratorResult;

typedef enum
{
	ADDRESSABLE_UNKNOWN,
	ADDRESSABLE_ENTITY,
	ADDRESSABLE_ACCESSOR,
	ADDRESSABLE_ARR_ACCESSOR,
	ADDRESSABLE_PTR_ACCESSOR
} _addressable_kind;

typedef struct {
	// last type which was assigned in last recursion (mostly all functions
	// for determingn the addressible data are recursive).
	// Type needed majorly for specifying the ptr prefix
	Type* type;
	// offset added to address register.
	// will be cleared every time in recursion with pointer addressing
	int32_t offset;
	// register which stores the last address
	// needed for '->', '[ ]', where we have access to heap
	int32_t reg;
	// flag that specifies that we have base address stored in register
	bool in_reg;
	// variant for determing the base address for offset,
	// in case of entity is stack memory, in register case - heap
	StackFrameEntity* entity;
	_addressable_kind kind;
} _addressable_data;

_addressable_data* gen_addressable_data(Expr* expr, StackFrame* frame);

_addressable_data* addressable_data_new();
void addressable_data_free(_addressable_data* data);
char* addressable_data_arg(_addressable_data* data);

void gen_expr32(Expr* expr, StackFrame* frame);
void gen_idnt32(Idnt* idnt, int reg, StackFrame* frame);
void gen_string32(Str* str, int reg);
void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame);
void gen_unary_sizeof32(UnaryExpr* expr);
void gen_unary_lengthof32(UnaryExpr* expr);
void gen_unary_expr32(UnaryExpr* unary_expr, StackFrame* frame);

void gen_binary_comma_expr32(BinaryExpr* expr, StackFrame* frame);
void gen_binary_assign_expr32(BinaryExpr* assign_expr, StackFrame* frame);
void gen_binary_relative_expr32(BinaryExpr* relative_expr, StackFrame* frame);
void gen_binary_expr32(BinaryExpr* binary_expr, StackFrame* frame);
void gen_ternary_expr(TernaryExpr* ternary_expr, StackFrame* frame);
void gen_func_call32(FuncCall* func_call, StackFrame* frame);

void gen_callee_stack_clearing(FuncDecl* func_decl);
void gen_caller_stack_clearing(FuncCall* func_call);

#endif 