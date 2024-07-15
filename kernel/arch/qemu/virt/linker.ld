ENTRY(_start)

FLASH_START = 0x000100000;
RAM_START = 0x040100000;

SECTIONS
{
	. = RAM_START;

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
}
