#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include <types.h>

extern volatile u8* UART0_DR;

/********************/
/** Linker Symbols **/
/********************/

extern volatile u8 __kernel_start;
extern volatile u8 __kernel_end;
extern volatile u8 __fdt_header;

extern volatile u8 __stack_top;
extern volatile u8 __stack_bottom;

/*************************/
/** Address Definitions **/
/*************************/

#define KERNEL_START (&__kernel_start)
#define KERNEL_END (&__kernel_end)
#define KERNEL_SIZE ((u64) KERNEL_END - (u64) KERNEL_START)

#define FDT_HEADER_ADDR (&__fdt_header)

// Qemu places the fdt header at the start of RAM
#define RAM_START FDT_HEADER_ADDR

// This is the size of the device tree + kernel at the start of RAM
#define RAM_PRELUDE_SIZE ((u64) KERNEL_END - (u64) RAM_START)

#endif
