/*
$ gcc ldclient.c bmp.c leddata.c videoclient.c povgeometry_gen3.c -o video -lavformat -lavcodec -lavutil -lswscale -lm
$ ./video ~/Videos/WILDLIFE\ IN\ 4K\ \(ULTRA\ HD\).mp4


With gamma correction and limited brightness
*/


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <ffmpeg/swscale.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <time.h>
#include "leddata.h"
#include "ldclient.h"
#include "povgeometry_g3.h"

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define OUTPUT_WIDTH  256 //768
#define OUTPUT_HEIGHT 256 //768

//------------------------------------------
// POV stuff
//#define NOF_SECTORS 120
//#define NOF_LEDS 32
#define MAX_BRIGHTNESS 192 //128

#define SLEEP_PER_FRAME  0 //10000000 // nanoseconds


static uint8_t leddata[NOF_SECTORS * NOF_LEDS * 4] = {0};

static uint32_t rotation = 950; // Default to make rotation look good on fan
static uint8_t brightness = MAX_BRIGHTNESS;

//-------------------------------------

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

typedef enum ArgParseState {
	APS_INITIAL,
	APS_BRIGHTNESS,
	APS_ROTATION,
	APS_STARTLED,
	APS_ENDLED
} ArgParseState;

///////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
  // Initalizing these to NULL prevents segfaults!
  AVFormatContext   *pFormatCtx = NULL;
  int               i, videoStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVFrame           *pFrame = NULL;
  AVFrame           *pFrameRGB = NULL;
  AVPacket          packet;
  int               frameFinished;
  int               numBytes;
  uint8_t           *buffer = NULL;
  struct SwsContext *sws_ctx = NULL;
  int               connected = 0;
  struct timespec sleeper, dummy;
  int startLed = 0;
  int endLed = 1000;
  int gamma = 0;
  int algo = 3;
  char *pszServer = NULL;
  char *pszVideoFileName = NULL;
  

  struct BITMAPHEADER bmh = {0};

  if(argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }
  if(argc < 3)
  {
    printf("usage:\n%s <server ip> <video file> [-g]\ng: gamma correction\n", argv[0]);
    return -1;
  }

  pszServer = argv[1];
  pszVideoFileName = argv[2];

  ArgParseState aps = APS_INITIAL;
  for(i=3; i<argc; i++)
  {
    printf("arg %d:%s\n", i, argv[i]);
    
    if(aps == APS_BRIGHTNESS)
    {
    	brightness = atoi(argv[i]);
    	aps = APS_INITIAL;
    	printf("brightness=%d\n", brightness);
    }  	 
    
    if(aps == APS_ROTATION)
    {
    	rotation = atoi(argv[i]);
    	aps = APS_INITIAL;
    	printf("rotation=%d\n", rotation);
    }
    
    if(aps == APS_STARTLED)
    {
    	startLed = atoi(argv[i]);
    	aps = APS_INITIAL;
    	printf("startLed=%d\n", startLed);
    }
    
    if(aps == APS_ENDLED)
    {
    	endLed = atoi(argv[i]);
    	aps = APS_INITIAL;
    	printf("endLed=%d\n", endLed);
    }
  	 
    if(!strcmp(argv[i], "-g"))
    {
      gamma = 1;
    }
    
    if(!strcmp(argv[i], "-a3"))
    {
      algo = 3;
      printf("algo 3!!\n");
    }
    
    if(!strcmp(argv[i], "-a4"))
    {
      algo = 4;
      printf("algo 4!!\n");
    }
    
    if(!strcmp(argv[i], "-b"))
    {
      aps = APS_BRIGHTNESS;
    }
    
    if(!strcmp(argv[i], "-r"))
    {
      aps = APS_ROTATION;
    }
    
    if(!strcmp(argv[i], "-s"))
    {
      aps = APS_STARTLED;
    }
    
    if(!strcmp(argv[i], "-e"))
    {
      aps = APS_ENDLED;
    }
  }

  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, pszVideoFileName, NULL, NULL)!=0)
  {
    fprintf(stderr, "Couldn't open file %s\n", pszVideoFileName);
    return -1;
  }
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
  {
    fprintf(stderr, "Couldn't find stream information\n");
    return -1;
  }
  
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, pszVideoFileName, 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Copy context
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
    fprintf(stderr, "Couldn't copy codec context");
    return -1; // Error copying codec context
  }

  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=av_frame_alloc();
  
  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if(pFrameRGB==NULL)
    return -1;

  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_BGR24, OUTPUT_WIDTH,
			      OUTPUT_HEIGHT);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		 OUTPUT_WIDTH, OUTPUT_HEIGHT);
  
  // initialize SWS context for software scaling
  sws_ctx = sws_getContext(pCodecCtx->width,
			   pCodecCtx->height,
			   pCodecCtx->pix_fmt,
			   OUTPUT_WIDTH,
			   OUTPUT_HEIGHT,
			   PIX_FMT_BGR24,
			   SWS_BILINEAR,
			   NULL,
			   NULL,
			   NULL
			   );

