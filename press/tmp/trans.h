#ifndef TRANS_H
#define TRANS_H

#include <stdint.h>

int16_t *shift_x(int16_t x, const int16_t *in, uint64_t nin);
void shift_x_inplace(int16_t x, int16_t *in, uint64_t nin);
void zigzag_inplace(int16_t *in, uint64_t nin);
void unzigzag_inplace(int16_t *in, uint64_t nin);
int16_t *zigdelta(const int16_t *in, uint64_t nin, uint64_t *nout);
void unzigdelta_inplace(int16_t *in, uint64_t nin);

#endif
