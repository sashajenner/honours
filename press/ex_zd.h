#ifndef EX_ZD_H
#define EX_ZD_H

#include <stdint.h>
#include <stdio.h>

#define ASSERT(statement, ret) \
if (!(statement)) { \
    fprintf(stderr, "line %d: assertion `%s' failed\n", __LINE__, #statement); \
    return ret; \
}

uint8_t *ptr_compress_ex_zd_v0(const int16_t *ptr, size_t count, size_t *n);
int16_t *ptr_depress_ex_zd(const uint8_t *ptr, size_t count, size_t *n);

#endif
