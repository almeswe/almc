#ifndef _ALMC_CONTEXT_H
#define _ALMC_CONTEXT_H

#include <stdint.h>
#include <stdlib.h>

#include "common.h"

typedef struct SrcContext {
	uint32_t size;
	uint32_t line;
	uint32_t start;
	const char* file;
} SrcContext;

typedef struct SrcArea {
	SrcContext* begins;
	SrcContext* ends;
	const char* file;
} SrcArea;

const char* src_area_tostr(const SrcArea* area);
const char* src_context_tostr(const SrcContext* context);

SrcArea* src_area_new(SrcContext* begins, SrcContext* ends);
SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line);

void src_area_free(SrcArea* area);
void src_context_free(SrcContext* context);

#endif // _ALMC_CONTEXT_H