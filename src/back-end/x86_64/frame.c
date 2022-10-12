#include "frame.h"

StackFrame* stack_frame_new(FuncDecl* func_decl) {
    StackFrame* frame = new(StackFrame);
    frame->func_decl_ref = func_decl;
    frame->table_ref = func_decl->scope;
    frame->func_decl_name = func_decl->name->value;
    //frame->func_decl_definition = alloc_func_definition();
    return frame;
}