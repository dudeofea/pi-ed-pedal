The Pi'ed Pedal
===========

To start the JACK server, simply run

    jackd -P70 -p16 -t2000 -d alsa -d hw:1 -p 256 -n 3 -r 44100 -s &

Delete git identity once done
