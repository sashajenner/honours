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
	uint32_t (*bound)(const int16_t *, uint32_t);
	uint32_t (*press)(const int16_t *, uint32_t, uint8_t *, uint32_t);
	uint32_t (*depress)(const uint8_t *, uint32_t, uint32_t, int16_t *,
			    uint32_t);
};

/*
 * nin: number of signals in in
 * return: upper bound on the number of bytes to store compressed array
 */
uint32_t none_bound(const int16_t *in, uint32_t nin);
/*
 * out: cannot be NULL
 * nout_bytes: number of bytes allocated to out
 * return: number of bytes written to out
 */
uint32_t none_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes);
/*
 * nin_elems: number of signals in in
 * nin_bytes: number of bytes in in
 * out: cannot be NULL
 * return: number of signals written to out
 */
uint32_t none_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(none, "none");

/* store each int16_t as uintx_t */
uint32_t uintx_bound(uint8_t x, const int16_t *in, uint32_t nin);
uint32_t uintx_press(uint8_t x, const int16_t *in, uint32_t nin, uint8_t *out,
		     uint32_t nout_bytes);
uint32_t uintx_depress(uint8_t x, const uint8_t *in, uint32_t nin_elems,
		       uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

static inline uint32_t uint11_bound(const int16_t *in, uint32_t nin)
{
	return uintx_bound(11, in, nin);
}
static inline uint32_t uint11_press(const int16_t *in, uint32_t nin,
				    uint8_t *out, uint32_t nout_bytes)
{
	return uintx_press(11, in, nin, out, nout_bytes);
}
static inline uint32_t uint11_depress(const uint8_t *in, uint32_t nin_elems,
				      uint32_t nin_bytes, int16_t *out,
				      uint32_t nout_bytes)
{
	return uintx_depress(11, in, nin_elems, nin_bytes, out, nout_bytes);
}

DEFINE_PRESS_METHOD(uint11, "uint11");

/*
 * store each int16_t as uintx_t where x is decided after one pass
 * compressed: [x, sigs as uintx_t]
 */
uint32_t uint_bound(const int16_t *in, uint32_t nin);
uint32_t uint_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes);
uint32_t uint_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(uint, "uintx");

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
uint32_t uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes);
uint32_t uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes);

DEFINE_PRESS_METHOD(uint_submin, "subtract min | uintx");

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zlib compress the result
 */
uint32_t zlib_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t zlib_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_uint_submin, "subtract min | uintx | zlib");

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 * zstd compress the result
 */
uint32_t zstd_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t zstd_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd_uint_submin, "subtract min | uintx | zstd");

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 */
uint32_t uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint32_t nout_bytes);
uint32_t uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			 uint32_t nin_bytes, int16_t *out,
			 uint32_t nout_bytes);

DEFINE_PRESS_METHOD(uint_zd, "delta | zigzag | uintx");

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zlib compress the result
 */
uint32_t zlib_uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t zlib_uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			      uint32_t nin_bytes, int16_t *out,
			      uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_uint_zd, "delta | zigzag | uintx | zlib");

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 * zstd compress the result
 */
uint32_t zstd_uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			    uint32_t nout_bytes);
uint32_t zstd_uint_zd_depress(const uint8_t *in, uint32_t nin_elems,
			      uint32_t nin_bytes, int16_t *out,
			      uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd_uint_zd, "delta | zigzag | uintx | zstd");

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

DEFINE_PRESS_METHOD(uint_zsubmean, "subtract mean | zigzag | uintx");

/*
 * zigzag subtract mean: subtract mean from all sigs and map to unsigned integers
 * compressed: [mean, x, zigzag(sigs - mean) as uintx_t]
 * zlib compress the result
 */
uint32_t zlib_uint_zsubmean_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_uint_zsubmean_press(const int16_t *in, uint32_t nin,
				  uint8_t *out, uint32_t nout_bytes);
uint32_t zlib_uint_zsubmean_depress(const uint8_t *in, uint32_t nin_elems,
				    uint32_t nin_bytes, int16_t *out,
				    uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_uint_zsubmean, "subtract mean | zigzag | uintx | zlib");

/* num_sigs + min/start + x */
#define NBYTES_FLAT_UINT_HDR (sizeof (uint32_t) + sizeof (int16_t) + \
			      sizeof (uint8_t))
#define NBITS_FLAT_UINT_HDR BYTES_TO_BITS(NBYTES_FLAT_UINT_HDR)

/*
 * separate into flat regions, compress using uint_submin
 * compressed: [[num_sigs, min, x, sigs - min as uintx_t]...]
 */
uint32_t flat_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t flat_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out,
				uint32_t nout_bytes);
uint32_t flat_uint_submin_depress(const uint8_t *in, uint32_t nin_elems,
				  uint32_t nin_bytes, int16_t *out,
				  uint32_t nout_bytes);

DEFINE_PRESS_METHOD(flat_uint_submin, "subtract min | uintx | flats");

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
uint32_t zlib_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes);
uint32_t zlib_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib, "zlib");

/* bzip2 */
#define PRESS_LVL_BZIP2 (9)
#define PRESS_VERBOSE_BZIP2 (0)
#define PRESS_WORKFACTOR_BZIP2 (30)
#define PRESS_SMALL_BZIP2 (0)
uint32_t bzip2_bound(const int16_t *in, uint32_t nin);
uint32_t bzip2_press(const int16_t *in, uint32_t nin, uint8_t *out,
		     uint32_t nout_bytes);
