#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* nin: number of elements in in */
uint16_t get_max_u16(const uint16_t *in, uint64_t nin);

#endif /* util.h */
