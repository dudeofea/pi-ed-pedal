#ifndef _ENGINE_H_
#define _ENGINE_H_

//Half Step definitions (X is the octave)
#define NOTE_A(X)	0+(X)*12
#define NOTE_Bb(X)	1+(X)*12
#define NOTE_B(X)	2+(X)*12
#define NOTE_C(X)	3+(X-1)*12
#define NOTE_Db(X)	4+(X-1)*12
#define NOTE_D(X)	5+(X-1)*12
#define NOTE_Eb(X)	6+(X-1)*12
#define NOTE_E(X)	7+(X-1)*12
#define NOTE_F(X)	8+(X-1)*12
#define NOTE_Gb(X)	9+(X-1)*12
#define NOTE_G(X)	10+(X-1)*12
#define NOTE_Ab(X)	11+(X-1)*12

//struct definitions
typedef struct
{
	int module;			//The selected module to run
	int *inp;			//The selected module outputs to use as inputs
	int *inp_ports;		//Which output port to use in each selected input
	int *arg;			//The selected module outputs to use as arguments
	int *arg_ports;		//Which output port to use in each selected argument
} wire;

typedef struct
{
	int inp_ports;		//number of input buffers
	int out_ports;		//number of output buffers
	int arg_ports;		//number of argument buffers
	int inp_size;		//input buffer size
	int out_size;		//output buffer size
						//argument port size is set to 1
	float *inp_buf;		//buffer used to hold input data
	float *out_buf;		//buffer used to hold output data
	float *arg_buf;		//buffer used to hold argument data
						//argument data can also be placed directly in the buffer by way
						//of directly editing it in the UI

	void* aux;			//pointer to any global arguments you might need
	int aux_size;		//size of global buffer in bytes
	char* name;			//name of the effect
	void (*effect_function)(float *in, float *out, float *arg, void* aux);
} effect_module;

typedef struct
{
	int effects_alloc;			//size of effect_module buffer
	int effects_size;			//number of active effects
	effect_module* effects;		//all active effects

	int run_order_alloc;		//size of run_order buffer
	int run_order_size;			//number of wire patches
	wire *run_order;			//how to run the program
	//the last item in run_order contains which module outputs
	//to the global JACKD output
} engine_config;

typedef struct $
{
	float index;			//position of index in sample
	int sample_l;		//length of attack buffer
	float *sample_buf;	//attack sample buffer

} midi_sample;

#define JACKD_INPUT 		-1
#define JACKD_OUTPUT		-2
#define NO_INPUT			-3

void ms_log(char* str);
engine_config ms_init(void);
void ms_exit(engine_config* config);
void ms_refresh(engine_config* config);
int ms_run_engine(float* in, float* out, int len, engine_config* config);
void ms_set_effect_arg(int index, int arg_port, float val, engine_config* config);
void ms_add_effect(effect_module e, engine_config* config);
void ms_remove_effect(int index, engine_config* config);
void ms_set_output_module(int module, int port, engine_config* config);
void ms_remove_and_insert_wire(int index, int new_index, engine_config* config);
void ms_sort_wires(engine_config* config);
void ms_add_wire(wire w, engine_config* config);
void ms_remove_wire(int index, engine_config* config);
void ms_wire_alloc(wire *w, engine_config* config);
int ms_get_assoc_wire_index(int module, engine_config* config);
midi_sample ms_create_midi(const char* filename, float start, float end);

#endif