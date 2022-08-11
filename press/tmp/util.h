#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MIN(x, y) ((x) ? (x) < (y) : (y))
#define MAX(x, y) ((x) ? (x) > (y) : (y))

uint8_t get_uint_bound(int16_t min, int16_t max);

#endif /* util.h */
