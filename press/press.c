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
#include "flac-1.3.4/include/FLAC/stream_encoder.h"
#include "flac-1.3.4/include/FLAC/stream_decoder.h"
#include "TurboPFor-Integer-Compression/vp4.h"
#include "TurboPFor-Integer-Compression/bitpack.h"
#include "huffman/huffman.h"
#include "Turbo-Range-Coder/turborc.h"
#include "sigtk/src/jnn.h"

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
DEFINE_UINTX(32);

uint64_t bound_uint_submin_16(uint64_t nin);
struct meta_uint_submin_16;
int init_meta_uint_submin_16(const int16_t *in, uint32_t nin,
			     struct flat_meta *meta);
void free_meta_uint_submin_16(struct flat_meta *meta, uint32_t nin);
void fill_meta_uint_submin_16(const int16_t *in, uint32_t nin, uint32_t step,
			      struct flat_meta *meta);
uint32_t get_nbytes_uint_submin_16(uint32_t i, uint32_t j,
				   struct flat_meta *meta);
int press_uint_submin_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint32_t *nout);
int depress_uint_submin_16(const uint8_t *in, uint32_t nin, int16_t *out,
			   uint32_t *nout);

#define DEFINE_FLAT_METHOD_UINT_SUBMIN_16(name) \
struct flat_method name = { \
	bound_uint_submin_16, \
	init_meta_uint_submin_16, \
	free_meta_uint_submin_16, \
	fill_meta_uint_submin_16, \
	press_uint_submin_16, \
	depress_uint_submin_16, \
	ntobytes_uint_submin_16, \
}

struct flac_data_u8 {
	uint8_t *data;
	uint64_t cap;
	uint64_t n;
	uint64_t offset;
};

struct flac_press_data {
	struct flac_data_u8 out;
};

FLAC__StreamEncoderWriteStatus
flac_press_write_callback(const FLAC__StreamEncoder *encoder,
			  const FLAC__byte buffer[], size_t bytes,
			  uint32_t samples, uint32_t current_frame,
			  void *client_data);
FLAC__StreamEncoderSeekStatus
flac_press_seek_callback(const FLAC__StreamEncoder *encoder,
			 FLAC__uint64 absolute_byte_offset, void *client_data);
FLAC__StreamEncoderTellStatus
flac_press_tell_callback(const FLAC__StreamEncoder *encoder,
			 FLAC__uint64 *absolute_byte_offset, void *client_data);

struct flac_data_const_u8 {
	const uint8_t *data;
	uint64_t cap;
	uint64_t n;
	uint64_t offset;
};

struct flac_data_32 {
	int32_t *data;
	uint64_t cap;
	uint64_t n;
	uint64_t offset;
};

struct flac_depress_data {
	struct flac_data_const_u8 in;
	struct flac_data_32 out;
	int error;
};

FLAC__StreamDecoderReadStatus
flac_depress_read_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__byte buffer[], size_t *bytes,
			   void *client_data);
FLAC__StreamDecoderSeekStatus
flac_depress_seek_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__uint64 absolute_byte_offset,
			   void *client_data);
FLAC__StreamDecoderTellStatus
flac_depress_tell_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__uint64 *absolute_byte_offset,
			   void *client_data);
FLAC__StreamDecoderLengthStatus
flac_depress_length_callback(const FLAC__StreamDecoder *decoder,
			     FLAC__uint64 *stream_length, void *client_data);
FLAC__bool flac_depress_eof_callback(const FLAC__StreamDecoder *decoder,
				     void *client_data);
FLAC__StreamDecoderWriteStatus
flac_depress_write_callback(const FLAC__StreamDecoder *decoder,
			    const FLAC__Frame *frame,
			    const FLAC__int32 *const buffer[],
			    void *client_data);
void flac_depress_error_callback(const FLAC__StreamDecoder *decoder,
				 FLAC__StreamDecoderErrorStatus status,
				 void *client_data);

/*
#define SIZEOF_WAVE_OBJ
#define SIZEOF_CONV_OBJ
#define SIZEOF_WT_SET(nin) (SIZEOF_WAVE_OBJ + SIZEOF_CONV_OBJ + 112 * sizeof(int)
*/

#define MAX_JUMP_LEN (16)

enum JUMP_TREND { NO_JUMP, JUMP_UP, JUMP_DOWN };

static void find_jumps_16(const int16_t *in_d, uint32_t nin,
			  uint32_t *jumps_start, uint32_t *falls_start,
			  uint8_t *jumps_len, uint8_t *falls_len,
			  uint16_t *njumps, uint16_t *nfalls);

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

uint8_t uint_get_minbits_32(const uint32_t *in, uint64_t nin)
{
	uint32_t max;

	max = get_max_u32(in, nin);
	return uint_get_minbits(max);
}

uint64_t uint_bound_32(uint8_t out_bits, uint64_t nin)
{
	uint64_t nin_bytes;
	nin_bytes = nin * sizeof (uint32_t);

	return sizeof out_bits + uintx_bound_32(out_bits, nin_bytes);
}

int uint_press_32(uint8_t out_bits, const uint32_t *in, uint64_t nin,
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

	ret = uintx_press_32(out_bits, in_uintx, nin, out_uintx, &nout_uintx);

	*nout = 1 + nout_uintx;
	return ret;
}

int uint_depress_32(const uint8_t *in, uint64_t nin, uint32_t *out,
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

	ret = uintx_depress_32(in_bits, in_uintx, nin, out_uintx, &nout_uintx);

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

/* submin | uint | zlib */

uint8_t zlib_uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
					uint16_t *min)
{
	return uint_submin_get_minbits_16(in, nin, min);
}

uint64_t zlib_uint_submin_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_submin_bound_16(out_bits, nin);
	return zlib_bound(nout_uint);
}

int zlib_uint_submin_press_16(uint8_t out_bits, uint16_t min,
			      const uint16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_submin_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_submin data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_submin_press_16(out_bits, min, in, nin,
				   out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = zlib_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int zlib_uint_submin_depress_16(const uint8_t *in, uint32_t nin, uint16_t *out,
				uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zlib, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_submin_depress_16(out_zlib + sizeof nout_signals,
					     nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zlib);
	return ret;
}

/* delta | zigzag | uint | zlib */

uint8_t zlib_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				    uint16_t **in_zd)
{
	return uint_zd_get_minbits_16(in, nin, in_zd);
}

uint64_t zlib_uint_zd_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_zd_bound_16(out_bits, nin);
	return zlib_bound(nout_uint);
}

int zlib_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			  const uint16_t *in_zd, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_zd_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_zd data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_zd_press_16(out_bits, in0, nin, in_zd,
			       out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = zlib_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int zlib_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			    uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zlib, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_zd_depress_16(out_zlib + sizeof nout_signals,
					 nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zlib);
	return ret;
}

/* submin | uint | zstd */

uint8_t zstd_uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
					uint16_t *min)
{
	return uint_submin_get_minbits_16(in, nin, min);
}

uint64_t zstd_uint_submin_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_submin_bound_16(out_bits, nin);
	return zstd_bound(nout_uint);
}

int zstd_uint_submin_press_16(uint8_t out_bits, uint16_t min,
			      const uint16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_submin_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_submin data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_submin_press_16(out_bits, min, in, nin,
				   out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = zstd_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int zstd_uint_submin_depress_16(const uint8_t *in, uint32_t nin, uint16_t *out,
				uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_submin_depress_16(out_zstd + sizeof nout_signals,
					     nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zstd);
	return ret;
}

/* delta | zigzag | uint | zstd */

uint8_t zstd_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				    uint16_t **in_zd)
{
	return uint_zd_get_minbits_16(in, nin, in_zd);
}

uint64_t zstd_uint_zd_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_zd_bound_16(out_bits, nin);
	return zstd_bound(nout_uint);
}

int zstd_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			  const uint16_t *in_zd, uint8_t *out, uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_zd_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_zd data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_zd_press_16(out_bits, in0, nin, in_zd,
			       out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = zstd_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int zstd_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			    uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_zd_depress_16(out_zstd + sizeof nout_signals,
					 nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zstd);
	return ret;
}

/* delta | zigzag | uint | bzip2 */

uint8_t bzip2_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				     uint16_t **in_zd)
{
	return uint_zd_get_minbits_16(in, nin, in_zd);
}

uint64_t bzip2_uint_zd_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_zd_bound_16(out_bits, nin);
	return bzip2_bound(nout_uint);
}

int bzip2_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			   const uint16_t *in_zd, uint8_t *out,
			   uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_zd_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_zd data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_zd_press_16(out_bits, in0, nin, in_zd,
			       out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = bzip2_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int bzip2_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			     uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_bzip2;
	uint8_t *out_bzip2;

	nout_bzip2 = bzip2_bound(*nout * sizeof *out);
	out_bzip2 = malloc(nout_bzip2);

	ret = bzip2_depress(in, nin, out_bzip2, &nout_bzip2);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_bzip2, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_zd_depress_16(out_bzip2 + sizeof nout_signals,
					 nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_bzip2);
	return ret;
}

/* delta | zigzag | uint | fast_lzma2 */

uint8_t fast_lzma2_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
					  uint16_t **in_zd)
{
	return uint_zd_get_minbits_16(in, nin, in_zd);
}

uint64_t fast_lzma2_uint_zd_bound_16(uint8_t out_bits, uint64_t nin)
{
	uint64_t nout_uint;

	nout_uint = sizeof (uint32_t) + uint_zd_bound_16(out_bits, nin);
	return fast_lzma2_bound(nout_uint);
}

