#ifndef TALLY_U64_H
#define TALLY_U64_H

#include <stdint.h>
#include <slow5/slow5.h>

uint64_t *gettally_readlen_slow5(struct slow5_file *fp, uint64_t *min,
				 uint64_t *max);
void printtally_u64(const uint64_t *tally, uint64_t min, uint64_t max);

#endif /* TALLY_U64_H */
