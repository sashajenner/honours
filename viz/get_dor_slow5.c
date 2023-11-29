/*
 * get the digitisation, offset and range from the first read
 * given (s|b)low5 file
 * cc get_dor_slow5.c -I PATH_TO_SLOW5LIB_INCLUDE PATH_TO_LIBSLOW5 -o get_dor_slow5
 * ./get_dor_slow5 (S|B)LOW5_FILE
 */

#include <stdio.h>
#include <slow5/slow5.h>

#define USAGE ("usage: %s (S|B)LOW5_FILE\n")

void print_dor(struct slow5_file *fp) {
	struct slow5_rec *rec;
	int ret;

	/* init */
	rec = NULL;

	ret = slow5_get_next(&rec, fp);
	if (ret >= 0)
		printf("digitisation\toffset\trange\n%f\t%f\t%f\n",
		       rec->digitisation, rec->offset, rec->range);
	else
		fprintf(stderr, "error retrieving slow5 record\n");

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
	print_dor(fp);

	/* close file */
	slow5_close(fp);

	return 0;
}
