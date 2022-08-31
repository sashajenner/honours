#include <string.h>
#include <stdio.h> /* TODO testing */
#include <inttypes.h> /* TODO testing */
#include <zlib.h>
#include <zstd.h>
#include <endian.h>
#include <math.h>
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

uint64_t uintx_bound(uint8_t in_bits, uint8_t out_bits, uint64_t nin);
void uintx_htobe(uint8_t in_bits, const uint8_t *h, uint8_t *be, uint64_t n);
void uintx_betoh(uint8_t out_bits, const uint8_t *be, uint8_t *h, uint64_t n);
void uintx_update(uint8_t in_bits, uint8_t out_bits, uint64_t *in_i,
		  uint64_t *out_i, uint8_t *in_bits_free,
		  uint8_t *out_bits_free, uint8_t *bits_left);

/* nin: number of elements in in
 * nout: number of bytes in out */
int uintx_press_core(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		     uint64_t nin, uint8_t *out, uint64_t *nout);

/* in_bits: must be a multiple of BITS_PER_BYTE
 * nin: number of in_bits elements in in */
int uintx_press(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		uint64_t nin, uint8_t *out, uint64_t *nout);

/* out_bits: must be multiple of BITS_PER_BYTE
 * nin: number of out_bits elements in in */
int uint0_depress(uint8_t out_bits, uint64_t nin, uint8_t *out,
		  uint64_t *nout);
int uintx_depress(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		  uint64_t nin, uint8_t *out, uint64_t *nout);

uint8_t uint_get_minbits(uint64_t max);

#define DEFINE_UINTX(bits) \
uint64_t uintx_bound_##bits(uint8_t out_bits, uint64_t nin) \
{ \
	return uintx_bound(bits, out_bits, nin); \
} \
int uintx_press_##bits(uint8_t out_bits, const uint8_t *in, uint64_t nin, \
		       uint8_t *out, uint64_t *nout) \
{ \
	return uintx_press(bits, out_bits, in, nin, out, nout); \
} \
int uintx_depress_##bits(uint8_t in_bits, const uint8_t *in, uint64_t nin, \
			 uint8_t *out, uint64_t *nout) \
{ \
	return uintx_depress(in_bits, bits, in, nin, out, nout); \
} \

DEFINE_UINTX(16);

uint64_t bound_uint_submin_16(uint64_t nin);
struct meta_uint_submin_16;
int init_meta_uint_submin_16(const int16_t *in, uint32_t nin,
			     struct flat_meta *meta);
void free_meta_uint_submin_16(struct flat_meta *meta, uint32_t nin);
void fill_meta_uint_submin_16(const int16_t *in, uint32_t nin,
			      struct flat_meta *meta);
uint32_t get_nbytes_uint_submin_16(uint32_t i, uint32_t j,
				   struct flat_meta *meta);
int press_uint_submin_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint32_t *nout);
int depress_uint_submin_16(const uint8_t *in, uint32_t nin, int16_t *out,
			   uint32_t *nout);

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

	if (!out_bits)
		return 0;

	return BITS_TO_BYTES(nin_elems * out_bits);
}

/* copy h from host endian into be as big endian */
void uintx_htobe(uint8_t in_bits, const uint8_t *h, uint8_t *be, uint64_t n)
{
	uint64_t i;

	if (in_bits <= BYTES_TO_BITS(sizeof (uint8_t))) {
		return;
	} else if (in_bits <= BYTES_TO_BITS(sizeof (uint16_t))) {
		for (i = 0; i < n / sizeof (uint16_t); i++) {
			((uint16_t *) be)[i] = htobe16(((uint16_t *) h)[i]);
		}
	} else if (in_bits <= BYTES_TO_BITS(sizeof (uint32_t))) {
		for (i = 0; i < n / sizeof (uint32_t); i++) {
			((uint32_t *) be)[i] = htobe32(((uint32_t *) h)[i]);
		}
	} else if (in_bits <= BYTES_TO_BITS(sizeof (uint64_t))) {
		for (i = 0; i < n / sizeof (uint64_t); i++) {
			((uint64_t *) be)[i] = htobe64(((uint64_t *) h)[i]);
		}
	}
}

