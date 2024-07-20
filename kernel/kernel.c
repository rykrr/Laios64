#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/platform.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <stdlib.h>

#define DIV "================================"

#define xkprintf(fmt, ...) \
	printf("[%-20s] " fmt, __FILE_NAME__, ##__VA_ARGS__)

#define xkputs(s) kprintf("%s\n", s)

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

	kputs(fdt_parser_node_name(&parser));
}

void test() {
	kprintf("Hi %X\n", 0xAABBCCDD);
}

void kmain() {
	kputs(DIV);

	kprintf("Test: AABBCCDD = %X\n", sizeof(u64));
	kprintf("Test: AABBCCDD = %X\n", 0xAABBCCDD);
	kprintf("Test: DDCCBBAA = %X\n", htobe32(0xAABBCCDD));
	kprintf("Test: deadbeef = %x\n", 0xDEADBEEF);
	kprintf("Test:     BEEF = %8X\n", 0xBEEF);

	kprintf("Test: 0123456789ABCDEF = %X\n", 0x012345678ABCDEF);

	kprintf("Test: atoi(\"64\")  = %d\n", atoi("64"));
	kprintf("Test: atoi(\"64\")  = %4d\n", atoi("64"));
	kprintf("Test: atoi(\"-64\") = %d\n", atoi("-64"));
	kprintf("Test: atoi(\"-64\") = %4d\n", atoi("-64"));
	kprintf("Test: atoi(\"1234567890\")  = %11d\n", atoi("1234567890"));
	kprintf("Test: atoi(\"-1234567890\") = %11d\n", atoi("-1234567890"));
	kprintf("Test: atoi(\"5\")           = %90d\n", atoi("5"));

	kprintf("Test: [%.8s] [%-.8s]\n", "0123456789ABCDEF", "0123456789ABCDEF");

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

