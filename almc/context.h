#include <stdint.h>

#ifndef CONTEXT_H
#define CONTEXT_H
 
typedef struct SrcContext
{
	uint32_t size;
	uint32_t line;
	uint32_t start;
	const char* file;
} SrcContext;

#endif

const char* src_context_tostr(SrcContext* context);
SrcContext* src_context_new(const char* file, uint32_t start, uint32_t size, uint32_t line);