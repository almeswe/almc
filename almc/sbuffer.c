#include "sbuffer.h"

typedef struct Buffer
{
	size_t cap;
	size_t len;
	char buffer[0];
} Buffer;

void* sbuffer__extend(const char* buffer, const size_t typesize)
{
	size_t new_cap = buffer ? sbuffer__cap(buffer) * 2 : 1;
	size_t new_size = offsetof(Buffer, buffer) + (typesize * new_cap);
	Buffer* hdr = (Buffer*)(buffer ?
		realloc(sbuffer__hdr(buffer), new_size) : calloc(1, new_size));
	hdr->cap = new_cap;
	return hdr->buffer;
}