#ifndef _STRETCHY_BUFFERS_H
#define _STRETCHY_BUFFERS_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct Buffer {
	size_t cap;
	size_t len;
	char buffer[0];
} Buffer;

#define sbuffer__hdr(buf) ((Buffer*)((char*)(buf) - offsetof(Buffer, buffer)))
#define sbuffer__len(buf) sbuffer__hdr(buf)->len
#define sbuffer__cap(buf) sbuffer__hdr(buf)->cap

#define sbuffer__extnd(buf) sbuffer__extend(buf, sizeof(*(buf)))
#define sbuffer__need(buf) ((buf) ? (sbuffer__cap(buf) <= sbuffer__len(buf)) : 1)
#define sbuffer__fits(buf) ((sbuffer__need(buf)) ? ((buf) = sbuffer__extnd(buf)) : 0)

#define sbuffer_len(buf) ((buf) ? sbuffer__len(buf) : 0)
#define sbuffer_cap(buf) ((buf) ? sbuffer__cap(buf) : 0)
#define sbuffer_add(buf, item) (sbuffer__fits(buf), (buf)[sbuffer__len(buf)++] = (item))
#define sbuffer_pop(buf) ((buf) ? (sbuffer__len(buf) > 0 ? sbuffer__len(buf)-- : 0) : 0)
#define sbuffer_free(buf) ((buf) ? free(sbuffer__hdr(buf)) : 0)

void* sbuffer__extend(void* buffer, const size_t typesize);

#endif // _STRETCHY_BUFFERS_H