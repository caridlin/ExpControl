#include "dataskt.h"
#include <userint.h>
#include "tools.h"

#include "CONTROL_datasocket.h"    
#include "CONTROL_definitions.h"
#include "CONTROL_GUI_Main.h"    
#include "watchdog_variables.h"    


//const char DS_VARIABLE_PREALERT[] = "%s/watchdog/prealert";
//const char DS_VARIABLE_UNLOCKED[] = "%s/watchdog/unlocked";
//const char DS_VARIABLE_RECEIVED[] = "%s/watchdog/received";

const char DS_SERVER[] = "dstp://localhost";



int serverRunning;

int	panelLED_watchdog = -1;
int	ctrlLED_watchdog = -1;
int	panelLED_watchdog2 = -1;
int	ctrlLED_watchdog2 = -1;

int	panelStatus_watchdog = -1;
int	ctrlStatus_watchdog = -1;

long watchdogStatus = 0;
long commandStatus  = 0;
long statusReceived = 0;
long waitingForTrigger = -1;


DSHandle DShandle_watchdogStatus = 0;
DSHandle DShandle_watchdogReceived = 0;
DSHandle DShandle_command = 0;
DSHandle DShandle_commandUpdated = 0;
DSHandle DShandle_commandReceived = 0;
DSHandle DShandle_waitingForTrigger = 0;

// complie this function only if linked by software 
// other than ExperimentControl
#ifndef EXPERIMENTCONTROL
	int SEQUENCE_getStatus (void) 
	{
		return SEQ_STATUS_FINISHED;
	}
#endif



void printState (void)
{
	static int i = -1000;
	static int oldStatus = -2000;
	
	
	#ifdef _CVI_DEBUG_
	if ((i != statusReceived) || (oldStatus != watchdogStatus)) {
		i = statusReceived;
		oldStatus = watchdogStatus;
		tprintf ("%sh: status=%d, received=%d\n", timeStr(), watchdogStatus, i);
	}
	#endif
}



int DATASOCKET_error (int errorCode, const char *functionName)
{
	char message[400];
	
	if (errorCode == 0) return 0;
	DS_GetLibraryErrorString (errorCode, message, 400);

	PostMessagePopupf ("DataSocket error", "Function %s failed.\n\nErrorcode %08x: %s", 
				  functionName, errorCode, message);

	return -1;
}


int DATASOCKET_serverRunning (void) 
{
	return serverRunning;	
}


int DATASOCKET_initServer (int launchServer)
{
    HRESULT hr = S_OK;

	if (!launchServer) {
		serverRunning = TRUE;
		return 0;
	}
	
	hr = DS_ControlLocalServer (DSConst_ServerLaunch);
    if (hr >= 0) serverRunning = TRUE;
    else return DATASOCKET_error (hr, "DS_ControlLocalServer (DSConst_ServerLaunch)");

	hr = DS_ControlLocalServer (DSConst_ServerHide);
	if (hr < 0) return DATASOCKET_error (hr, "DS_ControlLocalServer (DSConst_ServerHide)");
    
    return 0;
}



int DATASOCKET_closeServer (void)
{
    HRESULT hr = S_OK;

    if (!serverRunning) return 0;
    
	DATASOCKET_disconnectFromServer ();
    
    hr = DS_ControlLocalServer(DSConst_ServerClose);
//    if (hr < 0) return DATASOCKET_error (hr, "DS_ControlLocalServer (DSConst_ServerClose)");
    
//    if (hr == S_OK)
        serverRunning = FALSE;
        
    return 0;
}



int watchdogOn (t_sequence *seq)
{
    if (!seq->considerWatchdog) return 0;
	WATCHDOG_getStatus ();
	return watchdogStatus != STATUS_OK;
}



int watchdogConfigure (void)
{
	return 0;
	
}