int fast_lzma2_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
				const uint16_t *in_zd, uint8_t *out,
				uint64_t *nout)
{
	int ret;
	uint64_t nout_uint;
	uint8_t *out_uint;

	nout_uint = sizeof nin + uint_zd_bound_16(out_bits, nin);
	out_uint = malloc(nout_uint);

	/* encode nin before uint_zd data */
	(void) memcpy(out_uint, &nin, sizeof nin);
	nout_uint -= sizeof nin;
	ret = uint_zd_press_16(out_bits, in0, nin, in_zd,
			       out_uint + sizeof nin, &nout_uint);
	nout_uint += sizeof nin;

	if (ret == 0)
		ret = fast_lzma2_press(out_uint, nout_uint, out, nout);

	free(out_uint);
	return ret;
}

int fast_lzma2_uint_zd_depress_16(const uint8_t *in, uint32_t nin,
				  int16_t *out, uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_fast_lzma2;
	uint8_t *out_fast_lzma2;

	nout_fast_lzma2 = fast_lzma2_bound(*nout * sizeof *out);
	out_fast_lzma2 = malloc(nout_fast_lzma2);

	ret = fast_lzma2_depress(in, nin, out_fast_lzma2, &nout_fast_lzma2);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_fast_lzma2, sizeof nout_signals);
		nout_64 = *nout;
		ret = uint_zd_depress_16(out_fast_lzma2 + sizeof nout_signals,
					 nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_fast_lzma2);
	return ret;
}

/* flat */

/* worst case the whole input is compressed as one */
uint64_t flat_bound_16(uint32_t nin, const struct flat_method *method)
{
	return sizeof nin + method->bound(nin);
}

int flat_press_16(const int16_t *in, uint32_t nin, uint32_t step, uint8_t *out,
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

	ret = get_flats(in, nin, step, flats, nflats, &flat_nbytes, method);
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

void fill_meta_uint_submin_16(const int16_t *in, uint32_t nin, uint32_t step,
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
	DEFINE_FLAT_METHOD_UINT_SUBMIN_16(method);
	return flat_bound_16(nin, &method);
}

int flat_uint_submin_press_16(const int16_t *in, uint32_t nin, uint32_t step,
			      uint8_t *out, uint32_t *nout, uint32_t **flats,
			      uint32_t *nflats)
{
	DEFINE_FLAT_METHOD_UINT_SUBMIN_16(method);
	return flat_press_16(in, nin, step, out, nout, &method, flats, nflats);
}

int flat_uint_submin_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
				uint32_t *nout)
{
	DEFINE_FLAT_METHOD_UINT_SUBMIN_16(method);
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

/* delta | zigzag | svb */

uint64_t svb_zd_bound_16(uint64_t nin)
{
	return svb_bound(nin);
}

void svb_zd_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
		     uint64_t *nout)
{
	uint32_t *in_zd;
	in_zd = zigdelta_16_u32(in, nin);

	svb_press(in_zd, nin, out, nout);
	free(in_zd);
}

void svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
		       uint64_t *nout)
{
	uint32_t *out_zd;
	out_zd = malloc(nin * sizeof *out_zd);

	svb_depress(in, nin, out_zd);

	unzigdelta_u32_16(out_zd, nin, out);
	free(out_zd);

	*nout = nin;
}

/* delta | zigzag | svb0124 */

uint64_t svb0124_zd_bound_16(uint64_t nin)
{
	return svb0124_bound(nin);
}

void svb0124_zd_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
			 uint64_t *nout)
{
	uint32_t *in_zd;
	in_zd = zigdelta_16_u32(in, nin);

	svb0124_press(in_zd, nin, out, nout);
	free(in_zd);
}

void svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint64_t *nout)
{
	uint32_t *out_zd;
	out_zd = malloc(nin * sizeof *out_zd);

	svb0124_depress(in, nin, out_zd);

	unzigdelta_u32_16(out_zd, nin, out);
	free(out_zd);

	*nout = nin;
}

/* delta | zigzag | svb12 */

uint64_t svb12_zd_bound(uint64_t nin)
{
	return svb12_bound(nin);
}

void svb12_zd_press(const int16_t *in, uint64_t nin, uint8_t *out,
		    uint64_t *nout)
{
	uint16_t *in_zd;
	in_zd = zigdelta_16_u16(in, nin);

	svb12_press(in_zd, nin, out, nout);
	free(in_zd);
}

void svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
		      uint64_t *nout)
{
	uint16_t *out_zd;
	out_zd = malloc(nin * sizeof *out_zd);

	svb12_depress(in, nin, out_zd);

	unzigdelta_u16_16(out_zd, nin, out);
	free(out_zd);

	*nout = nin;
}

/* delta | zigzag | svb | zlib */

uint64_t zlib_svb_zd_bound_16(uint32_t nin)
{
	return zlib_bound(sizeof nin + svb_zd_bound_16(nin));
}

int zlib_svb_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb_zd_bound_16(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb_zd_press_16(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zlib_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zlib_svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zlib, sizeof nout_signals);
		nout_64 = *nout;
		svb_zd_depress_16(out_zlib + sizeof nout_signals, nout_signals,
				  out, &nout_64);
		*nout = nout_64;
	}

	free(out_zlib);
	return ret;
}

/* delta | zigzag | svb0124 | zlib */

uint64_t zlib_svb0124_zd_bound_16(uint32_t nin)
{
	return zlib_bound(sizeof nin + svb0124_zd_bound_16(nin));
}

int zlib_svb0124_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb0124_zd_bound_16(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb0124_zd_press_16(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zlib_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zlib_svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zlib, sizeof nout_signals);
		nout_64 = *nout;
		svb0124_zd_depress_16(out_zlib + sizeof nout_signals,
				      nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zlib);
	return ret;
}

/* delta | zigzag | svb12 | zlib */

uint64_t zlib_svb12_zd_bound(uint32_t nin)
{
	return zlib_bound(sizeof nin + svb12_zd_bound(nin));
}

int zlib_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb12_zd_bound(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb12_zd_press(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zlib_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zlib_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
			  uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zlib, sizeof nout_signals);
		nout_64 = *nout;
		svb12_zd_depress(out_zlib + sizeof nout_signals, nout_signals,
				 out, &nout_64);
		*nout = nout_64;
	}

	free(out_zlib);
	return ret;
}

/* delta | zigzag | svb | zstd */

uint64_t zstd_svb_zd_bound_16(uint32_t nin)
{
	return zstd_bound(sizeof nin + svb_zd_bound_16(nin));
}

int zstd_svb_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb_zd_bound_16(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb_zd_press_16(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zstd_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zstd_svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		nout_64 = *nout;
		svb_zd_depress_16(out_zstd + sizeof nout_signals, nout_signals,
				  out, &nout_64);
		*nout = nout_64;
	}

	free(out_zstd);
	return ret;
}

/* delta | zigzag | svb0124 | zstd */

uint64_t zstd_svb0124_zd_bound_16(uint32_t nin)
{
	return zstd_bound(sizeof nin + svb0124_zd_bound_16(nin));
}

int zstd_svb0124_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb0124_zd_bound_16(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb0124_zd_press_16(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zstd_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zstd_svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		nout_64 = *nout;
		svb0124_zd_depress_16(out_zstd + sizeof nout_signals,
				      nout_signals, out, &nout_64);
		*nout = nout_64;
	}

	free(out_zstd);
	return ret;
}

/* svb(16) 1,2 bytes zstd */

uint64_t zstd_svb12_bound(uint32_t nin)
{
	return zstd_bound(sizeof nin + svb12_zd_bound(nin));
}

int zstd_svb12_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		     uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb12_zd_bound(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb12_press(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zstd_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zstd_svb12_depress(const uint8_t *in, uint64_t nin, uint16_t *out,
		       uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		svb12_depress(out_zstd + sizeof nout_signals, nout_signals,
			      out);
		*nout = nout_signals;
	}

	free(out_zstd);
	return ret;
}

/* delta | zigzag | svb12 | zstd */

uint64_t zstd_svb12_zd_bound(uint32_t nin)
{
	return zstd_bound(sizeof nin + svb12_zd_bound(nin));
}

int zstd_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb12_zd_bound(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb12_zd_press(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = zstd_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int zstd_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
			  uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_zstd, sizeof nout_signals);
		nout_64 = *nout;
		svb12_zd_depress(out_zstd + sizeof nout_signals, nout_signals,
				 out, &nout_64);
		*nout = nout_64;
	}

	free(out_zstd);
	return ret;
}

/* delta | zigzag | svb12 | bzip2 */

uint64_t bzip2_svb12_zd_bound(uint32_t nin)
{
	return bzip2_bound(sizeof nin + svb12_zd_bound(nin));
}

int bzip2_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb12_zd_bound(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb12_zd_press(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = bzip2_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int bzip2_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_bzip2;
	uint8_t *out_bzip2;

	nout_bzip2 = bzip2_bound(*nout * sizeof *out);
	out_bzip2 = malloc(nout_bzip2);

	ret = bzip2_depress(in, nin, out_bzip2, &nout_bzip2);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_bzip2, sizeof nout_signals);
		nout_64 = *nout;
		svb12_zd_depress(out_bzip2 + sizeof nout_signals, nout_signals,
				 out, &nout_64);
		*nout = nout_64;
	}

	free(out_bzip2);
	return ret;
}

/* delta | zigzag | svb12 | fast_lzma2 */

uint64_t fast_lzma2_svb12_zd_bound(uint32_t nin)
{
	return fast_lzma2_bound(sizeof nin + svb12_zd_bound(nin));
}

int fast_lzma2_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	int ret;
	uint64_t nout_svb;
	uint8_t *out_svb;

	nout_svb = sizeof nin + svb12_zd_bound(nin);
	out_svb = malloc(nout_svb);

	/* encode nin before svb_zd data */
	(void) memcpy(out_svb, &nin, sizeof nin);
	nout_svb -= sizeof nin;
	svb12_zd_press(in, nin, out_svb + sizeof nin, &nout_svb);
	nout_svb += sizeof nin;

	ret = fast_lzma2_press(out_svb, nout_svb, out, nout);

	free(out_svb);
	return ret;
}

