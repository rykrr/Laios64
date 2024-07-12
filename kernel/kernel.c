#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/platform.h>
#include <string.h>
#include <endian.h>

int has_prefix(const char *prefix, const char *str) {
	return 0;
}

void print_fdt_header(const struct fdt_header *fdt_header) {
	u32 *dword_header = (u32*) fdt_header;

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

extern inline void prd(int d, const char *c, u32 dword) {
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
	if (be32toh(header->magic) != 0xD00DFEED) {
		panic("Could not find flat device tree header.");
	}

	struct fdt_parser parser = fdt_parser_init(header);

	puts(fdt_parser_node_name(&parser)); putc('\n');

	while (!str_starts_with(fdt_parser_node_name(&parser), "memory")) {
		if (!fdt_parser_next_node(&parser))
			panic("init_mem: could not find memory node in device tree.");
	}
}

void kmain() {
	puts("\n\n================================\n");
	puts("Hello World.\n");

	puts("Test: "); print_dword(0xAABBCCDD); putc('\n');
	puts("Test: "); print_dword(be32toh(htobe32(0xAABBCCDD)));
	puts("\n================================\n\n");

	u32 x;
	asm("mrs %0,CurrentEL" : "=r"(x));
	puts("CurrentEL: "); print_dword(x >> 2); putc('\n');
	asm("mrs %0,vbar_el1" : "=r"(x));
	puts("VBAR EL1:  "); print_dword(x); putc('\n');

	print_fdt_header((struct fdt_header*) RAM_BASE);
	memory_init((struct fdt_header*) RAM_BASE);
}
