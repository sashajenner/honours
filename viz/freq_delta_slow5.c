/*
 * print the frequencies of the differences between the signal raw values
 * in a TSV format
 * given (s|b)low5 file
 * e.g.
 *
 * signal	freq
 * -10	7
 * -9	13
 * -6	44
 * ...
 *
 * cc freq_delta_slow5.c getsig.c tally.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o freq_delta_slow5
 * ./freq_delta_slow5 (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "tally.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

uint64_t *gettally_delta_slow5(struct slow5_file *fp, int16_t *min,
			       int16_t *max)
{
	uint64_t *tally;
	struct slow5_rec *rec;
	int16_t x;
	int16_t cur;
	int16_t prev;
	int ret;
	uint64_t i;

	rec = NULL;
	*min = INT16_MAX;
	*max = INT16_MIN;

	tally = calloc(TALLY_SZ_EXP, sizeof (*tally));
	if (!tally)
		return NULL;

	ret = slow5_get_next(&rec, fp);

	while (ret >= 0) {
		if (rec->len_raw_signal > 1) {
			prev = rec->raw_signal[0];
			for (i = 1; i < rec->len_raw_signal; i++) {
				cur = rec->raw_signal[i];
				x = cur - prev;
				updatetally(tally, x, min, max);
				prev = cur;
			}
		}

		ret = slow5_get_next(&rec, fp);
	}

	slow5_rec_free(rec);

	if (ret != SLOW5_ERR_EOF) {
		free(tally);
		return NULL;
	}

	return tally;
}


void printtally_slow5(struct slow5_file *fp)
{
	uint64_t *tally;
	uint16_t min;
	uint16_t max;

	tally = gettally_delta_slow5(fp, &min, &max);
	printtally(tally, min, max);

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
	printtally_slow5(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
