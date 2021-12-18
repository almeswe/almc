#include "gen.h"

void gen_elif_stmt(ElseIf* elseif_stmt, char* label_elif_end, 
	char* label_final, StackFrame* frame)
{
	gen_expr32(elseif_stmt->cond, frame);
	unreserve_register(REGISTERS, EAX);
	char* eax_reg_arg = get_register_str(EAX);

	// structure of elif is the same as if part
	PROC_CODE_LINE2(CMP, eax_reg_arg, frmt("%d", 0));
	PROC_CODE_LINE1(JE, label_elif_end);
	gen_block(elseif_stmt->body, frame);
	PROC_CODE_LINE1(JMP, label_final);
	PROC_CODE_LINE1(_LABEL, label_elif_end);
}

void gen_if_stmt(IfStmt* if_stmt, StackFrame* frame)
{
	gen_expr32(if_stmt->cond, frame);
	unreserve_register(REGISTERS, EAX);

	char* eax_reg_arg = get_register_str(EAX);

	// reserving label which maps to the end of the if body
	NEW_LABEL(label_if_branch_end);
	// reserving labels for each elif's body ending
	char** labels_elif_branches_end = NULL;
	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		ADD_NEW_LABEL(labels_elif_branches_end);
	// reserving label which maps to the end of all if statement
	NEW_LABEL(label_final);

	// if part
	PROC_CODE_LINE2(CMP, eax_reg_arg, frmt("%d", 0));
	PROC_CODE_LINE1(JE, label_if_branch_end);
	gen_block(if_stmt->body, frame);
	PROC_CODE_LINE1(JMP, label_final);
	PROC_CODE_LINE1(_LABEL, label_if_branch_end);
	// elifs part ?
	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		gen_elif_stmt(if_stmt->elifs[i], labels_elif_branches_end[i], 
			label_final, frame);
	// else part ?
	if (if_stmt->else_body)
		gen_block(if_stmt->else_body, frame);
	PROC_CODE_LINE1(_LABEL, label_final);
	sbuffer_free(labels_elif_branches_end);
}

void gen_loop_stmt(LoopStmt* loop_stmt, StackFrame* frame)
{
	switch (loop_stmt->kind)
	{
	case LOOP_DO:
		return gen_do_loop_stmt(loop_stmt->do_loop, frame);
	case LOOP_FOR:
		return gen_for_loop_stmt(loop_stmt->for_loop, frame);
	case LOOP_WHILE:
		return gen_while_loop_stmt(loop_stmt->while_loop, frame);
	default:
		assert(0);
	}
}

void loop_init_frame_labels(char* label_break,
	char* label_continue, StackFrame* frame)
{
	frame->loop_break_label = label_break;
	frame->loop_continue_label = label_continue;
}

void loop_free_frame_labels(StackFrame* frame)
{
	frame->loop_break_label =
		frame->loop_continue_label = NULL;
}

void gen_do_loop_stmt(DoLoop* do_loop, StackFrame* frame)
{
	NEW_LABEL(label_loop_start);
	NEW_LABEL(label_loop_body_end);
	NEW_LABEL(label_loop_end);
	loop_init_frame_labels(label_loop_end,
		label_loop_body_end, frame);

	char* eax_reg_arg = get_register_str(EAX);

	PROC_CODE_LINE1(_LABEL, label_loop_start);
	gen_block(do_loop->body, frame);
	PROC_CODE_LINE1(_LABEL, label_loop_body_end);
	gen_expr32(do_loop->cond, frame);
	unreserve_register(REGISTERS, EAX);
	PROC_CODE_LINE2(CMP, eax_reg_arg, frmt("%d", 0));
	PROC_CODE_LINE1(JE, label_loop_end);
	PROC_CODE_LINE1(JMP, label_loop_start);
	PROC_CODE_LINE1(_LABEL, label_loop_end);
	loop_free_frame_labels(frame);
}

