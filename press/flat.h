#ifndef FLAT_H
#define FLAT_H

#include <stdint.h>
#include "stats.h"

#define INDEX_2D_TRIANGLE(i, j) ((i) + ((j) * (j + 1)) / 2)
#define I2(i, j) INDEX_2D_TRIANGLE(i, j)

/* metadata for a signal sequence */
struct flat_meta {
	uint32_t nbytes;	/* number of bytes after compressing the
				   sequence as one */
	void *method_meta;	/* method-specific data */
	uint32_t *flats;	/* the first indices of the subsequences with
				   min total number of bytes after compressing
				   each separately */
	uint32_t nflats;	/* number of subsequences */
	uint32_t flats_nbytes;	/* total nbytes after compressing each
				   subsequence */
};

struct flat_method {
	uint64_t (*bound)(uint64_t);
	int (*init_meta)(const int16_t *, uint32_t, struct flat_meta *);
	void (*free_meta)(struct flat_meta *, uint32_t);
	void (*fill_meta)(const int16_t *, uint32_t, struct flat_meta *);
	/* write the length before the compressed data as uint32_t */
	int (*press)(const int16_t *, uint32_t, uint8_t *, uint32_t *);
	int (*depress)(const uint8_t *, uint32_t, int16_t *, uint32_t *);
	uint32_t (*ntobytes)(const uint8_t *, uint32_t);
};

/*
 * greedy algorithm: is it cheaper to include or disclude the next signal?
 * returns the length of the next flat sequence starting from index 0
 */
/*
uint32_t end_flat(const int16_t *in, uint32_t nin, struct stats *st);
*/

/* dynamic programming */
int get_flats(const int16_t *in, uint32_t nin, uint32_t **flats,
	      uint32_t *nflats, uint32_t *flats_nbytes,
	      const struct flat_method *method);

#endif /* flat.h */
