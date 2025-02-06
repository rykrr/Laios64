#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdbool.h>

#ifndef NULL
#define NULL ((void*) 0)
#endif

typedef __UINT8_TYPE__	u8;
typedef __UINT16_TYPE__	u16;
typedef __UINT32_TYPE__	u32;
typedef __UINT64_TYPE__	u64;

typedef __INT8_TYPE__	i8;
typedef __INT16_TYPE__	i16;
typedef __INT32_TYPE__	i32;
typedef __INT64_TYPE__	i64;

typedef u64	usize;
typedef u64	uptr;
typedef u64	umax;
typedef i64	imax;

#endif
