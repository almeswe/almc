#ifndef _ALMC_CONST_EXPRESSION_EVALUATOR_H
#define _ALMC_CONST_EXPRESSION_EVALUATOR_H

#include "ast.h"

#include <float.h>
#include <limits.h>

#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

extern Type* retrieve_expr_type(Expr* expr);
extern SrcArea* get_expr_area(Expr* expr);

double evaluate_expr(Expr* expr);
int64_t evaluate_expr_itype(Expr* expr);
double  evaluate_expr_ftype(Expr* expr);

bool value_in_bounds_of_type(Type* type, double value);

#endif // _ALMC_CONST_EXPRESSION_EVALUATOR_H