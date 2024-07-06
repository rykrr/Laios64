#include "bios.s"
#include "exception.s"

.section .text.boot
.globl _start
_start:
	// Place stack before _start
	ldr x5, =_start
	mov sp, x5

	// Set exception table
	ldr x5, =_vector_table
	msr vbar_el1, x5

	// Only use the main core
	mrs x0, mpidr_el1
	and x0, x0, #3
	cbz x0, _clear_bss
	b _halt

_clear_bss:
	ldr x5, =__bss_start
	ldr w6, =__bss_size
1:	cbz w6, _main
	str xzr, [x5], #8
	sub w6, w6, #1
	cbnz w6, 1b

_main:
	bl kmain
	bl _shutdown

.globl _start
_halt:
	wfi
	b _halt
