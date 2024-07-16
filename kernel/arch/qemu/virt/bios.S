#ifdef __COMPILE__
.equ PL011_UART_WRITE, 0x09000000

.globl bios_putc
bios_putc:
	mov x9, PL011_UART_WRITE
	str w0, [x9]
	ret

.globl bios_puts
bios_puts:
	mov x9, PL011_UART_WRITE
1:	ldrb w10, [x0]
	cbz w10, 2f
	str w10, [x9]
	add x0, x0, #1
	b 1b
2:	mov w10, #0xA
	str w10, [x9]
	ret

.globl bios_shutdown
bios_shutdown:
	ldr x0, =0x84000008
	hvc #0
#endif
