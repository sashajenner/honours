#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

int test_none(const int16_t *sigs, const uint32_t nr_sigs)
{
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint32_t press_bound;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t nr_sigs_depress;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = none_bound(nr_sigs_bytes);
	ASSERT(press_bound == nr_sigs_bytes);

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	ret = none_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	ASSERT(ret == 0);

	ASSERT(press_len == press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	ret = none_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	ASSERT(ret == 0);

	nr_sigs_depress = depress_len / sizeof *sigs;
	ASSERT(nr_sigs_depress == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs_depress; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	return EXIT_SUCCESS;
}

int test_uintx_16(uint8_t bits_out, const int16_t *sigs,
		  const uint32_t nr_sigs)
{
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t nr_sigs_depress;
	uint64_t press_bound;
	uint64_t press_len;
	uint8_t *sigs_press;

	printf("bits out:\t%" PRIu8 "\n", bits_out);

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = uintx_bound_16(bits_out, nr_sigs_bytes);
	/*printf("press bound:\t%" PRIu32 "\n", press_bound);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	ret = uintx_press_16(bits_out, (const uint8_t *) sigs, nr_sigs,
			     sigs_press, &press_len);
	ASSERT(ret == 0);

	/*printf("press len:\t%" PRIu64 "\n", press_len);*/
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	ret = uintx_depress_16(bits_out, sigs_press, nr_sigs,
			       (uint8_t *) sigs_depress, &depress_len);
	ASSERT(ret == 0);

	nr_sigs_depress = depress_len / sizeof *sigs;
	ASSERT(nr_sigs_depress == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs_depress; i++) {
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

int test_uint_16(const int16_t *sigs, const uint32_t nr_sigs)
{
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_bound;
	uint64_t press_len;
	uint8_t *sigs_press;
	uint8_t minbits;

	printf("num sigs:\t%" PRIu32 "\n", nr_sigs);

	minbits = uint_get_minbits_16((const uint16_t *) sigs, nr_sigs);
	printf("min bits out:\t%" PRIu8 "\n", minbits);

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = uint_bound_16(minbits, nr_sigs);
	/*printf("press bound:\t%" PRIu32 "\n", press_bound);*/

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	ret = uint_press_16(minbits, (const uint16_t *) sigs, nr_sigs,
			    sigs_press, &press_len);
	ASSERT(ret == 0);

	/*printf("press len:\t%" PRIu64 "\n", press_len);*/
	ASSERT(press_len <= press_bound);
	printf("press ratio:\t%f\n", (float) nr_sigs_bytes / press_len);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	ret = uint_depress_16(sigs_press, nr_sigs, (uint16_t *) sigs_depress,
			      &depress_len);
	ASSERT(ret == 0);

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
	/* TODO test array > 0 */
	ASSERT(test_none(P11, LENGTH(P11)) == EXIT_SUCCESS);
	ASSERT(test_uintx_16(11, P11, LENGTH(P11)) == EXIT_SUCCESS);
	ASSERT(test_uint_16(P11, LENGTH(P11)) == EXIT_SUCCESS);
}
