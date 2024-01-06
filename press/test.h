#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <time.h>
#include <slow5/slow5.h>

/* search for and shift unused lower bits before and after compression */
//#define QTS

#define GET_CLOCK_SECS(before, after) ((double) (after - before) / CLOCKS_PER_SEC)

#define TEST(method, res, fp) \
{ \
	struct slow5_rec *rec;\
	struct slow5_file *sfp;\
	int ret;\
\
	fprintf(stderr, "%s\n", #method);\
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
	fflush(fp);\
	/* close file */\
	slow5_close(sfp);\
}

#define ASSERT(statement) \
if (!(statement)) { \
    fprintf(stderr, "line %d: assertion `%s' failed\n", __LINE__, #statement); \
    return EXIT_SUCCESS; \
}

#define LENGTH(arr) ((sizeof (arr)) / sizeof *arr)

#ifdef QTS
#define RESULTS_HDR ("method\t" \
		     "pressbound_bytes\t" \
		     "press_bytes\t" \
		     "press_ratio\t" \
		     "depress_bytes\t" \
		     "pressbound_time\t" \
		     "press_time\t" \
		     "depress_time\t" \
		     "qts_press_time\t" \
		     "qts_depress_time\n")
#else
#define RESULTS_HDR ("method\t" \
		     "pressbound_bytes\t" \
		     "press_bytes\t" \
		     "press_ratio\t" \
		     "depress_bytes\t" \
		     "pressbound_time\t" \
		     "press_time\t" \
		     "depress_time\n")
#endif /* QTS */
		     /*"nflats\t" \
		     "flats\n")*/
#ifdef QTS
#define RESULTS_FORMAT ("%s\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\n")
#else
#define RESULTS_FORMAT ("%s\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\t" \
			"%f\n")
#endif /* QTS */
			/*"%" PRIu32 "\t" \
			"%s\n")*/

#ifdef QTS
#define RESULTS_ARGS res->method_name, \
		     res->pressbound_bytes, \
		     res->press_bytes, \
		     res->press_ratio, \
		     res->depress_bytes, \
		     res->pressbound_clocktime, \
		     res->press_clocktime, \
		     res->depress_clocktime, \
		     res->qts_press_clocktime, \
		     res->qts_depress_clocktime
#else
#define RESULTS_ARGS res->method_name, \
		     res->pressbound_bytes, \
		     res->press_bytes, \
		     res->press_ratio, \
		     res->depress_bytes, \
		     res->pressbound_clocktime, \
		     res->press_clocktime, \
		     res->depress_clocktime
#endif /* QTS */

/* running mean */
/*#define UPDATE_RES(res, attr, update) (res->attr += ((update) - res->attr) / res->n)*/
/* total */
#define UPDATE_RES(res, attr, update) (res->attr += update)

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
#ifdef QTS
	double qts_press_clocktime;
	double qts_depress_clocktime;
#endif /* QTS */
	uint64_t n;
};

/* the first read from PAF25452_pass_bfdfd1d8_1 */
#define P11_BITS_PER_SAMPLE (12) /* actually 11 but FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE error */
#define P11_SAMPLING_RATE (4000)

# endif /* test.h */
