#include "svb16.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

inline uint16_t zigzag_decode(uint16_t val) {
    return (val >> 1) ^ (uint16_t)(0 - (val & 1));
}
inline uint16_t decode_data(uint8_t const *SVB_RESTRICT *dataPtrPtr, uint8_t code) {
    const uint8_t *dataPtr = *dataPtrPtr;
    uint16_t val;

    if (code == 0) {  // 1 byte
        val = (uint16_t)*dataPtr;
        dataPtr += 1;
    } else {  // 2 bytes
        val = 0;
        memcpy(&val, dataPtr, 2);  // assumes little endian
        dataPtr += 2;
    }

    *dataPtrPtr = dataPtr;
    return val;
}

uint8_t const *decode_scalar(bool UseDelta, bool UseZigzag, int16_t *out,
                             uint8_t const *SVB_RESTRICT keys,
                             uint8_t const *SVB_RESTRICT data,
                             uint32_t count,
                             int16_t prev) {
    if (count == 0) {
        return data;
    }

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = *keys++;
    // need to do the arithmetic in unsigned space so it wraps
    uint16_t u_prev = (uint16_t)(prev);
    for (uint32_t c = 0; c < count; c++, shift++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        uint16_t value = decode_data(&data, (key_byte >> shift) & 0x01);
        SVB16_IF_CONSTEXPR(UseZigzag) { value = zigzag_decode(value); }
        SVB16_IF_CONSTEXPR(UseDelta) {
            value += u_prev;
            u_prev = value;
        }
        *out++ = (int16_t)(value);
    }
    return data;
}
