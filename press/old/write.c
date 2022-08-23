#include <stdio.h>

int main(void)
{
	int x = 2;
	FILE *fp = fopen("test", "w");
	fwrite(&x, sizeof (int), 1, fp);
	fclose(fp);
}
