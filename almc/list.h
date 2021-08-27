#include <stdlib.h>

typedef struct
{
	size_t cap;
	size_t length;

	int* data;
} List;

void list__extend(List* const list)
{
	list->cap = list->data ?
		1 : list->cap * 2;
	list->data = (int*)(list->data ?
		malloc(sizeof(int)) : realloc(list->data, list->cap * sizeof(int)));
}

List* list_init()
{
	List* list = (List*)calloc(1, sizeof(List));
	return list;
}

void list_free()
{

}

typedef struct
{
	size_t cap;
	size_t len;

	int* buffer;
} Buffer;

#define __sbuffer_hdr(buf) ((Buffer*)((int*)(buf) - offsetof(Buffer, buffer)))
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
		size_t new_size = offsetof(Buffer, buffer) + new_cap;
		hdr = (Buffer*)realloc(__sbuffer_hdr(buffer), new_size * sizeof(int));
		hdr->cap = new_cap;
	}
	return hdr + offsetof(Buffer, buffer);
}

void list_append(List* const list, void* item)
{
	if (list->cap == list->length)
		list__extend(list);
	list->data[list->length++] = (int)item;
}
/*

List* list_init(uint typesize)
{
	List list = *((List*)malloc(sizeof(List)));
	list.length = 0;
	list._alloc = 0;
	list._data = NULL;
	list._typesize = typesize;
	return &list;
}

//memcpy?
void* list_get(List* const list, const uint index)
{
	return list->length <= index ?
		NULL : list->_data[index];
}

void list_set(List* const list, const uint index, void* item)
{
	if (list->length <= index)
		list->_data[index] = item;
}

void* list_pop(List* const list)
{
	if (list->_data && list->length > 0)
	{
		void* item = list->_data[list->length-1];
		list->_data[list->length--] = NULL;
		return item;
	}
	return NULL;
}

void list_clear(List* const list)
{
	if (!list)
		return;
	for (uint i = 0; i < list->length; i++)
		if (list->_data)
			free((void*)list->_data[i]);
	
	if (list->_data)
		free(list->_data);
	list->_alloc = 0;
	list->length = 0;
	list->_data = NULL;
}

void list_free(List* const list)
{
	list_clear(list);
	if (list)
		free(list);
}*/