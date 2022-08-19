#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

int test(const int16_t *sigs,
	 const uint32_t nr_sigs,
	 struct press_method method)
{
	uint32_t i;
	uint32_t nr_sigs_bytes;
	uint32_t press_bound;
	uint32_t press_len;
	uint32_t depress_len;
	uint8_t *sigs_press;
	int16_t *sigs_depress;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = method.bound(sigs, nr_sigs);
	printf("press bound:\t%" PRIu32 "\n", press_bound);
	/*ASSERT(press_bound <= nr_sigs_bytes);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = method.press(sigs, nr_sigs, sigs_press, press_bound);
	printf("press len:\t%" PRIu32 "\n", press_len);
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = method.depress(sigs_press, nr_sigs, press_len, sigs_depress);
	/*
	fprintf(stderr, "depress_len:\t%" PRIu32 "\n", depress_len);
	fprintf(stderr, "nr_sigs:\t%" PRIu32 "\n", nr_sigs);
	*/
	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
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
	TEST(P11, none_method);
	TEST(P11, uint11_method);
	TEST(P11, uint_method);
	TEST(P11, uint_submin_method);
	TEST(P11, uint_zd_method);
	TEST(P11, uint_zsubmean_method);
	//TEST(P11, flat_uint_submin_method);
	TEST(P11, zlib_method);

	TEST(P11_SHORT, none_method);
	TEST(P11_SHORT, uint11_method);
	TEST(P11_SHORT, uint_method);
	TEST(P11_SHORT, uint_submin_method);
	TEST(P11_SHORT, uint_zd_method);
	TEST(P11_SHORT, uint_zsubmean_method);
	TEST(P11_SHORT, flat_uint_submin_method);
	TEST(P11_SHORT, zlib_method);

	TEST(P11_MEDIUM, none_method);
	TEST(P11_MEDIUM, uint11_method);
	TEST(P11_MEDIUM, uint_method);
	TEST(P11_MEDIUM, uint_submin_method);
	TEST(P11_MEDIUM, uint_zd_method);
	TEST(P11_MEDIUM, uint_zsubmean_method);
	//TEST(P11_MEDIUM, flat_uint_submin_method);
	TEST(P11_MEDIUM, zlib_method);

	TEST(P11_LONG, none_method);
	TEST(P11_LONG, uint11_method);
	TEST(P11_LONG, uint_method);
	TEST(P11_LONG, uint_submin_method);
	TEST(P11_LONG, uint_zd_method);
	TEST(P11_LONG, uint_zsubmean_method);
	//TEST(P11_LONG, flat_uint_submin_method);
	TEST(P11_LONG, zlib_method);

	TEST(ONE, none_method);
	TEST(ONE, uint11_method);
	TEST(ONE, uint_method);
	TEST(ONE, uint_submin_method);
	TEST(ONE, uint_zd_method);
	TEST(ONE, uint_zsubmean_method);
	TEST(ONE, flat_uint_submin_method);
	TEST(ONE, zlib_method);

	TEST(SAME, none_method);
	TEST(SAME, uint11_method);
	TEST(SAME, uint_method);
	TEST(SAME, uint_submin_method);
	TEST(SAME, uint_zd_method);
	TEST(SAME, uint_zsubmean_method);
	TEST(SAME, flat_uint_submin_method);
	TEST(SAME, zlib_method);

	TEST(ZERO, none_method);
	TEST(ZERO, uint11_method);
	TEST(ZERO, uint_method);
	TEST(ZERO, uint_submin_method);
	TEST(ZERO, uint_zd_method);
	TEST(ZERO, uint_zsubmean_method);
	TEST(ZERO, flat_uint_submin_method);
	TEST(ZERO, zlib_method);

	return 0;
}
