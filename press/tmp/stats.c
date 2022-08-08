#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "stats.h"

void init_stats(struct stats *st)
{
	st->n = 0;
	st->min = INT16_MAX;
	st->max = INT16_MIN;
	st->delta = 0;
	st->mean = 0;
	st->m2 = 0;
	st->var = 0;
	st->sd = 0;
}

static inline void update_stats_n(struct stats *st)
{
	st->n ++;
}

static inline void update_stats_min(int16_t x, struct stats *st)
{
	if (x < st->min)
		st->min = x;
}

static inline void update_stats_max(int16_t x, struct stats *st)
{
	if (x > st->max)
		st->max = x;
}

static inline void update_stats_delta(int16_t x, struct stats *st)
{
	st->delta = x - st->mean;
}

static inline void update_stats_mean(int16_t x, struct stats *st)
{
	update_stats_delta(x, st);
	st->mean += st->delta / st->n;
}

static inline void update_stats_m2(int16_t x, struct stats *st)
{
	st->m2 += st->delta * (x - st->mean);
}

static inline void update_stats_var(struct stats *st)
{
	st->var = st->m2 / st->n;
}

static inline void update_stats_sd(struct stats *st)
{
	st->sd = sqrt(st->var);
}

void update_stats(int16_t x, struct stats *st)
{
	update_stats_n(st);
	update_stats_min(x, st);
	update_stats_max(x, st);
	update_stats_mean(x, st);
	update_stats_m2(x, st);
}

void update_stats_end(struct stats *st)
{
	update_stats_var(st);
	update_stats_sd(st);
}

void get_stats(const int16_t *sigs, uint64_t nsigs, struct stats *st)
{
	uint64_t i;

	init_stats(st);

	for (i = 0; i < nsigs; i++) {
		update_stats(sigs[i], st);
	}

	update_stats_end(st);
}

void print_stats(const struct stats *st)
{
	printf("n: %" PRIu64 "\n"
		"min: %" PRIu16 "\n"
		"max: %" PRIu16 "\n"
		"mean: %f\n"
		"var: %f\n"
		"sd: %f\n",
		st->n,
		st->min,
		st->max,
		st->mean,
		st->var,
		st->sd);
}
