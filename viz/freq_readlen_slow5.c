/*
 * print the frequencies of signal read lengths in a TSV format
 * given (s|b)low5 file
 * e.g.
 *
 * readlen	freq
 * 500	3
 * 501	0
 * 502	10
 * ...
 *
 * cc freq_readlen_slow5.c tally_u64.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o freq_readlen_slow5
 * ./freq_readlen_slow5 (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "tally_u64.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

uint64_t *gettally_readlen_slow5(struct slow5_file *fp, uint64_t *min,
				 uint64_t *max)
{
	uint64_t *tally;
	struct slow5_rec *rec;
	uint64_t x;
	int ret;
	uint64_t i;

	rec = NULL;
	*min = UINT64_MAX;
	*max = 0;
	tally = NULL;

	ret = slow5_get_next(&rec, fp);

	while (ret >= 0) {
		x = rec->len_raw_signal;
		if (x < *min)
			*min = x;
		if (x > *max) {
			*max = x;
			/* +1 to accomodate for 0 */
			tally = realloc(tally, (x + 1) * sizeof (*tally));
			if (!tally)
				return NULL;
		}
		tally[x] ++;

		ret = slow5_get_next(&rec, fp);
	}

	slow5_rec_free(rec);

	if (ret != SLOW5_ERR_EOF) {
		free(tally);
		return NULL;
	}

	return tally;
}

void printtally_readlen_slow5(struct slow5_file *fp)
{
	uint64_t *tally;
	uint64_t min;
	uint64_t max;

	tally = gettally_readlen_slow5(fp, &min, &max);
	printtally_u64(tally, min, max);

	free(tally);
}

int main(int argc, char **argv)
{
	struct slow5_file *fp;
	uint16_t min;
	uint16_t max;

	if (argc != 2) {
		printf(USAGE, argv[0]);
		return 1;
	}

	/* open file */
	fp = slow5_open(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "error opening file\n");
		return 1;
	}

	/* do the work */
	printtally_readlen_slow5(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
