#ifndef __BIOS_H__
#define __BIOS_H__

#include <types.h>

extern const char * HEX_DIGITS;

void shutdown();

void _putc(char);
void putc(char);
void puts(const char*);

void print_byte(u8);
void print_word(u16);
void print_dword(u32);
void print_qword(u64);

#endif
