#include "hash_table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

static char tombstone[] = "(deleted)";

static inline bool exponent_ok(u32 exp) {
	return exp >= 1 && exp < 64 && exp < sizeof(usize) * CHAR_BIT;
}

u64 hash64(String8 key) {
	u64 hash = 0xcbf29ce484222325;
	usize i;
	for (i = 0; i < key.size; ++i) {
		//Because str[i] is char, we use `& 255` to mask against sign-extension.
		//Imagine str[i] is -1, with 1 in the high bit: Sign-extension would
		//duplicate that 1 across all of the higher bits (>255), creating a different
		//value.
		hash ^= key.str[i] & 255;
		hash *= 0x100000001b3;
	}
	return hash;
}

void ht_init(HashTable *ht, u32 exp, usize elem_size) {
	if (!exponent_ok(exp)) {
		fprintf(stderr, "Hash table capacity limit exceeded!\n");
		abort();
	}

	ht->capacity 	= (usize)1 << exp;
	ht->elem_size 	= elem_size;
	ht->keys 		= (String8 *)calloc(ht->capacity, sizeof(String8));
	ht->values 		= (unsigned char *)calloc(ht->capacity, elem_size);
	ht->tomb 		= 0;
	ht->size 		= 0;
	ht->exp 		= exp;
}

void ht_destroy(HashTable *ht) {
	free(ht->keys);
	free(ht->values);
}

void ht_resize(HashTable *ht) {
	double full = (double)ht->size / ht->capacity;
	if (full >= .7) {
		String8 	   *old_keys 	= ht->keys;
		unsigned char  *old_vals 	= ht->values;
		usize 			old_cap		= ht->capacity;

		ht_init(ht, ht->exp + 1, ht->elem_size);

		usize i;
		for (i = 0; i < old_cap; ++i) {
			String8 key = old_keys[i];
			if (key.str && key.str != tombstone) {
				void *val = ht_upsert(ht, key);
				memcpy(val, old_vals + i * ht->elem_size, ht->elem_size);
			}
		}

		free(old_keys);
		free(old_vals);
	}
}

void *ht_upsert(HashTable *ht, String8 key) {
	ht_resize(ht);

	u64 hash 	= hash64(key);
	usize mask 	= ht->capacity - 1;
	usize step 	= (usize)(hash >> (64 - ht->exp)) | 1;
	//`| 1` forces the step to be odd, which allows probing
	//to touch every index before repeating.

	usize tombstone_idx = ht->capacity;
	usize i = (usize)hash & mask;

	for (;;) {
		if (!ht->keys[i].str) {
			//We don't return the tombstone address in ht->values right away
			//because we need to continue probing to find the passed key.
			//If we don't find the passed key, we can safely insert at the
			//first tombstone index, which is preferable to inserting
			//at the first empty key because it helps reduce tombstone 
			//pollution.
			if (tombstone_idx < ht->capacity) {
				i = tombstone_idx;
				ht->tomb--;
			} else {
				ht->size++;
			}
			ht->keys[i] = key;
			return (unsigned char *)(ht->values + i * ht->elem_size);
		}

		if (ht->keys[i].str == tombstone) {
			if (tombstone_idx == ht->capacity) {
				tombstone_idx = i;
			}
		} else if (str8_match(ht->keys[i], key)) {
			return (unsigned char *)(ht->values + i * ht->elem_size);
		}

		i = (i + step) & mask;
	}
	return NULL;
}

void *ht_find(HashTable *ht, String8 key) {
	u64 hash = hash64(key);
	usize mask = ht->capacity - 1;
	usize step = (usize)(hash >> (64 - ht->exp)) | 1;

	usize i = (usize)hash & mask;

	for (;;) {
		if (!ht->keys[i].str) {
			return NULL;
		}

		if (ht->keys[i].str != tombstone && str8_match(ht->keys[i], key)) {
			return ht->values + (i * ht->elem_size);
		}
		
		i = (i + step) & mask;
	}
	return NULL;
}

int ht_count(HashTable *ht, String8 key) {
	u64 hash 	= hash64(key);
	usize mask 	= ht->capacity - 1;
	usize step 	= (usize)(hash >> (64 - ht->exp)) | 1;

	usize i 	= (usize)hash & mask;

	for (;;) {
		if (!ht->keys[i].str) {
			return 0;
		}

		if (ht->keys[i].str != tombstone && str8_match(ht->keys[i], key)) {
			return 1;
		}

		i = (i + step) & mask;
	}
}

void ht_remove(HashTable *ht, String8 key) {
	u64 hash 	= hash64(key);
	usize mask 	= ht->capacity - 1;
	usize step 	= (usize)(hash >> (64 - ht->exp)) | 1;

	usize i 	= (usize)hash & mask;

	for (;;) {
		if (!ht->keys[i].str) {
			return;
		}

		if (ht->keys[i].str != tombstone && str8_match(ht->keys[i], key)) {		
			ht->keys[i].str = tombstone;
			ht->keys[i].size = sizeof(tombstone) - 1;
			ht->tomb++;
			return;
		}

		i = (i + step) & mask;
	}
}