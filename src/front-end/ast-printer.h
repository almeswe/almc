#ifndef _ALMC_AST_CONSOLE_PRINTER_H
#define _ALMC_AST_CONSOLE_PRINTER_H

#include "ast.h"
#include "../utils/os.h"
#include "type-checker.h"
#include "ast-evaluator.h"
#include "../utils/console/colors.h"

void print_expr(Expr* expr, const char* indent);
void print_stmt(Stmt* stmt, const char* indent);

void print_ast(AstRoot* ast);

void print_type_spec(Type* type, const char* indent);
void print_type(Type* type);
void print_idnt(Idnt* idnt);
void print_str(Str* str);
void print_const(Const* cnst);
void print_call(FuncCall* func_call, const char* indent);
void print_unary_expr(UnaryExpr* expr, const char* indent);
void print_binary_expr(BinaryExpr* expr, const char* indent);
void print_ternary_expr(TernaryExpr* expr, const char* indent);
void print_initializer(Initializer* initializer, const char* indent);
void print_expr(Expr* expr, const char* indent);
void print_type_var(TypeVar* type_var, const char* indent);
void print_block_stmt(Block* block, const char* indent);
void print_var_decl_stmt(VarDecl* var_decl, const char* indent);
void print_func_decl_stmt(FuncDecl* func_decl, const char* indent);
void print_label_decl_stmt(LabelDecl* label_decl, const char* indent);
void print_member(Member* member, const char* indent);
void print_enum_decl(EnumDecl* enum_decl, const char* indent);
void print_union_decl(UnionDecl* union_decl, const char* indent);
void print_struct_decl(StructDecl* struct_decl, const char* indent);
void print_type_decl_stmt(TypeDecl* type_decl, const char* indent);
void print_do_loop(DoLoop* do_loop, const char* indent);
void print_for_loop(ForLoop* for_loop, const char* indent);
void print_while_loop(WhileLoop* while_loop, const char* indent);
void print_loop_stmt(LoopStmt* loop_stmt, const char* indent);
void print_expr_stmt(ExprStmt* expr_stmt, const char* indent);
void print_empty_stmt(EmptyStmt* empty_stmt, const char* indent);
void print_if_stmt(IfStmt* if_stmt, const char* indent);
void print_jump_stmt(JumpStmt* jump_stmt, const char* indent);
void print_case_stmt(Case* case_stmt, const char* indent);
void print_switch_stmt(SwitchStmt* switch_stmt, const char* indent);
void print_import_stmt(ImportStmt* import_stmt, const char* indent);
void print_stmt(Stmt* stmt, const char* indent);

#endif // _ALMC_AST_CONSOLE_PRINTER