/* copy be from big endian into h as host endian */
void uintx_betoh(uint8_t out_bits, const uint8_t *be, uint8_t *h, uint64_t n)
{
	uint64_t i;

	if (out_bits <= BYTES_TO_BITS(sizeof (uint8_t))) {
		return;
	} else if (out_bits <= BYTES_TO_BITS(sizeof (uint16_t))) {
		for (i = 0; i < n / sizeof (uint16_t); i++) {
			((uint16_t *) h)[i] = be16toh(((uint16_t *) be)[i]);
		}
	} else if (out_bits <= BYTES_TO_BITS(sizeof (uint32_t))) {
		for (i = 0; i < n / sizeof (uint32_t); i++) {
			((uint32_t *) h)[i] = be32toh(((uint32_t *) be)[i]);
		}
	} else if (out_bits <= BYTES_TO_BITS(sizeof (uint64_t))) {
		for (i = 0; i < n / sizeof (uint64_t); i++) {
			((uint64_t *) h)[i] = be64toh(((uint64_t *) be)[i]);
		}
	}
}

void uintx_update(uint8_t in_bits, uint8_t out_bits, uint64_t *in_i,
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

/* if in_bits > out_bits: in must be in big endian format */
int uintx_press_core(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		     uint64_t nin, uint8_t *out, uint64_t *nout)
{
	/*
	 * in_bits = 16
	 * out_bits = 11
	 * gap = 5
	 * in_free_bits = 8
	 * in = [00000{010, 10001011}, ...]
	 * out = [{01010001, 011}..., ...]
	 *
	 * press operations on P11:
	 * out[0] = in[0] << 5 | in[1] >> 3;
	 * out[1] = in[1] << 5 | in[2] >> 3;
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

	int dirty;
	int8_t gap;
	uint64_t i;
	uint64_t in_i;
	uint64_t out_i;
	uint8_t bits_left;
	uint8_t cur_out;
	uint8_t in_bits_free;
	uint8_t mask;
	uint8_t out_bits_free;

	/* when decompressing some of out may be skipped */
	if (in_bits < out_bits)
		(void) memset(out, 0, *nout);

	bits_left = 0;
	cur_out = 0;
	dirty = 0;
	i = 0;
	in_bits_free = BITS_PER_BYTE;
	in_i = 0;
	out_bits_free = BITS_PER_BYTE;
	out_i = 0;

	uintx_update(in_bits, out_bits, &in_i, &out_i, &in_bits_free,
		     &out_bits_free, &bits_left);

	while (i < nin) {
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
		dirty = 1;

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
			dirty = 0;
		}

		if (!bits_left) {
			uintx_update(in_bits, out_bits, &in_i, &out_i,
				     &in_bits_free, &out_bits_free,
				     &bits_left);
			i++;
		}
	}

	/* if there is still data to flush */
	if (dirty) {
		if (out_i == *nout)
			return -1;
		out[out_i++] = cur_out;
	}

	*nout = out_i;

	return 0;
}

int uintx_press(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nin_bytes;
	uint8_t *in_be;

	if (!out_bits) {
		*nout = 0;
		return 0;
	}

	nin_bytes = nin * BITS_TO_BYTES(in_bits);
	in_be = malloc(nin_bytes);
	if (!in_be)
		return -1;
	uintx_htobe(in_bits, in, in_be, nin_bytes);

	ret = uintx_press_core(in_bits, out_bits, in_be, nin, out, nout);
	free(in_be);

	return ret;
}

int uint0_depress(uint8_t out_bits, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	uint64_t nout_bytes;

	nout_bytes = nin * BITS_TO_BYTES(out_bits);
	if (*nout < nout_bytes)
		return -1;

	(void) memset(out, 0, nout_bytes);
	*nout = nout_bytes;

	return 0;
}

int uintx_depress(uint8_t in_bits, uint8_t out_bits, const uint8_t *in,
		  uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint8_t *out_be;

	if (!in_bits)
		return uint0_depress(out_bits, nin, out, nout);

	out_be = malloc(*nout);
	if (!out_be)
		return -1;

	ret = uintx_press_core(in_bits, out_bits, in, nin, out_be, nout);

	if (ret == 0)
		uintx_betoh(out_bits, out_be, out, *nout);

	free(out_be);
	return ret;
}

/* uint */

uint8_t uint_get_minbits(uint64_t max)
{
	uint8_t i;

	for (i = 0; i <= BYTES_TO_BITS(sizeof max); i++) {
		if (max < pow(2, i))
			return i;
	}

	return BYTES_TO_BITS(sizeof max);
}

uint8_t uint_get_minbits_16(const uint16_t *in, uint64_t nin)
{
	uint16_t max;

	max = get_max_u16(in, nin);
	return uint_get_minbits(max);
}

