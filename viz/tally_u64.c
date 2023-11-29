#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tally.h"
#include "tally_u64.h"

uint64_t *gettally_readlen_slow5(struct slow5_file *fp, uint64_t *min,
				 uint64_t *max)
{
	uint64_t *tally;
	struct slow5_rec *rec;
	uint64_t x;
	int ret;
	uint64_t i;

	rec = NULL;
	*min = UINT64_MAX;
	*max = 0;
	tally = NULL;

	ret = slow5_get_next(&rec, fp);

	while (ret >= 0) {
		x = rec->len_raw_signal;
		if (x < *min)
			*min = x;
		if (x > *max) {
			*max = x;
			/* +1 to accomodate for 0 */
			tally = realloc(tally, (x + 1) * sizeof (*tally));
			if (!tally)
				return NULL;
		}
		tally[x] ++;

		ret = slow5_get_next(&rec, fp);
	}

	slow5_rec_free(rec);

	if (ret != SLOW5_ERR_EOF) {
		free(tally);
		return NULL;
	}

	return tally;
}

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
