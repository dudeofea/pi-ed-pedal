#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"

#define AUDIO_INBUF_SIZE	20480
#define AUDIO_REFILL_THRESH 4096

/*int effects_alloc;		//size of effect_module buffer
int effects_size;			//number of active effects
effect_module *effects;		//all active effects

int run_order_alloc;		//size of run_order buffer
int run_order_size;			//number of wire patches
wire *run_order;			//how to run the program
//the last item in run_order contains which module outputs
//to the global JACKD output*/

void print_wire(wire w, engine_config* config){
	if (w.module < 0)
	{
		if (w.module == JACKD_OUTPUT)
			printf("Module: Global Output\n");
		if(w.inp[0] < 0){
			if (w.inp[0] == JACKD_INPUT){
				printf("Inputs: Global\n");
			}
		}else{
			printf("Inputs: %d[%d]", w.inp[0], w.inp_ports[0]);
		}
		printf("Arguments: None\n");
	}else{
		printf("Module: %d\n", w.module);
		effect_module tmp = config->effects[w.module];
		printf("Inputs: ");
		for (int i = 0; i < tmp.inp_ports; ++i)
		{
			if(w.inp[i] < 0){
				if (w.inp[i] == JACKD_INPUT){
					printf(" Global");
				}
			}else{
				printf(" %d[%d]", w.inp[i], w.inp_ports[i]);
			}
		}
		printf("\n");
		printf("Arguments: ");
		for (int i = 0; i < tmp.arg_ports; ++i)
		{
			if (w.arg[i] < 0)
			{
				printf(" None");
			}else{
				printf(" %d[%d]", w.arg[i], w.arg_ports[i]);
			}
		}
		printf("\n");
	}
}

void print_all_wires(engine_config* config){
	for (int i = 0; i < config->run_order_size; ++i)
	{
		print_wire(config->run_order[i], config);
	}
}

void print_effect(effect_module e){
	printf("-----%s-----\n", e.name);
	printf("Input: %d ports, %d samples each\n", e.inp_ports, e.inp_size);
	printf("Input Buffer:\n");
	for (int i = 0; i < e.inp_ports; ++i)
	{
		printf("Port %d: ", i);
		for (int j = 0; j < e.inp_size; ++j)
		{
			printf("%f ", e.inp_buf[i*e.inp_size+j]);
		}
		printf("\n");
	}
	printf("Output:%d ports, %d samples each\n", e.out_ports, e.out_size);
	printf("Output Buffer:\n");
	for (int i = 0; i < e.out_ports; ++i)
	{
		printf("Port %d: ", i);
		for (int j = 0; j < e.out_size; ++j)
		{
			printf("%f ", e.out_buf[i*e.out_size+j]);
		}
		printf("\n");
	}
	printf("Arguments: %d ports\n", e.arg_ports);
}

//Logging library to track bugs
void ms_log(char* str){
	FILE *f = fopen("log.txt", "a");
	//get time
	time_t timer;
	char buffer[25] = {};
	struct tm* tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
	fprintf(f, "[%s] %s\n", buffer, str);
	fclose(f);
	f = NULL;
}

//allocates all data used in input/output buffers
//and preps everything so it can be run
void ms_refresh(engine_config* config){
	//allocate buffers if they don't exist
	for (int i = 0; i < config->effects_size; ++i)
	{
		if(config->effects[i].inp_buf == NULL){
			config->effects[i].inp_buf = (float*)malloc(config->effects[i].inp_size * config->effects[i].inp_ports * sizeof(float));
		}
		if(config->effects[i].out_buf == NULL){
			config->effects[i].out_buf = (float*)malloc(config->effects[i].out_size * config->effects[i].out_ports * sizeof(float));
		}
		if(config->effects[i].arg_buf == NULL){
			config->effects[i].arg_buf = (float*)malloc(1 * config->effects[i].arg_ports * sizeof(float));
		}
	}
	ms_sort_wires(config);
}

