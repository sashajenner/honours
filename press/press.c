#include <string.h>
#include <stdio.h> /* TODO testing */
#include <inttypes.h> /* TODO testing */
#include <zlib.h>
#include <zstd.h>
#include "press.h"
#include "bitmap.h"
#include "stats.h"
#include "trans.h"
#include "util.h"
#include "flat.h"
#include "streamvbyte/include/streamvbyte.h"
#include "streamvbyte/include/streamvbyte_zigzag.h"
#include "streamvbyte/include/streamvbytedelta.h"
#include "bzip2/bzlib.h"
#include "fast-lzma2/fast-lzma2.h"

#define MAX_NBITS_PER_SIG (12)

uint32_t uint_upper_bound(uint32_t nin);
uint32_t uint0_depress(uint32_t nin_elems, int16_t *out);
uint32_t zlib_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);
uint32_t zstd_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);

/* none */

int none_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	if (*nout < nin)
		return -1;

	(void) memcpy(out, in, nin);
	*nout = nin;

	return 0;
}

/* uintx */

uint64_t uintx_bound(uint8_t in_bits, uint8_t out_bits, uint64_t nin)
{
	uint64_t nin_elems;

	nin_elems = BYTES_TO_BITS(nin) / in_bits;
	return BITS_TO_BYTES(nin_elems * out_bits);
}

void uintx_press_update(uint8_t in_bits, uint8_t out_bits, uint64_t *in_i,
			uint64_t *out_i, uint8_t *in_bits_free,
			uint8_t *out_bits_free, uint8_t *bits_left)
{
	int8_t gap;

	gap = in_bits - out_bits;

	if (gap > 0) {
		if (gap > *in_bits_free) {
			(*in_i)++;
			gap -= *in_bits_free;
		}
		*bits_left = out_bits;
		*in_bits_free = BITS_PER_BYTE - gap % BITS_PER_BYTE;
		*in_i += gap / BITS_PER_BYTE;
	} else {
		gap *= -1;
		if (gap > *out_bits_free) {
			(*out_i)++;
			gap -= *out_bits_free;
		}
		*bits_left = in_bits;
		*out_bits_free = BITS_PER_BYTE - gap % BITS_PER_BYTE;
		*out_i += gap / BITS_PER_BYTE;
	}
}

int uintx_press(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		uint64_t nin, uint8_t *out, uint64_t *nout)
{
	/*
	 * in_bits = 16
	 * out_bits = 11
	 * gap = 5
	 * in_free_bits = 8
	 *
	 * in = [00000{010, 10001011}, ...]
	 * out = [{01010001, 011}..., ...]
	 * press operations on P11:
	 * out[0] = in[0] << 5 | in[1] >> 3;
	 * out[1] = in[1] << 5 | in[2] >> 3;
	 *
	 * in = [10001011}, 00000{010, ...]
	 * out = [10001011}, {010..., ...]
	 * ...
	 *
	 * in_bits = 14
	 * out_bits = 9
	 * gap = 5
	 * in_free_bits = 8,2
	 * in = [00000{010, 100010}00, 000 ...]
	 * out = [{01010001, 0}..., ...]
	 *
	 * reverse:
	 * in_bits = 11
	 * out_bits = 16
	 * gap = -5
	 * in = [{01010001, 011}..., ...]
	 * out = [00000{010, 10001011}, ...]
	 *
	 * press operations on compressed P11:
	 * out[0] = in[0] >> 5
	 * out[1] = in[0] << 3 | in[1] >> 5;
	 * out[2] = in[1] << 3 | in[2] >> 5;
	 * ...
	 */

	uint64_t in_i;
	uint64_t out_i;
	uint8_t bits_left;
	uint8_t cur_out;
	int8_t gap;
	uint8_t in_bits_free;
	uint8_t mask;
	uint8_t out_bits_free;

	bits_left = 0;
	cur_out = 0;
	in_bits_free = BITS_PER_BYTE;
	in_i = 0;
	out_bits_free = BITS_PER_BYTE;
	out_i = 0;

	while (in_i < nin) {
		if (!bits_left)
			uintx_press_update(in_bits, out_bits, &in_i, &out_i,
					   &in_bits_free, &out_bits_free,
					   &bits_left);

		mask = 0xFF >> (BITS_PER_BYTE - in_bits_free);
		gap = in_bits_free - out_bits_free;
		if (gap > 0) {
			cur_out |= (in[in_i] & mask) >> gap;
			in_bits_free -= out_bits_free;
			bits_left -= out_bits_free;
			out_bits_free = 0;
		} else {
			cur_out |= (in[in_i] & mask) << (-1 * gap);
			out_bits_free -= in_bits_free;
			bits_left -= in_bits_free;
			in_bits_free = 0;
		}

		if (!in_bits_free) {
			in_bits_free = BITS_PER_BYTE;
			in_i++;
		}

		if (!out_bits_free) {
			if (out_i == *nout)
				return -1;
			out[out_i++] = cur_out;
			out_bits_free = BITS_PER_BYTE;
			cur_out = 0;
		}
	}

	/* if there is still data to flush */
	if (!bits_left)
		out[out_i++] = cur_out;

	*nout = out_i;

	return 0;
}
