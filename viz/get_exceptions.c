/*
 * get 2 byte diff exceptions for each read
 * given (s|b)low5 file
 * cc get_exceptions.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o get_exceptions
 * ./get_exceptions (S|B)LOW5_FILE OUT_NEXC_FILE OUT_EXC_FILE
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <slow5/slow5.h>

#define USAGE ("usage: %s (S|B)LOW5_FILE OUT_NEXC_FILE OUT_EXC_FILE\n")

void write_exc(struct slow5_file *fp, FILE *nexc_fout, FILE *exc_fout)
{
	struct slow5_rec *rec;
	int ret;
	uint64_t i;
	int16_t prev;
	int16_t cur;
	int16_t diff;
	int n;

	/* init */
	rec = NULL;

	/* print each read's statistics */
	ret = slow5_get_next(&rec, fp);
	while (ret >= 0) {
		n = 0;
		prev = rec->raw_signal[0];
		for (i = 1; i < rec->len_raw_signal; i++) {
			cur = rec->raw_signal[i];
			diff = cur - prev;
			prev = cur;

			if (diff > 127 || diff < -128) {
				n++;
				fprintf(exc_fout, "%" PRId16 "\n", diff);
			}
		}
		fprintf(nexc_fout, "%d\n", n);

		ret = slow5_get_next(&rec, fp);
	}

	/* let it go */
	slow5_rec_free(rec);
}

int main(int argc, char **argv)
{
	struct slow5_file *fp;
	FILE *nexc_fout;
	FILE *exc_fout;

	/* check args */
	if (argc != 4) {
		fprintf(stderr, USAGE, argv[0]);
		return 1;
	}

	/* open file */
	fp = slow5_open(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "error opening file\n");
		return 1;
	}

	nexc_fout = fopen(argv[2], "w");
	if (!nexc_fout) {
		fprintf(stderr, "error opening file\n");
		return 1;
	}

	exc_fout = fopen(argv[3], "w");
	if (!exc_fout) {
		fprintf(stderr, "error opening file\n");
		return 1;
	}

	/* do the work */
	write_exc(fp, nexc_fout, exc_fout);

	/* close file */
	slow5_close(fp);
	fclose(nexc_fout);
	fclose(exc_fout);

	return 0;
}
