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
