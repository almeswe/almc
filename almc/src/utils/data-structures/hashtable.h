//todo: write hash-table for token-table!
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>

typedef struct HashTable
{
	uint32_t len;
	uint32_t cap;
} HashTable;



uint32_t hash();

#endif //HASH_TABLE_H