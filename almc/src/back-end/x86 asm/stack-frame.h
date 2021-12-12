#ifndef ALMC_BACK_END_STACK_FRAME
#define ALMC_BACK_END_STACK_FRAME

#include "x86types.h"
#include "regtable.h"
#include "..\..\front-end\ast\ast.h"

typedef struct x86_AsmCodeProc AsmCodeProc;

typedef enum x86_StackFrameEntityKind
{
	
} StackFrameEntityKind;

typedef struct x86_StackFrameEntity
{
	int offset;
	char* definition;
	StackFrameEntityKind kind;
	union
	{
		VarDecl* local;
		TypeVar* argument;
	};
} StackFrameEntity;

typedef struct x86_StackFrame
{
	VarDecl** locals;    // local variables which were declared in this stack frame
	TypeVar** arguments; // function arguments which were passed in this function
	
	RegisterTable* regtable;
	StackFrameEntity** entities;

	AsmCodeProc* of_proc;

	int* local_offsets;    // offset for each local variable in this stack frame
	int* argument_offsets; // offset for each function argument in this stack frame

	int required_space_for_locals;
	int required_space_for_arguments;

	char* func_name;
	char return_stmt_mentioned;
	uint64_t label_counter;
} StackFrame;

StackFrame* stack_frame_new(FuncDecl* func);
int get_local_by_name(const char* name, StackFrame* frame);
int get_argument_by_name(const char* name, StackFrame* frame);

void add_local(VarDecl* local, StackFrame* frame);
void add_argument(TypeVar* argument, StackFrame* frame);

#endif