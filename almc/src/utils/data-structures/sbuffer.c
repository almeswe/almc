#include "sbuffer.h"

void* sbuffer__extend(void* buffer, const uint32_t typesize)
{
	uint32_t new_cap = buffer ? sbuffer__cap(buffer) * 2 : 1;
	uint32_t new_size = offsetof(Buffer, buffer) + (typesize * new_cap);
	Buffer* hdr = (Buffer*)(buffer ?
		realloc(sbuffer__hdr(buffer), new_size) : calloc(1, new_size));
	if (!hdr)
		return buffer;
	hdr->cap = new_cap;
	return hdr->buffer;
}