void WATCHDOG_setLed1 (int draw)
{
	if (panelLED_watchdog > 0) {
		SetCtrlAttribute (panelLED_watchdog, ctrlLED_watchdog, ATTR_ON_COLOR,
						  watchdogStatus == STATUS_PREALERT ? VAL_YELLOW : VAL_RED);
		SetCtrlVal (panelLED_watchdog, ctrlLED_watchdog, watchdogStatus != STATUS_OK);
	}
//	printf (".");
	if (draw) ProcessDrawEvents();
//	SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
}


void WATCHDOG_setLed2 (int draw)
{
	if (panelLED_watchdog2 > 0) {
		SetCtrlAttribute (panelLED_watchdog2, ctrlLED_watchdog2, ATTR_ON_COLOR,
						  watchdogStatus == STATUS_PREALERT ? VAL_YELLOW : VAL_RED);
		SetCtrlVal (panelLED_watchdog2, ctrlLED_watchdog2, watchdogStatus != STATUS_OK);
	}
//	printf (".");
	if (draw) ProcessDrawEvents();
//	SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
}


void WATCHDOG_handshake (void)
{
	switch (watchdogStatus) {
		case STATUS_OK:
			WATCHDOG_setReceived (0);
			break;
		case STATUS_UNLOCKED:
			WATCHDOG_setReceived (1);
			break;
	}
}



void WATCHDOG_getStatus (void)
{
    HRESULT hr = S_OK;
    static int oldStatus;
    
    oldStatus = watchdogStatus;

	hr = DS_GetDataValue (DShandle_watchdogStatus, CAVT_LONG,
						  &watchdogStatus, 0, 0, 0);
	
	// no sequence running: 
	if (oldStatus != watchdogStatus) {
		// no sequence is active: set received flag directly to 1
		if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
	 		WATCHDOG_handshake ();
		}
		WATCHDOG_setLed1 (0);		
		WATCHDOG_setLed2 (1);		
	}
}





void WATCHDOG_setReceived (int newStatus)
{
    HRESULT hr = S_OK;
    
    if (newStatus >= 0) statusReceived = newStatus;

	hr = DS_SetDataValue (DShandle_watchdogReceived, CAVT_LONG,
						  &statusReceived, 0, 0);
	printState ();
	WATCHDOG_setLed1 (0);								  
	WATCHDOG_setLed2 (0);								  
}




void CVICALLBACK DS_CommandReceived (DSHandle dsHandle, int event, void *callbackData)
{
    HRESULT hr = S_OK;
    char message[1000];
    int error;
    char *variable;
    char *buffer = NULL;
	static int dataSockedConnected;
	long bufferSize;
	long updated = 0;
	long received = 0;
	unsigned readSize;
	
	variable = (char *) callbackData;

	switch (event) {
        case DS_EVENT_STATUSUPDATED: /* on status updated*/
            hr = DS_GetLastMessage (dsHandle, message, 1000);
            strcat (message, variable);
            strcat (message, "\n");
			dataSockedConnected = (strstr (message, "Active: Connected") != 0);
			error = (strstr (message, "DataSocket error.") != 0);
			tprintf (message);
//            if (panelStatus_watchdog > 0) {
//            	SetCtrlVal (panelStatus_watchdog, ctrlStatus_watchdog, message);
//				SetCtrlAttribute (panelStatus_watchdog, ctrlStatus_watchdog,
//  ATTR_TEXT_COLOR, error ? VAL_RED: VAL_BLACK);
//			}
			break;
		case DS_EVENT_DATAUPDATED:
			hr = DS_GetDataValue (DShandle_commandUpdated, CAVT_LONG,
								  &updated, 0, 0, 0);
			if (hr != STATUS_OK) return;
			if (!updated) return;					
			hr = DS_GetAttrValue (DShandle_command, DS_ATTRIBUTE_SIZE, CAVT_LONG, &bufferSize, 0, 0,
							 0);
			if (hr != STATUS_OK) bufferSize = DS_MAX_BUFFERSIZE;

			buffer = (char *) malloc (bufferSize);
			hr = DS_GetDataValue (DShandle_command,
								CAVT_UCHAR | CAVT_ARRAY, buffer,
								bufferSize, &readSize, 0);
			// handshake
			if (hr == S_OK) {
				received = 1;
				DS_SetDataValue (DShandle_commandReceived, CAVT_LONG,
								  &received, 0, 0);
			}

			MAIN_parseTCPString (buffer, readSize);								
			tprintf ("received: %s", buffer);
			free (buffer);
			updated = 0;
			hr = DS_SetDataValue (DShandle_commandUpdated, CAVT_LONG,
								  &updated, 0, 0);
			break;
	}
    return;
}