//init mothership
engine_config ms_init(){
	engine_config config;
	config.effects_alloc = 10;
	config.effects_size = 0;
	config.effects = (effect_module*)malloc(config.effects_alloc * sizeof(effect_module));
	config.run_order_alloc = 10;
	config.run_order_size = 1;
	config.run_order = (wire*)malloc(config.run_order_alloc * sizeof(wire));
	//set output wire to input
	wire w = {
		JACKD_OUTPUT, NULL, NULL, NULL, NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	config.run_order[0] = w;
	return config;
}

//close mothership and free all memory
void ms_exit(engine_config* config){
	//free run_order
	for (int i = 0; i < config->run_order_size; ++i)
	{
		if (config->run_order[i].inp != NULL)
			free(config->run_order[i].inp);
		if (config->run_order[i].inp_ports != NULL)
			free(config->run_order[i].inp_ports);
		if (config->run_order[i].arg != NULL)
			free(config->run_order[i].arg);
		if (config->run_order[i].arg_ports != NULL)
			free(config->run_order[i].arg_ports);
	}
	if (config->run_order != NULL)
		free(config->run_order);
	//free effects
	for (int i = 0; i < config->effects_size; ++i)
	{
		if(config->effects[i].inp_buf != NULL)
			free(config->effects[i].inp_buf);
		if(config->effects[i].out_buf != NULL)
			free(config->effects[i].out_buf);
		if(config->effects[i].arg_buf != NULL)
			free(config->effects[i].arg_buf);
	}
	if (config->effects != NULL)
		free(config->effects);
}

//runs all effects
int ms_run_engine(float* in, float* out, int len, engine_config* config){
	wire current;		//current wire
	effect_module tmp;
	int ports, size;
	//printf("# of effects: %d, # of wires: %d\n", config->effects_size, config->run_order_size);
	for (int i = 0; i < config->run_order_size - 1; ++i)
	{
		//printf("iteration %d\n", i+1);
		current = config->run_order[i];
		//copy inputs
		ports = config->effects[current.module].inp_ports;
		size = config->effects[current.module].inp_size;
		for (int j = 0; j < ports; ++j)
		{
			if (current.inp[j] >= 0)
			{
				//copy from other module
				//printf("Input: Module %d\n", current.inp[j]);
				tmp = config->effects[current.inp[j]];
				//print_effect(tmp);
				memcpy(config->effects[current.module].inp_buf + j*size, 
					tmp.out_buf + current.inp_ports[j]*tmp.out_size, size * sizeof(float));
			}else if (current.inp[j] == JACKD_INPUT)
			{
				//copy from global input
				//printf("Input: Global\n");
				memcpy(config->effects[current.module].inp_buf + j*size, in, size * sizeof(float));
			}
		}
		//copy arguments
		ports = config->effects[current.module].arg_ports;
		size = 1;
		for (int j = 0; j < ports; ++j)
		{
			if (current.arg[j] != NO_INPUT){
				//printf("Argument: Module %d\n", current.arg[j]);
				tmp = config->effects[current.arg[j]];
				memcpy(config->effects[current.module].arg_buf + j*size, 
					tmp.out_buf + current.arg_ports[j]*tmp.out_size, size * sizeof(float));
			}
		}
		//printf("running function\n");
		//run the module function
		tmp = config->effects[current.module];
		tmp.effect_function(tmp.inp_buf, tmp.out_buf, tmp.arg_buf, tmp.aux);
	}
	//output to jackd
	current = config->run_order[config->run_order_size - 1];
	if (current.module != JACKD_OUTPUT)
	{
		//printf("Something's not wired right\n");
		return -1;
	}
	if(current.inp[0] == JACKD_INPUT){
		//printf("Outputting from Global Input\n");
		//no effects
		memcpy(out, in, len * sizeof(float));
	}else{
		//printf("Outputting from Module %d\n", current.inp[0]);
		//an effect is attached to the output
		tmp = config->effects[current.inp[0]];
		memcpy(out, tmp.out_buf + current.inp_ports[0]*tmp.out_size, len * sizeof(float));
	}
	return 0;
}

void ms_set_effect_arg(int index, int arg_port, float val, engine_config* config){
	config->effects[index].arg_buf[arg_port] = val;
}

//Adds a effect to the list of active effects.
//Assumes there is room then doubles the size
//if neccessary.
void ms_add_effect(effect_module e, engine_config* config){
	//copy to array
	config->effects[config->effects_size] = e;
	//create new global buffer so nothing conflicts
	config->effects[config->effects_size].aux = malloc(config->effects[config->effects_size].aux_size);
	//copy over init values
	if (config->effects[config->effects_size].aux != NULL){
		//copy from old values or set to 0
		if(e.aux != NULL)
			memcpy(config->effects[config->effects_size].aux, e.aux, e.aux_size);
		else
			memset(config->effects[config->effects_size].aux, 0, e.aux_size);
	}
	//clear other buffers just in case
	config->effects[config->effects_size].inp_buf = NULL;
	config->effects[config->effects_size].out_buf = NULL;
	config->effects[config->effects_size].arg_buf = NULL;
	config->effects_size++;
	if (config->effects_size > config->effects_alloc)
	{
		config->effects_alloc *= 2;		//double array size
		config->effects = (effect_module*)realloc(config->effects, 
			config->effects_alloc * sizeof(effect_module));
	}
	ms_refresh(config);
}

//TODO: re-index all wires when this happens
//Removes effect at specified index
void ms_remove_effect(int index, engine_config* config){
	effect_module* effects = config->effects;
	wire* run_order = config->run_order;
	//if out of bounds
	if (index < 0 || index >= config->effects_size)
		return;
	//deallocate
	free(effects[index].inp_buf);
	effects[index].inp_buf = NULL;
	free(effects[index].out_buf);
	effects[index].out_buf = NULL;
	free(effects[index].arg_buf);
	effects[index].arg_buf = NULL;
	//downshift everything
	for (int i = index; i < config->effects_size - 2; i++)
	{
		effects[i] = effects[i+1];
	}
	//remove all wires referencing that module
	for (int i = 0; i < config->run_order_size; ++i)
	{
		//remove if equal or decrement if larger
		if (run_order[i].module == index)
		{
			ms_remove_wire(i, config);
		}else if (run_order[i].module > index)
		{
			run_order[i].module--;
		}
		//remove if referencing
		effect_module tmp = effects[run_order[i].module];
		for (int j = 0; j < tmp.inp_ports; ++j)
		{
			//remove if equal or decrement if larger
			if (run_order[i].inp[j] == index)
			{
				ms_remove_wire(i, config);
			}else if (run_order[i].inp[j] > index)
			{
				run_order[i].inp[j]--;
			}
		}
		for (int j = 0; j < tmp.arg_ports; ++j)
		{
			//remove if equal or decrement if larger
			if (run_order[i].arg[j] == index)
			{
				ms_remove_wire(i, config);
			}else if (run_order[i].arg[j] > index)
			{
				run_order[i].arg[j]--;
			}
		}
	}
}

//sets which module outputs to JACKD_OUTPUT
void ms_set_output_module(int module, int port, engine_config* config){
	config->run_order[config->run_order_size - 1].inp[0] = module;
	config->run_order[config->run_order_size - 1].inp_ports[0] = port;
}

void ms_remove_and_insert_wire(int index, int new_index, engine_config* config){
	wire* run_order = config->run_order;
	//if not equal
	if (index == new_index)
		return;
	//if out of bounds
	if (index < 0 || index >= config->run_order_size)
		return;
	wire w = run_order[index];
	if (index < new_index)
	{
		//downshift everything
		for (int i = index; i < new_index; i++)
		{
			run_order[i] = run_order[i+1];
		}
	}else{
		//upshift everything
		for (int i = index - 1; i >= new_index; i--)
		{
			run_order[i+1] = run_order[i];
		}
	}
	//if out of bounds
	if (new_index < 0 || new_index >= config->run_order_size)
		return;
	run_order[new_index] = w;
}

//Adds a patch cable to a specified index
void add_wire_to_index(wire w, int index, engine_config* config){
	wire* run_order = config->run_order;
	//printf("adding wire to %d\n", index);
	config->run_order_size++;
	if (config->run_order_size > config->run_order_alloc)
	{
		config->run_order_alloc *= 2;		//double array size
		config->run_order = (wire*)realloc(config->run_order, 
			config->run_order_alloc * sizeof(wire));
	}
	//upshift everything
	for (int i = config->run_order_size - 1; i >= index; i--)
	{
		run_order[i+1] = run_order[i];
	}
	run_order[index] = w;
}

//Finds the index of the wire associated with 
//a certain module
int ms_get_assoc_wire_index(int module, engine_config* config){
	wire* run_order = config->run_order;
	for (int i = 0; i < config->run_order_size; ++i)
	{
		if (run_order[i].module == module)
		{
			return i;
		}
	}
	return -1;
}

//Makes sure the wires have their
//dependencies met
void ms_sort_wires(engine_config* config){
	effect_module* effects = config->effects;
	wire* run_order = config->run_order;
	for (int i = 0; i < config->run_order_size - 1; ++i)
	{
		//print_wire(run_order[i]);
		int insert_index = 0;
		int assoc_index = 0;
		effect_module tmp = effects[run_order[i].module];
		//must come after all it's inputs
		for (int j = 0; j < tmp.inp_ports; ++j)
		{
			assoc_index = ms_get_assoc_wire_index(run_order[i].inp[j], config);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index;
			}
		}
		//and arguments
		for (int j= 0; j < tmp.arg_ports; ++j)
		{
			assoc_index = ms_get_assoc_wire_index(run_order[i].arg[j], config);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index;
			}
		}
		//printf("%d belongs in %d\n", i, insert_index);
		if (insert_index > i)
		{
			ms_remove_and_insert_wire(i, insert_index, config);
			i--;
			continue;
		}
	}
}

