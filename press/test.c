#include <stdlib.h>
#include <stdint.h>
#include "test.h"
#include "press.h"
#include "bitmap.h"
#include <stdio.h> /* TODO remove */
#include <inttypes.h> /* TODO remove */

void init_res(struct result *res)
{
	res->press_bound_bytes = 0;
	res->press_bytes = 0;
	res->depress_bytes = 0;
	res->press_clocktime = 0;
	res->depress_clocktime = 0;
}

void fwrite_res_hdr(FILE *fp)
{
	(void) fputs(RESULTS_HDR, fp);
}

void fwrite_res(FILE *fp, struct result *res)
{
	(void) fprintf(fp, RESULTS_FORMAT,
		       res->method_name,
		       res->data_name,
		       res->press_bound_bytes,
		       res->press_bytes,
		       res->depress_bytes,
		       res->press_clocktime,
		       res->depress_clocktime);
}

int test_none(const int16_t *sigs, const uint32_t nr_sigs, struct result *res)
{
	clock_t after;
	clock_t before;
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
	before = clock();
	ret = none_press((const uint8_t *) sigs, nr_sigs_bytes, sigs_press,
			 &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len == press_bound);

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
	res->press_bound_bytes = press_bound;
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
	uint64_t press_bound;
	uint64_t press_len;
	uint8_t *sigs_press;

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = uintx_bound_16(bits_out, nr_sigs_bytes);

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	before = clock();
	ret = uintx_press_16(bits_out, (const uint8_t *) sigs, nr_sigs,
			     sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= press_bound);

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
	res->press_bound_bytes = press_bound;
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
	uint64_t press_bound;
	uint64_t press_len;
	uint8_t *sigs_press;
	uint8_t minbits;

	minbits = uint_get_minbits_16((const uint16_t *) sigs, nr_sigs);
	printf("min bits out:\t%" PRIu8 "\n", minbits);

	nr_sigs_bytes = sizeof *sigs * nr_sigs;

	/* bound sigs_press */
	press_bound = uint_bound_16(minbits, nr_sigs);

	/* init sigs_press */
	sigs_press = malloc(press_bound);
	ASSERT(sigs_press);

	/* compress sigs */
	press_len = press_bound;
	before = clock();
	ret = uint_press_16(minbits, (const uint16_t *) sigs, nr_sigs,
			    sigs_press, &press_len);
	after = clock();
	res->press_clocktime = GET_CLOCK_SECS(before, after);
	ASSERT(ret == 0);

	ASSERT(press_len <= press_bound);

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
	res->press_bound_bytes = press_bound;
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

	TEST(none, ONE, &res, fp);
	TEST(uint11_16, ONE, &res, fp);
	TEST(uint_16, ONE, &res, fp);

	TEST(none, SAME, &res, fp);
	TEST(uint11_16, SAME, &res, fp);
	TEST(uint_16, SAME, &res, fp);

	TEST(none, ZERO, &res, fp);
	TEST(uint11_16, ZERO, &res, fp);
	TEST(uint_16, ZERO, &res, fp);

	(void) fclose(fp);

	return 0;
}
