#ifndef SVBYTEBOUND
#define SVBYTEBOUND

#include <stdint.h>

#define BYTE_1_UPRBND (1 << BITS_PER_BYTE)
/*
#define BYTE_2_UPRBND (1 << BYTES_TO_BITS(2))
#define BYTE_3_UPRBND (1 << BYTES_TO_BITS(3))
#define BYTE_4_UPRBND (1 << BYTES_TO_BITS(4))
*/

uint64_t svbytebound(const uint16_t *in, uint64_t nin);
void svbytecode(const uint16_t *in, uint64_t nin, uint8_t *out);
void svbytedecode(const uint8_t *in, uint64_t nin, uint16_t *out);

#endif
