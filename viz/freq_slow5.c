/*
 * print the frequencies of signal raw values in a TSV format
 * given (s|b)low5 file
 * e.g.
 *
 * signal	freq
 * 500	3
 * 501	0
 * 502	10
 * ...
 *
 * cc freq_slow5.c getsig.c tally.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o freq_slow5
 * ./freq (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "tally.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

void printtally_slow5(struct slow5_file *fp)
{
	uint64_t *tally;
	uint16_t min;
	uint16_t max;

	tally = gettally_slow5(fp, &min, &max);
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
