/* Mixed and mashed from the linux kernel */

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DIV_ROUND_UP(n, d)	((n - 1) / d + 1)
#define BITS_PER_BYTE		(8)
#define BITS_TO_BYTES(n)	DIV_ROUND_UP(n, BITS_PER_BYTE)
#define BYTES_TO_BITS(n)	(n * BITS_PER_BYTE)
#define SIZEOF_BITMAP(n)	BITS_TO_BYTES(n)

#define BIT_MASK(n)		((uint8_t)(1) << ((n) % BITS_PER_BYTE))
#define BIT_WORD(n)		((n) / BITS_PER_BYTE)

#define DECLARE_TYPE_TO_BIN(type) char *type ## _to_bin(type val)
#define DEFINE_TYPE_TO_BIN(type) \
char *type ## _to_bin(type val) \
{ \
	char *buf; \
	int32_t i; \
	int8_t j; \
	uint16_t bits; \
\
	/* +1 for spaces and trailing '\0' */ \
	bits = sizeof (type) * (BITS_PER_BYTE + 1); \
	buf = malloc(bits); \
	i = bits - 1; \
	buf[i--] = '\0'; \
\
	while (i >= 0) { \
		for (j = 0; j < BITS_PER_BYTE; j++) { \
			buf[i--] = (val & 1) + '0'; \
			val >>= 1; \
		} \
		if (i >= 0) \
			buf[i--] = ' '; \
	} \
\
	return buf; \
}

DECLARE_TYPE_TO_BIN(int16_t);
DECLARE_TYPE_TO_BIN(uint8_t);

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
