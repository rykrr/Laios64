#include <endian.h>

u32 switch_endian_dword(u32 x) {
	// TODO: Write a generalizable implementation.
	u8 *y = (u8*) &x;
	u8 z[4] = { y[3], y[2], y[1], y[0] };
	return *((u32*) z);
}
