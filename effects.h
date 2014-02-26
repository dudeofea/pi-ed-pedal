#ifndef _EFFECTS_H_
#define _EFFECTS_H_
#include <fftw3.h>

#define BUFFER_LEN 256

int run_effects(float* in, float* out, fftw_complex* fft_out);

#endif
