#include <types.h>

u8 msb(u64 x) {
	u8 b = 0;
	while (x >>= 1)
		b++;
	return b;
}
