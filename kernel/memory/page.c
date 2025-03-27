#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>

#define __MODULE_NAME__ "page"

struct page_allocator_header g_page_allocator;

u8 find_msb(u64 x) {
	u8 b = 0;
	while (x >>= 1)
		b++;
	return b;
}

void find_rank(usize x, usize *rank, usize *count) {
	*rank = 0;
	while (!(x & 1)) {
		x >>= 1;
		(*rank)++;
	}
	*count = x;
}

void page_allocator_print_status(struct page_allocator_header *allocator) {
	struct page_allocator_node *node = NULL;
	kprintf("Physical Memory Allocator Status\n");

	for (u8 o = 0; o < PAGE_ALLOCATOR_MAX_ORDER; o++) {
		node = allocator->head[o];
		while (node) {
			kprintf("    %08X + %08X (%d)\n", node, 1 << o, o);
			node = node->next;
		}
	}
}

struct page_allocator_node*
page_allocator_partition(struct page_allocator_header *allocator, u8 target_order) {
	struct page_allocator_node *node = NULL;
	u8 order = target_order;

	if (target_order < PAGE_ORDER)
		panic("Error: page_allocator_partition() order too small!");

	while (order < PAGE_ALLOCATOR_MAX_ORDER) {
		if (allocator->head[order])
			break;
		order++;
	}

	if (order == PAGE_ALLOCATOR_MAX_ORDER) {
		panic("No order large enough");
		return NULL;
	}

	while (target_order < order) {
		node = allocator->head[order];

		// Advance current order allocator to next.
		allocator->head[order] = node->next;

		order--;

		// Move current node to lower order.
		// We know there are no available nodes at this level.
		allocator->head[order] = node;

		// Calculate buddy.
		node->next = ((uptr) node) + (1 << order);
		*(node->next) = (struct page_allocator_node) {
			.magic = PAGE_ALLOCATOR_FREE_MAGIC,
			.order = order,
			.next =  NULL,
		};
	}

	return node;
}

void *page_allocator_alloc(struct page_allocator_header *allocator, u8 order) {
	page_allocator_partition(allocator, order);

	struct page_allocator_node *node = allocator->head[order];
	allocator->head[order] = node->next;

	if (!node)
		return NULL;

	// Clear pages.
	u64 *n = (u64*) node;
	for (usize i = 0; i < (1 << order); i += 8) {
		*n = 0;
		n++;
	}

	return node;
}

/* PANIC IF DOUBLE FREE */
void page_allocator_double_free_check(
	const struct page_allocator_header *allocator,
	const struct page_allocator_node *node,
	u8 n_pages
) {
	struct page_allocator_node *free_node;

	for (usize i = 0; i < n_pages; i++) {
		#ifdef PAGE_ALLOCATOR_ENABLE_MAGIC_CHECK
		if (node->magic != PAGE_ALLOCATOR_FREE_MAGIC) {
			node = (struct page_allocator_node *) ((uptr) node + PAGE_SIZE);
			continue;
		}
		#endif

		for (u8 o = PAGE_ORDER; o < PAGE_ALLOCATOR_MAX_ORDER; o++) {
			uptr block = ALIGN_DOWN((uptr) node, 1 << o);
			for (free_node = allocator->head[o]; free_node; free_node = free_node->next) {
				if (block == (uptr) free_node)
					panic("FATAL: PAGE DOUBLE-FREE");
			}
		}

		node = (struct page_allocator_node *) ((uptr) node + PAGE_SIZE);
	}
}

