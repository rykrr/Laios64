#ifdef __COMPILE__
.globl _vector_table
_vector_table:
_el0_syn_handler:
	mov x0, xzr
	eret
.balign 128
_el0_irq_handler:
	mov x0, xzr
	eret
.balign 128
_el0_fiq_handler:
	mov x0, xzr
	eret
.balign 128
_el0_err_handler:
	mov x0, xzr
	eret
.balign 128
_el1_vector_table:
_el1_syn_handler:
	stp x0, x1, [sp, #-0x10]!
	stp x9, x10, [sp, #-0x10]!
	adr x0, _el1_format_str
	mrs x1, esr_el1
	bl printf
	ldp x9, x10, [sp], #0x10
	ldp x0, x1, [sp], #0x10
	/*
	;;;; mov x9, 0x09000000
	;;;; str x0, [x9]
	;;;; mov x0, xzr
	;;;; mrs x0, esr_el1
	;;;; bl print_qword
	;;;; mov x0, '\n'
	;;;; mov x9, 0x09000000
	;;;; str x0, [x9]
	;;;; mrs x0, hcr_el2
	;;;; bl print_qword
	;;;; mov x0, '\n'
	;;;; mov x9, 0x09000000
	;;;; str x0, [x9]
	*/
	b bios_shutdown
	eret
.balign 128
_el1_irq_handler:
	mov x0, xzr
	eret
.balign 128
_el1_fiq_handler:
	mov x0, xzr
	eret
.balign 128
_el1_err_handler:
	mov x0, xzr
	eret
.balign 128

.section .rodata

_el1_format_str:
	.asciz "Exception EL1 [ESR %8X]\n"

#endif
