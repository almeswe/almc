#ifndef ALMC_BACK_END_STMT_GENERATOR
#define ALMC_BACK_END_STMT_GENERATOR

#include <stdio.h>
#include <assert.h>
#include "expr-gen.h"
#include "..\..\utils\common.h"

void gen_var_decl(VarDecl* var_decl, StackFrame* frame);
void gen_func_decl(FuncDecl* func_decl);

#endif