//Adds a patch to the list of patch cables.
//Assumes there is room then doubles the size
//if neccessary. Assumes the wire's inp / inp_ports
//have already been alocated
void ms_add_wire(wire w, engine_config* config){
	//if effect exists
	if (w.module < config->effects_size){
		int insert_index = 0;
		int assoc_index = 0;
		effect_module tmp = config->effects[w.module];
		//must come after all it's inputs
		for (int i = 0; i < tmp.inp_ports; ++i)
		{
			assoc_index = ms_get_assoc_wire_index(w.inp[i], config);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index + 1;
				//printf("wire moved to %d\n", insert_index);
			}
		}
		//and arguments
		for (int i = 0; i < tmp.arg_ports; ++i)
		{
			assoc_index = ms_get_assoc_wire_index(w.arg[i], config);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index + 1;
				//printf("wire moved to %d\n", insert_index);
			}
		}
		add_wire_to_index(w, insert_index, config);
	}
}

//Removes patch cable at specified index
void ms_remove_wire(int index, engine_config* config){
	wire* run_order = config->run_order;
	//if out of bounds
	if (index < 0 || index >= config->run_order_size)
		return;
	//downshift everything
	for (int i = index; i < config->run_order_size; i++)
	{
		run_order[i] = run_order[i+1];
	}
	config->run_order_size--;
}

