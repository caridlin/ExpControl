#include <cvirte.h>		
#include <userint.h>
#include "dataskt.h"
#include "nidaqcns.h"  
#include "tools.h"

#include "UIR_Demo_RemoteCommand.h"

//#include "CONTROL_DataStructure.h"
//#include "CONTROL_LoadSave.h"
#include "CONTROL_Definitions.h"
#include "CONTROL_DataSocket_Client.h"


int panelHandle;

const char UIR_FILE[] = "UIR_Demo_RemoteCommand.uir";
const char datasocketServer[] = "dstp://localhost";







int main (int argc, char *argv[])
{
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, UIR_FILE, PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	// set control where the library functions shall display the
	// status messages
	DATASOCKET_setDisplayCtrl (panelHandle, PANEL_STRING_status);
	
	// open connection
	if (DATASOCKET_openConnection (datasocketServer) != 0) {
		PostMessagePopup ("Error!", "No connection to DataSocket server established.");
		return -1;
	}
	
	
	RunUserInterface ();
	
	DiscardPanel (panelHandle);
	return 0;
}



int CVICALLBACK callback_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			QuitUserInterface(0);
			break;
	}
	return 0;
}



int CVICALLBACK callback_F9 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			// send command to hit F9 key
			// see "CONTROL_Definitions.h" for a list of all commands
			DATASOCKET_transferCommand (TCP_CMD_F9);
			break;
	}
	return 0;
}

int CVICALLBACK callback_STOP (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			// send command to STOP execution of sequence
			// see "CONTROL_Definitions.h" for a list of all commands
			DATASOCKET_transferCommand (TCP_CMD_STOP);
			break;
	}
	return 0;
}
