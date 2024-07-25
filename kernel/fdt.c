#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <kernel/stdio.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>

#define __MODULE_NAME__ "fdt"

const char fdt_header_labels[FDT_HEADER_ENTRIES][18] = {
	"magic",
	"totalsize",
	"off_dt_struct",
	"off_dt_strings",
	"off_mem_rsvmap",
	"version",
	"last_comp_version",
	"boot_cpuid_phys",
	"size_dt_strings",
	"size_dt_struct",
};

void swap_fdt_struct_endianness(u32 *ptr, u8 entries) {
	for (u8 i = 0; i < entries; i++)
		ptr[i] = be32toh(ptr[i]);
}

struct fdt_parser fdt_parser_init(struct fdt_header *header) {
	struct fdt_parser parser = {
		.header = header,
		.node_ptr = NULL,
		.prop_ptr = NULL,
		.end = 0,
		.addr_cells_str_off = 0,
		.size_cells_str_off = 0,
		.compatible_str_off = 0,
	};

	u32 off_dt_strings = be32toh(header->off_dt_strings);
	u32 size_dt_strings = be32toh(header->size_dt_strings);

	char *s = ((u8*) header) + off_dt_strings;

	for (usize i = 0; i < size_dt_strings; i++) {
		if (*(s+i) == '#') {
			if (streq(FDT_STR_ADDRESS_CELLS, s+i))
				parser.addr_cells_str_off = i;
			else if (streq(FDT_STR_SIZE_CELLS, s+i))
				parser.size_cells_str_off = i;
		}
		else if (streq("compatible", s+i)) {
			parser.compatible_str_off = i;
		}
		i += strlen(s+i);
	}

	return parser;
}

void fdt_parser_push(struct fdt_parser *parser, u32 *node_ptr) {
	if (parser->depth >= FDT_PARSER_MAX_DEPTH - 1)
		panic("FDT PARSE FAIL: PARSER STACK DEPTH EXCEEDED");
	parser->node_ptr = node_ptr;
	parser->prop_ptr = NULL;
	parser->addr_cells[parser->depth+1] = parser->addr_cells[parser->depth];
	parser->size_cells[parser->depth+1] = parser->size_cells[parser->depth];
	parser->depth++;
}

void fdt_parser_pop(struct fdt_parser *parser) {
	parser->prop_ptr = NULL;
	parser->depth--;
}

u8 fdt_parser_next_node(struct fdt_parser *parser) {
	if (parser->end)
		return FDT_PARSER_ERR;

	if (parser->node_ptr == NULL) {
		u32 off_dt_struct = be32toh(parser->header->off_dt_struct);
		fdt_parser_push(parser, ((u32*) parser->header) + (off_dt_struct >> 2));
		return FDT_PARSER_OK;
	}

	u32 *ptr = parser->node_ptr;
	u32 type = be32toh(*ptr);

	u32 off_dt_strings = be32toh(parser->header->off_dt_strings);

	if (type != FDT_BEGIN_NODE)
		panic("FDT PARSE FAIL: EXPECTED FDT_BEGIN_NODE");

	usize node_name_len = strlen((const char*)(ptr+1)) + 1;
	ptr += FDT_ALIGN_U32(node_name_len) + 1;

	for (;;) {
		type = be32toh(*ptr);
		switch (type) {
			case FDT_BEGIN_NODE:
				fdt_parser_push(parser, ptr);
				return FDT_PARSER_OK;

			case FDT_END_NODE:
				fdt_parser_pop(parser);
				ptr++; // Skip tag
				break;

			case FDT_PROP:
				ptr++; // Skip tag

				u32 len = be32toh(*ptr++);
				u32 off = be32toh(*ptr++);
				u32 size = be32toh(*ptr);

				u32 off_dt_strings = be32toh(parser->header->off_dt_strings);
				char *s = ((u8*) parser->header) + off_dt_strings;

				if (off == parser->addr_cells_str_off)
					parser->addr_cells[parser->depth] = size;
				else if (off == parser->size_cells_str_off)
					parser->size_cells[parser->depth] = size;

				// Skip property data
				ptr += FDT_ALIGN_U32(len);
				break;

			case FDT_NOP:
				ptr++; // Skip tag
				break;

			case FDT_END:
				parser->node_ptr = NULL;
				parser->prop_ptr = NULL;
				parser->end = 1;
				return FDT_PARSER_ERR;

			default:
				panic("FDT PARSE FAIL: INVALID TOKEN TYPE");
				return FDT_PARSER_ERR;
		}
	}

	return FDT_PARSER_ERR;
}

const char* fdt_parser_node_name(struct fdt_parser *parser) {
	if (parser->end)
		return NULL;
	return (const char*) (parser->node_ptr + 1);
}

u8 fdt_parser_find_next_node_by_prefix(struct fdt_parser *parser, const char *prefix) {
	while (fdt_parser_next_node(parser) == FDT_PARSER_OK)
		if (str_starts_with(fdt_parser_node_name(parser), prefix))
			return FDT_PARSER_OK;
	return FDT_PARSER_ERR;
}

