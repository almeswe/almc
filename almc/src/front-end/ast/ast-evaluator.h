#ifndef ALMC_CONST_EXPRESSION_EVALUATOR
#define ALMC_CONST_EXPRESSION_EVALUATOR

#include "ast.h"

#include <float.h>
#include <limits.h>

#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

int64_t evaluate_expr_itype(Expr* expr);
double  evaluate_expr_ftype(Expr* expr);

bool value_in_bounds_of_type(Type* type, double value);

#endif 