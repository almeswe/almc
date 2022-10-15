#ifndef _ALMC_BACK_END_STACK_FRAME_H
#define _ALMC_BACK_END_STACK_FRAME_H

#include "ir.h"

#define glob_frame_get(name) stack_frame_get(glob_info.curr_frame, name)

extern char* alloc_var_def(const char* base);
extern char* alloc_func_def(const char* base);

typedef enum x86_64_StackFrameEntityKind {
    STACKFRAME_LOCAL,
    STACKFRAME_ARGUMENT
} StackFrameEntityKind;

typedef struct x86_64_StackFrameEntity {
    Type* type;
    const char* name;
    char* definition;
    struct stored_in {
        regid reg;
        int offset;
    } in;
    StackFrameEntityKind kind;
} StackFrameEntity;

typedef struct x86_64_StackFrame {
    struct reserved_space {
        size_t locals;
        size_t params;
    } reserved;
    Table* table_ref;
    char* func_decl_def;
    char* func_decl_name;
    FuncDecl* func_decl_ref;
    StackFrameEntity** entities;
} StackFrame;

StackFrame* stack_frame_new(FuncDecl* func_decl);
StackFrameEntity* stack_frame_entity_new(TypeVar* typevar);
StackFrameEntity* stack_frame_get(StackFrame* frame, const char* name);

void stack_frame_free(StackFrame* frame);
void stack_frame_entity_free(StackFrameEntity* entity);

#endif //_ALMC_BACK_END_STACK_FRAME_H