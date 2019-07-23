#ifndef BMP_H
#define BMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 64 bit linux types
#define U32 uint32_t
#define S32 int32_t
#define U16 uint16_t
#define U8  uint8_t

#define BMPFILEHEADERSIZE (sizeof(BMPFILEHEADER) + sizeof(BITMAPHEADER))

#pragma pack(push)
#pragma pack(1)
typedef struct BMPFILEHEADER
{
    U16  FileType;     /* File type, always 4D42h ("BM") */
    U32  FileSize;     /* Size of the file in bytes */
    U16  Reserved1;    /* Always 0 */
    U16  Reserved2;    /* Always 0 */
    U32  BitmapOffset; /* Starting position of image data in bytes */
} BMPFILEHEADER;


typedef struct BITMAPHEADER
{
    U32 Size;            /* Size of this header in bytes */
    U32 Width;           /* Image width in pixels */
    U32 Height;          /* Image height in pixels */
    U16 Planes;          /* Number of color planes */
    U16 BitsPerPixel;    /* Number of bits per pixel */
    U32 Compression;     /* Compression methods used */
    U32 SizeOfBitmap;    /* Size of bitmap in bytes */
    U32 HorzResolution;  /* Horizontal resolution in pixels per meter */
    U32 VertResolution;  /* Vertical resolution in pixels per meter */
    U32 ColorsUsed;      /* Number of colors in the image */
    U32 ColorsImportant; /* Minimum number of important colors */
} BITMAPHEADER;
#pragma pack(pop)

int OpenBmp(const char * const pszFileName, char** const ppBuf, BITMAPHEADER *pHeader);
void ReleaseBmp(char ** const ppBuf);
int GetPixel(const BITMAPHEADER * const pHeader, const char * const pBuf, const unsigned int x, const unsigned int y);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // BMP_H
