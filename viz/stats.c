#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <slow5/slow5.h>
#include "stats.h"

#define TO_PICOAMPS(SIGNAL, DIG, OFFSET, RANGE) (((SIGNAL)+(OFFSET))*((RANGE)/(DIG)))
#define VAR_TO_PICOAMPS(VAR, RANGE, DIG) ((VAR) * pow((RANGE)/(DIG), 2))

void init_stats(struct stats *st)
{
	st->id = NULL;
	st->n = 0;
	st->dig = 0;
	st->offset = 0;
	st->range = 0;
	st->rate = 0;
	st->channel_num = 0;
	st->median_before = 0;
	st->read_num = 0;
	st->start_mux = 0;
	st->start_time = 0;
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
	st->sd_pa = sqrt(st->var_pa);
}

static inline void update_stats_pa(const struct slow5_rec *rec, struct stats *st)
{
	st->min_pa = TO_PICOAMPS(st->min, rec->digitisation, rec->offset, rec->range);
	st->max_pa = TO_PICOAMPS(st->max, rec->digitisation, rec->offset, rec->range);
	st->mean_pa = TO_PICOAMPS(st->mean, rec->digitisation, rec->offset, rec->range);
	st->var_pa = VAR_TO_PICOAMPS(st->var, rec->range, rec->digitisation);
}

static inline void update_stats_id(const struct slow5_rec *rec, struct stats *st)
{
	st->id = rec->read_id;
}

static inline void update_stats_dig(const struct slow5_rec *rec, struct stats *st)
{
	st->dig = rec->digitisation;
}

static inline void update_stats_offset(const struct slow5_rec *rec, struct stats *st)
{
	st->offset = rec->offset;
}

static inline void update_stats_range(const struct slow5_rec *rec, struct stats *st)
{
	st->range = rec->range;
}

static inline void update_stats_rate(const struct slow5_rec *rec, struct stats *st)
{
	st->rate = rec->sampling_rate;
}

static inline void update_stats_channel_num(const struct slow5_rec *rec, struct stats *st)
{
	st->channel_num = slow5_aux_get_string(rec, "channel_number", NULL, NULL);
}

static inline void update_stats_median_before(const struct slow5_rec *rec, struct stats *st)
{
	st->median_before = slow5_aux_get_double(rec, "median_before", NULL);
}

static inline void update_stats_read_num(const struct slow5_rec *rec, struct stats *st)
{
	st->read_num = slow5_aux_get_int32(rec, "read_number", NULL);
}

static inline void update_stats_start_mux(const struct slow5_rec *rec, struct stats *st)
{
	st->start_mux = slow5_aux_get_uint8(rec, "start_mux", NULL);
}

static inline void update_stats_start_time(const struct slow5_rec *rec, struct stats *st)
{
	st->start_time = slow5_aux_get_uint64(rec, "start_time", NULL);
}

void update_stats_start(const struct slow5_rec *rec, struct stats *st)
{
	update_stats_id(rec, st);
	update_stats_dig(rec, st);
	update_stats_offset(rec, st);
	update_stats_range(rec, st);
	update_stats_rate(rec, st);
	update_stats_channel_num(rec, st);
	update_stats_median_before(rec, st);
	update_stats_read_num(rec, st);
	update_stats_start_mux(rec, st);
	update_stats_start_time(rec, st);
}

void update_stats(int16_t x, struct stats *st)
{
	update_stats_n(st);
	update_stats_min(x, st);
	update_stats_max(x, st);
	update_stats_mean(x, st);
	update_stats_m2(x, st);
}

void update_stats_end(const struct slow5_rec *rec, struct stats *st)
{
	update_stats_var(st);
	update_stats_pa(rec, st);
	update_stats_sd(st);
}

void print_hdr_stats(void)
{
	puts("id\t"
		"n\t"
		"min\t"
		"max\t"
		"mean\t"
		"var\t"
		"sd\t"
		"min_pa\t"
		"max_pa\t"
		"mean_pa\t"
		"var_pa\t"
		"sd_pa\t"
		"dig\t"
		"offset\t"
		"range\t"
		"rate\t"
		"channel_num\t"
		"median_before\t"
		"read_num\t"
		"start_mux\t"
		"start_time");
}

void print_stats(const struct stats *st)
{
	printf("%s\t" /* id */
		"%zu\t" /* n */
		"%d\t" /* min */
		"%d\t" /* max */
		"%f\t" /* mean */
		"%f\t" /* var */
		"%f\t" /* sd */
		"%f\t" /* min_pa */
		"%f\t" /* max_pa */
		"%f\t" /* mean_pa */
		"%f\t" /* var_pa */
		"%f\t" /* sd_pa */
		"%f\t" /* dig */
		"%f\t" /* offset */
		"%f\t" /* range */
		"%f\t" /* rate */
		"%s\t" /* channel_num */
		"%f\t" /* median_before */
		"%" PRId32 "\t" /* read_num */
		"%" PRIu8 "\t" /* start_mux */
		"%" PRIu64 "\n", /* start_time */
		st->id,
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
		st->sd_pa,
		st->dig,
		st->offset,
		st->range,
		st->rate,
		st->channel_num,
		st->median_before,
		st->read_num,
		st->start_mux,
		st->start_time);
}