int fast_lzma2_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
				uint32_t *nout)
{
	int ret;
	uint32_t nout_signals;
	uint64_t nout_64;
	uint64_t nout_fast_lzma2;
	uint8_t *out_fast_lzma2;

	nout_fast_lzma2 = fast_lzma2_bound(*nout * sizeof *out);
	out_fast_lzma2 = malloc(nout_fast_lzma2);

	ret = fast_lzma2_depress(in, nin, out_fast_lzma2, &nout_fast_lzma2);
	if (ret == 0) {
		(void) memcpy(&nout_signals, out_fast_lzma2, sizeof nout_signals);
		nout_64 = *nout;
		svb12_zd_depress(out_fast_lzma2 + sizeof nout_signals, nout_signals,
				 out, &nout_64);
		*nout = nout_64;
	}

	free(out_fast_lzma2);
	return ret;
}

/* flac */

uint64_t flac_bound(uint64_t nin)
{
	/* TODO properly bound */
	return MAX(256, nin * 1.5);
}

int flac_press(const int32_t *in, uint64_t nin, uint8_t *out, uint64_t *nout,
	       uint32_t bps, uint32_t sample_rate)
{
	FLAC__StreamEncoder *encoder;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__bool ok;
	FLAC__int32 *in_flac;
	int ret;
	struct flac_press_data client_data;
	uint64_t i;

	ret = 0;
	client_data.out.data = out;
	client_data.out.cap = *nout;
	client_data.out.n = 0;
	client_data.out.offset = 0;

	encoder = FLAC__stream_encoder_new();
	if (!encoder)
		return -1;

	/*ok = FLAC__stream_encoder_set_verify(encoder, true);*/
	ok = FLAC__stream_encoder_set_compression_level(encoder, PRESS_LVL_FLAC);
	ok &= FLAC__stream_encoder_set_channels(encoder, PRESS_CHANNELS_FLAC);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, nin);
	ok &= FLAC__stream_encoder_set_do_exhaustive_model_search(encoder, true);

	if (!ok)
		return -1;

	init_status =
		FLAC__stream_encoder_init_stream(encoder,
						 flac_press_write_callback,
						 flac_press_seek_callback,
						 flac_press_tell_callback,
						 NULL, &client_data);

	if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
		FLAC__stream_encoder_delete(encoder);
		return -1;
	}

	in_flac = malloc(nin * sizeof *in_flac);
	if (!in_flac) {
		FLAC__stream_encoder_delete(encoder);
		return -1;
	}

	for (i = 0; i < nin; i++) {
		in_flac[i] = (FLAC__int32) in[i];
	}

	ok = FLAC__stream_encoder_process_interleaved(encoder, in_flac, nin);
	ok &= FLAC__stream_encoder_finish(encoder);
	free(in_flac);

	if (!ok)
		ret = -1;
	else
		*nout = client_data.out.n;

	FLAC__stream_encoder_delete(encoder);
	return ret;
}

FLAC__StreamEncoderWriteStatus
flac_press_write_callback(const FLAC__StreamEncoder *encoder,
			  const FLAC__byte buffer[], size_t bytes,
			  uint32_t samples, uint32_t current_frame,
			  void *client_data)
{
	struct flac_data_u8 *out;
	struct flac_press_data *press_data;

	press_data = client_data;
	out = &(press_data->out);

	if (bytes + out->offset > out->cap)
		return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;

	(void) memcpy(out->data + out->offset, buffer, bytes);
	out->offset += bytes;

	if (out->offset > out->n)
		out->n = out->offset;

	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

FLAC__StreamEncoderSeekStatus
flac_press_seek_callback(const FLAC__StreamEncoder *encoder,
			 FLAC__uint64 absolute_byte_offset, void *client_data)
{
	struct flac_data_u8 *out;
	struct flac_press_data *press_data;

	press_data = client_data;
	out = &(press_data->out);

	if (absolute_byte_offset > out->cap || absolute_byte_offset < 0)
		return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;

	out->offset = absolute_byte_offset;

	return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
}

FLAC__StreamEncoderTellStatus
flac_press_tell_callback(const FLAC__StreamEncoder *encoder,
			 FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	struct flac_data_u8 *out;
	struct flac_press_data *press_data;

	press_data = client_data;
	out = &(press_data->out);

	*absolute_byte_offset = out->offset;

	return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
}

int flac_depress(const uint8_t *in, uint64_t nin, int32_t *out,
		 uint64_t *nout)
{
	FLAC__StreamDecoder *decoder;
	FLAC__StreamDecoderInitStatus init_status;
	FLAC__bool ok;
	int ret;
	struct flac_depress_data client_data;

	ret = 0;

	client_data.in.data = in;
	client_data.in.cap = nin;
	client_data.in.n = nin;
	client_data.in.offset = 0;

	client_data.out.data = out;
	client_data.out.cap = *nout;
	client_data.out.n = 0;
	client_data.out.offset = 0;

	client_data.error = 0;

	decoder = FLAC__stream_decoder_new();
	if (!decoder)
		return -1;

	init_status =
		FLAC__stream_decoder_init_stream(decoder,
						 flac_depress_read_callback,
						 flac_depress_seek_callback,
						 flac_depress_tell_callback,
						 flac_depress_length_callback,
						 flac_depress_eof_callback,
						 flac_depress_write_callback,
						 NULL,
						 flac_depress_error_callback,
						 &client_data);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		FLAC__stream_decoder_delete(decoder);
		return -1;
	}

	ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
	if (!ok || client_data.error)
		ret = -1;
	else
		*nout = client_data.out.n;

	FLAC__stream_decoder_delete(decoder);
	return ret;
}


FLAC__StreamDecoderReadStatus
flac_depress_read_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__byte buffer[], size_t *bytes,
			   void *client_data)
{
	struct flac_data_const_u8 *in;
	struct flac_depress_data *depress_data;
	uint64_t bytes_to_cp;

	depress_data = client_data;
	in = &(depress_data->in);

	bytes_to_cp = MIN(*bytes, in->n - in->offset);

	(void) memcpy(buffer, in->data + in->offset, bytes_to_cp);

	in->offset += bytes_to_cp;
	*bytes = bytes_to_cp;

	if (bytes_to_cp)
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	else
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
}


FLAC__StreamDecoderSeekStatus
flac_depress_seek_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__uint64 absolute_byte_offset,
			   void *client_data)
{
	struct flac_data_const_u8 *in;
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	in = &(depress_data->in);

	if (absolute_byte_offset > in->n || absolute_byte_offset < 0)
		return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;

	in->offset = absolute_byte_offset;

	return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus
flac_depress_tell_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__uint64 *absolute_byte_offset,
			   void *client_data)
{
	struct flac_data_const_u8 *in;
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	in = &(depress_data->in);

	*absolute_byte_offset = in->offset;

	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}


FLAC__StreamDecoderLengthStatus
flac_depress_length_callback(const FLAC__StreamDecoder *decoder,
			     FLAC__uint64 *stream_length, void *client_data)
{
	struct flac_data_const_u8 *in;
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	in = &(depress_data->in);

	*stream_length = in->n;

	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool flac_depress_eof_callback(const FLAC__StreamDecoder *decoder,
				     void *client_data)
{
	struct flac_data_const_u8 *in;
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	in = &(depress_data->in);

	return in->offset == in->n;
}

FLAC__StreamDecoderWriteStatus
flac_depress_write_callback(const FLAC__StreamDecoder *decoder,
			    const FLAC__Frame *frame,
			    const FLAC__int32 *const buffer[],
			    void *client_data)
{
	struct flac_data_32 *out;
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	out = &(depress_data->out);

	(void) memcpy(out->data + out->offset, buffer[0],
		      frame->header.blocksize * sizeof *(out->data));
	out->offset += frame->header.blocksize;

	if (out->offset > out->n)
		out->n = out->offset;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_depress_error_callback(const FLAC__StreamDecoder *decoder,
				 FLAC__StreamDecoderErrorStatus status,
				 void *client_data)
{
	struct flac_depress_data *depress_data;

	depress_data = client_data;
	depress_data->error = 1;
}

/* flac | zstd */

uint64_t zstd_flac_bound(uint64_t nin)
{
	return zstd_bound(flac_bound(nin));
}

int zstd_flac_press(const int32_t *in, uint64_t nin, uint8_t *out,
		    uint64_t *nout, uint32_t bps, uint32_t sample_rate)
{
	int ret;
	uint64_t nout_flac;
	uint8_t *out_flac;

	nout_flac = flac_bound(nin);
	out_flac = malloc(nout_flac);

	ret = flac_press(in, nin, out_flac, &nout_flac, bps, sample_rate);
	if (ret == 0)
		ret = zstd_press(out_flac, nout_flac, out, nout);

	free(out_flac);
	return ret;
}

int zstd_flac_depress(const uint8_t *in, uint64_t nin, int32_t *out,
		      uint64_t *nout)
{
	int ret;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = MAX(256, zstd_bound(*nout * sizeof *out));
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0)
		ret = flac_depress(out_zstd, nout_zstd, out, nout);

	free(out_zstd);
	return ret;
}

/* turbopfor */

uint64_t turbopfor_bound_16(uint64_t nin)
{
	/* hacky */
	return MAX(nin, 128);
}

void turbopfor_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
			uint64_t *nout)
{
	int16_t *in_delta;

	in_delta = delta_16(in, nin);
	*nout = p4nzenc128v16((uint16_t *) in_delta, nin, out);

	free(in_delta);

	/*
	uint16_t *in_zd;

	in_zd = zigdelta_16_u16(in, nin);
	*nout = p4nenc128v16(in_zd, nin, out);

	free(in_zd);
	*/
}

void turbopfor_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			  uint64_t nout)
{
	(void) p4nzdec128v16(in, nin, (uint16_t *) out);
	undelta_inplace_16(out, nout);

	/*
	(void) p4ndec128v16(in, nin, (uint16_t *) out);
	unzigdelta_inplace_16(out, nout);
	*/
}