u8 fdt_parser_next_prop(struct fdt_parser *parser) {
	if (parser->end)
		return FDT_PARSER_ERR;

	u32 *ptr = parser->prop_ptr;
	u32 type = be32toh(*ptr);

	if (ptr == NULL) {
		// Skip node tag
		ptr = parser->node_ptr + 1;
		usize node_name_len = strlen((const char*) ptr) + 1;
		ptr += FDT_ALIGN_U32(node_name_len);
	}
	else {
		ptr++; // Skip tag
		u32 len = be32toh(*ptr++);
		ptr += FDT_ALIGN_U32(len) + 1;
	}

	for (;;) {
		type = be32toh(*ptr);
		switch(type) {
			case FDT_NOP:
				ptr++;
				break;
			case FDT_PROP:
				parser->prop_ptr = ptr;
				return FDT_PARSER_OK;
			default:
				parser->prop_ptr = NULL;
				return FDT_PARSER_ERR;
		}
	}
}

const char *fdt_parser_prop_name(struct fdt_parser *parser) {
	if (!parser->prop_ptr)
		return NULL;

	u32 off_dt_strings = be32toh(parser->header->off_dt_strings);
	u32 offset = be32toh(*(parser->prop_ptr + 2));
	return ((u8*) parser->header) + off_dt_strings + offset;
}

u8 fdt_parser_find_next_prop_by_name(struct fdt_parser *parser, const char *name) {
	while (fdt_parser_next_prop(parser) == FDT_PARSER_OK)
		if (streq(fdt_parser_prop_name(parser), name))
			return FDT_PARSER_OK;
	return FDT_PARSER_ERR;
}

void fdt_parser_prop_strlist(struct fdt_parser *parser) {
	if (parser->prop_ptr == NULL)
		return;

	u32 *ptr = parser->prop_ptr + 1;
	u32 len = be32toh(*ptr++);
	u32 off = be32toh(*ptr++);

	if (off != parser->compatible_str_off)
		return;

	u8 *s = (u8*) ptr;
	printf("Prop String List Length: %04X\n", len);
	for (usize i = 0; i < len; i++) {
		printf("%s\n", s+i);
		i += strlen(s+i);
	}
	putc('\n');
}

usize fdt_parser_prop_reg(struct fdt_parser *parser, struct fdt_memory_range *ranges, usize len) {
	if (!parser->prop_ptr || !streq(fdt_parser_prop_name(parser), "reg")) {
		kputs("Error parsing reg: reg property not found.");
		return 0;
	}

	u32 prop_len = be32toh(*(parser->prop_ptr + 1)) >> 2;
	u32 name_off = be32toh(*(parser->prop_ptr + 2));

	u32 *ptr = (parser->prop_ptr + 3);

	u8 addr_cells = parser->addr_cells[parser->depth];
	u8 size_cells = parser->size_cells[parser->depth];

	if (prop_len < addr_cells + size_cells) {
		kputs("Error parsing reg: function expects an even number of reg values.");
		return 0;
	}

	usize nranges = 0;
	usize i = 0;

	while (i < prop_len && nranges < len) {
		ranges[nranges].address = 0;
		ranges[nranges].size = 0;

		for (usize k = 0; k < addr_cells; k++) {
			if (prop_len <= i) {
				kputs("Error parsing reg: insufficient #reg values");
				return 0;
			}

			ranges[nranges].address |= be32toh(*(ptr++)) << (k << 5);
			i++;
		}

		for (usize k = 0; k < size_cells; k++) {
			if (prop_len <= i) {
				kputs("Error parsing reg: insufficient #reg values");
				return 0;
			}

			ranges[nranges].size |= be32toh(*(ptr++)) << (k << 5);
			i++;
		}

		nranges++;
	}

	return nranges;
}

usize fdt_parser_mem_rsvlist(struct fdt_header *header, struct fdt_memory_range *ranges, usize len) {
	u8 *ptr = (u8*) header + be32toh(header->off_mem_rsvmap);
	struct fdt_memory_range *rsvlist = (struct fdt_memory_range*) ptr;
	usize nranges = 0;

	while (nranges < len && (rsvlist->address || rsvlist->size))
		ranges[nranges] = *rsvlist++;

	return nranges;
}

void fdt_parser_print(struct fdt_parser *parser) {
	kputs("Parser State");
	kprintf("node_ptr:   %8X\n", (u32) parser->node_ptr);
	kprintf("prop_ptr:   %8X\n", (u32) parser->prop_ptr);
	kprintf("depth:      %8d\n", (u32) parser->depth);
	kprintf("addr_cells: [ ");
	for (usize i = 0; i < parser->depth; i++)
		printf("%d:%d ", i, parser->addr_cells[i]);
	printf("]\n");
	kprintf("size_cells: [ ");
	for (usize i = 0; i < parser->depth; i++)
		printf("%d ", parser->size_cells[i]);
	printf("]\n");
}
#undef __MODULE_NAME__