uint64_t uint_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nin_bytes;
	nin_bytes = nin * sizeof (uint16_t);

	return sizeof out_bits + uintx_bound_16(out_bits, nin_bytes);
}

int uint_press_16(uint8_t out_bits, const uint16_t *in, uint64_t nin,
		  uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nout_uintx;
	const uint8_t *in_uintx;
	uint8_t *out_uintx;

	in_uintx = (const uint8_t *) in;

	out[0] = out_bits;
	out_uintx = out + 1;
	nout_uintx = *nout - sizeof *out;

	ret = uintx_press_16(out_bits, in_uintx, nin, out_uintx, &nout_uintx);

	*nout = 1 + nout_uintx;
	return ret;
}

int uint_depress_16(const uint8_t *in, uint64_t nin, uint16_t *out,
		    uint64_t *nout)
{
	int ret;
	uint64_t nout_uintx;
	const uint8_t *in_uintx;
	uint8_t *out_uintx;
	uint8_t in_bits;

	in_bits = in[0];
	in_uintx = in + 1;

	out_uintx = (uint8_t *) out;
	nout_uintx = *nout * sizeof *out;

	ret = uintx_depress_16(in_bits, in_uintx, nin, out_uintx, &nout_uintx);

	*nout = nout_uintx / sizeof *out;
	return ret;
}

/* submin | uint */

uint8_t uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
				   uint16_t *min)
{
	uint16_t max;

	get_minmax_u16(in, nin, min, &max);
	return uint_get_minbits(max - *min);
}

uint64_t uint_submin_bound_16(uint8_t out_bits, uint64_t nin)
{
	return sizeof (uint16_t) + uint_bound_16(out_bits, nin);
}

int uint_submin_press_16(uint8_t out_bits, uint16_t min, const uint16_t *in,
			 uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint16_t *in_submin;
	uint64_t nout_tmp;

	in_submin = shift_x_u16(-1 * min, in, nin);

	(void) memcpy(out, &min, sizeof min);

	nout_tmp = *nout - sizeof min;
	ret = uint_press_16(out_bits, in_submin, nin, out + sizeof min, &nout_tmp);
	free(in_submin);

	*nout = nout_tmp + sizeof min;
	return ret;
}

int uint_submin_depress_16(const uint8_t *in, uint64_t nin, uint16_t *out,
			   uint64_t *nout)
{
	int ret;
	uint16_t min;

	(void) memcpy(&min, in, sizeof min);
	ret = uint_depress_16(in + sizeof min, nin, out, nout);

	shift_x_inplace_u16(min, out, *nout);

	return ret;
}

/* delta | zigzag | uint */

/* in_zd: malloced zigzag delta of in with nin - 1 elements */
uint8_t uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
			       uint16_t **in_zd)
{
	uint16_t max_zd;

	*in_zd = zigdelta_16(in, nin);

	max_zd = get_max_u16(*in_zd, nin - 1);
	return uint_get_minbits(max_zd);
}

uint64_t uint_zd_bound_16(uint8_t out_bits, uint64_t nin)
{
	return sizeof (int16_t) + uint_bound_16(out_bits, nin - 1);
}

int uint_zd_press_16(uint8_t out_bits, int16_t in0, uint64_t nin,
		     const uint16_t *in_zd, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nout_tmp;

	(void) memcpy(out, &in0, sizeof in0);

	nout_tmp = *nout - sizeof in0;
	ret = uint_press_16(out_bits, in_zd, nin - 1, out + sizeof in0,
			    &nout_tmp);

	*nout = nout_tmp + sizeof in0;
	return ret;
}

int uint_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
		       uint64_t *nout)
{
	int ret;
	uint64_t nout_tmp;

	(void) memcpy(out, in, sizeof *out);

	nout_tmp = *nout - 1;
	ret = uint_depress_16(in + sizeof *out, nin - 1,
			      (uint16_t *) (out + 1), &nout_tmp);

	*nout = 1 + nout_tmp;
	unzigdelta_inplace_16(out, *nout);

	return ret;
}

/* subtract mean | zigzag | uint */

uint8_t uint_zsm_get_minbits_16(const int16_t *in, uint64_t nin,
				uint16_t **in_zsm, int16_t *in_mean)
{
	uint16_t max_zsm;

	*in_mean = get_mean_16(in, nin);
	*in_zsm = (uint16_t *) shift_x_16(-1 * (*in_mean), in, nin);
	zigzag_inplace_16((int16_t *) (*in_zsm), nin);

	max_zsm = get_max_u16(*in_zsm, nin);
	return uint_get_minbits(max_zsm);
}

