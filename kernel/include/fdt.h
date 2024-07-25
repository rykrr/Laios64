#ifndef __FDT_H__
#define __FDT_H__

#include <types.h>

// From: https://devicetree-specification.readthedocs.io
struct fdt_header {
	u32 magic;
	u32 totalsize;
	u32 off_dt_struct;
	u32 off_dt_strings;
	u32 off_mem_rsvmap;
	u32 version;
	u32 last_comp_version;
	u32 boot_cpuid_phys;
	u32 size_dt_strings;
	u32 size_dt_struct;
} __attribute__((packed,aligned(__alignof__(u32))));
#define FDT_HEADER_ENTRIES 10

extern const char fdt_header_labels[FDT_HEADER_ENTRIES][18];

struct fdt_prop_header {
	u32 length;
	u32 name_offset;
} __attribute__((packed,aligned(__alignof__(u32))));
#define FDT_PROP_HEADER_ENTRIES 2

struct fdt_memory_range {
	u64 address;
	u64 size;
} __attribute__((packed,aligned(__alignof__(u64))));
#define FDT_MEMORY_RANGE_ENTRIES 2

#define fdt_reserve_entry fdt_memory_range
#define FDT_RESERVE_HEADER_ENTRIES 2

/****************/
/** FDT Parser **/
/****************/

#define FDT_PARSER_MAX_DEPTH 8

#define FDT_PARSER_OK 1
#define FDT_PARSER_ERR 0

struct fdt_parser {
	struct fdt_header *header;
	u32 *node_ptr;
	u32 *prop_ptr;
	u32 addr_cells_str_off;
	u32 size_cells_str_off;
	u32 compatible_str_off;
	u8 depth;
	u8 end;
	u8 addr_cells[FDT_PARSER_MAX_DEPTH];
	u8 size_cells[FDT_PARSER_MAX_DEPTH];
};

struct fdt_parser fdt_parser_init(struct fdt_header*);

u8 fdt_parser_next_node(struct fdt_parser*);
const char* fdt_parser_node_name(struct fdt_parser*);

u8 fdt_parser_find_next_node_by_prefix(struct fdt_parser*, const char *);

u8 fdt_parser_next_prop(struct fdt_parser*);
const char* fdt_parser_prop_name(struct fdt_parser*);

u8 fdt_parser_find_next_prop_by_name(struct fdt_parser*, const char *);

void fdt_parser_prop_strlist(struct fdt_parser*);
usize fdt_parser_prop_reg(struct fdt_parser*, struct fdt_memory_range*, usize len);

usize fdt_parser_mem_rsvlist(struct fdt_header*, struct fdt_memory_range*, usize);

void fdt_parser_print(struct fdt_parser *parser);

/****************************/
/** FDT Helper Definitions **/
/****************************/

//#define FDT_ALIGN_U32(x) ((x >> 2) + !!(x & 3))
#define FDT_ALIGN_U32(x) (((x + 3) & -4) >> 2)


/***************************/
/** FDT Token Definitions **/
/***************************/

#define FDT_BEGIN_NODE 1
#define FDT_END_NODE 2
#define FDT_PROP 3
#define FDT_NOP 4
#define FDT_END 9

/************************/
/** FDT Common Strings **/
/************************/

#define FDT_STR_REG "reg"
#define FDT_STR_COMPATIBLE "compatible"
#define FDT_STR_SIZE_CELLS "#size-cells"
#define FDT_STR_ADDRESS_CELLS "#address-cells"
#define FDT_STR_INTERRUPT_CELLS "#interrupt-cells"

#endif
