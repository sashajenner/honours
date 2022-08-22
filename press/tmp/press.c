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

#define MAX_NBITS_PER_SIG (12)

uint32_t uint0_depress(uint32_t nin_elems, int16_t *out);
uint32_t zlib_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);
uint32_t zstd_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);

/* none */

uint32_t none_bound(const int16_t *in, uint32_t nin)
{
	return nin * sizeof *in;
}

uint32_t none_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes)
{
	(void) memcpy(out, in, nin * sizeof *in);
	return nin * sizeof *in;
}

uint32_t none_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	(void) memcpy(out, in, nin_elems * sizeof *out);
	return nin_elems;
}

/* uintx */

uint32_t uintx_bound(uint8_t x, const int16_t *in, uint32_t nin)
{
	uint64_t nbits;

	nbits = nin * x;
	return BITS_TO_BYTES(nbits);
}

uint32_t uintx_press(uint8_t x, const int16_t *in, uint32_t nin, uint8_t *out,
		     uint32_t nout_bytes)
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
	uint32_t in_i;
	uint32_t out_i;
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

uint32_t uintx_depress(uint8_t x, const uint8_t *in, uint32_t nin_elems,
		       uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	/*
	 * x = 11
	 * in = [{01010001, 011}..., ...]
	 * out = [00000{010 10001011}, ...]
	 */

	int gap;
	int in_free_bits;
	int out_free_bits;
	uint32_t in_i;
	uint32_t out_i;
	int16_t cur_out;
	int16_t mask;

	if (!x)
		return uint0_depress(nin_elems, out);

	cur_out = 0;
	in_free_bits = BITS_PER_BYTE;
	in_i = 0;
	out_free_bits = x;
	out_i = 0;

	/* the last input must have enough free bits to fill the output */
	while (out_i < nin_elems) {

		/*
		if (out_i == 2) {
			char *buf;
			fprintf(stderr, "in_i: %" PRIu32 "\n", in_i);
			fprintf(stderr, "in_free_bits: %d\n", in_free_bits);
			buf = uint8_t_to_bin(in[in_i]);
			fprintf(stderr, "in (press): %" PRIu8 "\t(%s)\n", in[in_i], buf);
			free(buf);
			fprintf(stderr, "out_i: %" PRIu32 "\n", out_i);
			fprintf(stderr, "out_free_bits: %d\n", out_free_bits);
			buf = int16_t_to_bin(cur_out);
			fprintf(stderr, "cur_out: %" PRId16 "\t(%s)\n", cur_out, buf);
			free(buf);
		}
		*/

		gap = in_free_bits - out_free_bits;
		mask = ~(0xFF << in_free_bits);
		if (gap > 0) {
			cur_out |= (((int16_t) in[in_i]) & mask) >> gap;
			in_free_bits -= out_free_bits;
			out_free_bits = 0;
		} else {
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

		/*
		if (out_i - 1 == 2) {
			char *buf;
			buf = int16_t_to_bin(out[out_i - 1]);
			fprintf(stderr, "out (dep): %" PRId16 "\t(%s)\n", out[out_i - 1], buf);
			free(buf);
		}
		*/
	}

	return out_i;
}

uint32_t uint0_depress(uint32_t nin_elems, int16_t *out)
{
	uint32_t i;

	for (i = 0; i < nin_elems; i++)
		out[i] = 0;

	return nin_elems;
}

/* uint */

uint32_t uint_bound(const int16_t *in, uint32_t nin)
{
	struct stats st;
	uint8_t x;

	get_stats(in, nin, &st);
	x = get_uint_bound(st.min, st.max);

	return sizeof x + uintx_bound(x, in, nin);
}

uint32_t uint_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes)
{
	struct stats st;
	uint32_t nout;
	uint8_t x;

	/* TODO have this as an argument */
	get_stats(in, nin, &st);
	x = get_uint_bound(st.min, st.max);
	print_stats(&st);
	printf("bits per sig: %" PRIu8 "\n", x);
	out[0] = x;

	nout = sizeof x;
	nout += uintx_press(x, in, nin, out + nout, nout_bytes);

	return nout;
}

uint32_t uint_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	uint8_t x;

	x = in[0];
	return uintx_depress(x, in + sizeof x, nin_elems, nin_bytes - sizeof x,
			     out, nout_bytes);
}

/* subtract min | uint */

uint32_t uint_submin_bound(const int16_t *in, uint32_t nin)
{
	struct stats st;
	uint8_t x;

	get_stats(in, nin, &st);
	x = get_uint_bound(0, st.max - st.min);

	return sizeof st.min + sizeof x + uintx_bound(x, in, nin);
}

