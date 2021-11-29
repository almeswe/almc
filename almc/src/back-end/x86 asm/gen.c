#include "gen.h"

void gen_jump_stmt(JumpStmt* jump_stmt, StackFrame* frame)
{
	switch (jump_stmt->kind)
	{
	case JUMP_RETURN:
		if (jump_stmt->additional_expr)
			gen_expr2(jump_stmt->additional_expr, frame);
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
		gen_expr2(var_decl->var_init, frame);
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
		gen_expr2(stmt->expr_stmt->expr, frame);
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
	printf(frmt("%s proc\n", func_decl->func_name->svalue));

	// header
	PUSH32(get_register_str(ESP));
	MOV32(get_register_str(EBP), get_register_str(ESP));

	StackFrame* frame = stack_frame_new(func_decl);
	// body
	gen_block(func_decl->func_body, frame);
	//todo: free stack frame
	
	// footer
	//if (frame->return_stmt_mentioned)
	//{
		MOV32(get_register_str(ESP), get_register_str(EBP));
		POP32(get_register_str(EBP));
		OUT("ret");
	//}
	printf(frmt("%s endp\n", func_decl->func_name->svalue));
}