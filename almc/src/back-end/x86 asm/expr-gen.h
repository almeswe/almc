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

typedef struct x86_ExprGenerator
{
	RegisterTable* regtable;
	ExprGeneratorResult result;
} ExprGenerator;

ExprGenerator* expr_gen_new(RegisterTable* regtable);
void expr_gen_free(ExprGenerator* expr_gen);

void gen_expr32(Expr* expr, StackFrame* frame);
void gen_idnt32(Idnt* idnt, StackFrame* frame);
void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame);
void gen_unary_expr2(UnaryExpr* unary_expr, StackFrame* frame);
void gen_binary_expr2(BinaryExpr* binary_expr, StackFrame* frame);
void gen_func_call(FuncCall* func_call, StackFrame* frame);

#endif 