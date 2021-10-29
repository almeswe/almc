#include "gen.h"

void gen_var_decl(VarDecl* var_decl, StackFrame* frame)
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
	switch (stmt->type)
	{
	case STMT_EXPR:
		gen_expr2(stmt->expr_stmt->expr, frame);
		break;
	case STMT_VAR_DECL:
		gen_var_decl(stmt->var_decl, frame);
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

void gen_func_decl(FuncDecl* func_decl)
{
	OUT(frmt("%s proc", func_decl->func_name));

	// header
	PUSH32(get_register_str(ESP));
	MOV32(get_register_str(EBP), get_register_str(ESP));

	StackFrame* frame = stack_frame_new(func_decl);
	// body
	gen_block(func_decl->func_body, frame);
	//todo: free stack frame
	
	// footer
	MOV32(get_register_str(ESP), get_register_str(EBP));
	POP32(get_register_str(EBP));

	OUT(frmt("%s endp", func_decl->func_name));
}