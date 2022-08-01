#ifndef STATS_H
#define STATS_H

struct stats {
	size_t n;

	int16_t min;
	int16_t max;
	int16_t median;
	/* int16_t mode; */ /* TODO */
	double min_pa; /* in picoamperes */
	double max_pa;
	double median_pa;

	double delta; /* difference between current and mean */
	double mean;
	double mean_pa;

	double m2;
	double var;
	double sd;
	double var_pa;
	double sd_pa;
};

void init_stats(struct stats *st);
void update_stats(int16_t x, struct stats *st);
void update_stats_start(const struct slow5_rec *rec, struct stats *st);
void update_stats_end(const struct slow5_rec *rec, struct stats *st);
void print_hdr_stats(void);
void print_stats(const struct stats *st);

#endif
