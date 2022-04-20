#ifndef GETSIG_H
#define GETSIG_H

#include <stdint.h>

#define MAX_WIDTH_INT16 (6)
#define MAX_WIDTH_LINE (MAX_WIDTH_INT16 + 2) /* \n + \0 */

int getsig(char *line, int16_t *sig);
int getnextsig(FILE *fp, char *line, size_t n, uint16_t *x);
void skipline(FILE *fp);

#endif
