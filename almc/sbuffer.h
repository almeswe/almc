#include <stdlib.h>

typedef struct Buffer Buffer;

#define sbuffer__hdr(buf) ((Buffer*)((char*)(buf) - offsetof(Buffer, buffer)))
#define sbuffer__len(buf) sbuffer__hdr(buf)->len
#define sbuffer__cap(buf) sbuffer__hdr(buf)->cap

#define sbuffer__extnd(buf) sbuffer__extend(buf, sizeof(*(buf)))
#define sbuffer__need(buf) ((buf) ? (sbuffer__cap(buf) <= sbuffer__len(buf)) : 1)
#define sbuffer__fit(buf) ((sbuffer__need(buf)) ? ((buf) = sbuffer__extnd(buf)) : 0)

#define sbuffer_len(buf) ((buf) ? sbuffer__len(buf) : 0)
#define sbuffer_cap(buf) ((buf) ? sbuffer__cap(buf) : 0)
#define sbuffer_add(buf, item) (sbuffer__fit(buf), (buf)[sbuffer__len(buf)++] = (item))
#define sbuffer_pop(buf) ((buf) ? (sbuffer__len(buf) > 0 ? sbuffer__len(buf)-- : 0) : 0)
#define sbuffer_free(buf) ((buf) ? free(sbuffer__hdr(buf)) : 0)

void* sbuffer__extend(const char* buffer, const size_t typesize);