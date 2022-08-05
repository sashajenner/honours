/*
 * non-classical vbyte
 * inspired by svb
 * for uint16_t data
 * with only 2 byte lengths allowed
 * 1 and 2
 */

#include "svbyte.h"
#include "bitmap.h"
#include <stdint.h>

uint64_t svbytebound(const uint16_t *in, uint64_t nin)
{
	uint64_t nbits;
	uint64_t i;

	nbits = BYTES_TO_BITS(SIZEOF_BITMAP(nin));
	for (i = 0; i < nin; i++) {
		if (in[i] < BYTE_1_UPRBND) {
			nbits += BITS_PER_BYTE;
		} else {
			nbits += BYTES_TO_BITS(2);
		}
	}

	return nbits;
}

void svbytecode(const uint16_t *in, uint64_t nin, uint8_t *out)
{
	uint64_t i;
	uint64_t data_offset;
	uint8_t bytes;

	bitmap_zero(out, nin);
	data_offset = SIZEOF_BITMAP(nin);

	for (i = 0; i < nin; i++) {
		if (in[i] < BYTE_1_UPRBND) {
			bytes = 1;
		} else {
			set_bit(i, out);
			bytes = 2;
		}
		memcpy(out + data_offset, in + i, bytes);
		data_offset += bytes;
	}
}

void svbytedecode(const uint8_t *in, uint64_t nin, uint16_t *out)
{
	uint64_t i;
	uint64_t data_offset;
	uint8_t bytes;

	data_offset = SIZEOF_BITMAP(nin);

	for (i = 0; i < nin; ++i) {
		bytes = get_bit(i, in) + 1;
		memcpy(out + i, in + data_offset, bytes);
		data_offset += bytes;
	}
}
