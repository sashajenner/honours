#ifndef TRANS_H
#define TRANS_H

#include <stdint.h>

int16_t *shift_x(int16_t x, const int16_t *in, uint64_t nin);
int16_t *delta(const int16_t *in, uint64_t nin, uint64_t *nout);
int16_t *zigdelta(const int16_t *in, uint64_t nin, uint64_t *nout);
void unzigdelta_inplace(int16_t *in, uint64_t nin);

#endif
