#ifndef PRESS_H
#define PRESS_H

#include <stdint.h>
#include <zlib.h>
#include "flat.h"

/* no compression */

/*
 * nin: number of bytes
 * return: upper bound on the number of bytes to compress array with nin bytes
 */
static inline uint64_t none_bound(uint64_t nin)
{
	return nin;
}

/*
 * nin: number of bytes in in
 * out: cannot be NULL
 * *nout: number of bytes allocated to out, set to number of bytes written to out
 * return: 0 on success, -1 on failure
 */
int none_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout);

static inline
int none_depress(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout)
{
	return none_press(in, nin, out, nout);
}

/* convert from 16 bits per element to out_bits */

uint64_t uintx_bound_16(uint8_t out_bits, uint64_t nin);
/* nin: number of uint16_t elements in in */
int uintx_press_16(uint8_t out_bits, const uint8_t *in, uint64_t nin,
		   uint8_t *out, uint64_t *nout);
/* nin: number of in_bits elements in in */
int uintx_depress_16(uint8_t in_bits, const uint8_t *in, uint64_t nin,
		     uint8_t *out, uint64_t *nout);

/*
 * store each uint16_t as uintx_t where x is decided after one pass
 * compressed: [x, sigs as uintx_t]
 */

/* nin: number of elements in in */
uint8_t uint_get_minbits_16(const uint16_t *in, uint64_t nin);
uint64_t uint_bound_16(uint8_t out_bits, uint64_t nin);
/* TODO use digitisation
 * uint64_t uint_bound_16_loose(uint64_t nin);*/
int uint_press_16(uint8_t out_bits, const uint16_t *in, uint64_t nin,
		  uint8_t *out, uint64_t *nout);
int uint_depress_16(const uint8_t *in, uint64_t nin, uint16_t *out,
		    uint64_t *nout);

/*
 * TODO store outliers separately
 * where to store?
 * - at the beginning: store indices of outliers before data
 * - in sequence: use an extra bit to represent if outlier or not, then data
 * determine outlier?
 * - sds from mean: approximate with normal distribution
 */
/*
uint32_t uint_outliers_bound(const int16_t *in, uint32_t nin);
uint32_t uint_outliers_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes);
uint32_t uint_outliers_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);
*/

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 */
uint8_t uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
				   uint16_t *min);
uint64_t uint_submin_bound_16(uint8_t out_bits, uint64_t nin);
int uint_submin_press_16(uint8_t out_bits, uint16_t min, const uint16_t *in,
			 uint64_t nin, uint8_t *out, uint64_t *nout);
int uint_submin_depress_16(const uint8_t *in, uint64_t nin, uint16_t *out,
			   uint64_t *nout);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 */

uint8_t uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
			       uint16_t **in_zd);
uint64_t uint_zd_bound_16(uint8_t out_bits, uint64_t nin);
int uint_zd_press_16(uint8_t out_bits, int16_t in0, uint64_t nin,
		     const uint16_t *in_zd, uint8_t *out, uint64_t *nout);
int uint_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
		       uint64_t *nout);

/*
 * zigzag subtract mean: subtract mean from all sigs and map to unsigned integers
 * compressed: [mean, x, zigzag(sigs - mean) as uintx_t]
 */

uint8_t uint_zsm_get_minbits_16(const int16_t *in, uint64_t nin,
				uint16_t **in_zsm, int16_t *in_mean);
uint64_t uint_zsm_bound_16(uint8_t out_bits, uint64_t nin);
int uint_zsm_press_16(uint8_t out_bits, int16_t in_mean, uint64_t nin,
		      const uint16_t *in_zsm, uint8_t *out, uint64_t *nout);
int uint_zsm_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			uint64_t *nout);

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zlib compress the result
 */

uint8_t zlib_uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
					uint16_t *min);
