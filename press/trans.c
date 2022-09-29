#include <stdlib.h>
#include <stdint.h>
#include "trans.h"

uint16_t *shift_x_u16(int16_t x, const uint16_t *in, uint64_t nin)
{
	uint16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin; i++) {
		out[i] = in[i] + x;
	}

	return out;
}

int16_t *shift_x_16(int16_t x, const int16_t *in, uint64_t nin)
{
	int16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin; i++) {
		out[i] = in[i] + x;
	}

	return out;
}

void shift_x_inplace_u16(uint16_t x, uint16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] += x;
	}
}

void shift_x_inplace_16(int16_t x, int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] += x;
	}
}

static inline uint16_t zigzag_one_16(int16_t x)
{
	return (x + x) ^ (x >> 15);
}

static inline int16_t unzigzag_one_16(uint16_t x)
{
	return (x >> 1) ^ -(x & 1);
}

void zigzag_inplace_16(int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] = zigzag_one_16(in[i]);
	}
}

void unzigzag_inplace_16(uint16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 0; i < nin; i++) {
		in[i] = unzigzag_one_16(in[i]);
	}
}

int16_t *delta_16(const int16_t *in, uint64_t nin)
{
	int16_t prev;
	int16_t *out;
	uint64_t i;

	prev = 0;
	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin; i++) {
		out[i] = in[i] - prev;
		prev = in[i];
	}

	return out;
}

uint32_t *delta_increasing_u32(const uint32_t *in, uint64_t nin)
{
	uint32_t prev;
	uint32_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	out[0] = in[0];
	prev = in[0];

	for (i = 1; i < nin; i++) {
		out[i] = in[i] - prev - 1;
		prev = in[i];
	}

	return out;
}

void undelta_inplace_16(int16_t *in, uint64_t nin)
{
	int16_t prev;
	uint64_t i;

	prev = 0;

	for (i = 0; i < nin; i++) {
		in[i] += prev;
		prev = in[i];
	}
}

void undelta_inplace_increasing_u32(uint32_t *in, uint64_t nin)
{
	uint32_t prev;
	uint64_t i;

	prev = in[0];

	for (i = 1; i < nin; i++) {
		in[i] += prev + 1;
		prev = in[i];
	}
}

/* return zigzag delta of in with nin - 1 elements */
uint16_t *zigdelta_16(const int16_t *in, uint64_t nin)
{
	uint16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);

	for (i = 0; i < nin - 1; i++) {
		out[i] = zigzag_one_16(in[i + 1] - in[i]);
	}

	return out;
}

/* return zigzag delta of in with nin elements */
uint16_t *zigdelta_16_u16(const int16_t *in, uint64_t nin)
{
	int16_t prev;
	uint16_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);
	prev = 0;

	for (i = 0; i < nin; i++) {
		out[i] = zigzag_one_16(in[i] - prev);
		prev = in[i];
	}

	return out;
}

/* return zigzag delta of in with nin elements */
uint32_t *zigdelta_16_u32(const int16_t *in, uint64_t nin)
{
	int16_t prev;
	uint32_t *out;
	uint64_t i;

	out = malloc(nin * sizeof *out);
	prev = 0;

	for (i = 0; i < nin; i++) {
		out[i] = zigzag_one_16(in[i] - prev);
		prev = in[i];
	}

	return out;
}

/* in[0] must be the first signal */
void unzigdelta_inplace_16(int16_t *in, uint64_t nin)
{
	uint64_t i;

	for (i = 1; i < nin; i++) {
		in[i] = in[i - 1] + unzigzag_one_16(in[i]);
	}
}

void unzigdelta_u16_16(const uint16_t *in, uint64_t nin, int16_t *out)
{
	int16_t prev;
	uint64_t i;

	prev = 0;
	for (i = 0; i < nin; i++) {
		out[i] = prev + unzigzag_one_16(in[i]);
		prev = out[i];
	}
}

void unzigdelta_u32_16(const uint32_t *in, uint64_t nin, int16_t *out)
{
	int16_t prev;
	uint64_t i;

	prev = 0;
	for (i = 0; i < nin; i++) {
		out[i] = prev + unzigzag_one_16(in[i]);
		prev = out[i];
	}
}
