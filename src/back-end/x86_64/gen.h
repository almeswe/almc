#ifndef _ALMC_BACK_END_GEN_H
#define _ALMC_BACK_END_GEN_H

#include "ir.h"

typedef struct x86_64_global_gen_data {
    struct counters {
        size_t labels;                  // count of labels in this assembly file
        size_t floats;                  // count of float constants in this assembly file 
        size_t strings;                 // count of string constants in this assembly file
    } count_of;
    Assembly* curr_asm;                 // current generating assembly reference
    BssSegment* curr_bss_seg;           // reference to current bss section 
    TextSegment* curr_text_seg;         // reference to current text section
    DataSegment* curr_data_seg;         // reference to current data section
    char** allocated_definitions;       // pointers to all allocated definitions (to free them in future), to avoid name conflicts and 
                                        // duplicated items (like same strings or same float constants)
} global_gen_data;

typedef struct x86_64_gen_data {
    regid reg;                          // register in which result of generation is stored
    uint8_t is_in_reg;                  // indicates if result is stored in register
    TextInstruction** text_instrs;      // generated text instructions
    DataInstruction** data_instrs;      // generated data instructiins
} gen_data;

// these functions are needed for creating definitions for different objects
// they are allocated here because all allocated definitions must be appended to `allocated_definitions` in global_gen_data
// few of these functions are defined as `extern` in frame.h, because stack frame generates these definitions inside

char* alloc_func_definition();
char* alloc_label_definition();
char* alloc_float_definition();
char* alloc_string_definition();
char* alloc_variable_definition();

Assembly* gen(AstRoot* ast);

gen_data gen_expr(Expr* expr);
gen_data gen_const_expr(Expr* expr);
gen_data gen_const(Const* const_expr);
gen_data gen_str_const(Str* str_const_expr);

void global_gen_data_free();

#endif