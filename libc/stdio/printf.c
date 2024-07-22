#include <kernel/bios.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char LOWER_HEX_DIGITS[16] = "0123456789abcdef";
const char UPPER_HEX_DIGITS[16] = "0123456789ABCDEF";

void print_hex(u64 x, i8 width, bool zero_pad, bool uppercase) {
	#define NIBBLE 4
	#define NUM_DIGITS 16
	#define HIGHEST_NIBBLE_SHIFT 60

	static char buf[NUM_DIGITS+1] = {[NUM_DIGITS] = '\0'};
	char *hex_digits = uppercase? UPPER_HEX_DIGITS : LOWER_HEX_DIGITS;
	bool zero = !x;

	if (width < 0)
		width = -width;

	if (width > NUM_DIGITS)
		width = NUM_DIGITS;

	u8 start = NUM_DIGITS - width;

	for (u8 i = 0; i < NUM_DIGITS; i++) {
		u8 value = (x >> HIGHEST_NIBBLE_SHIFT) & 0x0F;

		if (value && i < start)
			start = i;

		zero_pad |= value;
		buf[i] = zero_pad? hex_digits[value] : ' ';

		x <<= NIBBLE;
	}

	if (zero)
		buf[NUM_DIGITS-1] = '0';

	bios_puts2(buf + start);

	#undef HIGHEST_NIBBLE_SHIFT
	#undef NUM_DIGITS
	#undef NIBBLE
}

void print_decimal(i32 x, i32 width) {
	#define NUM_DIGITS 10
	static char buf[NUM_DIGITS+2] = {[NUM_DIGITS+1] = '\0'};
	bool negative = x < 0;
	u8 start = NUM_DIGITS;

	if (width < 0)
		width = -width;

	if (width > NUM_DIGITS+1)
		width = NUM_DIGITS+1;

	if (negative)
		x = -x;

	do {
		buf[start] = UPPER_HEX_DIGITS[x % 10];
		x /= 10;
	} while(x && start--);

	if (negative)
		buf[--start] = '-';

	u8 len2 = NUM_DIGITS - start + 1;

	while (len2 < width) {
		buf[--start] = ' ';
		len2++;
	}

	bios_puts2(buf + start);
	#undef NUM_DIGITS
}

void print_string(const char *s, i32 width, bool truncate) {
	bool right_pad = width < 0;
	usize len = strlen(s);

	if (width < 0) {
		width = -width;
		right_pad = true;

		if (truncate && width < len)
			s += len - width;
	}

	for (usize i = len; !right_pad && i < width; i++)
		putc(' ');

	for (usize i = 0; *s && (!truncate || i < width); i++)
		putc(*s++);

	if (right_pad && len < width)
		for (usize i = 0; i < width - len; i++)
			putc(' ');
}

void printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	i32 width = 1;

	bool special = false;
	bool truncate = false;
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
				print_string(s, width, truncate);
				break;
			}
			case 'd': {
				i32 value = (i32) va_arg(va, i32);
				print_decimal(value, width);
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
			case '.': {
				truncate = true;
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
