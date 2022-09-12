#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "util.h"

#define UINT32_MAX_LENGTH (10)

uint16_t get_max_u16(const uint16_t *in, uint64_t nin)
{
	uint16_t max;
	uint64_t i;

	max = 0;

	for (i = 0; i < nin; i++) {
		if (in[i] > max)
			max = in[i];
	}

	return max;
}

void get_minmax_u16(const uint16_t *in, uint64_t nin, uint16_t *min,
		    uint16_t *max)
{
	uint16_t max_tmp;
	uint16_t min_tmp;
	uint64_t i;

	max_tmp = 0;
	min_tmp = UINT16_MAX;

	for (i = 0; i < nin; i++) {
		if (in[i] < min_tmp)
			min_tmp = in[i];
		if (in[i] > max_tmp)
			max_tmp = in[i];
	}

	*max = max_tmp;
	*min = min_tmp;
}

int16_t get_mean_16(const int16_t *in, uint64_t nin)
{
	double delta;
	double mean;
	uint64_t i;

	mean = 0;

	for (i = 0; i < nin; i++) {
		delta = in[i] - mean;
		mean += delta / (i + 1);
	}

	return (int16_t) mean;
}

char *array_to_str(const uint32_t *x, uint32_t n)
{
	char *str;
	int nout;
	uint32_t i;

	if (!n)
		return NULL;

	str = (char *) malloc(n * UINT32_MAX_LENGTH + 1);
	nout = 0;

	for (i = 0; i < n - 1; i++) {
		nout += sprintf(str + nout, "%" PRIu32 ",", x[i]);
	}
	sprintf(str + nout, "%" PRIu32, x[n - 1]);

	return str;
}