void CVICALLBACK DS_watchdog_dataChanged (DSHandle dsHandle, int event, void *callbackData)
{
    HRESULT hr = S_OK;
    char message[1000];
    int error;
    char *variable;
    
	static int dataSockedConnected;
	variable = (char *) callbackData;

	switch (event) {
        case DS_EVENT_STATUSUPDATED: /* on status updated*/
            hr = DS_GetLastMessage (dsHandle, message, 1000);
            strcat (message, variable);
            strcat (message, "\n");
			dataSockedConnected = (strstr (message, "Active: Connected") != 0);
			error = (strstr (message, "DataSocket error.") != 0);
            if (panelStatus_watchdog > 0) {
				SetCtrlVal (panelStatus_watchdog, ctrlStatus_watchdog, "Watchdog: ");
            	SetCtrlVal (panelStatus_watchdog, ctrlStatus_watchdog, message);
				SetCtrlAttribute (panelStatus_watchdog, ctrlStatus_watchdog,
								  ATTR_TEXT_COLOR, error ? VAL_RED: VAL_BLACK);
			}
			break;
		case DS_EVENT_DATAUPDATED:
			WATCHDOG_getStatus ();
			break;
	}
    return;
}


/*
void CVICALLBACK DS_received (DSHandle dsHandle, int event, void *callbackData)
{
    HRESULT hr = S_OK;
    char message[1000];
	
	switch (event) {
        case DS_EVENT_STATUSUPDATED: 
            hr = DS_GetLastMessage (dsHandle, message, 1000);
//			SetCtrlVal (panelMain, MAIN_STRING_status, message);
			break;
		case DS_EVENT_DATAUPDATED:
			hr = DS_GetDataValue (DShandle_watchdogReceived, CAVT_FLOAT, &statusReceived, 0,
								  0, 0);
			SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
			break;
			
	}
    return;
}
 */

int DATASOCKET_disconnectFromServer (void)
{
	tprintf ("Disconnecting from DataSocket server...  ");
	if (DShandle_watchdogStatus) {
        DS_DiscardObjHandle(DShandle_watchdogStatus); 
        DShandle_watchdogStatus = 0;
    }
	if (DShandle_watchdogReceived) {
        DS_DiscardObjHandle(DShandle_watchdogReceived); 
        DShandle_watchdogReceived = 0;
    }
	if (DShandle_command) {
        DS_DiscardObjHandle(DShandle_command); 
        DShandle_command = 0;
    }
	if (DShandle_commandUpdated) {
        DS_DiscardObjHandle(DShandle_commandUpdated); 
        DShandle_commandUpdated = 0;
    }
	if (DShandle_commandReceived) {
        DS_DiscardObjHandle(DShandle_commandReceived); 
        DShandle_commandReceived = 0;
    }
	if (DShandle_waitingForTrigger) {
        DS_DiscardObjHandle(DShandle_waitingForTrigger); 
        DShandle_waitingForTrigger = 0;
		waitingForTrigger = -1;
	}
	tprintf ("Ok.\n");
    return 0;
}


 
int DATASOCKET_setWaitingForTrigger (int state)
{
    
    HRESULT hr = S_OK;

	if (waitingForTrigger == state) return 0;
	if (DShandle_waitingForTrigger == 0) return 0;

	waitingForTrigger = state;
	hr = DS_SetDataValue (DShandle_waitingForTrigger, CAVT_LONG,
						  &waitingForTrigger, 0, 0);
	return hr != S_OK;
}

 

