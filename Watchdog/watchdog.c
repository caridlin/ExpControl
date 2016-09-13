#include <analysis.h>
#include "inifile.h"
#include <ansi_c.h>
#include <userint.h>
#include "UIR_watchdog.h"  
//#include "fieldpoint.h"
#include "dataskt.h"
//#include "FieldPoint.h"
#include <utility.h>
#include "easytab.h"
#include "watchdog.h"
#include "watchdog_variables.h"
#include "tools.h"
#include "NIDAQmx.h"

#define TESTMODE 1

#define N_SCREENS 2

int panelMain = -1;
int panelConfig = -1;
int panelChannelConf = -1;
int panelAlert[N_SCREENS] = {-1, -1};
int nPlaySound = 0;

#define N_SOUND_REPEAT 1
DSHandle DShandle_status = 0;
DSHandle DShandle_received = 0;
int DS_lastError = 0;

const char WATCHDOG_UIR[] =  "UIR_watchdog.uir";


#define CONFIG_FILENAME "watchdog.config"
#define DEFAULT_filenameIAK "config_S15a.iak"
#define DEFAULT_dataSocketServer "dstp://troubadix.lkb.ens.fr"

#define DEFAULT_resource       "FP @ 129_199_119_23"
#define DEFAULT_DIGOutModule   "FP-DO-403 @7"
#define DEFAULT_DIGInModule    "FP-DI-330 @6"

#define DEFAULT_name		    "laser %d"
//#define DEFAULT_AIModule       "FP-AI-100 @5"
#define DEFAULT_channelName	   "Dev3/Ai0"

#define FIRST_PANEL_TOP 103
#define FIRST_PANEL_LEFT 10

#define MIN_PANEL_HEIGHT 80
#define MIN_PANEL_WIDTH 300

#define FP_CALLBACK_ASYNCHRONOUS 1

#define COLOR_LINE      VAL_GREEN
#define COLOR_THRESHOLD VAL_DK_YELLOW
#define COLOR_UNLOCKED  VAL_ORANGE
#define COLOR_PREALERT  VAL_YELLOW

t_config *config = NULL;
t_config *newConfig = NULL;

//IAHandle FP_server;
//IAHandle IOhandleUnlocked = 0;
//IAHandle IOhandleBeeper   = 0;
//IAHandle IOhandleReceived = 0;

long watchdogStatus = 0;
long statusReceived = 0;




//#define RECEIVED_BUFSIZE 100
//static IAByte ReceivedBuffer[RECEIVED_BUFSIZE];   //Buffer to hold advise data.
//static IAByte* Databuffer;  		//Pointer to Buffer to store callback data.



//GetConfigurationInfo

void Shift (double *inputArray, int numberOfElements, int numShifts, double *outputArray)
{
	memmove (outputArray, inputArray+numShifts*sizeof(double), numberOfElements*sizeof (double));
}




void CHANNEL_allocateMemory (t_channel *c)
{
	int i;
	int size;
	
	free (c->dataBuffer);
	free (c->timeBuffer);
	free (c->statusBuffer);	   
	free (c->thresholdUp);
	free (c->thresholdDown);
	
	if (c->nPoints < MIN_NPOINTS) c->nPoints = MIN_NPOINTS;
	
	size = DATA_SHIFT_BUFFER + c->nPoints;
	c->dataBufferSize = c->nPoints;
	c->dataBuffer = (double *) malloc (sizeof (double) * size);
	c->timeBuffer = (double *) malloc (sizeof (double) * size);
	c->thresholdUp = (double *) malloc (sizeof (double) * size);
	c->thresholdDown = (double *) malloc (sizeof (double) * size);
	c->statusBuffer = (int *) malloc (sizeof (int) * size);
	
	c->posPlotStart = 0;
	// initialize with negative times
	for (i = 0; i < c->dataBufferSize; i++) {
		c->dataBuffer[i] = 0.0;
		c->timeBuffer[i] = - (c->dataBufferSize - i) * c->timeInterval;
		c->statusBuffer[i] = 0;
	}
	c->firstTime = -1;
	c->nValuesRead = -1;
}


void CHANNEL_resetValues (t_channel *c)
{
	c->panel = -1;
	c->alarmState = STATUS_OK;
//	c->alert = 0;
//	c->preAlert = 0;
	c->status = 0;
	c->nAveragedValues = 0;
	c->sum = 0;
	c->firstTime = -1;
	c->lastTime = -1;
	c->valuesToGo = -1;
	c->nValuesRead = -1;
	c->preAlertValuesToDo = 0;

	c->dataBufferSize = 0;
	c->dataBuffer = NULL;
	c->timeBuffer = NULL;
	c->statusBuffer = NULL;
	c->thresholdUp = NULL;
	c->thresholdDown = NULL;
	c->lastValue = 0;
	c->updateGraph = 1;
	
}

void CHANNEL_init (t_channel *c)
{
	c->active = 1;
	c->watch  = 1;
	c->name[0] = 0;
	c->nPoints = DEFAULT_NPOINTS;
	c->nPreAlertPoints =  DEFAULT_NPREALERTPOINTS;
//	c->locked = 0;
	c->thresholdFixedUpper = 2;
	c->thresholdFixedLower = 1;
	c->detectionMode = DETECTIONMODE_DIFFERENCE;
//	c->detectionMode = DETECTIONMODE_FIXED;
	c->thresholdDiff = 0.5;
	c->thresholdDiffAverages = 5;
	c->timeInterval = 100;
//	c->changeDetect = 0;
	
	c->nAverage = 1;
	c->monitorChannel = 0;
	c->saveData = 0;
	c->saveMinutes = 10;
	c->changeUnits = 0;
	c->scaleVolts = 1;
	c->scaleToUnit = 1.0;
	strcpy (c->scaleUnit, "?");
	c->saveFilename[0] = 0;
	c->window = MakeRect (-1, -1, -1, -1);
	
	
	strcpy (c->channel, DEFAULT_channelName);

	CHANNEL_resetValues (c);
}


void CHANNEL_copy (t_channel *dest,  t_channel *source)
{
	memcpy (dest, source, sizeof (t_channel));
	CHANNEL_resetValues (dest);
}

void CHANNEL_free (t_channel *c)
{
	if (c->panel > 0) DiscardPanel (c->panel);
	c->panel = -1;
}


void CHANNEL_displayValues (t_channel *c)
{
	if (c->monitorChannel) {
		SetPanelAttribute (c->panel, ATTR_TITLE, c->name);
		SetCtrlVal	(c->panel, MONITOR_TEXTMSG_unit, c->changeUnits ? c->scaleUnit : "V");
		SetCtrlAttribute (c->panel, MONITOR_NUMERIC_volts, ATTR_VISIBLE, c->changeUnits);
	}
	else {
		SetCtrlVal	(c->panel, CHANNEL_STRING_name, c->name);
		SetCtrlVal	(c->panel, CHANNEL_RADIOBUTTON_watch, c->watch);
		SetCtrlVal	(c->panel, CHANNEL_TEXTMSG_channel, c->channel);
	}
}











// ==========================================================
//
//     save config file (.ini style)
//
// ==========================================================

const char INI_SECTION_CHANNEL[] = "WatchdogChannel%d";
const char INI_TAG_CHANNEL_active[] = "active";
const char INI_TAG_CHANNEL_watch[] = "watch";
const char INI_TAG_CHANNEL_name[] = "name"; 
const char INI_TAG_CHANNEL_module[] = "module";             
const char INI_TAG_CHANNEL_channel[] = "channel";            
const char INI_TAG_CHANNEL_timeInterval[] = "timeInterval";            
const char INI_TAG_CHANNEL_detectionMode[] = "detectionMode";
const char INI_TAG_CHANNEL_thresholdFixedUpper[] = "thresholdFixedUpper";
const char INI_TAG_CHANNEL_thresholdFixedLower[] = "thresholdFixedLower";
const char INI_TAG_CHANNEL_thresholdDiff[] = "thresholdDiff";
const char INI_TAG_CHANNEL_thresholdDiffAverages[] = "thresholdDiffAverages";
const char INI_TAG_CHANNEL_nPoints[] = "nPoints";          
const char INI_TAG_CHANNEL_nAverage[] = "nAverage";   

const char INI_TAG_CHANNEL_monitorChannel[] = "monitorChannel";
const char INI_TAG_CHANNEL_changeDetect[] = "changeDetect";

const char INI_TAG_CHANNEL_lastMinutes[] = "lastMinutes";
const char INI_TAG_CHANNEL_saveMinutes[] = "saveMinutes";
const char INI_TAG_CHANNEL_changeUnits[] = "changeUnits";
const char INI_TAG_CHANNEL_scaleVolts[] = "scaleVolts";
const char INI_TAG_CHANNEL_scaleToUnit[] = "scaleToUnit";
const char INI_TAG_CHANNEL_scaleUnit[] = "scaleUnit";
const char INI_TAG_CHANNEL_saveData[] = "saveData";

const char INI_TAG_CHANNEL_saveFilename[] = "saveFilename";
const char INI_TAG_CHANNEL_window[] = "window";

//const char INI_TAG_CHANNEL_[] = "";


const char INI_SECTION_CONFIG[] = "Config";
const char INI_TAG_CONFIG_nChannels[] = "nChannels";
const char INI_TAG_CONFIG_filenameIAK[] = "filnameIAK";
const char INI_TAG_CONFIG_dataSocketServer[] = "dataSocketServer";
const char INI_TAG_CONFIG_FPresource[]     = "FPresource";
const char INI_TAG_CONFIG_moduleBeeper[]   = "moduleBeeper";
const char INI_TAG_CONFIG_chBeeper[]       = "chBeeper";
const char INI_TAG_CONFIG_beeperActive[]   = "beeperActive";
const char INI_TAG_CONFIG_soundActive[]    = "soundActive";
const char INI_TAG_CONFIG_soundFilename[]   = "soundFilename";

const char INI_TAG_CONFIG_panelWidth[] 	   = "panelWidth";
const char INI_TAG_CONFIG_panelHeight[]    = "panelHeight";
const char INI_TAG_CONFIG_beep[] 	       = "beep";







