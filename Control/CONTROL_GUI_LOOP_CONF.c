#include <ansi_c.h>
#include <string.h>
#include <userint.h>
#include "CONTROL_HARDWARE_Adwin.h"
#include "ExperimentControl.h"
#include "UIR_ExperimentControl.h"

#define SEND_CONFIG_PAR_INDEX 73

int confPanel = -1;

void sendConfFile (void)
{   			
	// tell Drogon to expect a configuration file
	Set_Par_Err(SEND_CONFIG_PAR_INDEX, 1);
	
	// send Drogon the configuration file
	char configFile[2000];
	GetCtrlVal(confPanel, LCONFTAB_CONFTEXT, configFile);
	send(ConnectSocket, configFile, 2000, 0);
}

int CVICALLBACK onSendConfButton (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			confPanel = panel;
			sendConfFile();
			break;
	}
	return 0;
}
					
