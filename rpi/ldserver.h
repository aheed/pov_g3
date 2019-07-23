/*
Receives raw POV led data in a separate thread
*/


#ifndef LDCLIENT_H
#define LDCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

// Starts server thread
// Returns 0 if successful
// Non-blocking
int LDListen(char * const pBuf, int bufBytes);

int LDGetReceivedFrames();

void LDStopServer();

#ifdef __cplusplus
} //extern "C"
#endif

#endif // LDCLIENT_H

