#include "lexer.h"
#include "parser.h"

//todo: add DEBUG MACRO for in/ex cluding this two headers
//#ifdef ALMC_DEBUG
	#include "ast\ast-debug\ast-expr-eval.h"
	#include "ast\ast-debug\ast-console-printer.h"
//#endif

#include "semantics/table.h"
#include "semantics/visitor.h"