#ifndef INTERFACE_TCP
#define INTERFACE_TCP

#define TCP_MAX_COMMANDLEN 200

#include "inifile.h"
#define INTERFACE_TCP_UIR_File "UIR_TCP_IP.uir"

int TCP_panelServer(void);
int TCP_panelClient(void);
void TCP_setPanelServer(int newPanel);

void TCP_initPanelServer (int parentPanel);
void TCP_initPanelClient (int parentPanel);

void TCP_initServer (void *parseFunction);
int TCP_initClient (const char *serverName, void (*parseFunction )(char *, unsigned long), int showErrorMsg);



void TCP_closeServer (void);
void TCP_closeClient (void);

void TCP_clientSetLed (int panel, int sendLED, int receiveLED);
void TCP_serverSetLed (int panel, int sendLED, int receiveLED);

const char *TCP_getComputerName (void);
const char *TCP_getServerName (void);
void TCP_setServerName (char *serverName);

const char *TCP_errorString (int errorNo);
int TCP_displayError (const char *functionName, const char *subroutineName, int errorNo);

int TCP_clientIniTransmit (IniText ini, const char *commandStr, int showErrorMsg);
int TCP_clientSendBuffer (char *buffer, unsigned long bufferLen, int showErrorMsg);
int TCP_clientSendStrf (char *format, ...);
int TCP_clientSendCommand (char *commandStr);

int TCP_serverIniTransmit (IniText ini, const char *commandStr);
int TCP_serverSendBuffer (char *buffer, unsigned long bufferLen);
int TCP_serverSendStrf (char *format, ...);

void TCP_setDisplayProtocol (int display);
int TCP_serverOnSameComputer (void);
int TCP_displayErrorMessages (int display);



#endif
