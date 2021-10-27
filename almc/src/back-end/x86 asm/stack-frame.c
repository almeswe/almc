#include "stack-frame.h"

StackFrame* create_new_stack_frame(TypeVar** arguments)
{
	StackFrame* frame = new_s(StackFrame, frame);
	frame->locals = NULL;
	frame->local_offsets = NULL;
	frame->arguments = arguments;
	frame->argument_offsets = NULL;
	frame->regtable = regtable_new();
	frame->expr_gen = expr_gen_new(frame->regtable);
	for (size_t i = 0; i < sbuffer_len(frame->arguments); i++)
		sbuffer_add(frame->argument_offsets,
			get_type_size(frame->arguments[i]->type));
	return frame;
}

int get_local_by_name(const char* name, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(frame->locals); i++)
		if (strcmp(frame->locals[i]->type_var->var, name))
			return i;
	return -1;
}

int get_argument_by_name(const char* name, StackFrame* frame)
{
	for (size_t i = 0; i < sbuffer_len(frame->arguments); i++)
		if (strcmp(frame->arguments[i]->var, name))
			return i;
	return -1;
}