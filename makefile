CC=gcc
CFLAGS=-Wall -std=c99
LFLAGS=-lm -lfftw3 -lncurses -ljack

all:
	$(CC) $(CFLAGS) -c engine.c -o engine.o
	$(CC) $(CFLAGS) -c effects.c -o effects.o
	$(CC) $(CFLAGS) -c pied-pedal.c -o pied-pedal.o
	$(CC) $(CFLAGS) -o pied-pedal *.o $(LFLAGS)
	rm -rf *.o
run:
	jackd -P80 -p16 -t2000 -d alsa -d hw:1 -p 1024 -n 2 -r 44100 -s &
clean:
	rm -rf *.o pied-pedal
