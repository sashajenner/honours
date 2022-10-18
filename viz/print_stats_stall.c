/*
 * print statistics for each read's stall in tsv format
 * given (s|b)low5 file
 * cc print_stats_stall.c stats.c ../press/sigtk/build/jnn.o ../press/sigtk/build/sigtk.o -lm -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o print_stats_stall
 * ./print_stats_stall (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <stdint.h>
#include <slow5/slow5.h>
#include "stats.h"
#include "../press/sigtk/src/jnn.h"

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

int find_stall(const int16_t *in, uint32_t nin, uint16_t *stall_start,
	       uint16_t *stall_len)
{
	int nsegs;
	jnn_pair_t *segs;

	jnn_param_t param = JNNV1_CDNA_PARAM;
	segs = jnn_raw(in, nin, param, &nsegs);

	if (!nsegs)
		return 0;

	*stall_start = segs[0].x;
	*stall_len = segs[0].y - segs[0].x + 1;

	free(segs);

	return 1;
}

void print_all_stats_stall(struct slow5_file *fp)
{
	struct slow5_rec *rec;
	struct stats rec_info;
	int ret;
	uint64_t i;
	uint16_t stall_start;
	uint16_t stall_len;
	int is_stall;

	/* init */
	rec = NULL;
	init_stats(&rec_info);

	/* print header */
	printf("stall_start\t");
	print_hdr_stats();

	/* print each read's statistics */
	ret = slow5_get_next(&rec, fp);
	while (ret >= 0) {
		is_stall = find_stall(rec->raw_signal, rec->len_raw_signal,
				      &stall_start, &stall_len);
		if (is_stall) {
			update_stats_start(rec, &rec_info);
			for (i = stall_start; i < stall_start + stall_len; i++) {
				update_stats(rec->raw_signal[i], &rec_info);
			}
			update_stats_end(rec, &rec_info);
			printf("%" PRIu16 "\t", stall_start);
			print_stats(&rec_info);
		}

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
	print_all_stats_stall(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
