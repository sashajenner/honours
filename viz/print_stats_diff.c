/*
 * print statistics for each read in tsv format
 * given (s|b)low5 file
 * cc print_stats_diff.c stats.c -lm -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o print_stats_diff
 * ./print_stats (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "stats.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

void print_all_stats(struct slow5_file *fp)
{
	struct slow5_rec *rec;
	struct stats rec_info;
	int ret;
	uint64_t i;

	/* init */
	rec = NULL;
	init_stats(&rec_info);

	/* print header */
	print_hdr_stats();

	/* print each read's statistics */
	ret = slow5_get_next(&rec, fp);
	while (ret >= 0) {
		update_stats_start(rec, &rec_info);
		for (i = 0; i < rec->len_raw_signal - 1; i++) {
			update_stats(rec->raw_signal[i + 1] - rec->raw_signal[i], &rec_info);
		}
		update_stats_end(rec, &rec_info);
		print_stats(&rec_info);

		init_stats(&rec_info);
		ret = slow5_get_next(&rec, fp);
	}

	/* let it go */
	slow5_rec_free(rec);
	/*free_stats(rec_info);*/
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
	print_all_stats(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
