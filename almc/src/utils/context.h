#ifndef ALMC_CONTEXT_H
#define ALMC_CONTEXT_H

#include <stdint.h>

typedef struct SrcContext
{
	uint32_t size;
	uint32_t line;
	uint32_t start;
	const char* file;
} SrcContext;

const char* src_context_tostr(SrcContext* context);
SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line);
void src_context_free(SrcContext* context);

#endif