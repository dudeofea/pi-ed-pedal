#include <math.h>

#include "effects.h"

int run_effects(float* in, float* out, fftw_complex* fft_out){
	double val = 0;
	double sum = 0;
	double tmp = 0;
	for(int i = 0; i < BUFFER_LEN; i++){
		tmp = fft_out[i][0]*fft_out[i][0];
		val += tmp*i;
		sum += tmp;
		out[i] = 3*in[i];
	}
	val /= sum;
	//if(val > 1)
	//	printf("%lf\n", val);
	return 0;
}
