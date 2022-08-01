#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <slow5/slow5.h>
#include "stats.h"

#define TO_PICOAMPS(SIGNAL, DIG, OFFSET, RANGE) (((SIGNAL)+(OFFSET))*((RANGE)/(DIG)))

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

static inline void update_stats_n(const struct slow5_rec *rec,
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

static inline void update_stats_pa(const struct slow5_rec *rec, struct stats *st)
{
	st->min_pa = TO_PICOAMPS(st->min, rec->digitisation, rec->offset, rec->range);
	st->max_pa = TO_PICOAMPS(st->max, rec->digitisation, rec->offset, rec->range);
	st->mean_pa = TO_PICOAMPS(st->mean, rec->digitisation, rec->offset, rec->range);
	st->var_pa = TO_PICOAMPS(st->var, rec->digitisation, rec->offset, rec->range);
	st->sd_pa = TO_PICOAMPS(st->sd, rec->digitisation, rec->offset, rec->range);
}

void update_stats(int16_t x, struct stats *st)
{
	update_stats_min(x, st);
	update_stats_max(x, st);
	update_stats_mean(x, st);
	update_stats_var(x, st);
}

void update_stats_start(const struct slow5_rec *rec, struct stats *st)
{
	update_stats_n(rec, st);
}

void update_stats_end(const struct slow5_rec *rec, struct stats *st)
{
	update_stats_sd(st);
	update_stats_pa(rec, st);
}

void print_hdr_stats(void)
{
	puts("n\t"
		"min\t"
		"max\t"
		"mean\t"
		"var\t"
		"sd\t"
		"min_pa\t"
		"max_pa\t"
		"mean_pa\t"
		"var_pa\t"
		"sd_pa");
}

void print_stats(const struct stats *st)
{
	printf("%zu\t" /* n */
		"%d\t" /* min */
		"%d\t" /* max */
		"%f\t" /* mean */
		"%f\t" /* var */
		"%f\t" /* sd */
		"%f\t" /* min_pa */
		"%f\t" /* max_pa */
		"%f\t" /* mean_pa */
		"%f\t" /* var_pa */
		"%f\n", /* sd_pa */
		st->n,
		st->min,
		st->max,
		st->mean,
		st->var,
		st->sd,
		st->min_pa,
		st->max_pa,
		st->mean_pa,
		st->var_pa,
		st->sd_pa);
}