uint32_t uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes)
{
	struct stats st;
	int16_t *in_submin;
	uint32_t nout;

	/* TODO have this as an argument */
	get_stats(in, nin, &st);
	in_submin = shift_x(-1 * st.min, in, nin);

	(void) memcpy(out, &st.min, sizeof st.min);

	nout = sizeof st.min;
	/* TODO pass stats as argument */
	nout += uint_press(in_submin, nin, out + nout, nout_bytes);
	free(in_submin);

	return nout;
}

uint32_t uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes)
{
	struct stats st;
	uint32_t nout;

	(void) memcpy(&st.min, in, sizeof st.min);
	nout = uint_depress(in + sizeof st.min, nin_elems,
			    nin_bytes - sizeof st.min, out, nout_bytes);
	/*fprintf(stderr, "min: %" PRId16 "\n", st.min);*/
	shift_x_inplace(st.min, out, nout);

	return nout;
}

/* delta | zigzag | uint */

uint32_t uint_zd_bound(const int16_t *in, uint32_t nin)
{
	int16_t *in_zd;
	uint32_t nin_zd;
	uint32_t nout;

	in_zd = zigdelta(in, nin, &nin_zd);

	nout = sizeof *in + uint_bound(in_zd, nin_zd);
	free(in_zd);

	return nout;
}

uint32_t uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint32_t nout_bytes)
{
	int16_t *in_zd;
	uint32_t nin_zd;
	uint32_t nout;

	in_zd = zigdelta(in, nin, &nin_zd);

	(void) memcpy(out, in, sizeof *in);
	nout = sizeof *in;
	nout += uint_press(in_zd, nin_zd, out + nout, nout_bytes);

	free(in_zd);
	return nout;
}

uint32_t uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			 uint32_t nin_bytes, int16_t *out,
			 uint32_t nout_bytes)
{
	uint32_t nout;

	(void) memcpy(out, in, sizeof *out);
	nout = 1;
	nout += uint_depress(in + sizeof *out, nin_elems - 1,
			     nin_bytes - sizeof *out, out + nout,
			     nout_bytes - nout);

	unzigdelta_inplace(out, nout);

	return nout;
}

/* subtract mean | zigzag | uint */

uint32_t uint_zsubmean_bound(const int16_t *in, uint32_t nin)
{
	int16_t *in_zsm;
	int16_t mean;
	struct stats st;
	uint32_t nout;

	get_stats(in, nin, &st);
	mean = (int16_t) st.mean;

	in_zsm = shift_x(-1 * mean, in, nin);
	zigzag_inplace(in_zsm, nin);

	nout = sizeof mean + uint_bound(in_zsm, nin);
	free(in_zsm);

	return nout;
}

uint32_t uint_zsubmean_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes)
{
	int16_t *in_zsm;
	int16_t mean;
	struct stats st;
	uint32_t nout;

	get_stats(in, nin, &st);
	mean = (int16_t) st.mean;

	in_zsm = shift_x(-1 * mean, in, nin);
	zigzag_inplace(in_zsm, nin);

	(void) memcpy(out, &mean, sizeof mean);
	nout = sizeof mean;
	nout += uint_press(in_zsm, nin, out + nout, nout_bytes);
	free(in_zsm);

	return nout;
}

uint32_t uint_zsubmean_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes)
{
	int16_t mean;
	uint32_t nout;

	(void) memcpy(&mean, in, sizeof mean);
	nout = uint_depress(in + sizeof mean, nin_elems,
			    nin_bytes - sizeof mean, out, nout_bytes);

	unzigzag_inplace(out, nout);
	shift_x_inplace(mean, out, nout);

	return nout;
}

/* optimal subsequences | subtract min | uint */

uint32_t flat_uint_submin_bound(const int16_t *in, uint32_t nin)
{
	/*
	struct stats st;
	uint64_t nbits;
	uint64_t nsigs;
	uint64_t start;
	uint8_t x;

	nbits = 0;
	start = 0;

	while (start < nin) {
		printf("%" PRIu64 ",", start);
		nsigs = end_flat(in + start, nin - start, &st);
		start += nsigs;
		x = get_uint_bound(0, st.max - st.min);
		nbits += NBITS_FLAT_UINT_HDR + nsigs * x;
	}
	puts("");
	*/

	/*
	uint64_t nbits;
	uint32_t *flats;
	uint64_t nflats;
	uint64_t i;

	nbits = get_flats(in, nin, &flats, &nflats);
	for (i = 0; i < nflats; i++) {
		printf("%" PRIu32 ",", flats[i]);
	}
	puts("");
	free(flats);
	*/

	return NBYTES_FLAT_UINT_HDR + BITS_TO_BYTES(nin * MAX_NBITS_PER_SIG);
}

