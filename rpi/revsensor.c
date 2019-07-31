#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include "revsensor.h"

int revsensor_init(void (*revisr)(void))
{
  if (wiringPiSetup () == -1)
    return 2 ;
  
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 to generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (5, INT_EDGE_FALLING, revisr) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  return 0;
}
