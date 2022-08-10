#ifndef STATS_H
#define STATS_H

#include <stdint.h>

struct stats {
	uint64_t n;

	int16_t min;
	int16_t max;
	/* int16_t median; */ /* TODO */
	/* int16_t mode; */ /* TODO */

	double delta; /* difference between current and mean */
	double mean;

	double m2;
	double var;
	double sd;
};

void init_stats(struct stats *st);
void update_stats(int16_t x, struct stats *st);
void update_stats_end(struct stats *st);
void get_stats(const int16_t *sigs, uint64_t nsigs, struct stats *st);
void print_stats(const struct stats *st);

#endif /* stat.h */
