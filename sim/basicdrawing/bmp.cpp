#include <QtGlobal>

#include <stdio.h>
#include <malloc.h>
#include "bmp.h"

#define BMP_DEBUG

//////////////////////////////////////////////////////////////
void DumpBmpFileHeader(const BMPFILEHEADER * const pFileHeader)
{
    printf("FileType:0x%x\nFileSize:%d\nReserved1:%d\nReserved2:%d\nBitmapOffset:%d\n",
           pFileHeader->FileType, pFileHeader->FileSize, pFileHeader->Reserved1, pFileHeader->Reserved2, pFileHeader->BitmapOffset);
}

//////////////////////////////////////////////////////////////
void DumpBmpHeader(const BITMAPHEADER * const pHeader)
{
    printf("Size:%d\nWidth:%d\nHeight:%d\nPlanes:%d\nBitsPerPixel:%d\nCompression:%d\nSizeOfBitmap:%d\nHorzResolution:%d\nVertResolution:%d\nColorsUsed:%d\nColorsImportant:%d\n",
           pHeader->Size, pHeader->Width, pHeader->Height, pHeader->Planes, pHeader->BitsPerPixel, pHeader->Compression, pHeader->SizeOfBitmap, pHeader->HorzResolution, pHeader->VertResolution, pHeader->ColorsUsed, pHeader->ColorsImportant);
}

//////////////////////////////////////////////////////////////
int OpenBmp(const char * const pszFileName, char** const ppBuf, BITMAPHEADER *pHeader)
{
    FILE *infile;
    unsigned int filesize, paddingsize;
    BMPFILEHEADER fileheader;

    /////
    Q_ASSERT(sizeof(U32) == 4);
    Q_ASSERT(sizeof(S32) == 4);
    Q_ASSERT(sizeof(U16) == 2);
    Q_ASSERT(sizeof(U8) == 1);
    /////

    infile = fopen(pszFileName, "rb");
    if(!infile)
    {
        //printf("Failed to open file %s for writing\n", pszFile);
        return 1;
    }

    fseek(infile, 0L, SEEK_END);
    filesize = ftell(infile);
    if(!filesize || (filesize <= BMPFILEHEADERSIZE))
    {
        // input file too small
        return 2;
    }

    fseek(infile, 0L, SEEK_SET); //rewind to start of file

    *ppBuf = (char*)malloc(filesize-BMPFILEHEADERSIZE);
    if(!*ppBuf)
    {
        //failed to allocate memory
        fclose(infile);
        return 3;
    }

    if(!fread(&fileheader, sizeof(fileheader), 1, infile))
    {
        //failed to read file header
        free(*ppBuf);
        *ppBuf = NULL;
        fclose(infile);
        return 5;
    }

    if(!fread(pHeader, sizeof(*pHeader), 1, infile))
    {
        //failed to read bitmap header
        free(*ppBuf);
        *ppBuf = NULL;
        fclose(infile);
        return 6;
    }

    // For now there is only support for monochrome and 24-bit pixel formats
    if(!( (pHeader->BitsPerPixel == 1) || (pHeader->BitsPerPixel == 24) ))
    {
        //unsupported format
        free(*ppBuf);
        *ppBuf = NULL;
        fclose(infile);
        return 7;
    }

    paddingsize = fileheader.BitmapOffset - BMPFILEHEADERSIZE;
    if(paddingsize > 0)
    {
        if(!fread(*ppBuf, paddingsize, 1, infile))
        {
            //failed to read padding
            free(*ppBuf);
            *ppBuf = NULL;
            fclose(infile);
            return 8;
        }
    }

    if(!fread(*ppBuf, pHeader->SizeOfBitmap, 1, infile))
    {
        //failed to read bitmap data
        free(*ppBuf);
        *ppBuf = NULL;
        fclose(infile);
        return 9;
    }

#ifdef BMP_DEBUG
    // Dump for debugging
    printf("Opening file %s\n", pszFileName);
    printf("file size: %d\n", filesize);
    printf("bitmapoffset - header size: %d\n", int(fileheader.BitmapOffset - BMPFILEHEADERSIZE));
    DumpBmpFileHeader(&fileheader);
    DumpBmpHeader(pHeader);
#endif

    // success!
    fclose(infile);
    return 0;
}


//////////////////////////////////////////////////////////////
void ReleaseBmp(char ** const ppBuf)
{
  free(*ppBuf);
  *ppBuf = NULL;
}

////////////////////////////////////////////////////////////////
// Get monochrome value
int GetPixelMonochrome(BITMAPHEADER *pHeader, char *pBuf, unsigned int x, unsigned int y)
{
    unsigned int pixelindex;
    unsigned int rowsize;
    unsigned int row;
    unsigned int byteindex;
    unsigned int bitindex;

    Q_ASSERT(x <= pHeader->Width);
    Q_ASSERT(y <= pHeader->Height);

    pixelindex = y * pHeader->Width + x;
    Q_ASSERT(pixelindex < (pHeader->Width * pHeader->Height));


    row = pixelindex / pHeader->Width;
    row = (pHeader->Height - 1) - row; // convert bottom-up to top-down row order

    rowsize = pHeader->Width / 8;
    if(pHeader->Width % 8)
    {
        ++rowsize;
    }

    while(rowsize % 4)
    {
      rowsize++;
    }

    bitindex = (pixelindex % pHeader->Width) % 8;
    byteindex = row * rowsize + (pixelindex % pHeader->Width) / 8;

    return (pBuf[byteindex] >> (7 - bitindex)) & 1;
}

////////////////////////////////////////////////////////////////
// Get 24-bit pixel value
int GetPixel24bit(BITMAPHEADER *pHeader, char *pBuf, unsigned int x, unsigned int y)
{
    unsigned int pixelindex;
    unsigned int rowsize; //bytes
    unsigned int row;
    unsigned int byteindex;
    unsigned int retval;
    int i;
//    unsigned int bitindex;

    Q_ASSERT(x <= pHeader->Width);
    Q_ASSERT(y <= pHeader->Height);

    pixelindex = y * pHeader->Width + x;
    Q_ASSERT(pixelindex < (pHeader->Width * pHeader->Height));


    row = pixelindex / pHeader->Width;
    row = (pHeader->Height - 1) - row; // convert bottom-up to top-down row order

    //----------------------------------------------------
    rowsize = ((pHeader->Width * 24) + 7) / 8;

    while(rowsize % 4)
    {
      rowsize++;
    }

    //bitindex = (pixelindex % pHeader->Width) % 8;
    byteindex = row * rowsize + (pixelindex % pHeader->Width) * 3;

    retval = 0;
    for(i = 0; i < 3; i++) //iterate bytes
    {
      retval <<= 8;
      retval |= (unsigned int)(((unsigned char*)pBuf)[byteindex++]);
    }

    return retval;
}

////////////////////////////////////////////////////////////////
//
int GetPixel(BITMAPHEADER *pHeader, char *pBuf, unsigned int x, unsigned int y)
{

    if(pHeader->BitsPerPixel == 1)
    {
        return GetPixelMonochrome(pHeader, pBuf, x, y);
    }
    else if(pHeader->BitsPerPixel == 24)
    {
        return GetPixel24bit(pHeader, pBuf, x, y);
    }
    else
    {
        // unsupported format
        Q_ASSERT(0);
        return 0;
    }
}
