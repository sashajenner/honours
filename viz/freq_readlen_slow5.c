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
 * cc freq_readlen_slow5.c getsig.c tally_u64.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o freq_readlen_slow5
 * ./freq_readlen_slow5 (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "tally_u64.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

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
