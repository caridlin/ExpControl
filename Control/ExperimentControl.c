#include "ExperimentControl.h"
#include <utility.h>
#include <tcpsupp.h>
#include "INTERFACE_TCP_IP.h"
/*************************************************************************

   Experiment-Control
   
   
   (c) Stefan Kuhr 
   
*************************************************************************
*/   	


#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"
#include "CONTROL_HARDWARE_GPIB.h"


#ifdef SMANAGER
   #include "SESSIONMANAGER/SESSIONMANAGER_GUI_Main.h"    
	int quitProgramFlag = 0;
#endif

#define LOOP_IP "192.168.10.2"
#define PORT_NUMBER "1327"	
	
int panelTitle = -1;
extern int panelMain;
extern int panelMain2;

int threadFunctionId;
volatile int threadStopRequest; 


extern const char DS_SERVER[];



void setStartButton (int on);

void test2 (void);


/************************************************************************
 *
 *   Init panels
 *
 ************************************************************************/

void TITLE_initPanel (int lowerStartWindow)
{
	char date[] = __DATE__;
	char time[] = __TIME__;
	double cvi = _CVI_;
	char help[50];

    if (panelTitle == -1) {
    	panelTitle = LoadPanel (0, UIR_File, TITEL);
    	if (lowerStartWindow)
    		SetPanelAttribute (panelTitle, ATTR_TOP, panelTop(panelTitle) + panelHeight(panelTitle));
    }
    

	// insert creation date
 	sprintf (help, "Version %1.2f", ProgramVersion);
	SetCtrlVal (panelTitle, TITEL_TEXTMSG_release, help);
 	sprintf (help, "built: %s, %s  (CVI %1.2f)", date, time, cvi / 100);
	SetCtrlVal (panelTitle, TITEL_TEXTMSG_date, help);
    
	SetCtrlAttribute (panelTitle, TITEL_TEXTMSG_Expc,
					  ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute (panelTitle, TITEL_TEXTMSG_release,
					  ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute (panelTitle, TITEL_TEXTMSG_Kuhr,
					  ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute (panelTitle, TITEL_TEXTMSG_date,
					  ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute (panelTitle, TITEL_STRING_status,
					  ATTR_TEXT_BGCOLOR, VAL_WHITE);
	SetCtrlAttribute (panelTitle, TITEL_COMMANDBUTTON_Close,
					  ATTR_VISIBLE, 0);
	SetPanelAttribute (panelTitle, ATTR_BACKCOLOR, VAL_WHITE);

}






int initPanels (int outPanel, int outCtrl)
{
//    initStdIO();
    if (MAIN_initPanel (outPanel, outCtrl) < 0) return -1;
    if (MAIN2_initPanel (outPanel, outCtrl) < 0) return -1;

    return 0;
    
}




/************************************************************************
 *
 *    Menu callbacks 
 *
 ************************************************************************/

//----------------------------------------------------------------------
//
//       "About"
//
//----------------------------------------------------------------------
void CVICALLBACK MENU_About (int menuBar, int menuItem, void *callbackData,
        int panel)
{
	SetCtrlAttribute (panelTitle, TITEL_COMMANDBUTTON_Close,
					  ATTR_VISIBLE, 1);
	SetCtrlAttribute (panelTitle, TITEL_STRING_status,
					  ATTR_VISIBLE, 0);
    InstallPopup (panelTitle);
}                                                       

//----------------------------------------------------------------------
//
//   close title panel
//
//----------------------------------------------------------------------

int CVICALLBACK TITLE_close_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			RemovePopup (0);
			break;
		}
	return 0;
}





void parseCommandLine (int argc, char *argv[]) 
{
    int i;

    for (i=0; i<argc; i++) {
        if (strcmp (argv[i], "/DEBUG") == 0) {
            // display some more information in some routines
            // and load a predefined testfile, see testIni()
            config->DEBUGMODE = 1;
        }
        else if (strcmp (argv[i], "/NOCHECK") == 0) {
            // skip check and initialization of boards at startup
            config->CHECK_DISABLED = 1;
        }
        else if (strcmp (argv[i], "/SESSIONMANAGER") == 0) {
           // skip check and initialization of boards at startup
//		    config->START_SESSIONMANAGER = 1;
        }
        else if (strcmp (argv[i], "/REMOTE") == 0) {
           // skip check and initialization of boards at startup
		    config->REMOTE = 1;
        }
        else if (strcmp (argv[i], "/GPIB") == 0) {
            // enable GPIB-commands
		    config->enableGPIB = 1;
        }
/*        else if (strcmp (argv[i], "/FAKEDATA") == 0) {
            // does not stop if no hardware detected, but fakes data
            config->FAKEDATA = 1;
        }*/
    }

}




#ifdef GUI_MAIN
int launchExperimentControl(void) 
{
	return 0;
}
#endif




void test (void)
{
	short testmem[111];
	short source = 16000;
	
	memfill (testmem, &source, 2, 111);
}



int main(int argc, char *argv[])
{
	// initialize networking
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ADDRINFOA *result = NULL;
	ADDRINFOA hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(LOOP_IP, PORT_NUMBER, &hints, &result);
	
	// start connecting on our PORT_NUMBER
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// connect to system control
	connect(ConnectSocket, result->ai_addr, (int)(result->ai_addrlen));
	freeaddrinfo(result);
	  
	int thereIsAnother;
	
	/* avoid to show a task button in windows for the application and every window (which results in two buttons for one window) */
	SetSystemAttribute(ATTR_TASKBAR_BUTTON_VISIBLE,FALSE);
	SetSystemAttribute(ATTR_DISABLE_PROG_PANEL_SIZE_EVENTS,TRUE);
	
    if (InitCVIRTE (0, argv, 0) == 0)   /* Initialize CVI libraries */
        return (-1);    /* out of memory */
	
	if (CheckForDuplicateAppInstance (ACTIVATE_OTHER_INSTANCE, &thereIsAnother) < 0)
		return -1; /* out of memory, or not Win 2000/NT/9x */
	if (thereIsAnother) return 0; /* prevent duplicate instance */

	
//	if (!isIPAddressOK ()) return 0;
	setLaunchState (0);

	if (PROTOCOLFILE_active) PROTOCOLFILE_open (PROTOCOLFILE_name);
	
   	DisableBreakOnLibraryErrors ();
	SetSleepPolicy (VAL_SLEEP_SOME);
	
	// initialize all program settings
	config = (t_config *) malloc (sizeof (t_config));
	CONFIG_init (config);		  
	// create list of all supported GPIB devices
	GPIB_deviceInitAllAvailableDevices ();
	// ------------------------------------------
	//    parse command line 
	// ------------------------------------------
    parseCommandLine (argc, argv);
   	// ------------------------------------------
	//    load and init title panels 
	// ------------------------------------------
    TITLE_initPanel (config->REMOTE);
    if (!config->REMOTE) DisplayPanel (panelTitle);
	// -------------------------------------
	//      read config data
	// -------------------------------------
    CONFIG_read (config, 0);

	// -------------------------------------
	//      start data socket server
	// -------------------------------------
	SetCtrlVal (panelTitle, TITEL_STRING_status, "Launching DataSocket server...");
	DATASOCKET_initServer (config->launchDataSocketServer);
    
   	// ------------------------------------------
	// ####  load and init other panels 
	// ------------------------------------------
    if (initPanels(panelTitle, TITEL_STRING_status) < 0) goto ERRORID;
	

	SetCtrlVal (panelTitle, TITEL_STRING_status, "Resetting outputs...");


	SetCtrlVal (panelTitle, TITEL_STRING_status, "Connecting to DataSocket server...");
	if (config->watchdogActivate) {
		DATASOCKET_connectToServer (DS_SERVER);
	}

	// ------------------------------------------
	//    get last settings from registry
	//    + load stored sequences
	// ------------------------------------------
	SetCtrlVal (panelTitle, TITEL_STRING_status, "Loading sequences...");
    CONFIG_read (config, 1);
	if (activeSeq()== NULL) {
		MENU_New (-1, -1, NULL, -1);
	}
	HidePanel (panelTitle);
	
// 
//	config->enableTCP = 0;
	//if (config->REMOTE) {
	//	SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_MINIMIZE);
	//} else {
		DisplayPanel (panelMain);
		SetPanelSize(panelMain,848,1674);
		SetPanelPos(panelMain,44,67);
	
		
		
	//}
	if (config->enableTCP) TCP_initServer (MAIN_parseTCPString);

//	if (config->START_SESSIONMANAGER) {
#ifdef SMANAGER
		setLaunchState (1);
		SESSIONMANAGER_init(-1, -1);	

		SetMenuBarAttribute (MAINMENU, MAINMENU_FILE_QUIT,
						 ATTR_CALLBACK_FUNCTION_POINTER,
						 MAIN_MENU_QuitProgram);

#endif
		//SetPanelAttribute (panelMain, ATTR_VISIBLE, 0);
//	}
	SetWaitCursor (0);
	
	if (config->REMOTE) {
		SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_MINIMIZE);
	}
	
	setLaunchState (1);
//	displayMemoryInfo ();
	DisplayPanel (panelMain2);
	SetPanelAttribute (panelMain2, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	SetActivePanel (panelMain2);

	HARDWARE_boot (panelMain, MAIN_TEXTBOX_Output1);
	SEQUENCE_setAsActive (config->activeSequenceNr);	
	
//	test2();
	RunUserInterface ();
	
	if (config->enableTCP) TCP_closeServer ();
    
//    SetBreakOnFirstChanceExceptions (0);

ERRORID:	
	setLaunchState (0);
	PROTOCOLFILE_close();
	return 0;
}




void quitProgram (int remoteQuit)
{   
	// shut down networking
	closesocket(ConnectSocket);
	WSACleanup();
	
	threadStopRequest = 1;
	tprintf("saving config ...\n");  
	ProcessDrawEvents();
	CONFIG_write (config); // save config
	tprintf("closing program ...\n");       
	ProcessDrawEvents();
	
	if (threadFunctionId != 0) {
		// wait for thread to stop
		tprintf("waiting for background thread to stop ...\n");
		ProcessDrawEvents();
		if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) {
			CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		}
	}
	//if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
	if (MAIN_quit () != 0) return;
	DATASOCKET_closeServer ();
	#ifdef _CVI_DEBUG_        	
		exit (EXIT_SUCCESS);
	#else	
		QuitUserInterface (0);
	#endif  
	//}
}




//=======================================================================
//
//    Menu: "quit" 
//
//=======================================================================
void CVICALLBACK MAIN_MENU_QuitProgram (int menuBar, int menuItem, void *callbackData,
        int panel)
{
    quitProgram (0);
}








void test2 (void)
{
/*
	int i;
	t_digitizeParameters *p;
	double v2;
	int dig;
	

	return;
	
	
//	p = digitizeParameters_ADWIN();
	
	
	v2 = 0;
	dig = AnalogToDigital (v2, p);
	DebugPrintf ("%2.4f --> %5d --> %2.8f \n", v2, dig, DigitalToAnalog (dig,p));

	v2 = 0.1;
	dig = AnalogToDigital(v2, p);
	DebugPrintf ("%2.4f --> %5d --> %2.8f \n", v2, dig, DigitalToAnalog (dig,p));

	v2 = 1;
	dig = AnalogToDigital (v2, p);
	DebugPrintf ("%2.4f --> %5d --> %2.8f \n", v2, dig, DigitalToAnalog (dig,p));
*/
	
/*	v2 = 9;
	dig = AnalogToDigital (v2, p);
	DebugPrintf ("%2.4f --> %5d --> %2.8f -->%5d \n", v2, dig, DigitalToAnalog (dig+1,p),AnalogToDigital (DigitalToAnalog (dig,p),p));
	

	v2 = 10;
	dig = AnalogToDigital (v2, p);
	DebugPrintf ("%2.4f --> %5d --> %2.8f \n", v2, dig, DigitalToAnalog (dig,p));
	
	
	return;
	for (i = p->minDigital; i < p->maxDigital; i++) {
		v2 = DigitalToAnalog (i,p);
		if (i % 100 == 0) DebugPrintf ("%5d --> %2.4f --> %5d \n", i, v2, AnalogToDigital (v2,p));
//		if (v2 != i) Breakpoint();
	}
	
	return;
*/	
}