uint64_t uint_zsm_bound_16(uint8_t out_bits, uint64_t nin)
{
	return sizeof (int16_t) + uint_bound_16(out_bits, nin);
}

int uint_zsm_press_16(uint8_t out_bits, int16_t in_mean, uint64_t nin,
		      const uint16_t *in_zsm, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nout_tmp;

	(void) memcpy(out, &in_mean, sizeof in_mean);

	nout_tmp = *nout - sizeof in_mean;
	ret = uint_press_16(out_bits, in_zsm, nin, out + sizeof in_mean,
			    &nout_tmp);

	*nout = nout_tmp + sizeof in_mean;
	return ret;
}

int uint_zsm_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			uint64_t *nout)
{
	int ret;
	int16_t mean;

	(void) memcpy(&mean, in, sizeof mean);
	ret = uint_depress_16(in + sizeof mean, nin, (uint16_t *) out, nout);

	unzigzag_inplace_16((uint16_t *) out, *nout);
	shift_x_inplace_16(mean, out, *nout);

	return ret;
}

/* flat */

/* worst case the whole input is compressed as one */
uint64_t flat_bound_16(uint32_t nin, const struct flat_method *method)
{
	return sizeof nin + method->bound(nin);
}

int flat_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
		  uint32_t *nout, const struct flat_method *method,
		  uint32_t **flats, uint32_t *nflats)
{
	const int16_t *in_cur;
	int ret;
	uint32_t i;
	uint32_t flat_nbytes;
	uint32_t nin_cur;
	uint32_t nout_cur;
	uint32_t nout_total;

	ret = get_flats(in, nin, flats, nflats, &flat_nbytes, method);
	if (ret)
		return ret;

	nout_total = 0;
	for (i = 0; i < *nflats; i++) {
		/*fprintf(stderr, "%" PRIu32 "\n", flats[i]);*/
		in_cur = in + (*flats)[i];
		if (i < *nflats - 1)
			nin_cur = (*flats)[i + 1] - (*flats)[i];
		else
			nin_cur = nin - (*flats)[i];

		/*
		(void) memcpy(out, &nin_cur, sizeof nin_cur);
		nout_total += sizeof nin_cur;
		*/
		nout_cur = *nout - nout_total;
		ret = method->press(in_cur, nin_cur, out + nout_total,
				    &nout_cur);

		if (ret) {
			return ret;
		}

		nout_total += nout_cur;
	}

	if (flat_nbytes != nout_total)
		return -1;
	*nout = flat_nbytes;

	return 0;
}

int flat_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
		    uint32_t *nout, const struct flat_method *method)
{
	int ret;
	uint32_t nin_cur;
	uint32_t nin_total;
	uint32_t nout_cur;
	uint32_t nout_total;

	nin_total = 0;
	nout_total = 0;

	while (nin_total < nin) {

		(void) memcpy(&nin_cur, in + nin_total, sizeof nin_cur);
		nin_total += sizeof nin_cur;

		nout_cur = *nout - nout_total;
		ret = method->depress(in + nin_total, nin_cur,
				      out + nout_total, &nout_cur);
		if (ret)
			return ret;

		nin_total += method->ntobytes(in + nin_total, nin_cur);
		nout_total += nout_cur;
	}

	*nout = nout_total;
	return 0;
}

/* submin | uint | flat */

/* loose upper bound */
uint64_t bound_uint_submin_16(uint64_t nin)
{
	/* min + bits_per_sig + sigs */
	return sizeof (int16_t) + sizeof (uint8_t) + nin * sizeof (uint16_t);
}

struct meta_uint_submin_16 {
	int16_t min;
	int16_t max;
};

int init_meta_uint_submin_16(const int16_t *in, uint32_t nin,
			     struct flat_meta *meta)
{
	uint32_t i;
	uint32_t j;
	struct meta_uint_submin_16 *method_meta;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			method_meta = malloc(sizeof *method_meta);
			if (!method_meta)
				return -1;
			meta[I2(i, j)].method_meta = method_meta;
		}
	}

	return 0;
}

void free_meta_uint_submin_16(struct flat_meta *meta, uint32_t nin)
{
	uint32_t i;
	uint32_t j;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			free(meta[I2(i, j)].method_meta);
		}
	}
}

