#include "context.h"

char* src_context_tostr(SrcContext* context)
{
	return frmt("(line: %d, position: %d, file: \'%s\')",
		context->line, context->start, context->file);
}

SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line)
{
	SrcContext* c = malloc(sizeof(SrcContext));
	c->size = size;
	c->file = file;
	c->line = line;
	c->start = start - size + 1;
	return c;
}

void src_context_free(SrcContext* context)
{
	//free(context->file);
	free(context);
}