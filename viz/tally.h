#ifndef TALLY_H
#define TALLY_H

#include <stdio.h>
#include <stdint.h>

#define COL1 "signal"
#define SEP "\t"
#define PRINTHDR(col1, col2) printf(col1 SEP col2 "\n")
#define PRINTSTDHDR(col2) PRINTHDR(COL1, col2)

uint64_t *gettally(FILE *fp, uint16_t *min, uint16_t *max);
void printtally(const uint64_t *tally, uint16_t min, uint16_t max);
void printprob(const uint64_t *tally, uint16_t min, uint16_t max);
void printparity(const uint64_t *tally, uint16_t min, uint16_t max);

#endif
