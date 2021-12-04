#include "flow-checker.h"

int32_t check_flow_scope(Block* scope)
{
	int32_t scope_resolved = 0;
	int32_t branches_resolved = 0;
	int32_t first_branch_met = 0;

	for (size_t i = 0; i < sbuffer_len(scope->stmts); i++)
	{
		Stmt* stmt = scope->stmts[i];
		switch (stmt->kind)
		{
		case STMT_IF:
			if (!first_branch_met)
				first_branch_met = branches_resolved = 1;
			branches_resolved &= check_flow_if_stmt(
				stmt->if_stmt); break;
		case STMT_JUMP:
			scope_resolved =
				stmt->jump_stmt->kind == JUMP_RETURN;
			break;
		}
	}
	return branches_resolved || scope_resolved;
}

int32_t check_flow_if_stmt(IfStmt* if_stmt)
{
	int32_t stmt_resolved = check_flow_scope(
		if_stmt->if_body);

	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		stmt_resolved &= check_flow_scope(
			if_stmt->elifs[i]->elif_body);

	if (!if_stmt->else_body)
		stmt_resolved = 0;
	else
		stmt_resolved &= check_flow_scope(
			if_stmt->else_body);

	return stmt_resolved;
}

int32_t check_flow_loop_stmt(LoopStmt* loop_stmt)
{
	return 0; //
	switch (loop_stmt->kind)
	{
	case LOOP_DO:
		return check_flow_scope(
			loop_stmt->do_loop->do_body);
	case LOOP_FOR:
		return check_flow_scope(
			loop_stmt->for_loop->for_body);
	case LOOP_WHILE:
		return check_flow_scope(
			loop_stmt->while_loop->while_body);
	}
	return 0;
}

int32_t check_flow_switch_stmt(SwitchStmt* switch_stmt)
{
	int32_t stmt_resolved = 0;

	for (size_t i = 0; i < sbuffer_len(switch_stmt->switch_cases); i++)
		if (switch_stmt->switch_cases[i]->case_body)
			stmt_resolved &= check_flow_scope(
				switch_stmt->switch_cases[i]->case_body);

	if (switch_stmt->switch_default)
		stmt_resolved &= check_flow_scope(
			switch_stmt->switch_default);

	return stmt_resolved;
}

void check_func_return_flow(FuncDecl* func_decl)
{
	if (!IS_VOID_TYPE(func_decl->func_type))
		if (!check_flow_scope(func_decl->func_body))
			report_error(frmt("Not every code flow in function \'%s\' returns value.",
				func_decl->func_name->svalue), func_decl->func_name->context);
}