void fill_meta_uint_submin_16(const int16_t *in, uint32_t nin,
			      struct flat_meta *meta)
{
	int16_t max;
	int16_t min;
	struct flat_meta *cur;
	struct meta_uint_submin_16 *cur_method_meta;
	uint32_t i;
	uint32_t j;

	for (j = 0; j < nin; j++) {
		/*
		 * min(in[i, j]) = min(in[j], min(in[i, j - 1]))
		 * max(in[i, j]) = max(in[j], max(in[i, j - 1]))
		 */
		for (i = 0; i < j; i++) {
			cur = meta + I2(i, j - 1);
			cur_method_meta = (struct meta_uint_submin_16 *)
				cur->method_meta;

			min = MIN(in[j], cur_method_meta->min);
			max = MAX(in[j], cur_method_meta->max);

			cur = meta + I2(i, j);
			cur_method_meta = (struct meta_uint_submin_16 *)
				cur->method_meta;

			cur_method_meta->min = min;
			cur_method_meta->max = max;
			cur->nbytes = get_nbytes_uint_submin_16(i, j, meta);
		}
		/* min(in[j, j]) = max(in[j, j]) = in[j] */
		cur = meta + I2(j, j);
		cur_method_meta = (struct meta_uint_submin_16 *)
			cur->method_meta;

		cur_method_meta->min = in[j];
		cur_method_meta->max = in[j];
		cur->nbytes = get_nbytes_uint_submin_16(j, j, meta);
	}
}

uint32_t get_nbytes_uint_submin_16(uint32_t i, uint32_t j,
				   struct flat_meta *meta)
{
	struct flat_meta *cur;
	struct meta_uint_submin_16 *cur_method_meta;
	uint16_t range;
	uint8_t minbits;

	cur = meta + I2(i, j);
	cur_method_meta = (struct meta_uint_submin_16 *) cur->method_meta;

	range = cur_method_meta->max - cur_method_meta->min;
	minbits = uint_get_minbits(range);

	return sizeof (uint32_t) + uint_submin_bound_16(minbits, j - i + 1);
}

int press_uint_submin_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint32_t *nout)
{
	int ret;
	uint16_t min;
	uint64_t nout_tmp;
	uint8_t minbits;

	/* TODO use meta */
	minbits = uint_submin_get_minbits_16((const uint16_t *) in, nin, &min);

	nout_tmp = *nout - sizeof nin;
	ret = uint_submin_press_16(minbits, min, (const uint16_t *) in, nin,
				   out + sizeof nin, &nout_tmp);
	(void) memcpy(out, &nin, sizeof nin);

	*nout = nout_tmp + sizeof nin;
	return ret;
}

int depress_uint_submin_16(const uint8_t *in, uint32_t nin, int16_t *out,
			   uint32_t *nout)
{
	int ret;
	uint64_t nout_tmp;

	nout_tmp = *nout;
	ret = uint_submin_depress_16(in, nin, (uint16_t *) out, &nout_tmp);

	*nout = nout_tmp;
	return ret;
}

uint32_t ntobytes_uint_submin_16(const uint8_t *in, uint32_t n)
{
	uint8_t minbits;

	minbits = in[sizeof (int16_t)];
	return uint_submin_bound_16(minbits, n);
}

uint64_t flat_uint_submin_bound_16(uint32_t nin)
{
	struct flat_method method = {
		bound_uint_submin_16,
		init_meta_uint_submin_16,
		free_meta_uint_submin_16,
		fill_meta_uint_submin_16,
		press_uint_submin_16,
		depress_uint_submin_16,
		ntobytes_uint_submin_16,
	};

	return flat_bound_16(nin, &method);
}

int flat_uint_submin_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint32_t *nout, uint32_t **flats,
			      uint32_t *nflats)
{
	struct flat_method method = {
		bound_uint_submin_16,
		init_meta_uint_submin_16,
		free_meta_uint_submin_16,
		fill_meta_uint_submin_16,
		press_uint_submin_16,
		depress_uint_submin_16,
		ntobytes_uint_submin_16,
	};

	return flat_press_16(in, nin, out, nout, &method, flats, nflats);
}

int flat_uint_submin_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
				uint32_t *nout)
{
	struct flat_method method = {
		bound_uint_submin_16,
		init_meta_uint_submin_16,
		free_meta_uint_submin_16,
		fill_meta_uint_submin_16,
		press_uint_submin_16,
		depress_uint_submin_16,
		ntobytes_uint_submin_16,
	};

	return flat_depress_16(in, nin, out, nout, &method);
}

