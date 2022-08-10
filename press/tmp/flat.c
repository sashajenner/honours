#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */
#include "flat.h"
#include "util.h"
#include "press.h"

#define INDEX(i, j, k, jmax, kmax) ((i) + (j) * (jmax) + (k) * (jmax) * (kmax))
#define FLATS_MIN_CAPACITY (64)

static uint64_t get_flats_between(const int16_t *in, uint64_t nin,
				  uint64_t start, uint64_t end,
				  uint32_t **flats, uint64_t *nflats,
				  int16_t *cache);
static void update_cache(const int16_t *in, uint64_t nin, uint64_t start,
			 uint64_t end, int16_t *cache);
static uint64_t flat_nbits(uint64_t start, uint64_t end, uint64_t nin,
			   int16_t *cache);

uint64_t end_flat(const int16_t *in, uint64_t nin, struct stats *st)
{
	int is_flat;
	struct stats st_prev;
	uint64_t nbits;
	uint64_t nbits_new;
	uint8_t x;
	uint8_t x_new;

	is_flat = 1;
	nbits = NBITS_FLAT_UINT_SUBMIN_HDR;
	x = 0;

	init_stats(st);

	while (is_flat && st->n < nin) {
		st_prev = *st;
		update_stats(in[st->n], st);

		x_new = get_uint_bound(0, st->max - st->min);
		nbits_new = NBITS_FLAT_UINT_SUBMIN_HDR + st->n * x_new;

		/* if adding the current signal is more expensive than placing
		 * it on its own */
		if (nbits_new > nbits + NBITS_FLAT_UINT_SUBMIN_HDR) {
			is_flat = 0;
			*st = st_prev;
		} else {
			nbits = nbits_new;
		}
	}

	return st->n;
}

uint64_t get_flats(const int16_t *in, uint64_t nin, uint32_t **flats,
		   uint64_t *nflats)
{
	int16_t *cache;
	uint64_t nbits;

	cache = calloc(nin * nin * 2, sizeof *cache);
	fprintf(stderr, "cache:\t%p\n", cache);
	fprintf(stderr, "cache elements:\t%ld\n", nin * nin * 2);
	nbits = get_flats_between(in, nin, 0, nin - 1, flats, nflats, cache);
	free(cache);

	return nbits;
}

static uint64_t get_flats_between(const int16_t *in, uint64_t nin,
				  uint64_t start, uint64_t end,
				  uint32_t **flats, uint64_t *nflats,
				  int16_t *cache)
{
	uint32_t *flats_l;
	uint32_t *flats_r;
	uint64_t capflats;
	uint64_t i;
	uint64_t nbits;
	uint64_t nbits_l;
	uint64_t nbits_r;
	uint64_t nflats_l;
	uint64_t nflats_r;

	capflats = FLATS_MIN_CAPACITY;
	*flats = malloc(capflats * sizeof *flats);
	*nflats = 1;
	(*flats)[0] = end;
	update_cache(in, nin, start, end, cache);
	nbits = flat_nbits(start, end, nin, cache);

	for (i = 0; i < end; i++) {
		nbits_l = get_flats_between(in, nin, 0, i, &flats_l, &nflats_l,
					    cache);
		nbits_r = get_flats_between(in, nin, i + 1, end, &flats_r,
					    &nflats_r, cache);

		if (nbits > nbits_l + nbits_r) {
			nbits = nbits_l + nbits_r;
			*nflats = nflats_l + nflats_r;
			if (*nflats > capflats) {
				capflats <<= 2;
				*flats = realloc(*flats, capflats);
			}
			memcpy(*flats, flats_l, nflats_l * sizeof *flats);
			memcpy(*flats + nflats_l, flats_r, nflats_r * sizeof *flats);
		}

		free(flats_l);
		free(flats_r);
	}

	return nbits;
}

static void update_cache(const int16_t *in, uint64_t nin, uint64_t start,
			 uint64_t end, int16_t *cache)
{
	uint64_t j;
	int16_t min;
	int16_t max;

	fprintf(stderr, "cache index:\t%ld\n", INDEX(start, end, 0, nin, 2));
	min = cache[INDEX(start, end, 0, nin, 2)];
	max = cache[INDEX(start, end, 1, nin, 2)];
	if (min && max)
		return;

	for (j = start; j <= end; j++) {
		min = cache[INDEX(start, end, 0, nin, 2)];
		max = cache[INDEX(start, end, 1, nin, 2)];
		if (!min || in[j] < min) {
			cache[INDEX(start, j, 0, nin, 2)] = in[j];
		}
		if (in[j] > max) {
			cache[INDEX(start, j, 1, nin, 2)] = in[j];
		}
	}
}

static uint64_t flat_nbits(uint64_t start, uint64_t end, uint64_t nin,
			   int16_t *cache)
{
	uint8_t x;
	int16_t min;
	int16_t max;

	min = cache[INDEX(start, end, 0, nin, 2)];
	max = cache[INDEX(start, end, 1, nin, 2)];

	x = get_uint_bound(0, max - min);
	return NBITS_FLAT_UINT_SUBMIN_HDR + x * (start - end + 1);
}
