/*
 * test compression methods
 * print average results over (s|b)low5 file
 * given (s|b)low5 file
 * make
 * ./test (S|B)LOW5_FILE
 */

#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include "util.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */
#include <slow5/slow5.h>

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

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
	/*
	char *flats_str;

	flats_str = array_to_str(res->flats, res->nflats);
	free(res->flats);
	*/

	res->press_ratio = (double) res->depress_bytes / res->press_bytes;

	(void) fprintf(fp, RESULTS_FORMAT,
		       res->method_name,
		       res->pressbound_bytes,
		       res->press_bytes,
		       res->press_ratio,
		       res->depress_bytes,
		       res->pressbound_clocktime,
		       res->press_clocktime,
		       res->depress_clocktime);
		       /*res->nflats,
		       flats_str);*/

	/*free(flats_str);*/
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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));
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
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
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
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
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

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uintx_press_16(bits_out, (const uint8_t *) sigs, nr_sigs,
			     sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = uintx_depress_16(bits_out, sigs_press, nr_sigs,
			       (uint8_t *) sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
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

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_press_16(minbits, (const uint16_t *) sigs, nr_sigs,
			    sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_depress_16(sigs_press, nr_sigs, (uint16_t *) sigs_depress,
			      &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_submin_press_16(minbits, min, (const uint16_t *) sigs,
				   nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_submin_depress_16(sigs_press, nr_sigs,
				     (uint16_t *) sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zd);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				 &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = uint_zsm_press_16(minbits, mean, nr_sigs, sigs_zsm, sigs_press,
				&press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zsm);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = uint_zsm_depress_16(sigs_press, nr_sigs, sigs_depress,
				  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_uint_submin_16(const int16_t *sigs, const uint32_t nr_sigs,
			     struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t min;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = zlib_uint_submin_get_minbits_16((const uint16_t *) sigs,
						  nr_sigs, &min);
	pressbound = zlib_uint_submin_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_uint_submin_press_16(minbits, min, (const uint16_t *) sigs,
					nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_uint_submin_depress_16(sigs_press, press_len,
					  (uint16_t *) sigs_depress,
					  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_uint_zd_16(const int16_t *sigs, const uint32_t nr_sigs,
			 struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t *sigs_zd;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = zlib_uint_zd_get_minbits_16(sigs, nr_sigs, &sigs_zd);
	pressbound = zlib_uint_zd_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd,
				    sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zd);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_uint_zd_depress_16(sigs_press, press_len, sigs_depress,
				      &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_uint_submin_16(const int16_t *sigs, const uint32_t nr_sigs,
			     struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t min;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = zstd_uint_submin_get_minbits_16((const uint16_t *) sigs,
						  nr_sigs, &min);
	pressbound = zstd_uint_submin_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_uint_submin_press_16(minbits, min, (const uint16_t *) sigs,
					nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_uint_submin_depress_16(sigs_press, press_len,
					  (uint16_t *) sigs_depress,
					  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_uint_zd_16(const int16_t *sigs, const uint32_t nr_sigs,
			 struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t *sigs_zd;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = zstd_uint_zd_get_minbits_16(sigs, nr_sigs, &sigs_zd);
	pressbound = zstd_uint_zd_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd,
				    sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zd);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_uint_zd_depress_16(sigs_press, press_len, sigs_depress,
				      &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_bzip2_uint_zd_16(const int16_t *sigs, const uint32_t nr_sigs,
			  struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t *sigs_zd;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = bzip2_uint_zd_get_minbits_16(sigs, nr_sigs, &sigs_zd);
	pressbound = bzip2_uint_zd_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = bzip2_uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd,
				     sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zd);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = bzip2_uint_zd_depress_16(sigs_press, press_len, sigs_depress,
				       &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_fast_lzma2_uint_zd_16(const int16_t *sigs, const uint32_t nr_sigs,
			       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint16_t *sigs_zd;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	uint8_t minbits;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	minbits = fast_lzma2_uint_zd_get_minbits_16(sigs, nr_sigs, &sigs_zd);
	pressbound = fast_lzma2_uint_zd_bound_16(minbits, nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* TODO store this info */
	/*printf("min bits out:\t%" PRIu8 "\n", minbits);*/

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = fast_lzma2_uint_zd_press_16(minbits, sigs[0], nr_sigs, sigs_zd,
					  sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	free(sigs_zd);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = fast_lzma2_uint_zd_depress_16(sigs_press, press_len,
					    sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = flat_uint_submin_press_16(sigs, nr_sigs, step, sigs_press,
					&press_len, &flats, &nflats);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = flat_uint_submin_depress_16(sigs_press, press_len, sigs_depress,
					  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	/*UPDATE_RES(res, nflats, nflats);
	UPDATE_RES(res, flats, flats);*/

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = zlib_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = zstd_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			   &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = bzip2_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			  &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = bzip2_depress(sigs_press, press_len, (uint8_t *) sigs_depress,
			    &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = fast_lzma2_press((const uint8_t *) sigs, nr_sigs_bytes,
			       sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs_bytes;
	before = clock();
	ret = fast_lzma2_depress(sigs_press, press_len,
				 (uint8_t *) sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs_bytes);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb_press(sigs_u32, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress_u32 = malloc(nr_sigs * sizeof *sigs_depress_u32);
	ASSERT(sigs_depress_u32);

	/* decompress sigs_press */
	before = clock();
	svb_depress(sigs_press, nr_sigs, sigs_depress_u32);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_u32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_u32);
	free(sigs_press);
	free(sigs_depress_u32);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb0124_press(sigs_u32, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress_u32 = malloc(nr_sigs * sizeof *sigs_depress_u32);
	ASSERT(sigs_depress_u32);

	/* decompress sigs_press */
	before = clock();
	svb0124_depress(sigs_press, nr_sigs, sigs_depress_u32);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_u32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_u32);
	free(sigs_press);
	free(sigs_depress_u32);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

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
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb12_press((const uint16_t *) sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs * sizeof *sigs_depress);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	before = clock();
	svb12_depress(sigs_press, nr_sigs, sigs_depress);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_svb_zd(const int16_t *sigs, const uint32_t nr_sigs,
		struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = svb_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	depress_len = nr_sigs;
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	before = clock();
	svb_zd_depress_16(sigs_press, nr_sigs, sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_svb0124_zd(const int16_t *sigs, const uint32_t nr_sigs,
		    struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = svb0124_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb0124_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	depress_len = nr_sigs;
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	before = clock();
	svb0124_zd_depress_16(sigs_press, nr_sigs, sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_svb12_zd(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = svb12_zd_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	svb12_zd_press(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	depress_len = nr_sigs;
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	before = clock();
	svb12_zd_depress(sigs_press, nr_sigs, sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_svb_zd(const int16_t *sigs, const uint32_t nr_sigs,
		     struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zlib_svb_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_svb_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_svb_zd_depress_16(sigs_press, press_len, sigs_depress,
				     &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_svb0124_zd(const int16_t *sigs, const uint32_t nr_sigs,
			 struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zlib_svb0124_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_svb0124_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_svb0124_zd_depress_16(sigs_press, press_len, sigs_depress,
					 &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_svb12_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zlib_svb12_zd_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_svb12_zd_press(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_svb12_zd_depress(sigs_press, press_len, sigs_depress,
				    &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_svb_zd(const int16_t *sigs, const uint32_t nr_sigs,
		     struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_svb_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_svb_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_svb_zd_depress_16(sigs_press, press_len, sigs_depress,
				     &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_svb0124_zd(const int16_t *sigs, const uint32_t nr_sigs,
			 struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_svb0124_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_svb0124_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_svb0124_zd_depress_16(sigs_press, press_len, sigs_depress,
					 &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_svb12_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_svb12_zd_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_svb12_zd_press(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_svb12_zd_depress(sigs_press, press_len, sigs_depress,
				    &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_bzip2_svb12_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = bzip2_svb12_zd_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = bzip2_svb12_zd_press(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = bzip2_svb12_zd_depress(sigs_press, press_len, sigs_depress,
				     &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_fast_lzma2_svb12_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = fast_lzma2_svb12_zd_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = fast_lzma2_svb12_zd_press(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = fast_lzma2_svb12_zd_depress(sigs_press, press_len, sigs_depress,
					  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_flac(const int16_t *sigs, const uint32_t nr_sigs, uint32_t bps,
	      uint32_t sample_rate, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int32_t *sigs_32;
	int32_t *sigs_depress_32;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	sigs_32 = malloc(nr_sigs * sizeof *sigs_32);
	ASSERT(sigs_32);
	for (i = 0; i < nr_sigs; i++) {
		sigs_32[i] = sigs[i];
	}

	/* bound sigs_press */
	before = clock();
	pressbound = flac_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = flac_press(sigs_32, nr_sigs, sigs_press, &press_len, bps,
			 sample_rate);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	depress_len = nr_sigs;
	sigs_depress_32 = malloc(depress_len * sizeof *sigs_depress_32);
	ASSERT(sigs_depress_32);

	/* decompress sigs_press */
	before = clock();
	ret = flac_depress(sigs_press, press_len, sigs_depress_32, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_32);
	free(sigs_press);
	free(sigs_depress_32);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_flac_P11(const int16_t *sigs, const uint32_t nr_sigs,
		  struct result *res)
{
	return test_flac(sigs, nr_sigs, P11_BITS_PER_SAMPLE, P11_SAMPLING_RATE,
			 res);
}

int test_zstd_flac(const int16_t *sigs, const uint32_t nr_sigs, uint32_t bps,
		   uint32_t sample_rate, struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int32_t *sigs_32;
	int32_t *sigs_depress_32;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	sigs_32 = malloc(nr_sigs * sizeof *sigs_32);
	ASSERT(sigs_32);
	for (i = 0; i < nr_sigs; i++) {
		sigs_32[i] = sigs[i];
	}

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_flac_bound(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_flac_press(sigs_32, nr_sigs, sigs_press, &press_len, bps,
			      sample_rate);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	depress_len = nr_sigs;
	sigs_depress_32 = malloc(depress_len * sizeof *sigs_depress_32);
	ASSERT(sigs_depress_32);

	/* decompress sigs_press */
	before = clock();
	ret = zstd_flac_depress(sigs_press, press_len, sigs_depress_32,
				&depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < nr_sigs; i++) {
		ASSERT(sigs_depress_32[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_32);
	free(sigs_press);
	free(sigs_depress_32);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_flac_P11(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	return test_zstd_flac(sigs, nr_sigs, P11_BITS_PER_SAMPLE,
			      P11_SAMPLING_RATE, res);
}

int test_turbopfor(const int16_t *sigs, const uint32_t nr_sigs,
		   struct result * res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint64_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = turbopfor_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	turbopfor_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	turbopfor_depress_16(sigs_press, press_len, sigs_depress,
			     depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_vb1e2_zd(const int16_t *sigs, const uint32_t nr_sigs,
		  struct result * res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint32_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = vb1e2_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	vb1e2_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	vb1e2_zd_depress_16(sigs_press, press_len, sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		  struct result * res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t i;
	uint32_t depress_len;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	vbe21_zd_depress_16(sigs_press, press_len, sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_vb1e2_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_vb1e2_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_vb1e2_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_vb1e2_zd_depress_16(sigs_press, press_len, sigs_depress,
				       &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zstd_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zstd_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zstd_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zstd_vbe21_zd_depress_16(sigs_press, press_len, sigs_depress,
				       &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_zlib_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = zlib_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = zlib_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = zlib_vbe21_zd_depress_16(sigs_press, press_len, sigs_depress,
				       &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_huffman_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			  struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = huffman_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = huffman_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = huffman_vbe21_zd_depress_16(sigs_press, press_len, sigs_depress,
					  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_huffman_vbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			  struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = huffman_vbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = huffman_vbbe21_zd_press_16(sigs, nr_sigs, sigs_press,
					 &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = huffman_vbbe21_zd_depress_16(sigs_press, press_len, sigs_depress,
					   &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_shuffman_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			   struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	unsigned int dataBytesOut;
	SymbolEncoder *se;
	huffman_node *root;
	FILE *fp;

	fp = fopen("NA12878_zd.huffman", "r");
	ret = read_code_table(fp, &root, &dataBytesOut);
	ASSERT(ret == 1);
	/*se = &NA12878_zd_se;*/
	se = malloc(sizeof(SymbolEncoder));
	build_symbol_encoder(root, se);

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = shuffman_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = shuffman_vbe21_zd_press_16(se, sigs, nr_sigs, sigs_press,
					 &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = shuffman_vbe21_zd_depress_16(root, sigs_press, press_len,
					   sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free_encoder(se);
	free_huffman_tree(root);
	free(sigs_press);
	free(sigs_depress);
	fclose(fp);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_shuffman_vbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			   struct result *res)
{
	clock_t after;
	clock_t before;
	int ret;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;
	unsigned int dataBytesOut;
	SymbolEncoder *se;
	huffman_node *root;
	FILE *fp;

	fp = fopen("NA12878_zd.huffman", "r");
	ret = read_code_table(fp, &root, &dataBytesOut);
	ASSERT(ret == 1);
	/*se = &NA12878_zd_se;*/
	se = malloc(sizeof(SymbolEncoder));
	build_symbol_encoder(root, se);

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = shuffman_vbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	ret = shuffman_vbbe21_zd_press_16(se, sigs, nr_sigs, sigs_press,
					  &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	ret = shuffman_vbbe21_zd_depress_16(root, sigs_press, press_len,
					    sigs_depress, &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));
	ASSERT(ret == 0);

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free_encoder(se);
	free_huffman_tree(root);
	free(sigs_press);
	free(sigs_depress);
	fclose(fp);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rice_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rice_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rice_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rice_vbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				 &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rice_vbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rice_vbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = malloc(pressbound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rice_vbbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rice_vbbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rc_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		     struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rc_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rc_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rc_vbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
			       &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rc_vbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		     struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rc_vbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rc_vbbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rc_vbbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				&depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rccdf_vbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rccdf_vbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rccdf_vbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rccdf_vbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				  &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rccdf_vbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
			struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rccdf_vbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rccdf_vbbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rccdf_vbbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				   &depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_rc_svbbe21_zd(const int16_t *sigs, const uint32_t nr_sigs,
		       struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	uint32_t i;
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = rc_svbbe21_zd_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	rc_svbbe21_zd_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	rc_svbbe21_zd_depress_16(sigs_press, nr_sigs, sigs_depress,
				&depress_len);
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int test_jumps(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
	int16_t *sigs_depress;
	uint32_t depress_len;
	/*uint32_t i;*/
	uint64_t nr_sigs_bytes;
	uint64_t press_len;
	uint64_t pressbound;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	before = clock();
	pressbound = jumps_bound_16(nr_sigs);
	after = clock();
	UPDATE_RES(res, pressbound_clocktime, GET_CLOCK_SECS(before, after));

	/* init sigs_press */
	sigs_press = calloc(pressbound, 1);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = pressbound;
	before = clock();
	jumps_press_16(sigs, nr_sigs, sigs_press, &press_len);
	after = clock();
	UPDATE_RES(res, press_clocktime, GET_CLOCK_SECS(before, after));

	/*ASSERT(press_len <= pressbound);*/

	/* init sigs_depress */
	sigs_depress = malloc(nr_sigs_bytes);
	ASSERT(sigs_depress);

	/* decompress sigs_press */
	depress_len = nr_sigs;
	before = clock();
	/*jumps_depress_16(sigs_press, nr_sigs, sigs_depress, &depress_len);*/
	after = clock();
	UPDATE_RES(res, depress_clocktime, GET_CLOCK_SECS(before, after));

	ASSERT(depress_len == nr_sigs);

	/* ensure decompressed == original */
	/*
	for (i = 0; i < depress_len / sizeof *sigs; i++) {
		ASSERT(sigs_depress[i] == sigs[i]);
	}
	*/

	/* let it go */
	free(sigs_press);
	free(sigs_depress);

	UPDATE_RES(res, depress_bytes, nr_sigs_bytes);
	UPDATE_RES(res, pressbound_bytes, pressbound);
	UPDATE_RES(res, press_bytes, press_len);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	FILE *fp;
	struct result res;

	/* check args */
	if (argc != 2) {
		fprintf(stderr, USAGE, argv[0]);
		return 1;
	}

	/*fp = fopen(stdout, "w");*/
	fp = stdout;
	fwrite_res_hdr(fp);

	/* TODO test array > 0 */
	/*TEST(none, &res, fp);
	TEST(uint11_16, &res, fp);
	TEST(uint_16, &res, fp);
	TEST(uint_submin_16, &res, fp);
	TEST(uint_zd_16, &res, fp);
	TEST(uint_zsm_16, &res, fp);
	TEST(zlib_uint_submin_16, &res, fp);
	TEST(zlib_uint_zd_16, &res, fp);
	TEST(zstd_uint_submin_16, &res, fp);
	TEST(zstd_uint_zd_16, &res, fp);
	TEST(bzip2_uint_zd_16, &res, fp);
	TEST(fast_lzma2_uint_zd_16, &res, fp);*/
	/*TEST(flat_uint_submin_16_step1, &res, fp);*/
	/*TEST(zlib, &res, fp);
	TEST(zstd, &res, fp);
	TEST(bzip2, &res, fp);
	TEST(fast_lzma2, &res, fp);
	TEST(svb, &res, fp);
	TEST(svb0124, &res, fp);
	TEST(svb12, &res, fp);
	TEST(svb_zd, &res, fp);
	TEST(svb0124_zd, &res, fp);*/
	//TEST(svb12_zd, &res, fp);
	/*TEST(zlib_svb_zd, &res, fp);
	TEST(zlib_svb0124_zd, &res, fp);
	TEST(zlib_svb12_zd, &res, fp);
	TEST(zstd_svb_zd, &res, fp);
	TEST(zstd_svb0124_zd, &res, fp);
	TEST(zstd_svb12_zd, &res, fp);
	TEST(bzip2_svb12_zd, &res, fp);
	TEST(fast_lzma2_svb12_zd, &res, fp);
	TEST(flac_P11, &res, fp);
	TEST(zstd_flac_P11, &res, fp);
	TEST(turbopfor, &res, fp);
	TEST(vb1e2_zd, &res, fp);
	TEST(vbe21_zd, &res, fp);
	TEST(zstd_vb1e2_zd, &res, fp);
	TEST(zstd_vbe21_zd, &res, fp);
	TEST(zlib_vbe21_zd, &res, fp);
	*/
	TEST(huffman_vbe21_zd, &res, fp);
	TEST(shuffman_vbe21_zd, &res, fp);
	/*
	TEST(rice_vbe21_zd, &res, fp);
	TEST(rc_vbe21_zd, &res, fp);
	TEST(rccdf_vbe21_zd, &res, fp);
	TEST(huffman_vbbe21_zd, &res, fp);
	TEST(shuffman_vbbe21_zd, &res, fp);
	TEST(rice_vbbe21_zd, &res, fp);
	TEST(rc_vbbe21_zd, &res, fp);
	TEST(rccdf_vbbe21_zd, &res, fp);
	TEST(jumps, &res, fp);
	TEST(rc_svbbe21_zd, &res, fp);*/

	(void) fclose(fp);

	return 0;
}