uint64_t zlib_uint_submin_bound_16(uint8_t out_bits, uint64_t nin);
int zlib_uint_submin_press_16(uint8_t out_bits, uint16_t min,
			      const uint16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout);
int zlib_uint_submin_depress_16(const uint8_t *in, uint32_t nin, uint16_t *out,
				uint32_t *nout);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zlib compress the result
 */

uint8_t zlib_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				    uint16_t **in_zd);
uint64_t zlib_uint_zd_bound_16(uint8_t out_bits, uint64_t nin);
int zlib_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			  const uint16_t *in_zd, uint8_t *out, uint64_t *nout);
int zlib_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			    uint32_t *nout);

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zstd compress the result
 */

uint8_t zstd_uint_submin_get_minbits_16(const uint16_t *in, uint64_t nin,
					uint16_t *min);
uint64_t zstd_uint_submin_bound_16(uint8_t out_bits, uint64_t nin);
int zstd_uint_submin_press_16(uint8_t out_bits, uint16_t min,
			      const uint16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout);
int zstd_uint_submin_depress_16(const uint8_t *in, uint32_t nin, uint16_t *out,
				uint32_t *nout);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zstd compress the result
 */

uint8_t zstd_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				    uint16_t **in_zd);
uint64_t zstd_uint_zd_bound_16(uint8_t out_bits, uint64_t nin);
int zstd_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			  const uint16_t *in_zd, uint8_t *out, uint64_t *nout);
int zstd_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			    uint32_t *nout);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * bzip2 compress the result
 */

uint8_t bzip2_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
				     uint16_t **in_zd);
uint64_t bzip2_uint_zd_bound_16(uint8_t out_bits, uint64_t nin);
int bzip2_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
			   const uint16_t *in_zd, uint8_t *out,
			   uint64_t *nout);
int bzip2_uint_zd_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
			     uint32_t *nout);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * fast_lzma2 compress the result
 */

uint8_t fast_lzma2_uint_zd_get_minbits_16(const int16_t *in, uint64_t nin,
					  uint16_t **in_zd);
uint64_t fast_lzma2_uint_zd_bound_16(uint8_t out_bits, uint64_t nin);
int fast_lzma2_uint_zd_press_16(uint8_t out_bits, int16_t in0, uint32_t nin,
				const uint16_t *in_zd, uint8_t *out,
				uint64_t *nout);
int fast_lzma2_uint_zd_depress_16(const uint8_t *in, uint32_t nin,
				  int16_t *out, uint32_t *nout);

/*
 * separate into flat regions, compress each using the same method
 * compressed: [[num_sigs, [compressed]]...]
 */

uint64_t flat_bound_16(uint32_t nin, const struct flat_method *method);
int flat_press_16(const int16_t *in, uint32_t nin, uint32_t step, uint8_t *out,
		  uint32_t *nout, const struct flat_method *method,
		  uint32_t **flats, uint32_t *nflats);
int flat_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
		    uint32_t *nout, const struct flat_method *method);

/*
 * separate into flat regions, compress using uint_submin
 * compressed: [[num_sigs, min, x, sigs - min as uintx_t]...]
 */

uint64_t flat_uint_submin_bound_16(uint32_t nin);
/* *flats: malloced array of starting indices of each flat */
int flat_uint_submin_press_16(const int16_t *in, uint32_t nin, uint32_t step,
			      uint8_t *out, uint32_t *nout, uint32_t **flats,
			      uint32_t *nflats);
int flat_uint_submin_depress_16(const uint8_t *in, uint32_t nin, int16_t *out,
				uint32_t *nout);

/*
 * TODO
 * separate into flat regions, compress using uint_zd
 * compressed: [[num_sigs, start, x, sigs zigzag delta as uintx_t]...]
 */
/*
uint32_t flat_uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t flat_uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t flat_uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			      uint32_t nin_bytes, int16_t *out,
			      uint32_t nout_bytes);
*/

/* zlib */

