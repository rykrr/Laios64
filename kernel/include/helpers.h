#include <types.h>

inline u64 get_bits(
	register u64 value,
	register u8 offset,
	register u64 mask) {

	return ((value >> offset) & mask);
}

inline void set_bits(
	register u64 *value,
	register u8 offset,
	register u64 mask) {

	*value = (value & ~(mask << offset))
			| (value << offset);
}
