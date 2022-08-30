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

/* uint submin */

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

	in_submin = u16_shift_x_u16(-1 * min, in, nin);

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

/* zigzag delta */

/* in_zd: malloced zigzag delta of in with nin - 1 elements */
uint8_t uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
			       uint16_t **in_zd)
{
	uint16_t max;

	*in_zd = zigdelta_16(in, nin);

	max = get_max_u16(*in_zd, nin - 1);
	return uint_get_minbits(max);
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
