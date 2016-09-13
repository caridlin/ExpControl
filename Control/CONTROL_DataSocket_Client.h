#ifndef DATASOCKET_Client
#define DATASOCKET_Client


void DATASOCKET_setDisplayCtrl (int panel, int ctrl);

int DATASOCKET_openConnection (const char *serverName);

int DATASOCKET_transferCommand (const char *commandStr);

int DATASOCKET_transferBuffer (const char *buffer, unsigned bufferSize);



#endif
