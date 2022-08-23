#ifndef SVB16_H
#define SVB16_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
#if __has_builtin(__builtin_popcount)
// likely to be a single instruction (POPCNT) on x86_64
#define svb16_popcount __builtin_popcount
#else
*/
// optimising compilers can often convert this pattern to POPCNT on x86_64
inline int svb16_popcount(unsigned int i) {
    i = i - ((i >> 1) & 0x55555555);                 // add pairs of bits
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
    i = (i + (i >> 4)) & 0x0F0F0F0F;                 // groups of 8
    return (i * 0x01010101) >> 24;                   // horizontal sum of bytes
}
/*
#endif
*/

/// Get the number of key bytes required to encode a given number of 16-bit integers.
inline uint32_t svb16_key_length(uint32_t count) {
    // ceil(count / 8.0), without overflowing or using fp arithmetic
    return (count >> 3) + (((count & 7) + 7) >> 3);
}

/// Get the maximum number of bytes required to encode a given number of 16-bit integers.
inline uint32_t svb16_max_encoded_length(uint32_t count) {
    return svb16_key_length(count) + (2 * count);
}

size_t encode(bool UseDelta, bool UseZigzag, int16_t const* in, uint8_t* SVB_RESTRICT out, uint32_t count);
uint8_t* encode_scalar(bool UseDelta, bool UseZigzag, int16_t const* in,
                       uint8_t* SVB_RESTRICT keys,
                       uint8_t* SVB_RESTRICT data,
                       uint32_t count,
                       int16_t prev);

#ifdef SVB16_X64
/*[[gnu::target("ssse3")]]*/ uint8_t *encode_sse(bool UseDelta, bool UseZigzag, int16_t const *in,
                                             uint8_t *SVB_RESTRICT keys_dest,
                                             uint8_t *SVB_RESTRICT data_dest,
                                             uint32_t count,
                                             int16_t prev);
#endif  // SVB16_X64

size_t decode(bool UseDelta, bool UseZigzag, int16_t* out, uint8_t const* SVB_RESTRICT in, uint32_t count);
uint8_t const *decode_scalar(bool UseDelta, bool UseZigzag, int16_t *out,
                             uint8_t const *SVB_RESTRICT keys,
                             uint8_t const *SVB_RESTRICT data,
                             uint32_t count,
                             int16_t prev);
#ifdef SVB16_X64
/*[[gnu::target("sse4.1")]]*/ uint8_t const *decode_sse(bool UseDelta, bool UseZigzag, int16_t *out,
                                                    uint8_t const *SVB_RESTRICT keys,
                                                    uint8_t const *SVB_RESTRICT data,
                                                    uint32_t count,
                                                    int16_t prev);
#endif  // SVB16_X64

#if defined(__cplusplus)
};
#endif

#endif  // SVB16_H
