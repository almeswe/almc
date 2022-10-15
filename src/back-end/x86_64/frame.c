#include "frame.h"

StackFrameEntity* stack_frame_entity_new(TypeVar* typevar) {
    StackFrameEntity* entity = new(StackFrameEntity);
    // todo: kind may be useless
    entity->kind = STACKFRAME_LOCAL;
    entity->in.reg = 0;
    entity->in.offset = 0;
    entity->name = typevar->var;
    entity->type = typevar->type;
    entity->definition = alloc_var_def(typevar->var);
    return entity;
}

void stack_frame_init_params_entry(StackFrame* frame) {
    FuncDecl* func_decl = frame->func_decl_ref;
    size_t params = sbuffer_len(func_decl->params); 
    if (params != 0) {
        // two params means that entry function has argc and argv 
        if (params == 2) {                      
            for (int i = 0, offset = 8; i < params; i++, offset+=8) {
                StackFrameEntity* entity = 
                    stack_frame_entity_new(func_decl->params[i]);
                entity->in.reg = RBP;
                entity->in.offset = offset;
                sbuffer_add(frame->entities, entity);
            }
        }
    }
}

void stack_frame_init_params_linux_x86_64(StackFrame* frame) {
    size_t space_required = 0;
    const regid gp_regs[] = { RDI, RSI, RDX, RCX, R8, R9 };
    const regid xmm_regs[] = { XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7 };
    const int gp_regs_count = sizeof gp_regs / sizeof(regid);
    const int xmm_regs_count = sizeof xmm_regs / sizeof(regid);
    int gp_index = 0, xmm_index = 0;
    FuncDecl* func_decl = frame->func_decl_ref;
    for (size_t i = 0; i < sbuffer_len(func_decl->scope->scopes.parameters); i++) {
        TypeVar* param = func_decl->scope->scopes.parameters[i]->value.parameter;
        Type* type = param->type;
        if (is_primitive_type(type) || is_pointer_like_type(type)) {
            StackFrameEntity* entity = stack_frame_entity_new(param);
            if (is_real_type(type)) {
                if (xmm_index < xmm_regs_count) {
                    entity->in.reg = xmm_regs[xmm_index++];
                    reserve(entity->in.reg);
                }
                else {
                    space_required += max(8, type->size);
                    entity->in.offset = space_required;
                }
            } 
            else {
                if (gp_index < gp_regs_count) {
                    entity->in.reg = gp_regs[gp_index++];
                    reserve(entity->in.reg);
                }
                else {
                    space_required += max(8, type->size);
                    entity->in.offset = space_required;
                }
            }
            sbuffer_add(frame->entities, entity);
        } else {
            report_error("unimplemeted yet.", NULL);
        }
    }
}

void stack_frame_init_params(StackFrame* frame) {
    FuncDecl* func_decl = frame->func_decl_ref;
    if (func_decl->specs & FUNC_SPEC_ENTRY) {
        stack_frame_init_params_entry(frame);
    }
    else {
        if (func_decl->callconv & CC_LINUX_x86_64) {
            stack_frame_init_params_linux_x86_64(frame);
        }
    }
}

void stack_frame_init_locals(StackFrame* frame) {
    size_t space_required = 0;
    FuncDecl* func_decl = frame->func_decl_ref;
    for (size_t i = 0; i < sbuffer_len(func_decl->scope->scopes.locals); i++) {
        VarDecl* local = func_decl->scope->scopes.locals[i]->value.local;
        space_required += local->type_var->type->size;
        StackFrameEntity* entity = stack_frame_entity_new(local->type_var);
        entity->in.reg = RBP;
        entity->in.offset = -space_required;
        sbuffer_add(frame->entities, entity);
    }
    frame->reserved.locals = space_required;
}

StackFrame* stack_frame_new(FuncDecl* func_decl) {
    StackFrame* frame = new(StackFrame);
    frame->entities = NULL;
    frame->table_ref = func_decl->scope;
    frame->func_decl_ref = func_decl;
    frame->func_decl_name = func_decl->name->value;
    frame->func_decl_def = alloc_func_def(func_decl->name->value);
    stack_frame_init_locals(frame);
    stack_frame_init_params(frame);
    return frame;
}

StackFrameEntity* stack_frame_get(StackFrame* frame, const char* name) {
    for (size_t i = 0; i < sbuffer_len(frame->entities); i++) {
        if (str_eq(name, frame->entities[i]->name)) {
            return frame->entities[i];
        }
    }
    return NULL;
}

void stack_frame_free(StackFrame* frame) {
    if (frame != NULL) {
        for (size_t i = 0; i < sbuffer_len(frame->entities); i++) {
            if (frame->entities[i]->in.offset == 0) {
                unreserve(frame->entities[i]->in.reg);
            }
            stack_frame_entity_free(frame->entities[i]);
        }
        sbuffer_free(frame->entities), free(frame);
    }
}

void stack_frame_entity_free(StackFrameEntity* entity) {
    if (entity != NULL) {
        free(entity);
    }
}