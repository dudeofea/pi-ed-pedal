#ifndef _EFFECTS_H_
#define _EFFECTS_H_
#include <fftw3.h>
#include <math.h>
#include "engine.h"

#define BUFFER_LEN 1024
#define SAMPLE_RATE	44100
//#define BUFFER_LEN 20

void init_effects(engine_config* config);
void myeffect2(float *in, float *out, float *arg, void* aux);
void print_array(float *arr, int size);

#endif
