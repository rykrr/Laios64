#include <endian.h>

u16 switch_endian_word(u16 x) {
	return (x << 8) | (x >> 8);
}
