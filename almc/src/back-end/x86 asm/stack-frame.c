#include "stack-frame.h"

StackFrame* stack_frame_new(FuncDecl* func)
{
	StackFrame* frame = cnew_s(StackFrame, frame, 1);
	frame->required_space_for_locals = -4;
	frame->required_space_for_arguments = 4;
	for (size_t i = 0; i < sbuffer_len(func->params); i++)
		add_argument(func->params[i], frame);
	return frame;
}

StackFrameEntity* stack_frame_entity_new(Type* type, uint32_t offset, char* definition,
	StackFrameEntityKind kind)
{
	StackFrameEntity* entity = cnew_s(StackFrameEntity, entity, 1);
	entity->type = type;
	entity->kind = kind;
	entity->offset = offset;
	entity->definition = definition;
	return entity;
}

StackFrameEntity* get_entity_by_name(const char* name, StackFrame* frame)
{
	char* definition = frmt("_%s$", name);
	for (size_t i = 0; i < sbuffer_len(frame->entities); i++)
		if (strcmp(frame->entities[i]->definition, definition) == 0)
			return free(definition), frame->entities[i];
	return free(definition), NULL;
}

StackFrameEntity* get_local_by_name(const char* name, StackFrame* frame)
{
	char* definition = frmt("_%s$", name);
	for (size_t i = 0; i < sbuffer_len(frame->entities); i++)
		if (strcmp(frame->entities[i]->definition, definition) == 0)
			return free(definition), frame->entities[i];
	return free(definition), NULL;
}

StackFrameEntity* get_argument_by_name(const char* name, StackFrame* frame)
{
	char* definition = frmt("_%s$", name);
	for (size_t i = 0; i < sbuffer_len(frame->entities); i++)
		if (strcmp(frame->entities[i]->definition, name) == 0)
			return free(definition), frame->entities[i];
	return free(definition), NULL;
}

StackFrameEntity* add_local(VarDecl* local, StackFrame* frame)
{
	char* definition = frmt("_%s$", local->type_var->var);
	StackFrameEntity* entity = stack_frame_entity_new(local->type_var->type, 
		frame->required_space_for_locals, definition, STACK_FRAME_ENTITY_LOCAL);
	frame->required_space_for_locals -= local->type_var->type->size;
	// if is aggregate type, need to specify the end of offset, because
	// it will be addressed from the end, not the beginning (for more simplicily)
	if (IS_ARRAY_TYPE(local->type_var->type))
		entity->offset = frame->required_space_for_locals + 
			local->type_var->type->base->size;
	if (IS_STRUCT_OR_UNION_TYPE(local->type_var->type))
		entity->offset = frame->required_space_for_locals + 1;
	sbuffer_add(frame->entities, entity);
	return entity;
}

StackFrameEntity* add_argument(TypeVar* argument, StackFrame* frame)
{
	char* definition = frmt("_%s$", argument->var);
	StackFrameEntity* entity = stack_frame_entity_new(argument->type,
		frame->required_space_for_arguments, definition, STACK_FRAME_ENTITY_ARGUMENT);
	frame->required_space_for_arguments += argument->type->size;
	sbuffer_add(frame->entities, entity);
	return entity;
}

char* get_current_label(StackFrame* frame)
{
	assert(0);
	return frmt("$LN%d@%s", frame->label_counter,
		frame->of_proc);
}

char* increase_label_counter(StackFrame* frame)
{
	return frame->label_counter += 1, 
		get_current_label(frame);
}