#include "sbuffer.h"

void* sbuffer__extend(const void* buffer, const size_t typesize)
{
	size_t new_cap = buffer ? sbuffer__cap(buffer) * 2 : 1;
	size_t new_size = offsetof(Buffer, buffer) + (typesize * new_cap);
	Buffer* hdr = (Buffer*)(buffer ?
		realloc(sbuffer__hdr(buffer), new_size) : calloc(1, new_size));
	hdr->cap = new_cap;
	return hdr->buffer;
}

void* sbuffer_reduce(const void* buffer, const size_t by_size, const size_t typesize)
{
	if (by_size > 0 && by_size <= sbuffer_len(buffer))
	{
		Buffer* new_buffer = realloc(sbuffer__hdr(buffer), offsetof(Buffer, buffer) + by_size * typesize);
 		new_buffer->cap = sbuffer_len(new_buffer->buffer);
		buffer = new_buffer->buffer;
	}
	return buffer;
}