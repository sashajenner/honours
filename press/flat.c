#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */
#include "flat.h"
#include "util.h"
#include "press.h"
#include "bitmap.h"

#define INDEX_2D(i, j, imax) ((i) + (j) * (imax))
#define INDEX_2D_TRIANGLE(i, j) ((i) + ((j) * (j + 1)) / 2)
#define I2(i, j) INDEX_2D_TRIANGLE(i, j)

/* metadata for a signal sequence */
struct flat_meta {
	int16_t max;		/* max signal */
	int16_t min;		/* min signal */
	uint64_t nbits;		/* number of bits after compressing the
				   sequence as one */
	/* TODO put in separate struct */
	uint32_t *flats;	/* the first indices of the subsequences with
				   min total number of bits after compressing
				   each separately */
	uint32_t nflats;	/* number of subsequences */
	uint64_t flats_nbits;	/* total nbits after compressing each
				   subsequence */
};

uint64_t get_flats_between(const int16_t *in, uint32_t nin, uint32_t i,
			   uint32_t j, uint32_t **flats, uint32_t *nflats,
			   struct flat_meta *meta);
void fill_meta(const int16_t *in, uint32_t nin, struct flat_meta *meta);
void fill_meta_nbits(const int16_t *in, uint32_t nin, struct flat_meta *meta);
void fill_meta_flats(uint32_t nin, struct flat_meta *meta);
void fill_meta_flat(uint32_t i, uint32_t j, uint32_t nin,
		    struct flat_meta *meta);
void fill_meta_flat_disjoint(uint32_t i, uint32_t j, uint32_t nin,
			     struct flat_meta *meta);
void fill_meta_flat_union(uint32_t i, uint32_t j, uint32_t nin,
			  struct flat_meta *meta);
uint64_t flat_nbits(uint32_t i, uint32_t j, uint32_t nin,
		    struct flat_meta *meta);
void free_meta(struct flat_meta *meta, uint32_t nin);
void print_meta_minmax(struct flat_meta *meta, uint32_t nin);
void print_meta_nbits(struct flat_meta *meta, uint32_t nin);

uint32_t end_flat(const int16_t *in, uint32_t nin, struct stats *st)
{
	int is_flat;
	struct stats st_prev;
	uint64_t nbits;
	uint64_t nbits_new;
	uint8_t x;

	is_flat = 1;
	nbits = NBITS_FLAT_UINT_HDR;

	init_stats(st);

	while (is_flat && st->n < nin) {
		st_prev = *st;
		update_stats(in[st->n], st);

		x = get_uint_bound(0, st->max - st->min);
		nbits_new = NBITS_FLAT_UINT_HDR + st->n * x;

		/* if adding the current signal is more expensive than placing
		 * it on its own */
		if (nbits_new > nbits + NBITS_FLAT_UINT_HDR) {
			is_flat = 0;
			*st = st_prev;
		} else {
			nbits = nbits_new;
		}
	}

	return st->n;
}

uint64_t get_flats(const int16_t *in, uint32_t nin, uint32_t **flats,
		   uint32_t *nflats)
{
	struct flat_meta *meta;
	uint64_t nbits;

	meta = malloc((uint64_t) nin * (nin + 1) / 2 * sizeof *meta);
	fprintf(stderr, "%p\n", meta);
	fill_meta(in, nin, meta);

	nbits = get_flats_between(in, nin, 0, nin - 1, flats, nflats,
				  meta);
	free_meta(meta, nin);

	return nbits;
}

uint64_t get_flats_between(const int16_t *in, uint32_t nin, uint32_t i,
			   uint32_t j, uint32_t **flats, uint32_t *nflats,
			   struct flat_meta *meta)
{
	uint32_t k;

	meta += I2(i, j);
	*nflats = meta->nflats;
	*flats = malloc(*nflats * sizeof *flats);

	for (k = 0; k < *nflats; k++) {
		(*flats)[k] = meta->flats[k];
	}

	return meta->flats_nbits;
}

void fill_meta(const int16_t *in, uint32_t nin, struct flat_meta *meta)
{
	fill_meta_nbits(in, nin, meta);
	fill_meta_flats(nin, meta);
}

void fill_meta_nbits(const int16_t *in, uint32_t nin, struct flat_meta *meta)
{
	uint32_t i;
	uint32_t j;
	int16_t min;
	int16_t max;
	struct flat_meta *cur;

	for (j = 0; j < nin; j++) {
		/*
		 * min(in[i, j]) = min(in[j], min(in[i, j - 1]))
		 * max(in[i, j]) = max(in[j], max(in[i, j - 1]))
		 */
		for (i = 0; i < j; i++) {
			min = MIN(in[j], meta[I2(i, j - 1)].min);
			max = MAX(in[j], meta[I2(i, j - 1)].max);

			cur = meta + I2(i, j);
			cur->min = min;
			cur->max = max;
			cur->nbits = flat_nbits(i, j, nin, meta);
		}
		/* min(in[j, j]) = max(in[j, j]) = in[j] */
		cur = meta + I2(j, j);
		cur->min = in[j];
		cur->max = in[j];
		cur->nbits = flat_nbits(j, j, nin, meta);
	}

	/* print_meta_minmax(meta, nin); */
}

