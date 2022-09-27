#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "../header/wavelib.h"

double absmax(int16_t *array, int N) {
	int16_t max;
	int i;

	max = 0.0;
	for (i = 0; i < N; ++i) {
		if (abs(array[i]) >= max) {
			max = abs(array[i]);
		}
	}

	return max;
}

int main(int argc, char **argv) {
	wave_object obj;
	wt_object wt;
	double *inp,*out;
	int16_t *diff;
	int i,J;
	size_t sz;
	size_t N;

	FILE *ifp;
	uint16_t *temp;

	char *name = "db4";
	obj = wave_init(name);// Initialize the wavelet

	ifp = fopen(argv[1], "r");
	i = 0;
	if (!ifp) {
		printf("Cannot Open File");
		exit(100);
	}

	fseek(ifp, 0L, SEEK_END);
	sz = ftell(ifp);
	fseek(ifp, 0L, SEEK_SET);
	N = sz / sizeof *temp;

	temp = malloc(sz);
	fread(temp, sizeof *temp, N, ifp);

	fclose(ifp);

	inp = (double*)malloc(sizeof(double)* N);
	out = (double*)malloc(sizeof(double)* N);
	diff = malloc(sizeof *diff * N);
	//wmean = mean(temp, N);

	for (i = 0; i < N; ++i) {
		inp[i] = temp[i];
		//printf("%g \n",inp[i]);
	}
	J = 3;

	wt = wt_init(obj, "dwt", N, J);// Initialize the wavelet transform object
	setDWTExtension(wt, "sym");// Options are "per" and "sym". Symmetric is the default option
	setWTConv(wt, "direct");

	dwt(wt, inp);// Perform DWT
	//DWT output can be accessed using wt->output vector. Use wt_summary to find out how to extract appx and detail coefficients

	/*
	for (i = 0; i < wt->outlength; ++i) {
		printf("%g ",wt->output[i]);
	}
	*/

	idwt(wt, out);// Perform IDWT (if needed)
	// Test Reconstruction
	for (i = 0; i < wt->siglength; ++i) {
		diff[i] = (int16_t) out[i] - inp[i];
	}

	printf("\n MAX %g \n", absmax(diff, wt->siglength)); // If Reconstruction succeeded then the output should be a small value.

	wt_summary(wt);// Prints the full summary.
	wave_free(obj);
	wt_free(wt);

	free(inp);
	free(out);
	free(diff);
	return 0;
}
