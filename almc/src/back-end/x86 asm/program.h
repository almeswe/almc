#ifndef ALMC_X86_BACKEND_ASM_PROGRAM
#define ALMC_X86_BACKEND_ASM_PROGRAM

#include "regtable.h"
#include "stack-frame.h"
#include "..\..\utils\common.h"

typedef struct FuncDecl FuncDecl;

typedef struct x86_AsmCodeDefine
{
	char* name;
	char* value;
} AsmCodeDefine;

typedef struct x86_AsmCodeLine
{
	char* command;
	char** arguments;
} AsmCodeLine;

typedef struct x86_AsmCodeProc
{
	char* name;
	StackFrame* frame;
	AsmCodeLine** lines;
	AsmCodeDefine** defines;
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
	RegisterTable* table;
	AsmDataSegment* data;
	AsmCodeSegment* code;
} AsmProgram;

void print_program(AsmProgram* program);

AsmProgram* program_new();
AsmDataSegment* data_new();
AsmCodeSegment* code_new();

AsmDataLine* dataline_new(int size, char* name, 
	char** values, DataSpecifier spec);
AsmCodeLine* codeline_new(char* command, 
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