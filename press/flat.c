#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */
#include "flat.h"
#include "util.h"
#include "press.h"
#include "bitmap.h"

/*#define INDEX_2D(i, j, imax) ((i) + (j) * (imax))*/

uint32_t get_flats_between(const int16_t *in, uint32_t nin, uint32_t i,
			   uint32_t j, uint32_t **flats, uint32_t *nflats,
			   struct flat_meta *meta);
void fill_meta_flats(uint32_t nin, uint32_t step, struct flat_meta *meta);
void fill_meta_flat(uint32_t i, uint32_t j, uint32_t nin, uint32_t step,
		    struct flat_meta *meta);
void fill_meta_flat_disjoint(uint32_t i, uint32_t j, uint32_t nin,
			     uint32_t step, struct flat_meta *meta);
/*
void fill_meta_flat_union(uint32_t i, uint32_t j, uint32_t nin,
			  struct flat_meta *meta);
			  */
void free_meta(struct flat_meta *meta, uint32_t nin, uint32_t step);
void print_meta_minmax(struct flat_meta *meta, uint32_t nin);
void print_meta_nbytes(struct flat_meta *meta, uint32_t nin);

/*
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

		*/
		/* if adding the current signal is more expensive than placing
		 * it on its own */
/*
		if (nbits_new > nbits + NBITS_FLAT_UINT_HDR) {
			is_flat = 0;
			*st = st_prev;
		} else {
			nbits = nbits_new;
		}
	}

	return st->n;
}
*/

int get_flats(const int16_t *in, uint32_t nin, uint32_t step, uint32_t **flats,
	      uint32_t *nflats, uint32_t *flats_nbytes,
	      const struct flat_method *method)
{
	int ret;
	struct flat_meta *meta;

	meta = malloc((uint64_t) nin * (nin + 1) / 2 * sizeof *meta);
	if (!meta)
		return -1;

	ret = method->init_meta(in, nin, meta);
	if (ret) {
		free_meta(meta, nin, step);
		return ret;
	}

	method->fill_meta(in, nin, step, meta);
	fill_meta_flats(nin, step, meta);
	*flats_nbytes = get_flats_between(in, nin, 0, nin - 1, flats, nflats,
					  meta);
	method->free_meta(meta, nin);
	free_meta(meta, nin, step);

	return 0;
}

uint32_t get_flats_between(const int16_t *in, uint32_t nin, uint32_t i,
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

	return meta->flats_nbytes;
}

void fill_meta_flats(uint32_t nin, uint32_t step, struct flat_meta *meta)
{
	uint32_t i;
	uint32_t len;

	for (len = step; len <= nin; len += step) {
		for (i = 0; i <= nin - len; i += step) {
			fill_meta_flat(i, i + len - 1, nin, step, meta);
		}
	}
}

void fill_meta_flat(uint32_t i, uint32_t j, uint32_t nin, uint32_t step,
		    struct flat_meta *meta)
{
	struct flat_meta *cur;

	cur = meta + I2(i, j);

	cur->nflats = 1;
	cur->flats = malloc(cur->nflats * sizeof *(cur->flats));
	cur->flats[0] = i;
	cur->flats_nbytes = cur->nbytes;

	fill_meta_flat_disjoint(i, j, nin, step, meta);
	/*fill_meta_flat_union(i, j, nin, meta);*/
}

void fill_meta_flat_disjoint(uint32_t i, uint32_t j, uint32_t nin,
			     uint32_t step, struct flat_meta *meta)
{
	struct flat_meta *cur;
	struct flat_meta *left;
	struct flat_meta *left_min;
	struct flat_meta *right;
	struct flat_meta *right_min;
	uint32_t flats_nbytes;
	uint32_t k;

	cur = meta + I2(i, j);
	left_min = NULL;
	right_min = NULL;

	for (k = i + step - 1; k < j; k += step) {
		left = meta + I2(i, k);
		right = meta + I2(k + 1, j);
		flats_nbytes = left->flats_nbytes + right->flats_nbytes;
		if (flats_nbytes < cur->flats_nbytes) {
			cur->flats_nbytes = flats_nbytes;
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

/*
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
	uint64_t flats_nbytes;
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

		flats_nbytes = (left->flats_nbytes - mid_left->nbytes) + mid->nbytes
			     + (right->flats_nbytes - mid_right->nbytes);
		if (flats_nbytes < cur->flats_nbytes) {
			cur->flats_nbytes = flats_nbytes;
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
*/

void free_meta(struct flat_meta *meta, uint32_t nin, uint32_t step)
{
	uint32_t i;
	uint32_t j;

	for (j = step - 1; j < nin; j += step) {
		for (i = 0; i <= j; i += step) {
			free(meta[I2(i, j)].flats);
		}
	}

	free(meta);
}

/*
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

void print_meta_nbytes(struct flat_meta *meta, uint32_t nin)
{
	uint32_t i;
	uint32_t j;

	for (j = 0; j < nin; j++) {
		for (i = 0; i <= j; i++) {
			printf("[%" PRIu32 ",%" PRIu32 "]: nbytes %" PRIu64 "\n",
			       i, j, meta[I2(i, j)].nbytes);
		}
	}
}
*/
