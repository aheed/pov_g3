/* Bare bones test of Dotstar strip on rPi with C, HW SPI

$ gcc dstest.c -o dstest
$ ./dstest

*/


#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>


#define NOF_LEDS 34
#define SPI_BITRATE 8000000

int main( int argc, char* args[] )
{
  int i;
  uint8_t leddata[NOF_LEDS * 4] = {0};
  int      fd;         // File descriptor if using hardware SPI

  struct spi_ioc_transfer xfer[3] = {
  { .tx_buf        = 0, // Header (zeros)
    .rx_buf        = 0,
    .len           = 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0 },
  { .rx_buf        = 0, // Color payload
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0 },
  { .tx_buf        = 0, // Footer (zeros)
    .rx_buf        = 0,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0 }
  };


  /////////////////////////////////////////
  // initialize LED data.
  for(i=0; i<(NOF_LEDS * 4); i++)
  {
    if((i % 4) == 0)
    {
      leddata[i] = 0xFF;
    }
    else
    {
      leddata[i] = 0;
    }
  }

  //First LED
  leddata[0] = 0xFF;
  leddata[1] = 64;
  leddata[2] = 0;
  leddata[3] = 0;

  // Second LED
  leddata[4] = 0xFF;
  leddata[5] = 0;
  leddata[6] = 64;
  leddata[7] = 0;

  // ...etc
  leddata[8] = 0xFF;
  leddata[9] = 0;
  leddata[10] = 0;
  leddata[11] = 64;

  leddata[12] = 0xFF;
  leddata[13] = 1;
  leddata[14] = 0;
  leddata[15] = 0;

  leddata[16] = 0xFF;
  leddata[17] = 0;
  leddata[18] = 1;
  leddata[19] = 0;

  leddata[20] = 0xFF;
  leddata[21] = 0;
  leddata[22] = 0;
  leddata[23] = 1;

  leddata[24] = 0xFF;
  leddata[25] = 1;
  leddata[26] = 1;
  leddata[27] = 1;

  ///////////////////////////////////////7
  // Init SPI  
  if((fd = open("/dev/spidev0.0", O_RDWR)) < 0) {
    printf("Can't open /dev/spidev0.0 (try 'sudo')\n");
    return 0;
  }
 
  uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
  ioctl(fd, SPI_IOC_WR_MODE, &mode);
  // The actual data rate may be less than requested.
  // Hardware SPI speed is a function of the system core
  // frequency and the smallest power-of-two prescaler
  // that will not exceed the requested rate.
  // e.g. 8 MHz request: 250 MHz / 32 = 7.8125 MHz.
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, SPI_BITRATE);

  
  ////////////////////////////////////////
  // Transmit LED data on SPI
  xfer[0].speed_hz = SPI_BITRATE;
  xfer[1].speed_hz = SPI_BITRATE;
  xfer[2].speed_hz = SPI_BITRATE;
  xfer[1].tx_buf   = (unsigned long)leddata;
  xfer[1].len      = NOF_LEDS * 4;
  xfer[2].len = (NOF_LEDS + 15) / 16;


  // All that spi_ioc_transfer struct stuff earlier in
  // the code is so we can use this single ioctl to concat
  // the data & footer into one operation:
  (void)ioctl(fd, SPI_IOC_MESSAGE(3), xfer);

  
  // Cleanup
  if(fd) {
   close(fd);
   fd = -1;
  }

  return 0;
}