uint32_t flat_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes)
{
	const int16_t *in_cur;
	uint32_t *flats;
	uint32_t i;
	uint32_t nflats;
	uint32_t nin_cur;
	uint32_t nout;
	/*uint32_t nout_total;*/
	uint64_t nbits;

	nbits = get_flats(in, nin, &flats, &nflats);

	/*nout_total = 0;*/
	for (i = 0; i < nflats; i++) {
		/*fprintf(stderr, "%" PRIu32 "\n", flats[i]);*/
		in_cur = in + flats[i];
		if (i < nflats - 1)
			nin_cur = flats[i + 1] - flats[i];
		else
			nin_cur = nin - flats[i];

		(void) memcpy(out, &nin_cur, sizeof nin_cur);
		nout = sizeof nin_cur;
		nout += uint_submin_press(in_cur, nin_cur, out + nout,
					  nout_bytes);
		out += nout;
		/*nout_total += nout;*/
	}
	free(flats);

	return BITS_TO_BYTES(nbits);
	/*return nout_total;*/
}

uint32_t flat_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes)
{
	struct stats st;
	uint32_t nin_cur;
	uint32_t nin_total;
	uint32_t nout;
	uint32_t nout_total;
	uint8_t x;

	nin_total = 0;
	nout_total = 0;

	while (nin_total < nin_elems) {

		(void) memcpy(&nin_cur, in, sizeof nin_cur);
		in += sizeof nin_cur;
		/* nin_bytes is not correct here */
		nout = uint_submin_depress(in, nin_cur, nin_bytes, out, nout_bytes);

		/* TODO nicer way of moving in to next flat? */
		in += sizeof st.min;
		x = in[0];
		if (!x)
			in += sizeof x;
		else
			in += sizeof x + BITS_TO_BYTES(nin_cur * x);

		nin_total += nin_cur;
		nout_total += nout;
		out += nout;
		nout_bytes -= nout * sizeof *out;

		/*
		if (nout_total > 56) {
			fprintf(stderr, "(out + st.min)[56]: %" PRId16 "\n", (out - nout_total)[56]);
		}
		*/
	}

	return nout_total;
}

/* zlib */

uint32_t zlib_bound(const int16_t *in, uint32_t nin)
{
	return compressBound(nin * sizeof *in);
}

uint32_t zlib_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes)
{
	return zlib_press_uint8((const uint8_t *) in, nin * sizeof *in, out,
				nout_bytes);
}

uint32_t zlib_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes)
{
	int ret;
	uint64_t nout;

	nout = nout_bytes;
	ret = compress2(out, &nout, in, nin, Z_BEST_COMPRESSION);
	switch (ret) {
		case Z_MEM_ERROR:
			fprintf(stderr, "error: zlib compress2 out of memory\n");
			break;
		case Z_BUF_ERROR:
			fprintf(stderr, "error: zlib compress2 not enough room in out\n");
			break;
		case Z_STREAM_ERROR:
			fprintf(stderr, "error: zlib compress2 invalid level param\n");
			break;
	}

	return nout;
}

uint32_t zlib_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	uint64_t nout;
	int ret;

	nout = nout_bytes;
	ret = uncompress((uint8_t *) out, &nout, in, nin_bytes);

	switch (ret) {
		case Z_MEM_ERROR:
			fprintf(stderr, "error: zlib uncompress out of memory\n");
			break;
		case Z_BUF_ERROR:
			fprintf(stderr, "error: zlib uncompress not enough room in out\n");
			break;
		case Z_DATA_ERROR:
			fprintf(stderr, "error: zlib uncompress input corrupted\n");
			break;
	}

	return nout / sizeof *out;
}

/* zstd */

uint32_t zstd_bound(const int16_t *in, uint32_t nin)
{
	return ZSTD_compressBound(nin * sizeof *in);
}

uint32_t zstd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes)
{
	return zstd_press_uint8((const uint8_t *) in, nin * sizeof *in, out,
				nout_bytes);
}

/* TODO Hint : compression runs faster if `dstCapacity` >=  `ZSTD_compressBound(srcSize)` */
uint32_t zstd_press_uint8(const uint8_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes)
{
	uint32_t nout;

	nout = ZSTD_compress(out, nout_bytes, in, nin, ZSTD_CLEVEL_DEFAULT);
	if (ZSTD_isError(nout))
		fprintf(stderr, "error: zstd compress\n");

	return nout;
}

uint32_t zstd_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	uint32_t nout;

	nout = ZSTD_decompress(out, nout_bytes, in, nin_bytes);
	if (ZSTD_isError(nout))
		fprintf(stderr, "error: zstd decompress\n");

	return nout / sizeof *out;
}

/* svb */

uint32_t svb_bound(const int16_t *in, uint32_t nin)
{
	return streamvbyte_max_compressedbytes(nin);
}

