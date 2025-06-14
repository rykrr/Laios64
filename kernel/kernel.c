#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>
#include <kernel/platform.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <stdlib.h>

#define __MODULE_NAME__ "kmain"

#define DIV "================================================"

void print_fdt_header(const struct fdt_header *fdt_header) {
	u32 *dword_header = (u32*) fdt_header;
	kputs("Flat Device Tree Header");
	for (int i = 0; i < FDT_HEADER_ENTRIES; i++)
		kprintf("    %-18s: %08X\n", fdt_header_labels[i], be32toh(dword_header[i]));
}

void khalt() {
	kputs("Halted.");
}

void test() {
	kprintf("Hi %X\n", 0xAABBCCDD);
}

void kmain() {
	kputs(DIV);

	u32 x;
	asm("mrs %0,CurrentEL" : "=r"(x));
	kprintf("CurrentEL: %X\n", x >> 2);
	asm("mrs %0,vbar_el1" : "=r"(x));
	kprintf("VBAR EL1:  %X\n", x);

	kputs(DIV);

	struct fdt_header *fdt_header = (struct fdt_header*) &__fdt_header;
	print_fdt_header(fdt_header);

	kputs(DIV);
	memory_init(fdt_header);

	#define N 8
	void *p[N] = {NULL};

	for (usize i = 0; i < N; i++) {
		p[i] = page_alloc();
		kprintf("[%d] AllocatedPage %08X\n", i, p[i]);
	}
	page_allocator_print_status(&g_page_allocator);

	for (i8 i = N - 1; i >= 0; i--) {
		kprintf("[%d] Freeing Page %08X %08X\n", i, p[i], *((u64*) p[i]));
		page_free(p[i]);
	}

	page_allocator_print_status(&g_page_allocator);

	kputs(DIV);
}

#undef __MODULE_NAME__
