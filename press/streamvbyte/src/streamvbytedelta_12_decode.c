#include "streamvbyte_isadetection.h"
#include "streamvbytedelta.h"

#include <string.h>  // for memcpy

static inline uint16_t zigzag_decode_16(uint16_t val) {
    return (val >> 1) ^ (uint16_t)(0 - (val & 1));
}

static inline uint16_t _decode_data(const uint8_t **dataPtrPtr, uint8_t code) {
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

static const uint8_t *svb_decode_scalar_d1_init(uint16_t *outPtr,
                                                const uint8_t *keyPtr,
                                                const uint8_t *dataPtr,
                                                uint32_t count,
                                                uint16_t prev) {
    if (count == 0)
        return dataPtr;  // no reads or writes if no data

    uint8_t shift = 0;
    uint16_t key = *keyPtr++;

    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            key = *keyPtr++;
        }
        uint16_t val = zigzag_decode_16(_decode_data(&dataPtr, (key >> shift) & 0x1));
        //uint16_t val = _decode_data(&dataPtr, (key >> shift) & 0x1);
        val += prev;
        *outPtr++ = val;
        prev = val;
        shift += 1;
    }

    return dataPtr;  // pointer to first unused byte after end
}

#ifdef STREAMVBYTE_X64
#include "streamvbytedelta_12_x64_decode.c"
#endif

size_t streamvbyte_zigzag_delta_decode_12(const uint8_t *in,
                                          uint16_t *out,
                                          uint32_t count,
                                          uint16_t prev) {
    // keyLen = ceil(count / 8), without overflowing (1 bit per input value):
    uint32_t keyLen = (count >> 3) + (((count & 7) + 7) >> 3);
    const uint8_t *keyPtr = in;
    const uint8_t *dataPtr = keyPtr + keyLen;  // data starts at end of keys
#ifdef STREAMVBYTE_X64
    if (streamvbyte_ssse3()) {
        return svb_decode_avx_d1_init(out, keyPtr, dataPtr, count, prev) - in;
    }
#endif
    return svb_decode_scalar_d1_init(out, keyPtr, dataPtr, count, prev) - in;
}
