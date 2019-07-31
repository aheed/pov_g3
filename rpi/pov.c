/* 
Persistence Of Vision project
  Raspberry Pi
  Adafruit Dotstar strip, HW SPI
  Interrupt driven rev synch, wiringPi

$ gcc bmp.c ldserver.c ns_clock.c pov.c -o pov -lwiringPi -lrt -lm -lpthread
$ sudo ./pov

*/


#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <string.h>
#include <wiringPi.h>
#include "../common/ledconfig.h"
#include "ldserver.h"
#include "ns_clock.h"



#define SPI_BITRATE 8000000

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
uint8_t* pleddata = NULL;



// -------------------------------------------------------------------------
void myInterrupt(void) {
   eventCounter++;
}


// -------------------------------------------------------------------------
int main( int argc, char* args[] )
{
  int i;
  int      fd;         // File descriptor if using hardware SPI
  struct timespec sleeper, dummy;
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


  struct spi_ioc_transfer xfer[3] = {
  { .tx_buf        = 0, // Header (zeros)
    .rx_buf        = 0,
    .len           = 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE},
  { .rx_buf        = 0, // Color payload
    .len           = NOF_LEDS * 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE},
  { .tx_buf        = 0, // Footer (zeros)
    .rx_buf        = 0,
    .len = (NOF_LEDS + 15) / 16,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE}
  };


  ///////////////////////////////////////7
  // Init SPI  
  if((fd = open("/dev/spidev0.0", O_RDWR)) < 0) {
    printf("Can't open /dev/spidev0.0 (try 'sudo')\n");
    return 2;
  }
 
  uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
  ioctl(fd, SPI_IOC_WR_MODE, &mode);
  // The actual data rate may be less than requested.
  // Hardware SPI speed is a function of the system core
  // frequency and the smallest power-of-two prescaler
  // that will not exceed the requested rate.
  // e.g. 8 MHz request: 250 MHz / 32 = 7.8125 MHz.
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, SPI_BITRATE);

  printf("\n\n");
  printf("NOF_SECTORS=%d\nNOF_LEDS=%d\n\n", NOF_SECTORS, NOF_LEDS);

  /////////////////////////////////////////////////////////
  // Set up interrupt routine for rev synch with WiringPi

  if (wiringPiSetup () == -1)
    return 3 ;
  
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 to generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (5, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }


  if(LDListen(leddata, BUFFER_SIZE, POV_FRAME_SIZE))
  {
    fprintf (stderr, "Failed to setup server\n");
    return 1;
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

        ////////////////////////////////////////
        // Transmit LED data on SPI
        pleddata = leddata + (current_frame * POV_FRAME_SIZE); // pointer arithmetic
        //xfer[1].tx_buf   = (unsigned long)&(pleddata[current_sector * SECTOR_DATA_SIZE]);
        xfer[1].tx_buf   = (unsigned long)(pleddata + (current_sector * SECTOR_DATA_SIZE)); // pointer arithmetic
        (void)ioctl(fd, SPI_IOC_MESSAGE(3), xfer);

        /*//sleep 75% of expected time until next sector
        sleeper.tv_sec  = 0;
        sleeper.tv_nsec = (avg_rev_time * 2) / (NOF_SECTORS * 4);
        nanosleep(&sleeper, &dummy);*/
      }
      else
      {
        // wait a while
        sleeper.tv_sec  = 0;
        sleeper.tv_nsec = SLEEP_PER_LOOP;
        nanosleep(&sleeper, &dummy);
      }
    }
    else
    {
      // wait a while
      sleeper.tv_sec  = 0;
      sleeper.tv_nsec = SLEEP_PER_LOOP;
      nanosleep(&sleeper, &dummy);
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
  

  /*
  // Cleanup
  if(fd) {
   close(fd);
   fd = -1;
  }
  */

  return 0;
}

