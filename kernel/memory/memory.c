#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>
#include <kernel/platform.h>

#define __MODULE_NAME__ "memory"

usize memory_get_ranges(struct fdt_header *header, struct fdt_memory_range *ranges, usize len) {
	usize nranges = 0;
	struct fdt_parser parser = fdt_parser_init(header);
	while (fdt_parser_find_next_node_by_prefix(&parser, "memory") == FDT_PARSER_OK) {
		kprintf("Found %s\n", fdt_parser_node_name(&parser));

		if (fdt_parser_find_next_prop_by_name(&parser, "reg") == FDT_PARSER_OK)
			nranges += fdt_parser_prop_reg(&parser, ranges + nranges, len - nranges);
	}
	return nranges;
}

// TODO: Enforce ordering of reservation list.
usize memory_get_rsvlist(struct fdt_header *header, struct fdt_memory_range *rsvlist, usize len) {
	rsvlist[0] = (struct fdt_memory_range) {
		.address = (uptr) RAM_START,
		.size = ALIGN_UP_PAGE(RAM_PRELUDE_SIZE),
	};
	return fdt_parser_mem_rsvlist(header, rsvlist+1, MEMORY_MAX_RANGES) + 1;
}

struct fdt_memory_range *range_has_rsv(struct fdt_memory_range range, struct fdt_memory_range *rsvlist, usize nrsvs) {
	uptr start = range.address;
	uptr end = start + range.size;

	for (; nrsvs > 0; nrsvs--) {
		uptr rsv_start = rsvlist->address;
		uptr rsv_end = rsv_start + rsvlist->size;
		#define in_range(value) (rsv_start <= value && value < rsv_end)

		if (in_range(start) || in_range(end))
			return rsvlist;

		#undef in_range
		rsvlist++;
	}

	return NULL;
}

void memory_init(struct fdt_header *header) {
	kputs("Initializing page allocator...");

	if (!FDT_HEADER_MAGIC_OK(header))
		panic("Corrupt device tree header.");

	#if 0 /* DISABLED */
	g_page_list_allocator = (struct page_list_allocator_header) {
		.n_pages = 0,
		.n_free_pages = 0,
		.next_free_page = NULL,
	};
	#endif

	struct fdt_memory_range ranges[MEMORY_MAX_RANGES];
	usize nranges = memory_get_ranges(header, ranges, MEMORY_MAX_RANGES);
	usize total_bytes = 0;

	kprintf("\n");
	kprintf("Found %d memory range(s)\n", nranges);
	for (usize i = 0; i < nranges; i++) {
		kprintf("    %016X + %08X\n", ranges[i].address, ranges[i].size);
		total_bytes += ranges[i].size;
	}
	kprintf("\n");

	struct fdt_memory_range rsvlist[MEMORY_MAX_RANGES];
	usize nrsvs = memory_get_rsvlist(header, rsvlist, MEMORY_MAX_RANGES);

	kprintf("Found %d memory reservation(s)\n", nrsvs);
	for (usize i = 0; i < nrsvs; i++)
		kprintf("    %016X + %08X\n", rsvlist[i].address, rsvlist[i].size);
	kprintf("\n");

	//page_list_allocator_init_range(ranges[i].address, ranges[i].size, rsvlist, nrsvs);
	for (usize i = 0; i < nranges; i++)
		page_allocator_init(
			&g_page_allocator,
			ranges[i],
			rsvlist,
			nrsvs);
	kputs("");
	page_allocator_print_status(&g_page_allocator);

	#define print_row(prefix, size) \
		kprintf("%-6s | %8d | %5d | %10d\n",\
			prefix,\
			size >> PAGE_ORDER,\
			size >> 20,\
			size >> 10)

	kputs("");
	kprintf("%-6s | %8s | %5s | %10s\n", "Memory", "Pages", "~MiB", "KiB");
	kprintf("------ | -------- | ----- | ----------\n");
	print_row("Free", g_page_allocator.n_free_bytes);
	print_row("Used", (g_page_allocator.n_bytes - g_page_allocator.n_free_bytes));
	print_row("Total", g_page_allocator.n_bytes);
	kputs("");

	#undef print_row
}

inline void *page_alloc() {
	return page_allocator_alloc(&g_page_allocator, PAGE_ORDER);
}

inline void page_free(void *p) {
	page_allocator_free_pages(&g_page_allocator, p, PAGE_ORDER);
}


#undef __MODULE_NAME__