/*
 * variable byte 1 except 2
 * [num exceptions][exception indices]data
 * maximum 256 exceptions
 */

uint64_t vb1e2_bound(uint32_t nin)
{
	return sizeof (uint8_t) + VB1E2_MAX_EXCEPTIONS * (sizeof (uint32_t) + 2) + nin - VB1E2_MAX_EXCEPTIONS;
}

void vb1e2_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout)
{
	uint16_t nex;
	uint32_t *ex_pos;
	uint32_t i;
	uint32_t j;
	uint64_t offset;
	uint8_t nbytes;

	nex = 0;
	ex_pos = malloc(VB1E2_MAX_EXCEPTIONS * sizeof *ex_pos);

	for (i = 0; i < nin; i++) {
		if (in[i] > UINT8_MAX) {
			ex_pos[nex++] = i;
			if (nex == 0)
				fprintf(stderr, "error: vb1e2 too many exceptions\n");
		}
	}

	(void) memcpy(out, &nex, sizeof nex);
	offset = sizeof nex;
	(void) memcpy(out + offset, ex_pos, nex * sizeof *ex_pos);
	offset += nex * sizeof *ex_pos;

	j = 0;
	for (i = 0; i < nin; i++) {
		if (j < nex && i == ex_pos[j]) {
			nbytes = 2;
			j++;
		} else {
			nbytes = 1;
		}

		(void) memcpy(out + offset, in + i, nbytes);
		offset += nbytes;
	}

	free(ex_pos);
	*nout = offset;
}

void vb1e2_depress(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout)
{
	uint16_t nex;
	uint32_t *ex_pos;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	(void) memcpy(&nex, in, sizeof nex);
	offset = sizeof nex;
	ex_pos = malloc(nex * sizeof *ex_pos);
	(void) memcpy(ex_pos, in + offset, nex * sizeof *ex_pos);
	offset += nex * sizeof *ex_pos;

	i = 0;
	j = 0;
	while (offset < nin) {
		if (j < nex && i == ex_pos[j]) {
			(void) memcpy(out + i, in + offset, 2);
			j++;
			offset += 2;
		} else {
			out[i] = in[offset];
			offset++;
		}

		i++;
	}

	free(ex_pos);
	*nout = i;
}

/*
 * variable byte 1 except 2 before
 * [num exceptions][exception indices][exceptions]data
 * maximum 256 exceptions
 */

uint64_t vbe21_bound(uint32_t nin)
{
	return vb1e2_bound(nin);
}

void vbe21_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout)
{
	uint16_t nex;
	uint32_t *ex_pos;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	nex = 0;
	ex_pos = malloc(VB1E2_MAX_EXCEPTIONS * sizeof *ex_pos);

	for (i = 0; i < nin; i++) {
		if (in[i] > UINT8_MAX) {
			ex_pos[nex] = i;
			nex++;
			if (nex == 0)
				fprintf(stderr, "error: vbe21 too many exceptions\n");
		}
	}

	(void) memcpy(out, &nex, sizeof nex);
	offset = sizeof nex;
	(void) memcpy(out + offset, ex_pos, nex * sizeof *ex_pos);
	offset += nex * sizeof *ex_pos;

	for (i = 0; i < nex; i++) {
		(void) memcpy(out + offset, in + ex_pos[i], 2);
		offset += 2;
	}

	j = 0;
	for (i = 0; i < nin; i++) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			(void) memcpy(out + offset, in + i, 1);
			offset++;
		}
	}

	free(ex_pos);
	*nout = offset;
}

void vbe21_depress(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout)
{
	uint16_t nex;
	uint32_t *ex_pos;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	(void) memcpy(&nex, in, sizeof nex);
	offset = sizeof nex;
	ex_pos = malloc(nex * sizeof *ex_pos);
	(void) memcpy(ex_pos, in + offset, nex * sizeof *ex_pos);
	offset += nex * sizeof *ex_pos;

	for (i = 0; i < nex; i++) {
		(void) memcpy(out + ex_pos[i], in + offset, 2);
		offset += 2;
	}

	i = 0;
	j = 0;
	while (offset < nin || j < nex) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			out[i] = in[offset];
			offset++;
		}

		i++;
	}

	free(ex_pos);
	*nout = i;
}

/*
 * variable byte 1 except 2 before bitpack
 * [num exceptions]
 * [num bytes of next block][exception indices | diff - 1 | bitpack]
 * [num bytes of next block][exceptions - 256 | bitpack]
 * data
 * maximum 256 exceptions
 */

uint64_t vbbe21_bound(uint32_t nin)
{
	return vb1e2_bound(nin);
}

void vbbe21_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		  uint64_t *nout)
{
	uint16_t nex;
	uint16_t *ex;
	uint8_t *ex_press;
	uint32_t *ex_pos;
	uint32_t *ex_pos_delta;
	uint8_t *ex_pos_press;
	uint8_t minbits;
	uint64_t nr_press_tmp;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	nex = 0;
	ex_pos = malloc(VB1E2_MAX_EXCEPTIONS * sizeof *ex_pos);
	ex = malloc(VB1E2_MAX_EXCEPTIONS * sizeof *ex);

	for (i = 0; i < nin; i++) {
		if (in[i] > UINT8_MAX) {
			ex_pos[nex] = i;
			ex[nex] = in[i] - UINT8_MAX - 1;
			nex++;
			if (nex == 0)
				fprintf(stderr, "error: vbbe21 too many exceptions\n");
		}
	}

	(void) memcpy(out, &nex, sizeof nex);
	offset = sizeof nex;

	if (nex > 1) {
		ex_pos_delta = delta_increasing_u32(ex_pos, nex);

		minbits = uint_get_minbits_32(ex_pos_delta, nex);
		nr_press_tmp = uint_bound_32(minbits, nex);
		ex_pos_press = malloc(nr_press_tmp);
		/* TODO don't ignore return */
		(void) uint_press_32(minbits, ex_pos_delta, nex, ex_pos_press,
				     &nr_press_tmp);
		free(ex_pos_delta);
		nex_pos_press = (uint16_t) nr_press_tmp;
		/*nex_pos_press = bitnd1pack32(ex_pos, nex, ex_pos_press);*/

		(void) memcpy(out + offset, &nex_pos_press, sizeof nex_pos_press);
		offset += sizeof nex_pos_press;

		if (sizeof nex_pos_press + nex_pos_press > nex * sizeof *ex_pos) {
			fprintf(stderr, "ex_pos bitpack (%ld) > standard (%ld)\n",
					sizeof nex_pos_press + nex_pos_press, nex * sizeof *ex_pos);
		}

		/*
		(void) memcpy(out + offset, ex_pos, nex * sizeof *ex_pos);
		offset += nex * sizeof *ex_pos;
		*/
		(void) memcpy(out + offset, ex_pos_press, nex_pos_press);
		free(ex_pos_press);
		offset += nex_pos_press;

		minbits = uint_get_minbits_16(ex, nex);
		nr_press_tmp = uint_bound_16(minbits, nex);
		ex_press = malloc(nr_press_tmp);
		(void) uint_press_16(minbits, ex, nex, ex_press, &nr_press_tmp);
		nex_press = (uint16_t) nr_press_tmp;

		(void) memcpy(out + offset, &nex_press, sizeof nex_press);
		offset += sizeof nex_press;

		if (sizeof nex_press + nex_press > nex * sizeof *ex) {
			fprintf(stderr, "ex bitpack (%ld) > standard (%ld)\n",
					sizeof nex_press + nex_press, nex * sizeof *ex);
		}

		(void) memcpy(out + offset, ex_press, nex_press);
		free(ex_press);
		offset += nex_press;
	} else if (nex == 1) {
		(void) memcpy(out + offset, ex_pos, nex * sizeof *ex_pos);
		offset += nex * sizeof *ex_pos;
		(void) memcpy(out + offset, ex, nex * sizeof *ex);
		offset += nex * sizeof *ex;
	}
	free(ex);

	j = 0;
	for (i = 0; i < nin; i++) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			(void) memcpy(out + offset, in + i, 1);
			offset++;
		}
	}

	free(ex_pos);
	*nout = offset;
}

void vbbe21_depress(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout)
{
	uint16_t nex;
	uint16_t *ex;
	uint32_t *ex_pos;
	uint8_t *ex_pos_press;
	uint8_t *ex_press;
	uint16_t nex_press;
	uint16_t nex_pos_press;
	uint64_t nex_pos_delta;
	uint64_t nex_cp;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	(void) memcpy(&nex, in, sizeof nex);
	offset = sizeof nex;
	ex_pos = malloc(nex * sizeof *ex_pos);

	if (nex > 0) {
		ex = malloc(nex * sizeof *ex);

		if (nex > 1) {
			/*
			(void) memcpy(ex_pos, in + offset, nex * sizeof *ex_pos);
			offset += nex * sizeof *ex_pos;
			*/
			(void) memcpy(&nex_pos_press, in + offset, sizeof nex_pos_press);
			offset += sizeof nex_pos_press;

			ex_pos_press = malloc(nex_pos_press);
			(void) memcpy(ex_pos_press, in + offset, nex_pos_press);
			offset += nex_pos_press;

			/* TODO don't ignore return */
			nex_pos_delta = nex;
			(void) uint_depress_32(ex_pos_press, nex, ex_pos, &nex_pos_delta);
			free(ex_pos_press);
			undelta_inplace_increasing_u32(ex_pos, nex);

			/*(void) bitnd1unpack32(ex_pos_press, nex_pos_press, ex_pos);*/

			(void) memcpy(&nex_press, in + offset, sizeof nex_press);
			offset += sizeof nex_press;

			ex_press = malloc(nex_press);
			(void) memcpy(ex_press, in + offset, nex_press);
			offset += nex_press;

			nex_cp = nex;
			(void) uint_depress_16(ex_press, nex, ex, &nex_cp);
			free(ex_press);
		} else if (nex == 1) {
			(void) memcpy(ex_pos, in + offset, nex * sizeof *ex_pos);
			offset += nex * sizeof *ex_pos;
			(void) memcpy(ex, in + offset, nex * sizeof *ex);
			offset += nex * sizeof *ex;
		}

		for (i = 0; i < nex; i++) {
			out[ex_pos[i]] = ex[i] + UINT8_MAX + 1;
		}
		free(ex);
	}

	i = 0;
	j = 0;
	while (offset < nin || j < nex) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			out[i] = in[offset];
			offset++;
		}

		i++;
	}

	free(ex_pos);
	*nout = i;
}

