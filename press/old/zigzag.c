#include <stdio.h>
#include <inttypes.h>

uint32_t zigzag(int32_t x)
{
	return (x + x) ^ (x >> 31);
}

int main(void)
{
	int32_t in[] = {1039,-451,0,5,-7,-12,-4,15,3,-2,-8,4,2,2,11};
	uint32_t out;

	for (int i = 0; i < sizeof(in) / sizeof(int32_t); ++ i) {
		if (in[i] < 0) {
		printf("%x\n", in[i]);
		printf("%x\n", in[i] + in[i]);
		printf("%x\n\n", zigzag(in[i]));
		}
	}

	return 0;
}
