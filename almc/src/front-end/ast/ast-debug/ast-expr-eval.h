#ifndef ALMC_AST_EXPR_PRECENDENCE_EVALUATOR
#define ALMC_AST_EXPR_PRECENDENCE_EVALUATOR

#include "..\ast.h"

int32_t eval_expr(Expr* expr);
int32_t eval_ast(AstRoot* ast);
int32_t eval_const(Const* cnst);
int32_t eval_uexpr(UnaryExpr* uexpr);
int32_t eval_bexpr(BinaryExpr* bexpr);
int32_t eval_texpr(TernaryExpr* texpr);

#endif //ALMC_AST_EXPR_PRECENDENCE_EVALUATOR