#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

uint8_t get_uint_bound(int16_t min, int16_t max);

#endif /* util.h */