/*
 * delta | zigzag | vb1e2
 * store first signal at start before vb
 */

uint64_t vb1e2_zd_bound_16(uint32_t nin)
{
	return sizeof (uint16_t) + vb1e2_bound(nin - 1);
}

void vb1e2_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint64_t *nout)
{
	uint16_t *in_zd;
	uint64_t nout_tmp;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);
	nout_tmp = *nout - sizeof *in_zd;
	vb1e2_press(in_zd + 1, nin - 1, out + sizeof *in_zd, &nout_tmp);

	*nout = nout_tmp + sizeof *in_zd;
	free(in_zd);
}

void vb1e2_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			 uint32_t *nout)
{
	uint16_t *out_zd;
	uint32_t nout_tmp;

	out_zd = malloc(nin * sizeof *out_zd);
	(void) memcpy(out_zd, in, sizeof *out_zd);

	nout_tmp = nin - 1;
	vb1e2_depress(in + sizeof *out_zd, nin - sizeof *out_zd, out_zd + 1,
		      &nout_tmp);
	*nout = nout_tmp + 1;

	unzigdelta_u16_16(out_zd, *nout, out);
	free(out_zd);
}

/*
 * delta | zigzag | vbe21
 * store first signal at start before vb
 */

uint64_t vbe21_zd_bound_16(uint32_t nin)
{
	return sizeof (uint16_t) + vb1e2_bound(nin - 1);
}

void vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint64_t *nout)
{
	uint16_t *in_zd;
	uint64_t nout_tmp;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);
	nout_tmp = *nout - sizeof *in_zd;
	vbe21_press(in_zd + 1, nin - 1, out + sizeof *in_zd, &nout_tmp);

	*nout = nout_tmp + sizeof *in_zd;
	free(in_zd);
}

void vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			 uint32_t *nout)
{
	uint16_t *out_zd;
	uint32_t nout_tmp;

	out_zd = malloc(nin * sizeof *out_zd);
	(void) memcpy(out_zd, in, sizeof *out_zd);

	nout_tmp = nin - 1;
	vbe21_depress(in + sizeof *out_zd, nin - sizeof *out_zd, out_zd + 1,
		      &nout_tmp);
	*nout = nout_tmp + 1;

	unzigdelta_u16_16(out_zd, *nout, out);
	free(out_zd);
}

/* delta | zigzag | vb1e2 | zstd */

uint64_t zstd_vb1e2_zd_bound_16(uint32_t nin)
{
	return zstd_bound(vb1e2_zd_bound_16(nin));
}

int zstd_vb1e2_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint64_t *nout)
{
	int ret;
	uint64_t nout_vb;
	uint8_t *out_vb;

	nout_vb = vb1e2_zd_bound_16(nin);
	out_vb = malloc(nout_vb);

	vb1e2_zd_press_16(in, nin, out_vb, &nout_vb);
	ret = zstd_press(out_vb, nout_vb, out, nout);

	free(out_vb);
	return ret;
}

int zstd_vb1e2_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			     uint32_t *nout)
{
	int ret;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0)
		vb1e2_zd_depress_16(out_zstd, nout_zstd, out, nout);

	free(out_zstd);
	return ret;
}

/* delta | zigzag | vbe21 | zstd */

uint64_t zstd_vbe21_zd_bound_16(uint32_t nin)
{
	return zstd_bound(vbe21_zd_bound_16(nin));
}

int zstd_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint64_t *nout)
{
	int ret;
	uint64_t nout_vb;
	uint8_t *out_vb;

	nout_vb = vb1e2_zd_bound_16(nin);
	out_vb = malloc(nout_vb);

	vbe21_zd_press_16(in, nin, out_vb, &nout_vb);
	ret = zstd_press(out_vb, nout_vb, out, nout);

	free(out_vb);
	return ret;
}

int zstd_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			     uint32_t *nout)
{
	int ret;
	uint64_t nout_zstd;
	uint8_t *out_zstd;

	nout_zstd = zstd_bound(*nout * sizeof *out);
	out_zstd = malloc(nout_zstd);

	ret = zstd_depress(in, nin, out_zstd, &nout_zstd);
	if (ret == 0)
		vbe21_zd_depress_16(out_zstd, nout_zstd, out, nout);

	free(out_zstd);
	return ret;
}

/* delta | zigzag | vbe21 | zlib */

uint64_t zlib_vbe21_zd_bound_16(uint32_t nin)
{
	return zlib_bound(vbe21_zd_bound_16(nin));
}

int zlib_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint64_t *nout)
{
	int ret;
	uint64_t nout_vb;
	uint8_t *out_vb;

	nout_vb = vb1e2_zd_bound_16(nin);
	out_vb = malloc(nout_vb);

	vbe21_zd_press_16(in, nin, out_vb, &nout_vb);
	ret = zlib_press(out_vb, nout_vb, out, nout);

	free(out_vb);
	return ret;
}

int zlib_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			     uint32_t *nout)
{
	int ret;
	uint64_t nout_zlib;
	uint8_t *out_zlib;

	nout_zlib = zlib_bound(*nout * sizeof *out);
	out_zlib = malloc(nout_zlib);

	ret = zlib_depress(in, nin, out_zlib, &nout_zlib);
	if (ret == 0)
		vbe21_zd_depress_16(out_zlib, nout_zlib, out, nout);

	free(out_zlib);
	return ret;
}

/*
 * delta | zigzag | vbe21 | huffman
 * huffman on the 1 byte data
 */

uint64_t huffman_vbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

int huffman_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	int ret;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = 0;
	out_huffman = NULL;
	ret = huffman_encode_memory(out_vb + exlen, nout_tmp_vb - exlen,
				    &out_huffman, &nout_tmp);
	(void) memcpy(out + offset, out_huffman, nout_tmp);
	free(out_huffman);

	*nout = nout_tmp + offset;
	free(out_vb);

	return ret;
}

int huffman_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
				uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint32_t nout_tmp_vb;
	int ret;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = 0;
	out_huffman = NULL;
	ret = huffman_decode_memory(in + offset, nin - offset, &out_huffman,
				    &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_huffman, nout_tmp_vb);
	free(out_huffman);
	nout_tmp_vb += exlen;

	if (ret == 0) {
		nout_tmp = *nout - 1;
		out_zd = malloc(*nout * sizeof *out_zd);
		vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
		free(out_vb);

		(void) memcpy(out_zd, in, sizeof *out_zd);

		unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
		free(out_zd);
		*nout = nout_tmp + 1;
	}

	return ret;
}

/*
 * delta | zigzag | vbbe21 | huffman
 * huffman on the 1 byte data
 */

uint64_t huffman_vbbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

int huffman_vbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			       uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	int ret;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = 0;
	out_huffman = NULL;
	ret = huffman_encode_memory(out_vb + exlen, nout_tmp_vb - exlen,
				    &out_huffman, &nout_tmp);
	(void) memcpy(out + offset, out_huffman, nout_tmp);
	free(out_huffman);

	*nout = nout_tmp + offset;
	free(out_vb);

	return ret;
}

int huffman_vbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
				 uint32_t *nout)
{
	uint16_t nex;
	uint64_t offset;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint32_t nout_tmp_vb;
	int ret;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = 0;
	out_huffman = NULL;
	ret = huffman_decode_memory(in + offset, nin - offset, &out_huffman,
				    &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_huffman, nout_tmp_vb);
	free(out_huffman);
	nout_tmp_vb += exlen;

	if (ret == 0) {
		nout_tmp = *nout - 1;
		out_zd = malloc(*nout * sizeof *out_zd);
		vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
		free(out_vb);

		(void) memcpy(out_zd, in, sizeof *out_zd);

		unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
		free(out_zd);
		*nout = nout_tmp + 1;
	}

	return ret;
}

/*
 * delta | zigzag | vbe21 | static huffman
 * static huffman on the 1 byte data
 */

uint64_t shuffman_vbe21_zd_bound_16(uint32_t nin)
{
	return huffman_vbe21_zd_bound_16(nin);
}

int shuffman_vbe21_zd_press_16(SymbolEncoder *se, const int16_t *in,
			       uint32_t nin, uint8_t *out, uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	int ret;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = 0;
	out_huffman = NULL;
	ret = shuffman_encode_memory(se, out_vb + exlen, nout_tmp_vb - exlen,
				     &out_huffman, &nout_tmp);
	(void) memcpy(out + offset, out_huffman, nout_tmp);
	free(out_huffman);

	*nout = nout_tmp + offset;
	free(out_vb);

	return ret;
}

int shuffman_vbe21_zd_depress_16(huffman_node *root, uint8_t *in, uint64_t nin,
				 int16_t *out, uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint32_t nout_tmp_vb;
	int ret;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout;
	out_huffman = NULL;
	ret = shuffman_decode_memory(root, in + offset, nin - offset,
				     &out_huffman, &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_huffman, nout_tmp_vb);
	free(out_huffman);
	nout_tmp_vb += exlen;

	if (ret == 0) {
		nout_tmp = *nout - 1;
		out_zd = malloc(*nout * sizeof *out_zd);
		vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
		free(out_vb);

		(void) memcpy(out_zd, in, sizeof *out_zd);

		unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
		free(out_zd);
		*nout = nout_tmp + 1;
	}

	return ret;
}

