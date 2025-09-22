#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <kernel/memory.h>

#include <stdlib.h>

#define __MODULE_NAME__ "vmm"

u64 addr_to_table_entry(const void *addr) {
	return ((u64) addr & PAGE_ADDR_MASK_4KB)
		| VM_ENTRY_TYPE_TABLE;
}

u64 addr_to_page_entry(const void *addr) {
	return ((u64) addr & PAGE_ADDR_MASK_4KB)
		| VM_ENTRY_AF_ENABLE
		| VM_ENTRY_SH_OUTER
		| VM_ENTRY_TYPE_PAGE;
}

// Level 1 4KB
u64 addr_to_block_entry(const void *addr) {
	u64 block_addr = (u64) addr & 0x0000FFFFC0000000;
	return block_addr
		| VM_ENTRY_AF_ENABLE
		| VM_ENTRY_SH_OUTER
		| VM_ENTRY_TYPE_BLOCK;
}

u64 get_tcr_el1() {
	u64 tcr_el1;
	asm("mrs %0, TCR_EL1" : "=r"(tcr_el1));
	return tcr_el1;
}

void set_tcr_el1(const u64 tcr_el1) {
	asm("msr TCR_EL1, %0" : /* no outputs */ : "r"(tcr_el1));
}

u64 get_ttbr0_el1() {
	u64 ttbr0_el1;
	asm("mrs %0, TTBR0_EL1" : "=r"(ttbr0_el1));
	return ttbr0_el1;
}

void set_ttbr0_el1(const register uptr ttbr0_el1) {
	asm("msr TTBR0_EL1, %0" : /* no outputs */ : "r"(ttbr0_el1));
}

u64 get_ttbr1_el1() {
	u64 ttbr1_el1;
	asm("mrs %0, TTBR1_EL1" : "=r"(ttbr1_el1));
	return ttbr1_el1;
}

void set_ttbr1_el1(const register uptr ttbr1_el1) {
	asm("msr TTBR1_EL1, %0" : /* no outputs */ : "r"(ttbr1_el1));
}

void print_vm_registers() {
	u64 tcr_el1 = get_tcr_el1();
	kprintf("Translation Control Register EL1 (TCR_EL1)\n");
	kprintf("TCR_EL1:      %016X\n", (u64) tcr_el1);
	kprintf("TCR_EL1.EPD0: %d\n", GET_TCR_ELX_EPD0(tcr_el1));
	kprintf("TCR_EL1.EPD1: %d\n", GET_TCR_ELX_EPD1(tcr_el1));
	kprintf("TCR_EL1.IPS:  %d\n", GET_TCR_ELX_IPS(tcr_el1));
	kprintf("TCR_EL1.SH0:  %d\n", GET_TCR_ELX_SH0(tcr_el1));
	kprintf("TCR_EL1.SH1:  %d\n", GET_TCR_ELX_SH1(tcr_el1));
	kprintf("TCR_EL1.TG0:  %d\n", GET_TCR_ELX_TG0(tcr_el1));
	kprintf("TCR_EL1.TG1:  %d\n", GET_TCR_ELX_TG1(tcr_el1));
	kprintf("TCR_EL1.T0SZ: %d\n", GET_TCR_ELX_T0SZ(tcr_el1));
	kprintf("TCR_EL1.T1SZ: %d\n", GET_TCR_ELX_T1SZ(tcr_el1));
	kprintf("TCR_EL1.DS:   %d\n", GET_TCR_ELX_DS(tcr_el1));

	u64 ttbr0_el1 = get_ttbr0_el1();
	u64 ttbr1_el1 = get_ttbr1_el1();
	kprintf("TTBR0_EL1:    %016X\n", (u64) ttbr0_el1);
	kprintf("TTBR1_EL1:    %016X\n", (u64) ttbr1_el1);

	u64 id_aa64mmfr0_el1;
	asm("mrs %0, ID_AA64MMFR0_EL1" : "=r"(id_aa64mmfr0_el1));
	kprintf("ID_AA64MMFR0_EL1: %016X\n", (u64) id_aa64mmfr0_el1);

	u64 sctlr;
	asm("mrs %0, SCTLR_EL1" : "=r"(sctlr));
	kprintf("SCTLR_EL1: %016X\n", sctlr);
}

