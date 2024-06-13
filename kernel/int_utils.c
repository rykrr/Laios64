#include "includes/int_utils.h"

const char * HEX_DIGITS = "0123456789ABCDEF";

void print_byte(u8 x) {
	putc(HEX_DIGITS[(x >> 4) & 0x0F]);
	putc(HEX_DIGITS[x & 0x0F]);
}

void print_word(u16 x) {
	print_byte((u8)((x >> 8) & 0xFF));
	print_byte((u8)(x & 0xFF));
}

void print_dword(u32 x) {
	print_word((u16)((x >> 16) & 0xFFFF));
	print_word((u16)(x & 0xFFFF));
}

void print_qword(u64 x) {
	print_dword((u32)((x >> 32) & 0xFFFFFFFF));
	print_dword((u32)(x & 0xFFFFFFFF));
}

u16 switch_endian_word(u16 x) {
	return (x << 8) | (x >> 8);
}

u32 switch_endian_dword(u32 x) {
	// TODO: Write a generalizable implementation.
	u8 *y = (u8*) &x;
	u8 z[4] = { y[3], y[2], y[1], y[0] };
	return *((u32*) z);
}
