#ifndef POVGEOMETRY_H
#define POVGEOMETRY_H

#define NOF_SECTORS 120
#define NOF_LEDS 32
#define LED_DATA_SIZE 4
#define POV_FRAME_SIZE (NOF_SECTORS * NOF_LEDS * LED_DATA_SIZE)

//millimeters x10
extern const int povledRadius[/*NOF_LEDS*/];

#endif // POVGEOMETRY_H
