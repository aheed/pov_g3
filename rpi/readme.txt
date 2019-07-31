Persistence Of Vision project
  Raspberry Pi
  Adafruit Dotstar strip, HW SPI
  Interrupt driven rev synch, wiringPi

$ gcc ldserver.c ns_clock.c ledstrip.c revsensor.c pov.c -o pov -lwiringPi -lrt -lm -lpthread
$ sudo ./pov


---------------------------------------------------
To make rpi zero w automount a network share:

Edit /etc/fstab
Add this line:
//192.168.0.16/src /media/networkshare/julius_src cifs guest,uid=1000,gid=1000,iocharset=utf8 0 0

Enable the option in raspi-config for ‘wait for network on boot’
If it fails to mount automatically on startup for some reason it can be mounted later with this command:
sudo mount -a

To enable HW SPI:
Enable the corresponding option in raspi-config to automatically load SPI drivers



