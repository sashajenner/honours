#include "bitmap.h"
#include <stdint.h>

/*
 * Set bit in bitmap addr.
 * addr cannot be NULL.
 */
void set_bit(uint64_t bit, uint8_t *addr)
{
	uint8_t mask = BIT_MASK(bit);

	addr += BIT_WORD(bit);
	*addr |= mask;
}

/*
 * Clear bit in bitmap addr.
 * addr cannot be NULL.
 */
void clear_bit(uint64_t bit, uint8_t *addr)
{
	uint64_t mask = BIT_MASK(bit);

	addr += BIT_WORD(bit);
	*addr &= ~mask;
}

uint8_t get_bit(uint64_t bit, const uint8_t *addr)
{
	uint8_t onoff;
	uint64_t mask = BIT_MASK(bit);

	addr += BIT_WORD(bit);
	onoff = *addr & mask;

	return onoff;
}

// buffer must have length >= sizeof(int) + 1
// Write to the buffer backwards so that the binary representation
// is in the correct order i.e.  the LSB is on the far right
// instead of the far left of the printed string
char *int2bin(int16_t a, char *buffer, int buf_size) {
	/*fprintf(stderr, "%" PRId16 "\n", a);*/
	buffer += (buf_size - 1);

	for (int i = 15; i >= 0; i--) {
		*buffer-- = (a & 1) + '0';

		a >>= 1;
	}

	return buffer;
}

DEFINE_TYPE_TO_BIN(int16_t);
DEFINE_TYPE_TO_BIN(uint8_t);
