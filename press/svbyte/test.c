#include "test.h"
#include "svbyte.h"
#include "../bitmap.h"
#include <stdint.h>
#include <string.h>

/*
 * x0 x1 x2 x3 ...
 * {[padding]x0 nbytes|x1 nbytes...}x0 data|x1 data...
 */

/*
 * 0 1 2 3
 * {[0000]0|0|0|0}00000000|00000001|00000010|00000011
 * 8 * (1 + 4) = 40
 */
const uint16_t SEQ0[] = { 0, 1, 2, 3 };
const uint8_t SEQ0_CODE[] = { 0x00, 0x00, 0x01, 0x02, 0x03 };

/*
 * 0 1 2 256
 * {[0000]1|0|0|0}00000000|00000001|00000010|00000000 00000001
 * 8 * (1 + 3 + 2) = 48
 */
const uint16_t SEQ1[] = { 0, 1, 2, 256 };
const uint8_t SEQ1_CODE[] = { 0x08, 0x00, 0x01, 0x02, 0x00, 0x01 };

/*
 * 0 1 2 256 2 256 1 0
 * {0|0|1|0|1|0|0|0}00000000|00000001|00000010|00000000 00000001|00000010|00000000 00000001|00000001|00000000
 * 8 * (1 + 6 + 2 * 2) = 88
 */
const uint16_t SEQ2[] = { 0, 1, 2, 256, 2, 256, 1, 0 };
const uint8_t SEQ2_CODE[] = { 0x28, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x01, 0x00 };

/*
 * 0 1 2 256 2 256 1 0 9224
 * {0|0|1|0|1|0|0|0 [00000000]1}00000000|00000001|00000010|00000000 00000001|00000010|00000000 00000001|00000001|00000000|00001000 00100100
 * 8 * (2 + 6 + 3 * 2) = 112
 */
const uint16_t SEQ3[] = { 0, 1, 2, 256, 2, 256, 1, 0, 9224 };
const uint8_t SEQ3_CODE[] = { 0x28, 0x01, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x01, 0x00, 0x08, 0x24 };

int svbytebound_test(void) {
	ASSERT(svbytebound(SEQ0, LENGTH(SEQ0)) == BYTES_TO_BITS(LENGTH(SEQ0_CODE)));
	ASSERT(svbytebound(SEQ1, LENGTH(SEQ1)) == BYTES_TO_BITS(LENGTH(SEQ1_CODE)));
	ASSERT(svbytebound(SEQ2, LENGTH(SEQ2)) == BYTES_TO_BITS(LENGTH(SEQ2_CODE)));
	ASSERT(svbytebound(SEQ3, LENGTH(SEQ3)) == BYTES_TO_BITS(LENGTH(SEQ3_CODE)));

	return EXIT_SUCCESS;
}

int svbytecode_test(void) {
	uint8_t *code = malloc(LENGTH(SEQ0_CODE));
	ASSERT(code);

	svbytecode(SEQ0, LENGTH(SEQ0), code);
	ASSERT(memcmp(code, SEQ0_CODE, LENGTH(SEQ0_CODE)) == 0);

	code = realloc(code, LENGTH(SEQ1_CODE));
	ASSERT(code);

	svbytecode(SEQ1, LENGTH(SEQ1), code);
	ASSERT(memcmp(code, SEQ1_CODE, LENGTH(SEQ1_CODE)) == 0);

	code = realloc(code, LENGTH(SEQ2_CODE));
	ASSERT(code);

	svbytecode(SEQ2, LENGTH(SEQ2), code);
	ASSERT(memcmp(code, SEQ2_CODE, LENGTH(SEQ2_CODE)) == 0);

	code = realloc(code, LENGTH(SEQ3_CODE));
	ASSERT(code);

	svbytecode(SEQ3, LENGTH(SEQ3), code);
	ASSERT(memcmp(code, SEQ3_CODE, LENGTH(SEQ3_CODE)) == 0);

	free(code);

	return EXIT_SUCCESS;
}

int main(void) {

	struct command tests[] = {
		CMD(svbytebound_test),
		CMD(svbytecode_test),
	};

	return RUN_TESTS(tests);
}
