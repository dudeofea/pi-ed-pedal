The Pi'ed Pedal
===========

To start the JACK server, simply run

    make run


How to setup from nothing
-------------------------

First, burn the following [image](http://www.emlid.com/raspberry-pi-real-time-kernel/) (or any other low latency kernel) onto the pi's SD card

run an update and install some required libraries:

    sudo apt-get update
    sudo apt-get install libfftw3-dev libncurses5-dev
    
Next to fix some issues with jackd2 on the pi. first add the following to you ~/.bashrc:

    export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket
    
then, add the following under the default policy in /etc/dbus-1/system.conf:

    <allow own="org.freedesktop.ReserveDevice1.Audio0"/>
