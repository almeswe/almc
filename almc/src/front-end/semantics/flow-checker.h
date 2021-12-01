#ifndef ALMC_RETURN_FLOW_CHECKER_H
#define ALMC_RETURN_FLOW_CHECKER_H

/*
	Module responsible for checking that the all code paths
	return something
*/

#include "type-checker.h"

int32_t check_flow_scope(Block* scope);
int32_t check_flow_if_stmt(IfStmt* if_stmt);
int32_t check_flow_loop_stmt(LoopStmt* loop_stmt);
int32_t check_flow_switch_stmt(SwitchStmt* switch_stmt);

void check_func_return_flow(FuncDecl* func_decl);

#endif