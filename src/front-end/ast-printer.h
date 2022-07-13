#ifndef _ALMC_AST_CONSOLE_PRINTER_H
#define _ALMC_AST_CONSOLE_PRINTER_H

#include "ast.h"
#include "ast-evaluator.h"

#include "type-checker.h"

#include "../utils/os.h"
#include "../utils/console/colors.h"

void print_ast_root(AstRoot* ast, const char* indent);
void print_ast(AstRoot* ast);
void print_ast_header(AstRoot* ast);
void print_type(Type* type);
void print_idnt(Idnt* idnt);
char get_explicit_escapec(char escapec);
void print_const(Const* cnst);
void print_str(Str* str);
void print_unary_expr(UnaryExpr* unary_expr, const char* indent);
void print_binary_expr(BinaryExpr* binary_expr, const char* indent);
void print_ternary_expr(TernaryExpr* ternary_expr, const char* indent);
void print_initializer_expr(Initializer* init_expr, const char* indent);
void print_expr(Expr* expr, const char* indent);
void print_block_stmt(Block* block, const char* indent);
void print_var_decl_stmt(VarDecl* var_decl_stmt, const char* indent);
void print_struct_member(Member* member, const char* struct_name);
void print_enum(EnumDecl* enum_decl, const char* indent);
void print_struct(StructDecl* struct_decl, const char* indent);
void print_union(UnionDecl* union_decl, const char* indent);
void print_type_decl_stmt(TypeDecl* type_decl_stmt, const char* indent);
void print_func_decl_stmt(FuncDecl* func_decl, const char* indent);
void print_elif_stmt(ElseIf* elif_stmt, const char* indent);
void print_if_stmt(IfStmt* if_stmt, const char* indent);
void print_expr_stmt(ExprStmt* expr_stmt, const char* indent);
void print_break_stmt(const char* indent);
void print_continue_stmt(const char* indent);
void print_goto_stmt(JumpStmt* jump_stmt, const char* indent);
void print_return_stmt(JumpStmt* jump_stmt, const char* indent);
void print_jump_stmt(JumpStmt* jump_stmt, const char* indent);
void print_while_loop_stmt(WhileLoop* while_stmt, const char* indent);
void print_for_loop_stmt(ForLoop* for_stmt, const char* indent);
void print_do_loop_stmt(DoLoop* do_stmt, const char* indent);
void print_loop_stmt(LoopStmt* loop_stmt, const char* indent);
void print_label_decl_stmt(LabelDecl* label_decl, const char* indent);
void print_switch_case_stmt(SwitchStmt* switch_stmt, const char* indent);
void print_typedef_stmt(TypedefStmt* typedef_stmt, const char* indent);
void print_stmt(Stmt* stmt, const char* indent);

#endif // _ALMC_AST_CONSOLE_PRINTER