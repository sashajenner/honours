#include <stdint.h>
#include <stdio.h>

uint64_t vbytebound(uint32_t *in, uint64_t nin);

int main(void)
{
	uint32_t in[1];
	uint64_t nin;
	uint64_t nout;

	in[0] = 200;
	nin = 1;

	nout = vbytebound(in, nin);
	printf("%zu\n", nout);

	return 0;
}
