#ifndef __KERNEL_MEMORY_H__
#define __KERNEL_MEMORY_H__

#include <kernel/fdt.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)

#define ALIGN_DOWN(addr,size) (addr & -size)
#define ALIGN_DOWN_PAGE(addr) ALIGN_DOWN(addr, PAGE_SIZE)

#define ALIGN_NEXT(addr,size) ((addr + (size-1)) & -size)
#define ALIGN_NEXT_PAGE(addr) ALIGN_NEXT(addr, PAGE_SIZE)

#define MEMORY_MAX_RANGES 16

#define FREE_PAGE_MAGIC 0xDEADDEADDEADDEAD

struct page {
	struct page *next_page;
	struct page *self;
	umax magic;
};

struct memory_page_info {
	usize n_pages;
	usize n_free_pages;
	struct page *next_free_page;
};

extern struct memory_page_info global_page_info;

void page_init(struct fdt_header*);

// Returns a single page.
struct page *page_alloc();

// Returns a linked list of pages.
struct page *page_allocn(usize);

// Frees a page.
// [UNSAFE] Minimal checks for double-free.
void page_free(struct page*);

#endif
