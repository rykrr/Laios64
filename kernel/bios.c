#include <kernel/bios.h>

void bios_puts(const char *s) {
	while (*s) bios_putc(*s++);
	bios_putc('\n');
}

void panic(const char *msg) {
	bios_puts("\n\n!!! PANIC !!!\n\n");
	bios_puts(msg);
	bios_puts("\n\n\n\n");
	bios_shutdown();
}