void fill_meta_flats(uint32_t nin, struct flat_meta *meta)
{
	uint32_t i;
	uint32_t len;

	for (len = 1; len <= nin; len++) {
		for (i = 0; i <= nin - len; i++) {
			fill_meta_flat(i, i + len - 1, nin, meta);
		}
	}
}

void fill_meta_flat(uint32_t i, uint32_t j, uint32_t nin,
		    struct flat_meta *meta)
{
	struct flat_meta *cur;

	cur = meta + I2(i, j);

	cur->nflats = 1;
	cur->flats = malloc(cur->nflats * sizeof *(cur->flats));
	cur->flats[0] = i;
	cur->flats_nbits = cur->nbits;

	fill_meta_flat_disjoint(i, j, nin, meta);
	/*fill_meta_flat_union(i, j, nin, meta);*/
}

void fill_meta_flat_disjoint(uint32_t i, uint32_t j, uint32_t nin,
			     struct flat_meta *meta)
{
	struct flat_meta *cur;
	struct flat_meta *left;
	struct flat_meta *left_min;
	struct flat_meta *right;
	struct flat_meta *right_min;
	uint64_t flats_nbits;
	uint32_t k;

	cur = meta + I2(i, j);
	left_min = NULL;
	right_min = NULL;

	for (k = i; k < j; k++) {
		left = meta + I2(i, k);
		right = meta + I2(k + 1, j);
		flats_nbits = left->flats_nbits + right->flats_nbits;
		if (flats_nbits < cur->flats_nbits) {
			cur->flats_nbits = flats_nbits;
			left_min = left;
			right_min = right;
		}
	}

	if (left_min && right_min) {
		cur->nflats = left_min->nflats + right_min->nflats;
		cur->flats = realloc(cur->flats, cur->nflats *
				     sizeof(*cur->flats));
		for (k = 0; k < left_min->nflats; k++) {
			cur->flats[k] = left_min->flats[k];
			/*
			if (k > 0)
				printf("flat_idx:\t%" PRIu32 "\n", left_min->flats[k]);
				*/
		}
		for (k = 0; k < right_min->nflats; k++) {
			cur->flats[left_min->nflats + k] = right_min->flats[k];
			/*
			if (k > 0)
				printf("flat_idx:\t%" PRIu32 "\n", right_min->flats[k]);
				*/
		}
	}
}

void fill_meta_flat_union(uint32_t i, uint32_t j, uint32_t nin,
			  struct flat_meta *meta)
{
	struct flat_meta *cur;
	struct flat_meta *left;
	struct flat_meta *left_min;
	struct flat_meta *mid;
	struct flat_meta *mid_left;
	struct flat_meta *mid_right;
	struct flat_meta *right;
	struct flat_meta *right_min;
	uint64_t flats_nbits;
	uint32_t k;
	uint32_t x;
	uint32_t y;

	cur = meta + I2(i, j);
	left_min = NULL;
	right_min = NULL;

	for (k = i; k < j; k++) {
		left = meta + I2(i, k);
		right = meta + I2(k + 1, j);

		x = left->flats[left->nflats - 1];
		if (right->nflats >= 2)
			y = right->flats[1] - 1;
		else
			y = j;

		mid = meta + I2(x, y);
		mid_left = meta + I2(x, k);
		mid_right = meta + I2(k + 1, y);

		flats_nbits = (left->flats_nbits - mid_left->nbits) + mid->nbits
			     + (right->flats_nbits - mid_right->nbits);
		if (flats_nbits < cur->flats_nbits) {
			cur->flats_nbits = flats_nbits;
			left_min = left;
			right_min = right;
		}
	}

	if (left_min && right_min) {
		cur->nflats = left_min->nflats + right_min->nflats - 1;
		cur->flats = realloc(cur->flats, cur->nflats *
				     sizeof(*cur->flats));
		for (k = 0; k < left_min->nflats; k++) {
			cur->flats[k] = left_min->flats[k];
		}
		for (k = 1; k < right_min->nflats; k++) {
			cur->flats[left_min->nflats + k] = right_min->flats[k];
		}
	}
}

uint64_t flat_nbits(uint32_t i, uint32_t j, uint32_t nin,
		    struct flat_meta *meta)
{
	struct flat_meta *cur;
	uint8_t x;
	uint32_t nbytes;

	cur = meta + I2(i, j);

	x = get_uint_bound(0, cur->max - cur->min);
	/* TODO do not waste space to nearest byte boundary */
	nbytes = NBYTES_FLAT_UINT_HDR + uintx_bound(x, NULL, j - i + 1);
	return nbytes * BITS_PER_BYTE;
}

void free_meta(struct flat_meta *meta, uint32_t nin)
{
	uint32_t i;
	uint32_t j;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			free(meta[I2(i, j)].flats);
		}
	}

	free(meta);
}

void print_meta_minmax(struct flat_meta *meta, uint32_t nin)
{
	uint32_t i;
	uint32_t j;
	int16_t min;
	int16_t max;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			min = meta[I2(i, j)].min;
			max = meta[I2(i, j)].max;
			printf("[%" PRIu32 ",%" PRIu32 "]: min %" PRId16 ", max %" PRId16 "\n",
			       i, j, min, max);
		}
	}
}

void print_meta_nbits(struct flat_meta *meta, uint32_t nin)
{
	uint32_t i;
	uint32_t j;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			printf("[%" PRIu32 ",%" PRIu32 "]: nbits %" PRIu64 "\n",
			       i, j, meta[I2(i, j)].nbits);
		}
	}
}