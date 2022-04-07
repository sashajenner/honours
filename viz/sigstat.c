/*
 * get stats about numbers in a column
 * ignore the first line
 * cc sigstat.c -lm -o sigstat
 * ./sigstat DATA_FILE
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define USAGE ("usage: %s DATA_FILE")
#define MAX_WIDTH_INT16 (6)

struct stats {
	size_t n;
	int16_t min;
	int16_t max;
	int16_t median;
	int16_t mode;
	double delta;
	double mean;
	double m2;
	double var;
	double sd;
};

static inline void updatestatsn(struct stats *st)
{
	st->n ++;
}

static inline void updatestatsmin(int16_t x, struct stats *st)
{
	if (x < st->min)
		st->min = x;
}

static inline void updatestatsmax(int16_t x, struct stats *st)
{
	if (x > st->max)
		st->max = x;
}

static inline void updatestatsdelta(int16_t x, struct stats *st)
{
	st->delta = x - st->mean;
}

static inline void updatestatsmean(int16_t x, struct stats *st)
{
	updatestatsdelta(x, st);
	st->mean += st->delta / st->n;
}

static inline void updatestatsm2(int16_t x, struct stats *st)
{
	st->m2 += st->delta * (x - st->mean);
}

static inline void updatestatsvar(int16_t x, struct stats *st)
{
	updatestatsm2(x, st);
	st->var = st->m2 / st->n;
}

static inline void updatestatssd(struct stats *st)
{
	st->sd = sqrt(st->var);
}

void updatestats(int16_t x, struct stats *st)
{
	updatestatsn(st);
	updatestatsmin(x, st);
	updatestatsmax(x, st);
	updatestatsmean(x, st);
	updatestatsvar(x, st);
}

void updatestatsend(struct stats *st)
{
	updatestatssd(st);
}

void initstats(struct stats *st)
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

void printstats(const struct stats *st)
{
	printf("n: %zu\n"
		"min: %d\n"
		"max: %d\n"
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

/* expects line = "[NUM]\n\0" */
int getsig(char *line, size_t n, int16_t *sig)
{
	char *endptr;

	*sig = strtol(line, &endptr, 10);
	if (endptr[0] != '\n') {
		perror("parsing failed");
		return 1;
	}

	return 0;
}

int sigstat(FILE *fp)
{
	char *line;
	size_t n;
	int16_t sig;
	int ret;
	struct stats sigstats;

	n = MAX_WIDTH_INT16 + 2; /* \n + \0 */
	line = malloc(n);
	if (!line)
		return 1;

	initstats(&sigstats);

	while (getline(&line, &n, fp) != -1) {
		ret = getsig(line, n, &sig);
		if (ret)
			return 1;
		updatestats(sig, &sigstats);
	}
	updatestatsend(&sigstats);

	printstats(&sigstats);

	free(line);
	return 0;
}

int main(int argc, char **argv)
{
	FILE *fp;
	int ret;
	char *line;
	size_t n;

	if (argc != 2) {
		printf(USAGE, argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		perror(NULL);
		return 1;
	}

	line = NULL;
	n = 0;
	ret = getline(&line, &n, fp);
	if (ret == -1)
		perror("empty file");
	free(line);

	ret = sigstat(fp);
	if (ret)
		perror("sigstat failed");

	ret = fclose(fp);
	if (ret == EOF) {
		perror(NULL);
		return 1;
	}

	return 0;
}
