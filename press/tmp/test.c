#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

int test(const int16_t *sigs,
	 const uint64_t nr_sigs,
	 uint64_t (*bound)(const int16_t *, uint64_t),
	 uint64_t (*press)(const int16_t *, uint64_t, uint8_t *),
	 uint64_t (*depress)(const uint8_t *, uint64_t, int16_t *))
{
	uint64_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_bound;
	uint64_t press_len;
	uint64_t depress_len;
	uint8_t *sigs_press;
	int16_t *sigs_depress;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = bound(sigs, nr_sigs);
	ASSERT(press_bound <= nr_sigs_bytes);

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press(sigs, nr_sigs, sigs_press);
	ASSERT(press_len <= press_bound);
	fprintf(stderr, "press ratio:\t%f\n",
			(float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = depress(sigs_press, press_len, sigs_depress);
	/*fprintf(stderr, "depress_len: %" PRIu64 "\n", depress_len);
	fprintf(stderr, "nr_sigs: %" PRIu64 "\n", nr_sigs);*/
	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		/*
		char *buf;
		fprintf(stderr, "i: %" PRIu64 "\n", i);
		buf = int16_t_to_bin(sigs_depress[i]);
		fprintf(stderr, "depress: %" PRIu16 "\t(%s)\n", sigs_depress[i], buf);
		free(buf);
		buf = int16_t_to_bin(sigs[i]);
		fprintf(stderr, "sig: %" PRIu16 "\t(%s)\n", sigs[i], buf);
		free(buf);
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
	TEST(P11, none_bound, none_press, none_depress);
	TEST(P11, uint11_bound, uint11_press, uint11_depress);
	TEST(P11, uint_bound, uint_press, uint_depress);
	return 0;
}
