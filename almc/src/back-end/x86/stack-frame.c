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
	// if is aggregate type, need to specify the end of offset, because
	// it will be addressed from the end, not the beginning (for more simplicily)
	entity->offset = IS_AGGREGATE_TYPE(entity->type) ?
		offset - type->size + 4 : offset;
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
	sbuffer_add(frame->entities, entity);
	return entity;
}

StackFrameEntity* add_argument(TypeVar* argument, StackFrame* frame)
{
	char* definition = frmt("_%s$", argument->var);
	StackFrameEntity* entity = stack_frame_entity_new(argument->type,
		frame->required_space_for_arguments, definition, STACK_FRAME_ENTITY_ARGUMENT);
	entity->offset = frame->required_space_for_arguments
		+= argument->type->size;
	sbuffer_add(frame->entities, entity);
	return entity;
}