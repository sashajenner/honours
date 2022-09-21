#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <time.h>
#include <slow5/slow5.h>

#define GET_CLOCK_SECS(before, after) ((double) (after - before) / CLOCKS_PER_SEC)

#define TEST(method, res, fp) \
{ \
	struct slow5_rec *rec;\
	struct slow5_file *sfp;\
	int ret;\
\
	/* open file */\
	sfp = slow5_open(argv[1], "r");\
	if (!sfp) {\
		fprintf(stderr, "error opening file\n");\
		return 1;\
	}\
	\
	init_res(res); \
	(res)->method_name = #method; \
	rec = NULL;\
	\
	ret = slow5_get_next(&rec, sfp);\
	while (ret >= 0) {\
		(res)->n++;\
		ASSERT(test_##method(rec->raw_signal, rec->len_raw_signal, res) == EXIT_SUCCESS); \
		ret = slow5_get_next(&rec, sfp);\
	}\
	slow5_rec_free(rec);\
	fwrite_res(fp, res); \
	/* close file */\
	slow5_close(sfp);\
}

#define ASSERT(statement) \
if (!(statement)) { \
    fprintf(stderr, "line %d: assertion `%s' failed\n", __LINE__, #statement); \
    return EXIT_FAILURE; \
}

#define LENGTH(arr) ((sizeof (arr)) / sizeof *arr)

#define RESULTS_HDR ("method\t" \
		     "pressbound_bytes\t" \
		     "press_bytes\t" \
		     "press_ratio\t" \
		     "depress_bytes\t" \
		     "pressbound_time\t" \
		     "press_time\t" \
		     "depress_time\n")
		     /*"nflats\t" \
		     "flats\n")*/
#define RESULTS_FORMAT ("%s\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\n")
			/*"%" PRIu32 "\t" \
			"%s\n")*/

/* running mean */
#define UPDATE_RES(res, attr, update) (res->attr += ((update) - res->attr) / res->n)

struct result {
	const char *method_name;
	double depress_clocktime;
	double press_clocktime;
	double press_ratio;
	double pressbound_clocktime;
	/*uint32_t *flats;
	uint32_t nflats;*/
	double depress_bytes;
	double press_bytes;
	double pressbound_bytes;
	uint64_t n;
};

/* the first read from PAF25452_pass_bfdfd1d8_1 */
#define P11_BITS_PER_SAMPLE (12) /* actually 11 but FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE error */
#define P11_SAMPLING_RATE (4000)

# endif /* test.h */
