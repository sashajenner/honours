#ifndef TALLY_H
#define TALLY_H

#include <stdio.h>
#include <stdint.h>

uint64_t *gettally(FILE *fp, uint16_t *min, uint16_t *max);
void printtally(const uint64_t *tally, uint16_t min, uint16_t max);
void printprob(const uint64_t *tally, uint16_t min, uint16_t max);

#endif
