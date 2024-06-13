#include "includes/bios.h"

void putc(char c) {
	_putc(c);
}

void puts(const char *s) {
	while (*s) putc(*s++);
}
