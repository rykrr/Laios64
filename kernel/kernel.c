#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/platform.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <stdlib.h>

void print_fdt_header(const struct fdt_header *fdt_header) {
	u32 *dword_header = (u32*) fdt_header;

	char spaces[20] = {' ', [19] = '\0'};

	puts("Flat Device Tree Header");

	for (int i = 0; i < FDT_HEADER_ENTRIES; i++)
		printf("%-20s: %08X\n", fdt_header_labels[i], be32toh(dword_header[i]));
}

extern inline void prd(int d, const char *c, u32 dword) {
	for (usize i = 0; i < d << 2; i++)
		putc(' ');
	printf("%s: %8X\n", c, d);
}

void khalt() {
	puts("Halted.\n");
}

void memory_init(struct fdt_header *header) {
	if (be32toh(header->magic) != 0xD00DFEED) {
		panic("Could not find flat device tree header.");
	}

	struct fdt_parser parser = fdt_parser_init(header);

	while (!str_starts_with(fdt_parser_node_name(&parser), "memory")) {
		if (!fdt_parser_next_node(&parser))
			panic("Could not find memory node in device tree.");
	}

	puts(fdt_parser_node_name(&parser));
}

const char *DIV = "================================";

void kmain() {
	putc('\n');
	puts(DIV);

	printf("Hello world!\n");
	printf("Test: AABBCCDD = %X\n", 0xAABBCCDD);
	printf("Test: DDCCBBAA = %X\n", htobe32(0xAABBCCDD));
	printf("Test: deadbeef = %x\n", 0xDEADBEEF);
	printf("Test:     BEEF = %8X\n", 0xBEEF);

	printf("\nTest: atoi(\"64\") = %X\n", atoi("64"));

	puts(DIV);

	u32 x;
	asm("mrs %0,CurrentEL" : "=r"(x));
	printf("CurrentEL: %X\n", x >> 2);
	asm("mrs %0,vbar_el1" : "=r"(x));
	printf("VBAR EL1:  %X\n", x);

	puts(DIV);

	print_fdt_header((struct fdt_header*) RAM_BASE);

	puts(DIV);

	memory_init((struct fdt_header*) RAM_BASE);

	puts(DIV);

	printf("%X", 0xE);
}
