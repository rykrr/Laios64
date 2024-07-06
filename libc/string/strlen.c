#include <string.h>

usize strlen(const char *s) {
	usize l = 0;
	while (*s) {
		l++;
		s++;
	}
	return l;
}
