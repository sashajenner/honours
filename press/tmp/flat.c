#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */
#include "flat.h"
#include "util.h"
#include "press.h"

/* min and max of each subsequence is cached */
#define NUM_ELEMENTS (2)
#define MIN_INDEX (0)
#define MAX_INDEX (1)

#define INDEX_2D(i, j, imax) ((i) + (j) * (imax))
#define INDEX_3D(i, j, k, imax, jmax) ((i) + (j) * (imax) + (k) * (imax) * (jmax))
/* cache index */
#define I2(i, j, nin) INDEX_2D(i, j, nin)
#define I3(i, j, k, nin) INDEX_3D(i, j, k, nin, nin)

#define FLATS_MIN_CAPACITY (64)

static uint64_t get_flats_between(const int16_t *in, uint64_t nin,
				  uint64_t start, uint64_t end,
				  uint32_t **flats, uint64_t *nflats,
				  uint32_t *cache);
static void fill_cache_minmax(const int16_t *in, uint64_t nin, int16_t *cache);
static void fill_cache_nbits(const int16_t *in, uint64_t nin,
			     int16_t *cache_minmax, uint32_t *cache_nbits);
static uint32_t flat_nbits(uint64_t start, uint64_t end, uint64_t nin,
			   int16_t *cache);
static void print_cache_minmax(int16_t *cache, uint64_t nin);
static void print_cache_nbits(uint32_t *cache, uint64_t nin);

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
	int16_t *cache_minmax;
	uint32_t *cache_nbits;
	uint64_t nbits;

	cache_minmax = malloc(nin * nin * NUM_ELEMENTS * sizeof *cache_minmax);
	fill_cache_minmax(in, nin, cache_minmax);
	fprintf(stderr, "cache_minmax:\t%p\n", cache_minmax);

	/* TODO reuse minmax cache */
	cache_nbits = malloc(nin * nin * sizeof *cache_nbits);
	fprintf(stderr, "cache_nbits:\t%p\n", cache_minmax);
	fill_cache_nbits(in, nin, cache_minmax, cache_nbits);
	free(cache_minmax);

	nbits = get_flats_between(in, nin, 0, nin - 1, flats, nflats,
				  cache_nbits);
	free(cache_nbits);

	return nbits;
}

static uint64_t get_flats_between(const int16_t *in, uint64_t nin,
				  uint64_t start, uint64_t end,
				  uint32_t **flats, uint64_t *nflats,
				  uint32_t *cache)
{
	uint32_t *flats_l;
	uint32_t *flats_r;
	uint32_t nbits;
	uint32_t nbits_l;
	uint32_t nbits_r;
	uint64_t capflats;
	uint64_t i;
	uint64_t nflats_l;
	uint64_t nflats_r;

	capflats = FLATS_MIN_CAPACITY;
	*flats = malloc(capflats * sizeof *flats);
	*nflats = 1;
	(*flats)[0] = end;
	nbits = cache[I2(start, end, nin)];

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
			memcpy(*flats + nflats_l, flats_r,
			       nflats_r * sizeof *flats);
		}

		free(flats_l);
		free(flats_r);
	}

	return nbits;
}

static void fill_cache_minmax(const int16_t *in, uint64_t nin, int16_t *cache)
{
	uint64_t i;
	uint64_t j;

	for (j = 0; j < nin; j++) {
		/*
		 * min(in[i, j]) = min(in[j], min(in[i, j - 1]))
		 * max(in[i, j]) = max(in[j], max(in[i, j - 1]))
		 */
		for (i = 0; i < j; i++) {
			cache[I3(i, j, MIN_INDEX, nin)] = MIN(in[j], cache[I3(i, j - 1, MIN_INDEX, nin)]);
			cache[I3(i, j, MAX_INDEX, nin)] = MAX(in[j], cache[I3(i, j - 1, MAX_INDEX, nin)]);
		}
		/* min(in[j, j]) = max(in[j, j]) = in[j] */
		cache[I3(j, j, MIN_INDEX, nin)] = in[j];
		cache[I3(j, j, MAX_INDEX, nin)] = in[j];
	}

	/* print_cache_minmax(cache, nin); */
}

static void fill_cache_nbits(const int16_t *in, uint64_t nin,
			     int16_t *cache_minmax, uint32_t *cache_nbits)
{
	uint64_t i;
	uint64_t j;
	uint32_t nbits;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			nbits = flat_nbits(i, j, nin, cache_minmax);
			cache_nbits[I2(i, j, nin)] = nbits;
		}
	}

	/* print_cache_nbits(cache_nbits, nin); */
}

static uint32_t flat_nbits(uint64_t start, uint64_t end, uint64_t nin,
			   int16_t *cache)
{
	uint8_t x;
	int16_t min;
	int16_t max;

	min = cache[I3(start, end, MIN_INDEX, nin)];
	max = cache[I3(start, end, MAX_INDEX, nin)];

	x = get_uint_bound(0, max - min);
	return NBITS_FLAT_UINT_SUBMIN_HDR + x * (end - start + 1);
}

static void print_cache_minmax(int16_t *cache, uint64_t nin)
{
	uint64_t i;
	uint64_t j;
	int16_t min;
	int16_t max;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			min = cache[I3(i, j, MIN_INDEX, nin)];
			max = cache[I3(i, j, MAX_INDEX, nin)];
			printf("[%" PRIu64 ",%" PRIu64 "]: min %" PRId16 ", max %" PRId16 "\n",
			       i, j, min, max);
		}
	}
}

static void print_cache_nbits(uint32_t *cache, uint64_t nin)
{
	uint64_t i;
	uint64_t j;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			printf("[%" PRIu64 ",%" PRIu64 "]: nbits %" PRIu32 "\n",
			       i, j, cache[I2(i, j, nin)]);
		}
	}
}