void Ini_PutChannel (IniText ini, t_channel *c, int nr)
{
    char sectionName[30];
    char tagName[40];
	
	sprintf (sectionName, INI_SECTION_CHANNEL, nr);
	Ini_PutString (ini, sectionName, INI_TAG_CHANNEL_name, c->name); 
	Ini_PutString (ini, sectionName, INI_TAG_CHANNEL_channel, c->channel); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_active, c->active); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_watch, c->watch); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_detectionMode, c->detectionMode); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdFixedUpper, c->thresholdFixedUpper); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdFixedLower, c->thresholdFixedLower); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdDiff, c->thresholdDiff); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_thresholdDiffAverages, c->thresholdDiffAverages); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_nPoints, c->nPoints); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_nAverage, c->nAverage); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_timeInterval, c->timeInterval); 

	//
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_monitorChannel, c->monitorChannel); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_saveMinutes, c->saveMinutes); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_changeUnits, c->changeUnits); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_scaleVolts, c->scaleVolts); 
	Ini_PutInt (ini, sectionName, INI_TAG_CHANNEL_changeUnits, c->changeUnits); 
	Ini_PutDouble (ini, sectionName, INI_TAG_CHANNEL_scaleToUnit, c->scaleToUnit); 
	Ini_PutString (ini, sectionName, INI_TAG_CHANNEL_scaleUnit, c->scaleUnit); 
	Ini_PutString (ini, sectionName, INI_TAG_CHANNEL_saveFilename, c->saveFilename); 
	Ini_PutRect (ini, sectionName, INI_TAG_CHANNEL_window, c->window); 
	
	
	
}


void Ini_GetChannel (IniText ini, t_channel *c, int nr)
{
    char sectionName[30];
    char tagName[40];
	
	sprintf (sectionName, INI_SECTION_CHANNEL, nr);
	Ini_GetStringIntoBuffer (ini, sectionName, INI_TAG_CHANNEL_name, c->name,
							 MAX_CHANNELNAME_LEN); 
	Ini_GetStringIntoBuffer (ini, sectionName, INI_TAG_CHANNEL_channel, c->channel,
							 MAX_CHANNELNAME_LEN); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_active, &c->active); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_watch, &c->watch); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_detectionMode, &c->detectionMode); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdFixedUpper, &c->thresholdFixedUpper); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdFixedLower, &c->thresholdFixedLower); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_thresholdDiff, &c->thresholdDiff); 
	Ini_GetInt    (ini, sectionName, INI_TAG_CHANNEL_thresholdDiffAverages, &c->thresholdDiffAverages); 
	Ini_GetInt    (ini, sectionName, INI_TAG_CHANNEL_nPoints, &c->nPoints); 
	Ini_GetInt    (ini, sectionName, INI_TAG_CHANNEL_nAverage, &c->nAverage); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_timeInterval, &c->timeInterval); 
	

	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_monitorChannel, &c->monitorChannel); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_saveMinutes, &c->saveMinutes); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_changeUnits, &c->changeUnits); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_scaleVolts, &c->scaleVolts); 
	Ini_GetInt (ini, sectionName, INI_TAG_CHANNEL_changeUnits, &c->changeUnits); 
	Ini_GetDouble (ini, sectionName, INI_TAG_CHANNEL_scaleToUnit, &c->scaleToUnit); 
	Ini_GetStringIntoBuffer (ini, sectionName, INI_TAG_CHANNEL_scaleUnit, c->scaleUnit, MAX_UNITNAME_LEN); 
	Ini_GetStringIntoBuffer (ini, sectionName, INI_TAG_CHANNEL_saveFilename, c->saveFilename, MAX_PATHNAME_LEN); 
	Ini_GetRect (ini, sectionName, INI_TAG_CHANNEL_window, &c->window); 
	
}



void Ini_PutConfig (IniText ini, t_config *w)
{
	int i;
	
	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_nChannels, w->nChannels); 
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_filenameIAK, w->filenameIAK);
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_FPresource, w->FP_resourceName);
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_dataSocketServer, w->dataSocketServer);
	// "beeper"
	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_beeperActive, w->beeperActive);
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_moduleBeeper, w->moduleBeeper);
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_chBeeper, w->chBeeper);

	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_panelWidth, w->panelWidth);
	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_panelHeight, w->panelHeight);
	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_beep, w->beep);
	Ini_PutInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_soundActive, w->soundActive);
	Ini_PutString (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_soundFilename, w->soundFilename);

	for (i = 0; i < w->nChannels; i++) {
		Ini_PutChannel (ini, w->channels[i], i);
	}
}


void Ini_GetConfig (IniText ini, t_config *w)
{
	int nChannels = 0;
	int i;
	
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_nChannels, &nChannels); 
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG,
							 INI_TAG_CONFIG_filenameIAK,
							 w->filenameIAK, MAX_PATHNAME_LEN);
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_dataSocketServer, 
							 w->dataSocketServer, MAX_PATHNAME_LEN);
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG,
							 INI_TAG_CONFIG_FPresource,
							 w->FP_resourceName,
							 MAX_MODULENAME_LEN);
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG,
							 INI_TAG_CONFIG_moduleBeeper,
							 w->moduleBeeper, MAX_MODULENAME_LEN);
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG,
							 INI_TAG_CONFIG_chBeeper, w->chBeeper,
							 MAX_CHANNELNAME_LEN);
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_beeperActive, &w->beeperActive);
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_panelWidth, &w->panelWidth);
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_panelHeight, &w->panelHeight);
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_beep, &w->beep);
	Ini_GetInt (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_soundActive, &w->soundActive);
	Ini_GetStringIntoBuffer (ini, INI_SECTION_CONFIG, INI_TAG_CONFIG_soundFilename, 
							 w->soundFilename, MAX_PATHNAME_LEN);
	
	
	CONFIG_allocate (w, nChannels);
	for (i = 0; i < w->nChannels; i++) {
		Ini_GetChannel (ini, w->channels[i], i);
//		CHANNEL_allocateMemory (w->channels[i]);
	}
}




// ================================================
//
//		t_config: functions
//
// ================================================

void CONFIG_init (t_config *w)
{
	char dirname[MAX_PATHNAME_LEN];
	
	GetProjectDir (dirname);
	MakePathname (dirname, CONFIG_FILENAME, w->filename);
	
	strcpy (w->FP_resourceName, DEFAULT_resource);
	strcpy (w->dataSocketServer, DEFAULT_dataSocketServer);
	w->beeperActive   = 1;
	w->beep = 1;
//	w->suppressProtocol = 0;
	w->soundActive = 0;
	w->soundFilename[0] = 0;

	
	strcpy (w->moduleBeeper,   DEFAULT_DIGOutModule); 
	strcpy (w->chBeeper, DEFAULT_channelName); 
	strcpy (w->filenameIAK, DEFAULT_filenameIAK);
	
	w->nChannels = 0;
	w->channels = NULL;
	w->panelWidth = 600;
	w->panelHeight = 100;
}




void CONFIG_allocate (t_config *w, int nChannels)
{
	t_channel **newChannels;
	int i;
	
	newChannels = (t_channel **) malloc (nChannels * sizeof (t_channel*));
	// copy existing channels
	
	for (i = 0; i < nChannels; i++) {
		if (i < w->nChannels) newChannels[i] = w->channels[i];
		else {
			newChannels[i] = (t_channel *) malloc (sizeof (t_channel));
			CHANNEL_init (newChannels[i]);
		}
	}
	// free channels that are no longer used
	for (i = nChannels; i < w->nChannels; i++) {
		CHANNEL_free (w->channels[i]);
		free (w->channels[i]);
	}
	
	w->nChannels = nChannels;
	free (w->channels);
	w->channels = newChannels;
}



void CONFIG_free (t_config *w)
{
	int i;
	
	for (i = 0; i < w->nChannels; i++) {
		CHANNEL_free (w->channels[i]);
		free (w->channels[i]);
	}
	free (w->channels);
	CONFIG_init (w);
}



void CONFIG_setDefault (t_config  *w)
{
	int i;
	
	CONFIG_allocate (w, 3);

	for (i = 0; i < w->nChannels; i++) {
		CHANNEL_allocateMemory (w->channels[i]);
		sprintf (w->channels[i]->name, DEFAULT_name, i);
	}
}



void CONFIG_initPanel (t_config *w)
{
	if (panelConfig == -1) {
		panelConfig = LoadPanel (0, WATCHDOG_UIR, CONFIG);  
		SetCtrlAttribute (panelConfig, CONFIG_STRING_resource,
						  ATTR_MAX_ENTRY_CHARS, MAX_RESOURCENAME_LEN-1);
		SetCtrlAttribute (panelConfig, CONFIG_STRING_moduleBeeper,
						  ATTR_MAX_ENTRY_CHARS, MAX_MODULENAME_LEN-1);
		SetCtrlAttribute (panelConfig, CONFIG_STRING_chBeeper,
						  ATTR_MAX_ENTRY_CHARS, MAX_CHANNELNAME_LEN-1);
	}
	SetPanelAttribute (panelConfig, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelConfig, CONFIG_COMMANDBUTTON_done, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelConfig, CONFIG_COMMANDBUTTON_apply, ATTR_DIMMED, 0);
	
	
}





int CONFIG_load (t_config *c)
{
    IniText ini;
    int err;

	if (!FileExists (c->filename, 0)) {
		if (ConfirmPopupf ("Warning!", "The configuration file\n\n'%s'\n\ncannot be opened.\n\nCreate a new config file with default settings instead?", c->filename)) {
			CONFIG_setDefault (c);
			CONFIG_save (c);
			return 0;
		}
		else {
			return -1;
		}
	}
	
	ini = Ini_New (1);
	if ((err = Ini_ReadFromFile (ini, c->filename)) != 0) goto ERROR2;
	
    Ini_GetConfig (ini, c);
	
	Ini_Dispose (ini);
	return 0;

ERROR2:
	// display error
	SetWaitCursor (0);
	displayIniFileError (c->filename, err);
	// free memory
	Ini_Dispose (ini);
	return -1;

}
		


int CONFIG_save (t_config *c)
{
    int err;
    IniText ini;
    
	if (c == NULL) return 0;
	ini = Ini_New (0);
	
	SetWaitCursor (1);
	Ini_PutConfig (ini, c);

	err = Ini_WriteToFile(ini, c->filename);
	
	SetWaitCursor (0);
	Ini_Dispose (ini);
	return displayIniFileError (c->filename, err);
}




