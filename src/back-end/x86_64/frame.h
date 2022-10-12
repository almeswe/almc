#ifndef _ALMC_BACK_END_STACK_FRAME_H
#define _ALMC_BACK_END_STACK_FRAME_H

#include "ir.h"

extern char* alloc_func_definition();
extern char* alloc_variable_definition();

typedef enum x86_64_StackFrameEntityKind {
    STACKFRAME_LOCAL,
    STACKFRAME_ARGUMENT
} StackFrameEntityKind;

typedef struct x86_64_StackFrameEntity {
    char* name;
    Type* type;
    char* definition;
    long stack_offset;
    StackFrameEntityKind kind;
} StackFrameEntity;

typedef struct x86_64_StackFrame {
    struct reserved_space {
        size_t locals;
        size_t arguments;
    } reserved;
    Table* table_ref;
    char* func_decl_name;
    char* func_decl_definition;
    FuncDecl* func_decl_ref;
    StackFrameEntity** entites;
} StackFrame;

StackFrame* stack_frame_new(FuncDecl* func_decl);

#endif //_ALMC_BACK_END_STACK_FRAME_H