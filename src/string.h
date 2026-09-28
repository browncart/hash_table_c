#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

#include "types.h"

typedef struct String8 String8;
struct String8 {
	char *str;
	usize size;
};

String8 str8		(char *str, usize size);
bool 	str8_match	(String8 s1, String8 s2);

#endif