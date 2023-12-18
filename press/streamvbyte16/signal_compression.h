#ifndef SIGNAL_COMPRESSION_H
#define SIGNAL_COMPRESSION_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Return the maximum VBZ compressed size given the number of 16-bit samples
 * sample_count.
 */
size_t compressed_signal_max_size(size_t sample_count);

/*
 * VBZ compress count samples and return the result with byte length *n.
 * n cannot be NULL. Responsibility for freeing the return value is passed onto
 * the caller.
 */
uint8_t *compress_signal(
        const int16_t *samples,
        uint32_t count,
        size_t *n);

/*
 * Decompress VBZ-compressed compressed_bytes with count bytes and return the
 * result with array length *n. Responsibility for freeing the return value is
 * passed onto the caller.
 */
int16_t *decompress_signal(
        const uint8_t *compressed_bytes,
        size_t count,
        uint32_t *n);

/*
 * Compress in using 16-bit streamvbyte zigzag delta with nin samples. Write the
 * result to out and return the number of bytes written. The caller is
 * responsible for allocating enough space for out.
 */
uint64_t compress_signal_nozstd(
        bool zd,
        const int16_t *in,
        uint32_t nin,
        uint8_t *out);

/*
 * Decompress 16-bit streamvbyte zigzag delta compressed in with nin bytes and
 * write the result to out. The caller is responsible for allocating enough
 * space for out.
 */
void decompress_signal_nozstd(
        bool zd,
        const uint8_t *in,
        uint64_t nin,
        int16_t *out);

#endif /* signal_compression.h */