#define PRESS_LVL_ZLIB (Z_DEFAULT_COMPRESSION)
uint64_t zlib_bound(uint64_t nin);
int zlib_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout);
int zlib_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		 uint64_t *nout);

/* bzip2 */

#define PRESS_LVL_BZIP2 (9)
#define PRESS_VERBOSE_BZIP2 (0)
#define PRESS_WORKFACTOR_BZIP2 (30)
#define PRESS_SMALL_BZIP2 (0)
uint64_t bzip2_bound(uint64_t nin);
int bzip2_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout);
int bzip2_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		  uint64_t *nout);

/* zstd */

#define PRESS_LVL_ZSTD (1)
uint64_t zstd_bound(uint64_t nin);
int zstd_press(const uint8_t *in, uint64_t nin, uint8_t *out, uint64_t *nout);
int zstd_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		 uint64_t *nout);

/* fast lzma2 */

#define PRESS_LVL_FAST_LZMA2 (6)
#define PRESS_NTHREADS_FAST_LZMA2 (0)
uint64_t fast_lzma2_bound(uint64_t nin);
int fast_lzma2_press(const uint8_t *in, uint64_t nin, uint8_t *out,
		     uint64_t *nout);
int fast_lzma2_depress(const uint8_t *in, uint64_t nin, uint8_t *out,
		       uint64_t *nout);

/* classical svb 1,2,3,4 bytes */

/* nin: number of elements in in */
uint64_t svb_bound(uint64_t nin);
void svb_press(const uint32_t *in, uint64_t nin, uint8_t *out, uint64_t *nout);
/* nin: number of elements in the original in */
void svb_depress(const uint8_t *in, uint64_t nin, uint32_t *out);

/* svb 0,1,2,4 bytes */

/* nin: number of elements in in */
uint64_t svb0124_bound(uint64_t nin);
void svb0124_press(const uint32_t *in, uint64_t nin, uint8_t *out,
		   uint64_t *nout);
/* nin: number of elements in the original in */
void svb0124_depress(const uint8_t *in, uint64_t nin, uint32_t *out);

/* svb(16) 1,2 bytes */

uint64_t svb12_bound(uint64_t nin);
void svb12_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout);
void svb12_depress(const uint8_t *in, uint64_t nin, uint16_t *out);

/* zigzag delta classical svb */

uint64_t svb_zd_bound_16(uint64_t nin);
void svb_zd_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
		     uint64_t *nout);
void svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
		       uint64_t *nout);

/* zigzag delta svb 0,1,2,4 bytes */

uint64_t svb0124_zd_bound_16(uint64_t nin);
void svb0124_zd_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
			 uint64_t *nout);
void svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint64_t *nout);

/* zigzag delta svb(16) 1,2 bytes */

uint64_t svb12_zd_bound(uint64_t nin);
void svb12_zd_press(const int16_t *in, uint64_t nin, uint8_t *out,
		    uint64_t *nout);
void svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
		      uint64_t *nout);

/* zigzag delta classical svb zlib */

uint64_t zlib_svb_zd_bound_16(uint32_t nin);
int zlib_svb_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint64_t *nout);
int zlib_svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint32_t *nout);

/* zigzag delta svb 0,1,2,4 bytes zlib */

uint64_t zlib_svb0124_zd_bound_16(uint32_t nin);
int zlib_svb0124_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout);
int zlib_svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout);

/* zigzag delta svb(16) 1,2 bytes zlib */

uint64_t zlib_svb12_zd_bound(uint32_t nin);
int zlib_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			uint64_t *nout);
int zlib_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
			  uint32_t *nout);

/* zigzag delta classical svb zstd */

uint64_t zstd_svb_zd_bound_16(uint32_t nin);
int zstd_svb_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			 uint64_t *nout);
int zstd_svb_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			   uint32_t *nout);

/* zigzag delta svb 0,1,2,4 bytes zstd */

