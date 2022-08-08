#include <string.h>
#include <math.h>
#include <stdio.h> /* TODO testing */
#include <inttypes.h> /* TODO testing */
#include "press.h"
#include "bitmap.h"
#include "stats.h"
#include "trans.h"

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
	while ((in_i < nin - 1 || in_free_bits >= out_free_bits) && in_i < nin) {
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

uint8_t get_uint_bound(int16_t min, int16_t max)
{
	uint8_t i;

	/* can't unsigned bound if min < 0 */
	if (min < 0)
		return 0;

	for (i = 1; i <= 16; i++) {
		if (max < pow(2, i))
			return i;
	}

	return 0;
}

uint64_t uint_bound(const int16_t *in, uint64_t nin)
{
	struct stats st;
	uint8_t x;

	get_stats(in, nin, &st);
	x = get_uint_bound(st.min, st.max);

	/* +1 to store x */
	return uintx_bound(x, in, nin) + 1;
}

uint64_t uint_press(const int16_t *in, uint64_t nin, uint8_t *out)
{
	struct stats st;
	uint8_t x;

	/* TODO have this as an argument */
	get_stats(in, nin, &st);
	x = get_uint_bound(st.min, st.max);
	print_stats(&st);
	printf("bits per sig: %" PRIu8 "\n", x);

	out[0] = x;

	return uintx_press(x, in, nin, out + 1) + 1;
}

uint64_t uint_depress(const uint8_t *in, uint64_t nin, int16_t *out)
{
	uint8_t x;

	x = in[0];

	return uintx_depress(x, in + 1, nin - 1, out);
}

uint64_t uint_minusmin_bound(const int16_t *in, uint64_t nin)
{
	struct stats st;
	uint8_t x;

	get_stats(in, nin, &st);
	x = get_uint_bound(0, st.max - st.min);

	/* +1 to store x */
	return uintx_bound(x, in, nin) + sizeof st.min + 1;
}

uint64_t uint_minusmin_press(const int16_t *in, uint64_t nin, uint8_t *out)
{
	struct stats st;
	int16_t *in_minusmin;
	uint64_t i;
	uint64_t nout;

	/* TODO have this as an argument */
	get_stats(in, nin, &st);
	in_minusmin = shift_x(-1 * st.min, in, nin);

	(void) memcpy(out, &st.min, sizeof st.min);
	/* TODO pass stats as argument */
	nout = uint_press(in_minusmin, nin, out + sizeof st.min) + sizeof st.min;

	free(in_minusmin);
	return nout;
}

uint64_t uint_minusmin_depress(const uint8_t *in, uint64_t nin, int16_t *out)
{
	struct stats st;
	uint64_t i;
	uint64_t nout;

	(void) memcpy(&st.min, in, sizeof st.min);
	nout = uint_depress(in + sizeof st.min, nin - sizeof st.min, out);

	for (i = 0; i < nout; i++) {
		out[i] += st.min;
	}

	return nout;
}

uint64_t uint_zd_bound(const int16_t *in, uint64_t nin)
{
	int16_t *in_zd;
	uint64_t nin_zd;
	uint64_t nout;

	in_zd = zigdelta(in, nin, &nin_zd);

	nout = uint_bound(in_zd, nin_zd) + sizeof *in;
	free(in_zd);

	return nout;
}

uint64_t uint_zd_press(const int16_t *in, uint64_t nin, uint8_t *out)
{
	int16_t *in_zd;
	uint64_t nin_zd;
	uint64_t nout;

	in_zd = zigdelta(in, nin, &nin_zd);

	(void) memcpy(out, in, sizeof *in);
	nout = uint_press(in_zd, nin_zd, out + sizeof *in) + sizeof *in;

	free(in_zd);
	return nout;
}

uint64_t uint_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out)
{
	uint64_t i;
	uint64_t nout;

	(void) memcpy(out, in, sizeof *out);
	nout = uint_depress(in + sizeof *out, nin - sizeof *out, out + 1) + 1;

	unzigdelta_inplace(out, nout);

	return nout;
}
