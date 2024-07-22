#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/platform.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <stdlib.h>

#define __MODULE_NAME__ "kmain"

#define DIV "================================"

void print_fdt_header(const struct fdt_header *fdt_header) {
	u32 *dword_header = (u32*) fdt_header;
	kputs("Flat Device Tree Header");
	for (int i = 0; i < FDT_HEADER_ENTRIES; i++)
		kprintf("%-18s: %08X\n", fdt_header_labels[i], be32toh(dword_header[i]));
}

void khalt() {
	kputs("Halted.");
}

void memory_init(struct fdt_header *header) {
	if (be32toh(header->magic) != 0xD00DFEED)
		panic("Could not find flat device tree header.");

	struct fdt_parser parser = fdt_parser_init(header);

	while (!str_starts_with(fdt_parser_node_name(&parser), "memory")) {
		if (!fdt_parser_next_node(&parser))
			panic("Could not find memory node in device tree.");
	}

	kprintf("Memory Startup: %s\n", fdt_parser_node_name(&parser));
	fdt_parser_print(&parser);
}

void test() {
	kprintf("Hi %X\n", 0xAABBCCDD);
}

void kmain() {
	kputs(DIV);

	kprintf("%X %X\n", 5, 9);

	kputs(DIV);

	u32 x;
	asm("mrs %0,CurrentEL" : "=r"(x));
	kprintf("CurrentEL: %X\n", x >> 2);
	asm("mrs %0,vbar_el1" : "=r"(x));
	kprintf("VBAR EL1:  %X\n", x);

	kputs(DIV);

	print_fdt_header((struct fdt_header*) RAM_BASE);

	kputs(DIV);

	memory_init((struct fdt_header*) RAM_BASE);

	kputs(DIV);
}


#undef __MODULE_NAME__
