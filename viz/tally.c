#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tally.h"
#include "getsig.h"

#define TALLY_SZ (65536) /* 2^16 */
#define TALLY_SZ_EXP (4096) /* 2^12 */

uint64_t *gettally(FILE *fp, int16_t *min, int16_t *max)
{
	uint64_t *tally;
	int16_t x;
	int ret;
	size_t n;
	char *line;

	*min = INT16_MAX;
	*max = INT16_MIN;

	tally = calloc(TALLY_SZ_EXP, sizeof (*tally));
	if (!tally)
		return NULL;

	n = MAX_WIDTH_LINE;
	line = malloc(n);
	if (!line) {
		free(tally);
		return NULL;
	}

	while ((ret = getnextsig(fp, line, n, &x)) == 0) {
		tally[(x << 1) ^ (x >> 15)] ++;
		if (x < *min)
			*min = x;
		if (x > *max)
			*max = x;
	}

	free(line);

	if (ret != 1) {
		free(tally);
		return NULL;
	}

	return tally;
}

uint64_t *gettrally(FILE *fp, uint16_t *min, uint16_t *max)
{
	uint64_t *trally;
	uint16_t prev;
	uint16_t x;
	int ret;
	size_t n;
	char *line;

	trally = calloc(TALLY_SZ_EXP * TALLY_SZ_EXP, sizeof (*trally));
	if (!trally)
		return NULL;

	n = MAX_WIDTH_LINE;
	line = malloc(n);
	if (!line) {
		free(trally);
		return NULL;
	}

	(void) getnextsig(fp, line, n, &prev);
	*min = prev;
	*max = prev;

	while ((ret = getnextsig(fp, line, n, &x)) == 0) {
		trally[prev * TALLY_SZ_EXP + x] ++;
		if (x < *min)
			*min = x;
		if (x > *max)
			*max = x;
		prev = x;
	}

	free(line);

	if (ret != 1) {
		free(trally);
		return NULL;
	}

	return trally;
}

void printtally(const uint64_t *tally, int16_t min, int16_t max)
{
	PRINTSTDHDR("freq");

	int i;

	for (i = min; i <= max; ++i) {
		printf("%d" SEP "%zu\n", i, tally[(i << 1) ^ (i >> 15)]);
	}
}

void printprob(const uint64_t *tally, uint16_t min, uint16_t max)
{
	int i;
	uint64_t tot;
	double prob;

	for (i = min, tot = 0; i <= max; ++i) {
		tot += tally[i];
	}

	PRINTSTDHDR("prob");

	for (i = min; i <= max; ++i) {
		prob = ((double) tally[i]) / tot;
		printf("%d" SEP "%f\n", i, prob);
	}
}

void printparity(const uint64_t *tally, uint16_t min, uint16_t max)
{
	int i;
	uint64_t even;
	uint64_t odd;

	for (i = min, even = 0, odd = 0; i <= max; ++i) {
		if (tally[i] % 2 == 0)
			even += tally[i];
		else
			odd += tally[i];
	}

	PRINTHDR2("parity", "freq");
	printf("even" SEP "%zu\n", even);
	printf("odd" SEP "%zu\n", odd);
}

void printtrallymat(const uint64_t *trally, uint16_t min, uint16_t max)
{
	printf("from/to" SEP);

	int i;
	int j;

	for (i = min; i < max; ++i) {
		printf("%d" SEP, i);
	}
	printf("%d\n", i);

	for (i = min; i <= max; ++i) {
		printf("%d" SEP, i);
		for (j = min; j < max; ++j) {
			printf("%zu" SEP, trally[i * TALLY_SZ_EXP + j]);
		}
		printf("%zu\n", trally[i * TALLY_SZ_EXP + j]);
	}
}

void printtrally(const uint64_t *trally, uint16_t min, uint16_t max)
{
	PRINTHDR3("from", "to", "freq");

	int i;
	int j;

	for (i = min; i <= max; ++i) {
		for (j = min; j <= max; ++j) {
			printf("%d" SEP "%d" SEP "%zu\n", i, j,
			       trally[i * TALLY_SZ_EXP + j]);
		}
	}
}

