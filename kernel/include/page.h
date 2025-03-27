#ifndef __KERNEL_PAGE_ALLOCATOR_H__
#define __KERNEL_PAGE_ALLOCATOR_H__

#include <types.h>

#define PAGE_ORDER 12
#define PAGE_SIZE (1 << PAGE_ORDER)

#define PAGE_ALLOCATOR_SUPER_PAGE_ORDER 16
#define PAGE_ALLOCATOR_SUPER_PAGE_SIZE (1 << PAGE_ALLOCATOR_SUPER_PAGE_ORDER)
#define PAGE_ALLOCATOR_SUPER_PAGE_COUNT (PAGE_ALLOCATOR_SUPER_PAGE_SIZE >> PAGE_ORDER)

#define PAGE_ALLOCATOR_MIN_ORDER 8
#define PAGE_ALLOCATOR_MIN_SIZE (1 << PAGE_ALLOCATOR_MIN_ORDER)

#define PAGE_ALLOCATOR_MAX_ORDER 32

#define PAGE_ALLOCATOR_FREE_MAGIC 0xDEAD

extern struct page_allocator_header g_page_allocator;

struct page_allocator_node {
	u16 magic; // 0xDEAD
	u8 order;
	struct page_allocator_node *next;
};

struct page_allocator_header {
	usize n_bytes;
	usize n_free_bytes;
	u8 min_order;
	struct page_allocator_node *head[PAGE_ALLOCATOR_MAX_ORDER];
};

void page_allocator_init(
	struct page_allocator_header *allocator,
	struct fdt_memory_range range,
	struct fdt_memory_range *rsvlist,
	usize nrsvs
);

void page_allocator_print_status(struct page_allocator_header *allocator);
void page_allocator_free_pages(struct page_allocator_header *allocator, void *p, u8 order);
void *page_allocator_alloc(struct page_allocator_header *allocator, u8 order);

#endif
