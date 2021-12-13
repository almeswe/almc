#include "program.h"

void print_define(StackFrameEntity* entity)
{
	printf("%s\t= %d\n", entity->definition, entity->offset);
}

void print_codeline(AsmCodeLine* line)
{
	int count = sbuffer_len(line->arguments);
	switch (count)
	{
	case 0:
		printf("\t%s\n", instr_tostr(line->instruction));
		break;
	case 1:
		printf("\t%s\t%s\n", instr_tostr(line->instruction),
			line->arguments[0]);
		break;
	case 2:
		printf("\t%s\t%s, %s\n", instr_tostr(line->instruction),
			line->arguments[0], line->arguments[1]);
		break;
	}
}

void print_proc(AsmCodeProc* proc)
{
	for (size_t i = 0; i < sbuffer_len(proc->frame->entities); i++)
		print_define(proc->frame->entities[i]);
	printf("%s proc\n", proc->name);
	for (size_t i = 0; i < sbuffer_len(proc->lines); i++)
		print_codeline(proc->lines[i]);
	printf("%s endp\n", proc->name);
}

void print_program(AsmProgram* program)
{
	printf(".386\n");
	printf(".model flat, stdcall\n\n");
	
	//includes...
	//printf(";-----------INCLUDES------------\n");
	//printf(";-------------------------------\n");
	//
	
	// .data segment
	printf(".data\n\n");
	//

	// .code segment
	printf(".code\n\n");
	for (size_t i = 0; i < sbuffer_len(program->code->procs); i++)
		print_proc(program->code->procs[i]);
	//

	printf("\nend %s\n", program->entry);
}

AsmProgram* program_new(Table* table)
{
	AsmProgram* program = new_s(AsmProgram, program);
	program->entry = NULL;
	program->incs = NULL;
	program->libs = NULL;
	program->table = table;
	program->regtable = regtable_new();
	program->code = code_new();
	program->data = data_new();
	return program;
}

AsmDataSegment* data_new()
{
	AsmDataSegment* data = 
		new_s(AsmDataSegment, data);
	data->lines = NULL;
	return data;
}

AsmCodeSegment* code_new()
{
	AsmCodeSegment* code =
		new_s(AsmCodeSegment, code);
	code->procs = NULL;
	return code;
}

AsmDataLine* dataline_new(int size, char* name,
	char** values, DataSpecifier spec)
{
	AsmDataLine* line = new_s(AsmDataLine, line);
	line->size = size;
	line->name = name;
	line->values = values;
	line->spec = spec;
	return line;
}

AsmCodeLine* codeline_new(uint32_t instruction, char* arg1, char* arg2)
{
	AsmCodeLine* line = new_s(AsmCodeLine, line);
	line->instruction = instruction;
	line->arguments = NULL;
	if (arg1)
		sbuffer_add(line->arguments, arg1);
	if (arg1 && arg2)
		sbuffer_add(line->arguments, arg2);
	return line;
}

AsmCodeProc* proc_new(FuncDecl* func_decl)
{
	AsmCodeProc* proc = 
		new_s(AsmCodeProc, proc);
	proc->lines = NULL;
	proc->name = func_decl->name->svalue;
	proc->frame = stack_frame_new(func_decl);
	proc->frame->of_proc = proc;
	return proc;
}

AsmCodeDefine* define_new(char* name, char* value)
{
	AsmCodeDefine* define = 
		new_s(AsmCodeDefine, define);
	define->name = name;
	define->value = value;
	return define;
}

void program_free(AsmProgram* program)
{
	if (program)
	{
		data_free(program->data);
		code_free(program->code);
		for (size_t i = 0; i < sbuffer_len(program->incs); i++)
			free(program->incs[i]);
		free(program->regtable);
		free(program);
	}
}

void data_free(AsmDataSegment* data)
{
	if (data)
	{
		for (size_t i = 0; i < sbuffer_len(data->lines); i++)
			dataline_free(data->lines[i]);
		sbuffer_free(data->lines);
		free(data);
	}
}

void code_free(AsmCodeSegment* code)
{
	if (code)
	{
		for (size_t i = 0; i < sbuffer_len(code->procs); i++)
			proc_free(code->procs[i]);
		sbuffer_free(code->procs);
		free(code);
	}
}

void dataline_free(AsmDataLine* dataline)
{
	if (dataline)
	{
		for (size_t i = 0; i < sbuffer_len(dataline->values); i++)
			proc_free(dataline->values[i]);
		sbuffer_free(dataline->values);
		free(dataline->name);
		free(dataline);
	}
}

void codeline_free(AsmCodeLine* codeline)
{
	if (codeline)
	{
		//todo: think about freeing this
		//for (size_t i = 0; i < sbuffer_len(codeline->arguments); i++)
		//	free(codeline->arguments[i]);
		sbuffer_free(codeline->arguments);
		free(codeline);
	}
}

void proc_free(AsmCodeProc* proc)
{
	if (proc)
	{
		//for (size_t i = 0; i < sbuffer_len(proc->defines); i++)
		//	define_free(proc->defines[i]);
		//sbuffer_free(proc->defines);
		for (size_t i = 0; i < sbuffer_len(proc->lines); i++)
			codeline_free(proc->lines[i]);
		sbuffer_free(proc->lines);
		free(proc->frame);
		free(proc);
	}
}

void define_free(AsmCodeDefine* define)
{
	if (define)
	{
		free(define->name);
		free(define->value);
		free(define);
	}
}