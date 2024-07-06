#include <kernel/bios.h>

void putc(char c) {
	_putc(c);
}

void puts(const char *s) {
	while (*s) putc(*s++);
}

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
