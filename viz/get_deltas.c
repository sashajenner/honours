/*
 * get the deltas between numbers in a column with a header
 * output as one column with 'raw_signal_delta' as the header
 * cc get_deltas.c getsig.c -o get_deltas
 * ./get_deltas DATA_FILE
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "getsig.h"

#define USAGE ("usage: %s DATA_FILE\n")

int main(int argc, char **argv)
{
	int16_t cur;
	int16_t prev;
	int16_t diff;
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
	puts("raw_signal_delta");

	n = MAX_WIDTH_LINE;
	line = malloc(n);

	ret = getnextsig(fp, line, n, &prev);
	if (ret == 0)
		ret = getnextsig(fp, line, n, &cur);

	while (ret == 0) {
		diff = cur - prev;

		printf("%" PRId16 "\n", diff);

		prev = cur;
		ret = getnextsig(fp, line, n, &cur);
	}

	fclose(fp);
	free(line);
	return 0;
}