int DATASOCKET_connectToServer (const char *serverName)
{
	int error;
	char URL[500];

	DATASOCKET_disconnectFromServer ();

	if (panelStatus_watchdog > 0) 
		DeleteTextBoxLines (panelStatus_watchdog, ctrlStatus_watchdog, 0, -1);
	
	tprintf ("Connecting to DataSocket server \"%s\"...  ", serverName);
	// --------------------------------------
	//     open variable "watchdog status"
	// --------------------------------------
	sprintf(URL, DS_WATCHDOG_PATH, serverName, DS_VAR_WD_STATUS);
	error = DS_OpenEx (URL, DSConst_ReadAutoUpdate, DS_watchdog_dataChanged,
					   " (VAR: "DS_VAR_WD_STATUS")", DSConst_EventModel,
					   DSConst_Asynchronous, &DShandle_watchdogStatus);
	// --------------------------------------
	//     open VAR "watchdog received"
	// --------------------------------------
	if (error >= 0) {
		sprintf(URL, DS_WATCHDOG_PATH, serverName, DS_VAR_WD_RECEIVED);
		error = DS_OpenEx (URL, DSConst_WriteAutoUpdate, NULL,
						   " (VAR: "DS_VAR_WD_RECEIVED")", DSConst_EventModel,
						   DSConst_Asynchronous, &DShandle_watchdogReceived);
	}
	// --------------------------------------
	//     open VAR "commandStr"
	// --------------------------------------
	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_COMMAND);
		error = DS_OpenEx (URL, DSConst_ReadAutoUpdate, NULL,
						   " (VAR: "DS_VAR_COMMAND")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_command);
	}

	// --------------------------------------
	//     open VAR "command_updated"
	// --------------------------------------
	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_UPDATED);
		error = DS_OpenEx (URL, DSConst_ReadAutoUpdate, DS_CommandReceived,
						   " (VAR: "DS_VAR_UPDATED")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_commandUpdated);
	}
	

	// --------------------------------------
	//     open variable "command_received"
	// --------------------------------------
	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_RECEIVED);
		error = DS_OpenEx (URL, DSConst_WriteAutoUpdate, NULL,
						   " (VAR: "DS_VAR_RECEIVED")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_commandReceived);
	}

	// ----------------------------------------
	//     open variable "waiting_for_trigger"
	// ----------------------------------------
	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_WAITING_FOR_TRIGGER);
		error = DS_OpenEx (URL, DSConst_WriteAutoUpdate, NULL,
						   " (VAR: "DS_VAR_WAITING_FOR_TRIGGER")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_waitingForTrigger);
		if (error >=0) DATASOCKET_setWaitingForTrigger (0);						   
	}

	if (error < 0) tprintf ("No connection.\n");
	else tprintf ("Ok.\n");
	
	WATCHDOG_setReceived (-1);  	
//	if(DS_Open (serverName, DSConst_Write, DSCounterUpdated_CB,
//				"browsing", &dsCounterUpdatedHandle)<0){

	return 0;
}





int CVICALLBACK WATCHDOG_LED_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
		case EVENT_LEFT_CLICK:
			if ((statusReceived == 0) && (watchdogStatus != STATUS_OK)) {
				WATCHDOG_setReceived (1);
				WATCHDOG_setLed1(0);
				WATCHDOG_setLed2(1);
			}
			return 1;
		default:
			WATCHDOG_setLed1(0);
			WATCHDOG_setLed2(0);
			return 1;
	}
}



void WATCHDOG_setLedCtrl1 (int panel, int ctrl)
{
	panelLED_watchdog = panel;
	ctrlLED_watchdog = ctrl;
}

void WATCHDOG_setLedCtrl2 (int panel, int ctrl)
{
	panelLED_watchdog2 = panel;
	ctrlLED_watchdog2 = ctrl;
}

void WATCHDOG_setStatusCtrl (int panel, int ctrl)
{
	panelStatus_watchdog = panel;
	ctrlStatus_watchdog = ctrl;
}



