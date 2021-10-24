#include "ht.h"

//http://www.cse.yorku.ca/~oz/hash.html

Ht* ht_new()
{
	Ht* ht = (Ht*)malloc(sizeof(Ht));
	if (!ht)
		return NULL;
	ht->len = 0;
	ht->cap = HT_START_CAP;
	ht->entries = (HtEntry*)calloc(ht->cap, sizeof(HtEntry*));
	return ht;
}

void ht_rehash(Ht* ht)
{

}

uint32_t hash_str_key(const char* key)
{
	int c;
	uint32_t hash = 5381;
	while (c = *key++)
		hash = ((hash << 5) + hash) + c;
	return hash;
}

HtEntry* ht_entry_new(long* key, long* value)
{
	HtEntry* entry = (HtEntry*)malloc(sizeof(HtEntry));
	if (!entry)
		return NULL;
	entry->key = key;
	entry->value = value;
	entry->next = NULL;
	return entry;
}

//void ht_set(Ht* ht, long* key, long* value);
void ht_str_set(Ht* ht, char* key, long* value)
{
	if (ht->len / ht->cap >= HT_LOAD_FACTOR)
		ht_rehash(ht);

	uint32_t index = hash_str_key(key) % ht->cap;

	if (!ht->entries[index])
		(ht->entries[index] = ht_entry_new(key, value), ht->len++);
	else
	{
		HtEntry* curr = ht->entries[index];
		HtEntry* prev = curr;
		while (curr)
		{
			if (curr->key == key)
			{
				curr->value = value;
				return;
			}
			prev = curr;
			curr = curr->next;
		}
		prev->next = ht_entry_new(key, value);
	}
}