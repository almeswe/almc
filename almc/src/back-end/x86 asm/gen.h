#include <stdio.h>
#include <assert.h>

#include "..\..\utils\common.h"
#include "../../front-end/ast/ast.h"

void init_reserved_registers();
int get_unreserved_register();
void reserve_register(int reg);
void unreserve_register(int reg);

void gen_const(Const* cnst, char* to_reg);
void gen_unary_expr(Expr* expr);
void gen_binary_expr(BinaryExpr* expr);
void gen_expr(Expr* expr);