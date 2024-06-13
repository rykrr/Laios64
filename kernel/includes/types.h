#ifndef __TYPES_H_
#define __TYPES_H_
#include <stdint.h>

#define NULL ((void*) 0)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef __attribute__((packed)) uint32_t packed_u32;
#endif
