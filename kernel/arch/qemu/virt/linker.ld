ENTRY(_start)

SECTIONS
{
	. = 0x40000000;
	__fdt_header = .;

	. += 0x00100000;
	__kernel_start = .;

	.startup : {
		KEEP(*(.text.boot))
	}

	.text : {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}

	.data : {
		*(.data)
	}

	__bss_start = .;
	.bss : {
		bss = .;
		*(.bss)
	}

	. = ALIGN(4096);

	__bss_end = .;
	__bss_size = __bss_end - __bss_start;

	. = ALIGN(8);

	__stack_bottom = .;
	. += 0x10000;
	__stack_top = .;

	__kernel_end = .;
}