void gen_for_loop_stmt(ForLoop* for_loop, StackFrame* frame)
{
	if (for_loop->init)
		gen_var_decl_stmt(for_loop->init, frame);
	NEW_LABEL(label_loop_start);
	NEW_LABEL(label_loop_body_end);
	NEW_LABEL(label_loop_end);
	loop_init_frame_labels(label_loop_end,
		label_loop_body_end, frame);
	char* eax_reg_arg = get_register_str(EAX);

	PROC_CODE_LINE1(_LABEL, label_loop_start);
	if (for_loop->cond)
	{
		gen_expr32(for_loop->cond, frame),
			unreserve_register(REGISTERS, EAX);
		PROC_CODE_LINE2(CMP, eax_reg_arg, frmt("%d", 0));
		PROC_CODE_LINE1(JE, label_loop_end);
	}
	gen_block(for_loop->body, frame);
	PROC_CODE_LINE1(_LABEL, label_loop_body_end);
	if (for_loop->step)
		gen_expr32(for_loop->step, frame), 
			unreserve_register(REGISTERS, EAX);
	PROC_CODE_LINE1(JMP, label_loop_start);
	PROC_CODE_LINE1(_LABEL, label_loop_end);
	loop_free_frame_labels(frame);
}

void gen_while_loop_stmt(WhileLoop* while_loop, StackFrame* frame)
{
	NEW_LABEL(label_loop_start);
	NEW_LABEL(label_loop_body_end);
	NEW_LABEL(label_loop_end);
	loop_init_frame_labels(label_loop_end, 
		label_loop_body_end, frame);
	char* eax_reg_arg = get_register_str(EAX);

	PROC_CODE_LINE1(_LABEL, label_loop_start);
	gen_expr32(while_loop->cond, frame);
	unreserve_register(REGISTERS, EAX);
	PROC_CODE_LINE2(CMP, eax_reg_arg, frmt("%d", 0));
	PROC_CODE_LINE1(JE, label_loop_end);
	gen_block(while_loop->body, frame);
	PROC_CODE_LINE1(_LABEL, label_loop_body_end);
	PROC_CODE_LINE1(JMP, label_loop_start);
	PROC_CODE_LINE1(_LABEL, label_loop_end);
	loop_free_frame_labels(frame);
}

void declare_return_stmt(StackFrame* frame)
{
	if (!frame->return_declared)
		frame->return_label = program_new_label(program),
			frame->return_declared = true;
}

void gen_jump_stmt(JumpStmt* jump_stmt, StackFrame* frame)
{
	switch (jump_stmt->kind)
	{
	case JUMP_RETURN:
		// check if return is last statement?
		declare_return_stmt(frame);
		if (jump_stmt->additional_expr)
			gen_expr32(jump_stmt->additional_expr, frame),
				unreserve_register(REGISTERS, EAX);
		PROC_CODE_LINE1(JMP, frame->return_label);
		break;
	case JUMP_GOTO:
		PROC_CODE_LINE1(JMP, frmt("LN_%s", 
			jump_stmt->additional_expr->idnt->svalue));
		break;
	case JUMP_BREAK:
		PROC_CODE_LINE1(JMP, frame->loop_break_label);
		break;
	case JUMP_CONTINUE:
		PROC_CODE_LINE1(JMP, frame->loop_continue_label);
		break;
	default:
		assert(0);
	}
}

void gen_var_decl_stmt(VarDecl* var_decl, StackFrame* frame)
{
	StackFrameEntity* local = 
		add_local(var_decl, frame);
	
	if (var_decl->var_init)
	{
		if (IS_AGGREGATE_TYPE(var_decl->type_var->type))
			assert(0);
		else
		{
			assert(var_decl->type_var->type);
			Type* type = var_decl->type_var->type;

			gen_expr32(var_decl->var_init, frame);
			int reg = 0;
			char* arg1 = frmt("%s ptr %s[ebp]",
				get_ptr_prefix(type), local->definition);
			char* arg2 = get_register_str(
				reg = get_part_of_reg(EAX, type->size * 8));

			PROC_CODE_LINE2(MOV, arg1, arg2);
			unreserve_register(REGISTERS, reg);
		}
	}
}

