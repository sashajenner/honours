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

void place_bit(uint8_t x, uint64_t bit, uint8_t *addr)
{
	if (x)
		set_bit(bit, addr);
	else
		clear_bit(bit, addr);
}

DEFINE_TYPE_TO_BIN(int16_t);
DEFINE_TYPE_TO_BIN(uint8_t);
