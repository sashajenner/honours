/*
 * get the deltas between numbers in a column with a header then zigzag that
 * output as one column with 'raw_signal_zd' as the header
 * cc get_zd.c getsig.c -o get_zd
 * ./get_zd DATA_FILE
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "getsig.h"

#define USAGE ("usage: %s DATA_FILE\n")

static inline uint16_t zigzag(int16_t x)
{
	return (x + x) ^ (x >> 15);
}

int main(int argc, char **argv)
{
	int16_t cur;
	int16_t prev;
	int16_t diff;
	uint16_t zd;
	FILE *fp;
	int ret;
	size_t n;
	char *line;

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
	puts("raw_signal_zd");

	n = MAX_WIDTH_LINE;
	line = malloc(n);

	ret = getnextsig(fp, line, n, &prev);
	if (ret == 0)
		ret = getnextsig(fp, line, n, &cur);

	while (ret == 0) {
		diff = cur - prev;
		zd = zigzag(diff);

		printf("%" PRIu16 "\n", zd);

		prev = cur;
		ret = getnextsig(fp, line, n, &cur);
	}

	fclose(fp);
	free(line);
	return 0;
}
