/*
Transmits raw POV led data to a server
*/


#ifndef LDCLIENT_H
#define LDCLIENT_H


// Returns 0 if successful
int LDconnect(const char * const pszServerURL);

// Returns 0 if successful
int LDTransmit(const char * const pData, int bytes);

// Returns 0 if ack was received from server before timeout
int LDWaitforAck();

void LDDisconnect();

#endif // LDCLIENT_H

