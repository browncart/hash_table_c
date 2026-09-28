#include "string.h"

#include <string.h>

String8 str8(char *str, usize size) {
	String8 result = {
		str,
		size
	};

	return result;
}

bool str8_match(String8 s1, String8 s2) {
	return s1.size == s2.size && memcmp(s1.str, s2.str, s1.size) == 0;
}