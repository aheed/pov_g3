#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "../common/ledconfig.h"
#include "ledstrip.h"

#define SPI_BITRATE 8000000

static struct spi_ioc_transfer xfer[3] = {
  { .tx_buf        = 0, // Header (zeros)
    .rx_buf        = 0,
    .len           = 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE},
  { .rx_buf        = 0, // Color payload
    .len           = NOF_LEDS * LED_DATA_SIZE,
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
  
static int fd;

////////////////////////////////////////////////////////
int ledstrip_init()
{
  if((fd = open("/dev/spidev0.0", O_RDWR)) < 0) {
    printf("Can't open /dev/spidev0.0 (try 'sudo')\n");
    return 1;
  }
 
  uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
  ioctl(fd, SPI_IOC_WR_MODE, &mode);
  // The actual data rate may be less than requested.
  // Hardware SPI speed is a function of the system core
  // frequency and the smallest power-of-two prescaler
  // that will not exceed the requested rate.
  // e.g. 8 MHz request: 250 MHz / 32 = 7.8125 MHz.
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, SPI_BITRATE);
  return 0;
}

////////////////////////////////////////////////////////
void ledstrip_setcolors(uint8_t* pleddata)
{
    xfer[1].tx_buf   = (unsigned long)(pleddata);
    (void)ioctl(fd, SPI_IOC_MESSAGE(3), xfer);
}