/*  //-----------------------------------
  if(LDconnect("127.0.0.1"))
  {
    return 1;
  }
 
  //-----------------------------------*/

  // Read frames and save first five frames to disk
  i=0;
//  while((av_read_frame(pFormatCtx, &packet)>=0) && (i < 500)) {
  while((av_read_frame(pFormatCtx, &packet)>=0)) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	// Convert the image from its native format to RGB
	sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
		  pFrame->linesize, 0, pCodecCtx->height,
		  pFrameRGB->data, pFrameRGB->linesize);

        // prepare LED data for POV
        if(i == 0)
        {
          bmh.Width = OUTPUT_WIDTH;
          bmh.Height = OUTPUT_HEIGHT;
          bmh.BitsPerPixel = 24;

          if(LDInitFromBmpData(pFrameRGB->data[0],
                  &bmh,
                  brightness,
                  NOF_SECTORS,
                  NOF_LEDS,
                  povledRadius,
                  povledAngle,
                  1,
                  rotation,
                  1)) //yflip
           {
             fprintf(stderr, "Failed to init LED data\n");
             return 1;
           }
        }
        
        /*LDgetLedDataFromBmpData3(pFrameRGB->data[0],
                             MAX_BRIGHTNESS,
                             leddata,
                             1,
                             1);*/
        /*
        LDgetLedDataFromBmpData(pFrameRGB->data[0],
                             MAX_BRIGHTNESS,
                             leddata,
                             1,
                             0);*/
        LDgetLedDataFromBmpData4(pFrameRGB->data[0],
                            brightness,
                            leddata,
                            gamma,
                            startLed,
                             endLed);

        //Transmit LED data to POV server
        if(!connected)
        {
          printf("Connecting %s\n", pszServer);
          if(LDconnect(pszServer))
          {
            fprintf(stderr, "Failed to connect\n");
             return 1;
          }
          connected = 1;
        }

        if(LDTransmit(leddata, POV_FRAME_SIZE))
        {
          fprintf(stderr, "Failed to xmit\n");
          return 3;
        }
        else 
        {
          printf("xmitted frame %d\n", i);
        }

        if(LDWaitforAck())
        {
          printf("got no ack\n");
          LDDisconnect();
          connected = 0;
        }

        ++i;

        if(SLEEP_PER_FRAME > 0)
        {
          // wait a while
          sleeper.tv_sec  = 0;
          sleeper.tv_nsec = SLEEP_PER_FRAME;
          nanosleep(&sleeper, &dummy);
          //sleep(2); //temp
        }
	
	// Save the frame to disk
        
	/*if(i<=5)
	  SaveFrame(pFrameRGB, OUTPUT_WIDTH, OUTPUT_HEIGHT, 
		    i);*/

      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);

  }
  
  // Free the RGB image
  av_free(buffer);
  av_frame_free(&pFrameRGB);
  
  // Free the YUV frame
  av_frame_free(&pFrame);
  
  // Close the codecs
  avcodec_close(pCodecCtx);
  avcodec_close(pCodecCtxOrig);

  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}

