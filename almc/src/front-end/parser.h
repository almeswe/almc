#ifndef ALMC_PARSER_H
#define ALMC_PARSER_H

#include "lexer.h"
#include "ast\ast.h"

#include "..\utils\os.h"

#define ALMC_FILE_EXTENSION ".almc"

typedef struct Parser
{
	char* file;
	Token** tokens;
	uint32_t token_index;
} Parser;

Parser* parser_new(char* file, Token** tokens);
void parser_free(Parser* parser);

AstRoot* parse(Parser* parser);
Expr* parse_expr(Parser* parser);
Expr* parse_paren_expr(Parser* parser);

Type* parse_type_name(Parser* parser);

Expr* parse_primary_expr(Parser* parser);
Expr* parse_postfix_expr(Parser* parser);
Expr* parse_unary_expr(Parser* parser);
Expr* parse_cast_expr(Parser* parser);
Expr* parse_sizeof_expr(Parser* parser);

Expr* parse_mul_arith_expr(Parser* parser);
Expr* parse_add_arith_expr(Parser* parser);
Expr* parse_sft_expr(Parser* parser);
Expr* parse_rel_expr(Parser* parser);
Expr* parse_equ_expr(Parser* parser);

Expr* parse_and_bw_expr(Parser* parser);
Expr* parse_xor_bw_expr(Parser* parser);
Expr* parse_or_bw_expr(Parser* parser);

Expr* parse_and_lg_expr(Parser* parser);
Expr* parse_or_lg_expr(Parser* parser);

Expr* parse_conditional_expr(Parser* parser);
Expr* parse_constant_expr(Parser* parser);
Expr* parse_assignment_expr(Parser* parser);

Expr* parse_initializer_expr(Parser* parser);

Stmt* parse_stmt(Parser* parser);
Stmt* parse_type_decl_stmt(Parser* parser);
Stmt* parse_enum_decl_stmt(Parser* parser);
Stmt* parse_union_decl_stmt(Parser* parser);
Stmt* parse_struct_decl_stmt(Parser* parser);

Stmt* parse_block(Parser* parser);
Stmt* parse_expr_stmt(Parser* parser);
Stmt* parse_empty_stmt(Parser* parser);
Stmt* parse_var_decl_stmt(Parser* parser);
Stmt* parse_func_decl_stmt(Parser* parser);
Stmt* parse_label_decl_stmt(Parser* parser);

Stmt* parse_loop_stmt(Parser* parser);
Stmt* parse_do_loop_stmt(Parser* parser);
Stmt* parse_for_loop_stmt(Parser* parser);
Stmt* parse_while_loop_stmt(Parser* parser);

Stmt* parse_if_stmt(Parser* parser);
ElseIf* parse_elif_stmt(Parser* parser);

Stmt* parse_jump_stmt(Parser* parser);

Case* parse_case_stmt(Parser* parser);
Block* parse_case_block(Parser* parser);
Stmt* parse_switch_stmt(Parser* parser);

void clear_imported_modules();
char is_module_imported(const char* module);
AstRoot* parse_module(const char* module_path);
Stmt* parse_import_stmt(Parser* parser);
char* parse_import_path_desc(Parser* parser);
Stmt* parse_from_import_stmt(Parser* parser);
Stmt* parse_from_import_member_stmt(Parser* parser, AstRoot* import_module);

TypeVar* parse_type_var(Parser* parser);

#endif