/*
 * delta | zigzag | vbbe21 | static huffman
 * static huffman on the 1 byte data
 */

uint64_t shuffman_vbbe21_zd_bound_16(uint32_t nin)
{
	return huffman_vbbe21_zd_bound_16(nin);
}

int shuffman_vbbe21_zd_press_16(SymbolEncoder *se, const int16_t *in,
				uint32_t nin, uint8_t *out, uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	int ret;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = 0;
	out_huffman = NULL;
	ret = shuffman_encode_memory(se, out_vb + exlen, nout_tmp_vb - exlen,
				     &out_huffman, &nout_tmp);
	(void) memcpy(out + offset, out_huffman, nout_tmp);
	free(out_huffman);

	*nout = nout_tmp + offset;
	free(out_vb);

	return ret;
}

int shuffman_vbbe21_zd_depress_16(huffman_node *root, uint8_t *in,
				  uint64_t nin, int16_t *out, uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_huffman;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint32_t nout_tmp_vb;
	int ret;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout;
	out_huffman = NULL;
	ret = shuffman_decode_memory(root, in + offset, nin - offset,
				     &out_huffman, &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_huffman, nout_tmp_vb);
	free(out_huffman);
	nout_tmp_vb += exlen;

	if (ret == 0) {
		nout_tmp = *nout - 1;
		out_zd = malloc(*nout * sizeof *out_zd);
		vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
		free(out_vb);

		(void) memcpy(out_zd, in, sizeof *out_zd);

		unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
		free(out_zd);
		*nout = nout_tmp + 1;
	}

	return ret;
}

/*
 * rice coding
 * modified from Emil Mikulic 2002
 * https://unix4lyfe.org/rice-coding/
 */

uint64_t rice_bound(uint64_t nin)
{
	/* this is a guess worst case */
	return nin * 1.5;
}

static int rice_len(uint8_t x, uint8_t k)
{
	int m = 1 << k;
	int q = x / m;
	return q + 1 + k;
}

static uint8_t rice_find_k(const uint8_t *in, uint64_t nin)
{
	uint8_t k;
	uint64_t outsize;
	uint64_t best_size;
	uint8_t best_k;
	uint64_t i;

	best_k = 0;
	best_size = UINT64_MAX;

	/* find length */
	for (k = 0; k < 8; k++) {
		outsize = 0;
		for (i = 0; i < nin; i++)
			outsize += rice_len(in[i], k);

		if (outsize < best_size) {
			best_size = outsize;
			best_k = k;
		}
	}

	return best_k;
}

void rice_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	uint64_t i;
	int16_t j;
	uint64_t n;
	uint8_t m;
	uint8_t q;
	uint8_t k;

	k = rice_find_k(in, nin);
	m = 1 << k;
	n = 0;

	place_bit((k >> 2) & 1, n++, out);
	place_bit((k >> 1) & 1, n++, out);
	place_bit(k & 1, n++, out);

	for (i = 0; i < nin; i++) {
		q = in[i] / m;

		for (j = 0; j < q; j++) {
			set_bit(n++, out);
		}
		clear_bit(n++, out);

		for (j = k - 1; j >= 0; j--) {
			place_bit((in[i] >> j) & 1, n++, out);
		}
	}

	*nout = BITS_TO_BYTES(n);
}

/* nin is the number of elements not bytes */
void rice_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		  uint64_t *nout)
{
	uint8_t m;
	uint8_t q;
	uint64_t n;
	int x;
	uint8_t bit;
	int16_t i;
	uint64_t j;
	uint8_t k;

	j = 0;

	bit = get_bit(0, in);
	if (bit)
		bit = 1;
	k = bit << 2;

	bit = get_bit(1, in);
	if (bit)
		bit = 1;
	k = k | bit << 1;

	bit = get_bit(2, in);
	if (bit)
		bit = 1;
	k = k | bit;

	n = 3;
	m = 1 << k;

	while (j < nin) {
		q = 0;
		while (get_bit(n++, in)) {
			q++;
		}

		x = m * q;

		for (i = k - 1; i >= 0; i--) {
			bit = get_bit(n++, in);
			if (bit)
				bit = 1;
			x = x | (bit << i);
		}

		out[j++] = x;
	}

	*nout = j;
}

/*
 * delta | zigzag | vbe21 | rice
 * rice on the 1 byte data
 */

uint64_t rice_vbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rice_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rice;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rice = malloc(nout_tmp);
	rice_press(out_vb + exlen, nout_tmp_vb - exlen, out_rice, &nout_tmp);
	(void) memcpy(out + offset, out_rice, nout_tmp);
	free(out_rice);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* nin is the number of elements not bytes */
void rice_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			      uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rice;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout;
	out_rice = malloc(nout_tmp_vb);
	rice_depress(in + offset, nin - nex - 1, out_rice, &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_rice, nout_tmp_vb);
	free(out_rice);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * delta | zigzag | vbbe21 | rice
 * rice on the 1 byte data
 */

uint64_t rice_vbbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rice_vbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rice;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rice = malloc(nout_tmp);
	rice_press(out_vb + exlen, nout_tmp_vb - exlen, out_rice, &nout_tmp);
	(void) memcpy(out + offset, out_rice, nout_tmp);
	free(out_rice);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* nin is the number of elements not bytes */
void rice_vbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rice;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout;
	out_rice = malloc(nout_tmp_vb);
	rice_depress(in + offset, nin - nex - 1, out_rice, &nout_tmp_vb);
	(void) memcpy(out_vb + exlen, out_rice, nout_tmp_vb);
	free(out_rice);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * delta | zigzag | vbe21 | range
 * range on the 1 byte data
 */

uint64_t rc_vbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rc_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			  uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rc_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			    uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rcsdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * vbbe21 | range
 * range on the 1 byte data
 */

uint64_t rc_vbbe21_bound_16(uint32_t nin)
{
	return vbe21_bound(nin);
}

void rc_vbbe21_press_16(const uint16_t *in, uint32_t nin, uint8_t *out,
			uint64_t *nout)
{
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	nout_tmp_vb = *nout;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in, nin, out_vb, &nout_tmp_vb);

	memcpy(&nex, out_vb, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out, out_vb, exlen);
	offset = exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rc_vbbe21_depress_16(uint8_t *in, uint64_t nin, uint16_t *out,
			  uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex;
	out_rc = malloc(nout_tmp_vb);
	(void) rcsdec(in + offset, *nout - nex, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout;
	vbbe21_depress(out_vb, nout_tmp_vb, out, &nout_tmp);
	free(out_vb);

	*nout = nout_tmp;
}

/*
 * delta | zigzag | vbbe21 | range
 * range on the 1 byte data
 */

uint64_t rc_vbbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rc_vbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rc_vbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			     uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rcsdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * delta | zigzag | vbe21 | range context mixing
 * range context mixing on the 1 byte data
 */

uint64_t rccm_vbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rccm_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcmsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rccm_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			      uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rcmsdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * vbbe21 | range context mixing
 * range context mixing on the 1 byte data
 */

uint64_t rccm_vbbe21_bound_16(uint32_t nin)
{
	return vbe21_bound(nin);
}

void rccm_vbbe21_press_16(const uint16_t *in, uint32_t nin, uint8_t *out,
			  uint64_t *nout)
{
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	nout_tmp_vb = *nout;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in, nin, out_vb, &nout_tmp_vb);

	memcpy(&nex, out_vb, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out, out_vb, exlen);
	offset = exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcmsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rccm_vbbe21_depress_16(uint8_t *in, uint64_t nin, uint16_t *out,
			    uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex;
	out_rc = malloc(nout_tmp_vb);
	(void) rcmsdec(in + offset, *nout - nex, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout;
	vbbe21_depress(out_vb, nout_tmp_vb, out, &nout_tmp);
	free(out_vb);

	*nout = nout_tmp;
}

/*
 * delta | zigzag | vbbe21 | range context mixing
 * range context mixing on the 1 byte data
 */

uint64_t rccm_vbbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rccm_vbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rcmsenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rccm_vbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rcmsdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * delta | zigzag | vbe21 | range cdf
 * range cdf on the 1 byte data
 */

uint64_t rccdf_vbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rccdf_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rccdfenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rccdf_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout)
{
	uint32_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex + nex * (sizeof (uint32_t) + 2);
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rccdfdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * delta | zigzag | vbbe21 | range cdf
 * range cdf on the 1 byte data
 */

uint64_t rccdf_vbbe21_zd_bound_16(uint32_t nin)
{
	return vb1e2_zd_bound_16(nin);
}

void rccdf_vbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	uint16_t *in_zd;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint64_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;

	in_zd = zigdelta_16_u16(in, nin);

	(void) memcpy(out, in_zd, sizeof *in_zd);

	nout_tmp_vb = *nout - sizeof *in_zd;
	out_vb = malloc(nout_tmp_vb);
	vbbe21_press(in_zd + 1, nin - 1, out_vb, &nout_tmp_vb);
	free(in_zd);

	memcpy(&nex, out_vb, sizeof nex);
	offset = sizeof *in_zd;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, out_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, out_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, out_vb, exlen);
	offset += exlen;

	/* I know...this is just a safe upper bound */
	nout_tmp = rice_bound(nout_tmp_vb - exlen);
	out_rc = malloc(nout_tmp);
	nout_tmp = rccdfenc(out_vb + exlen, nout_tmp_vb - exlen, out_rc);
	(void) memcpy(out + offset, out_rc, nout_tmp);
	free(out_rc);

	*nout = nout_tmp + offset;
	free(out_vb);
}

