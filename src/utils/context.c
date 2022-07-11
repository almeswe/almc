#include "context.h"

const char* src_context_tostr(const SrcContext* context) {
	return frmt("(line: %d, position: %d, file: \'%s\')",
		context->line, context->start, context->file);
}

SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line) {
	SrcContext* context = new(SrcContext);
	context->size = size;
	context->file = file;
	context->line = line;
	context->start = start - size + 1;
	return context;
}

void src_context_free(SrcContext* context) {
	if (context) {
		free(context);
	}
}

const char* src_area_tostr(const SrcArea* area) {
	if (!area->ends) {
		return src_context_tostr(area->begins);
	}
	return frmt("starts at: line: %d, position: %d | ends at: line: %d, position: %d, file: \'%s\'",
		area->begins->line, area->begins->start, area->ends->line, area->ends->start + area->ends->size-1, area->begins->file);
}

SrcArea* src_area_new(SrcContext* begins, SrcContext* ends) {
	SrcArea* area = new(SrcArea);
	area->begins = begins;
	area->ends = ends;
	area->file = area->begins->file;
	return area;
}

void src_area_free(SrcArea* area) {
	if (area) {
		//src_context_free(area->begins);
		//src_context_free(area->ends);
		free(area);
	}
}