#ifndef ALMC_FRONT_END_VISITOR_H
#define ALMC_FRONT_END_VISITOR_H

#include "table.h"

typedef struct Visitor
{
	Table* global;
} Visitor;

Visitor* visitor_new();
void visitor_free(Visitor* visitor);

void visit(AstRoot* ast, Visitor* visitor);
void visit_stmt(Stmt* stmt, Table* table);
void visit_type(Type* type, Table* table);

void visit_scope(Stmt** stmts, Table* table);
void visit_block(Block* block, Table* table);

void visit_expr(Expr* expr, Table* table);
void visit_idnt(Idnt* idnt, Table* table); 
void visit_unary_expr(Expr* expr, Table* table);
void visit_binary_expr(Expr* expr, Table* table);

void visit_var_decl(VarDecl* var_decl, Table* table);
void visit_type_decl(TypeDecl* type_decl, Table* table);
void visit_func_decl(FuncDecl* func_decl, Table* table);

int is_addressable_value(Expr* expr);

#endif