/* *nout must be the exact number of original elements */
void rccdf_vbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
				uint32_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_zd;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;

	memcpy(&nex, in + sizeof *out, sizeof nex);
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof *out + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof *out + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof *out;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex - 1;
	out_rc = malloc(nout_tmp_vb);
	(void) rccdfdec(in + offset, *nout - nex - 1, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout - 1;
	out_zd = malloc(*nout * sizeof *out_zd);
	vbbe21_depress(out_vb, nout_tmp_vb, out_zd + 1, &nout_tmp);
	free(out_vb);

	(void) memcpy(out_zd, in, sizeof *out_zd);

	unzigdelta_u16_16(out_zd, nout_tmp + 1, out);
	free(out_zd);
	*nout = nout_tmp + 1;
}

/*
 * stall zigzag delta vbbe21 range coding context mixing
 * TODO can do len_stall - 1 and bitpack
 * [start_stall][len_stall][stall | submin | vbbe21 | rccm]
 * [non-stall | delta | zigzag | vbbe21 | rccm]
 */

uint64_t rccm_svbbe21_zd_bound_16(uint32_t nin)
{
	return rccm_vbbe21_zd_bound_16(nin);
}

int find_stall(const int16_t *in, uint32_t nin, uint16_t *stall_start,
	       uint16_t *stall_len)
{
	int nsegs;
	jnn_pair_t *segs;

	jnn_param_t param = JNNV1_CDNA_PARAM;
	segs = jnn_raw(in, nin, param, &nsegs);

	if (!nsegs)
		return 0;

	*stall_start = segs[0].x;
	*stall_len = segs[0].y - segs[0].x + 1;

	free(segs);

	return 1;
}

void rccm_svbbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout)
{
	uint16_t stall_start;
	uint16_t stall_len;
	uint8_t exists_stall;
	uint64_t offset;
	uint16_t *in_stall;
	int16_t *in_nonstall;
	uint16_t nin_stall_press;
	uint64_t nin_stall_press_tmp;
	uint32_t nin_nonstall_press;
	uint64_t nin_nonstall_press_tmp;

	exists_stall = find_stall(in, nin, &stall_start, &stall_len);
	if (stall_len < 1000) {
		exists_stall = 0;
		stall_len = 0;
		stall_start = 0;
	}

	(void) memcpy(out, &exists_stall, sizeof exists_stall);
	offset = sizeof exists_stall;
	if (exists_stall) {
		stall_start += 20;
		stall_len -= 40;

		(void) memcpy(out + offset, &stall_start, sizeof stall_start);
		offset += sizeof stall_start;
		(void) memcpy(out + offset, &stall_len, sizeof stall_len);
		offset += sizeof stall_len;

		in_stall = malloc(stall_len * sizeof *in_stall);
		(void) memcpy(in_stall, in + stall_start,
			      stall_len * sizeof *in_stall);

		offset += sizeof nin_stall_press;
		nin_stall_press_tmp = *nout - offset;
		rccm_vbbe21_submin_press_16(in_stall, stall_len, out + offset,
					    &nin_stall_press_tmp);
		free(in_stall);
		nin_stall_press = nin_stall_press_tmp;

		offset -= sizeof nin_stall_press;
		(void) memcpy(out + offset, &nin_stall_press,
			      sizeof nin_stall_press);
		offset += sizeof nin_stall_press + nin_stall_press;
	}

	in_nonstall = malloc((nin - stall_len) * sizeof *in_nonstall);
	(void) memcpy(in_nonstall, in, stall_start * sizeof *in);
	(void) memcpy(in_nonstall + stall_start, in + stall_start + stall_len,
		      (nin - stall_len - stall_start) * sizeof *in);

	offset += sizeof nin_nonstall_press;
	nin_nonstall_press_tmp = *nout - offset;
	rccm_vbbe21_zd_press_16(in_nonstall, nin - stall_len, out + offset,
				&nin_nonstall_press_tmp);
	free(in_nonstall);
	nin_nonstall_press = nin_nonstall_press_tmp;

	offset -= sizeof nin_nonstall_press;
	(void) memcpy(out + offset, &nin_nonstall_press,
		      sizeof nin_nonstall_press);
	offset += sizeof nin_nonstall_press + nin_nonstall_press;

	*nout = offset;
}

void rccm_svbbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
				uint32_t *nout)
{
	uint8_t exists_stall;
	uint64_t offset;
	uint16_t stall_start;
	uint16_t stall_len;
	uint8_t *stall_press;
	uint8_t *nonstall_press;
	uint16_t nin_stall_press;
	uint32_t nin_nonstall_press;
	uint16_t *stall;
	int16_t *nonstall;
	uint64_t nstall;
	uint32_t nnonstall;

	(void) memcpy(&exists_stall, in, sizeof exists_stall);
	offset = sizeof exists_stall;

	stall_start = 0;
	stall_len = 0;
	if (exists_stall) {
		(void) memcpy(&stall_start, in + offset, sizeof stall_start);
		offset += sizeof stall_start;
		(void) memcpy(&stall_len, in + offset, sizeof stall_len);
		offset += sizeof stall_len;

		(void) memcpy(&nin_stall_press, in + offset,
			      sizeof nin_stall_press);
		offset += sizeof nin_stall_press;

		stall_press = malloc(nin_stall_press);
		(void) memcpy(stall_press, in + offset, nin_stall_press);
		offset += nin_stall_press;

		nstall = stall_len;
		stall = malloc(nstall * sizeof *stall);
		rccm_vbbe21_submin_depress_16(stall_press, nin_stall_press,
					      stall, &nstall);
		free(stall_press);

		(void) memcpy(out + stall_start, stall,
			      stall_len * sizeof *stall);
		free(stall);
	}

	(void) memcpy(&nin_nonstall_press, in + offset,
		      sizeof nin_nonstall_press);
	offset += sizeof nin_nonstall_press;

	nonstall_press = calloc(nin_nonstall_press + 4, 1);
	(void) memcpy(nonstall_press, in + offset, nin_nonstall_press);
	offset += nin_nonstall_press;

	nnonstall = nin - stall_len;
	nonstall = calloc(nnonstall, sizeof nonstall);
	rccm_vbbe21_zd_depress_16(nonstall_press, nin_nonstall_press, nonstall,
				  &nnonstall);
	free(nonstall_press);

	(void) memcpy(out, nonstall, stall_start * sizeof *nonstall);
	(void) memcpy(out + stall_start + stall_len, nonstall + stall_len,
		      (nin - stall_len) * sizeof *nonstall);
	free(nonstall);
	*nout = nin;
}

/*
 * subtract min from all sigs
 * apply range coding
 * compressed: [min, sigs - min as rccm]
 */
uint64_t rccm_vbbe21_submin_bound_16(uint64_t nin)
{
	return sizeof (uint16_t) + rccm_vbbe21_bound_16(nin);
}

void rccm_vbbe21_submin_press_16(const uint16_t *in, uint64_t nin,
				 uint8_t *out, uint64_t *nout)
{
	uint16_t min;
	uint16_t *in_submin;
	uint8_t *in_vb;
	uint64_t nout_tmp;
	uint16_t nex;
	uint64_t offset;
	uint32_t exlen;
	uint16_t nex_pos_press;
	uint16_t nex_press;

	min = get_min_u16(in, nin);
	in_submin = shift_x_u16(-1 * min, in, nin);

	(void) memcpy(out, &min, sizeof min);

	nout_tmp = *nout - sizeof min;
	nout_tmp = vbbe21_bound(nout_tmp);
	in_vb = malloc(nout_tmp);
	vbbe21_press(in_submin, nin, in_vb, &nout_tmp);
	free(in_submin);

	memcpy(&nex, in_vb, sizeof nex);
	offset = sizeof min;
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in_vb + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in_vb + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	(void) memcpy(out + offset, in_vb, exlen);
	offset += exlen;

	nout_tmp -= exlen;
	nout_tmp = rcmsenc(in_vb, nout_tmp, out + offset);

	free(in_vb);
	*nout = nout_tmp + offset;
}

void rccm_vbbe21_submin_depress_16(uint8_t *in, uint64_t nin, uint16_t *out,
				   uint64_t *nout)
{
	uint16_t nex_pos_press;
	uint16_t nex_press;
	uint16_t exlen;
	uint16_t nex;
	uint64_t offset;
	uint8_t *out_vb;
	uint8_t *out_rc;
	uint16_t *out_sm;
	uint32_t nout_tmp;
	uint64_t nout_tmp_vb;
	uint16_t min;

	nex = in[sizeof min];
	exlen = sizeof nex;
	if (nex > 1) {
		(void) memcpy(&nex_pos_press, in + sizeof min + exlen, sizeof nex_pos_press);
		exlen += sizeof nex_pos_press + nex_pos_press;
		(void) memcpy(&nex_press, in + sizeof min + exlen, sizeof nex_press);
		exlen += sizeof nex_press + nex_press;
	} else if (nex == 1) {
		exlen += nex * (sizeof (uint32_t) + sizeof (uint16_t));
	}
	offset = sizeof min;

	out_vb = malloc(*nout * sizeof *out);
	(void) memcpy(out_vb, in + offset, exlen);
	offset += exlen;

	nout_tmp_vb = *nout - nex;
	out_rc = malloc(nout_tmp_vb);
	(void) rcmsdec(in + offset, *nout - nex, out_rc);
	(void) memcpy(out_vb + exlen, out_rc, nout_tmp_vb);
	free(out_rc);
	nout_tmp_vb += exlen;

	nout_tmp = *nout;
	out_sm = malloc(*nout * sizeof *out_sm);
	vbbe21_depress(out_vb, nout_tmp_vb, out_sm, &nout_tmp);

	free(out_vb);

	(void) memcpy(&min, in, sizeof min);

	shift_x_u16_u16(min, out_sm, nin, out);
	free(out_sm);

	*nout = nout_tmp;
}

