#ifndef ALMC_BACK_END_EXPR_GENERATOR
#define ALMC_BACK_END_EXPR_GENERATOR

#include "program.h"
#include "regtable.h"
#include "x86types.h"
#include "stack-frame.h"
#include "instructions.h"
#include "..\..\front-end\front-end.h"

// TOPICS 
/*

https://www.plantation-productions.com/Webster/www.artofasm.com/Windows/HTML/RealArithmetica3.html
	
*/

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

void gen_expr32(Expr* expr, StackFrame* frame);
void gen_idnt32(Idnt* idnt, StackFrame* frame);
void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame);
void gen_unary_expr2(UnaryExpr* unary_expr, StackFrame* frame);
void gen_binary_expr32(BinaryExpr* binary_expr, StackFrame* frame);
void gen_func_call32(FuncCall* func_call, StackFrame* frame);

#endif 