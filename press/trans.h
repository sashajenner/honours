#ifndef TRANS_H
#define TRANS_H

#include <stdint.h>

uint16_t *u16_shift_x_u16(uint16_t x, const uint16_t *in, uint64_t nin);
void shift_x_inplace_u16(uint16_t x, uint16_t *in, uint64_t nin);
void zigzag_inplace(int16_t *in, uint32_t nin);
void unzigzag_inplace(int16_t *in, uint32_t nin);
int16_t *zigdelta(const int16_t *in, uint32_t nin, uint32_t *nout);
void unzigdelta_inplace(int16_t *in, uint32_t nin);

#endif /* trans.h */
