#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "ldprotocol.h"

#define LS_BUFFER_SIZE_FRAMES (g_bufBytes / g_frameBytes)

static char * g_pBuf;
static int g_bufBytes;
static pthread_t g_worker;
static int g_servsock;
static int g_framesread = 0;
static int g_framesInBuffer = 0;
static int g_frameBytes = 1;

////////////////////////////////////////////////
//
static void *worker_entry(void *param)
{
  int connsock;
  char response[LD_ACK_SIZE] = {LD_ACK_CHAR};
  int bytesread, totalbytesread;
  g_framesread = 0;
  struct pollfd fds[1];
  int timeout_msecs = 1000;


  printf("entering new thread\n");

  for( ; ; ){
    
    if((connsock=accept(g_servsock,(struct sockaddr*) NULL, NULL)) < 0){
      fprintf(stderr,"Kan inte acceptera anslutningen\n");
      continue;
    }
  
    printf("got connection\n");

    g_framesInBuffer = 0;

    fds[0].fd = connsock;
    fds[0].events = POLLIN;

    for( ; ; )
    {
      bytesread = 0;
      totalbytesread = 0;
      
      if(g_framesInBuffer >= LS_BUFFER_SIZE_FRAMES)
      {
        printf("Buffer full  g_framesInBuffer:%d LS_BUFFER_SIZE_FRAMES:%d\n", g_framesInBuffer, LS_BUFFER_SIZE_FRAMES);
        break;
      }

      do
      {
        fds[0].fd = connsock;
        fds[0].events = POLLIN;

        if(poll(fds, 1, timeout_msecs) != 1)
        {
          //timeot
          printf("timeout\n");
          break;
        }

        bytesread = read(connsock,
                         g_pBuf + (g_framesInBuffer * g_frameBytes) + totalbytesread,  //pointer arithmetic
                         g_frameBytes - totalbytesread);
        totalbytesread += bytesread;
        //printf("bytesread:%d totalbytesread:%d g_frameBytes:%d\n", bytesread, totalbytesread, g_frameBytes);
      } while((totalbytesread < g_frameBytes) && (bytesread > 0));

      //printf("totalbytesread:%d bytesread:%d g_frameBytes:%d\n", totalbytesread, bytesread, g_frameBytes);

      if((totalbytesread != g_frameBytes) || (bytesread < 0))
      {
        fprintf(stderr,"failed to receive frame\n");
        printf("totalbytesread:%d bytesread:%d g_frameBytes:%d\n", totalbytesread, bytesread, g_frameBytes);
        break;
      }

      g_framesInBuffer++;
      g_framesread++;
      printf("g_framesInBuffer=%d g_framesread=%d\n", g_framesInBuffer, g_framesread);
      if(write(connsock, response, LD_ACK_SIZE) < 0)
      {
        fprintf(stderr,"fel vid skrivningen till socket\n");
        break;
      }
    }
    
    printf("closing connection\n");
    close(connsock); 
  }

  return 0;
}

////////////////////////////////////////////////
//
int LDListen(char * const pBuf, int bufBytes, int frameBytes)
{
  struct sockaddr_in server;
  pthread_attr_t attr;
  struct sched_param param;

  g_pBuf = pBuf;
  g_bufBytes = bufBytes;
  g_frameBytes = frameBytes;

  // Set up server socket
  if((g_servsock=socket(AF_INET,SOCK_STREAM,0)) < 0){ 
    fprintf(stderr, "socket error\n");
    return 1;
  }

  memset(&server,0,sizeof(server));
  
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=htonl(INADDR_ANY);
  server.sin_port=htons(LDPORT);

  size_t t1 = g_frameBytes;
  size_t t2 = sizeof(int);
  if (setsockopt(g_servsock, SOL_SOCKET, SO_RCVLOWAT, &t1, t2) < 0) {
      perror(": setsockopt");
  }

  if(bind(g_servsock, (struct sockaddr*) &server, sizeof(server)) < 0){
    fprintf(stderr,"bind failed\n");
    return 2;
  }

  if(listen(g_servsock, 256) < 0){
    fprintf(stderr,"listen failed\n");
    return 3;
  }

  printf("listening on port %d\n", LDPORT);

 
  // Set the main thread (this thread) to high prio:
  param.sched_priority = 99;
  if(sched_setscheduler(0, SCHED_FIFO, &param))
  {
    fprintf(stderr, "failed to set main thread sched policy and prio\n");
  }

  // Start listener thread
  pthread_attr_init(&attr);

  if(pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
  {
    fprintf(stderr, "failed to set sched policy\n");
  }

  if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED))
  {
    fprintf(stderr, "failed to set sched inherit\n");
  }

  // lower prio
  param.sched_priority = 1;
  if(pthread_attr_setschedparam(&attr, &param))
  {
    fprintf(stderr, "failed to set sched param\n");
  }

  if(pthread_create(&g_worker,
                    &attr,          //attributes
                    worker_entry,
                    NULL))         //parameter
  {
    fprintf(stderr, "Error creating thread\n");
    return 4;
  }

  // Set listener thread to low prio
/*
  int errnumber = pthread_setschedprio(g_worker, 90);
  if(EINVAL == errnumber)
  {
    fprintf(stderr, "Failed to change thread priority: prio not valid\n");
  }
  if(EPERM == errnumber)
  {
    fprintf(stderr, "Failed to change thread priority: permission denied\n");
  }
  if(ESRCH == errnumber)
  {
    fprintf(stderr, "Failed to change thread priority: no such thread\n");
  }  */
  
  pthread_attr_destroy(&attr);

  printf("server started LS_BUFFER_SIZE_FRAMES=%d g_bufBytes=%d g_frameBytes=%d\n", LS_BUFFER_SIZE_FRAMES, g_bufBytes, g_frameBytes);
  return 0;
}

////////////////////////////////////////////////
//
int LDGetReceivedFrames()
{
  return g_framesread;
}

////////////////////////////////////////////////
//
int LDGetNofFramesInBuffer()
{
  return g_framesInBuffer;
}

////////////////////////////////////////////////
//
void LDStopServer()
{
  //FIXME: implement
}

