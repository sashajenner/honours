#include <stdint.h>
#include <math.h>
#include "util.h"

uint8_t get_uint_bound(int16_t min, int16_t max)
{
	uint8_t i;

	/* can't unsigned bound if min < 0 */
	if (min < 0)
		return 0;

	for (i = 0; i <= 16; i++) {
		if (max < pow(2, i))
			return i;
	}

	return 0;
}