int MAIN_initPanel (void)
{
	int height;
	Rect bounds;
	char title[100];
	int left2, width2, height2;
	int twoScreens;
	
	if (panelMain == -1) {
		if ((panelMain = LoadPanel (0, WATCHDOG_UIR, MAIN)) < 0)
			return -1;
		setStdTextOut (panelMain, MAIN_TEXTBOX);
		sprintf (title, "Watchdog V%1.2f",WATCHDOG_PROGRAM_VERSION); 
		SetPanelAttribute (panelMain, ATTR_TITLE, title);
	}
	
	WATCHDOG_updateDisplay (config);
	WATCHDOG_initAllPanels (config);
	DisplayPanel (panelMain);
	SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);

	twoScreens = (GetMonitorAttribute (2, ATTR_WIDTH, &width2) >= 0);
	// TODO  ref to useTwoScreens??  could also use ATTR_NUM_MONITORS
	
	panelAlert[0] = LoadPanel (0, WATCHDOG_UIR, ALERT);
	if (twoScreens) {
		panelAlert[1] = LoadPanel (0, WATCHDOG_UIR, ALERT);
		GetMonitorAttribute (2, ATTR_LEFT, &left2);		
		GetMonitorAttribute (2, ATTR_HEIGHT, &height2);		
		SetPanelPos (panelAlert[1], (height2 - panelHeight (panelAlert[1]))/2, 
					left2 + (width2 - panelWidth (panelAlert[1])) /2 );
	}
	return 0;
}



/*
int CVICALLBACK callback_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			CONFIG_save (config);
			if (ConfirmPopup ("Quit Program", "Do you really want to exit Watchdog?")) {
				QuitUserInterface (0);
			}
			break;
	}
	return 0;
}
*/


// ==========================================================
//
//       communication with NIUSB
//
// ==========================================================


int NIUSB_showError (const char *functionName, const char *module, const char *channel, int status)
{
	char message[260]; // Error string
	char moduleStr[MAX_MODULENAME_LEN+20]; 
	char channelStr[MAX_CHANNELNAME_LEN+20]; 

	if (status !=0) {
		// Get the error string from the server
		if (FP_ErrorMsg (FP_server, status, message) != 0) message[0] = 0;
		moduleStr[0] = 0;
		channelStr[0] = 0;
		if (module != NULL) sprintf (moduleStr, "Module: %s   ", module);
		if (channel != NULL) sprintf (channelStr, "Channel: %s", channel);
		
		// Popup the error string
		
		PostMessagePopupf ("Error", "Function %s failed.   %s%s\n\nErrorcode %d: %s", 
					  functionName, moduleStr, channelStr, status, message);
		return -1;
	}
	return 0;
	
}


void CHANNEL_close (t_channel *c, int destroyIOPoint)
{
	int status;
	
//	c->active = 0;
    tprintf ("Closing '%s' at '%s'... ", c->channel, c->module);
   	ProcessDrawEvents ();

	if (status = FP_Stop (FP_server, c->advise_ID)) {
		FP_showError ("FP_Stop", c->module, c->channel, status);
	}
	c->advise_ID = 0;

	if (destroyIOPoint) {
		if (status = FP_DestroyIOPoint (FP_server, c->IO_handle)) {
			FP_showError ("FP_DestroyIOPoint", c->module, c->channel, status);
		}
		// reset IOhandles to 0
		c->IO_handle = 0;
	}

    tprintf ("Ok.\n");
   	ProcessDrawEvents ();
}





int FP_WriteDigLine (t_config *c, IAHandle IOHandle, int state, int errorChk)
{
	int status;
 
 	status = FP_Write (FP_server, IOHandle, (IAByte*)&state, 1);
 	if (errorChk && status) return FP_showError ("FP_Write", 0, 0, status);
	return 0;
}


/*
void CVICALLBACK FP_readReceived (void* buffer)
{
	FP_CallbackParamType g_param;
	unsigned short* value; //IAByte Buffer is cast to unsigned short.
	int i,j;
	g_param = *((FP_CallbackParamType*) buffer); //cast data from void*
	Databuffer = g_param.buffer;				   
	value = (unsigned short*)(Databuffer);	   
	//Read bits from unsigned short and write bit values to indicators.
	statusReceived = *value;
	//  DebugPrintf ("%d", statusReceived);
  
}
*/



int FP_openAll (t_config *c)
{
	static IATaskID Task;				//Advise task ID. Used for starting and stopping advise function. 
	IAStatus status;
	int i;

	stdCls();
	if (!FileExists (c->filenameIAK, 0)) {
		PostMessagePopupf ("Error", "Error. File %s does not exist.", c->filenameIAK);
		return -1;
	}

	tprintf ("Connecting to %s... ", c->FP_resourceName);
	ProcessDrawEvents ();
	if (status = FP_Open (c->filenameIAK, &FP_server) != 0) 
		return FP_showError ("FP_Open", 0, 0, status);
	tprintf ("Ok.\n");
	ProcessDrawEvents ();

	for (i = 0; i < c->nChannels; i++) {
		CHANNEL_allocateMemory (c->channels[i]);
		CHANNEL_open (c, c->channels[i]);
	}
/*
	// open handshake channels
	if (c->unlockedActive) {
		if (status = FP_CreateTagIOPoint (FP_server, c->FP_resourceName, c->moduleUnlocked, c->chUnlocked,
					 &IOhandleUnlocked))
			FP_showError ("FP_Open", c->moduleUnlocked, c->chUnlocked, status);
		else
 			FP_WriteDigLine (c, IOhandleUnlocked, statusUnlocked, 1);
	}
*/
	if (c->beeperActive) {
		if (status = FP_CreateTagIOPoint (FP_server, c->FP_resourceName, c->moduleBeeper, c->chBeeper,
					 &IOhandleBeeper))
			FP_showError ("FP_Open", c->moduleBeeper, c->chBeeper, status);
		else	
 			if (c->beep) FP_WriteDigLine (c, IOhandleBeeper, watchdogStatus == STATUS_UNLOCKED, 1);
	}

/*	if (c->receivedActive) {
		if (status = FP_CreateTagIOPoint (FP_server, c->FP_resourceName, c->moduleReceived, c->chReceived,
					 &IOhandleReceived))
			FP_showError ("FP_Open", c->moduleReceived, c->chReceived, status);
		else
			//FP_Advise is used to acquire data. FPAdviseCallback will be called at the Advise Rate to poll the device.
		if (status = FP_Advise (FP_server, IOhandleReceived, 100, 1,
								ReceivedBuffer, RECEIVED_BUFSIZE, 1,
								FP_readReceived, NULL, &Task))
			FP_showError ("FP_Advise", c->moduleReceived, c->chReceived, status);
								
			
	}
*/		

   WATCHDOG_connectToDataSocketServer (c->dataSocketServer);
		
   return 0;
}
										 

void minMax (double *array, unsigned arraySize, double *min, double *max)
{
	double maxData, minData;
	unsigned indexMax, indexMin;

	MaxMin1D (array, arraySize, &maxData, &indexMax, &minData, &indexMin);
	if (minData < *min) *min = minData;
	if (maxData > *max) *max = maxData;
}





double average (double *array, int start, int nPoints)
{
	double sum;
	int i;
	
	if (nPoints <= 0) return 0;
	sum = 0;
	for (i = 0; i < nPoints; i++) sum += array[start+i];
	return sum / nPoints;
}


