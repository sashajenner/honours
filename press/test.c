#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include "util.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

void init_res(struct result *res)
{
	memset(res, 0, sizeof *res);
}

void fwrite_res_hdr(FILE *fp)
{
	(void) fputs(RESULTS_HDR, fp);
}

void fwrite_res(FILE *fp, struct result *res)
{
	char *flats_str;

	flats_str = array_to_str(res->flats, res->nflats);

	(void) fprintf(fp, RESULTS_FORMAT,
		       res->method_name,
		       res->data_name,
		       res->pressbound_bytes,
		       res->press_bytes,
		       res->depress_bytes,
		       res->pressbound_clocktime,
		       res->press_clocktime,
		       res->depress_clocktime,
		       res->nflats,
		       flats_str);

	free(flats_str);
}

int test_none(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint32_t pressbound;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t nr_sigs_depress;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = none_bound(nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(pressbound == nr_sigs_bytes);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = none_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len == pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = none_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
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

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_uintx_16(uint8_t bits_out, const int16_t *sigs,
		  const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t nr_sigs_depress;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = uintx_bound_16(bits_out, nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uintx_press_16(bits_out, (const uint8_t *) sigs, nr_sigs,
			     sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = uintx_depress_16(bits_out, sigs_press, nr_sigs,
			       (uint8_t *) sigs_depress, &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
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

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_uint11_16(const int16_t *sigs, const uint32_t nr_sigs,
		   struct result *res)
{
	return test_uintx_16(11, sigs, nr_sigs, res);
}

int test_uint_16(const int16_t *sigs, const uint32_t nr_sigs,
		 struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = uint_get_minbits_16((const uint16_t *) sigs, nr_sigs);
	pressbound = uint_bound_16(minbits, nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	printf("min bits out:\t%" PRIu8 "\n", minbits);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_press_16(minbits, (const uint16_t *) sigs, nr_sigs,
			    sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_depress_16(sigs_press, nr_sigs, (uint16_t *) sigs_depress,
			      &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_uint_submin_16(const int16_t *sigs, const uint32_t nr_sigs,
			struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;
	uint8_t minbits;
	uint16_t min;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = uint_submin_get_minbits_16((const uint16_t *) sigs, nr_sigs,
					     &min);
	pressbound = uint_submin_bound_16(minbits, nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	printf("min bits out:\t%" PRIu8 "\n", minbits);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_submin_press_16(minbits, min, (const uint16_t *) sigs,
				   nr_sigs, sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_submin_depress_16(sigs_press, nr_sigs,
				     (uint16_t *) sigs_depress, &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_uint_zd_16(const int16_t *sigs, const uint32_t nr_sigs,
		    struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t *sigs_zd;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = uint_zd_get_minbits_16(sigs, nr_sigs, &sigs_zd);
	pressbound = uint_zd_bound_16(minbits, nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	printf("min bits out:\t%" PRIu8 "\n", minbits);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd, sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	free(sigs_zd);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				 &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_uint_zsm_16(const int16_t *sigs, const uint32_t nr_sigs,
		     struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	int16_t mean;
	uint16_t *sigs_zsm;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = uint_zsm_get_minbits_16(sigs, nr_sigs, &sigs_zsm, &mean);
	pressbound = uint_zsm_bound_16(minbits, nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	printf("min bits out:\t%" PRIu8 "\n", minbits);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_zsm_press_16(minbits, mean, nr_sigs, sigs_zsm, sigs_press,
				&press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	free(sigs_zsm);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_zsm_depress_16(sigs_press, nr_sigs, sigs_depress,
				  &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_flat_uint_submin_16(uint32_t step, const int16_t *sigs,
			     const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t *flats;
	uint32_t depress_len;
	uint32_t i;
	uint32_t nflats;
	uint32_t nr_sigs_bytes;
	uint32_t press_len;
	uint32_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = flat_uint_submin_bound_16(nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = flat_uint_submin_press_16(sigs, nr_sigs, step, sigs_press,
					&press_len, &flats, &nflats);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = flat_uint_submin_depress_16(sigs_press, press_len, sigs_depress,
					  &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	res->nflats = nflats;
	res->flats = flats;

	return EXIT_SUCCESS;
}

int test_flat_uint_submin_16_step1(const int16_t *sigs, const uint32_t nr_sigs,
				   struct result *res)
{
	return test_flat_uint_submin_16(1, sigs, nr_sigs, res);
}

int test_flat_uint_submin_16_step2(const int16_t *sigs, const uint32_t nr_sigs,
				   struct result *res)
{
	return test_flat_uint_submin_16(2, sigs, nr_sigs, res);
}

int test_flat_uint_submin_16_step50(const int16_t *sigs, const uint32_t nr_sigs,
				   struct result *res)
{
	return test_flat_uint_submin_16(50, sigs, nr_sigs, res);
}

int test_flat_uint_submin_16_step100(const int16_t *sigs, const uint32_t nr_sigs,
				   struct result *res)
{
	return test_flat_uint_submin_16(100, sigs, nr_sigs, res);
}

int test_zlib(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zlib_bound(nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = zlib_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_zstd(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_bound(nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = zstd_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_bzip2(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = bzip2_bound(nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = bzip2_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			  &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = bzip2_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			    &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_fast_lzma2(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = fast_lzma2_bound(nr_sigs_bytes);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = fast_lzma2_press((const uint8_t *) sigs, nr_sigs_bytes,
			       sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = fast_lzma2_depress(sigs_press, press_len,
				 (uint8_t *) sigs_depress, &depress_len);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_svb(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	uint32_t *sigs_depress_u32;
	uint32_t *sigs_u32;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	sigs_u32 = malloc(nr_sigs * sizeof *sigs_u32);
	ASSERT(sigs_u32);
	for (i = 0; i < nr_sigs; i++) {
		sigs_u32[i] = sigs[i];
	}

	/* bound sigs_press */
	before = clock();
	pressbound = svb_bound(nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb_press(sigs_u32, nr_sigs, sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress_u32 = malloc(nr_sigs * sizeof *sigs_depress_u32);
	ASSERT(sigs_depress_u32);

	/* decompress sigs_press */
	before = clock();
	svb_depress(sigs_press, nr_sigs, sigs_depress_u32);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_u32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_u32);
	free(sigs_press);
	free(sigs_depress_u32);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_svb0124(const int16_t *sigs, const uint32_t nr_sigs,
		 struct result *res)
{
	clock_t after;
	clock_t before;
	uint32_t *sigs_depress_u32;
	uint32_t *sigs_u32;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	sigs_u32 = malloc(nr_sigs * sizeof *sigs_u32);
	ASSERT(sigs_u32);
	for (i = 0; i < nr_sigs; i++) {
		sigs_u32[i] = sigs[i];
	}

	/* bound sigs_press */
	before = clock();
	pressbound = svb0124_bound(nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb0124_press(sigs_u32, nr_sigs, sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress_u32 = malloc(nr_sigs * sizeof *sigs_depress_u32);
	ASSERT(sigs_depress_u32);

	/* decompress sigs_press */
	before = clock();
	svb0124_depress(sigs_press, nr_sigs, sigs_depress_u32);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_u32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_u32);
	free(sigs_press);
	free(sigs_depress_u32);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int test_svb12(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	uint16_t *sigs_depress;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t pressbound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = svb12_bound(nr_sigs);
	after = clock();
	res->pressbound_clocktime = GET_CLOCK_SECS(before, after);

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb12_press((const uint16_t *) sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);

	ASSERT(press_len <= pressbound);

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs * sizeof *sigs_depress);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	before = clock();
	svb12_depress(sigs_press, nr_sigs, sigs_depress);
	after = clock();
	res->depress_clocktime = GET_CLOCK_SECS(before, after);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	res->depress_bytes = nr_sigs_bytes;
	res->pressbound_bytes = pressbound;
	res->press_bytes = press_len;

	return EXIT_SUCCESS;
}

int main(void)
{
	FILE *fp;
	struct result res;

	fp = fopen("test.out", "w");
	fwrite_res_hdr(fp);

	/* TODO test array > 0 */
	TEST(none, P11, &res, fp);
	TEST(uint11_16, P11, &res, fp);
	TEST(uint_16, P11, &res, fp);
	TEST(uint_submin_16, P11, &res, fp);
	TEST(uint_zd_16, P11, &res, fp);
	TEST(uint_zsm_16, P11, &res, fp);
	/*TEST(flat_uint_submin_16_step1, P11, &res, fp);*/
	TEST(zlib, P11, &res, fp);
	TEST(zstd, P11, &res, fp);
	TEST(bzip2, P11, &res, fp);
	TEST(fast_lzma2, P11, &res, fp);
	TEST(svb, P11, &res, fp);
	TEST(svb0124, P11, &res, fp);
	TEST(svb12, P11, &res, fp);

	TEST(none, P11_0_66999, &res, fp);
	TEST(uint11_16, P11_0_66999, &res, fp);
	TEST(uint_16, P11_0_66999, &res, fp);
	TEST(uint_submin_16, P11_0_66999, &res, fp);
	TEST(uint_zd_16, P11_0_66999, &res, fp);
	TEST(uint_zsm_16, P11_0_66999, &res, fp);
	/*TEST(flat_uint_submin_16_step1, P11_0_66999, &res, fp);*/
	TEST(flat_uint_submin_16_step100, P11_0_66999, &res, fp);
	TEST(zlib, P11_0_66999, &res, fp);
	TEST(zstd, P11_0_66999, &res, fp);
	TEST(bzip2, P11_0_66999, &res, fp);
	TEST(fast_lzma2, P11_0_66999, &res, fp);
	TEST(svb, P11_0_66999, &res, fp);
	TEST(svb0124, P11_0_66999, &res, fp);
	TEST(svb12, P11_0_66999, &res, fp);

	TEST(none, P11_0_28997, &res, fp);
	TEST(uint11_16, P11_0_28997, &res, fp);
	TEST(uint_16, P11_0_28997, &res, fp);
	TEST(uint_submin_16, P11_0_28997, &res, fp);
	TEST(uint_zd_16, P11_0_28997, &res, fp);
	TEST(uint_zsm_16, P11_0_28997, &res, fp);
	/*TEST(flat_uint_submin_16_step1, P11_0_28997, &res, fp);
	TEST(flat_uint_submin_16_step2, P11_0_28997, &res, fp);*/
	TEST(zlib, P11_0_28997, &res, fp);
	TEST(zstd, P11_0_28997, &res, fp);
	TEST(bzip2, P11_0_28997, &res, fp);
	TEST(fast_lzma2, P11_0_28997, &res, fp);
	TEST(svb, P11_0_28997, &res, fp);
	TEST(svb0124, P11_0_28997, &res, fp);
	TEST(svb12, P11_0_28997, &res, fp);

	TEST(none, P11_0_1995, &res, fp);
	TEST(uint11_16, P11_0_1995, &res, fp);
	TEST(uint_16, P11_0_1995, &res, fp);
	TEST(uint_submin_16, P11_0_1995, &res, fp);
	TEST(uint_zd_16, P11_0_1995, &res, fp);
	TEST(uint_zsm_16, P11_0_1995, &res, fp);
	TEST(flat_uint_submin_16_step1, P11_0_1995, &res, fp);
	TEST(zlib, P11_0_1995, &res, fp);
	TEST(zstd, P11_0_1995, &res, fp);
	TEST(bzip2, P11_0_1995, &res, fp);
	TEST(fast_lzma2, P11_0_1995, &res, fp);
	TEST(svb, P11_0_1995, &res, fp);
	TEST(svb0124, P11_0_1995, &res, fp);
	TEST(svb12, P11_0_1995, &res, fp);

	TEST(none, P11_0_285, &res, fp);
	TEST(uint11_16, P11_0_285, &res, fp);
	TEST(uint_16, P11_0_285, &res, fp);
	TEST(uint_submin_16, P11_0_285, &res, fp);
	TEST(uint_zd_16, P11_0_285, &res, fp);
	TEST(uint_zsm_16, P11_0_285, &res, fp);
	TEST(flat_uint_submin_16_step1, P11_0_285, &res, fp);
	TEST(zlib, P11_0_285, &res, fp);
	TEST(zstd, P11_0_285, &res, fp);
	TEST(bzip2, P11_0_285, &res, fp);
	TEST(fast_lzma2, P11_0_285, &res, fp);
	TEST(svb, P11_0_285, &res, fp);
	TEST(svb0124, P11_0_285, &res, fp);
	TEST(svb12, P11_0_285, &res, fp);

	TEST(none, P11_999_1999, &res, fp);
	TEST(uint11_16, P11_999_1999, &res, fp);
	TEST(uint_16, P11_999_1999, &res, fp);
	TEST(uint_submin_16, P11_999_1999, &res, fp);
	TEST(uint_zd_16, P11_999_1999, &res, fp);
	TEST(uint_zsm_16, P11_999_1999, &res, fp);
	TEST(flat_uint_submin_16_step1, P11_999_1999, &res, fp);
	TEST(flat_uint_submin_16_step2, P11_999_1999, &res, fp);
	TEST(flat_uint_submin_16_step50, P11_999_1999, &res, fp);
	TEST(flat_uint_submin_16_step100, P11_999_1999, &res, fp);
	TEST(zlib, P11_999_1999, &res, fp);
	TEST(zstd, P11_999_1999, &res, fp);
	TEST(bzip2, P11_999_1999, &res, fp);
	TEST(fast_lzma2, P11_999_1999, &res, fp);
	TEST(svb, P11_999_1999, &res, fp);
	TEST(svb0124, P11_999_1999, &res, fp);
	TEST(svb12, P11_999_1999, &res, fp);

	TEST(none, ONE, &res, fp);
	TEST(uint11_16, ONE, &res, fp);
	TEST(uint_16, ONE, &res, fp);
	TEST(uint_submin_16, ONE, &res, fp);
	TEST(uint_zd_16, ONE, &res, fp);
	TEST(uint_zsm_16, ONE, &res, fp);
	TEST(flat_uint_submin_16_step1, ONE, &res, fp);
	TEST(zlib, ONE, &res, fp);
	TEST(zstd, ONE, &res, fp);
	TEST(bzip2, ONE, &res, fp);
	TEST(fast_lzma2, ONE, &res, fp);
	TEST(svb, ONE, &res, fp);
	TEST(svb0124, ONE, &res, fp);
	TEST(svb12, ONE, &res, fp);

	TEST(none, SAME, &res, fp);
	TEST(uint11_16, SAME, &res, fp);
	TEST(uint_16, SAME, &res, fp);
	TEST(uint_submin_16, SAME, &res, fp);
	TEST(uint_zd_16, SAME, &res, fp);
	TEST(uint_zsm_16, SAME, &res, fp);
	TEST(flat_uint_submin_16_step1, SAME, &res, fp);
	TEST(zlib, SAME, &res, fp);
	TEST(zstd, SAME, &res, fp);
	TEST(bzip2, SAME, &res, fp);
	TEST(fast_lzma2, SAME, &res, fp);
	TEST(svb, SAME, &res, fp);
	TEST(svb0124, SAME, &res, fp);
	TEST(svb12, SAME, &res, fp);

	TEST(none, ZERO, &res, fp);
	TEST(uint11_16, ZERO, &res, fp);
	TEST(uint_16, ZERO, &res, fp);
	TEST(uint_submin_16, ZERO, &res, fp);
	TEST(uint_zd_16, ZERO, &res, fp);
	TEST(uint_zsm_16, ZERO, &res, fp);
	TEST(flat_uint_submin_16_step1, ZERO, &res, fp);
	TEST(zlib, ZERO, &res, fp);
	TEST(zstd, ZERO, &res, fp);
	TEST(bzip2, ZERO, &res, fp);
	TEST(fast_lzma2, ZERO, &res, fp);
	TEST(svb, ZERO, &res, fp);
	TEST(svb0124, ZERO, &res, fp);
	TEST(svb12, ZERO, &res, fp);

	(void) fclose(fp);

	return 0;
}
