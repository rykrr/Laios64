#ifdef __COMPILE__
.globl bios_putc
bios_putc:
	mov x9, 0x09000000
	str x0, [x9]
	ret

.globl bios_shutdown
bios_shutdown:
	ldr x0, =0x84000008
	hvc #0
#endif
