#include <kernel/bios.h>

void panic(const char *msg) {
	bios_puts("\n\n!!! PANIC !!!\n");
	bios_puts(msg);
	bios_puts("\n\n\n");
	bios_shutdown();
}
