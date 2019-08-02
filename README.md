# pov_g3

Persistence Of Vision display, generation 3

Led data server


## Hardware
Adafruit Dotstar LEDs (64 of them)

Adafruit Feather HUZZAH ESP8266

LiPoly batteries to power the 8266 and the LEDs

Light detector TSL250R to sync LED display software with display rotation

Table fan

LED strip mounted on balsa wood

Hot glue, metal strips, metal wire

### Schematics

### Other HW
LiPoly battery charger

Stationary LED with separate power source to make the rotating light detector trigger an interrupt on a completed revolution.

## Sketch

Use Arduino IDE to compile and upload to the ESP8266 board

## Pic client

## Video client

libavcodec/avcodec.h:
sudo apt-get install libavcodec-dev

libavformat/avformat.h:
sudo apt-get install libavformat-dev

libswscale/swscale.h:
sudo apt-get install libswscale-dev

picclient_gen3.c


## povsim

Web based simulator

Led data server compatible with the same clients (picclient_gen3 etc) as the real POV display.
Pushes received LED data updates to any connected web browser via SignalR core.

### To run


```
$ cd povsim
$ libman restore
$ dotnet run
```

Browse to http://localhost:5201 with any web browser
```
$ cd ..
$ ./picg3 127.0.0.1 test_square.bmp  -a4 -b 255 -r -40
```


### Client parameters

For best result with simulator:

* Leave out g (Do not use gamma correction)
* -b 255 (max Brightness)
* -r -40 (Rotation)



# G4
rpi zero w
lipo shim
lipo batteries (separate for rpi and led strip)
Hall effect sensor TLV49645 + magnet for rev synch
More advanced LED geometry
Frame sequence stored in server and replayed at fixed rate
All components mounted on revolving Vinyl LP record



