.globl _putc
_putc:
	mov x9, 0x09000000
	str x0, [x9]
	ret

.globl _shutdown
_shutdown:
	ldr x0, =0x84000008
	hvc #0
