#ifndef _ALMC_BACK_END_IR_H
#define _ALMC_BACK_END_IR_H

#include "../x86_64/registers.h"
#include "../x86_64/instructions.h"
#include "../../front-end/front-end.h"

#define instr_size_tostr(size) (instr_size_str[size])
#define instr_spec_tostr(spec) (instr_spec_str[spec])

#define instr(x)        text_instr_new(x, INSTRUCTION_NOP, NULL, NULL)
#define instr_un(x)     text_instr_new(x, INSTRUCTION_UNARY, NULL, NULL)
#define instr_bin(x)    text_instr_new(x, INSTRUCTION_BINARY, NULL, NULL)
#define instr_arg(x)    text_instrarg_new(x, SPEC_NONE, 0, false)

#define instr_arg_reg(x)      instr_arg(reg_tostr(x))
#define instr_arg_cpy(x, y)   x = instr_arg(NULL), *x = *y

#define data_instr(n, v, s, t) data_instr_new(n, v, t, s, false, 0)

extern char* alloc_label_def();

extern regid regtable[regs_count]; // defining register table as external

typedef enum x86_64_InstructionArgumentSize {
    SPEC_BYTE,
    SPEC_WORD,
    SPEC_DWORD,
    SPEC_QWORD,
    SPEC_NONE
} InstrArgSize;

static const char* instr_size_str[] = {
    [SPEC_BYTE]  = "db",
    [SPEC_WORD]  = "dw",
    [SPEC_DWORD] = "dd",
    [SPEC_QWORD] = "dq"
};

static const char* instr_spec_str[] = {
    [SPEC_BYTE]  = "byte",
    [SPEC_WORD]  = "word",
    [SPEC_DWORD] = "dword",
    [SPEC_QWORD] = "qword"
};

typedef enum x86_64_DataInstructionKind {
    DATA_FLOAT_CONST,
    DATA_STRING_CONST,
    DATA_OTHER
} DataInstrKind;

typedef struct x86_64_DataInstruction {
    char* name;                      // name of label that will be allocated
    char** value;                    // values with which this label will be initialized
    bool times;                      // has times directive
    long times_value;                // if times directive is listed, indicates its argument
    DataInstrKind kind;              // kind of data instruction
    InstrArgSize size;               // size (db, dw, dd, dq)
} DataInstr;

typedef struct x86_64_DataSegment {
    DataInstr** instructions;
} DataSegment;

typedef struct x86_64_BssSegment {
    DataInstr** instructions;
} BssSegment;

typedef enum x86_64_TextInstructionKind {
    INSTRUCTION_BINARY,
    INSTRUCTION_UNARY,
    INSTRUCTION_NOP
} TextInstrKind;

typedef struct x86_64_TextInstructionArgument {
    long offset;                     // offset that may be applied for value ([ebp+8] <-- `8` is offset)
    const char* value;               // base value of instruction (may be number, register or in combination [ebp+8] <-- `ebp` is value)
    bool dereference;                // indicates if this value is dereferenced
    InstrArgSize spec;               // interpretation of size (analog of `word ptr [eax]` )
} TextInstrArg;

typedef struct x86_64_TextInstruction {
    unsigned int instruction;
    TextInstrKind kind;
    TextInstrArg* left;
    TextInstrArg* right;
    bool included_in_data;          // indicates if this instruction should be added to final generation
                                    // it can be used in cases of optmizations
} TextInstr;

typedef struct x86_64_TextSegment {
    TextInstr** instructions;
} TextSegment;

typedef struct x86_64_Assembly {
    const char* global;
    BssSegment* bss;
    DataSegment* data;
    TextSegment* text;
} Assembly;

regid get_subreg(regid reg, InstrArgSize size);
regid get_subreg_from_type(regid reg, Type* type);
InstrArgSize get_size_spec(Type* type);

TextInstr* label();
TextInstr* flabel(char* label);
TextInstr* comment(char* comment);

void text_instrarg_to_stream(TextInstrArg* text_instrarg, FILE* fd);
void text_instr_to_stream(TextInstr* text_instr, FILE* fd);
void text_seg_to_stream(Assembly* assembly, TextSegment* text_seg, FILE* fd);
void asm_to_stream(Assembly* assembly, FILE* fd);

TextSegment* text_seg_new();
TextInstr* text_instr_new(unsigned instr, TextInstrKind kind, TextInstrArg* lop, TextInstrArg* rop);
TextInstrArg* text_instrarg_new(const char* value, InstrArgSize spec, long offset, bool dereference);

BssSegment* bss_seg_new();
DataSegment* data_seg_new();
DataInstr* data_instr_new(char* name, char** value, DataInstrKind kind, InstrArgSize size, bool times, long times_value);

Assembly* asm_new();

void text_seg_free(TextSegment* text_seg);
void text_instr_free(TextInstr* text_instr);
void text_instrarg_free(TextInstrArg* text_instr_arg);

void bss_seg_free(BssSegment* bss_seg);
void data_seg_free(DataSegment* data_seg);
void data_instr_free(DataInstr* data_instr);

void asm_free(Assembly* assembly);

#endif