/*
 * jumps
 * find jumps and falls in the signal
 * defined by sequences length n which are
 * - non-empty n >= 1
 * - strictly (in|de)creasing
 * - have at least one diff x_{i+1} - x_i > 25
 * [num jumps][num falls]
 * [[jump start indices][fall start indices] | diff - 1 | zstd_svb12]
 * [[jump lengths      ][fall lengths      ] - 1 | rccm]
 * [first sig]
 * [[jump sigs         ][- fall sigs       ] | diff - 1 | rccm_vbbe21]
 * [flat sigs | diff | zigzag | rccm]
 */

/* TODO do tighter upper bound */
uint64_t jumps_bound_16(uint32_t nin)
{
	return sizeof (uint32_t) + sizeof (uint32_t) +
		nin * sizeof (uint8_t) +
		nin * sizeof (uint8_t) +
		nin * sizeof (uint8_t);
}

static void find_jumps_16(const int16_t *in_d, uint32_t nin,
			  uint32_t *jumps_start, uint32_t *falls_start,
			  uint8_t *jumps_len, uint8_t *falls_len,
			  uint16_t *njumps, uint16_t *nfalls)
{
	uint32_t i;
	uint32_t start;
	int in_jump;
	enum JUMP_TREND trend;
	uint16_t njumps_tmp;
	uint16_t nfalls_tmp;

	start = 0;
	in_jump = 0;
	trend = NO_JUMP;
	njumps_tmp = 0;
	nfalls_tmp = 0;
	for (i = 0; i < nin; i++) {
		if (in_jump) {
			if (in_d[i] <= 0 && trend == JUMP_UP) {
				jumps_start[njumps_tmp] = start;
				jumps_len[njumps_tmp] = i - start;
				njumps_tmp++;

				in_jump = 0;
				trend = NO_JUMP;
			} else if (in_d[i] >= 0 && trend == JUMP_DOWN) {
				falls_start[nfalls_tmp] = start;
				falls_len[nfalls_tmp] = i - start;
				nfalls_tmp++;

				in_jump = 0;
				trend = NO_JUMP;
			}
		}

		if (!in_d[i]) {
			trend = NO_JUMP;
			in_jump = 0;
		} else if (in_d[i] > 0 && trend != JUMP_UP) {
			trend = JUMP_UP;
			start = i;
		} else if (in_d[i] < 0 && trend != JUMP_DOWN) {
			trend = JUMP_DOWN;
			start = i;
		}

		if (abs(in_d[i]) > 25)
			in_jump = 1;
	}

	*njumps = njumps_tmp;
	*nfalls = nfalls_tmp;
}

void jumps_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint64_t *nout)
{
	int16_t *in_d;
	int8_t *flat_d;
	uint8_t *flat_d_press;
	uint32_t nflat_d_press;
	uint32_t nflat_d;
	uint32_t iflat_d;
	int16_t *jf_d;
	uint8_t *jf_d_press;
	uint32_t njf_d;
	uint32_t njf_d_press;
	uint32_t nj_d;
	uint32_t *j_start;
	uint32_t *f_start;
	uint8_t *j_len;
	uint8_t *f_len;
	uint8_t *jf_len;
	uint8_t *jf_len_press;
	uint16_t njf_len_press;
	uint16_t nj;
	uint16_t nf;
	uint16_t *j_start_diff;
	uint16_t *f_start_diff;
	uint16_t *jf_start_diff;
	uint8_t *jf_start_diff_press;
	uint32_t njf_start_diff_press;
	uint64_t press_len_tmp;
	int16_t njf;
	uint64_t offset;
	uint64_t offset_tmp;
	uint64_t to_copy;
	uint32_t i;
	uint32_t j;
	uint32_t f;

	in_d = delta_16(in, nin);

	j_start = malloc((nin - 1) * sizeof *j_start);
	f_start = malloc((nin - 1) * sizeof *f_start);
	j_len = malloc((nin - 1) * sizeof *j_len);
	f_len = malloc((nin - 1) * sizeof *f_len);
	find_jumps_16(in_d + 1, nin - 1, j_start, f_start, j_len, f_len, &nj, &nf);

	(void) memcpy(out, &nj, sizeof nj);
	offset = sizeof nj;
	(void) memcpy(out + offset, &nf, sizeof nf);
	offset += sizeof nf;

	/* start indices */

	j_start_diff = delta_increasing_u32_u16(j_start, nj);
	f_start_diff = delta_increasing_u32_u16(f_start, nf);
	if (nj > 0) {
		(void) memcpy(out + offset, j_start_diff, sizeof *j_start_diff);
		offset += sizeof *j_start_diff;
	}
	if (nf > 0) {
		(void) memcpy(out + offset, f_start_diff, sizeof *f_start_diff);
		offset += sizeof *f_start_diff;
	}
	njf = nj + nf - 2;
	if (njf > 0) {
		jf_start_diff = malloc(njf * sizeof *jf_start_diff);

		to_copy = (nj - 1) * sizeof *j_start_diff;
		(void) memcpy(jf_start_diff, j_start_diff + 1, to_copy);
		free(j_start_diff);
		offset_tmp = to_copy;

		to_copy = (nf - 1) * sizeof *f_start_diff;
		(void) memcpy(jf_start_diff + offset_tmp / sizeof *jf_start_diff,
			      f_start_diff + 1, to_copy);
		free(f_start_diff);

		press_len_tmp = zstd_svb12_bound(njf);
		jf_start_diff_press = malloc(press_len_tmp);
		/* TODO don't encode njf before svb data */
		(void) zstd_svb12_press(jf_start_diff, njf,
					jf_start_diff_press, &press_len_tmp);
		free(jf_start_diff);

		njf_start_diff_press = press_len_tmp;
		(void) memcpy(out + offset, &njf_start_diff_press,
			      sizeof njf_start_diff_press);
		offset += sizeof njf_start_diff_press;

		(void) memcpy(out + offset, jf_start_diff_press,
			      njf_start_diff_press);
		offset += njf_start_diff_press;
		free(jf_start_diff_press);
	}

	/* lengths */

	njf = nj + nf;
	if (njf > 0) {
		jf_len = malloc(njf * sizeof *jf_len);

		to_copy = nj * sizeof *j_len;
		(void) memcpy(jf_len, j_len, to_copy);
		offset_tmp = to_copy;

		to_copy = nf * sizeof *f_len;
		(void) memcpy(jf_len + offset_tmp / sizeof *jf_len, f_len,
			      to_copy);

		shift_x_inplace_u8(-1, jf_len, njf);

		/* I know...this is just a safe upper bound */
		press_len_tmp = rice_bound(njf);
		jf_len_press = malloc(press_len_tmp);
		press_len_tmp = rcmsenc(jf_len, njf, jf_len_press);
		free(jf_len);

		njf_len_press = press_len_tmp;
		(void) memcpy(out + offset, &njf_len_press,
			      sizeof njf_len_press);
		offset += sizeof njf_len_press;

		(void) memcpy(out + offset, jf_len_press, njf_len_press);
		offset += njf_len_press;
		free(jf_len_press);
	}

	/* first sig */

	(void) memcpy(out + offset, in_d, sizeof *in_d);
	offset += sizeof *in_d;

	/* jump sigs */

	njf_d = 0;
	if (njf > 0) {
		jf_d = malloc(MAX(nin - 1, njf * MAX_JUMP_LEN));

		offset_tmp = 0;
		for (i = 0; i < nj; i++) {
			to_copy = j_len[i] * sizeof *in_d;
			njf_d += j_len[i];
			(void) memcpy(jf_d + offset_tmp / sizeof *jf_d,
				      in_d + 1 + j_start[i], to_copy);
			offset_tmp += to_copy;
		}
		nj_d = njf_d;
		for (i = 0; i < nf; i++) {
			to_copy = f_len[i] * sizeof *in_d;
			njf_d += f_len[i];
			(void) memcpy(jf_d + offset_tmp / sizeof *jf_d,
				      in_d + 1 + f_start[i], to_copy);
			offset_tmp += to_copy;
		}

		times_x_inplace_16(-1, (int16_t *) jf_d + nj_d, njf_d - nj_d);
		shift_x_inplace_16(-1, jf_d, njf_d);

		press_len_tmp = rccm_vbbe21_bound_16(njf_d);
		jf_d_press = malloc(press_len_tmp);
		rccm_vbbe21_press_16((uint16_t *) jf_d, njf_d, jf_d_press,
				   &press_len_tmp);
		free(jf_d);

		njf_d_press = press_len_tmp;
		(void) memcpy(out + offset, &njf_d_press, sizeof njf_d_press);
		offset += sizeof njf_d_press;

		(void) memcpy(out + offset, jf_d_press, njf_d_press);
		offset += njf_d_press;
		free(jf_d_press);
	}

	/* flat sigs */

	nflat_d = nin - 1 - njf_d;
	flat_d = malloc(nflat_d * sizeof *flat_d);
	iflat_d = 0;
	j = 0;
	f = 0;
	i = 1;
	while (i < nin) {
		if (j < nj && i == j_start[j] + 1) {
			i += j_len[j];
			j++;
		} else if (f < nf && i == f_start[f] + 1) {
			i += f_len[f];
			f++;
		} else {
			flat_d[iflat_d++] = in_d[i++];
		}
	}

	zigzag_inplace_8(flat_d, nflat_d);

	/* I know...this is just a safe upper bound */
	press_len_tmp = rice_bound(nflat_d);
	flat_d_press = malloc(nflat_d);
	press_len_tmp = rcmsenc((uint8_t *) flat_d, nflat_d, flat_d_press);
	free(flat_d);

	nflat_d_press = press_len_tmp;
	(void) memcpy(out + offset, &nflat_d_press,
		      sizeof nflat_d_press);
	offset += sizeof nflat_d_press;

	(void) memcpy(out + offset, flat_d_press, nflat_d_press);
	offset += nflat_d_press;
	free(flat_d_press);

	free(in_d);
	free(j_start);
	free(f_start);
	free(j_len);
	free(f_len);

	*nout = offset;
}

void jumps_depress_16(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout)
{
}
