/** file jack_client.c
*
* mostly copied off of the simple_client.c code since it was the
* only thing around.
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include <jack/jack.h>
#include <fftw3.h>

#include "effects.h"

jack_port_t *input_port;
jack_port_t *output_port;

fftw_plan p;
fftw_complex fft_in[BUFFER_LEN], fft_out[BUFFER_LEN];

static bool running = true;

/**
* Sigint handler for shutting down client
*/
void intHandler(){
	running = false;
}

/**
* The process callback for this JACK application.
* It is called by JACK at the appropriate times.
*/
int process (jack_nframes_t nframes, void *arg)
{
        jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
        jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);

	//Copy data to FFT-input buffer
	for(int i = 0; i < BUFFER_LEN; i++){
		fft_in[i][0] = in[i];
	}
	//Run FFT
	fftw_execute(p);
	//Run all pedal effects
	run_effects(in, out, fft_out);

        //memcpy (out, in, sizeof (jack_default_audio_sample_t) * nframes);

        return 0;
}

/**
 * This is the shutdown callback for this JACK application.
 * It is called by JACK if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
	fftw_destroy_plan(p);
        exit (1);
}

int main (int argc, char *argv[])
{
        jack_client_t *client;
        const char **ports;

        /* try to become a client of the JACK server */

	jack_status_t err;
        client = jack_client_open ("pedal", JackNullOption, &err);
	if(0){
                fprintf (stderr, "jack server not running?\n");
                return 1;
        }

        /* tell the JACK server to call `process()' whenever
           there is work to be done.
        */

       jack_set_process_callback (client, process, 0);

        /* tell the JACK server to call `jack_shutdown()' if
           it ever shuts down, either entirely, or if it
           just decides to stop calling us.
        */

        jack_on_shutdown (client, jack_shutdown, 0);

        /* display the current sample rate.
         */

        printf ("engine sample rate: %" PRIu32 "\n",
                jack_get_sample_rate (client));
	/* Initialize FFT*/
	p = fftw_plan_dft_1d(BUFFER_LEN, fft_in, fft_out, FFTW_FORWARD, FFTW_MEASURE);

         /* create two ports */

        input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        /* tell the JACK server that we are ready to roll */

        if (jack_activate (client)) {
                fprintf (stderr, "cannot activate client");
                return 1;
        }

        /* connect the ports. Note: you can't do this before
           the client is activated, because we can't allow
           connections to be made to clients that aren't
           running.
        */

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

        /* Since this is just a toy, run for a few seconds, then finish */

	//Set signal handler
	signal(SIGINT, intHandler);
	while(running){
        	sleep (5);
	}
        jack_client_close (client);
	fftw_destroy_plan(p);
        exit (0);
}
