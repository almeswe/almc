#include "xmemory.h"

void* xmalloc(size_t size)
{
	void* memory = malloc(size);
	if (memory == NULL)
		printerr("xmalloc allocation error");
	return memory;
}

void* xcalloc(size_t blocks, size_t block_size)
{
	void* memory = calloc(blocks, block_size);
	if (memory == NULL)
		printerr("calloc allocation error");
	return memory;
}

void* xrealloc(void* memblock, size_t size)
{
	void* memory = realloc(memblock, size);
	if (memory == NULL)
		printerr("realloc allocation error");
	return memory;
}