#ifndef PRESS_H
#define PRESS_H

#include <stdint.h>
#include <zlib.h>

#define DEFINE_PRESS_METHOD(name, full_name) \
	static const struct press_method name ## _method = { \
		full_name, \
		name ## _bound, \
		name ## _press, \
		name ## _depress, \
	};

struct press_method {
	const char *name;
	uint64_t (*bound)(const uint8_t *, uint64_t);
	uint32_t (*press)(const uint8_t *, uint32_t, uint8_t *, uint32_t);
	uint32_t (*depress)(const uint8_t *, uint32_t, uint32_t, uint8_t *,
			    uint32_t);
};

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
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zlib compress the result
 */
/*
uint32_t zlib_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t zlib_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);
				  */

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zstd compress the result
 */
/*
uint32_t zstd_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t zstd_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);
				  */

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
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zlib compress the result
 */
/*
uint32_t zlib_uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t zlib_uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			      uint32_t nin_bytes, int16_t *out,
			      uint32_t nout_bytes);
			      */


/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zstd compress the result
 */
/*
uint32_t zstd_uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t zstd_uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			      uint32_t nin_bytes, int16_t *out,
			      uint32_t nout_bytes);
			      */

/*
 * zigzag subtract mean: subtract mean from all sigs and map to unsigned integers
 * compressed: [mean, x, zigzag(sigs - mean) as uintx_t]
 */
uint32_t uint_zsubmean_bound(const int16_t *in, uint32_t nin);
uint32_t uint_zsubmean_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t uint_zsubmean_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);

/*//DEFINE_PRESS_METHOD(uint_zsubmean, "subtract mean | zigzag | uintx");*/

/*
 * zigzag subtract mean: subtract mean from all sigs and map to unsigned integers
 * compressed: [mean, x, zigzag(sigs - mean) as uintx_t]
 * zlib compress the result
 */
/*
uint32_t zlib_uint_zsubmean_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_zsubmean_press(const int16_t *in, uint32_t nin,
				  uint8_t *out, uint32_t nout_bytes);
uint32_t zlib_uint_zsubmean_depress(const uint8_t *in, uint32_t nin_elems,
				    uint32_t nin_bytes, int16_t *out,
				    uint32_t nout_bytes);
				    */

/* num_sigs + min/start + x */
/*
#define NBYTES_FLAT_UINT_HDR (sizeof (uint32_t) + sizeof (int16_t) + \
			      sizeof (uint8_t))
#define NBITS_FLAT_UINT_HDR BYTES_TO_BITS(NBYTES_FLAT_UINT_HDR)
*/

/*
 * separate into flat regions, compress using uint_submin
 * compressed: [[num_sigs, min, x, sigs - min as uintx_t]...]
 */
/*
uint32_t flat_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t flat_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t flat_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);
				  */

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

/* zigzag delta svb */
/*
uint32_t svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		      uint32_t nout_bytes);
uint32_t svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);
			*/

/* zigzag delta svb0124 */
/*
uint32_t svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);
uint32_t svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
			    uint32_t nin_bytes, int16_t *out,
			    uint32_t nout_bytes);
			    */

/* zigzag delta svb12 */
/*
uint32_t svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
	       		uint32_t nout_bytes);
uint32_t svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			  uint32_t nin_bytes, int16_t *out,
			  uint32_t nout_bytes);
			  */

/* zigzag delta svb zlib */
/*
uint32_t zlib_svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes);
uint32_t zlib_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes);
			     */

/* zigzag delta svb0124 zlib */
/*
uint32_t zlib_svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			       uint32_t nout_bytes);
uint32_t zlib_svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
				 uint32_t nin_bytes, int16_t *out,
				 uint32_t nout_bytes);
				 */

/* zigzag delta svb12 zlib */
/*
uint32_t zlib_svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes);
uint32_t zlib_svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);
			       */

/* zigzag delta svb zstd */
/*
uint32_t zstd_svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes);
uint32_t zstd_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes);
			     */

/* zigzag delta svb0124 zstd */
/*
uint32_t zstd_svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			       uint32_t nout_bytes);
uint32_t zstd_svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
				 uint32_t nin_bytes, int16_t *out,
				 uint32_t nout_bytes);
				 */

/* zigzag delta svb12 zstd */
/*
uint32_t zstd_svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes);
uint32_t zstd_svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);
			       */

/* TODO
 * variable byte
 * huffman
 * peak-picking flat approximation
 */

#endif /* press.h */