// TODO: Add additional checks to ensure $p is within the allocator's memory range.
void page_allocator_free_pages(struct page_allocator_header *allocator, void *p, u8 order) {
	if (!allocator || !p)
		panic("Error: page_allocator_free_pages(null)");

	struct page_allocator_node *node = (struct page_allocator_node*) p;
	struct page_allocator_node *prev_free_ptr, *free_ptr;

	usize n_pages = 1 << (order - PAGE_ORDER);

	if ((uptr) p != ALIGN_DOWN((uptr) p, 1 << order)) {
		panic("Error: page_allocator_free_pages was passed a pointer\n"
			  "       that is not aligned to a region of specified order.");
	}

	page_allocator_double_free_check(allocator, node, n_pages);

	for (usize i = 0; i < (1 << (order - PAGE_ORDER)); i++) {
		if (node->magic == PAGE_ALLOCATOR_FREE_MAGIC) {
			for (u8 o = PAGE_ORDER; o < PAGE_ALLOCATOR_MAX_ORDER; o++) {
				uptr b = ALIGN_DOWN((uptr) p, 1 << o);
				for (free_ptr = allocator->head[o]; free_ptr; free_ptr = free_ptr->next) {
					if (b == (uptr) free_ptr)
						panic("FATAL: PAGE DOUBLE-FREE");
				}
			}
		}
	}

	// convert page(s) to free page.
	*node = (struct page_allocator_node) {
		.magic = PAGE_ALLOCATOR_FREE_MAGIC,
		.order = order,
		.next =  NULL,
	};

	u8 current_order = order;
	usize current_size = 1 << order;

	kprintf("Freeing pages @ %X (%d)\n", node, current_order);
	while (current_order < PAGE_ALLOCATOR_MAX_ORDER - 1) {
		uptr buddy = ALIGN_DOWN((uptr) node, current_size << 1);
		uptr align = buddy;

		if ((uptr) node == buddy)
			buddy += current_size;

		prev_free_ptr = NULL;
		free_ptr = allocator->head[current_order];

		// Search for buddy in current order.
		kprintf("Aligned to %X; Searching for buddeh %X in %d...", align, buddy, current_order);
		for (free_ptr = allocator->head[current_order]; free_ptr; free_ptr = free_ptr->next) {
			if (buddy == free_ptr)
				break;
			prev_free_ptr = free_ptr;
		}

		if (!free_ptr) {
			printf("Failed.\n");
			break;
		}

		// if buddy is free, remove buddy from free list.
		printf("OK\n", current_order, (uptr) free_ptr);
		if (prev_free_ptr)
			prev_free_ptr->next = free_ptr->next;
		else
			allocator->head[current_order] = free_ptr->next;

		current_order++;
		current_size <<= 1;
		node = (struct page_allocator_node *) ALIGN_DOWN((uptr) node, current_size);
	}

	kprintf("Adding to free list at %d\n", current_order);
	// Add page(s) to free list.
	node->next = allocator->head[current_order];
	allocator->head[current_order] = node;

	// Mark all pages as free.
	free_ptr = node;
	for (usize i = 0; i < (1 << (current_order - PAGE_ORDER)); i++) {
		free_ptr = (struct page_allocator_node *) ((uptr) free_ptr + PAGE_SIZE);
		free_ptr->magic = PAGE_ALLOCATOR_FREE_MAGIC;
	}
}

void page_allocator_init(
	struct page_allocator_header *allocator,
	struct fdt_memory_range range,
	struct fdt_memory_range *rsvlist,
	usize nrsvs
) {
	kprintf("Initializing physical memory allocator...\n");

	// Largest power of two that can fit within the specified range.
	u8 max_order = find_msb(range.size);

	uptr start = range.address;
	uptr end = start + range.size;

	u8 current_order = max_order;
	usize current_size = 1 << current_order;

	kprintf("Setting up allocator for %08X - %08X\n", start, end);

	// Initialize allocator structure.
	*allocator = (struct page_allocator_header) {
		.n_bytes = 0,
		.n_free_bytes = 0,
		.head = {NULL},
	};

	// Traverse the memory range, attempting to allocate the largest blocks possible.
	for (uptr p = start; p < end; p += current_size) {
		struct fdt_memory_range range = {
			.address = (u64) p,
			.size = current_size,
		};

		struct fdt_memory_range *rsv = range_has_rsv(range, rsvlist, nrsvs);
		uptr rsv_end = 0;
		uptr rsv_overlap_size = 0;

		if (rsv != NULL) {
			rsv_end = rsv->address + rsv->size;
			rsv_overlap_size = rsv_end - p;
		}

		if (rsv_overlap_size) {
			// While current block size is too large, reduce block size.
			while (rsv_overlap_size < current_size && PAGE_ORDER < current_order) {
				current_order--;
				current_size >>= 1;
			}

			kprintf("    [ Resv ] %08X + %08X\n", p, current_size, current_order);
			allocator->n_bytes += current_size;
			continue;
		}

		// While current block is the start of superblock, increase block size.
		while (current_order < PAGE_ALLOCATOR_MAX_ORDER
				&& p == ALIGN_DOWN(p, current_size << 1)) {
			rsv = range_has_rsv(range, rsvlist, nrsvs);

			if (rsv != NULL) {
				rsv_end = rsv->address + rsv->size;
				rsv_overlap_size = rsv_end - p;
			}

			// If superblock contains a reservation, abort increase.
			if (rsv_overlap_size)
				break;

			current_order++;
			current_size <<= 1;
		}

		kprintf("    [ Free ] %08X + %08X\n", p, current_size, current_order);

		struct page_allocator_node *new_node = (struct page_allocator_node*) p;
		*new_node = (struct page_allocator_node) {
			.magic = PAGE_ALLOCATOR_FREE_MAGIC,
			.order = current_order,
			.next =  NULL,
		};

		if (allocator->head[current_order] == NULL) {
			allocator->head[current_order] = new_node;
		}
		else if (allocator->head[current_order]) {
			new_node->next = allocator->head[current_order];
		}

		allocator->n_bytes += current_size;
		allocator->n_free_bytes += current_size;
	}
}

#undef __MODULE_NAME__
