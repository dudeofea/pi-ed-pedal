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
    
then, add the following under the default policy in /etc/dbus-1/system.conf to fix a permissions issue:

    <allow own="org.freedesktop.ReserveDevice1.Audio0"/>
    
then, add this to /boot/cmdline.txt to reduce USB speeds to 1.1 so the sound card can full-duplex:

    dwc_otg.speed=1

then, remove the existing jackd2 packages and replace them with patched ones:

    sudo apt-get remove jackd jackd2 libjackd0 libjack-jackd2-0
    wget http://asbradbury.org/tmp/raspi/jackd2_1.9.8~dfsg.4+20120529git007cdc37-5+rpi2_armhf.deb 
    wget http://asbradbury.org/tmp/raspi/libjack-jackd2-0_1.9.8~dfsg.4+20120529git007cdc37-5+rpi2_armhf.deb
    sudo dpkg -i *jackd2*.deb
    sudo apt-get install libjack-jackd2-dev
    
I might be missing a couple jackd libraries in that apt-get, but just be sure to purge yourself of all jackd's before intalling the downloaded deb files
