#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>
#include "engine.h"
#include "effects.h"

jack_port_t *input_port;
jack_port_t *output_port;

int process (jack_nframes_t nframes, void *arg);

int main(int argc, char const *argv[])
{
	engine_config config = ms_init();
	init_effects(&config);
	/* Setup jack client */
	//try to become a client of the JACK server
	jack_client_t *client;
	const char **ports;
	jack_status_t err;
	client = jack_client_open ("pedal", JackNullOption, &err);
	if(client == NULL){
		fprintf (stderr, "jack server not running?\n");
		return 1;
	}
	//bind middle man callback
	jack_set_process_callback (client, process, &config);
	//create two ports
	input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	//activate client
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}
	//connect the ports
	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
		fprintf(stderr, "Cannot find any physical capture ports\n");
		exit(1);
	}
	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}
	free (ports);
	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		fprintf(stderr, "Cannot find any physical playback ports\n");
		exit(1);
	}

	if (jack_connect (client, jack_port_name (output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}
		free (ports);
	/* Run main gui refresh loop */
	//while(mgui_refresh(&config) >= 0){ ; }
	jack_client_close(client);
	ms_exit(&config);
	return 0;
}

//process jackd input samples and output samples
int process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
	jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);
	//get configuration
	engine_config* config = (engine_config*)arg;
	//Run all pedal effects
	ms_run_engine(in, out, BUFFER_LEN, config);
	return 0;
}