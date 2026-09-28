#include "hash_table.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

String8 mock_key(char *key_name) {
	return str8(key_name, strlen(key_name));
}

int main(void) {
	HashTable ht = {0};
	ht_init(&ht, 10, sizeof(int));

	enum { KEY_COUNT = 200 };
	char 	key_names	[KEY_COUNT][32];
	String8 keys		[KEY_COUNT];

	for (int i = 0; i < KEY_COUNT; ++i) {
		snprintf(key_names[i], sizeof(key_names[i]), "key-%d", i);
		keys[i].str = key_names[i];
		keys[i].size = strlen(key_names[i]);

		int *value = ht_upsert(&ht, keys[i]);
		assert(value);
		*value = i;
	}

	for (int i = 0; i < KEY_COUNT; ++i) {
		int *value = ht_find(&ht, keys[i]);
		assert(value);
		assert(*value == i);
		printf("%d == %d\n", i, *value);
	}

	Sleep(10000);
	return 0;
}
