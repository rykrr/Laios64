#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>
#include <kernel/virtual.h>

#include <stdlib.h>

#define __MODULE_NAME__ "vmem"

void virt_init(struct virt_allocator_header *header) {

	*header = (struct virt_allocator_header) {
		.n_bytes = 0,
		.n_free_bytes = VIRT_ALLOCATOR_MAX_SIZE,
		.head = {NULL},
	};

	/*
	header->head[VIRT_ALLOCATOR_MAX_ORDER - 1] = (struct virt_allocator_node) {
		.magic = VIRT_ALLOCATOR_FREE_MAGIC,
		.order = VIRT_ALLOCATOR_MAX_ORDER,
		.next = NULL,
	};
	*/

}

void *virt_alloc(struct virt_allocator_header *allocator, u8 order) {

	return NULL;

}

void virt_free(struct virt_allocator_header *allocator, void *p, u8 order) {

}

#undef __MODULE_NAME__
