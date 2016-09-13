#include "dataskt.h"
#include <ansi_c.h>
#include <userint.h>

#include "CONTROL_definitions.h"


long updated = 0;
long received = 0;

DSHandle DShandle_command = 0;
DSHandle DShandle_commandUpdated = 0;
DSHandle DShandle_commandReceived = 0;


int panelMessage = -1;
int ctrlMessage = -1;


void DATASOCKET_setDisplayCtrl (int panel, int ctrl)
{
	panelMessage = panel;
	ctrlMessage  = ctrl;
}



int DATASOCKET_transferBuffer (const char *buffer, unsigned bufferSize)
{
    HRESULT hr = S_OK;
	
	// last command not yet read
	if (updated) return 1;
	
	if (DShandle_command == 0) return -1;
	if (DShandle_commandUpdated == 0) return -1;
	
	hr = DS_SetDataValue (DShandle_command,
						  CAVT_UCHAR | CAVT_ARRAY, buffer, bufferSize,
						  0);
	if (hr != S_OK) return -1;
	hr = DS_SetAttrValue (DShandle_command, DS_ATTRIBUTE_SIZE,
						  CAVT_LONG, &bufferSize, 0, 0);
						  
	if (hr != S_OK) return -1;
	hr = DS_Update (DShandle_command);
	if (hr != S_OK) return -1;
	
	// set update flag
	updated = 1;
	hr = DS_SetDataValue (DShandle_commandUpdated,
						  CAVT_LONG, &updated, 0, 0);
	if (hr != S_OK) return -1;
	
	return 0;
}


int DATASOCKET_transferCommand (const char *commandStr)
{
	return DATASOCKET_transferBuffer (commandStr, strlen (commandStr)+1);
}





void CVICALLBACK DS_received (DSHandle dsHandle, int event, void *callbackData)
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
			error = (strstr (message, "Error:") != 0);
			if (panelMessage > 0) {
				SetCtrlVal (panelMessage, ctrlMessage, message);
				SetCtrlAttribute (panelMessage, ctrlMessage, 
								   ATTR_TEXT_COLOR, error ? VAL_RED: VAL_BLACK);
			}
			break;
		case DS_EVENT_DATAUPDATED:
			// handshake
			hr = DS_GetDataValue (DShandle_commandReceived, CAVT_LONG,
								  &received, 0, 0, 0);
			if ((hr == S_OK) && received) {
				updated = 0;
				DS_SetDataValue (DShandle_commandUpdated, CAVT_LONG,
								  &updated, 0, 0);
			}
			break;
	}
    return;
}



int DATASOCKET_openConnection (const char *serverName)
{
	int error = 0;
	char URL[500];

	sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_COMMAND);
	error = DS_OpenEx (URL, DSConst_Write, NULL,
					   " (VAR: "DS_VAR_COMMAND")",
					   DSConst_EventModel, DSConst_Asynchronous,
					   &DShandle_command);

	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_UPDATED);
		error = DS_OpenEx (URL, DSConst_WriteAutoUpdate, NULL,
						   " (VAR: "DS_VAR_UPDATED")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_commandUpdated);
	}

	if (error >= 0) {
		sprintf(URL, DS_EXPCONTROL_PATH, serverName, DS_VAR_RECEIVED);
		error = DS_OpenEx (URL, DSConst_ReadAutoUpdate, DS_received,
						   " (VAR: "DS_VAR_RECEIVED")",
						   DSConst_EventModel, DSConst_Asynchronous,
						   &DShandle_commandReceived);
	}
	return error;
}



