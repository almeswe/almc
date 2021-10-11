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
	if (context)
	//free(context->file);
		free(context);
}

char* src_area_tostr(SrcArea* area)
{
	return frmt("starts at: line: %d, position: %s | ends at: line: %d, position: %d, file: \'%s\'",
		area->begins->line, area->begins->start, area->ends->line, area->ends->start, area->begins->file);
}

SrcArea* src_area_new(SrcContext* begins, SrcContext* ends)
{
	SrcArea* a = malloc(sizeof(SrcArea));
	a->begins = begins;
	a->ends = ends;
	a->file = a->begins->file;
	return a;
}

void src_area_free(SrcArea* area)
{
	if (area)
	{
		src_context_free(area->begins);
		src_context_free(area->ends);
		free(area);
	}
}