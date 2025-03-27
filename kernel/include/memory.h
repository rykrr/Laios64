#ifndef __KERNEL_MEMORY_H__
#define __KERNEL_MEMORY_H__

#include <kernel/fdt.h>
#include <kernel/page.h>

#define ALIGN_DOWN(addr,size) ((addr) & -(size))
#define ALIGN_DOWN_PAGE(addr) ALIGN_DOWN(addr, PAGE_SIZE)

#define ALIGN_UP(addr,size) (((addr) + ((size)-1)) & -(size))
#define ALIGN_UP_PAGE(addr) ALIGN_UP(addr, PAGE_SIZE)

#define MEMORY_MAX_RANGES 16


extern struct physical_allocator_header g_physical_page_allocator;

usize memory_get_ranges(struct fdt_header *header, struct fdt_memory_range *ranges, usize len);
usize memory_get_rsvlist(struct fdt_header *header, struct fdt_memory_range *rsvlist, usize len);
struct fdt_memory_range *range_has_rsv(struct fdt_memory_range range, struct fdt_memory_range *rsvlist, usize nrsvs);

void memory_init(struct fdt_header*);

// Allocate/free a single page.
void *page_alloc();
void page_free(void*);


#endif
