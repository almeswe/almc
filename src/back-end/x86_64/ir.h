#ifndef _ALMC_BACK_END_IR_H
#define _ALMC_BACK_END_IR_H

#include "../x86_64/registers.h"
#include "../x86_64/instructions.h"
#include "../../front-end/front-end.h"

#define CRLF "\r\n"
#define INDN "    "
#define instr_size_tostr(size) (instr_size_str[size])
#define instr_spec_tostr(spec) (instr_spec_str[spec])

#define putasm(fd, format, ...)                 \
    if (fprintf(fd, format, __VA_ARGS__) < 0) { \
        report_error(frmt("Runtime error in putasm macro %s:%d.", __FILE__, __LINE__), NULL);  \
    }

#define putasmn(fd, format, ...) \
    putasm(fd, format CRLF, __VA_ARGS__)

#define instr(i)        text_instr_new(i, INSTRUCTION_NOP, NULL, NULL);
#define instr_un(i)     text_instr_new(i, INSTRUCTION_UNARY, NULL, NULL);
#define instr_bin(i)    text_instr_new(i, INSTRUCTION_BINARY, NULL, NULL);

#define instrarg(v)     text_instrarg_new(v, SPEC_NONE, 0, false);

#define data_instr(n, v, s) data_instr_new(n, v, s, false, 0);

extern regid regtable[regs_count]; // defining register table as external

typedef enum x86_64_InstructionArgumentSize {
    SPEC_BYTE,
    SPEC_WORD,
    SPEC_DWORD,
    SPEC_QWORD,
    SPEC_NONE
} InstructionArgumentSize;

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
} DataInstructionKind;

typedef struct x86_64_DataInstruction {
    char* name;                                 // name of label that will be allocated
    char** value;                               // values with which this label will be initialized
    bool times;                                 // has times directive
    long times_value;                           // if times directive is listed, indicates its argument
    InstructionArgumentSize size;               // size (db, dw, dd, dq)
} DataInstruction;

typedef struct x86_64_DataSegment {
    DataInstruction** instructions;
} DataSegment;

typedef struct x86_64_BssSegment {
    DataInstruction** instructions;
} BssSegment;

typedef enum x86_64_TextInstructionKind {
    INSTRUCTION_BINARY,
    INSTRUCTION_UNARY,
    INSTRUCTION_NOP
} TextInstructionKind;

typedef struct x86_64_TextInstructionArgument {
    long offset;                                // offset that may be applied for value ([ebp+8] <-- `8` is offset)
    const char* value;                          // base value of instruction (may be number, register or in combination [ebp+8] <-- `ebp` is value)
    bool dereference;                           // indicates if this value is dereferenced
    InstructionArgumentSize spec;               // interpretation of size (analog of `word ptr [eax]` )
} TextInstructionArg;

typedef struct x86_64_TextInstruction {
    unsigned int instruction;
    TextInstructionKind kind;
    TextInstructionArg* left;
    TextInstructionArg* right;
} TextInstruction;

typedef struct x86_64_TextSegment {
    TextInstruction** instructions;
} TextSegment;

typedef struct x86_64_Assembly {
    const char* global;
    BssSegment* bss;
    DataSegment* data;
    TextSegment* text;
} Assembly;

void text_instrarg_to_stream(TextInstructionArg* text_instrarg, FILE* fd);
void text_instr_to_stream(TextInstruction* text_instr, FILE* fd);
void text_seg_to_stream(Assembly* assembly, TextSegment* text_seg, FILE* fd);
void asm_to_stream(Assembly* assembly, FILE* fd);

TextSegment* text_seg_new();
TextInstruction* text_instr_new(unsigned instr, TextInstructionKind kind, TextInstructionArg* lop, TextInstructionArg* rop);
TextInstructionArg* text_instrarg_new(const char* value, InstructionArgumentSize spec, long offset, bool dereference);

BssSegment* bss_seg_new();
DataSegment* data_seg_new();
DataInstruction* data_instr_new(char* name, char** value, InstructionArgumentSize size, bool times, long times_value);

Assembly* asm_new(const char* global);

void text_seg_free(TextSegment* text_seg);
void text_instr_free(TextInstruction* text_instr);
void text_instrarg_free(TextInstructionArg* text_instr_arg);

void bss_seg_free(BssSegment* bss_seg);
void data_seg_free(DataSegment* data_seg);
void data_instr_free(DataInstruction* data_instr);

void asm_free(Assembly* assembly);


#endif