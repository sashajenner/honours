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
	/*printf("press bound:\t%" PRIu32 "\n", press_bound);*/
	/*ASSERT(press_bound <= nr_sigs_bytes);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = method.press(sigs, nr_sigs, sigs_press, press_bound);
	/*printf("press len:\t%" PRIu32 "\n", press_len);*/
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = method.depress(sigs_press, nr_sigs, press_len,
				     sigs_depress, nr_sigs_bytes);
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
	const struct press_method ALL_METHODS[] = {
		none_method,
		zlib_method,
		zstd_method,
		/*uint11_method,*/
		uint_method,
		uint_submin_method,
		zlib_uint_submin_method,
		zstd_uint_submin_method,
		uint_zd_method,
		zlib_uint_zd_method,
		zstd_uint_zd_method,
		uint_zsubmean_method,
		zlib_uint_zsubmean_method,
		flat_uint_submin_method,
		svb_method,
		svb_zd_method,
		zlib_svb_zd_method,
		zstd_svb_zd_method,
		svb0124_method,
		svb0124_zd_method,
		zlib_svb0124_zd_method,
		zstd_svb0124_zd_method,
	};

	const struct press_method NO_FLAT_METHODS[] = {
		none_method,
		zlib_method,
		zstd_method,
		/*uint11_method,*/
		uint_method,
		uint_submin_method,
		zlib_uint_submin_method,
		zstd_uint_submin_method,
		uint_zd_method,
		zlib_uint_zd_method,
		zstd_uint_zd_method,
		uint_zsubmean_method,
		zlib_uint_zsubmean_method,
		/*flat_uint_submin_method,*/
		svb_method,
		svb_zd_method,
		zlib_svb_zd_method,
		zstd_svb_zd_method,
		svb0124_method,
		svb0124_zd_method,
		zlib_svb0124_zd_method,
		zstd_svb0124_zd_method,
	};

	TEST_ALL(ONE);
	TEST_ALL(SAME);
	TEST_ALL(ZERO);

	TEST_ALL(P11_SHORT);
	TEST_FOR(P11_MEDIUM, NO_FLAT_METHODS);
	TEST_FOR(P11_LONG, NO_FLAT_METHODS);
	TEST_FOR(P11, NO_FLAT_METHODS);

	return 0;
}
