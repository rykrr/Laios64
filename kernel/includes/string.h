#ifndef __STRING_H__
#define __STRING_H__
#include "types.h"
#include <stddef.h>

int streq(const char *a, const char *b);
size_t strlen(const char *s);
void* memset(void *s, u8 c, size_t n);

int str_starts_with(const char *s, const char *prefix);

#endif
