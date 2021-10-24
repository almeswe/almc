#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdlib.h>

#define HT_START_CAP 8
#define HT_LOAD_FACTOR 0.75

typedef struct HtEntry HtEntry;

typedef struct HtEntry
{
	long* key;
	long* value;
	HtEntry* next;
} HtEntry;

typedef struct Ht
{
	uint32_t cap;
	uint32_t len;
	HtEntry** entries;
} Ht;

Ht* ht_new();
//void ht_set(Ht* ht, long* key, long* value);
HtEntry* ht_entry_new(long* key, long* value);
void ht_str_set(Ht* ht, char* key, long* value);

#endif