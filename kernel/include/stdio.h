#ifndef __KERNEL_STDIO_H__
#define __KERNEL_STDIO_H__

#include <stdio.h>

#ifdef __LIBK__
#define kprintf(fmt, ...) \
	printf("%-16s | " fmt, __MODULE_NAME__, ##__VA_ARGS__)

#define kputs(s) kprintf("%s\n", s)
#endif

#endif
