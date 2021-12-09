#include "gen.h"

static AsmProgram* program;

#define REGISTERS program->table
#define PROGRAM_ADD_PROC(proc) \
	sbuffer_add(program->code->procs, proc)
#define PROGRAM_SET_ENTRY(name) \
	program->entry = name

#define PROC_CODE_LINE0(c) \
	sbuffer_add(proc->lines, codeline_new(c, NULL, NULL))
#define PROC_CODE_LINE1(c, arg1) \
	sbuffer_add(proc->lines, codeline_new(c, arg1, NULL))
#define PROC_CODE_LINE2(c, arg1, arg2) \
	sbuffer_add(proc->lines, codeline_new(c, arg1, arg2))

void gen_jump_stmt(JumpStmt* jump_stmt, StackFrame* frame)
{
	switch (jump_stmt->kind)
	{
	case JUMP_RETURN:
		if (jump_stmt->additional_expr)
			gen_expr(jump_stmt->additional_expr, frame);
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
	add_local(var_decl, frame);
	int size = get_type_size(
		var_decl->type_var->type);
	int prefix = get_type_prefix(
		var_decl->type_var->type);
	int index = get_local_by_name(
		var_decl->type_var->var, frame);

	SUB32(get_register_str(ESP), frmt("%d", size));
	if (var_decl->var_init)
	{
		//todo: extend to x64 + structs
		gen_expr(var_decl->var_init, frame);
		OUT(frmt("mov  %s PTR [ebp-%d], %s", get_predefined_type_str(prefix),
			frame->local_offsets[index], get_register_str(EAX)));
		unreserve_register(frame->regtable, EAX);
	}
}

void gen_stmt(Stmt* stmt, StackFrame* frame)
{
	switch (stmt->kind)
	{
	case STMT_EXPR:
		gen_expr(stmt->expr_stmt->expr, frame);
		break;
	case STMT_JUMP:
		gen_jump_stmt(stmt->jump_stmt, frame);
		break;
	case STMT_VAR_DECL:
		gen_var_decl_stmt(stmt->var_decl, frame);
		break;
	case STMT_FUNC_DECL:
		gen_func_decl_stmt(stmt->func_decl);
		break;
	default:
		assert(0);
	}
}

void gen_block(Block* block, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++)
		gen_stmt(block->stmts[i], frame);
}

void gen_func_decl_stmt(FuncDecl* func_decl)
{
	AsmCodeProc* proc = proc_new(func_decl);
	PROGRAM_ADD_PROC(proc);

	// function prologue
	reserve_register(REGISTERS, ESP);
	reserve_register(REGISTERS, EBP);
	PROC_CODE_LINE1("push", get_register_str(EBP));
	PROC_CODE_LINE2("mov",  get_register_str(EBP), 
		get_register_str(ESP));
	//------------------
	if (func_decl->func_spec.is_entry)
		PROGRAM_SET_ENTRY(proc->name);
	gen_block(func_decl->func_body, proc->frame);
	
	// function epilogue
	unreserve_register(REGISTERS, ESP);
	unreserve_register(REGISTERS, EBP);
	if (IS_VOID_TYPE(func_decl->func_type))
	{
		PROC_CODE_LINE2("mov", get_register_str(ESP),
			get_register_str(EBP));
		PROC_CODE_LINE1("pop", get_register_str(EBP));
		PROC_CODE_LINE0("ret");
	}
	//------------------
}

AsmProgram* gen(AstRoot* ast)
{
	program = program_new();
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		gen_global_stmt(ast->stmts[i]);
	return program;
}

void gen_global_stmt(Stmt* stmt)
{
	switch (stmt->kind)
	{
	case STMT_FUNC_DECL:
		gen_func_decl_stmt(stmt->func_decl);
		break;
	default:
		assert(0);
	}
}