#include "UIR_TCP_Demo.h"
#include <utility.h>
#include <userint.h>
#include "UIR_TCP_IP.h"
#include "INTERFACE_TCP_IP.h"    
#include "tools.h"
#include "easytab.h"



#define filenameUIR "UIR_TCP_Demo.uir"

int panelDemo = -1;
int panelCommands = -1;



char serverName[] = "localhost";




// this function is called when the client
// receives a TCP/IP command

void myParseTCPString (char *buffer, unsigned long bufferSize)
{
	char *parts[MAX_COMMANDSTRING_PARTS];
	int nParts;

    cutCommandString (buffer, parts, &nParts);
    
    if (stricmp (parts[0], TCP_CMD_DATA) == 0) {
    	// data received ...
    	//flag = 1;
    }
    else if (stricmp (parts[0], TCP_CMD_STOPPED) == 0) {
		// if not stopped by user:
		InsertTextBoxLine (panelCommands, COMMANDS_TEXTBOX, -1, "Sequence stopped.");
	}
	///
		

}	 




int main(int argc, char *argv[])
{
   
    if (InitCVIRTE (0, argv, 0) == 0)   /* Initialize CVI libraries */
        return (-1);    /* out of memory */
	
	// load main panel and init tab
	panelDemo = LoadPanel (0, filenameUIR, DEMO);
	EasyTab_ConvertFromCanvas (panelDemo, DEMO_CANVAS);
	
	EasyTab_LoadPanels (panelDemo, DEMO_CANVAS, 1, filenameUIR,
						__CVIUserHInst, COMMANDS, &panelCommands,0);

	// init client panel
	TCP_initPanelClient (0);
	// insert into tab
//	TCP_initPanelClient (panelDemo);
//	EasyTab_AddPanels (panelDemo, DEMO_CANVAS, 1, TCP_panelClient(), 0);
	
	SetPanelPos (TCP_panelClient(), 100, 100);
	DisplayPanel (TCP_panelClient());
	DisplayPanel (panelDemo);
	
	DisableBreakOnLibraryErrors ();
	
	TCP_initClient (serverName, myParseTCPString);

	RunUserInterface ();

	return 0;
}










int CVICALLBACK Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			QuitUserInterface (0); 
			break;
	}
	return 0;
}

int CVICALLBACK callback_F9 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			TCP_clientSendCommand (TCP_CMD_F9);
			break;
	}
	return 0;
}

int CVICALLBACK callback_Send (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char string[200];
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panelCommands, COMMANDS_STRING, string);
			TCP_clientSendCommand (string);
			break;
	}
	return 0;
}
