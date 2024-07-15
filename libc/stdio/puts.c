#include <kernel/bios.h>

void puts(const char *s) {
	while (*s) bios_putc(*s++);
	bios_putc('\n');
}
