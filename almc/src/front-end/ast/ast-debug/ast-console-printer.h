#ifndef ALMC_AST_CONSOLE_PRINTER
#define ALMC_AST_CONSOLE_PRINTER

#include <assert.h>
#include "..\ast.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      
#define RED     "\033[31m"     
#define GREEN   "\033[32m"     
#define YELLOW  "\033[33m"      
#define BLUE    "\033[34m"     
#define MAGENTA "\033[35m"      
#define CYAN    "\033[36m"     
#define WHITE   "\033[37m"      
#define BOLDBLACK   "\033[1m\033[30m"      
#define BOLDRED     "\033[1m\033[31m"      
#define BOLDGREEN   "\033[1m\033[32m"      
#define BOLDYELLOW  "\033[1m\033[33m"      
#define BOLDBLUE    "\033[1m\033[34m"     
#define BOLDMAGENTA "\033[1m\033[35m"      
#define BOLDCYAN    "\033[1m\033[36m"     
#define BOLDWHITE   "\033[1m\033[37m" 

void print_expr(Expr* expr, const char* indent);
void print_stmt(Stmt* stmt, const char* indent);

void print_ast(AstRoot* ast);

void print_type(Type* type, const char* indent);
void print_idnt(Idnt* idnt, const char* indent);
void print_str(Str* str, const char* indent);
void print_const(Const* cnst, const char* indent);
void print_func_call(FuncCall* func_call, const char* indent);
void print_unary_expr(UnaryExpr* expr, const char* indent);
void print_binary_expr(BinaryExpr* expr, const char* indent);
void print_ternary_expr(TernaryExpr* expr, const char* indent);
void print_initializer(Initializer* initializer, const char* indent);
void print_expr(Expr* expr, const char* indent);
void print_type_var(TypeVar* type_var, const char* indent);
void print_block(Block* block, const char* indent);
void print_var_decl(VarDecl* var_decl, const char* indent);
void print_func_decl(FuncDecl* func_decl, const char* indent);
void print_label_decl(LabelDecl* label_decl, const char* indent);
void print_enum_decl(EnumDecl* enum_decl, const char* indent);
void print_union_decl(UnionDecl* union_decl, const char* indent);
void print_struct_decl(StructDecl* struct_decl, const char* indent);
void print_type_decl(TypeDecl* type_decl, const char* indent);
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

#endif //ALMC_AST_CONSOLE_PRINTER