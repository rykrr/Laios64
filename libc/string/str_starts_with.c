#include <string.h>

u8 str_starts_with(const char *s, const char *prefix) {
	while (*s && *prefix && *s == *prefix) {
		s++;
		prefix++;
	}

	return !*prefix;
}
