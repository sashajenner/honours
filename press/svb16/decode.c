#include "svb16.h"  // svb16_key_length
#include "simd_detect_x64.h"

size_t decode(bool UseDelta, bool UseZigzag, int16_t* out, uint8_t const* SVB_RESTRICT in, uint32_t count) {
    int16_t prev = 0;
    uint8_t const* keys = in;
    uint8_t const* data = keys + svb16_key_length(count);
#ifdef SVB16_X64
    if (has_sse4_1()) {
        return decode_sse(UseDelta, UseZigzag, out, keys, data, count, prev) - in;
    }
#endif
    return decode_scalar(UseDelta, UseZigzag, out, keys, data, count, prev) - in;
}