void CVICALLBACK FP_readAnalogInput (void* buffer)
{
	
	t_channel *c;
	FP_CallbackParamType *p;
	float *value;
//	double time;
	int n;
	int i;
	int nAvg;
	int visible;
	double avg;
	
	p = (FP_CallbackParamType *) buffer;
	c = (t_channel *) p->cParam;
	value = (float *) p->buffer; 
	
	if (!c->watch && !c->monitorChannel) goto END;
	if (c->valuesToGo == 0) goto END;
	
//	if (c->firstTime < 0) 
//		c->firstTime = 86400.0*p->timeStamp;
//	time = (86400.0*p->timeStamp) - c->firstTime;

	// if time intervals are too short
	// ignore calls where time < lastTime 
//	if (time < c->lastTime) {
//		DebugPrintf ("%s: time=%f, lastTime=%f\n", c->name, time, c->lastTime);
//		return;
//	}
	c->lastTime = p->timeStamp;

	if (c->nAveragedValues == 0) c->sum = 0;
	c->sum+= *value;
	c->nAveragedValues++;
	if (c->nAveragedValues < c->nAverage) goto END;
	
	*value = c->sum / c->nAveragedValues;
	c->nAveragedValues = 0;

	c->posPlotStart ++;
    if (c->posPlotStart >= DATA_SHIFT_BUFFER) {
		// ------------------------------------------------
		//    shift buffer only after DATA_SHIFT_BUFFER 
		//    values have been written
		// ------------------------------------------------
	    Shift (c->dataBuffer, c->dataBufferSize+DATA_SHIFT_BUFFER, -DATA_SHIFT_BUFFER, c->dataBuffer);
	    Shift (c->timeBuffer, c->dataBufferSize+DATA_SHIFT_BUFFER, -DATA_SHIFT_BUFFER, c->timeBuffer);
	    for (i = 0; i < c->dataBufferSize; i++) c->statusBuffer[i] = c->statusBuffer[i+DATA_SHIFT_BUFFER];
		if (c->detectionMode == DETECTIONMODE_DIFFERENCE) {
		    // shift also thresholds
		    Shift (c->thresholdUp, c->dataBufferSize+DATA_SHIFT_BUFFER, -DATA_SHIFT_BUFFER,  c->thresholdUp);
		    Shift (c->thresholdDown, c->dataBufferSize+DATA_SHIFT_BUFFER, -DATA_SHIFT_BUFFER,  c->thresholdDown);
		}
	    c->posPlotStart = 0;
	}
	
	// ------------------------------------------------
	//   store measured values
	// ------------------------------------------------
	n = c->posPlotStart + c->dataBufferSize - 1;
	c->dataBuffer[n] = *value;
	c->timeBuffer[n] = p->timeStamp;
	c->statusBuffer[n] = 0;
	c->lastValue = *value;
	c->updateGraph = 1;

	if (c->nValuesRead == -1) {
		// ------------------------------------------------
		//   first measured value:
		//   fill everything with first value
		// ------------------------------------------------
		for (i = 0; i < c->dataBufferSize+DATA_SHIFT_BUFFER; i++) {
			c->dataBuffer[i] = c->dataBuffer[n];
			c->timeBuffer[i] = -1.0;
		}
		
	}

	if (c->monitorChannel) {
		c->nValuesRead++;
		goto END;
	}
	// calculate new threshold values
	switch (c->detectionMode) {
		case DETECTIONMODE_DIFFERENCE:
			if (c->alarmState != STATUS_OK) {
				// alert: keep last theshold
				c->thresholdUp[n]   = c->thresholdUp[n-1];
				c->thresholdDown[n] = c->thresholdDown[n-1];
			}
			else {
				// NO alert:
				// average the last 'c->thresholdDiffAverages' values
				// (if > nValuesRead)
				if (c->thresholdDiffAverages > c->dataBufferSize) c->thresholdDiffAverages = c->dataBufferSize;
				nAvg = min (c->nValuesRead, c->thresholdDiffAverages);
				if (nAvg <= 0) nAvg = 1;
				c->thresholdUp[n]   = average (c->dataBuffer, n-nAvg, nAvg) + c->thresholdDiff;
				c->thresholdDown[n] = average (c->dataBuffer, n-nAvg, nAvg) - c->thresholdDiff;
//				c->thresholdDown[n] = c->dataBuffer[n-1] - c->thresholdDiff;
			}
			break;
		case DETECTIONMODE_FIXED:
			c->thresholdUp[n] = c->thresholdFixedUpper;
			c->thresholdDown[n] = c->thresholdFixedLower;
			break;
	}
	
	if (c->nValuesRead == -1) {
		// ------------------------------------------------
		//   fill threshold with first value (part 2)
		// ------------------------------------------------
		for (i = 0; i < c->dataBufferSize+DATA_SHIFT_BUFFER; i++) {
			c->thresholdUp[i]   = c->thresholdUp[n];
			c->thresholdDown[i] = c->thresholdDown[n];
			c->statusBuffer[i] = 0;
		}
	}
	c->nValuesRead++;
	
	// check if only a 'c->nValuesToGo' values had to be recorded
	if (c->valuesToGo > 0) c->valuesToGo --;
	switch (c->alarmState) {
		case STATUS_OK:
			// ---------------------------------------------------------------
			//   OK:
			//
			// ---------------------------------------------------------------
			if ((c->dataBuffer[n] > c->thresholdUp[n]) || (c->dataBuffer[n] < c->thresholdDown[n])) {
				// value exceeds thresholds:
				c->alarmState = STATUS_PREALERT;
				c->nValuesRead = 0;
				c->preAlertValuesToDo = c->nPreAlertPoints;
				if (watchdogStatus == STATUS_OK) {
					watchdogStatus = STATUS_PREALERT;
				    DS_setStatus ();
				}
			}
			break;
		case STATUS_PREALERT:
			// ---------------------------------------------------------------
			//   PREALERT:
			//   one of the previous values has passed the thresholds
			// ---------------------------------------------------------------
			c->preAlertValuesToDo--;
			if (c->preAlertValuesToDo == 0) {
				// average the last recorded values
				nAvg = min (c->nValuesRead, c->nPreAlertPoints-2);
				if (nAvg <= 0) nAvg = 1;
				avg = average (c->dataBuffer, n-nAvg+1, nAvg);
				#ifdef _CVI_DEBUG_
					tprintf ("n=%d,nAvg=%d,avg=%f,  ",n,nAvg,avg);
					for (i = -2; i < nAvg+1; i++) tprintf ("[%d]%d=%f, ", n-nAvg+i, c->statusBuffer[n-nAvg+i], c->dataBuffer[n-nAvg+i]);
					tprintf ("\n");
				#endif
		
				// check if average passes limit
				if ((avg > c->thresholdUp[n]) || (avg < c->thresholdDown[n])) {
					c->statusBuffer[n] = STATUS_UNLOCKED; 
					// ------------------------
					//    ALERT! unlocked
					// ------------------------
					c->alarmState = STATUS_UNLOCKED;
					if (watchdogStatus != STATUS_UNLOCKED) {
						watchdogStatus = STATUS_UNLOCKED;
						nPlaySound = N_SOUND_REPEAT;
						if (TESTMODE) c->valuesToGo = c->nPoints / 4;
						FP_setUnlocked (config);
			//			if (config->unlockedActive) FP_WriteDigLine (config, IOhandleUnlocked, statusUnlocked, 0);
						if (config->beeperActive && config->beep) FP_WriteDigLine (config, IOhandleBeeper, watchdogStatus == STATUS_UNLOCKED, 0);
					}
				}
				else {
					// ------------------------
					//    stop PREALARLM state
					// ------------------------
					c->alarmState = STATUS_OK;
					c->nValuesRead = 0;
				}
	
			}
			break;
		case STATUS_UNLOCKED:
			break;
	}
	ALERT_displayPanels (watchdogStatus);
	
	c->statusBuffer[n] = c->alarmState; 

END:
//	FP_ReadCache (FP_handle, c->advise_ID, c->buffer, BUFFER_SIZE, 0);
    if (!FP_CALLBACK_ASYNCHRONOUS) FP_FreePDCallbackBuffer (buffer);	
}




int CHANNEL_open (t_config *w, t_channel *c)
{
	
	if (!c->active) return 0;
    tprintf ("Opening '%s' at '%s'... ", c->channel, c->module);
   	ProcessDrawEvents ();

	if (c->status = FP_CreateTagIOPoint (FP_server, w->FP_resourceName, c->module, c->channel,
								 &c->IO_handle)) 
		return FP_showError ("FP_CreateTagIOPoint", c->module, c->channel, c->status);   

	// Start the advise on the "All" item
 	if (c->status = FP_Advise (FP_server, c->IO_handle, c->timeInterval, 0,
							   c->buffer, FP_BUFFER_SIZE,
							   FP_CALLBACK_ASYNCHRONOUS,
							   FP_readAnalogInput, (IAParam) c,
							   &c->advise_ID))
		return FP_showError ("FP_Advise", c->module, c->channel, c->status);   

    tprintf ("Ok.\n");
   	ProcessDrawEvents ();

	c->firstTime = -1;
	c->lastTime = 0;
	c->nValuesRead = -1;   
	return 0;
}





int FP_closeAll (t_config *c)
{
	int status;
	int i;
	
	tprintf ("Closing %s... ", c->FP_resourceName);
	ProcessDrawEvents ();
	if (status = FP_Close (FP_server) != 0) return FP_showError ("FP_close", 0, 0, status);
	
	// reset all IOhandles to 0
	for (i = 0; i < c->nChannels; i++) {
		c->channels[i]->advise_ID = 0;
		c->channels[i]->IO_handle = 0;
	}

	IOhandleBeeper = 0;
	tprintf ("Ok.");
	ProcessDrawEvents ();

	return 0;
}
//			   SetPanelAttribute

/*
void test (void)
{
	t_channel *testCh;
	
	if (FP_reakIAK (config) != 0) return;
	testCh = config->channels[0];
	CHANNEL_open (config, testCh);

}
*/   




// ==========================================================
//
//       DISPLAY of channel values
//
// ==========================================================



void CHANNEL_monitor_UpdateGraph (t_channel *c)
{
	double *xTimes;
	int i;
	int nValues, start;
	int displaySeconds;
	__int64 seconds;
	
	if (!c->updateGraph) return;
	DeleteGraphPlot (c->panel, MONITOR_GRAPH, -1, VAL_DELAYED_DRAW);
	if (c->nValuesRead <= 0) return;
	// ----------------------
	//  create times
	// ----------------------
	
	seconds = (c->timeInterval * c->nAverage);
	seconds *= c->nPoints;
	displaySeconds = (seconds / 1000);

	SetAxisScalingMode (c->panel, MONITOR_GRAPH,
						VAL_BOTTOM_XAXIS,
						VAL_MANUAL,
						-displaySeconds, 0);


	xTimes = (double *) malloc (sizeof (double) * c->dataBufferSize);
	start = 0;
	for (i = 0; i < c->dataBufferSize; i++) {
		if (c->timeBuffer[c->posPlotStart + i] < 0) start = i+1;
		else xTimes[i] = (c->timeBuffer[c->posPlotStart + i] - c->lastTime) * 86400.0;
	}
/*
		if (c->firstTime < 0) 
		c->firstTime = 86400.0*p->timeStamp;
	time = (86400.0*p->timeStamp) - c->firstTime;

	// if time intervals are too short
	// ignore calls where time < lastTime 
	if (time < c->lastTime) {
//		DebugPrintf ("%s: time=%f, lastTime=%f\n", c->name, time, c->lastTime);
//		return;
	}
	c->lastTime = time;
*/

	nValues = c->dataBufferSize - start;
	if (nValues > 1) {
		PlotXY (c->panel, MONITOR_GRAPH,
		   &xTimes[start], &c->dataBuffer[c->posPlotStart+start], nValues,
		   VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
		   COLOR_LINE);
		SetCtrlAttribute (c->panel, MONITOR_GRAPH, ATTR_VISIBLE, 1);
	}
/*	for (i = 0; i < nValues; i++) {
		printf ("%3.2f %3.2f\n", xTimes[start+i], c->dataBuffer[c->posPlotStart+start+i]);
	}
	printf ("\n");
*/	

/*	PlotY (c->panel, MONITOR_GRAPH,
		   &c->dataBuffer[c->posPlotStart], c->dataBufferSize,
		   VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
		   COLOR_LINE);
*/
	SetCtrlAttribute (c->panel, MONITOR_GRAPH, ATTR_REFRESH_GRAPH, 1);
	SetCtrlAttribute (c->panel, MONITOR_GRAPH, ATTR_YLABEL_VISIBLE,
					  1);
	SetCtrlVal (c->panel, MONITOR_NUMERIC_value, c->lastValue);
	RefreshGraph (c->panel, MONITOR_GRAPH);
	c->updateGraph = 0;
	free (xTimes);
}



