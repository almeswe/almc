#ifndef ALMC_BACK_END_x86_ASM_PROGRAM_H
#define ALMC_BACK_END_x86_ASM_PROGRAM_H

#include "x86types.h"
#include "regtable.h"

#include "stack-frame.h"
#include "..\..\front-end\front-end.h"

#define TABLE     program->table
#define REGISTERS program->regtable

#define NEW_LABEL(name) \
	char* name = program_new_label(program)

#define LOGICAL_TRUE_ARG  frmt("%d", 1)
#define LOGICAL_FALSE_ARG frmt("%d", 0)

#define ADD_NEW_LABEL(to) \
	sbuffer_add(to, program_new_label(program))

#define PROGRAM_ADD_PROC(proc) \
	sbuffer_add(program->code->procs, proc)
#define PROGRAM_ADD_PROTO_PROC(proto) \
	sbuffer_add(program->code->proto_procs, proto)

#define PROGRAM_SET_ENTRY(name) \
	program->entry = name

#define PROC_CODE_LINE0(c) \
	proc_code_line0(get_current_proc(program), c)
#define PROC_CODE_LINE1(c, arg1) \
	proc_code_line1(get_current_proc(program), c, arg1)
#define PROC_CODE_LINE2(c, arg1, arg2) \
	proc_code_line2(get_current_proc(program), c, arg1, arg2)

#define PROC_DATA_LINE(dataline) \
	sbuffer_add(program->data->lines, dataline)

typedef struct FuncDecl FuncDecl;

typedef struct x86_AsmCodeDefine
{
	char* name;
	char* value;
} AsmCodeDefine;

typedef struct x86_AsmCodeLine
{
	uint32_t instruction;
	char** arguments;
} AsmCodeLine;

typedef struct x86_AsmCodeProtoProc
{
	char* name;
	Type** types;

	char* lib;
	char* convention;
	bool is_vararg;
} AsmCodeProtoProc;

typedef struct x86_AsmCodeProc
{
	char* name;
	StackFrame* frame;
	AsmCodeLine** lines;
} AsmCodeProc;

typedef struct x86_AsmCodeSegment
{
	AsmCodeProc**      procs;
	AsmCodeProtoProc** proto_procs;
} AsmCodeSegment;

typedef enum x86_DataSpecifier
{
	DATA_CONST,
	DATA_ARRAY,
	DATA_UNION,
	DATA_STRUCT,

	DATA_INITIALIZED_CONST,
	DATA_INITIALIZED_ARRAY,
	DATA_INITIALIZED_UNION,
	DATA_INITIALIZED_STRUCT,
	DATA_INITIALIZED_STRING,
} DataSpecifier;

typedef struct x86_AsmDataLine
{
	char* size;
	char* name;
	char** values;
	DataSpecifier spec;
} AsmDataLine;

typedef struct x86_AsmDataSegment
{
	AsmDataLine** lines;
} AsmDataSegment;

typedef struct x86_AsmProgram
{
	char* entry;
	char** incs;
	char** libs;
	char** labels;
	Table* table;
	RegisterTable* regtable;
	AsmDataSegment* data;
	AsmCodeSegment* code;
} AsmProgram;

AsmProgram* program;

void print_program(AsmProgram* program);
void print_program_to_file(AsmProgram* program);

AsmProgram* program_new(Table* table);
char* program_new_label(AsmProgram* program);
char* program_get_current_label(AsmProgram* program);
void program_add_lib(AsmProgram* program, char* lib);

AsmDataSegment* data_new();
AsmCodeSegment* code_new();

AsmDataLine* dataline_new(char* size, char* name, 
	char** values, DataSpecifier spec);
AsmCodeLine* codeline_new(uint32_t instruction,
	char* arg1, char* arg2);
AsmCodeProc* proc_new(FuncDecl* func_decl);
AsmCodeProtoProc* proto_proc_new(FuncDecl* func_decl);

AsmCodeDefine* define_new(char* name, char* value);

void proc_code_line0(AsmCodeProc* proc, int instr);
void proc_code_line1(AsmCodeProc* proc, int instr, char* arg1);
void proc_code_line2(AsmCodeProc* proc, int instr, char* arg1, char* arg2);
void proc_data_line(AsmProgram* proc, AsmDataLine* dataline);
AsmCodeProc* get_current_proc(AsmProgram* program);

void program_free(AsmProgram* program);
void data_free(AsmDataSegment* data);
void code_free(AsmCodeSegment* code);

char* data_add(AsmDataLine* dataline, AsmDataSegment* seg);

void dataline_free(AsmDataLine* dataline);
void codeline_free(AsmCodeLine* codeline);

void proc_free(AsmCodeProc* proc);
void proto_proc_free(AsmCodeProtoProc* proto_proc);

void define_free(AsmCodeDefine* define);

#endif