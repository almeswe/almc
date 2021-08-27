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

#define __sbuffer_extnd(buf) (int*)__sbuffer_extend(buf)

#define __sbuffer_fit(buf) __sbuffer_cap(buf) <= __sbuffer_len(buf) 
#define __sbuffer_fits(buf) ((buf) && (__sbuffer_fit(buf)) ? 1 : ((buf) = __sbuffer_extnd(buf)))

#define sbuffer_len(buf) ((buf) ? __sbuffer_len(buf) : 0)
#define sbuffer_cap(buf) ((buf) ? __sbuffer_cap(buf) : 0)
#define sbuffer_add(buf, item) (__sbuffer_fits(buf), (buf)[__sbuffer_len(buf)++] = (item))

void* __sbuffer_extend(const int* buffer)
{
	Buffer* hdr;
	if (!buffer)
	{
		hdr = (Buffer*)malloc(sizeof(Buffer));
		hdr->len = 0;
		hdr->cap = 1;
	}
	else
	{
		size_t new_cap = __sbuffer_cap(buffer) * 2;
		size_t new_size = offsetof(Buffer, buffer) + (new_cap * sizeof(int));
		hdr = (Buffer*)realloc(__sbuffer_hdr(buffer), new_size);
		hdr->cap = new_cap;
	}
	return hdr->buffer;
}