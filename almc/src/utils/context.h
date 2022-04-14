#ifndef _ALMC_CONTEXT_H
#define _ALMC_CONTEXT_H

#include <stdint.h>
#include <stdlib.h>

typedef struct SrcContext
{
	uint32_t size;
	uint32_t line;
	uint32_t start;
	char* file;
} SrcContext;

typedef struct SrcArea
{
	SrcContext* begins;
	SrcContext* ends;
	char* file;
} SrcArea;

char* src_context_tostr(SrcContext* context);
SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line);
void src_context_free(SrcContext* context);

char* src_area_tostr(SrcArea* area);
SrcArea* src_area_new(SrcContext* begins, SrcContext* ends);
void src_area_free(SrcArea* area);

#endif // _ALMC_CONTEXT_H