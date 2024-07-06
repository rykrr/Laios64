#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/platform.h>
#include <endian.h>

int has_prefix(const char *prefix, const char *str) {
	return 0;
}

void panic(const char *msg) {
	puts("\n\n!!! PANIC !!!\n\n");
	puts(msg);
	puts("\n\n\n\n");
	_shutdown();
}

void print_fdt_header(const struct fdt_header *fdt_header) {
	uint32_t *dword_header = (uint32_t *) fdt_header;

	puts("Flat Device Tree Header\n");

	for (int i = 0; i < FDT_HEADER_ENTRIES; i++) {
		puts(fdt_header_labels[i]);

		int s = 0;
		while (fdt_header_labels[i][s])
			s++;

		for (; s < 18; s++)
			putc(' ');

		puts(": ");
		print_dword(dword_header[i]);
		putc('\n');
	}
}

extern inline void print_prefix_spaces(int d) {
	for (int i = 0; i < d << 2; i++)
		putc(' ');
}

extern inline void prd(int d, const char *c, uint32_t dword) {
	print_prefix_spaces(d);
	puts(c);
	puts(": ");
	print_dword(dword);
	putc('\n');
}

void khalt() {
	puts("Halted.\n");
}

void memory_init(struct fdt_header *header) {
	struct fdt_parser parser = fdt_parser_init((struct fdt_header*) RAM_BASE);

	while (!str_starts_with(fdt_parser_node_name(&parser), "memory")) {
		if (!fdt_parser_next_node(&parser))
			panic("init_mem: could not find memory node in device tree.");
	}
}

void kmain() {
	puts("\n\n================================\n");
	puts("Hello World.\n");
	puts("Test: ");
	print_dword(0xDEADC0DE);
	putc('\n');
	print_dword(switch_endian_dword(0xDEADC0DE));
	puts("\n================================\n\n");

	u32 x;
	asm("mrs %0,CurrentEL" : "=r"(x));
	print_dword(x >> 2); putc('\n');
	asm("mrs %0,vbar_el1" : "=r"(x));
	print_dword(x); putc('\n');

	memory_init((struct fdt_header*) RAM_BASE);
}
