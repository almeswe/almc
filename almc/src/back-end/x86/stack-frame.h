#ifndef ALMC_BACK_END_STACK_FRAME
#define ALMC_BACK_END_STACK_FRAME

#include "x86types.h"
#include "regtable.h"
#include "..\..\front-end\ast\ast.h"

typedef struct x86_AsmCodeProc AsmCodeProc;

typedef enum x86_StackFrameEntityKind
{
	STACK_FRAME_ENTITY_LOCAL,
	STACK_FRAME_ENTITY_ARGUMENT,
} StackFrameEntityKind;

typedef struct x86_StackFrameEntity
{
	Type* type;
	int32_t offset;
	char* definition;
	StackFrameEntityKind kind;
} StackFrameEntity;

typedef struct x86_StackFrame
{
	StackFrameEntity** entities;

	AsmCodeProc* of_proc;

	int32_t required_space_for_locals;
	int32_t required_space_for_arguments;

	bool return_stmt_mentioned;
	uint32_t label_counter;
} StackFrame;

StackFrame* stack_frame_new(FuncDecl* func);
StackFrameEntity* stack_frame_entity_new(Type* type, uint32_t offset, 
	char* definition, StackFrameEntityKind kind);

StackFrameEntity* get_entity_by_name(const char* name, StackFrame* frame);

StackFrameEntity* get_local_by_name(const char* name, StackFrame* frame);
StackFrameEntity* get_argument_by_name(const char* name, StackFrame* frame);

StackFrameEntity* add_local(VarDecl* local, StackFrame* frame);
StackFrameEntity* add_argument(TypeVar* argument, StackFrame* frame);

#endif