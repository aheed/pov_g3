/* 
Persistence Of Vision project
  Raspberry Pi
  Adafruit Dotstar strip, HW SPI
  Interrupt driven rev synch, wiringPi

$ gcc bmp.c ldserver.c ns_clock.c ledstrip.c pov.c -o pov -lwiringPi -lrt -lm -lpthread
$ sudo ./pov

*/


#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <wiringPi.h>
#include "../common/ledconfig.h"
#include "ldserver.h"
#include "ns_clock.h"
#include "ledstrip.h"





#define SECTOR_DATA_SIZE (sizeof(leddata) / (MAX_FRAMES_IN_BUFFER * NOF_SECTORS)) //bytes per sector

#define SLEEP_PER_LOOP 10000 //500000 // nanoseconds

#define MIN_REV_TIME 1000000 //nsec
#define FRAMES_PER_SEC 24
#define FRAME_TIME_NS (1000000000 / FRAMES_PER_SEC) //nsec

#define REV_TIME_AVG_COEFFICIENT 10 //2
#define REV_TIME_CORRECTION_COEFFICIENT 2

#define MAX_FRAMES_IN_BUFFER 1000
#define BUFFER_SIZE (MAX_FRAMES_IN_BUFFER * POV_FRAME_SIZE)

volatile int eventCounter = 0;
volatile int oldeventCounter = 0;

static uint8_t leddata[BUFFER_SIZE] = {0};


// -------------------------------------------------------------------------
void myInterrupt(void) {
   eventCounter++;
}

void main_loop_sleep()
{
  struct timespec sleeper, dummy;
  sleeper.tv_sec  = 0;
  sleeper.tv_nsec = SLEEP_PER_LOOP;
  nanosleep(&sleeper, &dummy);
}


// -------------------------------------------------------------------------
int main( int argc, char* args[] )
{
  int i;  
  int led, sector;
  nsc_time_t rev_start_time = 0;
  nsc_timeperiod_t time_since_rev_start;
  nsc_timeperiod_t last_rev_time = 0;
  nsc_timeperiod_t avg_rev_time = 0;
  nsc_time_t current_time = 0;
  nsc_time_t rev_start_time_calc = 0;
  nsc_timeperiod_t time_since_rev_start_calc = 0;
  nsc_timeperiod_t start_time_diff = 0;
  nsc_timeperiod_t max_start_time_diff = 0;
  nsc_time_t frame_start_time = 0;
  nsc_timeperiod_t time_since_frame_start = 0;

  unsigned int current_sector = 93; //current orientation of LED strip
  unsigned int last_sector = 93;
  unsigned int current_frame = 0;

  printf("\n\n");
  printf("NOF_SECTORS=%d\nNOF_LEDS=%d\n\n", NOF_SECTORS, NOF_LEDS);

  if (ledstrip_init())
  {
    printf("Failed to init ledstrip\n");
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Set up interrupt routine for rev synch with WiringPi

  if (wiringPiSetup () == -1)
    return 2 ;
  
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 to generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (5, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 3;
  }


  if(LDListen(leddata, BUFFER_SIZE, POV_FRAME_SIZE))
  {
    fprintf (stderr, "Failed to setup server\n");
    return 4;
  }

  while(1)
  {
    current_time = nsc_get_current_time();
    time_since_rev_start = nsc_diff_time_time(current_time, rev_start_time);


    //-----------------------------------------------
    time_since_rev_start_calc = nsc_diff_time_time(current_time, rev_start_time_calc);

    while((avg_rev_time != 0) && (time_since_rev_start_calc > avg_rev_time))
    {
      // By now a new rev should have started.
      time_since_rev_start_calc = time_since_rev_start_calc - avg_rev_time;
      rev_start_time_calc = nsc_add_time_period(rev_start_time_calc, avg_rev_time);
    }
    //---------------------------------------------------

    ///////////////////////////////////////////
    // Handle any received rev synch interrupt
    if(oldeventCounter != eventCounter)
    {
      if(time_since_rev_start > MIN_REV_TIME)
      {
        rev_start_time = current_time;


        if( ((last_rev_time - time_since_rev_start) < (last_rev_time / 10)) &&
            ((time_since_rev_start - last_rev_time) < (last_rev_time / 10)) )
        {
          // Rev time differs little from last rev:
          // It is believable so let the last rev contribute to avg
          avg_rev_time = (avg_rev_time * (REV_TIME_AVG_COEFFICIENT-1) + time_since_rev_start) / REV_TIME_AVG_COEFFICIENT;
        }

        last_rev_time = time_since_rev_start;

        
        // Adjust calculated rev_start_time     
        start_time_diff = nsc_diff_time_time(rev_start_time, rev_start_time_calc);

        //Handle rev wrap
        start_time_diff = nsc_wrap_period(start_time_diff, avg_rev_time);

        rev_start_time_calc = nsc_add_time_period(rev_start_time_calc, start_time_diff / REV_TIME_CORRECTION_COEFFICIENT);
      }

      oldeventCounter = eventCounter;
    }

    

    ///////////////////////////////////////////
    // Update sector index
    if(avg_rev_time != 0)
    {
//      current_sector = (NOF_SECTORS * time_since_rev_start) / avg_rev_time;
      current_sector = (NOF_SECTORS * time_since_rev_start_calc) / avg_rev_time;
      while(current_sector >= NOF_SECTORS)
      {
        current_sector -= NOF_SECTORS;
      }
 
      if(current_sector != last_sector)
      {        
        //printf("sector:%u\n", current_sector);
        if( ((current_sector - last_sector) > 1) && (current_sector != 0))
        {
          // a sector was skipped
          //printf("sector:%u last:%u\n", current_sector, last_sector);
        }
        last_sector = current_sector;
        ledstrip_setcolors(leddata + (current_frame * POV_FRAME_SIZE) + (current_sector * SECTOR_DATA_SIZE)); // pointer arithmetic
      }
      else
      {
        main_loop_sleep();
      }
    }
    else
    {
      main_loop_sleep();
    }

    ///////////////////////////////////////////////////////
    // Update frame index
    time_since_frame_start = nsc_diff_time_time(current_time, frame_start_time);
    if(time_since_frame_start > FRAME_TIME_NS)
    {
      frame_start_time = current_time;
      if(++current_frame >= LDGetNofFramesInBuffer())
      {
        // replay from frame 0
        current_frame = 0;
      }
      //printf("current_frame=%d\n", current_frame);
    }

  }//while

  return 0;
}

