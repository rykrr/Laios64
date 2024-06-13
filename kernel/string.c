#include "includes/string.h"

int streq(const char *a, const char *b) {
	while (*a && *b && *a == *b) {
		a++;
		b++;
	}

	if (!*a && !*b)
		return 1;

	return 0;
}

int str_starts_with(const char *s, const char *prefix) {
	while (*s && *prefix && *s == *prefix) {
		s++;
		prefix++;
	}

	return !*prefix;
}

size_t strlen(const char *s) {
	size_t l = 0;
	while (*s) {
		l++;
		s++;
	}
	return l;
}

void *memset(void *s, u8 c, size_t n) {
	u8 *p = (u8*) s;
	for (size_t i = 0; i < n; i++) {
		p = c;
		p++;
	}
	return s;
}
