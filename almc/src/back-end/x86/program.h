#ifndef ALMC_X86_BACKEND_ASM_PROGRAM
#define ALMC_X86_BACKEND_ASM_PROGRAM

#define TABLE     program->table
#define REGISTERS program->regtable

#define PROGRAM_ADD_PROC(proc) \
	sbuffer_add(program->code->procs, proc)
#define PROGRAM_SET_ENTRY(name) \
	program->entry = name

#define GET_CURR_PROC \
	program->code->procs[sbuffer_len(program->code->procs)-1]

#define PROC_CODE_LINE0(c) \
	sbuffer_add(GET_CURR_PROC->lines, codeline_new(c, NULL, NULL))
#define PROC_CODE_LINE1(c, arg1) \
	sbuffer_add(GET_CURR_PROC->lines, codeline_new(c, arg1, NULL))
#define PROC_CODE_LINE2(c, arg1, arg2) \
	sbuffer_add(GET_CURR_PROC->lines, codeline_new(c, arg1, arg2))

#include "x86types.h"
#include "regtable.h"
#include "stack-frame.h"
#include "..\..\front-end\front-end.h"

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

typedef struct x86_AsmCodeProc
{
	char* name;
	bool is_external;
	StackFrame* frame;
	AsmCodeLine** lines;
} AsmCodeProc;

typedef struct x86_AsmCodeSegment
{
	AsmCodeProc** procs;
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
	int size;
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
	Table* table;
	RegisterTable* regtable;
	AsmDataSegment* data;
	AsmCodeSegment* code;
} AsmProgram;

AsmProgram* program;

void print_program(AsmProgram* program);

AsmProgram* program_new(Table* table);
AsmDataSegment* data_new();
AsmCodeSegment* code_new();

AsmDataLine* dataline_new(int size, char* name, 
	char** values, DataSpecifier spec);
AsmCodeLine* codeline_new(uint32_t instruction,
	char* arg1, char* arg2);
AsmCodeProc* proc_new(FuncDecl* func_decl);
AsmCodeDefine* define_new(char* name, char* value);

void program_free(AsmProgram* program);
void data_free(AsmDataSegment* data);
void code_free(AsmCodeSegment* code);

void dataline_free(AsmDataLine* dataline);
void codeline_free(AsmCodeLine* codeline);

void proc_free(AsmCodeProc* proc);
void define_free(AsmCodeDefine* define);

#endif