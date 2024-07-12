#include <kernel/fdt.h>
#include <kernel/bios.h>
#include <string.h>
#include <endian.h>

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
	puts("String 0: "); puts(s); putc('\n');

	for (usize i = 0; i < size_dt_strings; i++) {
		puts(s+i); putc('\n');
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

int fdt_parser_next_prop(struct fdt_parser *parser) {
	if (parser->end)
		return 0;

	u32 *ptr = parser->prop_ptr;
	u32 type = be32toh(*ptr);

	if (ptr == NULL) {
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
				return 1;
			default:
				parser->prop_ptr = NULL;
				return 0;
		}
	}
}

void fdt_parser_prop_string_list(struct fdt_parser *parser) {
	if (parser->prop_ptr == NULL)
		return;

	u32 *ptr = parser->prop_ptr + 1;
	u32 len = be32toh(*ptr++);
	u32 off = be32toh(*ptr++);

	if (off != parser->compatible_str_off)
		return;

	u8 *s = (u8*) ptr;
	print_dword(len); putc('\n');
	for (usize i = 0; i < len; i++) {
		puts(s+i); putc('\n');
		i += strlen(s+i);
	}
	putc('\n');
}

const char *fdt_parser_prop_name(struct fdt_parser *parser) {
	if (parser->prop_ptr == NULL)
		return NULL;

	u32 off_dt_strings = be32toh(parser->header->off_dt_strings);
	u32 offset = be32toh(*(parser->prop_ptr + 2));
	return ((u8*) parser->header) + off_dt_strings + offset;
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
	parser->depth--;
}

int fdt_parser_next_node(struct fdt_parser *parser) {
	if (parser->end)
		return 0;

	if (parser->node_ptr == NULL) {
		u32 off_dt_struct = be32toh(parser->header->off_dt_struct);
		fdt_parser_push(parser, ((u32*) parser->header) + (off_dt_struct >> 2));
		return 1;
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
				return 1;

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
				parser->end = 1;
				return 0;

			default:
				panic("FDT PARSE FAIL: INVALID TOKEN TYPE");
				return 0;
		}
	}

	return 0;
}

u64 fdt_parser_parse_reg(struct fdt_parser *parser) {

}

const char* fdt_parser_node_name(struct fdt_parser *parser) {
	if (parser->end)
		return NULL;
	return (const char*) (parser->node_ptr + 1);
}
