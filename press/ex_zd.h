#ifndef EX_ZD_H
#define EX_ZD_H

#include <stdint.h>
#include <stdio.h>

#define MAX_QTS_SEARCH (5)

uint8_t findo_qts(const int16_t *s, uint64_t n, uint8_t max, int16_t **qs);
void do_rev_qts_inplace(int16_t *s, uint64_t n, uint8_t q);
void do_rev_qts_inplace_u16(uint16_t *s, uint64_t n, uint8_t q);
void do_rev_qts_inplace_32(int32_t *s, uint64_t n, uint8_t q);
void do_rev_qts_inplace_u32(uint32_t *s, uint64_t n, uint8_t q);
uint8_t *ptr_compress_ex_zd_v0(const int16_t *ptr, size_t count, size_t *n);
int16_t *ptr_depress_ex_zd(const uint8_t *ptr, size_t count, size_t *n);

#endif
