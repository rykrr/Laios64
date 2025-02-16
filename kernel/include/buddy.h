#ifndef __KERNEL_BUDDY_H__
#define __KERNEL_BUDDY_H__

#include <kernel/fdt.h>

#define BUDDY_SUPER_PAGE_ORDER 16
#define BUDDY_SUPER_PAGE_SIZE (1 << BUDDY_SUPER_PAGE_ORDER)
#define BUDDY_SUPER_PAGE_COUNT (BUDDY_SUPER_PAGE_SIZE >> PAGE_ORDER)

#define BUDDY_MIN_ORDER 8
#define BUDDY_MIN_SIZE (1 << BUDDY_MIN_ORDER)

#define BUDDY_FREE_MAGIC 0xDEAD

struct buddy_alloc_node {
	u16 magic; // 0xDEAD
	u8 order;
	struct buddy_alloc_node *next, *child;
};

struct buddy_alloc_header {
	usize n_bytes;
	usize n_free_bytes;
	u8 min_order;
	struct buddy_alloc_node *head;
};

void buddy_alloc_init(
	struct buddy_alloc_header *buddy,
	struct fdt_memory_range range,
	struct fdt_memory_range *rsvlist,
	usize nrsvs,
	u8 min_order
);

void buddy_alloc_print(struct buddy_alloc_header *buddy);
void *buddy_alloc(struct buddy_alloc_header *buddy, usize size);

#endif
