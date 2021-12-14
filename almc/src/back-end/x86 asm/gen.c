#include "gen.h"

void gen_jump_stmt(JumpStmt* jump_stmt, StackFrame* frame)
{
	switch (jump_stmt->kind)
	{
	case JUMP_RETURN:
		if (jump_stmt->additional_expr)
			gen_expr32(jump_stmt->additional_expr, frame);
		MOV32(get_register_str(ESP), get_register_str(EBP));
		POP32(get_register_str(EBP));
		OUT("ret");
		break;
	case JUMP_GOTO:
		OUT(frmt("jmp  %s", 
			jump_stmt->additional_expr->idnt->svalue));
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

void gen_stmt(Stmt* stmt, StackFrame* frame)
{
	switch (stmt->kind)
	{
	case STMT_EXPR:
		gen_expr32(stmt->expr_stmt->expr, frame);
		break;
	/*case STMT_JUMP:
		gen_jump_stmt(stmt->jump_stmt, frame);
		break;*/
	case STMT_VAR_DECL:
		gen_var_decl_stmt(stmt->var_decl, frame);
		break;
	/*case STMT_FUNC_DECL:
		gen_func_decl_stmt(stmt->func_decl);
		break;*/
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
	if (proc->frame->required_space_for_locals + 4 != 0)
		codeline_free(proc->lines[2]), proc->lines[2] = codeline_new(SUB, get_register_str(ESP),
			frmt("%d", -(proc->frame->required_space_for_locals + 4)));
}

void gen_sdk_package(AsmCodeProc* proc, FuncDecl* func_decl)
{
	proc->is_external = true;
	sbuffer_add(program->incs, frmt("%s%s",
		"e:\\masm32\\include\\", func_decl->spec.from));
	sbuffer_add(program->libs, frmt("%s%s",
		"e:\\masm32\\lib\\", func_decl->spec.from));
}

void gen_func_decl_stmt(FuncDecl* func_decl)
{
	AsmCodeProc* proc = proc_new(func_decl);
	PROGRAM_ADD_PROC(proc);

	if (func_decl->spec.is_from_sdk)
		return gen_sdk_package(proc, func_decl);

	// function prologue
	reserve_register(REGISTERS, ESP);
	reserve_register(REGISTERS, EBP);
	PROC_CODE_LINE1(PUSH, get_register_str(EBP));
	PROC_CODE_LINE2(MOV,  get_register_str(EBP), 
		get_register_str(ESP));

	// require space for stack allocation command
	PROC_CODE_LINE0(NOP);
	//------------------
	if (func_decl->spec.is_entry)
		PROGRAM_SET_ENTRY(proc->name);
	gen_block(func_decl->body, proc->frame);
	
	// insert command for stack allocation
	gen_stack_space_alloc(proc);

	// function epilogue
	unreserve_register(REGISTERS, ESP);
	unreserve_register(REGISTERS, EBP);
	if (IS_VOID_TYPE(func_decl->type))
	{
		PROC_CODE_LINE2(MOV, get_register_str(ESP),
			get_register_str(EBP));
		PROC_CODE_LINE1(POP, get_register_str(EBP));
		PROC_CODE_LINE1(RET, frmt("%d", proc->frame->required_space_for_arguments - 4));
	}
	//------------------
}

AsmProgram* gen(AstRoot* ast, Table* table)
{
	program = program_new(table);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		gen_global_stmt(ast->stmts[i]);
	return program;
}

void gen_global_stmt(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_EMPTY:
	case STMT_TYPE_DECL:
		break;
	case STMT_FUNC_DECL:
		gen_func_decl_stmt(stmt->func_decl);
		break;
	default:
		assert(0);
	}
}