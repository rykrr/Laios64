#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

u8 streq(const char *a, const char *b);
usize strlen(const char *s);
void* memset(void *s, u8 c, size_t n);

/*** Non-standard functions ***/

u8 str_starts_with(const char *s, const char *prefix);

#endif
