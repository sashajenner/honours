#ifndef FLAT_H
#define FLAT_H

#include <stdint.h>
#include "stats.h"

/*
 * greedy algorithm: is it cheaper to include or disclude the next signal?
 * returns the length of the next flat sequence starting from index 0
 */
uint64_t end_flat(const int16_t *in, uint64_t nin, struct stats *st);

/* dynamic programming */
uint64_t get_flats(const int16_t *in, uint64_t nin, uint32_t **flats,
		   uint64_t *nflats);

#endif /* flat.h */
