#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tally.h"
#include "getsig.h"

#define TALLY_SZ (65536) /* 2^16 */

uint64_t *gettally(FILE *fp, uint16_t *min, uint16_t *max)
{
	uint64_t *tally;
	uint16_t x;
	int ret;
	size_t n;
	char *line;

	*min = UINT16_MAX;
	*max = 0;

	tally = calloc(TALLY_SZ, sizeof (*tally));
	if (!tally)
		return NULL;

	n = MAX_WIDTH_LINE;
	line = malloc(n);
	if (!line) {
		free(tally);
		return NULL;
	}

	while ((ret = getnextsig(fp, line, n, &x)) == 0) {
		tally[x] ++;
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

void printtally(const uint64_t *tally, uint16_t min, uint16_t max)
{
	PRINTHDR("freq");

	int i;

	for (i = min; i <= max; ++i) {
		printf("%d" SEP "%zu\n", i, tally[i]);
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

	PRINTHDR("prob");

	for (i = min; i <= max; ++i) {
		prob = ((double) tally[i]) / tot;
		printf("%d" SEP "%f\n", i, prob);
	}
}