//allocates memory for a wire, assumes null pointers
void ms_wire_alloc(wire *w, engine_config* config){
	int i_size;
	int a_size;
	if (w->module != JACKD_OUTPUT){
		i_size = config->effects[w->module].inp_ports;
		a_size = config->effects[w->module].arg_ports;
	}else{
		i_size = 1;
		a_size = 0;
	}
	w->inp = (int*)malloc(i_size * sizeof(int));
	w->inp_ports = (int*)malloc(i_size * sizeof(int));
	w->arg = (int*)malloc(a_size * sizeof(int));
	w->arg_ports = (int*)malloc(a_size * sizeof(int));
	//close all ports
	for (int i = 0; i < i_size; ++i)
	{
		w->inp[i] = NO_INPUT;
	}
	for (int i = 0; i < a_size; ++i)
	{
		w->arg[i] = NO_INPUT;
		config->effects[w->module].arg_buf[i] = 0.0f;
	}
}

//Creates a midi sample object given the attack, decay and release times
midi_sample ms_create_midi(const char* filename, float start, float end){
	midi_sample smp = {0, 0, NULL};

	//delete if exists
	system("rm -f sample.raw");

	//convert to raw 32-bit float
	char command[200];
	sprintf(command, "avconv -i %s -ac 1 -f f32le sample.raw -loglevel quiet", filename);
	system(command);

	int bytes, size;
	FILE* f = fopen("sample.raw", "rb");
	if(f == NULL){
		ms_log("File not found");
		return smp;
	}
	fseek(f, 0L, SEEK_END);
	size = ftell(f);
	rewind(f);
	float* samples = (float*)malloc(size);
	int pos = 0;
	float val = 0;
	//float max = 0.0;
	while((bytes = fread(&val, sizeof(float), 1, f))){
		samples[pos] = val;
		pos++;
	}
	//TODO: normalize

	//copy to struct
	smp.sample_l = size / sizeof(float);
	smp.sample_buf = samples;

	//clean up
	system("rm -f sample.raw");

	return smp;
}