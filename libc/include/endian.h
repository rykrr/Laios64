#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <types.h>

u16 switch_endian_word(u16);
u32 switch_endian_dword(u32);

#define htobe16(h) switch_endian_word((h))
#define htole16(h) (h)
#define be16toh(be) switch_endian_word((be))
#define le16toh(le) (h)

#define htobe32(h) switch_endian_dword((h))
#define htole32(h) (h)
#define be32toh(be) switch_endian_dword((be))
#define le32toh(le) (le)

#ifdef FALSE
#define htobe64(h) switch_endian_qword((h))
#define htole64(h) (h)
#define be64toh(be) switch_endian_qword((be))
#define le64toh(le) (le)
#endif

#endif
