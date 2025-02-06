#include <kernel/memory.h>
#include <kernel/stdio.h>
#include <kernel/bios.h>

#define __MODULE_NAME__ "memory"

struct page *page_alloc() {
	if (!global_page_info.next_free_page) {
		kprintf("Warning! No free pages.\n");
		return NULL;
	}
	struct page *page = global_page_info.next_free_page;
	global_page_info.next_free_page = page->next_page;
	global_page_info.n_free_pages--;
	*page = (struct page) {};
	return page;
}

struct page *page_allocn(usize n) {
	struct page *head = global_page_info.next_free_page;
	struct page *tail = head;

	for (usize i = 0; i < n; i++) {
		if (!tail) {
			kprintf("Warning! No free pages.\n");
			return NULL;
		}
		tail = tail->next_page;
	}

	global_page_info.next_free_page = tail->next_page;
	global_page_info.n_free_pages -= n;
	tail->next_page = NULL;

	for (usize i = 0; i < n; i++) {
		tail = head->next_page;
		*head = (struct page) {};
		head = tail;
	}

	return head;
}

void page_free(struct page *page) {
	if (ALIGN_DOWN_PAGE((uptr) page) != (uptr) page)
		panic("Attempted to free a page that is not page aligned.");

	if (page->magic == FREE_PAGE_MAGIC && page->self == page)
		panic("Potentially attempted to double free a page.");

	*page = (struct page) {
		.next_page = global_page_info.next_free_page,
		.magic = FREE_PAGE_MAGIC,
		.self = page,
	};

	global_page_info.next_free_page = page;
	global_page_info.n_free_pages++;
}

#undef __MODULE_NAME__
