#include <stdlib.h>

typedef struct
{
	size_t cap;
	size_t len;
	char buffer[0];
} Buffer;

#define __sbuffer_hdr(buf) ((Buffer*)((char*)(buf) - offsetof(Buffer, buffer)))
#define __sbuffer_len(buf) __sbuffer_hdr(buf)->len
#define __sbuffer_cap(buf) __sbuffer_hdr(buf)->cap

#define __sbuffer_extnd(buf) __sbuffer_extend(buf, sizeof(*(buf)))

#define __sbuffer_need(buf) ((!buf) ? 1 : (__sbuffer_cap(buf) <= __sbuffer_len(buf)))
#define __sbuffer_fit(buf) ((__sbuffer_need(buf)) ? ((buf) = __sbuffer_extnd(buf)) : 0)

#define sbuffer_len(buf) ((buf) ? __sbuffer_len(buf) : 0)
#define sbuffer_cap(buf) ((buf) ? __sbuffer_cap(buf) : 0)
#define sbuffer_add(buf, item) (__sbuffer_fit(buf), (buf)[__sbuffer_len(buf)++] = (item))

void* __sbuffer_extend(const int* buffer, size_t typesize)
{
	size_t new_cap = buffer ? __sbuffer_cap(buffer) * 2: 1;
	size_t new_size = offsetof(Buffer, buffer) + (typesize * (buffer ? new_cap : 1));
	Buffer* hdr = (Buffer*)(buffer ? 
		realloc(__sbuffer_hdr(buffer), new_size) : calloc(1, new_size));
	hdr->cap = new_cap;
	return hdr->buffer;
}