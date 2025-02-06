#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>
#include <kernel/platform.h>
#include <endian.h>

#define __MODULE_NAME__ "memory"

struct memory_page_info global_page_info;

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
		.size = ALIGN_NEXT_PAGE(RAM_PRELUDE_SIZE),
	};
	return fdt_parser_mem_rsvlist(header, rsvlist+1, MEMORY_MAX_RANGES) + 1;
}

void page_init_range(
	uptr address,
	usize size,
	struct fdt_memory_range *rsvlist,
	usize nrsvs
) {
	// TODO: Handle case where address is not aligned by skipping unaligned pages.
	if (address != ALIGN_DOWN_PAGE(address))
		panic("Memory range not aligned to page boundary.");

	// Assuming rsvlist is sorted by address, skip reservations before range.
	while (nrsvs && ALIGN_DOWN_PAGE(rsvlist->address) < address) {
		rsvlist++;
		nrsvs--;
	}

	usize total_pages = ALIGN_NEXT_PAGE(size) >> PAGE_SHIFT;
	uptr start_page = address;
	uptr end_page = start_page + (total_pages * PAGE_SIZE);

	uptr current_page = start_page;
	struct page *prev_page = global_page_info.next_free_page;

	while (current_page < end_page) {
		uptr next_rsv_start = 0;
		uptr next_rsv_end = 0;

		if (nrsvs) {
			next_rsv_start = ALIGN_DOWN_PAGE(rsvlist->address);
			next_rsv_end = next_rsv_start + ALIGN_NEXT_PAGE(rsvlist->size);

			// If the next reservation is outside of the range, ignore remaining.
			if (end_page < next_rsv_end)
				nrsvs = 0;
		}

		usize pages_to_init = (end_page - current_page) >> PAGE_SHIFT;

		if (nrsvs) {
			if (current_page < next_rsv_start) {
				// Calculate # of pages between current page and rsvlist pages.
				pages_to_init = (next_rsv_start - current_page) >> PAGE_SHIFT;
			}
			else {
				current_page = next_rsv_end;
				pages_to_init = 0;
				rsvlist++;
				nrsvs--;
			}
		}

		for (usize i = 0; i < pages_to_init; i++) {
			struct page *page = (struct page*) current_page;
			*page = (struct page) {
				.next_page = prev_page,
				.self = page,
				.magic = FREE_PAGE_MAGIC,
			};
			prev_page = (struct page*) current_page;
			current_page += PAGE_SIZE;
		}

		global_page_info.n_free_pages += pages_to_init;
	}

	global_page_info.n_pages += total_pages;
	global_page_info.next_free_page = prev_page;
}

void page_init(struct fdt_header *header) {
	kputs("Initializing physical pages...");

	if (!FDT_HEADER_MAGIC_OK(header))
		panic("Corrupt device tree header.");

	global_page_info = (struct memory_page_info) {
		.n_pages = 0,
		.n_free_pages = 0,
		.next_free_page = NULL,
	};

	struct fdt_memory_range ranges[MEMORY_MAX_RANGES];
	usize nranges = memory_get_ranges(header, ranges, MEMORY_MAX_RANGES);

	uptr bytes = 0;

	kprintf("Found %d memory range(s)\n", nranges);
	for (usize i = 0; i < nranges; i++) {
		kprintf("   %016X + %08X\n", ranges[i].address, ranges[i].size);
		bytes += ranges[i].size;
	}

	struct fdt_memory_range rsvlist[MEMORY_MAX_RANGES];
	usize nrsvs = memory_get_rsvlist(header, rsvlist, MEMORY_MAX_RANGES);

	kprintf("Found %d memory reservation(s)\n", nrsvs);
	for (usize i = 0; i < nrsvs; i++)
		kprintf("   %016X + %08X\n", rsvlist[i].address, rsvlist[i].size);

	kputs("Setting up pages...");

	for (usize i = 0; i < nranges; i++)
		page_init_range(ranges[i].address, ranges[i].size, rsvlist, nrsvs);

	#define print_row(prefix, size) \
		kprintf("%-6s | %8d | %5d | %10d\n",\
			prefix,\
			size >> PAGE_SHIFT,\
			size >> 20,\
			size >> 10)

	kputs("");
	kprintf("%-6s | %8s | %5s | %10s\n", "Memory", "Pages", "~MiB", "KiB");
	kprintf("------ | -------- | ----- | ----------\n");
	print_row("Free", global_page_info.n_free_pages << PAGE_SHIFT);
	print_row("Used",
			(global_page_info.n_pages - global_page_info.n_free_pages) << PAGE_SHIFT);
	print_row("Total", global_page_info.n_pages << PAGE_SHIFT);
	kputs("");

	#undef print_row
}

#undef __MODULE_NAME__
