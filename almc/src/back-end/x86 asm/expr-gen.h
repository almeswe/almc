#ifndef ALMC_BACK_END_EXPR_GENERATOR
#define ALMC_BACK_END_EXPR_GENERATOR

#include "regtable.h"
#include "..\..\front-end\ast\ast.h"

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

void gen_expr(ExprGenerator* expr_gen, Expr* expr);
void gen_primary_expr(ExprGenerator* expr_gen, Expr* prim_expr, int reg);
void gen_unary_expr(ExprGenerator* expr_gen, UnaryExpr* unary_expr);
void gen_binary_expr(ExprGenerator* expr_gen, BinaryExpr* binary_expr);

#endif 