#ifndef __STDIO_H__
#define __STDIO_H__

void putc(char);
void puts(const char *s);
void printf(const char *fmt, ...);

#ifdef __LIBK__

#define kprintf(fmt, ...) \
	printf("[%-20s] " fmt, __FILE_NAME__, ##__VA_ARGS__)

#define kputs(s) kprintf("%s\n", s)

#endif

#endif