uint32_t bzip2_depress(const uint8_t *in, uint32_t nin_elems,
		       uint32_t nin_bytes, int16_t *out,
		       uint32_t nout_bytes);

DEFINE_PRESS_METHOD(bzip2, "bzip2");

/* zstd */
#define PRESS_LVL_ZSTD (1)
uint32_t zstd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		    uint32_t nout_bytes);
uint32_t zstd_depress(const uint8_t *in, uint32_t nin_elems,
		      uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd, "zstd");

/* fast lzma2 */
#define PRESS_LVL_FAST_LZMA2 (6)
#define PRESS_NTHREADS_FAST_LZMA2 (0)
uint32_t fast_lzma2_bound(const int16_t *in, uint32_t nin);
uint32_t fast_lzma2_press(const int16_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);
uint32_t fast_lzma2_depress(const uint8_t *in, uint32_t nin_elems,
			    uint32_t nin_bytes, int16_t *out,
			    uint32_t nout_bytes);

DEFINE_PRESS_METHOD(fast_lzma2, "fast lzma2");

/* classical svb 1,2,3,4 bytes */
uint32_t svb_bound(const int16_t *in, uint32_t nin);
uint32_t svb_press(const int16_t *in, uint32_t nin, uint8_t *out,
		   uint32_t nout_bytes);
uint32_t svb_depress(const uint8_t *in, uint32_t nin_elems, uint32_t nin_bytes,
		     int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb, "svb");

/* svb 0,1,2,4 bytes */
uint32_t svb0124_bound(const int16_t *in, uint32_t nin);
uint32_t svb0124_press(const int16_t *in, uint32_t nin, uint8_t *out,
		       uint32_t nout_bytes);
uint32_t svb0124_depress(const uint8_t *in, uint32_t nin_elems,
			 uint32_t nin_bytes, int16_t *out,
			 uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb0124, "svb0124");

/* svb(16) 1,2 bytes */
uint32_t svb12_bound(const int16_t *in, uint32_t nin);
uint32_t svb12_press(const int16_t *in, uint32_t nin, uint8_t *out,
		     uint32_t nout_bytes);
uint32_t svb12_depress(const uint8_t *in, uint32_t nin_elems,
		       uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb12, "svb12");

/* zigzag delta svb */
uint32_t svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
		      uint32_t nout_bytes);
uint32_t svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			uint32_t nin_bytes, int16_t *out, uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb_zd, "delta | zigzag | svb");

/* zigzag delta svb0124 */
uint32_t svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			  uint32_t nout_bytes);
uint32_t svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
			    uint32_t nin_bytes, int16_t *out,
			    uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb0124_zd, "delta | zigzag | svb0124");

/* zigzag delta svb12 */
uint32_t svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
	       		uint32_t nout_bytes);
uint32_t svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			  uint32_t nin_bytes, int16_t *out,
			  uint32_t nout_bytes);

DEFINE_PRESS_METHOD(svb12_zd, "delta | zigzag | svb12");

/* zigzag delta svb zlib */
uint32_t zlib_svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes);
uint32_t zlib_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_svb_zd, "delta | zigzag | svb | zlib");

/* zigzag delta svb0124 zlib */
uint32_t zlib_svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			       uint32_t nout_bytes);
uint32_t zlib_svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
				 uint32_t nin_bytes, int16_t *out,
				 uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_svb0124_zd, "delta | zigzag | svb0124 | zlib");

/* zigzag delta svb12 zlib */
uint32_t zlib_svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zlib_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes);
uint32_t zlib_svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zlib_svb12_zd, "delta | zigzag | svb12 | zlib");

/* zigzag delta svb zstd */
uint32_t zstd_svb_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			   uint32_t nout_bytes);
uint32_t zstd_svb_zd_depress(const uint8_t *in, uint32_t nin_elems,
			     uint32_t nin_bytes, int16_t *out,
			     uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd_svb_zd, "delta | zigzag | svb | zstd");

/* zigzag delta svb0124 zstd */
uint32_t zstd_svb0124_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb0124_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			       uint32_t nout_bytes);
uint32_t zstd_svb0124_zd_depress(const uint8_t *in, uint32_t nin_elems,
				 uint32_t nin_bytes, int16_t *out,
				 uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd_svb0124_zd, "delta | zigzag | svb0124 | zstd");

/* zigzag delta svb12 zstd */
uint32_t zstd_svb12_zd_bound(const int16_t *in, uint32_t nin);
uint32_t zstd_svb12_zd_press(const int16_t *in, uint32_t nin, uint8_t *out,
			     uint32_t nout_bytes);
uint32_t zstd_svb12_zd_depress(const uint8_t *in, uint32_t nin_elems,
			       uint32_t nin_bytes, int16_t *out,
			       uint32_t nout_bytes);

DEFINE_PRESS_METHOD(zstd_svb12_zd, "delta | zigzag | svb12 | zstd");

/* TODO
 * variable byte
 * huffman
 * peak-picking flat approximation
 */

#endif /* press.h */