uint32_t svb_press(const int16_t *in, uint32_t nin, uint8_t *out,
		   uint32_t nout_bytes)
{
	uint32_t *in_32;
	uint32_t i;
	uint32_t nout;

	in_32 = malloc(nin * sizeof *in_32);
	for (i = 0; i < nin; i++) {
		in_32[i] = in[i];
	}

	nout = streamvbyte_encode(in_32, nin, out);
	free(in_32);

	return nout;
}

uint32_t svb_depress(const uint8_t *in, uint32_t nin_elems, uint32_t nin_bytes,
		     int16_t *out, uint32_t nout_bytes)
{
	uint32_t *out_32;
	uint32_t i;

	out_32 = malloc(nin_elems * sizeof *out_32);

	(void) streamvbyte_decode(in, out_32, nin_elems);

	for (i = 0; i < nin_elems; i++) {
		out[i] = out_32[i];
	}
	free(out_32);

	return nin_elems;
}

/* zigzag delta svb */

uint32_t svb_zd_bound(const int16_t *in, uint32_t nin)
{
	return svb_bound(in, nin);
}

uint32_t svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		      uint32_t nout_bytes)
{
	int32_t *in_32;
	uint32_t *in_zd;
	uint32_t i;
	uint32_t nout;

	in_32 = malloc(nin * sizeof *in_32);
	for (i = 0; i < nin; i++) {
		in_32[i] = in[i];
	}

	in_zd = malloc(nin * sizeof *in_zd);
	zigzag_delta_encode(in_32, in_zd, nin, 0);
	free(in_32);

	nout = streamvbyte_encode(in_zd, nin, out);
	free(in_zd);

	return nout;
}

uint32_t svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes)
{
	int32_t *out_32;
	uint32_t *out_zd;
	uint32_t i;

	out_zd = malloc(nin_elems * sizeof *out_zd);

	(void) streamvbyte_decode(in, out_zd, nin_elems);

	out_32 = malloc(nin_elems * sizeof *out_32);
	zigzag_delta_decode(out_zd, out_32, nin_elems, 0);
	free(out_zd);

	for (i = 0; i < nin_elems; i++) {
		out[i] = out_32[i];
	}
	free(out_32);

	return nin_elems;
}

/* zigzag delta svb zlib */

uint32_t zlib_svb_zd_bound(const int16_t *in, uint32_t nin)
{
	return compressBound(svb_bound(in, nin));
}

uint32_t zlib_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes)
{
	uint32_t nout;
	uint32_t nout_bytes_svb;
	uint8_t *out_svb;

	nout_bytes_svb = svb_zd_bound(in, nin);
	out_svb = malloc(nout_bytes_svb);

	nout = svb_zd_press(in, nin, out_svb, nout_bytes_svb);
	nout = zlib_press_uint8(out_svb, nout, out, nout_bytes);
	free(out_svb);

	return nout;
}

uint32_t zlib_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes)
{
	uint32_t nout;
	uint32_t nout_bytes_svb;
	uint8_t *out_svb;

	nout_bytes_svb = svb_zd_bound(NULL, nout_bytes);
	out_svb = malloc(nout_bytes_svb);

	nout_bytes_svb = zlib_depress(in, nin_elems, nin_bytes,
				      (int16_t *) out_svb, nout_bytes_svb);
	nout = svb_zd_depress(out_svb, nin_elems, nout_bytes_svb, out,
			      nout_bytes);
	free(out_svb);

	return nout;
}

/* zigzag delta svb zstd */

uint32_t zstd_svb_zd_bound(const int16_t *in, uint32_t nin)
{
	uint32_t out_bound;

	out_bound = svb_bound(in, nin);
	return zstd_bound(in, out_bound / sizeof *in);
}

uint32_t zstd_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes)
{
	uint32_t nout;
	uint32_t nout_bytes_svb;
	uint8_t *out_svb;

	nout_bytes_svb = svb_zd_bound(in, nin);
	out_svb = malloc(nout_bytes_svb);

	nout = svb_zd_press(in, nin, out_svb, nout_bytes_svb);
	nout = zstd_press_uint8(out_svb, nout, out, nout_bytes);
	free(out_svb);

	return nout;
}

uint32_t zstd_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes)
{
	uint32_t nout;
	uint32_t nout_bytes_svb;
	uint8_t *out_svb;

	nout_bytes_svb = svb_zd_bound(NULL, nout_bytes);
	out_svb = malloc(nout_bytes_svb);

	nout_bytes_svb = zstd_depress(in, nin_elems, nin_bytes,
				      (int16_t *) out_svb, nout_bytes_svb);
	nout = svb_zd_depress(out_svb, nin_elems, nout_bytes_svb, out,
			      nout_bytes);
	free(out_svb);

	return nout;
}
