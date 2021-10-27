#ifndef ALMC_BACK_END_STACK_FRAME
#define ALMC_BACK_END_STACK_FRAME

#include "types.h"
#include "expr-gen.h"
#include "..\..\front-end\ast\ast.h"

typedef struct x86_StackFrame
{
	VarDecl** locals;    // local variables which were declared in this stack frame
	TypeVar** arguments; // function arguments which were passed in this function
	
	RegisterTable* regtable;
	ExprGenerator* expr_gen;

	int* local_offsets;    // offset for each local variable in this stack frame
	int* argument_offsets; // offset for each function argument in this stack frame
} StackFrame;

StackFrame* create_new_stack_frame(TypeVar** arguments);
int get_local_by_name(const char* name, StackFrame* frame);
int get_argument_by_name(const char* name, StackFrame* frame);

#endif