void vm_init() {
	u64 tcr_el1 = get_tcr_el1();
	u64 ttbr0_el1 = get_ttbr0_el1();
	u64 ttbr1_el1 = get_ttbr1_el1();

	if (!ttbr0_el1) {
		u64 **identity_map = (u64**) page_alloc();
		for (u16 i = 0; i < 512; i++) {
			u64 addr = (u64) i << 30; // 1GB Blocks
			identity_map[i] = addr_to_block_entry(addr);
		}

		u64 *target_page = (u64 *) page_alloc();
		*target_page = 0x00DEAD0000CAFE00;

		u64 *lookup_tables[] = {
			page_alloc(),
			page_alloc(),
			page_alloc(),
			page_alloc(),
		};

		for (i8 i = 2; i >= 0; i--)
			lookup_tables[i][0] =
				addr_to_table_entry(lookup_tables[i+1]);

		printf("LUT: %016X\n", (u64) lookup_tables[0]);

		for (u16 i = 0; i < 512; i++)
			lookup_tables[3][i] =
				addr_to_page_entry(target_page);

		for (u8 i = 0; i < 4; i++)
			printf("L%d @ %016X [ 0: %016X ... ]\n",
				i,
				(u64) lookup_tables[i],
				(u64) lookup_tables[i][0]);

		set_ttbr0_el1(identity_map);
		set_ttbr1_el1(lookup_tables[0]);
	}

	// Disable 52-bit address support and stuff
	SET_TCR_ELX_DS(tcr_el1, 0);

	// Set 48-bit address space
	SET_TCR_ELX_IPS(tcr_el1, TCR_ELX_IPS_48);

	// Set the appropriate Translation Granule (TGx) sizes
	SET_TCR_ELX_TG0(tcr_el1, 0); // 4KB
	SET_TCR_ELX_TG1(tcr_el1, 2); // 4KB (different for some reason)
	
	// Set translation address sizes (TxSZ)
	SET_TCR_ELX_T0SZ(tcr_el1, 16); // 64 - 16 = 48 bits
	SET_TCR_ELX_T1SZ(tcr_el1, 16); // "

	// Cacheability settings that I don't understand
	//SET_TCR_ELX_IRGN0(tcr_el1, 0b01);
	//SET_TCR_ELX_IRGN1(tcr_el1, 0b01);
	//SET_TCR_ELX_ORGN0(tcr_el1, 0b01);
	//SET_TCR_ELX_ORGN1(tcr_el1, 0b01);
	//SET_TCR_ELX_SH0(tcr_el1, TCR_ELX_SH_INNER_SHAREABLE);
	//SET_TCR_ELX_SH1(tcr_el1, TCR_ELX_SH_INNER_SHAREABLE);

	// Save changes to register.
	set_tcr_el1(tcr_el1);

	register u64 mair_el1 = 0xFF00;
	asm(
		"msr MAIR_EL1, %0\n\t"
		: /* no output */
		: "r"(mair_el1)
	);

	asm("mrs %0, MAIR_EL1" : "=r"(mair_el1));
	kprintf("MAIR_EL1: %016X\n", mair_el1);

	// Enable EL0 and EL1 stage 1 address translation
	asm(
		"mrs x0, SCTLR_EL1\n\t"
		"orr x0, x0, 1\n\t"
		"msr SCTLR_EL1, x0\n\t"
		"isb"
		: /* no output */
	);

}

void page_map(void *virt_addr, void *phys_addr, u32 flags) {

}

void page_unmap(void *virt_addr) {

}

void set_active_page_table(void*) {

}

void *virt_to_phys(void *virt_addr) {
	return NULL;
}

// Allocate/free virtual pages.
void *valloc() {
	return NULL;
}

void *vfree() {
	return NULL;
}

#undef __MODULE_NAME__
