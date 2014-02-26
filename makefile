all:
	gcc -Wall -std=c99 -c effects.c -o effects.o -lfftw3 -lm
	gcc -Wall -std=c99 -c jack_client.c -o jack_client.o -ljack -lfftw3 -lm
	gcc -Wall -std=c99 -o jack_client jack_client.o effects.o -ljack -lfftw3 -lm
	rm -rf effects.o jack_client.o
run:
	jackd -P70 -p16 -t2000 -d alsa -d hw:1 -p 256 -n 3 -r 44100 -s &
clean:
	rm -rf effects.o jack_client.o jack_client