void CHANNEL_updateGraph (t_channel *c)
{
	int i;
	double max = -10000.0;
	double min = 10000.0;
	double diff;
	
	int start;
	int lastPoint;

	if (c->panel <= 0) return;
	if (c->monitorChannel) {
		CHANNEL_monitor_UpdateGraph (c);
		return;
	}
	setLED (c->panel, CHANNEL_LED_status, c->alarmState);
	if (!c->watch) return;

	SetCtrlAttribute (c->panel, CHANNEL_GRAPH, ATTR_REFRESH_GRAPH, 0);
	DeleteGraphPlot (c->panel, CHANNEL_GRAPH, -1, VAL_DELAYED_DRAW);
	if (!c->active) {
		PlotLine (c->panel, CHANNEL_GRAPH, 0, -10, 0, 10, VAL_BLACK);
		PlotLine (c->panel, CHANNEL_GRAPH, c->dataBufferSize, -10, c->dataBufferSize, 10, VAL_BLACK);
		goto REFRESH;
	}
	if (c->nValuesRead <= 0) return;

	minMax (&c->dataBuffer[c->posPlotStart], c->dataBufferSize, &min, &max);
	minMax (&c->thresholdUp[c->posPlotStart], c->dataBufferSize, &min, &max);
	minMax (&c->thresholdDown[c->posPlotStart], c->dataBufferSize, &min, &max);
	diff = max - min;
	min -= 0.1 * diff;
	max += 0.1 * diff;


	PlotLine (c->panel, CHANNEL_GRAPH, 0, min, 0, max, VAL_BLACK);
	// get start;
	i = 0;
	start = i;
	while (i < c->dataBufferSize) {
		if ((i == c->dataBufferSize-1) || (c->statusBuffer[c->posPlotStart+i] != c->statusBuffer[c->posPlotStart + start])) {
			// change detected
			lastPoint = i-1;
			if (i == c->dataBufferSize-1) lastPoint+=1;
			switch (c->statusBuffer[c->posPlotStart + start]) {
				case STATUS_UNLOCKED: 
//					DebugPrintf ("%d, start=%d, end=%d\n",c->statusBuffer[c->posPlotStart + start], start, lastPoint);
					if (start-1 < lastPoint) PlotRectangle (c->panel, CHANNEL_GRAPH, start-1, min, lastPoint, max, COLOR_UNLOCKED, COLOR_UNLOCKED);
					break;
				case STATUS_PREALERT:
//					DebugPrintf ("%d, start=%d, end=%d\n",c->statusBuffer[c->posPlotStart + start], start, lastPoint);
					if (start-1 < lastPoint) PlotRectangle (c->panel, CHANNEL_GRAPH, start-1, min, lastPoint, max, COLOR_PREALERT, COLOR_PREALERT);
					break;
			}
			start = i;
		}
		i++;
	}


	PlotY (c->panel, CHANNEL_GRAPH,
		   &c->dataBuffer[c->posPlotStart], c->dataBufferSize,
		   VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
		   COLOR_LINE);

	PlotY (c->panel, CHANNEL_GRAPH,
		   &c->thresholdUp[c->posPlotStart], c->dataBufferSize,
		   VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
		   COLOR_THRESHOLD);

	PlotY (c->panel, CHANNEL_GRAPH,
		   &c->thresholdDown[c->posPlotStart], c->dataBufferSize,
		   VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
		   COLOR_THRESHOLD);
REFRESH:	
	SetCtrlAttribute (c->panel, CHANNEL_GRAPH, ATTR_REFRESH_GRAPH, 1);
	SetCtrlAttribute (c->panel, CHANNEL_GRAPH, ATTR_YLABEL_VISIBLE,
					  1);
	RefreshGraph (c->panel, CHANNEL_GRAPH);
}










int main (int argc, char *argv[])
{
	
	DisableBreakOnLibraryErrors ();
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	config = (t_config *) malloc (sizeof (t_config));
	CONFIG_init (config);
	if (CONFIG_load (config) != 0) return -1;

	if (MAIN_initPanel () != 0) return -1;
	
// TEST
//	test();	
	FP_openAll (config);
	RunUserInterface ();
	CONFIG_save (config);

	DiscardPanel (panelMain);
	return 0;
}


void setLED (int panel, int ctrl, int status)
{
	SetCtrlAttribute (panel, ctrl, ATTR_ON_COLOR,
					  status == STATUS_PREALERT ? VAL_YELLOW : VAL_RED);
	SetCtrlVal (panel, ctrl, status != STATUS_OK);
}



void WATCHDOG_updateDisplay (t_config *c)
{
	setLED (panelMain, MAIN_LED_unlocked, watchdogStatus);
	SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
	SetCtrlVal (panelMain, MAIN_BINARYSWITCH_Beep, config->beep);
}



int CVICALLBACK TIMER_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	
	switch (event) {
		case EVENT_TIMER_TICK:
			for (i = 0; i < config->nChannels; i++) {
				CHANNEL_updateGraph (config->channels[i]);
			}
			WATCHDOG_updateDisplay (config);
			WATCHDOG_checkUnlockedState (config);
			break;
	}
	return 0;
}




int CVICALLBACK WATCHDOG_editConfig_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	
	switch (event) {
		case EVENT_COMMIT:
			if (newConfig == NULL) newConfig = (t_config *) malloc (sizeof (t_config));
			memcpy (newConfig, config, sizeof (t_config));
			newConfig->nChannels = 0;
			newConfig->channels = NULL;
			CONFIG_allocate (newConfig, config->nChannels);
			for (i = 0; i < newConfig->nChannels; i++) {
				CHANNEL_copy (newConfig->channels[i], config->channels[i]);
			}

			CONFIG_initPanel (newConfig);
			CONFIG_setValues (newConfig);
			InstallPopup (panelConfig);
			break;
	}
	return 0;
}

int CVICALLBACK CONFIG_abort_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}


int DS_setStatus (void)
{
    HRESULT hr = S_OK;

    if (DShandle_status) {
		hr = DS_SetDataValue (DShandle_status, CAVT_LONG,
							  &watchdogStatus, 0, 0);
    }
    return hr == S_OK;
}


int FP_setUnlocked (t_config *c)
{
	int status;
	int beepState;
	
    DS_setStatus ();

	if (c->beeperActive) {
		beepState = (watchdogStatus == STATUS_UNLOCKED) && c->beep; 
		if (status = FP_Write (FP_server, IOhandleBeeper, (IAByte*)&beepState, 1)) 
			return FP_showError ("FP_Write", c->moduleBeeper, c->chBeeper, status);   
	}
    
	
	return 0;
}



void CHANNEL_monitor_resizePanel (t_channel *ch)
{
	getPanelBounds (ch->panel, &ch->window);
	SetCtrlAttribute (ch->panel, MONITOR_GRAPH, ATTR_WIDTH, ch->window.width );
	SetCtrlAttribute (ch->panel, MONITOR_GRAPH, ATTR_HEIGHT, 
					  ch->window.height - ctrlTop (ch->panel, MONITOR_GRAPH));
	
}


void CONFIG_editApplyChanges (void)
{
	int i;
	
	SetPanelAttribute (panelConfig, ATTR_DIMMED, 1);
	SuspendTimerCallbacks ();
	
	FP_closeAll (config);
	HidePanel (panelMain);
	WATCHDOG_discardAllPanels (config);
	
	// get new config
	CONFIG_getValues (newConfig);

	memcpy (config, newConfig, sizeof (t_config));
	config->nChannels = 0;
	config->channels = NULL;
	CONFIG_allocate (config, newConfig->nChannels);
	for (i = 0; i < config->nChannels; i++) {
		CHANNEL_copy (config->channels[i], newConfig->channels[i]);
	}
	
	// reopen everything
	WATCHDOG_initAllPanels (config);
	DisplayPanel (panelMain);
	SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM); 
	CONFIG_save (config);

	FP_openAll (config); 
	ResumeTimerCallbacks ();
	SetPanelAttribute (panelConfig, ATTR_DIMMED, 0);

}



void WATCHDOG_discardAllPanels (t_config *w)
{
	int i;
	
	for (i = 0; i < w->nChannels; i++) {
		if (w->channels[i]->panel > 0) {
			DiscardPanel (w->channels[i]->panel);
			w->channels[i]->panel = -1;
		}
			
	}

}


void WATCHDOG_quit (void)
{
	int i;
	
	for (i = 0; i < N_SCREENS; i++) {
		DiscardPanel (panelAlert[i]);
		panelAlert[i] = -1;
	}
	QuitUserInterface(0);
}


void WATCHDOG_initAllPanels (t_config *w)
{
	static int panelChannelMaster = -1;
	static int panelMonitorMaster = -1;
	int pos, top, left;
	int i;
	t_channel *ch;
	int nPanels = 0;
	
	if (panelChannelMaster == -1) {
		panelChannelMaster = LoadPanel (0,  WATCHDOG_UIR, CHANNEL);
		SetCtrlAttribute (panelChannelMaster, CHANNEL_LED_status,
						  ATTR_ON_COLOR, VAL_RED);
		SetCtrlAttribute (panelChannelMaster, CHANNEL_LED_status,
						  ATTR_OFF_COLOR, VAL_GREEN);
		panelMonitorMaster = LoadPanel (0,  WATCHDOG_UIR, MONITOR);
	}
	
	// set panel size
	// position graph;
	pos = ctrlRight (panelChannelMaster, CHANNEL_DECORATION_frame);
	SetCtrlAttribute (panelChannelMaster, CHANNEL_GRAPH, ATTR_LEFT, pos);
	// position decoration frame
	top  = FIRST_PANEL_TOP;
	left = FIRST_PANEL_LEFT;
	if (w->panelWidth < MIN_PANEL_WIDTH) w->panelWidth = MIN_PANEL_WIDTH;
	if (w->panelHeight < MIN_PANEL_HEIGHT) w->panelHeight = MIN_PANEL_HEIGHT;
	for (i = 0; i < w->nChannels; i++) {
		ch = w->channels[i];
		if ((ch->panel <= 0) && (ch->active)) {
			if (ch->monitorChannel) {
				ch->panel = DuplicatePanel (0, panelMonitorMaster, "",
											VAL_AUTO_CENTER, VAL_AUTO_CENTER);
				setPanelBounds (ch->panel, ch->window, 0);
				CHANNEL_monitor_resizePanel (ch);
				SetCtrlAttribute (ch->panel, MONITOR_NUMERIC_value,
								  ATTR_ZPLANE_POSITION, 0);
			}
			else {
				ch->panel = DuplicatePanel (panelMain, panelChannelMaster, "",
													   top+w->panelHeight*nPanels, left);
				SetCtrlAttribute (ch->panel, CHANNEL_GRAPH,
								  ATTR_YLABEL_VISIBLE, 0);
			}
				
//			CHANNEL_updateGraph (ch);
			DisplayPanel (ch->panel);
		}
		if (ch->active) {
			CHANNEL_displayValues (ch);
			if (!ch->monitorChannel) {
				SetPanelPos (ch->panel, top+w->panelHeight*nPanels, left);
				SetPanelSize (ch->panel, w->panelHeight, w->panelWidth);
				SetCtrlAttribute (ch->panel, CHANNEL_GRAPH, ATTR_WIDTH, w->panelWidth - pos);
				SetCtrlAttribute (ch->panel, CHANNEL_GRAPH, ATTR_HEIGHT, w->panelHeight);
				SetCtrlAttribute (ch->panel, CHANNEL_DECORATION_frame,
						  		  ATTR_HEIGHT, w->panelHeight);
				nPanels ++; 
			}
			else {
			}
		}
	}
	SetPanelSize (panelMain, top+w->panelHeight*nPanels+left, w->panelWidth + 2 * left);
}







int CVICALLBACK CONFIG_done_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			CONFIG_editApplyChanges ();
			break;
	}
	return 0;
}


int CVICALLBACK CONFIG_apply_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			CONFIG_editApplyChanges ();
			break;
	}
	return 0;
}