/* zlib */

uint64_t zlib_bound(uint64_t nin)
{
	return compressBound(nin);
}

int zlib_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;

	ret = compress2(out, nout, in, nin, PRESS_LVL_ZLIB);
	if (ret != Z_OK)
		return -1;

	return 0;
}

int zlib_depress(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;

	ret = uncompress(out, nout, in, nin);
	if (ret != Z_OK)
		return -1;

	return 0;
}

/* bzip2 */

uint64_t bzip2_bound(uint64_t nin)
{
	/* To guarantee that the compressed data will fit in its buffer,
	 * allocate an output buffer of size 1% larger than the uncompressed
	 * data, plus six hundred extra bytes.
	 */
	return 1.01 * nin + 600;
}

int bzip2_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	int ret;

	ret = BZ2_bzBuffToBuffCompress((char *) out, (unsigned int *) nout,
				       (char *) in, (unsigned int) nin,
				       PRESS_LVL_BZIP2, PRESS_VERBOSE_BZIP2,
				       PRESS_WORKFACTOR_BZIP2);
	if (ret != BZ_OK)
		return -1;

	return 0;
}

int bzip2_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		  uint64_t *nout)
{
	int ret;

	ret = BZ2_bzBuffToBuffDecompress((char *) out, (unsigned int *) nout,
					 (char *) in, (unsigned int) nin,
					 PRESS_SMALL_BZIP2,
					 PRESS_VERBOSE_BZIP2);
	if (ret != BZ_OK)
		return -1;

	return 0;
}

/* zstd */

uint64_t zstd_bound(uint64_t nin)
{
	return ZSTD_compressBound(nin);
}

int zstd_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	*nout = ZSTD_compress(out, *nout, in, nin, PRESS_LVL_ZSTD);
	if (ZSTD_isError(*nout))
		return -1;

	return 0;
}

int zstd_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		 uint64_t *nout)
{
	*nout = ZSTD_decompress(out, *nout, in, nin);
	if (ZSTD_isError(*nout))
		return -1;

	return 0;
}

/* fast lzma2 */

uint64_t fast_lzma2_bound(uint64_t nin)
{
	return FL2_compressBound(nin);
}

int fast_lzma2_press(const uint8_t *in, uint64_t nin, uint8_t *out,
		     uint64_t *nout)
{
	*nout = FL2_compressMt(out, *nout, in, nin, PRESS_LVL_FAST_LZMA2,
			       PRESS_NTHREADS_FAST_LZMA2);

	if (FL2_isError(*nout))
		return -1;

	return 0;
}

int fast_lzma2_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		       uint64_t *nout)
{
	*nout = FL2_decompressMt(out, *nout, in, nin,
				 PRESS_NTHREADS_FAST_LZMA2);

	if (FL2_isError(*nout))
		return -1;

	return 0;
}

/* classical svb 1,2,3,4 bytes */

uint64_t svb_bound(uint64_t nin)
{
	return streamvbyte_max_compressedbytes(nin);
}

void svb_press(const uint32_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	*nout = streamvbyte_encode(in, nin, out);
}

void svb_depress(const uint8_t *in, uint64_t nin, uint32_t *out)
{
	(void) streamvbyte_decode(in, out, nin);
}

/* svb 0,1,2,4 bytes */

uint64_t svb0124_bound(uint64_t nin)
{
	/*return streamvbyte_compressedbytes_0124(in, nin);*/
	return streamvbyte_max_compressedbytes(nin);
}

void svb0124_press(const uint32_t *in, uint64_t nin, uint8_t *out,
		   uint64_t *nout)
{
	*nout = streamvbyte_encode_0124(in, nin, out);
}

void svb0124_depress(const uint8_t *in, uint64_t nin, uint32_t *out)
{
	(void) streamvbyte_decode_0124(in, out, nin);
}

/* svb(16) 1,2 bytes */

uint64_t svb12_bound(uint64_t nin)
{
	return streamvbyte_max_compressedbytes_12(nin);
}

void svb12_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout)
{
	*nout = streamvbyte_encode_12(in, nin, out);
}

void svb12_depress(const uint8_t *in, uint64_t nin, uint16_t *out)
{
	(void) streamvbyte_decode_12(in, out, nin);
}
