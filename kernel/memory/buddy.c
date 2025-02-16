#include <kernel/bios.h>
#include <kernel/buddy.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>

#define __MODULE_NAME__ "buddy"

struct buddy_alloc_header g_buddy_page_allocator;

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

void buddy_alloc_print(struct buddy_alloc_header *buddy) {
	struct buddy_alloc_node *node = buddy->head;
	kprintf("Buddy Allocator Status\n");

	while (node) {
		kprintf("    %08X + %08X (%d)\n", node, 1 << node->order, node->order);
		if (!node->next)
			node = node->child;
		else
			node = node->next;
	}
}

void *buddy_alloc(struct buddy_alloc_header *buddy, usize size) {
	usize asize = ALIGN_UP(size, 1 << buddy->min_order);

	u8 msb = find_msb(asize);
	kprintf("Request for %d bytes rounded to %d bytes (msb %d)\n", size, asize, msb);

	struct buddy_alloc_node *node = buddy->head;
	while (node) {
		kprintf("    %08X + %08X (%d)\n", node, 1 << node->order, node->order);
		if (node->child) {
			if (asize <= (1 << node->child->order))
				node = node->child;
		}
		else {
			break;
		}
	}
	
	return NULL;
}

void buddy_alloc_init(
	struct buddy_alloc_header *buddy,
	struct fdt_memory_range range,
	struct fdt_memory_range *rsvlist,
	usize nrsvs,
	u8 min_order
) {
	kprintf("Initializing buddy allocator...\n");

	#define print_row(key, value) \
		kprintf("    %-12s : %08X (%d)\n", key, value, value);

	u8 max_order = find_msb(range.size);

	print_row("Min Order", min_order);
	print_row("Min Size", 1 << min_order);
	print_row("Min Blocks", range.size >> min_order);
	print_row("Tracking Pages", ALIGN_UP_PAGE((range.size >> min_order >> 3)) >> PAGE_ORDER);
	print_row("TEST", ALIGN_UP_PAGE(1));
	print_row("Max Order", max_order);
	print_row("Max Size", 1 << max_order);
	print_row("Max Blocks", range.size >> max_order);
	print_row("Min / Max", 1 << (max_order - min_order));
	print_row("Orders", max_order - min_order);
	kprintf("\n");

	#undef print_row

	uptr start = range.address;
	uptr end = start + range.size;

	u8 current_order = max_order;
	usize current_size = 1 << current_order;

	kprintf("Setting up allocation for %08X - %08X\n", start, end);
	//kprintf("Setting up allocation for %08X - %08X\n", range.address, range.address + range.size);

	struct buddy_alloc_node *head = NULL;

	for (uptr p = start; p < end; p += current_size) {
		// if there is a reservation that takes up the entire block, skip this block (continue)
		struct fdt_memory_range range = {
			.address = (u64) p,
			.size = current_size,
		};

		struct fdt_memory_range *rsv = range_has_rsv(range, rsvlist, nrsvs);
		uptr rsv_end = 0;
		uptr sz = 0;

		if (rsv != NULL) {
			rsv_end = rsv->address + rsv->size;
			sz = rsv_end - p;
		}

		if (sz) {
			// while current block size is too large, reduce block size
			while (sz < current_size && min_order < current_order) {
				current_order--;
				current_size >>= 1;
			}

			kprintf("    [ Resv ] %08X + %08X\n", p, current_size, current_order);
		}
		else {
			// while current block is the start of superblock, increase block size
			while (p == ALIGN_DOWN(p, current_size << 1)) {
				rsv = range_has_rsv(range, rsvlist, nrsvs);

				if (rsv != NULL) {
					rsv_end = rsv->address + rsv->size;
					sz = rsv_end - p;
				}

				// if superblock contains a reservation, abort increase
				if (sz)
					break;

				current_order++;
				current_size <<= 1;
			}

			kprintf("    [ Free ] %08X + %08X\n", p, current_size, current_order);

			struct buddy_alloc_node *new_node = (struct buddy_alloc_node*) p;
			*new_node = (struct buddy_alloc_node) {
				.magic = BUDDY_FREE_MAGIC,
				.order = current_order,
				.next =  NULL,
				.child = NULL,
			};

			if (head == NULL) {
				head = new_node;
			}
			else if (head->order < current_order) {
				new_node->child = head;
				head = new_node;
			}
			else if (head->order == current_order) {
				new_node->next = head;
				new_node->child = head->child;
				head = new_node;
			}
			else {
				struct buddy_alloc_node *parent = NULL, *node = head;

				while (node->child && current_order < node->child->order) {
					parent = node;
					node = node->child;
				}

				new_node->next = node->next;
				new_node->child = node->child;

				for (node = parent; parent; parent = parent->next)
					node->child = new_node;
			}
		}
	}

	// Initialize *buddy
	*buddy = (struct buddy_alloc_header) {
		.n_bytes = 0,
		.n_free_bytes = 0,
		.min_order = min_order,
		.head = head,
	};
}

#undef __MODULE_NAME__
