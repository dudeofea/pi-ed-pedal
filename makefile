all:
	gcc -Wall -std=c99 -o jack_client jack_client.c -Wall -ljack -lfftw3 -lm
run:
	jackd -P70 -p16 -t2000 -d alsa -d hw:1 -p 128 -n 3 -r 44100 -s &
