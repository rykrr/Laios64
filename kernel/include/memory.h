#ifndef __KERNEL_MEMORY_H__
#define __KERNEL_MEMORY_H__

#include <kernel/fdt.h>
#include <kernel/page_allocator.h>

#define ALIGN_DOWN(addr,size) ((addr) & -(size))
#define ALIGN_DOWN_PAGE(addr) ALIGN_DOWN(addr, PAGE_SIZE)

#define ALIGN_UP(addr,size) (((addr) + ((size)-1)) & -(size))
#define ALIGN_UP_PAGE(addr) ALIGN_UP(addr, PAGE_SIZE)

#define MEMORY_MAX_RANGES 16

#define PAGE_ADDR_MASK_4KB 0x0000FFFFFFFFF000

usize memory_get_ranges(struct fdt_header *header, struct fdt_memory_range *ranges, usize len);
usize memory_get_rsvlist(struct fdt_header *header, struct fdt_memory_range *rsvlist, usize len);
struct fdt_memory_range *range_has_rsv(struct fdt_memory_range range, struct fdt_memory_range *rsvlist, usize nrsvs);

void memory_init(struct fdt_header*);

// Allocate/free a single physical page.
void *page_alloc();
void page_free(void*);


///////////////////////
// Translation Stuff //
///////////////////////

struct vm_page_entry {
	void *page_address;
	u8 tex	: 3;	// Type Extension
	u8 apx	: 2;	// Access Permission
	u8 ap	: 1;	// Access Permission
	u8 xn	: 1;	// Execute Never
	u8 ng	: 1;	// Non Global
	u8 b	: 1;	// Bufferable
	u8 c	: 1;	// Cacheable
	u8 s	: 1;	// Shareable
};

struct vm_table_entry {
	void *next_level_table; // Address of next-level table.
	// u8 ns_table	: 1; // Secure State
	// u8 ap_table	: 2; // Hierarchical Permissions Enable
	// u8 xnt_table: 1; // Single Priv / Dual Priv
	// u8 pxn_table: 1; // Single Priv OR EL1&0 Translation Regime
};

// MMU page mapping functions.
void page_map(void *virt_addr, void *phys_addr, u32 flags);
void page_unmap(void *virt_addr);

void set_active_page_table(void*);

void *virt_to_phys(void *virt_addr);

// Translation Granuale Size (Page Sizes) for TTBR0 and TTRB1
#define TCR_ELX_TG0 (1 << 14)
#define TCR_ELX_TG1 (1 << 30)

void mmu_enable();
void vm_init();

// Allocate/free virtual pages.
void *valloc();
void *vfree();


#define GET_TCR_ELX_REGISTER(tcr_elx, offset, mask) \
	((tcr_elx >> offset) & mask)

#define SET_TCR_ELX_REGISTER(tcr_elx, value, offset, mask) \
	tcr_elx = ((tcr_elx & ~((u64) mask << offset)) \
			| ((u64) value << offset))

#define GET_TCR_ELX_EPD0(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 7, 0b1)

#define SET_TCR_ELX_EPD0(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 7, 0b1)

#define GET_TCR_ELX_EPD1(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 23, 0b1)

#define SET_TCR_ELX_EPD1(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 23, 0b1)

#define SET_TCR_ELX_IRGN0(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 8, 0b11)

#define SET_TCR_ELX_IRGN1(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 24, 0b11)

#define SET_TCR_ELX_ORGN0(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 10, 0b11)

#define SET_TCR_ELX_ORGN1(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 26, 0b11)

#define GET_TCR_ELX_T0SZ(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx,  0, 0x1F)

#define SET_TCR_ELX_T0SZ(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 0, 0x1F)

#define GET_TCR_ELX_T1SZ(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 16, 0x1F)

#define SET_TCR_ELX_T1SZ(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 16, 0x1F)

#define SET_TCR_ELX_T1SZ(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 16, 0x1F)

#define GET_TCR_ELX_TG0(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 14, 0b11)

#define SET_TCR_ELX_TG0(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 14, 0b11)

#define GET_TCR_ELX_TG1(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 30, 0b11)

#define SET_TCR_ELX_TG1(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 30, 0b11)

#define GET_TCR_ELX_DS(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 59, 0b1)

#define SET_TCR_ELX_DS(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 59, 0b1)

#define GET_TCR_ELX_SH0(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 12, 0b11)

#define SET_TCR_ELX_SH0(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 12, 0b11)

#define GET_TCR_ELX_SH1(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 28, 0b11)

#define SET_TCR_ELX_SH1(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 28, 0b11)

#define TCR_ELX_SH_NON_SHAREABLE   0b00
#define TCR_ELX_SH_OUTER_SHAREABLE 0b10
#define TCR_ELX_SH_INNER_SHAREABLE 0b11

#define GET_TCR_ELX_IPS(tcr_elx) \
	GET_TCR_ELX_REGISTER(tcr_elx, 32, 0b111)

#define SET_TCR_ELX_IPS(tcr_elx, value) \
	SET_TCR_ELX_REGISTER(tcr_elx, value, 32, 0b111)

#define TCR_ELX_IPS_32 0b000
#define TCR_ELX_IPS_36 0b001
#define TCR_ELX_IPS_40 0b010
#define TCR_ELX_IPS_42 0b011
#define TCR_ELX_IPS_44 0b100
#define TCR_ELX_IPS_48 0b101
#define TCR_ELX_IPS_52 0b110
#define TCR_ELX_IPS_56 0b111

#define VM_ENTRY_TYPE_TABLE 0b11
#define VM_ENTRY_TYPE_BLOCK 0b01
#define VM_ENTRY_TYPE_PAGE  0b11

#define VM_ENTRY_AF_ENABLE (1 << 10)
#define VM_ENTRY_SH_INNER (0b11 << 10)
#define VM_ENTRY_SH_OUTER (0b10 << 10)
#define VM_TABLE_ENTRY_AP_PRWURW (0b01 << 61)

void print_vm_registers();

#endif
