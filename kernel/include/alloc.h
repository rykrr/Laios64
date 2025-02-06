#ifndef __KERNEL_ALLOC_H__
#define __KERNEL_ALLOC_H__

#include <kernel/memory.h>

#define MALLOC_BLOCK_FREE 0
#define MALLOC_BLOCK_USED 1

struct malloc_header {
	struct malloc_header *next;
	struct malloc_header *prev;
	u32 size;
	u8 status;
	u8 _padding0;
	u16 _padding1;
} __attribute__((packed,aligned(__alignof__(u32))));

struct malloc_info {
	struct malloc_header *next;
};

void *malloc(usize);
void free(void*);

#endif
