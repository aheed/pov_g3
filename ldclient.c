
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include "ldclient.h"
#include "ldprotocol.h"


static int sock;
static struct sockaddr_in server;

////////////////////////////////////////////////////////
//
int LDconnect(const char * const pszServerURL)
{

  if ((sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    fprintf(stderr, "socket error");

  memset(&server,0,sizeof(server));
  
  server.sin_family=AF_INET;
  server.sin_port=htons(LDPORT);
  
  if(inet_pton(AF_INET, pszServerURL, &server.sin_addr) <= 0){
    fprintf(stderr, "Invalid address: %s\n", pszServerURL);
    exit(-1);
  }
  
  if( connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
    fprintf(stderr,"Failed to connect to POV server\n");
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////
//
int LDTransmit(const char * const pData, int bytes)
{
  if(write(sock, pData, bytes) < 0)
  {
    fprintf(stderr,"failed to transmit LED data\n");
    return 1;
  }

  return 0;
}


////////////////////////////////////////////////////////
//
int LDWaitforAck()
{
  int bytesread = 0;
  unsigned char ackbuffer[LD_ACK_SIZE];
  struct pollfd fds[1];
  int timeout_msecs = 1000;

  fds[0].fd = sock;
  fds[0].events = POLLIN;

  if(poll(fds, 1, timeout_msecs) != 1)
  {
    //timeot
    fprintf(stderr,"timeout\n");
    return 1;
  }

  bytesread = read(sock, ackbuffer, LD_ACK_SIZE);

  if((bytesread != LD_ACK_SIZE) || (ackbuffer[0] != LD_ACK_CHAR))
  {
    fprintf(stderr,"wrong answer\n");
    printf("bytesread:%d\n", bytesread);
    if(bytesread > 0)
    {
      printf("first byte:%d\n", ackbuffer[0]);
    }
    return 2;
  }
  
  return 0;
}

////////////////////////////////////////////////////////
//
void LDDisconnect()
{
  close(sock); 
}

