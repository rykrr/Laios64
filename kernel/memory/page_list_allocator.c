#if 0

void page_list_allocator_init_range(
	struct fdt_memory_range range,
	struct fdt_memory_range *rsvlist,
	usize nrsvs
) {
	// TODO: Handle case where address is not aligned by skipping unaligned pages.
	if (range.address != ALIGN_DOWN_PAGE(range.address))
		panic("Memory range not aligned to page boundary.");

	// Skip reservations before range.
	// NOTE: Assuming rsvlist is sorted by address.
	while (nrsvs && ALIGN_DOWN_PAGE(rsvlist->address) < range.address) {
		rsvlist++;
		nrsvs--;
	}

	usize total_pages = ALIGN_UP_PAGE(range.size) >> PAGE_ORDER;
	uptr start_page = range.address;
	uptr end_page = start_page + (total_pages * PAGE_SIZE);

	uptr current_page = start_page;
	struct page *prev_page = g_page_list_allocator.next_free_page;

	while (current_page < end_page) {
		uptr next_rsv_start = 0;
		uptr next_rsv_end = 0;

		if (nrsvs) {
			next_rsv_start = ALIGN_DOWN_PAGE(rsvlist->address);
			next_rsv_end = next_rsv_start + ALIGN_UP_PAGE(rsvlist->size);

			// If the next reservation is outside of the range, ignore remaining.
			if (end_page < next_rsv_end)
				nrsvs = 0;
		}

		usize pages_to_init = (end_page - current_page) >> PAGE_ORDER;

		if (nrsvs) {
			if (current_page < next_rsv_start) {
				// Calculate # of pages between current page and rsvlist pages.
				pages_to_init = (next_rsv_start - current_page) >> PAGE_ORDER;
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
				.next = prev_page,
				.self = page,
				.magic = FREE_PAGE_MAGIC,
			};
			prev_page = (struct page*) current_page;
			current_page += PAGE_SIZE;
		}

		g_page_list_allocator.n_free_pages += pages_to_init;
	}

	g_page_list_allocator.n_pages += total_pages;
	g_page_list_allocator.next_free_page = prev_page;
}

#endif
