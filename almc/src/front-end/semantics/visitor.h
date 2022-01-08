#ifndef ALMC_FRONT_END_VISITOR_H
#define ALMC_FRONT_END_VISITOR_H

#include "type-checker.h"
#include "flow-checker.h"

//todo: mark functions and variables that in use, either they will be not generated

typedef struct Visitor
{
	Table* global;
} Visitor;

Visitor* visitor_new();
void visitor_free(Visitor* visitor);

void visit(AstRoot* ast, Visitor* visitor);
void visit_stmt(Stmt* stmt, Table* table);
void visit_type(Type* type, Table* table);
void visit_pointer_like_type(Type* type, Table* table);
void visit_non_void_type(Type* type, SrcArea* area, Table* table);

void visit_scope(Stmt** stmts, Table* table);
void visit_block(Block* block, Table* table);

void visit_expr(Expr* expr, Table* table);
void visit_idnt(Idnt* idnt, Table* table, int is_in_assign);
void visit_enum_member(Idnt* idnt, Table* table);
void visit_func_call(FuncCall* func_call, Table* table);
void visit_unary_expr(UnaryExpr* unary_expr, Table* table);
void visit_binary_expr(BinaryExpr* binary_expr, Table* table);
void visit_ternary_expr(TernaryExpr* ternary_expr, Table* table);
void visit_array_member_accessor(BinaryExpr* arr_accessor_expr, Table* table);

void visit_condition(Expr* condition, Table* table);
void visit_if_stmt(IfStmt* if_stmt, Table* table);
void visit_switch_stmt(SwitchStmt* switch_stmt, Table* table);

void check_for_conjuction_collisions(SwitchStmt* switch_stmt);
void check_for_duplicated_case_conditions(SwitchStmt* switch_stmt, Table* table);

void visit_loop_stmt(LoopStmt* loop_stmt, Table* table);
void visit_do_loop_stmt(DoLoop* do_loop, Table* table);
void visit_for_loop_stmt(ForLoop* for_loop, Table* table);
void visit_while_loop_stmt(WhileLoop* while_loop, Table* table);

void visit_jump_stmt(JumpStmt* jump_stmt, Table* table);
void visit_goto_stmt(JumpStmt* goto_stmt, Table* table);
void visit_break_stmt(JumpStmt* break_stmt, Table* table);
void visit_return_stmt(JumpStmt* return_stmt, Table* table);
void visit_continue_stmt(JumpStmt* continue_stmt, Table* table);

void visit_var_decl_stmt(VarDecl* var_decl, Table* table);
void visit_type_decl_stmt(TypeDecl* type_decl, Table* table);
void visit_func_decl_stmt(FuncDecl* func_decl, Table* table);
void visit_label_decl_stmt(LabelDecl* label_decl, Table* table);
void visit_import_stmt(ImportStmt* import_stmt, Table* table);

void visit_enum(EnumDecl* enum_decl, Table* table);
void visit_union(UnionDecl* union_decl, Table* table);
void visit_struct(StructDecl* struct_decl, Table* table);
void visit_struct_members(Member** members);

void check_entry_func_params(FuncDecl* func_decl);
void visit_entry_func_stmt(FuncDecl* func_decl, Table* table);

uint32_t get_size_of_aggregate_type(Type* type, Table* table);
uint32_t get_size_of_type(Type* type, Table* table);

void complete_size(Type* type, Table* table);
void complete_type(Type* type, Table* table);

#endif