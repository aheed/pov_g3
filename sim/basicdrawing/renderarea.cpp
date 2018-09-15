/****************************************************************************
****************************************************************************/

#include "renderarea.h"

#include <QPainter>
#include <QDebug>

#include "bmp.h"
#include "renderarea.h"
#include "povgeometry_g3.h"
#include "ldserver.h"

#ifndef __cplusplus
#define __cplusplus
#endif

////////////////////////////////////////////////////////////
/*
#define NOF_LEDS 32 //10 //4
#define NOF_SECTORS 120 //120 //60
*/
#define MYPI 3.14159265

const int ledRadius[NOF_LEDS] =
        #if NOF_LEDS == 4
        //  {210, 290, 370, 450};
{105, 145, 185, 225};
#elif NOF_LEDS == 10
        //  {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
{50, 70, 90, 110, 130, 150, 170, 190, 210, 230};
#elif NOF_LEDS == 20
        //  {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
{50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240};
#elif NOF_LEDS == 32
{50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230 ,236};
#elif NOF_LEDS == 64
{50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230 ,236};
#endif

const int xc = 280;
const int yc = 250;
//const int xc = 500;
//const int yc = 450;

const float povLedScale = 0.17;


// assume 24 bit colors
static int color_states[NOF_SECTORS][NOF_LEDS] = {0};


BITMAPHEADER bmh;
char *pBuf = NULL;



//////////////////////////////////////////////////////////////77
//
void ConvertPicCoordsToBmpCoords(unsigned int maxxin, unsigned int maxyin,
                                 unsigned int xin, unsigned int yin,
                                 const BITMAPHEADER * const pHeader,
                                 unsigned int *pxout, unsigned int *pyout)
{
    Q_ASSERT(xin <= maxxin);
    Q_ASSERT(yin <= maxyin);

    *pxout = xin * (pHeader->Width - 1)/ maxxin;
    *pyout = yin * (pHeader->Height - 1) / maxyin;
}

// Sets initial LED state
void SetLedStates()
{
    //////////////////////////////////////////////////////////////////

    const char *bmpfilename = "/home/anders/src/repo/pov_g3/sim/basicdrawing/sandra3.bmp";

    if(!pBuf)
    {
        if(OpenBmp(bmpfilename, &pBuf, &bmh))
        {
            //failed to open bmp file
            printf("Failed to open bmp file %s\n", bmpfilename);
            pBuf = NULL;
        }
    }


    //////////////////////////////////////////////////////////////////

    //--------------------------------------------------
    int sector;
    int shiftbits;
    int i;
    int aLen;
    int startAng;
    double startAngRad;
    int supersector;
    int x;
    int y;
    int maxx, maxy;
    int xin, yin;
    unsigned int bmpx;
    unsigned int bmpy;
    //    char ledstate;
    int ledstate;


    aLen = 5760 / NOF_SECTORS;

    maxx = 2 * ledRadius[NOF_LEDS-1];
    maxy = 2 * ledRadius[NOF_LEDS-1];

    for(sector=0; sector<NOF_SECTORS; sector++)
    {
        startAng = aLen * sector + aLen/2;

        shiftbits = 7 - sector % 8;  //[0,7]
        supersector = sector / 8; //[0, NOF_SECTORS/8]

        startAngRad = ((double)startAng / 5760) * 2 * MYPI;

        for(i=0; i<NOF_LEDS; i++)
        {
            x = xc + cos((double)startAngRad) * ledRadius[i];
            y = yc - sin((double)startAngRad) * ledRadius[i];

            xin = x - xc + ledRadius[NOF_LEDS-1]; // + anim_offset_x;
            yin = y - yc + ledRadius[NOF_LEDS-1]; // + anim_offset_y;

            if((xin <= maxx) && (yin <= maxy) && (xin > 0) && (yin > 0))
            {

                ConvertPicCoordsToBmpCoords(maxx, maxy, xin, yin, &bmh, &bmpx, &bmpy);

                if(pBuf)
                {
                    ledstate = GetPixel(&bmh, pBuf, bmpx, bmpy);
                }
            }
            else
            {
                ledstate = 0;
            }

            color_states[sector][i] = ledstate;
            //            printf("ledstate:0x%X\n", ledstate);

        }
    }
}


RenderArea* RenderArea::m_pInstance = nullptr;

static void ledDataReceived()
{
    RenderArea::UpdateTheInstance();
}

void RenderArea::UpdateTheInstance()
{
    m_pInstance->update();
}



//----------------------------------------------------
//---------------------------------------------------

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    SetLedStates();

    m_pLedDataBuf = (char*)malloc(POV_FRAME_SIZE);
    m_pInstance = this;
    if(LDListen(m_pLedDataBuf, POV_FRAME_SIZE, &ledDataReceived))
    {
        qDebug() << "Failed to launch POV listener!";
    }

    update();
}

QSize RenderArea::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize RenderArea::sizeHint() const
{
    return QSize(400, 200);
}


void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.save();
    {
        int sector;
        QPen pen;
        int shiftbits;
        int i;
        int aLen;
        int startAng;
        int supersector;
        unsigned char* pChar = (unsigned char*)m_pLedDataBuf;


        aLen = 5760 / NOF_SECTORS;

        //                    painter.drawRect(rect);

        pen.setWidth(60 * povLedScale);

        for(sector=0; sector<NOF_SECTORS; sector++)
        {
            startAng = aLen * sector;

            shiftbits = 7 - sector % 8;  //[0,7]
            supersector = sector / 8; //[0, NOF_SECTORS/8]

            for(i=0; i<NOF_LEDS; i++)
            {

                if(LDGetReceivedFrames() > 0)
                {
                    *pChar++; //header always contains 0xFF. Skip it.
                    int ledblue = *pChar++;
                    int ledgreeen = *pChar++;
                    int ledred = *pChar++;

                    pen.setColor(QColor(ledred, ledgreeen, ledblue));
                    painter.setPen(pen);
                    painter.drawArc(xc - povLedScale * povledRadius[i], yc - povLedScale * povledRadius[i], povLedScale * povledRadius[i] * 2, povLedScale * povledRadius[i] * 2, startAng, aLen);
                }
                else
                {
                    // Convert from BGR to RGB
                    pen.setColor(QColor(color_states[sector][i] & 0xFF,             //red
                                        (color_states[sector][i] & 0xFF00) >> 8,    //green
                                        (color_states[sector][i] & 0xFF0000) >> 16  //blue
                                        ));
                    painter.setPen(pen);
                    painter.drawArc(xc - ledRadius[i], yc - ledRadius[i], ledRadius[i] * 2, ledRadius[i] * 2, startAng, aLen);
                }


            }

        }
    }

    painter.restore();

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
