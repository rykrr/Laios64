#include <string.h>

void *memset(void *s, u8 c, usize n) {
	u8 *p = (u8*) s;
	for (usize i = 0; i < n; i++) {
		*p = c;
		p++;
	}
	return s;
}
