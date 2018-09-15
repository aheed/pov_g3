/*
$ gcc ldclient.c bmp.c leddata.c povgeometry_gen3.c picclient_gen3.c -o picg3 -lm
$ ./picg3 <server ip> test_photo.bmp
*/



#include <stdio.h>
#include <time.h>
#include "leddata.h"
#include "ldclient.h"
#include "povgeometry_g3.h"


#define MAX_BRIGHTNESS 64

static uint8_t leddata[POV_FRAME_SIZE] = {0};
static uint8_t leddata2[POV_FRAME_SIZE] = {0}; //For debug only


///////////////////////////////////////////////
//
int main(int argc, char *argv[]) {

  struct BITMAPHEADER bmh = {0};
  char *pBuf;
  int i;
  int gamma = 0;

  if(argc < 3)
  {
    printf("usage:\n%s <server ip> <bmp file> [g]\ng: gamma correction\n", argv[0]);
    return -1;
  }

  for(i=3; i<argc; i++)
  {
    if(argv[i][0] == 'g')
    {
      gamma = 1;
    }
  }

/*  if(LDInitFromBmp(argv[2],
                   MAX_BRIGHTNESS,
                   NOF_SECTORS,
                   NOF_LEDS,
                   povledRadius,
                   gamma))
  {
    printf("failed to load bmp file\n");
    return 2;
  }*/

  //////////////////////
  // Open bmp file
  if(OpenBmp(argv[2], &pBuf, &bmh))
  {
    //failed to open bmp file
    printf("Failed to open bmp file %s\n", argv[2]);
    pBuf = NULL;
    return 1;
  }

  if(LDInitFromBmpData(pBuf,
                    &bmh,
                    MAX_BRIGHTNESS,
                    NOF_SECTORS,
                    NOF_LEDS,
                    povledRadius,
                    gamma))
   {
     printf("failed to init\n");
     return 2;
   }


  if(LDconnect(argv[1]))
  {
    printf("failed to connect to server\n");
    return 1;
  }

  /*LDgetLedData(NOF_SECTORS,
               NOF_LEDS,
               povledRadius,
               0,  //x offset, LED radius unit (mm or whatever)
               0,  //y offset, const int yoffset,  //LED radius unit (mm or whatever)
               leddata2);*/


  /*LDgetLedData2(0,  //x offset, bmp pixels
                0,  //y offset, bmp pixels
                leddata);*/

  LDgetLedDataFromBmpData3(pBuf,
                             MAX_BRIGHTNESS,
                             leddata,
                             0,
                             gamma);


//------------------------------------
// Debug stuff
/*
 int ledoffset = 0;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             leddata[ledoffset + 0],
             leddata[ledoffset + 1],
             leddata[ledoffset + 2],
             leddata[ledoffset + 3]);

      ledoffset = 7676;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             leddata[ledoffset + 0],
             leddata[ledoffset + 1],
             leddata[ledoffset + 2],
             leddata[ledoffset + 3]);

  //integrity check
  for(i=0; i< POV_FRAME_SIZE; i++)
  {
    if(leddata[i] != leddata2[i])
    {
      printf("Error!!!! i:%d\n", i);
    }
  }
*/
//-----------------------------------

  if(LDTransmit(leddata, POV_FRAME_SIZE))
  {
    printf("Failed to transmit led data to server\n");
    return 3;
  }

  printf("Transmitted!\n");

  if(LDWaitforAck())
  {
    printf("got no ack\n");
  }

  LDDisconnect();  

  return 0;
}