int CVICALLBACK WATCHDOG_reset_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			FP_closeAll (config);
			HidePanel (panelMain);
			WATCHDOG_discardAllPanels (config);
			// reopen everything
			WATCHDOG_initAllPanels (config);
			DisplayPanel (panelMain);
			SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM); 
			FP_openAll (config); 
			break;
	}
	return 0;
}




void CHANNEL_setValues_dimItems (void)
{
	int mode;
	int nPoints;
	int watchdogCh, monitorCh;
	
	GetCtrlVal (panelChannelConf, CHCONF_RINGSLIDE_mode, &mode);
	GetCtrlVal (panelChannelConf, CHCONF_BTN_watchdogCh, &watchdogCh);
	
	SetAttributeForCtrls (panelChannelConf, ATTR_DIMMED, 
						  (mode != DETECTIONMODE_FIXED) || (!watchdogCh), 0,
						  CHCONF_NUMERIC_thresholdUp,
						  CHCONF_NUMERIC_thresholdLow,
						  CHCONF_TEXTMSG_V1,
						  CHCONF_TEXTMSG_V2,
						  0);
	SetAttributeForCtrls (panelChannelConf, ATTR_DIMMED, 
						  (mode != DETECTIONMODE_DIFFERENCE) || (!watchdogCh), 0,
						  CHCONF_NUMERIC_thrAverages,
						  CHCONF_NUMERIC_thresholdDiff,
						  CHCONF_TEXTMSG_points,
						  CHCONF_TEXTMSG_V3,
						  0);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_nPoints, &nPoints);
	SetCtrlAttribute (panelChannelConf, CHCONF_NUMERIC_thrAverages,
					  ATTR_MAX_VALUE, nPoints);
					  
	SetAttributeForCtrls (panelChannelConf, ATTR_DIMMED, 
						  !watchdogCh, 0,
						  CHCONF_RINGSLIDE_mode,						  
//						  CHCONF_NUMERIC_averages,
//						  CHCONF_NUMERIC_nPoints,
						  0);

	
	GetCtrlVal (panelChannelConf, CHCONF_BTN_MonitorCh, &monitorCh);
	SetAttributeForCtrls (panelChannelConf, ATTR_DIMMED, 
						  !monitorCh, 0,
//						  CHCONF_TEXTMSG_min1,
						  CHCONF_STRING_saveFilename,
						  CHCONF_BTN_browseFilename,
						  CHCONF_NUMERIC_saveMinutes,
						  CHCONF_TEXTMSG_min2,
						  CHCONF_BTN_changeUnits,
						  CHCONF_NUMERIC_scaleVolts,
						  CHCONF_NUMERIC_scaleToUnit,
						  CHCONF_STRING_scaleUnit,
						  0);

	
}


void CHANNEL_displayMaxAcquisitionTime (void)
{
	char help[50];
	__int64 seconds;
	unsigned sec, min, hour;
	int nAverage, nPoints, timeInterval;
	
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_interval, &timeInterval);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_averages, &nAverage);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_nPoints, &nPoints);
	
	seconds =  (timeInterval * nAverage);
	seconds *= nPoints;
	sec = (seconds / 1000);
	min = sec / 60;
	hour = (min / 60) % 60;
	min = min % 60;
	sec = sec % 60;
	
	if (hour > 0) sprintf (help, "%d h %02d min", hour, min);
	else if (min > 0) sprintf (help, "%d min %02d s", min, sec);
	else sprintf (help, "%d s", sec);
	
	SetCtrlVal (panelChannelConf, CHCONF_STRING_time, help);
}



void CHANNEL_config_setValues (t_channel *ch, int nr)
{
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_channel, nr+1);
	SetCtrlVal (panelChannelConf, CHCONF_STRING_name, ch->name);
	SetCtrlVal (panelChannelConf, CHCONF_STRING_module, ch->module);
	SetCtrlVal (panelChannelConf, CHCONF_STRING_channel, ch->channel);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_interval, ch->timeInterval);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_averages, ch->nAverage);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_nPoints, ch->nPoints);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_interval, ch->timeInterval);
	SetCtrlVal (panelChannelConf, CHCONF_RINGSLIDE_mode, ch->detectionMode);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdUp, ch->thresholdFixedUpper);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdLow, ch->thresholdFixedLower);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thrAverages, ch->thresholdDiffAverages);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdDiff, ch->thresholdDiff);
	
	
	SetCtrlVal (panelChannelConf, CHCONF_BTN_watchdogCh, !ch->monitorChannel);
	SetCtrlVal (panelChannelConf, CHCONF_BTN_MonitorCh, ch->monitorChannel);

//	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_lastMinutes, ch->lastMinutes);
	SetCtrlVal (panelChannelConf, CHCONF_STRING_saveFilename, ch->saveFilename);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_saveMinutes, ch->saveMinutes);
	SetCtrlVal (panelChannelConf, CHCONF_BTN_changeUnits, ch->changeUnits);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_scaleVolts, ch->scaleVolts);
	SetCtrlVal (panelChannelConf, CHCONF_NUMERIC_scaleToUnit, ch->scaleToUnit);
	SetCtrlVal (panelChannelConf, CHCONF_STRING_scaleUnit, ch->scaleUnit);
	CHANNEL_displayMaxAcquisitionTime ();

	
	CHANNEL_setValues_dimItems ();
}




void CHANNEL_config_getValues (t_channel *ch, int *nameChanged, int *reopen,
							   int *changeWindow)
{
	t_channel old;
	
	old = *ch;
	GetCtrlVal (panelChannelConf, CHCONF_STRING_name, ch->name);
	GetCtrlVal (panelChannelConf, CHCONF_STRING_module, ch->module);
	GetCtrlVal (panelChannelConf, CHCONF_STRING_channel, ch->channel);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_interval, &ch->timeInterval);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_averages, &ch->nAverage);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_nPoints, &ch->nPoints);
	  		
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_interval, &ch->timeInterval);
	GetCtrlVal (panelChannelConf, CHCONF_RINGSLIDE_mode, &ch->detectionMode);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdUp, &ch->thresholdFixedUpper);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdLow, &ch->thresholdFixedLower);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thrAverages, &ch->thresholdDiffAverages);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_thresholdDiff, &ch->thresholdDiff);

	GetCtrlVal (panelChannelConf, CHCONF_BTN_MonitorCh, &ch->monitorChannel);
//	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_lastMinutes, &ch->lastMinutes);
	GetCtrlVal (panelChannelConf, CHCONF_STRING_saveFilename, ch->saveFilename);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_saveMinutes, &ch->saveMinutes);
	
	GetCtrlVal (panelChannelConf, CHCONF_BTN_changeUnits, &ch->changeUnits);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_scaleVolts, &ch->scaleVolts);
	GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_scaleToUnit, &ch->scaleToUnit);
	GetCtrlVal (panelChannelConf, CHCONF_STRING_scaleUnit, ch->scaleUnit);


	*nameChanged = (strcmp (ch->module, old.module) != 0)
	  		|| (strcmp (ch->channel, old.channel) != 0);
	*reopen =  *nameChanged  
	  		|| (ch->timeInterval != old.timeInterval)
	  		|| (ch->nPoints  != old.nPoints)
	  		|| (ch->detectionMode != old.detectionMode);
	*changeWindow = (ch->monitorChannel != old.monitorChannel);
}



			

int CVICALLBACK CHANNEL_editParams_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			CHANNEL_setValues_dimItems ();
			CHANNEL_displayMaxAcquisitionTime ();
			break;
	}
	return 0;
}


void CHANNEL_getNrFromPanel (int panel, t_channel **channel, int *nr)
{
	int i;
	
	for (i = 0; i < config->nChannels; i++) {
		if (panel == config->channels[i]->panel) {
			if (channel != NULL) *channel = config->channels[i];
			if (nr != NULL) *nr = i;
			return;
		}
	}
	if (channel != NULL) *channel = NULL;
	if (nr != NULL) *nr = -1;
}
		
		
int CVICALLBACK CHANNEL_config_channelMode_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			if (control == CHCONF_BTN_watchdogCh) SetCtrlVal (panel, CHCONF_BTN_MonitorCh, 0);
			if (control == CHCONF_BTN_MonitorCh) SetCtrlVal (panel, CHCONF_BTN_watchdogCh, 0);
			SetCtrlVal (panel, control, 1);
			CHANNEL_setValues_dimItems ();
			break;
	}
	return 0;
}

		
		
	
void WATCHDOG_checkUnlockedState (t_config *c)
{
	int alert, i;
	t_channel *ch;
	
	
	alert = 0;
	for (i = 0; i < config->nChannels; i++) {
		ch = config->channels[i];
		if (ch->active && ch->watch) alert = max (alert, ch->alarmState);
	}
	
	if ((alert == STATUS_OK) && (watchdogStatus == STATUS_PREALERT)) {
		watchdogStatus = STATUS_OK;
		FP_setUnlocked (c);
		WATCHDOG_updateDisplay (c);
	}
	
	// check if global status "unlocked" can be changed
	if ((alert == STATUS_OK) && (watchdogStatus == STATUS_UNLOCKED)) {
		// no received handshake activated or handshake received;
		if (statusReceived) {
			watchdogStatus = STATUS_OK;
			FP_setUnlocked (c);
			#ifdef _CVI_DEBUG_
			tprintf ("RELOCKED.\n");
			printState ();
			#endif
			WATCHDOG_updateDisplay (c);
		}
	}
	ALERT_displayPanels (watchdogStatus);
}
	




int CVICALLBACK CANNEL_reset_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_channel *ch;
	int nr;
	
	switch (event) {
		case EVENT_COMMIT:
			CHANNEL_getNrFromPanel (panel, &ch, &nr);
			if (nr < 0) return 0;
			ch->nAveragedValues = 0;
			ch->nValuesRead     = -1;
			ch->alarmState      = STATUS_OK;
			ch->valuesToGo      = -1;
			ch->preAlertValuesToDo = 0;
			CHANNEL_updateGraph (ch);
			WATCHDOG_checkUnlockedState (config);
			break;
	}
	return 0;
}


int CVICALLBACK CHANNEL_configure_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_channel *channel;
	int nr; 

	switch (event) {
		case EVENT_COMMIT:
			CHANNEL_getNrFromPanel (panel, &channel, &nr);
			if (nr < 0) return 0;
			CHANNEL_config_initPanel ();
			CHANNEL_config_setValues (channel, nr);
			InstallPopup (panelChannelConf);
			break;
	}
	return 0;
}


