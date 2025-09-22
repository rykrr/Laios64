#ifndef __KERNEL_VIRT_ALLOCATOR_H__
#define __KERNEL_VIRT_ALLOCATOR_H__

#include <types.h>

#define VIRT_ALLOCATOR_FREE 0xFF

#define VIRT_ALLOCATOR_MIN_ORDER 8
#define VIRT_ALLOCATOR_MIN_SIZE (1 << VIRT_ALLOCATOR_MIN_ORDER)

#define VIRT_ALLOCATOR_MAX_ORDER 32
#define VIRT_ALLOCATOR_MAX_SIZE (1 << VIRT_ALLOCATOR_MAX_ORDER)


struct virt_allocator_node {
	u64 status;
	u64 size;
	struct virt_allocator_node *prev, *next;
};

struct virt_allocator_header {
	usize n_bytes;
	usize n_free_bytes;
	struct virt_allocator_node *head[VIRT_ALLOCATOR_MAX_ORDER];
};

void virt_init(struct virt_allocator_header *header);

void *virt_alloc(struct virt_allocator_header *allocator, u8 order);
void virt_free(struct virt_allocator_header *allocator, void *p, u8 order);

#endif
