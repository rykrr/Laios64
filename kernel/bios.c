#include <kernel/bios.h>

const char HEX_DIGITS[16] = "0123456789ABCDEF";

void puts(const char *s) {
	while (*s) bios_putc(*s++);
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

void panic(const char *msg) {
	puts("\n\n!!! PANIC !!!\n\n");
	puts(msg);
	puts("\n\n\n\n");
	bios_shutdown();
}

