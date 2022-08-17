#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

int test(const int16_t *sigs,
	 const uint32_t nr_sigs,
	 uint32_t (*bound)(const int16_t *, uint32_t),
	 uint32_t (*press)(const int16_t *, uint32_t, uint8_t *),
	 uint32_t (*depress)(const uint8_t *, uint32_t, int16_t *))
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
	press_bound = bound(sigs, nr_sigs);
	printf("press bound:\t%" PRIu32 "\n", press_bound);
	/*ASSERT(press_bound <= nr_sigs_bytes);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press(sigs, nr_sigs, sigs_press);
	printf("press len:\t%" PRIu32 "\n", press_len);
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = depress(sigs_press, nr_sigs, sigs_depress);
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
	/*
	TEST(P11, "none", none_bound, none_press, none_depress);
	TEST(P11, "uint11", uint11_bound, uint11_press, uint11_depress);
	TEST(P11, "uintx", uint_bound, uint_press, uint_depress);
	TEST(P11, "uintx subtract min", uint_submin_bound, uint_submin_press, uint_submin_depress);
	TEST(P11, "uintx zigzag delta", uint_zd_bound, uint_zd_press, uint_zd_depress);
	TEST(P11, "uintx zigzag subtract mean", uint_zsubmean_bound, uint_zsubmean_press, uint_zsubmean_depress);
	TEST(P11, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);
	*/

	/*
	TEST(P11_SHORT, "none", none_bound, none_press, none_depress);
	TEST(P11_SHORT, "uint11", uint11_bound, uint11_press, uint11_depress);
	TEST(P11_SHORT, "uintx", uint_bound, uint_press, uint_depress);
	TEST(P11_SHORT, "uintx subtract min", uint_submin_bound, uint_submin_press, uint_submin_depress);
	TEST(P11_SHORT, "uintx zigzag delta", uint_zd_bound, uint_zd_press, uint_zd_depress);
	TEST(P11_SHORT, "uintx zigzag subtract mean", uint_zsubmean_bound, uint_zsubmean_press, uint_zsubmean_depress);
	*/

	/*
	TEST(P11_SHORT, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);
	TEST(P11_MEDIUM, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);
	*/
	TEST(P11_LONG, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);

	/*
	TEST(ONE, "none", none_bound, none_press, none_depress);
	TEST(ONE, "uint11", uint11_bound, uint11_press, uint11_depress);
	TEST(ONE, "uintx", uint_bound, uint_press, uint_depress);
	TEST(ONE, "uintx subtract min", uint_submin_bound, uint_submin_press, uint_submin_depress);
	TEST(ONE, "uintx zigzag delta", uint_zd_bound, uint_zd_press, uint_zd_depress);
	TEST(ONE, "uintx zigzag subtract mean", uint_zsubmean_bound, uint_zsubmean_press, uint_zsubmean_depress);
	TEST(ONE, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);

	TEST(SAME, "none", none_bound, none_press, none_depress);
	TEST(SAME, "uint11", uint11_bound, uint11_press, uint11_depress);
	TEST(SAME, "uintx", uint_bound, uint_press, uint_depress);
	TEST(SAME, "uintx subtract min", uint_submin_bound, uint_submin_press, uint_submin_depress);
	TEST(SAME, "uintx zigzag delta", uint_zd_bound, uint_zd_press, uint_zd_depress);
	TEST(SAME, "uintx zigzag subtract mean", uint_zsubmean_bound, uint_zsubmean_press, uint_zsubmean_depress);
	TEST(SAME, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);

	TEST(ZERO, "none", none_bound, none_press, none_depress);
	TEST(ZERO, "uint11", uint11_bound, uint11_press, uint11_depress);
	TEST(ZERO, "uintx", uint_bound, uint_press, uint_depress);
	TEST(ZERO, "uintx subtract min", uint_submin_bound, uint_submin_press, uint_submin_depress);
	TEST(ZERO, "uintx zigzag delta", uint_zd_bound, uint_zd_press, uint_zd_depress);
	TEST(ZERO, "uintx zigzag subtract mean", uint_zsubmean_bound, uint_zsubmean_press, uint_zsubmean_depress);
	TEST(ZERO, "flat uintx subtract min", flat_uint_submin_bound, flat_uint_submin_press, flat_uint_submin_depress);
	*/

	return 0;
}
