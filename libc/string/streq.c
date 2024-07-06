#include <string.h>

u8 streq(const char *a, const char *b) {
	while (*a && *b && *a == *b) {
		a++;
		b++;
	}

	if (!*a && !*b)
		return 1;

	return 0;
}
