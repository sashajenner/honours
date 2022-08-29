#include <stdint.h>
#include "util.h"

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