int CVICALLBACK CHANNEL_config_done_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_channel *channel;
	t_config *newConfig;
	int nr;
	int reOpen, nameChanged, changeWindow;
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panelChannelConf, CHCONF_NUMERIC_channel, &nr);
			nr--;
			newConfig = (t_config *) callbackData;
			channel = newConfig->channels[nr];	
			CHANNEL_config_getValues (channel, &nameChanged, &reOpen, &changeWindow);
			if (newConfig == config) {
				if (changeWindow) {
					WATCHDOG_discardAllPanels (newConfig);
					WATCHDOG_initAllPanels (newConfig);
				}
				if (reOpen) {
					CHANNEL_close (channel, nameChanged);
					CHANNEL_allocateMemory (channel);
				//	c->active = 0;
					CHANNEL_open (config, channel);
				}
			//	DisplayPanel (panelMain);
				CHANNEL_displayValues (channel);
				CONFIG_save (config);
			}
			else {
				CONFIG_displayChannelsInTree (newConfig, panelConfig, CONFIG_TREE_channels);
			}
			RemovePopup (0);
			break;
	}
	return 0;
}


int CVICALLBACK CHANNEL_config_abort_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}

// ==========================================================
//
//       uir-control + callbacks: CONFIG
//
// ==========================================================




void CHANNEL_config_initPanel (void) 
{
	if (panelChannelConf == -1) {
		panelChannelConf = LoadPanel (0, WATCHDOG_UIR, CHCONF);
		SetCtrlAttribute (panelChannelConf, CHCONF_STRING_name,
						  ATTR_MAX_ENTRY_CHARS, MAX_CHANNELNAME_LEN-1);
		SetCtrlAttribute (panelChannelConf, CHCONF_STRING_module,
						  ATTR_MAX_ENTRY_CHARS, MAX_MODULENAME_LEN-1);
		SetCtrlAttribute (panelChannelConf, CHCONF_STRING_channel,
						  ATTR_MAX_ENTRY_CHARS, MAX_CHANNELNAME_LEN-1);
		SetCtrlAttribute (panelChannelConf, CHCONF_NUMERIC_nPoints,
						  ATTR_MIN_VALUE, MIN_NPOINTS);
	}
	SetCtrlAttribute (panelChannelConf, CHCONF_COMMANDBUTTON_done,
					  ATTR_CALLBACK_DATA, (void*) config);
	SetCtrlAttribute (panelChannelConf, CHCONF_NUMERIC_nPoints,
					  ATTR_MIN_VALUE,  MIN_NPOINTS);
//	SetCtrlAttribute (panelChannelConf, NUMERIC_thrAverages,
//					  ATTR_MAX_VALUE, MIN_NPOINTS);
}


void CONFIG_displayChannelsInTree (t_config *c, int panel, int ctrl)
{
	int i;
	t_channel *ch;
	int item;
	
	DeleteListItem (panel, ctrl, 0, -1);
	for (i = 0; i < c->nChannels; i++) {
		ch = c->channels[i];
		item = InsertTreeItem (panel, ctrl, VAL_SIBLING,
							   i == 0 ? 0 : i-1, VAL_NEXT, ch->name,
							   "", NULL, i);
		SetTreeCellAttribute (panel, ctrl, item, 1, ATTR_LABEL_TEXT,
							  ch->module);
		SetTreeCellAttribute (panel, ctrl, item, 2, ATTR_LABEL_TEXT,
							  ch->channel);
		SetTreeItemAttribute (panel, ctrl, item, ATTR_MARK_STATE,
							  ch->active ? VAL_MARK_ON : VAL_MARK_OFF);
	}
	
}

void CONFIG_setValues_dimItems (void)
{
	int active;
	
	GetCtrlVal (panelConfig, CONFIG_RADIOBUTTON_beeper, &active);
	SetCtrlAttribute (panelConfig, CONFIG_STRING_moduleBeeper, ATTR_DIMMED, !active);
	SetCtrlAttribute (panelConfig, CONFIG_STRING_chBeeper, ATTR_DIMMED, !active);
}



void CONFIG_setValues (t_config *c)
{
	SetCtrlVal (panelConfig, CONFIG_STRING_iak, c->filenameIAK);
	SetCtrlVal (panelConfig, CONFIG_STRING_resource, c->FP_resourceName);
	SetCtrlVal (panelConfig, CONFIG_STRING_dataSocket, c->dataSocketServer);
	// nChannels
//	SetCtrlVal (panelConfig, CONFIG_NUMERIC_nChannels, c->nChannels);
	SetCtrlVal (panelConfig, CONFIG_NUMERIC_channelHeight, c->panelHeight);
	SetCtrlVal (panelConfig, CONFIG_NUMERIC_channelWidth, c->panelWidth);
	
	// "beeper"	
	SetCtrlVal (panelConfig, CONFIG_RADIOBUTTON_beeper,   c->beeperActive);
	SetCtrlVal (panelConfig, CONFIG_STRING_moduleBeeper,   c->moduleBeeper);
	SetCtrlVal (panelConfig, CONFIG_STRING_chBeeper,       c->chBeeper);
	SetCtrlVal (panelConfig, CONFIG_STRING_soundFilename, c->soundFilename);
	SetCtrlVal (panelConfig, CONFIG_RADIOBUTTON_sound, c->soundActive);
	CONFIG_displayChannelsInTree (c, panelConfig, CONFIG_TREE_channels);
	
	
	CONFIG_setValues_dimItems ();
}


void CONFIG_getChannelsFromTree (t_config *c)
{
	int nItems, i, nr;
	t_channel **tmpCh;

	GetNumListItems (panelConfig, CONFIG_TREE_channels, &nItems);
	if (nItems != c->nChannels) Breakpoint(); // this should not happen
	tmpCh = (t_channel **) malloc (nItems * sizeof (t_channel *));
	memcpy (tmpCh, c->channels, nItems * sizeof (t_channel*));

	for (i = 0; i < nItems; i++) {
		GetValueFromIndex (panelConfig, CONFIG_TREE_channels, i, &nr);
		c->channels[i] = tmpCh[nr];
		GetTreeItemAttribute (panelConfig, CONFIG_TREE_channels, i, ATTR_MARK_STATE, &c->channels[i]->active);
	}
	free (tmpCh);
}


void CONFIG_getValues (t_config *c)
{
	int nChannels = 0;
	
	GetCtrlVal (panelConfig, CONFIG_STRING_iak, c->filenameIAK);
	GetCtrlVal (panelConfig, CONFIG_STRING_resource, c->FP_resourceName);
	GetCtrlVal (panelConfig, CONFIG_STRING_dataSocket, c->dataSocketServer);
	
	// nChannels
//	GetCtrlVal (panelConfig, CONFIG_NUMERIC_nChannels, &nChannels);

	CONFIG_getChannelsFromTree (c);

	GetCtrlVal (panelConfig, CONFIG_NUMERIC_channelHeight, &c->panelHeight);
	GetCtrlVal (panelConfig, CONFIG_NUMERIC_channelWidth, &c->panelWidth);
	
	// "beeper"	
	GetCtrlVal (panelConfig, CONFIG_RADIOBUTTON_beeper, &c->beeperActive);
	GetCtrlVal (panelConfig, CONFIG_STRING_moduleBeeper, c->moduleBeeper);
	GetCtrlVal (panelConfig, CONFIG_STRING_chBeeper, c->chBeeper);
	GetCtrlVal (panelConfig, CONFIG_STRING_soundFilename, c->soundFilename);
	GetCtrlVal (panelConfig, CONFIG_RADIOBUTTON_sound, &c->soundActive);
}


int CVICALLBACK CONFIG_editActive_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			CONFIG_setValues_dimItems ();
			break;
	}
	return 0;
}



int CVICALLBACK CONFIG_callback_browseIAK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	
	switch (event) {
		case EVENT_COMMIT:
			if (FileSelectPopup (config->filenameIAK, "*.iak", "", "Select *.iak file",
								 VAL_LOAD_BUTTON, 0, 1, 1, 1, filename) == 1){
				SetCtrlVal (panelConfig, CONFIG_STRING_iak, filename);
			}
			break;
	}
	return 0;
}




int CVICALLBACK CONFIG_newChannel_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nChannels;
	t_channel *newCh;
	
	switch (event) {
		case EVENT_COMMIT:
			GetNumListItems (panelConfig, CONFIG_TREE_channels, &nChannels);
			CONFIG_allocate (newConfig, nChannels+1);
			newCh = newConfig->channels[nChannels];
			if (nChannels > 0) CHANNEL_copy (newCh, newConfig->channels[nChannels-1]);
			strcpy (newCh->name, "new");
			CONFIG_displayChannelsInTree (newConfig, panelConfig, CONFIG_TREE_channels);
			break;
	}
	return 0;
}

int CVICALLBACK CONFIG_deleteChannel_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nChannels;
	int i, nr;
	t_channel *ch;
	
	switch (event) {
		case EVENT_COMMIT:
			GetNumListItems (panelConfig, CONFIG_TREE_channels, &nChannels); 
			if (nChannels <= 1) return 0;
			GetCtrlVal (panelConfig, CONFIG_TREE_channels, &nr);
			if ((nr < 0) || (nr > newConfig->nChannels)) Breakpoint();
			ch = newConfig->channels[nr];
			if (ConfirmPopupf ("Delete channel", "Do you really want to delete channel '%s'?", ch->name)) {
				// delete channel
				for (i = nr+1; i < nChannels; i++) {
					newConfig->channels[i-1] = newConfig->channels[i];
				}
				CHANNEL_free (newConfig->channels[nChannels-1]);
				newConfig->channels[nChannels-1] = NULL;
				newConfig->nChannels --;
				CONFIG_displayChannelsInTree (newConfig, panelConfig, CONFIG_TREE_channels);
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK CONFIG_configureChannel_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nr;
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panelConfig, CONFIG_TREE_channels, &nr);
			if ((nr < 0) || (nr > newConfig->nChannels)) Breakpoint();
			CHANNEL_config_initPanel ();
			CHANNEL_config_setValues (newConfig->channels[nr], nr);
			SetCtrlAttribute (panelChannelConf, CHCONF_COMMANDBUTTON_done,
							  ATTR_CALLBACK_DATA, (void*) newConfig);
			InstallPopup (panelChannelConf);
			break;
	}
	return 0;
}








int CVICALLBACK CONFIG_tree_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, nItems, level;
	int lastParent = 0;
	int selected = 0;
	int state;
	
