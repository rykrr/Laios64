#include <kernel/bios.h>

// TODO: I/O descriptor support.

void putc(char c) {
	bios_putc(c);
}
