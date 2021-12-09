#include "stack-frame.h"

StackFrame* stack_frame_new(FuncDecl* func)
{
	StackFrame* frame = new_s(StackFrame, frame);
	frame->locals = NULL;
	frame->local_offsets = NULL;
	frame->arguments = NULL;
	frame->argument_offsets = NULL;
	frame->regtable = regtable_new();
	frame->label_counter = 0;
	frame->func_name = func->func_name->svalue;
	//frame->return_stmt_mentioned = 0;
	//frame->expr_gen = expr_gen_new(frame->regtable);
	for (size_t i = 0; i < sbuffer_len(func->func_params); i++)
		add_argument(func->func_params[i], frame);
	return frame;
}

int get_local_by_name(const char* name, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(frame->locals); i++)
		if (strcmp(frame->locals[i]->type_var->var, name) == 0)
			return i;
	return -1;
}

int get_argument_by_name(const char* name, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(frame->arguments); i++)
		if (strcmp(frame->arguments[i]->var, name) == 0)
			return i;
	return -1;
}

void add_local(VarDecl* local, StackFrame* frame)
{
	size_t offset = get_type_size(local->type_var->type);
	for (size_t i = 0; i < sbuffer_len(frame->local_offsets); i++)
		offset += get_type_size(frame->locals[i]->type_var->type);
	sbuffer_add(frame->local_offsets, offset);
	sbuffer_add(frame->locals, local);
}

void add_argument(TypeVar* argument, StackFrame* frame)
{
	size_t offset = get_type_size(argument->type) + 4;
	for (size_t i = 0; i < sbuffer_len(frame->argument_offsets); i++)
		offset += get_type_size(frame->arguments[i]->type);
	sbuffer_add(frame->argument_offsets, offset);
	sbuffer_add(frame->arguments, argument);
}

char* get_current_label(StackFrame* frame)
{
	return frmt("$LN%d@%s", frame->label_counter,
		frame->func_name);
}

char* increase_label_counter(StackFrame* frame)
{
	return frame->label_counter += 1, 
		get_current_label(frame);
}