//	DebugPrintf ("%s (WAVEFORMS_editPoints)\n", eventStr(event, eventData1, eventData2));
	
	switch (event) {
		case EVENT_COMMIT:
		case EVENT_DROPPED:
		case EVENT_MARK_STATE_CHANGE:		
			GetNumListItems (panel,control, &nItems);
			for (i = 0; i < nItems; i++) {
				GetTreeItemLevel (panel,control, i, &level);
				if (level == 0) lastParent = i;
				else MoveTreeItem (panel,control, i, VAL_SIBLING, lastParent, VAL_NEXT);
				// check if one selected
				GetTreeItemAttribute (panel,control, i,
									  ATTR_MARK_STATE, &state);
				if (state == VAL_MARK_ON) selected = 1;
			}
			SetCtrlAttribute (panelConfig, CONFIG_COMMANDBUTTON_done, ATTR_DIMMED, !selected);
			SetCtrlAttribute (panelConfig, CONFIG_COMMANDBUTTON_apply, ATTR_DIMMED, !selected);
			break;
	}
	return 0;
}


int CVICALLBACK CHANNEL_watch_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nr;
	t_channel *channel;
	
	switch (event) {
		case EVENT_COMMIT:
			CHANNEL_getNrFromPanel (panel, &channel, &nr);
			if (nr < 0) return 0;
			GetCtrlVal (panel, CHANNEL_RADIOBUTTON_watch, &channel->watch);
			if (!channel->watch) {
				channel->nAveragedValues = 0;
				channel->nValuesRead = 0;
			}
			CONFIG_save (config);
			break;
	}
	return 0;
}



int CVICALLBACK WATCHDOG_panel_CB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	int i, nActive;
	int height, width;
	
	switch (event) {
		case EVENT_GOT_FOCUS:
			break;
		case EVENT_LOST_FOCUS:
			break;
		case EVENT_CLOSE:
			CONFIG_save (config);
			if (ConfirmPopup ("Quit Program", "Do you really want to exit Watchdog?")) {
				WATCHDOG_quit();
			}
			break;
		case EVENT_PANEL_SIZE:
			GetPanelAttribute (panelMain, ATTR_WIDTH, &width);
			GetPanelAttribute (panelMain, ATTR_HEIGHT, &height);
			nActive = 0;
			for (i = 0; i < config->nChannels; i++) {
				if (config->channels[i]->active && !config->channels[i]->monitorChannel) nActive ++;
			}
			if (nActive == 0) return 0;
			config->panelHeight = (height - FIRST_PANEL_TOP - FIRST_PANEL_LEFT) / nActive;
			config->panelWidth = (width - 2 * FIRST_PANEL_LEFT);
			WATCHDOG_initAllPanels (config);
			CONFIG_save (config);
	}
	return 0;
}

int CVICALLBACK WATCHDOG_beep_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panel, control, &config->beep);
			FP_setUnlocked (config);
			break;
	}
	return 0;
}




void CVICALLBACK DS_dataChanged (DSHandle dsHandle, int event, void *callbackData)
{
    HRESULT hr = S_OK;
    char message[1000];
//    int error;

	static int dataSockedConnected;
	
	switch (event) {
        case DS_EVENT_STATUSUPDATED: /* on status updated*/
            hr = DS_GetLastMessage (dsHandle, message, 1000);
            strcat (message, "\n");
            SetCtrlVal (panelMain, MAIN_TEXTBOX_status, message);
//			InsertTextBoxLine (panelMain, MAIN_TEXTBOX_status, -1, message);
//			SetCtrlVal (panelMain, MAIN_TEXTBOX_status, message);
			dataSockedConnected = (strstr (message, "Active: Connected") != 0);
			DS_lastError = (strstr (message, "Error:") != 0);
			SetCtrlAttribute (panelMain, MAIN_TEXTBOX_status,
							  ATTR_TEXT_COLOR, DS_lastError  ? VAL_RED: VAL_BLACK);
			break;
	}
    return;
}


void printState (void)
{
	int i;
	
	#ifdef _CVI_DEBUG_
	i = statusReceived;
	tprintf ("%s: status=%d, received=%d\n", timeStr(), watchdogStatus, i);
	#endif
}


void CVICALLBACK DS_received (DSHandle dsHandle, int event, void *callbackData)
{
    HRESULT hr = S_OK;
    char message[1000];
	
	switch (event) {
        case DS_EVENT_STATUSUPDATED: /* on status updated*/
            hr = DS_GetLastMessage (dsHandle, message, 1000);
//			SetCtrlVal (panelMain, MAIN_STRING_status, message);
			break;
		case DS_EVENT_DATAUPDATED:
			hr = DS_GetDataValue (DShandle_received, CAVT_LONG,
								  &statusReceived, 0, 0, 0);
			SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
			printState ();
			WATCHDOG_checkUnlockedState (config);
			break;
			
	}
    return;
}


int WATCHDOG_disconnectFromDataSocketServer (void)
{
	if (DShandle_status) {
		tprintf ("Disconnecting from DataSocket server...  ");
        DS_DiscardObjHandle(DShandle_status); 
        DShandle_status = 0;
        DS_DiscardObjHandle(DShandle_received); 
        DShandle_received = 0;
        tprintf ("Ok.\n");
    }
    return 0;
}



int WATCHDOG_connectToDataSocketServer (const char *serverName)
{
	int error;
	char URL[500];

	WATCHDOG_disconnectFromDataSocketServer ();

	DeleteTextBoxLines (panelMain, MAIN_TEXTBOX_status, 0, -1);
	tprintf ("Connecting to DataSocket server \"%s\"...  ", serverName);
	
	
	sprintf(URL, DS_WATCHDOG_PATH, serverName, DS_VAR_WD_STATUS);
//	if(DS_Open (URL, DSConst_Write, DSCounterUpdated_CB, "browsing",
//						&dsCounterUpdatedHandle)<0){
	error = DS_OpenEx (URL, DSConst_WriteAutoUpdate,
					   DS_dataChanged, NULL, DSConst_EventModel,
					   DSConst_Asynchronous, &DShandle_status);
	
	if (error >= 0) {
		sprintf(URL, DS_WATCHDOG_PATH, serverName, DS_VAR_WD_RECEIVED);
		error = DS_OpenEx (URL, DSConst_ReadAutoUpdate, DS_received,
						   NULL, DSConst_EventModel,
						   DSConst_Asynchronous, &DShandle_received);
	}
	if (error >= 0) {
	    DS_setStatus ();  
    }
	
	if (error < 0) tprintf ("No connection.\n");
	else tprintf ("Ok.\n");
//	if(DS_Open (serverName, DSConst_Write, DSCounterUpdated_CB,
//				"browsing", &dsCounterUpdatedHandle)<0){

	return 0;
}

int CVICALLBACK WATCHDOG_connectToServerCB_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			WATCHDOG_connectToDataSocketServer (config->dataSocketServer);

			break;
	}
	return 0;
}

int CVICALLBACK WATCHDOG_LED_received_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int state;
	
	switch (event) {
		case EVENT_COMMIT:

			break;
		case EVENT_LEFT_CLICK:
			GetCtrlVal (panelMain, MAIN_LED_received, &state);
			if (state == 1) {
				statusReceived = 0;
			}
			else {
				statusReceived = 1;
				printState ();
				WATCHDOG_checkUnlockedState (config);
				statusReceived = 0;
			}
			SetCtrlVal (panelMain, MAIN_LED_received, statusReceived);
			break;
			
	}
	return 0;
}



int CVICALLBACK CHANNEL_monitor_panelCallback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	t_channel *ch;
	
	switch (event) {
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		case EVENT_PANEL_SIZE:
		case EVENT_PANEL_MOVE:
			CHANNEL_getNrFromPanel (panel, &ch, NULL);
			getPanelBounds (panel, &ch->window);
			CONFIG_save (config);
			break;
	}
	return 0;
}

int CVICALLBACK CHANNEL_quit_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_channel *ch;

	switch (event) {
		case EVENT_COMMIT:
			CHANNEL_getNrFromPanel (panel, &ch, NULL);
			if (ch == NULL) return 0;
			ch->watch = 0;
			ch->nAveragedValues = 0;
			ch->nValuesRead = 0;
			HidePanel (ch->panel);
			CONFIG_save (config);
			break;
	}
	return 0;
}




int CVICALLBACK CONFIG_playSound_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, CONFIG_STRING_soundFilename, filename);
			CVI_PlaySound (filename, 0);
			CVI_PlaySound (filename, 0);
			CVI_PlaySound (filename, 0);
			CVI_StopCurrentSound ();
			break;
		}
	return 0;
}


int CVICALLBACK CONFIG_browse_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, CONFIG_STRING_soundFilename, filename);
			if (FileSelectPopup ("", "*.wav", "*.wav", "Select alert sound",
								 VAL_LOAD_BUTTON, 0, 1, 1, 0, filename) != 
				VAL_EXISTING_FILE_SELECTED) return 0;
			SetCtrlVal (panel, CONFIG_STRING_soundFilename, filename);
			break;
		}
	return 0;
}


void ALERT_displayPanels (int status)
{
	int i;
	static int lastStatus = STATUS_OK;
	
	if (status == lastStatus) return;
	lastStatus = status;
	
	for (i = 0; i < N_SCREENS; i++) {
		if (panelAlert[i] > 0) {
			SetCtrlAttribute (panelAlert[i], ALERT_TEXTMSG,
							  ATTR_TEXT_COLOR, 
							  status == STATUS_PREALERT ? COLOR_PREALERT : VAL_RED);
			SetPanelAttribute (panelAlert[i], ATTR_VISIBLE, status != STATUS_OK);
		}
	}
	
	
}


int CVICALLBACK TIMER_sound_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_TIMER_TICK:
		    if (!config->beep) return 0;
		    if (watchdogStatus != STATUS_UNLOCKED) return 0;
			if (config->soundActive) {
				if (!FileExists (config->soundFilename, 0)) return 0;
				if (nPlaySound <= 0) return 0;
				CVI_PlaySound (config->soundFilename, 0);
				CVI_PlaySound (config->soundFilename, 0);
				CVI_PlaySound (config->soundFilename, 0);
				CVI_StopCurrentSound ();
				nPlaySound --;
			}
			break;
		}
	return 0;
}

int CVICALLBACK ALERT_Panel_CB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
//			ALERT_Panel_CB (panel, EVENT_COLLAPSE, callbackData, eventData1,eventData2);
			break;
		}
	return 0;
}

int CVICALLBACK TIMER_checkConnection_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_TIMER_TICK:
			if (DS_lastError != 0) {
				WATCHDOG_disconnectFromDataSocketServer ();
//				pprintf (panelMain, MAIN_TEXTBOX_status, "Trying to establish connection...");
				WATCHDOG_connectToDataSocketServer (config->dataSocketServer);
			}
			break;
		}
	return 0;
}
