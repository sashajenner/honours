#ifndef TRANS_H
#define TRANS_H

#include <stdint.h>

uint16_t *shift_x_u16(int16_t x, const uint16_t *in, uint64_t nin);
int16_t *shift_x_16(int16_t x, const int16_t *in, uint64_t nin);
void shift_x_inplace_u16(uint16_t x, uint16_t *in, uint64_t nin);
void shift_x_inplace_16(int16_t x, int16_t *in, uint64_t nin);
void zigzag_inplace_16(int16_t *in, uint64_t nin);
void unzigzag_inplace_16(uint16_t *in, uint64_t nin);
uint16_t *zigdelta_16(const int16_t *in, uint64_t nin);
void unzigdelta_inplace_16(int16_t *in, uint64_t nin);

#endif /* trans.h */
