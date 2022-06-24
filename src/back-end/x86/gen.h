#ifndef _ALMC_BACK_END_x86_STMT_GENERATOR_H
#define _ALMC_BACK_END_x86_STMT_GENERATOR_H

#include <stdio.h>
#include <assert.h>
#include "expr-gen.h"
#include "program.h"
#include "..\..\utils\common.h"

void gen_if_stmt(IfStmt* if_stmt, StackFrame* frame);
void gen_loop_stmt(LoopStmt* loop_stmt, StackFrame* frame);
void gen_do_loop_stmt(DoLoop* do_loop, StackFrame* frame);
void gen_for_loop_stmt(ForLoop* for_loop, StackFrame* frame);
void gen_while_loop_stmt(WhileLoop* while_loop, StackFrame* frame);
void gen_jump_stmt(JumpStmt* jump_stmt, StackFrame* frame);

void gen_block(Block* block, StackFrame* frame);
void gen_var_decl_stmt(VarDecl* var_decl, StackFrame* frame);
void gen_func_decl_stmt(FuncDecl* func_decl);
void gen_stmt(Stmt* stmt, StackFrame* frame);
void gen_expr_stmt(ExprStmt* expr_stmt, StackFrame* frame);

void gen_global_stmt(Stmt* stmt);
AsmProgram* gen(AstRoot* ast, Table* table);

#endif // _ALMC_BACK_END_x86_STMT_GENERATOR_H