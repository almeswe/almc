#ifndef ALMC_CONST_EXPRESSION_EVALUATOR_H
#define ALMC_CONST_EXPRESSION_EVALUATOR_H

#include "ast.h"

#include <float.h>
#include <limits.h>

#define IN_BOUNDS_OF(ubound, bbound, value) \
	(((value) <= (ubound)) && ((value) >= (bbound)))

//todo: fix bug with cast: cast(i8)(1 << 2) etc...

int64_t evaluate_expr_itype(Expr* expr);
double  evaluate_expr_ftype(Expr* expr);

bool value_in_bounds_of_type(Type* type, double value);

#endif 