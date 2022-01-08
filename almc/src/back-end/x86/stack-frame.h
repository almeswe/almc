#ifndef ALMC_BACK_END_STACK_FRAME_H
#define ALMC_BACK_END_STACK_FRAME_H

#include "regtable.h"
#include "..\..\front-end\ast\ast.h"

typedef struct x86_AsmCodeProc AsmCodeProc;
typedef struct x86_AsmCodeProtoProc AsmCodeProtoProc;

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

	char* loop_break_label;
	char* loop_continue_label;

	char* return_label;
	// needed to determine the need of jump to 
	// origin ret by return_label
	bool  return_declared;
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