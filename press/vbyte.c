/*
 * classical vbyte
 * writes non-negative integers
 * most sig bit 0 when following byte continues otherwise 1
 * [0, 2^7) uses 1 byte
 * [2^7, 2^14) uses 2 bytes
 * ...
 * [2^28, 2^35) uses 5 bytes
 */

#include <stdint.h>

#define MAX_BYTE_1 (128)
#define MAX_BYTE_2 (16384)
#define MAX_BYTE_3 (2097152)
#define MAX_BYTE_4 (268435456)
#define MAX_BYTE_5 (34359738368)

uint64_t vbytepress(uint32_t *in, uint64_t nin, uint8_t **out)
{
	uint64_t i;
	uint64_t j;
	uint32_t cur;
	uint8_t curbyte;

	i = 0;
	j = 0;
	cur = in[0];

	while (i < nin) {
		while (cur) {
			curbyte = cur & 0xff000000;
			cur = curr << 8;
		}
		j++;
	}

	return out;
}

uint64_t vbytebound(uint32_t *in, uint64_t nin)
{
	uint64_t nout;
	uint64_t i;

	nout = 0;
	for (i = 0; i < nin; i++) {
		/* TODO use bit magic */
		if (in[i] < MAX_BYTE_1) {
			nout++;
		} else if (in[i] < MAX_BYTE_2) {
			nout += 2;
		} else if (in[i] < MAX_BYTE_3) {
			nout += 3;
		} else if (in[i] < MAX_BYTE_4) {
			nout += 4;
		} else {
			nout += 5;
		}
	}

	return nout;
}