uint64_t zstd_svb0124_zd_bound_16(uint32_t nin);
int zstd_svb0124_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint64_t *nout);
int zstd_svb0124_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			       uint32_t *nout);

/* zigzag delta svb(16) 1,2 bytes zstd */

uint64_t zstd_svb12_zd_bound(uint32_t nin);
int zstd_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			uint64_t *nout);
int zstd_svb12_zd_depress(const uint8_t *in, uint64_t nin, int16_t *out,
			  uint32_t *nout);

/* flac */

#define PRESS_LVL_FLAC (5)
#define PRESS_CHANNELS_FLAC (1)
uint64_t flac_bound(uint64_t nin);
int flac_press(const int32_t *in, uint64_t nin, uint8_t *out, uint64_t *nout,
	       uint32_t bps, uint32_t sample_rate);
int flac_depress(const uint8_t *in, uint64_t nin, int32_t *out,
		 uint64_t *nout);

/* flac zstd */

uint64_t zstd_flac_bound(uint64_t nin);
int zstd_flac_press(const int32_t *in, uint64_t nin, uint8_t *out,
		    uint64_t *nout, uint32_t bps, uint32_t sample_rate);
int zstd_flac_depress(const uint8_t *in, uint64_t nin, int32_t *out,
		      uint64_t *nout);

/* turbopfor */

uint64_t turbopfor_bound_16(uint64_t nin);
void turbopfor_press_16(const int16_t *in, uint64_t nin, uint8_t *out,
			uint64_t *nout);
void turbopfor_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			  uint64_t nout);

/* variable byte 1 except 2 */

#define VB1E2_MAX_EXCEPTIONS (256)
uint64_t vb1e2_bound(uint32_t nin);
void vb1e2_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout);
void vb1e2_depress(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout);

/* variable byte 1 except 2 before */

uint64_t vbe21_bound(uint32_t nin);
void vbe21_press(const uint16_t *in, uint32_t nin, uint8_t *out,
		 uint64_t *nout);
void vbe21_depress(uint8_t *in, uint64_t nin, uint16_t *out, uint32_t *nout);

/* zigzag delta vb1e2 */

uint64_t vb1e2_zd_bound_16(uint32_t nin);
void vb1e2_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint64_t *nout);
void vb1e2_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			 uint32_t *nout);

/* zigzag delta vb1e2 zstd */

uint64_t zstd_vb1e2_zd_bound_16(uint32_t nin);
int zstd_vb1e2_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint64_t *nout);
int zstd_vb1e2_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
			     uint32_t *nout);

/* zigzag delta vbe21 huffman */

uint64_t huffman_vbe21_zd_bound_16(uint32_t nin);
int huffman_vbe21_zd_press_16(const int16_t *in, uint32_t nin, uint8_t *out,
			      uint64_t *nout);
int huffman_vbe21_zd_depress_16(uint8_t *in, uint64_t nin, int16_t *out,
				uint32_t *nout);

/* TODO
 * determine flats more coarsely
 * flat using other methods
 * bzip/lzma svb_zd
 * variable bit
 * bzip/lzma uint_submin?
 * other variable byte
 * 	assume 1 but store 2 bytes exceptions with indices in meta
 * huffman
 * peak-picking flat approximation
 * hasindu basecalled data method (k-mer pore model)
 * huffman/golomb-rice/arithmetic/elias-gamma on zigzag delta
 * http://neurocline.github.io/dev/2015/09/17/zig-zag-encoding.html: Note that it would be possible to store negative numbers with a smaller number of bytes with a little more sophistication, and not require zigzag encoding: you store the number of bytes required by the absolute magnitude of the number, and on reading, you pick up the MSB of the sequence of stored bytes and recreate the number. I suspect that zig-zag encoding is used because the amount of code for encoding and decoding is actually less when expressed in a high-level language, and perhaps faster even in assembly.
 */

#endif /* press.h */
