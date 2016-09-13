#if !defined(DATASOCKET)
#define DATASOCKET


#include "CONTROL_DataStructure.h"

int DATASOCKET_initServer (int launchServer);
int DATASOCKET_closeServer (void);


void WATCHDOG_handshake (void);

int watchdogOn (t_sequence *seq);

int WATCHDOG_on (void);
int watchdogConfigure (void);


int DATASOCKET_setWaitingForTrigger (int state);

int DATASOCKET_connectToServer(const char *serverName);
int DATASOCKET_disconnectFromServer (void);

void WATCHDOG_getStatus (void);
void WATCHDOG_setReceived (int newStatus);



void WATCHDOG_setLed1 (int draw);
void WATCHDOG_setLed2 (int draw);





void WATCHDOG_setLedCtrl1 (int panel, int ctrl);
void WATCHDOG_setLedCtrl2 (int panel, int ctrl);
void WATCHDOG_setStatusCtrl (int panel, int ctrl);




#endif

