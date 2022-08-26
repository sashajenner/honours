#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

int test(const int16_t *sigs,
	 const uint32_t nr_sigs)
{
	uint32_t i;
	uint32_t nr_sigs_bytes;
	uint32_t press_bound;
	uint64_t press_len;
	uint64_t depress_len;
	uint8_t *sigs_press;
	int16_t *sigs_depress;
	int ret;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = uintx_bound_16(11, nr_sigs_bytes);
	printf("press bound:\t%" PRIu32 "\n", press_bound);
	/*ASSERT(press_bound <= nr_sigs_bytes);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	ret = uintx_press_16(11, (const uint8_t *) sigs, nr_sigs_bytes,
			     sigs_press, &press_len);
	ASSERT(ret == 0);

	printf("press len:\t%" PRIu64 "\n", press_len);
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	ret = uintx_depress_16(11, sigs_press, press_len, (uint8_t *)
			       sigs_depress, &depress_len);
	ASSERT(ret == 0);
	fprintf(stderr, "depress len:\t%" PRIu64 "\n", depress_len);
	fprintf(stderr, "nr sigs:\t%" PRIu32 "\n", nr_sigs);
	ASSERT(depress_len / sizeof *sigs == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		/*
		if (sigs_depress[i] != sigs[i]) {
			char *buf;
			fprintf(stderr, "i: %" PRIu32 "\n", i);
			buf = int16_t_to_bin(sigs_depress[i]);
			fprintf(stderr, "depress: %" PRId16 "\t(%s)\n", sigs_depress[i], buf);
			free(buf);
			buf = int16_t_to_bin(sigs[i]);
			fprintf(stderr, "sig: %" PRId16 "\t(%s)\n", sigs[i], buf);
			free(buf);
		}
		*/
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	return EXIT_SUCCESS;
}

int main(void)
{
	return test(P11, LENGTH(P11));
}
