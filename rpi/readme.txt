To make rpi zero w automount a network share:

Edit /etc/fstab
Add this line:
//192.168.0.16/src /media/networkshare/julius_src cifs guest,uid=1000,gid=1000,iocharset=utf8 0 0

Enable the option in raspi-config for ‘wait for network on boot’

To enable HW SPI:
Enable the corresponding option in raspi-config to automatically load SPI drivers



