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
	long suitable_count = 1;
	bool suitable_found = false;
	StackFrameEntity* suitable = NULL;
	char* definition = frmt("_%s$", name);

	/*
		The reason why loop located here is because in case of variables with the same
		name but in different scopes (of the same level of course) and with the same parent scope,
		creates the problem of local's name collision.
		To solve it we need to append the number to the end of local variable, and then when we need
		to retrieve the entity by name, we list all variables available in this scope,
		with appropriate number at the end. (which we increment inside loop)
		The last suitable entity will be which we wanted.
	*/

	while (!suitable_found)
	{
		suitable_found = true;
		for (size_t i = 0; i < sbuffer_len(frame->entities); i++)
			if (strcmp(frame->entities[i]->definition, definition) == 0)
				suitable_count++, suitable_found = false, suitable = frame->entities[i],
					free(definition), definition = frmt("_%s%d$", name, suitable_count);
	}

	return free(definition), suitable;
}

char* get_local_definition(VarDecl* local, StackFrame* frame)
{
	long suitable_count = 1;
	bool suitable_found = false;
	char* definition = frmt("_%s$", local->type_var->var);

	/*
		The same reason of why loop located here like in get_entity_by_name.
		But here we just return the definition, not entity.
	*/

	while (!suitable_found)
	{
		suitable_found = true;
		for (size_t i = 0; i < sbuffer_len(frame->entities); i++)
			if (strcmp(frame->entities[i]->definition, definition) == 0)
				suitable_count++, suitable_found = false, free(definition),
					definition = frmt("_%s%d$", local->type_var->var, suitable_count);
	}
	return definition;
}

StackFrameEntity* add_local(VarDecl* local, StackFrame* frame)
{
	char* definition = get_local_definition(local, frame);
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
		+= IS_AGGREGATE_TYPE(argument->type) ? 
			MACHINE_WORD :argument->type->size;
	sbuffer_add(frame->entities, entity);
	return entity;
}