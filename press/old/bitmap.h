/* Mixed and mashed from the linux kernel */

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DIV_ROUND_UP(n, d)	((n + d - 1) / d)
#define BITS_PER_BYTE		(8)
#define BITS_TO_BYTES(n)	DIV_ROUND_UP(n, BITS_PER_BYTE)
#define BYTES_TO_BITS(n)	(n * BITS_PER_BYTE)
#define SIZEOF_BITMAP(n)	BITS_TO_BYTES(n)

#define BIT_MASK(n)		((uint8_t)(1) << ((n) % BITS_PER_BYTE))
#define BIT_WORD(n)		((n) / BITS_PER_BYTE)

/* Init a bitmap of length nbits (malloced) */
static inline uint8_t *bitmap_init(uint64_t nbits)
{
	return malloc(SIZEOF_BITMAP(nbits));
}

/* Fill bitmap of length nbits with zeros */
static inline void bitmap_zero(uint8_t *addr, uint64_t nbits)
{
	memset(addr, 0, SIZEOF_BITMAP(nbits));
}

void set_bit(uint64_t bit, uint8_t *addr);
void clear_bit(uint64_t bit, uint8_t *addr);

uint8_t get_bit(uint64_t bit, const uint8_t *addr);

#endif /* bitmap.h */
