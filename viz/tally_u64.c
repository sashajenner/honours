#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "tally.h"
#include "tally_u64.h"

void printtally_u64(const uint64_t *tally, uint64_t min, uint64_t max)
{
	PRINTSTDHDR("freq");

	uint64_t i;
	uint64_t count;

	for (i = min; i <= max; ++i) {
		count = tally[i];
		if (count)
			printf("%" PRIu64 SEP "%" PRIu64 "\n", i, count);
	}
}
