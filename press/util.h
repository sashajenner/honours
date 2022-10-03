#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* nin: number of elements in in */
uint16_t get_min_u16(const uint16_t *in, uint64_t nin);
uint16_t get_max_u16(const uint16_t *in, uint64_t nin);
uint32_t get_max_u32(const uint32_t *in, uint64_t nin);
void get_minmax_u16(const uint16_t *in, uint64_t nin, uint16_t *min,
		    uint16_t *max);
int16_t get_mean_16(const int16_t *in, uint64_t nin);
char *array_to_str(const uint32_t *x, uint32_t n);

#endif /* util.h */
