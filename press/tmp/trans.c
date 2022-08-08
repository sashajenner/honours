#include <stdlib.h>
#include <stdint.h>
#include "trans.h"

int16_t *shift_x(int16_t x, const int16_t *in, uint64_t nin)
{
	int16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin; i++) {
		out[i] = in[i] + x;
	}

	return out;
}

void shift_x_inplace(int16_t x, int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] += x;
	}
}

static inline uint16_t zigzag_one(int16_t x)
{
	return (x + x) ^ (x >> 31);
}

static inline uint16_t unzigzag_one(int16_t x)
{
	return (x >> 1) ^ -(x & 1);
}

void zigzag_inplace(int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] = zigzag_one(in[i]);
	}
}

void unzigzag_inplace(int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] = unzigzag_one(in[i]);
	}
}

int16_t *zigdelta(const int16_t *in, uint64_t nin, uint64_t *nout)
{
	int16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin - 1; i++) {
		out[i] = zigzag_one(in[i + 1] - in[i]);
	}

	*nout = nin - 1;
	return out;
}

/* in[0] must be the first signal */
void unzigdelta_inplace(int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 1; i < nin; i++) {
		in[i] = in[i - 1] + unzigzag_one(in[i]);
	}
}
