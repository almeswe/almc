#include "program.h"

void print_define(StackFrameEntity* entity)
{
	printf("%s\t= %d\n", entity->definition, entity->offset);
}

void print_dataline(AsmDataLine* line)
{
	switch (line->spec)
	{
	case DATA_INITIALIZED_STRING:
		printf("\t%s\t%s ", line->name, line->size);
		for (size_t i = 0; i < sbuffer_len(line->values); i++)
			printf(i != 0 ? ", " : ""), printf("%s", line->values[i]);
		printf("\n");
		break;
	default:
		assert(0);
	}
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
	printf("%s endp\n\n", proc->name);
}

void print_proto_procs(AsmCodeSegment* codeseg)
{
	for (size_t i = 0; i < sbuffer_len(codeseg->proto_procs); i++)
	{
		printf("%s proto %s ", codeseg->proto_procs[i]->name,
			codeseg->proto_procs[i]->convention);
		for (size_t j = 0; j < sbuffer_len(codeseg->proto_procs[i]->types); j++)
			printf(j ? ", ": ""), printf(":%s", get_ptr_prefix(codeseg->proto_procs[i]->types[j]));
		if (codeseg->proto_procs[i]->is_vararg)
			printf(", :vararg");
		printf("\n");
	}
}

void print_data(AsmDataSegment* dataseg)
{
	printf(".data\n\n");
	for (size_t i = 0; i < sbuffer_len(dataseg->lines); i++)
		print_dataline(dataseg->lines[i]);
}

void print_code(AsmCodeSegment* codeseg)
{
	printf(".code\n\n");
	print_proto_procs(program->code);
	printf("\n");
	for (size_t i = 0; i < sbuffer_len(codeseg->procs); i++)
		print_proc(codeseg->procs[i]);
}

void print_includes(AsmProgram* program)
{
	printf(";-----------INCLUDES------------\n");
	for (size_t i = 0; i < sbuffer_len(program->incs); i++)
		printf("include    %s\\%s.inc\n", 
			MASM_SDK_INC_FOLDER, program->incs[i]);
	for (size_t i = 0; i < sbuffer_len(program->libs); i++)
		printf("includelib %s\\%s.lib\n", 
			MASM_SDK_LIB_FOLDER, program->libs[i]);
	printf(";-------------------------------\n");
}

void print_program(AsmProgram* program)
{
	printf(".386\n");
	printf(".model flat, stdcall\n\n");
	
	//includes
	print_includes(program);
	//
	
	// .data segment
	print_data(program->data);
	//

	// .code segment
	print_code(program->code);
	//

	printf("\nend %s\n", program->entry);
}

AsmProgram* program_new(Table* table)
{
	AsmProgram* program = 
		cnew_s(AsmProgram, program, 1);
	program->entry = NULL;
	program->incs = NULL;
	program->libs = NULL;
	program->table = table;
	program->regtable = regtable_new();
	program->code = code_new();
	program->data = data_new();
	return program;
}

void program_add_lib(AsmProgram* program, char* lib)
{
	for (size_t i = 0; i < sbuffer_len(program->libs); i++)
		if (strcmp(program->libs[i], lib) == 0)
			return;
	sbuffer_add(program->libs, lib);
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
	code->proto_procs = NULL;
	return code;
}

AsmDataLine* dataline_new(char* size, char* name,
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
		cnew_s(AsmCodeProc, proc, 1);
	proc->lines = NULL;
	proc->name = frmt("_%s", func_decl->name->svalue);
	proc->frame = stack_frame_new(func_decl);
	proc->frame->of_proc = proc;
	return proc;
}

AsmCodeProtoProc* proto_proc_new(FuncDecl* func_decl)
{
	AsmCodeProtoProc* proc = 
		cnew_s(AsmCodeProtoProc, proc, 1);
	proc->lib = func_decl->spec->proto->lib;
	proc->convention = func_decl->spec->proto->convention;

	proc->name = func_decl->name->svalue;
	proc->is_vararg = func_decl->spec->is_vararg;

	for (size_t i = 0; i < sbuffer_len(func_decl->params); i++)
		sbuffer_add(proc->types, func_decl->params[i]->type);
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

char* data_add_string(AsmDataLine* dataline, AsmDataSegment* seg)
{
	// iterating through all strings in data segment
	// and if we found the same string, return pointer to this dataline
	// otherwise create new name and add to data segment
	uint32_t count_of_strings = 1;
	for (size_t i = 0; i < sbuffer_len(seg->lines); i++)
	{
		if (seg->lines[i]->spec == DATA_INITIALIZED_STRING)
		{
			count_of_strings += 1;
			assert(seg->lines);
			if (strcmp(dataline->values[0], seg->lines[i]->values[0]) == 0)
				return dataline_free(dataline), seg->lines[i]->name;
		}
	}
	dataline->name = frmt("STR%d", count_of_strings);
	sbuffer_add(seg->lines, dataline);
	return dataline->name;
}

char* data_add(AsmDataLine* dataline, AsmDataSegment* seg)
{
	switch (dataline->spec)
	{
	case DATA_INITIALIZED_STRING:
		return data_add_string(dataline, seg);
	default:
		assert(0);
	}
}

void dataline_free(AsmDataLine* dataline)
{
	if (dataline)
	{
		for (size_t i = 0; i < sbuffer_len(dataline->values); i++)
			free(dataline->values[i]);
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
		free(proc->name);
		free(proc->frame);
		free(proc);
	}
}

void proto_proc_free(AsmCodeProtoProc* proto_proc)
{
	if (proto_proc)
	{
		sbuffer_free(proto_proc->types);
		free(proto_proc);
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