#ifndef ALMC_CONST_EXPRESSION_EVALUATOR
#define ALMC_CONST_EXPRESSION_EVALUATOR

#include "ast.h"

//todo: add cast processing

int64_t evaluate_expr_itype(Expr* expr);
double  evaluate_expr_ftype(Expr* expr);

#endif 