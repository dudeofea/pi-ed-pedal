#include "effects.h"

void run_effects(float* in, float* out, fftw_complex* fft_out){
	double max = 0;
	static int max_index = 0;
	for(int i = 0; i < BUFFER_LEN; i++){
		if(fft_out[i][0] > 0.2){
			//printf("%d ", i);
		}
		out[i] = 3*in[i];
	}
}
