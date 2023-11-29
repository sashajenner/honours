#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "getsig.h"

/* expects line = "[NUM]\n\0" */
int getsig(char *line, int16_t *sig)
{
	char *endptr;

	*sig = strtol(line, &endptr, 10);
	if (endptr[0] != '\n') {
		/*perror("parsing failed");*/
		return 1;
	}

	return 0;
}

int getnextsig(FILE *fp, char *line, size_t n, int16_t *x)
{
	if (getline(&line, &n, fp) == -1)
		return 1;

	if (getsig(line, x) != 0)
		return 2;

	return 0;
}

void skipline(FILE *fp)
{
	char *line;
	size_t n;

	line = NULL;
	n = 0;

	getline(&line, &n, fp);

	free(line);
}
