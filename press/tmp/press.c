#include <string.h>
#include "press.h"
#include "bitmap.h"

uint64_t none_bound(const int16_t *in, uint64_t nin)
{
	return nin * sizeof *in;
}

uint64_t none_press(const int16_t *in, uint64_t nin, uint8_t *out)
{
	(void) memcpy(out, in, nin * sizeof *in);
	return nin * sizeof *in;
}

uint64_t none_depress(const uint8_t *in, uint64_t nin, int16_t *out)
{
	(void) memcpy(out, in, nin);
	return DIV_ROUND_UP(nin, sizeof *out);
}

uint64_t uintx_bound(uint8_t x, const int16_t *in, uint64_t nin)
{
	uint64_t nbits;

	nbits = nin * x;
	return BITS_TO_BYTES(nbits);
}

uint64_t uintx_press(uint8_t x, const int16_t *in, uint64_t nin, uint8_t *out)
{
	/*
	 * x = 11
	 * in = [00000{010 10001011}, ...]
	 * out = [{01010001, 011}..., ...]
	 *
	 * press operations on P11:
	 * out[0] = in[0] >> 3;
	 * out[1] = in[0] << 5 | in[1] >> 6;
	 * out[2] = in[1] << 2 | in[2] >> 9;
	 * out[3] = in[2] << -1;
	 * ...
	*/

	int gap;
	int in_free_bits;
	int out_free_bits;
	uint64_t in_i;
	uint64_t out_i;
	uint8_t cur_out;

	cur_out = 0;
	in_free_bits = x;
	in_i = 0;
	out_free_bits = BITS_PER_BYTE;
	out_i = 0;

	while (in_i < nin) {
		gap = in_free_bits - out_free_bits;
		if (gap > 0) {
			cur_out |= in[in_i] >> gap;
			in_free_bits -= out_free_bits;
			out_free_bits = 0;
		} else {
			cur_out |= in[in_i] << (-1 * gap);
			out_free_bits -= in_free_bits;
			in_free_bits = 0;
		}

		if (!in_free_bits) {
			in_free_bits = x;
			in_i++;
		}

		if (!out_free_bits) {
			out[out_i++] = cur_out;
			out_free_bits = BITS_PER_BYTE;
			cur_out = 0;
		}
	}

	/* if there is still data to flush */
	if (out_free_bits != BITS_PER_BYTE)
		out[out_i++] = cur_out;

	return out_i;
}

uint64_t uintx_depress(uint8_t x, const uint8_t *in, uint64_t nin, int16_t *out)
{
	/*
	 * x = 11
	 * in = [{01010001, 011}..., ...]
	 * out = [00000{010 10001011}, ...]
	 */

	int gap;
	int in_free_bits;
	int out_free_bits;
	uint64_t in_i;
	uint64_t out_i;
	int16_t cur_out;
	int16_t mask;

	cur_out = 0;
	in_free_bits = BITS_PER_BYTE;
	in_i = 0;
	out_free_bits = x;
	out_i = 0;

	/* the last input must have enough free bits to fill the output */
	while (in_i < nin - 1 || in_free_bits >= out_free_bits) {
		gap = in_free_bits - out_free_bits;
		if (gap > 0) {
			cur_out |= in[in_i] >> gap;
			in_free_bits -= out_free_bits;
			out_free_bits = 0;
		} else {
			mask = ~(0xFF << in_free_bits);
			cur_out |= (((int16_t) in[in_i]) & mask) << (-1 * gap);
			out_free_bits -= in_free_bits;
			in_free_bits = 0;
		}

		if (!in_free_bits) {
			in_free_bits = BITS_PER_BYTE;
			in_i++;
		}

		if (!out_free_bits) {
			out[out_i++] = cur_out;
			out_free_bits = x;
			cur_out = 0;
		}
	}

	return out_i;
}
