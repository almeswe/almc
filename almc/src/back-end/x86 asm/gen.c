#include "gen.h"

int get_local_offset(const char* local_name, StackFrame* frame)
{
//	for (size_t i = 0; i < sbuffer_len(frame->local_variables); i++)
}

void gen_var_decl(VarDecl* var_decl, StackFrame* frame)
{
	int size = get_type_size(var_decl->type_var->type);
	int prefix = get_type_prefix(var_decl->type_var->type);

	SUB(get_register_str(ESP), frmt("%d", size));
	if (var_decl->var_init)
	{
		//todo: extend to x64 + structs
		gen_expr(frame->expr_gen, var_decl->var_init);
		OUT(frmt("mov %s PTR [ebp-%d], %s", get_predefined_type_str(prefix), size,
			get_register_str(EAX)));
		unreserve_register(frame->regtable, EAX);
	}
}

void gen_func_decl(FuncDecl* func_decl)
{
	OUT(frmt("%s proc", func_decl->func_name));

	// header
	PUSH(get_register_str(ESP));
	MOV(get_register_str(EBP), get_register_str(ESP));

	// body

	// footer
	MOV(get_register_str(ESP), get_register_str(EBP));
	POP(get_register_str(EBP));

	OUT(frmt("%s endp", func_decl->func_name));
}