void gen_label_decl_stmt(LabelDecl* label_decl)
{
	PROC_CODE_LINE1(_LABEL, 
		frmt("LN_%s", label_decl->label->svalue));
}

void gen_stmt(Stmt* stmt, StackFrame* frame)
{
	switch (stmt->kind)
	{
	case STMT_EXPR:
		return gen_expr32(stmt->expr_stmt->expr, frame);
	case STMT_JUMP:
		return gen_jump_stmt(stmt->jump_stmt, frame);
	case STMT_IF:
		return gen_if_stmt(stmt->if_stmt, frame);
	case STMT_VAR_DECL:
		return gen_var_decl_stmt(stmt->var_decl, frame);
	case STMT_LOOP:
		return gen_loop_stmt(stmt->loop_stmt, frame);
	case STMT_EMPTY:
	case STMT_TYPE_DECL:
		return;
	case STMT_LABEL_DECL:
		return gen_label_decl_stmt(stmt->label_decl);
	default:
		assert(0);
	}
}

void gen_block(Block* block, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++)
		gen_stmt(block->stmts[i], frame);
}

void gen_stack_space_alloc(AsmCodeProc* proc)
{
	if (proc->frame->required_space_for_locals != 0)
		codeline_free(proc->lines[2]), proc->lines[2] = codeline_new(SUB, get_register_str(ESP),
			frmt("%d", -(proc->frame->required_space_for_locals)));
}

void gen_proto_proc(FuncDecl* func_decl)
{
	AsmCodeProtoProc* proc = proto_proc_new(func_decl);
	PROGRAM_ADD_PROTO_PROC(proc);
	program_add_lib(program, proc->lib);
}

void gen_func_decl_stmt(FuncDecl* func_decl)
{
	if (func_decl->spec->is_external)
		return gen_proto_proc(func_decl);

	AsmCodeProc* proc = proc_new(func_decl);
	PROGRAM_ADD_PROC(proc);

	// function prologue
	reserve_register(REGISTERS, ESP);
	reserve_register(REGISTERS, EBP);
	PROC_CODE_LINE1(PUSH, get_register_str(EBP));
	PROC_CODE_LINE2(MOV,  get_register_str(EBP), 
		get_register_str(ESP));

	// require space for stack allocation command
	PROC_CODE_LINE0(NOP);
	//------------------
	if (func_decl->spec->is_entry)
		PROGRAM_SET_ENTRY(proc->name);
	gen_block(func_decl->body, proc->frame);
	
	// insert command for stack allocation
	gen_stack_space_alloc(proc);

	// function epilogue
	unreserve_register(REGISTERS, ESP);
	unreserve_register(REGISTERS, EBP);
	// reference to main return routine
	if (proc->frame->return_declared)
		PROC_CODE_LINE1(_LABEL, proc->frame->return_label);
	PROC_CODE_LINE2(MOV, get_register_str(ESP),
		get_register_str(EBP));
	PROC_CODE_LINE1(POP, get_register_str(EBP));
	gen_callee_stack_clearing(func_decl);
	//------------------
}

AsmProgram* gen(AstRoot* ast, Table* table)
{
	program = program_new(table);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		gen_global_stmt(ast->stmts[i]);
	return program;
}

void gen_import_stmt(ImportStmt* import_stmt)
{
	for (size_t i = 0; i < sbuffer_len(import_stmt->ast->stmts); i++)
		gen_global_stmt(import_stmt->ast->stmts[i]);
}

void gen_global_stmt(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_EMPTY:
	case STMT_TYPE_DECL:
		return;
	case STMT_IMPORT:
		return gen_import_stmt(stmt->import_stmt);
	case STMT_FUNC_DECL:
		return gen_func_decl_stmt(stmt->func_decl);
	default:
		assert(0);
	}
}