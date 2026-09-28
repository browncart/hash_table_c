#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "types.h"
#include "string.h"

typedef struct HashTable HashTable;
struct HashTable {
	String8 *keys;
	unsigned char *values;

	usize tomb;
	usize size;
	usize capacity;
	usize elem_size;
	u32 exp;
};

u64 hash64(String8 key);

void 	ht_init		(HashTable *ht, u32 exp, usize elem_size);
void 	ht_destroy	(HashTable *ht);
void 	ht_resize	(HashTable *ht);
void   *ht_upsert	(HashTable *ht, String8 key);
void   *ht_find		(HashTable *ht, String8 key);
int 	ht_count	(HashTable *ht, String8 key);
void 	ht_remove	(HashTable *ht, String8 key);

#endif