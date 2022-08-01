#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <slow5/slow5.h>
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

static inline void update_stats_n(struct slow5_rec *rec,
				  struct stats *st)
{
	st->n = rec->len_raw_signal;
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

static inline void update_stats_var(int16_t x, struct stats *st)
{
	update_stats_m2(x, st);
	st->var = st->m2 / st->n;
}

static inline void update_stats_sd(struct stats *st)
{
	st->sd = sqrt(st->var);
}

void update_stats(int16_t x, struct stats *st)
{
	update_stats_min(x, st);
	update_stats_max(x, st);
	update_stats_mean(x, st);
	update_stats_var(x, st);
}

void update_stats_start(struct slow5_rec *rec, struct stats *st)
{
	update_stats_n(rec, st);
}

void update_stats_end(struct stats *st)
{
	update_stats_sd(st);
}

void print_hdr_stats(void)
{
	puts("n\t"
		"min\t"
		"max\t"
		"mean\t"
		"var\t"
		"sd\n");
}

void print_stats(const struct stats *st)
{
	printf("%zu\t" /* n */
		"%d\t" /* min */
		"%d\t" /* max */
		"%f\t" /* mean */
		"%f\t" /* var */
		"%f\n", /* sd */
		st->n,
		st->min,
		st->max,
		st->mean,
		st->var,
		st->sd);
}
