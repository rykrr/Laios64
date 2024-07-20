#ifndef __BIOS_H__
#define __BIOS_H__

#include <types.h>

void bios_shutdown();

void panic(const char*);

void bios_putc(char);
void bios_puts(const char*);
void bios_puts2(const char*);

void print_byte(u8);
void print_word(u16);
void print_dword(u32);
void print_qword(u64);

#endif
