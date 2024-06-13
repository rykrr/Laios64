#ifndef __INT_UTILS_H_
#define __INT_UTILS_H_

#include "types.h"

extern const char * HEX_DIGITS;

void print_byte(u8);
void print_word(u16);
void print_dword(u32);
void print_qword(u32);

u16 switch_endian_word(u16);
u32 switch_endian_dword(u32);

#endif
