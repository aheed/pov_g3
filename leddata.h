/*
Converts a bmp picture file into circular POV coordinates
in Dotstar LED raw data format
*/


#ifndef LEDDATA_H
#define LEDDATA_H

#include "bmp.h"

// Returns 0 if successful
int LDInitFromBmp(const char * const pszFileName,
                  const unsigned char brightness,
                  const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int gamma);

// Returns 0 if successful
int LDInitFromBmpData(char * const pBmpBuf,
                  const BITMAPHEADER * pBmpHeader,
                  const unsigned char brightness,
                  const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int gamma);

void LDgetLedData(const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int xoffset,  //LED radius unit (mm or whatever)
                  const int yoffset,  //LED radius unit (mm or whatever)
                  char * const pLeddataOut);

//  Faster implementation using cached geometry
void LDgetLedData2(const int xbmpoffset,  //bmp pixels
                   const int ybmpoffset,  //bmp pixels
                   char * const pLeddataOut);

void LDgetLedDataFromBmpData(const char * pBmpBuf,
                             const unsigned char brightness,
                             char * const pLeddataOut,
                             const int yflip,
                             const int gamma);

void LDgetLedDataFromBmpData3(const char * const pBmpBuf,
                             const unsigned char brightness,
                             char * const pLeddataOut,
                             const int yflip,
                             const int gamma);

void LDsetLedDataBlack(const int nofSectors,
                       const int nofLeds,
                       char * const pLeddataOut);

void LDsetLed(const int nofLeds,
              const int sector,
              const int led,
              const unsigned char blue,
              const unsigned char green,
              const unsigned char red,
              char * const pLeddataOut);

void LDRelease();

void LDReleaseBmpData();


#endif // LEDDATA_H
