/*
 * find the frequencies of even and odd numbers in a column
 * ignore the first line
 * cc parity.c getsig.c tally.c -o parity
 * ./parity DATA_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "getsig.h"
#include "tally.h"

#define USAGE ("usage: %s DATA_FILE\n")

int main(int argc, char **argv)
{
	uint64_t *tally;
	uint16_t min;
	uint16_t max;
	FILE *fp;

	if (argc != 2) {
		printf(USAGE, argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		perror(NULL);
		return 1;
	}

	skipline(fp);

	tally = gettally(fp, &min, &max);
	if (!tally) {
		perror(NULL);
		return 1;
	}

	printparity(tally, min, max);

	free(tally);
	return 0;
}
