#include <kernel/bios.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char LOWER_HEX_DIGITS[16] = "0123456789abcdef";
const char UPPER_HEX_DIGITS[16] = "0123456789ABCDEF";

void print_byte(u8 x) {
	bios_putc(UPPER_HEX_DIGITS[(x >> 4) & 0x0F]);
	bios_putc(UPPER_HEX_DIGITS[x & 0x0F]);
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

void print_hex(u64 x, i8 width, bool zero_pad, bool uppercase) {
	#define NIBBLE 4
	#define NUM_DIGITS 16
	#define HIGHEST_NIBBLE_SHIFT 60

	static char buf[NUM_DIGITS];
	bool left_pad = width < 0;
	char *hex_digits = uppercase? UPPER_HEX_DIGITS : LOWER_HEX_DIGITS;

	if (left_pad)
		width = -width;
	else if (width > NUM_DIGITS)
		width = NUM_DIGITS;

	u8 start = NUM_DIGITS - (left_pad? 0 : width);

	for (u8 i = 0; i < NUM_DIGITS; i++) {
		u8 value = (x >> HIGHEST_NIBBLE_SHIFT) & 0x0F;
		buf[i] = hex_digits[value];
		
		if (value && i < start)
			start = i;

		x <<= NIBBLE;
	}

	if (left_pad) {
		for (u8 i = start; i < NUM_DIGITS; i++)
			putc(buf[i]);

		for (u8 i = NUM_DIGITS - start; i < width; i++)
			putc(' ');
	}
	else {
		bool space = !zero_pad;
		for (u8 i = start; i < NUM_DIGITS; i++) {
			space &= buf[i] == '0' && i < NUM_DIGITS - 1;
			putc(space? ' ' : buf[i]);
		}
	}

	#undef NIBBLE
	#undef NUM_DIGITS
	#undef HIGHEST_NIBBLE_SHIFT
}

void print_string(const char *s, i32 width) {
	bool left_pad = width < 0;
	usize len = strlen(s);

	if (left_pad) {
		width = -width;

		while (*s)
			putc(*s++);

		for (usize i = len; i < width; i++)
			putc(' ');
	}
	else {
		for (usize i = width - len; i < width; i++)
			putc(' ');

		while (*s)
			putc(*s++);
	}
}

void printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	i32 width = 1;

	bool special = false;
	bool zero_pad = false;

	while (*fmt) {
		if (!special) {
			if (*fmt == '%') {
				special = true;
				zero_pad = false;
				width = 1;
			}
			else
				putc(*fmt);
			fmt++;
			continue;
		}

		special = 0;

		switch (*fmt) {
			case 'x':
			case 'X': {
				u64 value = (u64) va_arg(va, u64);
				print_hex(value, width, zero_pad, *fmt == 'X');
				break;
			}
			case 's': {
				const char *s = (const char*) va_arg(va, const char*);
				print_string(s, width);
				break;
			}
			case 'c': {
				putc((char) va_arg(va, int));
				break;
			}
			case '-': {
				width = -1;
				special = true;
				break;
			}
			case '0': {
				zero_pad = true;
				special = true;
				break;
			}
			case '1'...'9': {
				width *= atoi(fmt);
				while (*fmt && '0' <= *fmt && *fmt <= '9')
					fmt++;
				fmt--;
				special = true;
				break;
			}
			default: {
				putc(*fmt);
			}
		}

		fmt++;
	}
	va_end(va);
}
