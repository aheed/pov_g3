#ifndef LEDSTRIP_H
#define LEDSTRIP

#include <stdint.h>

int ledstrip_init();
void ledstrip_setcolors(uint8_t* pleddata);

#endif // LEDSTRIP
