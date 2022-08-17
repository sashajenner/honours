#ifndef PRESS_H
#define PRESS_H

#include <stdint.h>

/*
 * nin: number of signals in in
 * return: upper bound on the number of bytes to store compressed array
 */
uint32_t none_bound(const int16_t *in, uint32_t nin);
/*
 * out: cannot be NULL
 * return: number of bytes written to out
 */
uint32_t none_press(const int16_t *in, uint32_t nin, uint8_t *out);
/*
 * nin: number of signals (not number of bytes) in in
 * out: cannot be NULL
 * return: number of signals written to out
 */
uint32_t none_depress(const uint8_t *in, uint32_t nin, int16_t *out);

/* store each int16_t as uintx_t */
uint32_t uintx_bound(uint8_t x, const int16_t *in, uint32_t nin);
uint32_t uintx_press(uint8_t x, const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uintx_depress(uint8_t x, const uint8_t *in, uint32_t nin, int16_t *out);

static inline uint32_t uint11_bound(const int16_t *in, uint32_t nin)
{
	return uintx_bound(11, in, nin);
}
static inline uint32_t uint11_press(const int16_t *in, uint32_t nin, uint8_t *out)
{
	return uintx_press(11, in, nin, out);
}
static inline uint32_t uint11_depress(const uint8_t *in, uint32_t nin, int16_t *out)
{
	return uintx_depress(11, in, nin, out);
}

/*
 * store each int16_t as uintx_t where x is decided after one pass
 * compressed: [x, sigs as uintx_t]
 */
uint32_t uint_bound(const int16_t *in, uint32_t nin);
uint32_t uint_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uint_depress(const uint8_t *in, uint32_t nin, int16_t *out);

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
uint32_t uint_outliers_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uint_outliers_depress(const uint8_t *in, uint32_t nin, int16_t *out);
*/

/*
 * subtract min from all sigs
 * compressed: [min, x, sigs - min as uintx_t]
 */
uint32_t uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uint_submin_depress(const uint8_t *in, uint32_t nin, int16_t *out);

/*
 * zigzag delta: take successive differences and map to unsigned integers
 * compressed: [start, x, sigs zigzag delta as uintx_t]
 */
uint32_t uint_zd_bound(const int16_t *in, uint32_t nin);
uint32_t uint_zd_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uint_zd_depress(const uint8_t *in, uint32_t nin, int16_t *out);

/*
 * zigzag subtract mean: subtract mean from all sigs and map to unsigned integers
 * compressed: [mean, x, zigzag(sigs - mean) as uintx_t]
 */
uint32_t uint_zsubmean_bound(const int16_t *in, uint32_t nin);
uint32_t uint_zsubmean_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t uint_zsubmean_depress(const uint8_t *in, uint32_t nin, int16_t *out);

/* num_sigs + min + x */
#define NBYTES_FLAT_UINT_SUBMIN_HDR (sizeof (uint32_t) + sizeof (int16_t) + \
				     sizeof (uint8_t))
#define NBITS_FLAT_UINT_SUBMIN_HDR (NBYTES_FLAT_UINT_SUBMIN_HDR * \
				    BITS_PER_BYTE)

/*
 * separate into flat regions, compress using uint_submin
 * compressed: [[num_sigs, min, x, sigs - min as uintx_t]...]
 */
uint32_t flat_uint_submin_bound(const int16_t *in, uint32_t nin);
uint32_t flat_uint_submin_press(const int16_t *in, uint32_t nin, uint8_t *out);
uint32_t flat_uint_submin_depress(const uint8_t *in, uint32_t nin, int16_t *out);

#endif /* press.h */
