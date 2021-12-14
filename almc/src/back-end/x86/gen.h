#ifndef ALMC_BACK_END_STMT_GENERATOR
#define ALMC_BACK_END_STMT_GENERATOR

#include <stdio.h>
#include <assert.h>
#include "expr-gen.h"
#include "program.h"
#include "..\..\utils\common.h"

void gen_var_decl_stmt(VarDecl* var_decl, StackFrame* frame);
void gen_func_decl_stmt(FuncDecl* func_decl);
void gen_stmt(Stmt* stmt, StackFrame* frame);

AsmProgram* gen(AstRoot* ast, Table* table);
void gen_global_stmt(Stmt* stmt);

#endif