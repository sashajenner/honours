#ifndef PRESS_H
#define PRESS_H

#include <stdint.h>

uint64_t none_bound(const int16_t *in, uint64_t nin);
uint64_t none_press(const int16_t *in, uint64_t nin, uint8_t *out);
uint64_t none_depress(const uint8_t *in, uint64_t nin, int16_t *out);

/* store each int16_t as uintx_t */
uint64_t uintx_bound(uint8_t x, const int16_t *in, uint64_t nin);
uint64_t uintx_press(uint8_t x, const int16_t *in, uint64_t nin, uint8_t *out);
uint64_t uintx_depress(uint8_t x, const uint8_t *in, uint64_t nin, int16_t *out);

static inline uint64_t uint11_bound(const int16_t *in, uint64_t nin)
{
	return uintx_bound(11, in, nin);
}
static inline uint64_t uint11_press(const int16_t *in, uint64_t nin, uint8_t *out)
{
	return uintx_press(11, in, nin, out);
}
static inline uint64_t uint11_depress(const uint8_t *in, uint64_t nin, int16_t *out)
{
	return uintx_depress(11, in, nin, out);
}

#endif /* press.h */
