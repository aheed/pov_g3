#ifndef POVGEOMETRY_G3_H
#define POVGEOMETRY_G3_H

#define NOF_SECTORS 100
#define NOF_LEDS 64
#define LED_DATA_SIZE 4
#define POV_FRAME_SIZE (NOF_SECTORS * NOF_LEDS * LED_DATA_SIZE)

//millimeters x10
extern const int povledRadius[/*NOF_LEDS*/];

#endif // POVGEOMETRY_G3_H
