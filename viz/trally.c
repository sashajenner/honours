/*
 * find the transition probabilities of numbers in a column
 * ignore the first line
 * cc trally.c getsig.c tally.c -o trally
 * ./trally DATA_FILE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "getsig.h"
#include "tally.h"

#define USAGE ("usage: %s DATA_FILE\n")

int main(int argc, char **argv)
{
	uint64_t *trally;
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

	trally = gettrally(fp, &min, &max);
	if (!trally) {
		perror(NULL);
		return 1;
	}

	printtrally(trally, min, max);

	free(trally);
	return 0;
}
