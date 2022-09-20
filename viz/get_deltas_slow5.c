/*
 * get the deltas between slow5 signals
 * given (s|b)low5 file
 * output as one column with 'raw_signal_delta' as the header
 * cc get_deltas_slow5.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o get_deltas_slow5
 * ./get_deltas_slow5 (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <slow5/slow5.h>

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

void print_all_deltas(struct slow5_file *fp)
{
	struct slow5_rec *rec;
	int ret;
	uint64_t i;
	int16_t diff;
	int16_t cur;
	int16_t prev;

	/* init */
	rec = NULL;

	puts("raw_signal_delta");

	/* print each read's statistics */
	ret = slow5_get_next(&rec, fp);
	while (ret >= 0) {
		prev = rec->raw_signal[0];
		for (i = 1; i < rec->len_raw_signal; i++) {
			cur = rec->raw_signal[i];
			diff = cur - prev;
			prev = cur;

			printf("%" PRId16 "\n", diff);
		}

		ret = slow5_get_next(&rec, fp);
	}

	/* let it go */
	slow5_rec_free(rec);
}

int main(int argc, char **argv)
{
	struct slow5_file *fp;

	/* check args */
	if (argc != 2) {
		fprintf(stderr, USAGE, argv[0]);
		return 1;
	}

	/* open file */
	fp = slow5_open(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "error opening file\n");
		return 1;
	}

	/* do the work */
	print_all_deltas(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
