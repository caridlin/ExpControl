
#include <utility.h>
#include "INCLUDES_CONTROL.h"    
#include <userint.h>
//#include <dataacq.h>	  
#include "inifile.h" 

#include "tools.h"
#include "CONTROL_GUI_Main.h"
#include "UIR_ExperimentControl.h"
#include "CONTROL_HARDWARE_GPIB.h"
#include "CONTROL_HARDWARE_ADWIN.h"
#include "CONTROL_LOADSAVE.h"
#include "INTERFACE_TCP_IP.h"    
#include "CONTROL_GUI_ExtDev.h"

extern int panelConfiguration;
extern int panelGpib;
extern int panelMain;

int subPanelGeneral  = -1;
int subPanelBoardPar = -1;
//int subPanelBoardPar   = -1;
int subPanelGpib     = -1;
int subPanelTCP		 = -1;
int subPanelEXTDEV	 = -1;
//int subPanelSignals  = -1;

int panelDeviceSettings = -1;

int controlDACchannelNames[N_DAC_CHANNELS];
int controlDACchannelConst[N_DAC_CHANNELS];
int diff;


// maximum number of boards present
//#define maxBoards 7





const char CONF_GENERAL[]   = "GENERAL";
const char CONF_HARDWARE[]  = "HARDWARE";
const char CONF_GPIB[]      = "GPIB";
const char CONF_SEQUENCES[] = "SEQUENCES";



const char CONF_TAG_DEFAULT_PATH[] = "DefaultPath";

const char CONF_TAG_DISPLAYED_SEQUENCE_NO[] 	   	= "DisplayedSequenceNo";
const char CONF_TAG_idleSequence[] 	   				= "idleSequence";
const char CONF_TAG_NUMBER_OF_SEQS[]        	   	= "NumberOfSeqs";
const char CONF_TAG_NAME_SEQS[] 				   	= "Name%02d";
const char CONF_TAG_REPETITIONS_SEQS[] 		   		= "Repetitions%02d";
const char CONF_TAG_SHORTCUT_SEQS[] 			   	= "Shortcut%02d";

const char CONF_TAG_GPIB_NDEVICES[] 	   	   	   	= "nGpibDevices";
//char CONF_TAG_GPIB_BOARDINDEX[] 	   	   = "gpibBoardIndex";
//const char CONF_TAG_GPIB_DEVICETYPE[] 	   	   		= "%02d_deviceType";
//const char CONF_TAG_GPIB_DEVICENAME[] 	   	   		= "%02d_deviceName";
const char CONF_TAG_GPIB_DEVICE_STRIDN[]			= "%02d_strIDN";
const char CONF_TAG_GPIB_hasPulseOption[]	= "%02d_hasPulseOption";

const char CONF_TAG_CONFIG_enableTCP[] 		   		= "enableTCP";
const char CONF_TAG_CONFIG_enableGPIB[] 		    = "enableGPIB";
const char CONF_TAG_CONFIG_launchDataSocketServer[] = "launchDataSocketServer";
const char CONF_TAG_CONFIG_LeCroyDeltaT[] 	   		= "LeCroyDeltaT";
const char CONF_TAG_CONFIG_separateTextWindow[] 	= "separateTextWindow";
const char CONF_TAG_CONFIG_separateOutputWindow[] 	= "separateOutputWindow";
const char CONF_TAG_CONFIG_buttonHeight[] 	        = "buttonHeight";
const char CONF_TAG_CONFIG_buttonWidth[] 	        = "buttonWidth";
const char CONF_TAG_CONFIG_splitterHPos[] 	    	= "splitterVPos";
const char CONF_TAG_CONFIG_splitterVPos[] 	    	= "splitterHPos";
const char CONF_TAG_CONFIG_panelBoundsMain[] 	    = "panelBoundsMain";
const char CONF_TAG_CONFIG_panelBoundsMain2[] 	    = "panelBoundsMain2";
const char CONF_TAG_CONFIG_panelMain2IsZoomed[]		= "panelMain2IsZoomed";

const char CONF_TAG_CONFIG_panelPosSimulate[] 	    = "panelPosSimulate";
const char CONF_TAG_CONFIG_useTwoScreens[] 	    	= "useTwoScreens";


const char CONF_TAG_CONFIG_watchdogAutoRepeat[]		= "watchdogAutoRepeat";
const char CONF_TAG_CONFIG_watchdogActivate[]		= "watchdogActivate";
const char CONF_TAG_CONFIG_suppressProtocol[]       = "suppressProtocol";

const char CONF_TAG_CONFIG_autoSaveSequencesAfterEachStart[]= "autoSaveSequencesAfterEachStart";
const char CONF_TAG_CONFIG_autoSaveSequencesPath[]			= "autoSaveSequencesPath";
const char CONF_TAG_CONFIG_createFilenameForImages[] 		= "createFilenameForImages";
const char CONF_TAG_CONFIG_imageFilenameExtensionPath[] 	= "imageFilenameExtensionPath";

// ### idle sequence and auto focus
const char CONF_TAG_CONFIG_idleSequenceDataPath[] 	= "idleSequenceDataPath";
const char CONF_TAG_CONFIG_pifocFocusVoltageLogFilePath[] 	= "pifocFocusVoltageLogFilePath";
const char CONF_TAG_CONFIG_pifocVoltageChannel[]    = "pifocVoltageChannel";
const char CONF_TAG_CONFIG_lastPifocFocusVoltage[]  = "lastPifocFocusVoltage";

// ### DMD
const char CONF_TAG_CONFIG_pathMagickConvert[]      = "pathMagickConvert";




const char CONF_TAG_CONFIG_hardwareType[] 			= "hardwareType";
const char CONF_TAG_CONFIG_invertDigitalOutput[]	= "invertDigitalOutput";

const char CONF_TAG_CONFIG_analogBoardID[]			= "analogBoardID";
const char CONF_TAG_CONFIG_digitalBoardID[]			= "digitalBoardID";

const char CONF_TAG_GPIB_externalSCLK[] 	   	   	= "%02d_externalSCLK";
const char CONF_TAG_GPIB_frequencySCLK[] 	   	   	= "%02d_frequencySCLK";
const char CONF_TAG_GPIB_divideFreq[] 	   	   		= "%02d_divideFreq_CHAN%d";
const char CONF_TAG_GPIB_triggerSlope[] 	   	   	= "%02d_triggerSlope";

//char CONF_TAG_GPIB_displayAsAnalogChannel[]= "%02d_displayAsAnalogChannel_CHAN%d";
const char CONF_TAG_GPIB_triggerSignal[] 		   	= "%02d_triggerSignal";
const char CONF_TAG_GPIB_maxVoltage[] 		   	    = "%02d_maxVoltage";
const char CONF_TAG_GPIB_load[]					    = "%02d_load";

int CONFIG_GPIB_deviceSettingsEditable (int device);



void Ini_PutGpibDeviceInfo (IniText ini, t_config *c, int deviceNr)
{
	t_gpibDevice *g;
	char h[50];
	//int i;

	g = GPIBDEVICE_ptrConfig (c, deviceNr);
	if (g == NULL) return;
	if (g->type == GPIBDEV_NONE) return;
//	sprintf (h, CONF_TAG_GPIB_DEVICETYPE, deviceNr);
//	Ini_PutInt (ini, CONF_GPIB, h, g->type);
/*	sprintf (h, CONF_TAG_GPIB_DEVICENAME, deviceNr);
	if (g->name != NULL) 
		Ini_PutString (ini, CONF_GPIB, h, g->name);
	else 
	    Ini_PutString (ini, CONF_GPIB, h, "");
*/
	sprintf (h, CONF_TAG_GPIB_DEVICE_STRIDN, deviceNr);
	if (g->strIDN[0] != 0) Ini_PutString (ini, CONF_GPIB, h, g->strIDN);
	sprintf (h, CONF_TAG_GPIB_hasPulseOption, deviceNr);
	Ini_PutInt0 (ini, CONF_GPIB, h, g->hasPulseOption);
	
	    
	if ((g->commandType1 == GPIB_COMMANDTYPE_ARBWFM) || (g->commandType2 == GPIB_COMMANDTYPE_ARBWFM)) {
//		sprintf (h, CONF_TAG_GPIB_externalSCLK, deviceNr);
//		Ini_PutInt (ini, CONF_GPIB, h, g->externalSCLK);
//		sprintf (h, CONF_TAG_GPIB_frequencySCLK, deviceNr);
//		Ini_PutDouble (ini, CONF_GPIB, h, g->frequencySCLK);
//		for (i = 0; i < N_CHANNELS; i++) {
//			sprintf (h, CONF_TAG_GPIB_divideFreq, deviceNr, i+1);
//			Ini_PutInt (ini, CONF_GPIB, h, g->divideFreq[i]);
//					sprintf (h, CONF_TAG_GPIB_displayAsAnalogChannel, deviceNr, i+1);
//					Ini_PutInt (ini, CONF_GPIB, h, g->displayAsAnalogChannel[i]);
//		}
		sprintf (h, CONF_TAG_GPIB_triggerSlope, deviceNr);
		Ini_PutInt (ini, CONF_GPIB, h, g->triggerSlope);
		sprintf (h, CONF_TAG_GPIB_triggerSignal, deviceNr);
		Ini_PutInt (ini, CONF_GPIB, h, g->triggerSignal);
		sprintf (h, CONF_TAG_GPIB_maxVoltage, deviceNr);
		Ini_PutDouble (ini, CONF_GPIB, h, g->maxVoltage);
		sprintf (h, CONF_TAG_GPIB_load, deviceNr);
		Ini_PutInt (ini, CONF_GPIB, h, g->load);
	}
}


void Ini_GetGpibDeviceInfo (IniText ini, t_config *c, int deviceNr)
{
	t_gpibDevice *g;
	//t_gpibDevice *devPtr = NULL;
	char h[50];
	//int i;

	g = GPIBDEVICE_new (c);
//	sprintf (h, CONF_TAG_GPIB_DEVICETYPE, deviceNr);
//	Ini_GetInt (ini, CONF_GPIB, h, &g->type);
//	sprintf (h, CONF_TAG_GPIB_DEVICENAME, deviceNr);
//	Ini_GetStringIntoBuffer (ini, CONF_GPIB, h, g->name, MAX_GPIBDEVICENAME_LEN);
	
	g->strIDN[0] = 0;
	sprintf (h, CONF_TAG_GPIB_DEVICE_STRIDN, deviceNr);
	Ini_GetStringIntoBuffer (ini, CONF_GPIB, h, g->strIDN, MAX_GPIBDEVICENAME_LEN);
	GPIB_identifyDevice (g, g->strIDN);
/*	if ((g->type > 0) && (g->type <= GPIB_ListNumItems (GPIB_listOfAvailableDevices)) {
		ListGetItem (GPIB_listOfAvailableDevices, &devPtr, g->type);
		GPIBDEVICE_copySettings (g, devPtr);
	}
*/		
	sprintf (h, CONF_TAG_GPIB_hasPulseOption, deviceNr);
	Ini_GetInt (ini, CONF_GPIB, h, &g->hasPulseOption);
		
	
//	GPIBDEV_copyValues ();
	if ((g->commandType1 == GPIB_COMMANDTYPE_ARBWFM) || (g->commandType2 == GPIB_COMMANDTYPE_ARBWFM)) {
//		sprintf (h, CONF_TAG_GPIB_externalSCLK, deviceNr);
//		Ini_GetInt (ini, CONF_GPIB, h, &g->externalSCLK);

//		sprintf (h, CONF_TAG_GPIB_frequencySCLK, deviceNr);
//		Ini_GetDouble (ini, CONF_GPIB, h, &g->frequencySCLK);
		
		sprintf (h, CONF_TAG_GPIB_triggerSlope, deviceNr);
		Ini_GetInt (ini, CONF_GPIB, h, &g->triggerSlope);
		
		sprintf (h, CONF_TAG_GPIB_triggerSignal, deviceNr);
		Ini_GetInt (ini, CONF_GPIB, h, &g->triggerSignal);

		sprintf (h, CONF_TAG_GPIB_maxVoltage, deviceNr);
		Ini_GetDouble (ini, CONF_GPIB, h, &g->maxVoltage);

		sprintf (h, CONF_TAG_GPIB_load, deviceNr);
		Ini_GetInt (ini, CONF_GPIB, h, &g->load);
		
//		for (i = 0; i < N_CHANNELS; i++) {
//			sprintf (h, CONF_TAG_GPIB_divideFreq, deviceNr, i+1);
//			Ini_GetInt (ini, CONF_GPIB, h, &g->divideFreq[i]);
//				sprintf (h, CONF_TAG_GPIB_displayAsAnalogChannel, deviceNr, i+1);
//				Ini_GetInt (ini, CONF_GPIB, h, &g->displayAsAnalogChannel[i]);
//		}
	}
}



void Ini_PutConfig (IniText ini, t_config *c)
{
	char h[MAX_PATHNAME_LEN];
	t_sequence *s;
	int i;

	Ini_PutString (ini, CONF_GENERAL, CONF_TAG_DEFAULT_PATH, 
		c->defaultPath);

	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_enableTCP,
				c->enableTCP);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_enableGPIB,
				c->enableGPIB);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_launchDataSocketServer,
				c->launchDataSocketServer);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_buttonWidth,
				c->buttonWidth);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_buttonHeight,
				c->buttonHeight);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_splitterVPos,
				c->splitterVPos);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_splitterHPos,
				c->splitterHPos);

	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_watchdogActivate,
				 c->watchdogActivate);
	Ini_PutInt0 (ini, CONF_GENERAL, CONF_TAG_CONFIG_watchdogAutoRepeat,
				 c->watchdogAutoRepeat);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_suppressProtocol, 
				c->suppressProtocol);
				
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_hardwareType,
				 c->hardwareType);
	Ini_PutInt0 (ini, CONF_GENERAL, CONF_TAG_CONFIG_invertDigitalOutput,
				 c->invertDigitalOutput);

	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_autoSaveSequencesAfterEachStart, 
				c->autoSaveSequencesAfterEachStart);
	Ini_PutString (ini, CONF_GENERAL, CONF_TAG_CONFIG_autoSaveSequencesPath, 
				c->autoSaveSequencesPath);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_createFilenameForImages, 
				c->createFilenameForImages);
	Ini_PutString (ini, CONF_GENERAL, CONF_TAG_CONFIG_imageFilenameExtensionPath, 
				c->imageFilenameExtensionPath);
	Ini_PutRect (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelBoundsMain,
				c->panelBoundsMain);
	Ini_PutRect (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelBoundsMain2,
				c->panelBoundsMain2);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelMain2IsZoomed,
				c->panelMain2IsZoomed);
	
	// idle seq and auto focus
	Ini_PutString (ini, CONF_GENERAL, CONF_TAG_CONFIG_idleSequenceDataPath, 
				c->idleSequenceDataPath);
	Ini_PutString (ini, CONF_GENERAL, CONF_TAG_CONFIG_pifocFocusVoltageLogFilePath, 
				c->pifocFocusVoltageLogFilePath);
	Ini_PutInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_pifocVoltageChannel, 
				c->pifocVoltageChannel);
	Ini_PutDouble (ini, CONF_GENERAL, CONF_TAG_CONFIG_lastPifocFocusVoltage, 
				c->lastPifocFocusVoltage);
	
	// DMD
	Ini_PutString  (ini, CONF_GENERAL, CONF_TAG_CONFIG_pathMagickConvert, 
				c->pathMagickConvert);
	
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN && config->loadAdwinBoardsFromConfig) {  
		Ini_PutDataArray(ini, CONF_GENERAL, CONF_TAG_CONFIG_analogBoardID, c->analogBoardID, c->nAnalogBoards,VAL_INTEGER); 
		Ini_PutDataArray(ini, CONF_GENERAL, CONF_TAG_CONFIG_digitalBoardID, c->digitalBoardID, c->nDigitalBoards,VAL_INTEGER);
	}	

	Ini_PutPoint (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelPosSimulate,
				c->panelPosSimulate);
	
	for (i = 0; i < N_TABLE_CONFIGS; i++) {
		Ini_PutTableConfig (ini, &c->tableConfig[i], i);
	}
//	c->digitalDoubleBuffering = 0;
//	c->analogDoubleBuffering = 0;
	
	

	
	Ini_PutInt (ini, CONF_SEQUENCES,    CONF_TAG_DISPLAYED_SEQUENCE_NO,
				c->activeSequenceNr);
	Ini_PutInt (ini, CONF_SEQUENCES, CONF_TAG_idleSequence, c->idleSequenceNr);

//=======================================================================
//    write names, repetitions, shortcuts of sequences
//=======================================================================
	Ini_PutInt (ini, CONF_SEQUENCES, CONF_TAG_NUMBER_OF_SEQS, 
		ListNumItems(c->listOfSequences));
	
    for (i = 1; i <= ListNumItems (c->listOfSequences); i++) {
    	s = SEQUENCE_ptr(i);
		if (s != NULL) {
//			if (strcmp (s->filename, SEQUENCE_defaultFilename) != 0) {
				sprintf (h, CONF_TAG_NAME_SEQS, i);
				Ini_PutString (ini, CONF_SEQUENCES, h, s->filename);
				sprintf (h, CONF_TAG_REPETITIONS_SEQS, i);
				Ini_PutInt (ini, CONF_SEQUENCES, h, s->nRepetitions);
				if (s->shortcut > 0) {
					sprintf (h, CONF_TAG_SHORTCUT_SEQS,i);
					Ini_PutInt (ini, CONF_SEQUENCES, h, s->shortcut);
				}
//			}
		}
	}

	Ini_PutInt (ini, CONF_GPIB, CONF_TAG_GPIB_NDEVICES, 
		c->nGpibDevices);
	for (i = 1; i < c->nGpibDevices; i++) {
    	Ini_PutGpibDeviceInfo (ini, c, i);
	}

}

int CONFIG_write (t_config *c)
{
	IniText ini;

	if ((ini = Ini_New (0)) == 0) return -1;
	// save board configuration
	
	Ini_PutConfig (ini, c);

	Ini_WriteToRegistry_New (ini, ROOT_KEY, CONF_REG_SUBKEY);
	Ini_Dispose (ini);
	
	return 0;
}


int CONFIG_readSequences (IniText ini, t_config *c)
{
    int nSequences = 0;
	char seqName[MAX_PATHNAME_LEN];
 	char h[100];
 	t_sequence *new;
 	int err;
 	int i;

	Ini_GetStringCopy (ini, CONF_GENERAL, CONF_TAG_DEFAULT_PATH, 
		&c->defaultPath);
	Ini_GetInt (ini, CONF_SEQUENCES, CONF_TAG_NUMBER_OF_SEQS, 
		&nSequences);
    for (i = 1; i <= nSequences; i++) {
		sprintf (h, CONF_TAG_NAME_SEQS, i);
		if (Ini_GetStringIntoBuffer (ini, CONF_SEQUENCES, h, seqName, MAX_PATHNAME_LEN) > 0) {
			if (FileExists (seqName, 0)) {
		    	new = SEQUENCE_new ();
			    if ((err = SEQUENCE_load (seqName, new)) == 0) {
					sprintf (h, CONF_TAG_REPETITIONS_SEQS, i);
					Ini_GetInt (ini, CONF_SEQUENCES, h, &new->nRepetitions);
					sprintf (h, CONF_TAG_SHORTCUT_SEQS,i);
					Ini_GetInt (ini, CONF_SEQUENCES, h, &new->shortcut);
	        		SEQUENCE_appendToList (new, i == nSequences);
			    }
			    else {  // error occured
			        free (new);
			    }
			}
		}
	}

	Ini_GetInt (ini, CONF_SEQUENCES,  CONF_TAG_DISPLAYED_SEQUENCE_NO,
				&c->activeSequenceNr);
	if (c->activeSequenceNr > (int)ListNumItems (c->listOfSequences))
		c->activeSequenceNr = ListNumItems (c->listOfSequences);
	// rewrite config
	return 0;
}



int Ini_GetConfig (IniText ini, t_config *c)
{
	int i;
	int* tmp; 
	
	Ini_ReadFromRegistry (ini, ROOT_KEY, CONF_REG_SUBKEY);

	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_enableTCP,
				&c->enableTCP);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_enableGPIB,
				&c->enableGPIB);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_launchDataSocketServer,
				&c->launchDataSocketServer);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_buttonWidth,
				&c->buttonWidth);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_buttonHeight,
				&c->buttonHeight);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_splitterVPos,
				&c->splitterVPos);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_splitterHPos,
				&c->splitterHPos);
				
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_watchdogActivate,
				&c->watchdogActivate);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_watchdogAutoRepeat,
				&c->watchdogAutoRepeat);
	
	
	Ini_GetInt (ini, CONF_SEQUENCES, CONF_TAG_idleSequence, &c->idleSequenceNr);
	
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_hardwareType,
				&c->hardwareType);
	
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_invertDigitalOutput,
				&c->invertDigitalOutput);
	
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_suppressProtocol, 
				&c->suppressProtocol);
	
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_autoSaveSequencesAfterEachStart, 
				&c->autoSaveSequencesAfterEachStart);
	Ini_GetStringIntoBuffer (ini, CONF_GENERAL,
							 CONF_TAG_CONFIG_autoSaveSequencesPath,
							 c->autoSaveSequencesPath, MAX_PATHNAME_LEN);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_createFilenameForImages, 
				&c->createFilenameForImages);
	Ini_GetStringIntoBuffer  (ini, CONF_GENERAL, CONF_TAG_CONFIG_imageFilenameExtensionPath, 
				c->imageFilenameExtensionPath, MAX_PATHNAME_LEN);
	Ini_GetRect (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelBoundsMain,
				&c->panelBoundsMain);
//	if (c->panelBoundsMain.left > screenWidth()) c->panelBoundsMain.left  = 25;
	Ini_GetRect (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelBoundsMain2,
				&c->panelBoundsMain2);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelMain2IsZoomed,
				&c->panelMain2IsZoomed);
	Ini_GetPoint (ini, CONF_GENERAL, CONF_TAG_CONFIG_panelPosSimulate,
				&c->panelPosSimulate);
	
	// idle seq and auto focus
	Ini_GetStringIntoBuffer  (ini, CONF_GENERAL, CONF_TAG_CONFIG_idleSequenceDataPath, 
				c->idleSequenceDataPath, MAX_PATHNAME_LEN);
	Ini_GetStringIntoBuffer  (ini, CONF_GENERAL, CONF_TAG_CONFIG_pifocFocusVoltageLogFilePath, 
				c->pifocFocusVoltageLogFilePath, MAX_PATHNAME_LEN);
	Ini_GetInt (ini, CONF_GENERAL, CONF_TAG_CONFIG_pifocVoltageChannel, 
				&c->pifocVoltageChannel);
	Ini_GetDouble (ini, CONF_GENERAL, CONF_TAG_CONFIG_lastPifocFocusVoltage, 
			&c->lastPifocFocusVoltage);
	
	
	// DMD
	Ini_GetStringIntoBuffer  (ini, CONF_GENERAL, CONF_TAG_CONFIG_pathMagickConvert, 
				c->pathMagickConvert, MAX_PATHNAME_LEN);
	
							 
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN && config->loadAdwinBoardsFromConfig) {
		
		tmp = (int*)Ini_GetDataArray (ini, CONF_GENERAL, CONF_TAG_CONFIG_analogBoardID, 
					        &c->nAnalogBoards, VAL_INTEGER);
		if (tmp == NULL) {
			c->nAnalogBoards = 0;
		} else {
			memcpy (c->analogBoardID,tmp,c->nAnalogBoards*sizeof(int));
		}
	
		tmp = (int*)Ini_GetDataArray (ini, CONF_GENERAL, CONF_TAG_CONFIG_digitalBoardID, 
					        &c->nDigitalBoards, VAL_INTEGER);
		if (tmp == NULL) {
			c->nDigitalBoards = 0;
		} else {
			memcpy (c->digitalBoardID,tmp,c->nDigitalBoards*sizeof(int)); 
		}
	
		// debug:
		/*tprintf("number of analog boards: %d\n",c->nAnalogBoards);
		for (i=0; i < c->nAnalogBoards; i++){
			tprintf("analogBoard #%d: id=%d\n",i,c->analogBoardID[i]);	
		}
		tprintf("number of digital boards: %d\n",c->nDigitalBoards);
		for (i=0; i < c->nDigitalBoards; i++){
			tprintf("digitalBoard #%d: id=%d\n",i,c->digitalBoardID[i]);	
		}*/
		// end debug
	}
	
	for (i = 0; i < N_TABLE_CONFIGS; i++) {
		Ini_GetTableConfig (ini, &c->tableConfig[i], i);
	}

	
	return 0;				
}



int CONFIG_read (t_config *c, int readSequences)
{    
    int i;
	IniText ini;

	if ((ini = Ini_New (0)) == 0) return -1;
	
	Ini_GetConfig (ini, c);
	if (readSequences) CONFIG_readSequences (ini, c);

//=======================================================================
//    get GPIB devices
//=======================================================================
	GPIBDEVICE_deleteAll (c);
//	Ini_GetInt (ini, CONF_GPIB, CONF_TAG_GPIB_NDEVICES, 
//		&c->nGpibDevices);
	for (i = 1; i < c->nGpibDevices; i++) {
   		Ini_GetGpibDeviceInfo (ini, c, i);
	}
	Ini_Dispose (ini);

	if (readSequences) CONFIG_write (c);
	return 0;
}







/************************************************************************/
/*
/*    list of boards
/*
/************************************************************************/



// ---------------------------------------------
//   return NI-device no. for analog board 
// ---------------------------------------------
int deviceNrAO (int nr)
{
    if (nr < MAX_AO_DEVICES) return config->analogBoardID[nr];
	return -1;
}

// ---------------------------------------------
//   return NI-device no. for digital board 
// ---------------------------------------------
int deviceNrDIO (int nr)
{
    if (nr < MAX_DIO_DEVICES) return config->digitalBoardID[nr];
	return -1;
	
}




void CONFIG_initPanelGpib (void);



// ---------------------------------------------
//    add new device to board list
// ---------------------------------------------
void addBoard (int typeCode, int deviceNo)
{
	ListInsertItem (config->listOfBoards, &typeCode, END_OF_LIST);
//	switch (typeCode) {
//        case ID_PCI_DIO_32_HS: 
//        case ID_PCI_6534:
//			if (config->nDigitalBoards > MAX_DIO_DEVICES) {
//				tprintf("Maximum number of DIO_DEVICES reached.\n"); 
//				return;
//			}
//        	config->digitalBoardID[config->nDigitalBoards] = deviceNo; //FIXME TODO no overflow check!
//        	config->nDigitalBoards++;
//        	break;
//		case ID_PCI_6733: 
//		case ID_PCI_6713: 
//			if (config->nAnalogBoards > MAX_AO_DEVICES) {
//				tprintf("Maximum number of AO_DEVICES reached.\n"); 
//				return;
//			}
//        	config->analogBoardID[config->nAnalogBoards] = deviceNo; //FIXME TODO no overflow check! 
//        	config->nAnalogBoards ++;
//        	break;
//	}
}

// ---------------------------------------------
//    get name of board
// ---------------------------------------------
char *getBoardName (int deviceNo)
{
	int boardType;
	
	ListGetItem (config->listOfBoards, &boardType, deviceNo);
//		return get_NIDeviceName(boardType);
	return "nothing";
}


// ---------------------------------------------
//   reset and find all boards
// ---------------------------------------------

/*
void detectBoards (int outPanel, int outCtrl)
{
    int deviceNo = 1;
    unsigned long typeCode;
    short board;
    int status;
    char help[200];
    const char strDetecting[] = "Detecting DAQ-Boards...    ";
    
	SetCtrlVal (outPanel, outCtrl, strDetecting);

	CONFIG_initBoards (config);
    do {
  		status = Init_DA_Brds (deviceNo, &board);
        if (status == 0)
           status = Get_DAQ_Device_Info (deviceNo, ND_DEVICE_TYPE_CODE, &typeCode);
        if (status == 0) {
           sprintf (help, "%s   found %s", strDetecting, get_NIDeviceName(typeCode));
		   if (outPanel > 0) SetCtrlVal (outPanel, outCtrl, help);
           addBoard (typeCode, deviceNo);
  		}
//  		else if (outPanel > 0) appendText (outPanel, outCtrl, ": not found");
		deviceNo++;
    } while (status == 0);
	
	HARDWARE_resetStoredValues();
	
	
}
*/



//=======================================================================
//
//     determine minimum height of panel 
//
//=======================================================================
int CONFIG_panelMinimumHeight(void)
{
	return ctrlBottom (panelConfiguration, CONFIG_BUTTON_applyChanges);
}


//=======================================================================
//
//    resize panel "Configuration"
//
//=======================================================================
void CONFIG_resizePanel (void)
{
	
}

int CVICALLBACK CONFIG_panelChanged (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
		
     switch (event)
        {
        case EVENT_PANEL_SIZE:
		case EVENT_PANEL_MOVE:
            break;
		case EVENT_GOT_FOCUS:
			break;
        case EVENT_LOST_FOCUS:
            break;
        case EVENT_CLOSE:
			RemovePopup (1);    
			break;
   		case EVENT_KEYPRESS:
			break;
		case EVENT_FILESDROPPED:
			break;
		case EVENT_DISCARD:
			return 1;
			
        }
    return 0;
}


//=======================================================================
//
//    init panel "Configuration"
//
//=======================================================================
void CONFIG_initPanel (void)
{
	int i;
	int boardType;
	//int panelTCP;
	int newPanel = 0;
	
	if (panelConfiguration<= 0) panelConfiguration = LoadPanel (0, UIR_File, CONFIG); 

	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB,
							   CONFIG_TAB_INDEX_general, &subPanelGeneral);
	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB,
							   CONFIG_TAB_INDEX_boardPar, &subPanelBoardPar);
	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB,
							   CONFIG_TAB_INDEX_gpib, &subPanelGpib);
	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB,
							   CONFIG_TAB_INDEX_tcpip, &subPanelTCP);
	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB,
							   CONFIG_TAB_INDEX_extdev, &subPanelEXTDEV);
	
//	SetActiveTabPage (panelConfiguration, CONFIG_TAB, 1);
	

	TCP_initPanelServer (panelConfiguration);
	DeleteTabPage (panelConfiguration, CONFIG_TAB,CONFIG_TAB_INDEX_tcpip,1);
	InsertPanelAsTabPage (panelConfiguration, CONFIG_TAB, 
						  CONFIG_TAB_INDEX_tcpip, TCP_panelServer());
	GetPanelHandleFromTabPage (panelConfiguration, CONFIG_TAB, 
						  CONFIG_TAB_INDEX_tcpip, &newPanel);
 	TCP_setPanelServer(newPanel);
	

					  
	// -----------------------------------------
	//    init all board subpanels 
	// -----------------------------------------
	HARDWARE_ADWIN_initOutCtrl (subPanelBoardPar, BOARDPAR_TEXTBOX_adWinOutput);

	CONFIG_initPanelGeneralSettings ();
	CONFIG_initPanelGpib ();
	CONFIG_SIGNALS_initPanel ();
	
	// -----------------------------------------
	//    write data to board subpanels
	// -----------------------------------------
    for (i = 1; i <= ListNumItems(config->listOfBoards); i++) {
		ListGetItem (config->listOfBoards, &boardType, i);
//		switch (boardType) {
//            case ID_PCI_DIO_32_HS: 
//            case ID_PCI_6534:
//				SetCtrlVal (subPanelBoardPar, BOARDPAR_STRING_DIOdeviceName,
//							get_NIDeviceName(boardType));
//            	break;
//            case ID_PCI_6733: 
//            case ID_PCI_6713: 
//				SetCtrlVal (subPanelBoardPar, BOARDPAR_STRING_AOdeviceName,
//							get_NIDeviceName(boardType));
//				break;
//        }
    }

	if (config->digitalBoardID[0] != -1) 
		SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_DIOdeviceNo1, config->digitalBoardID[0]); 
	if (config->digitalBoardID[1] != -1) {
		SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_DIOdeviceNo2, config->digitalBoardID[1]); 
	}
	else {
		SetCtrlAttribute  (subPanelBoardPar, BOARDPAR_NUMERIC_DIOdeviceNo2, ATTR_VISIBLE, 0); 
	}

	
	if (config->analogBoardID[0] != -1) 
		SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_AOdeviceNo1, config->analogBoardID[0]); 
	if (config->analogBoardID[1] != -1) {
		SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_AOdeviceNo2, config->analogBoardID[1]); 
	}
	else {
		SetCtrlAttribute  (subPanelBoardPar, BOARDPAR_NUMERIC_AOdeviceNo2, ATTR_VISIBLE, 0); 
	}
	EXTDEV_TABLE_init(subPanelEXTDEV,CONFIGdev_TABLE_extdev);

}




/************************************************************************/
/*
/*    initialization of panels etc. 
/*
/************************************************************************/

void CONFIG_initPanelGeneralSettings (void)
{
	WATCHDOG_setStatusCtrl (subPanelGeneral, CONFIGgen_TEXTBOX_watchdogStat);
	
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_NUMERIC_NDIO_Channels, ATTR_MAX_VALUE, N_TOTAL_DIO_CHANNELS);
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_NUMERIC_NAO_Channels, ATTR_MAX_VALUE, N_DAC_CHANNELS);
	
}





//=======================================================================
//
//    init subpanel "GPIB"
//
//=======================================================================
void CONFIG_initPanelGpib (void)
{
	Rect bounds;
	// -----------------------------------------
	//   set buffer size 
	// -----------------------------------------
	GetCtrlBoundingRect (panelConfiguration, CONFIG_TAB, &bounds.top,
						 &bounds.left, &bounds.height, &bounds.width);
 	GPIB_fillDevicesToList (subPanelGpib,
					  CONFIGgpib_LISTBOX_gpibDevices, -1);
	SetCtrlAttribute (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices,
					  ATTR_HEIGHT, 
					  bounds.height - ctrlTop (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices) - 20);
	CONFIG_displayGpibSettings ();
    GPIB_displaySupportedDevices (subPanelGpib, CONFIGgpib_TEXTBOX_supported);

}









//=======================================================================
//
//    write default path
//
//=======================================================================
void setDefaultDir (const char *path)		
{
	config->defaultPath = strnewcopy (config->defaultPath, path);
}




/************************************************************************
 ************************************************************************
 *
 *          edit board settings  
 *
 ************************************************************************
 ************************************************************************/





/************************************************************************
 *
 * DIGITAL IO-  CONTROL-BOARD 
 *
 ************************************************************************/


// updates values for digital board settings
void CONFIG_displayBoardParameters  (t_sequence *seq)
{
	//unsigned long DIOtimebase;
	//unsigned long AOtimebase;
	double timebase_us;

	
	timebase_us = (1.0*seq->DIO_timebase_50ns) / (1.0 * VAL_us);
	SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_DIOtimebase,
               timebase_us);

	timebase_us = (1.0*seq->AO_timebase_50ns) / (1.0 * VAL_us);
	SetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_AOtimebase,
               timebase_us);
	
	SetCtrlVal (subPanelBoardPar, BOARDPAR_RADIOBUTTON_invert, config->invertDigitalOutput);
	

}
												 



void CONFIG_getBoardParameters (t_sequence *seq)
{
    
	double timebase_us;
	
	GetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_DIOtimebase,
               &timebase_us);
	seq->DIO_timebase_50ns = RoundRealToNearestInteger (timebase_us * VAL_us);

	GetCtrlVal (subPanelBoardPar, BOARDPAR_NUMERIC_AOtimebase,
               &timebase_us);
	seq->AO_timebase_50ns = RoundRealToNearestInteger (timebase_us * VAL_us);
	OUTPUTDATA_getTimebases (seq);
	
	
	GetCtrlVal (subPanelBoardPar, BOARDPAR_RADIOBUTTON_invert, &config->invertDigitalOutput);
}










void CONFIG_displayGpibSettings (void)
{
	int device;
	
	GPIB_fillDevicesToList (subPanelGpib,
					  CONFIGgpib_LISTBOX_gpibDevices, -1); 
	GetCtrlVal (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices, &device);
				
	SetCtrlAttribute (subPanelGpib, CONFIGgpib_BUTTON_Settings, ATTR_DIMMED, 
				!CONFIG_GPIB_deviceSettingsEditable(device));
}

void CONFIG_displayExternalDevices (void)
{
	t_sequence *seq;
	
	seq = activeSeq();
	EXTDEV_TABLE_displayAllExtDevs (subPanelEXTDEV,CONFIGdev_TABLE_extdev,seq, -1,1); 

}


void CONFIG_getGpibSettings (void)
{
}


void CONFIG_GENERAL_changeVisibility (void)
{
	int clicked;
	
	GetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_autosaveSeq,
				&clicked);
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_STRING_autosavePath, ATTR_DIMMED,
					  !clicked);
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_BUTTON_browseSeqPath, ATTR_DIMMED,
					  !clicked);
	
	if (clicked) GetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_createFilename, &clicked);
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_STRING_pathImageFile, ATTR_DIMMED,
					  !clicked);
	SetCtrlAttribute (subPanelGeneral, CONFIGgen_BUTTON_browseImagPath, ATTR_DIMMED,
					  !clicked);
	
	CONFIG_isOfTypeIdleChanged (subPanelGeneral, CONFIGgen_CHECKBOX_isIdleSeq, EVENT_VAL_CHANGED, 0, 0, 0);

	
}



void CONFIG_GENERAL_setValues (t_sequence *seq)
{
	char digitalBoardIDs[MAX_PATHNAME_LEN];
	char analogBoardIDs[MAX_PATHNAME_LEN];
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_NDIO_Channels, seq->maxDigitalChannel);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_NAO_Channels, seq->maxAnalogChannels);

	
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_BTN_enableTCP,
						config->enableTCP);
	SetCtrlVal (subPanelGeneral, CONFIGgen_BTN_enableGPIB,
						config->enableGPIB);
	SetCtrlVal (subPanelGeneral, CONFIGgen_BTN_launchDataSocket,
						config->launchDataSocketServer);
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_RINGSLIDE_hardware,
				config->hardwareType);
	
						
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_btnWidth,
				config->buttonWidth);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_btnHeight,
				config->buttonHeight);
	SetCtrlVal (subPanelGeneral, CONFIGgen_BTN_watchdogActivate,
				config->watchdogActivate);
	SetCtrlVal (subPanelGeneral, CONFIGgen_BTN_autoRepeat,
				config->watchdogAutoRepeat);
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_autosaveSeq,
				config->autoSaveSequencesAfterEachStart);
	SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_autosavePath,
				config->autoSaveSequencesPath);
	
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_createFilename,
				config->createFilenameForImages);
	SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathImageFile,
				config->imageFilenameExtensionPath);


	
	// idle sequence and auto focus
	// attention: some of the values are saved to seq instead of config!!!
	SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_idleSeqDatPath,config->idleSequenceDataPath);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_PIFOC_Channel, config->pifocVoltageChannel);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_manualPifocV, seq->manualPifocFocusVoltage); 
	SetCtrlVal (subPanelGeneral, CONFIGgen_CHECKBOX_autofocus,  seq->enableFocusFeedback);
	SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pifocLogPath,config->pifocFocusVoltageLogFilePath);  
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_lastPifocVolt,config->lastPifocFocusVoltage);
	
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_numFocusImage, seq->autoFocusNumImages);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusCent, seq->autoFocusCenterVoltage);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusStep, seq->autoFocusStepVoltage);
	SetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusBloc, seq->autoFocusFirstBlock);
	SetCtrlVal (subPanelGeneral, CONFIGgen_CHECKBOX_isIdleSeq, seq->isOfTypeIdleSeq);   
	
	// DMD
	SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathMagick,config->pathMagickConvert);	
	
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN && config->loadAdwinBoardsFromConfig) {
		implodeIntArray(digitalBoardIDs,config->digitalBoardID,config->nDigitalBoards);
		//tprintf("digital board string: %s\n",digitalBoardIDs); // TODO
		SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_digitalBoardID,
					digitalBoardIDs);
		implodeIntArray(analogBoardIDs,config->analogBoardID,config->nAnalogBoards);
		//tprintf("analog board string: %s\n",analogBoardIDs); // TODO
		SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_analogBoardID,
					analogBoardIDs);
	}
	
	CONFIG_GENERAL_changeVisibility ();
	CONFIG_autoFocusSettingsChanged (subPanelGeneral, 0, EVENT_COMMIT,0,0,0);
	
	
//	EasyTab_SetTabAttribute (panelConfiguration, CONFIG_tabCtrl, subPanelGpib,
//							 ATTR_EASY_TAB_DIMMED,
//							 !config->enableGPIB);
/*	EasyTab_SetTabAttribute (panelConfiguration, CONFIG_tabCtrl, TCP_panelServer(),
							 ATTR_EASY_TAB_DIMMED,
							 !config->enableTCP);
							 
				 
*/
}



void CONFIG_GENERAL_getValues (t_sequence *seq)
{
	int oldEnableTCP;
	char digitalBoardIDs[MAX_PATHNAME_LEN];
	char analogBoardIDs[MAX_PATHNAME_LEN];
	//int tmp[MAX_PATHNAME_LEN]; 
	//int tmpint;
	//int i;
	
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_NDIO_Channels, &seq->maxDigitalChannel);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_NAO_Channels, &seq->maxAnalogChannels);
	
	oldEnableTCP = config->enableTCP;
	GetCtrlVal (subPanelGeneral, CONFIGgen_BTN_enableTCP,
						&config->enableTCP);
	// check if TCP status has chanhged
	if (oldEnableTCP != config->enableTCP) {
		if (config->enableTCP) TCP_initServer (MAIN_parseTCPString);
		else TCP_closeServer ();
	}

	GetCtrlVal (subPanelGeneral, CONFIGgen_RINGSLIDE_hardware,
				&config->hardwareType);

	
	GetCtrlVal (subPanelGeneral, CONFIGgen_BTN_enableGPIB,
						&config->enableGPIB);
	
	GetCtrlVal (subPanelGeneral, CONFIGgen_BTN_launchDataSocket,
						&config->launchDataSocketServer);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_btnWidth,
				&config->buttonWidth);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_btnHeight,
				&config->buttonHeight);
				
	GetCtrlVal (subPanelGeneral, CONFIGgen_BTN_watchdogActivate,
				&config->watchdogActivate);
	GetCtrlVal (subPanelGeneral, CONFIGgen_BTN_autoRepeat,
				&config->watchdogAutoRepeat);
	
	GetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_autosaveSeq,
				&config->autoSaveSequencesAfterEachStart);
	GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_autosavePath,
				config->autoSaveSequencesPath);
	GetCtrlVal (subPanelGeneral, CONFIGgen_BUTTON_createFilename,
				&config->createFilenameForImages);
	GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathImageFile,
				config->imageFilenameExtensionPath);
	
	
	// idle sequence and auto focus
	// attention: some of the values are saved to seq instead of config!!!
	GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_idleSeqDatPath,config->idleSequenceDataPath);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_PIFOC_Channel, &config->pifocVoltageChannel);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_manualPifocV, &seq->manualPifocFocusVoltage); 
	GetCtrlVal (subPanelGeneral, CONFIGgen_CHECKBOX_autofocus, &seq->enableFocusFeedback);
	GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pifocLogPath,config->pifocFocusVoltageLogFilePath);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_lastPifocVolt,&config->lastPifocFocusVoltage);
	
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_numFocusImage, &seq->autoFocusNumImages);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusCent, &seq->autoFocusCenterVoltage);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusStep, &seq->autoFocusStepVoltage);
	GetCtrlVal (subPanelGeneral, CONFIGgen_NUMERIC_autoFocusBloc, &seq->autoFocusFirstBlock);
	GetCtrlVal (subPanelGeneral, CONFIGgen_CHECKBOX_isIdleSeq, &seq->isOfTypeIdleSeq);  
	
	// DMD
	GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathMagick,
				config->pathMagickConvert);
	
	if (seq->isOfTypeIdleSeq) {
		SEQUENCE_updateAutoFocusValues(seq);
	}
	setChanges (seq, 1);
		
	
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN && config->loadAdwinBoardsFromConfig) {  
		GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_digitalBoardID,
					digitalBoardIDs);
	
	
		//tmpint = explodeIntArray(tmp,MAX_DIO_DEVICES,digitalBoardIDs);
		//tprintf("num digital boards %d (%s)\n",tmpint,digitalBoardIDs);
		//for (i=0; i < tmpint; i++) {
		//	tprintf("got %d\n",tmp[i]);	
		//}
		config->nDigitalBoards = explodeIntArray(config->digitalBoardID,MAX_DIO_DEVICES,digitalBoardIDs);
	

		GetCtrlVal (subPanelGeneral, CONFIGgen_STRING_analogBoardID,
					analogBoardIDs);
		config->nAnalogBoards = explodeIntArray(config->analogBoardID,MAX_AO_DEVICES,analogBoardIDs);
		//mpint = explodeIntArray(tmp,MAX_AO_DEVICES,analogBoardIDs);
		//tprintf("num analog boards %d (%s)\n",tmpint,analogBoardIDs);
		//for (i=0; i < tmpint; i++) {
		//	tprintf("got %d\n",tmp[i]);	
		//}
	}
	
	
}




void CONFIG_displayAllSettings (t_sequence *seq)
{
	CONFIG_displayBoardParameters (seq);
	CONFIG_displayGpibSettings ();
	CONFIG_GENERAL_setValues (seq);
	CONFIG_SIGNALS_setParameters (seq);
	CONFIG_displayExternalDevices ();
}


/************************************************************************
 *
 *    parameters edited
 *
 ************************************************************************/

int CVICALLBACK BoardSettingsEdited (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	t_sequence *seq;
	
	switch (event)
		{
		case EVENT_COMMIT:
			seq = activeSeq();
			if (panel == subPanelBoardPar) {
				CONFIG_getBoardParameters (seq);
			}
			seq->repetition = 0;
			CONFIG_displayBoardParameters(seq);
			MAIN_displayTimebases (seq);

		    setChanges (seq, 1);
			break;
		}
	return 0;
}






/*********************************************************

   callback functions for command buttons 
   
   "abort", "update", "done"
   
**********************************************************/


int CVICALLBACK BTN_Setup_Done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	
	switch (event)
		{
		case EVENT_COMMIT:
			seq = activeSeq();
		    //****************************
		    // update board settings
		    //****************************
			CONFIG_getBoardParameters (seq);
			CONFIG_GENERAL_getValues (seq);
			CONFIG_displayAllSettings (seq);
			if (control == CONFIG_BUTTON_done) RemovePopup (1);
			MAIN_resizePanel (seq);
			DIGITALBLOCKS_initPanel ();
			config->splitterHPos = 0;
			MAIN2_resizePanel (seq);
//			DIGITALBLOCKS_resetAllTableAttributes (seq);
			DIGITALBLOCKS_displayAllBlocks (seq);
			MAIN_displayTimebases (seq);
			SEQUENCE_displayStartNr (-1);
			if (config->hardwareTypeBooted != config->hardwareType) HARDWARE_boot(panelMain, MAIN_TEXTBOX_Output1);
			HARDWARE_sendStoredValues (seq);
			CONFIG_write (config);
 			break;
		}
	return 0;
}




int CVICALLBACK BTN_Setup_Abort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
    		RemovePopup (panelConfiguration);
			break;
		}
	return 0;
}




int CVICALLBACK CONFIG_GPIB_autoDetectDevices (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			GPIB_autodetectAllDevices (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices);
			break;
		}
	return 0;
}


int CONFIG_GPIB_deviceSettingsEditable (int device)
{
    t_gpibDevice *dev;

	dev = GPIBDEVICE_ptr (device);
	if (dev == NULL) return 0;
	return ((dev->commandType1 == GPIB_COMMANDTYPE_ARBWFM) || (dev->commandType2 == GPIB_COMMANDTYPE_ARBWFM));
}


int CVICALLBACK CONFIG_GPIB_deviceSelected (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int device;
	
	switch (event) {
		case EVENT_VAL_CHANGED:
			GetCtrlVal (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices, &device);
			SetCtrlAttribute (subPanelGpib, CONFIGgpib_BUTTON_Settings, ATTR_DIMMED, 
				!CONFIG_GPIB_deviceSettingsEditable(device));
			break;
	}
	return 0;
}

void fillDigitalChannelNamesToRing (int panel, int control)
{
	int i;
	
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		InsertListItem (panel, control, -1, str_ChannelNames(i), i);
	}
	
}


void CONFIG_GPIB_deviceSettings_displayItems (int gpibAddress, t_sequence *seq, t_gpibDevice *dev)
{
	//double frequencySCLK_MHz;
	//int devider2;

	if (gpibAddress >= 0) {
		SetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_gpibAddress,
					gpibAddress);
	}
	if (dev != NULL) {
		SetCtrlVal (panelDeviceSettings, DEVICE_STRING_deviceName,
					dev->name);

//		SetCtrlVal (panelDeviceSettings, DEVICE_RADIOBUTTON_SCLK,
//					dev->externalSCLK);
//		SetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_clockFreq,
//					dev->frequencySCLK / 1.0E6);
//		SetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_freqDevider2,
//					dev->divideFreq[1]);
		SetCtrlVal (panelDeviceSettings, DEVICE_RING_triggerSlope,
					dev->triggerSlope);
		SetCtrlVal (panelDeviceSettings, DEVICE_RING_triggerSignal,
					dev->triggerSignal);
		SetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_maxVoltage,
					dev->maxVoltage);
		SetCtrlVal (panelDeviceSettings, DEVICE_RING_load,
					dev->load);
	}
	
	


	
}


int CVICALLBACK CONGIG_GPIB_deviceSettingsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			CONFIG_GPIB_deviceSettings_displayItems (-1, NULL, NULL);
			break;
	}
	return 0;
}




int CVICALLBACK CONFIG_GPIB_EditDeviceSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    int device;
    t_gpibDevice *dev;

	switch (event) {
		case EVENT_COMMIT:
			if (panel == subPanelGpib) 
				GetCtrlVal (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices, &device);
			else 
				GetCtrlVal (panelGpib, GPIB_RING_device, &device);
			if (CONFIG_GPIB_deviceSettingsEditable(device)) {
				if (panelDeviceSettings == -1) {
					panelDeviceSettings = LoadPanel (0, UIR_File, DEVICE);
				}
				InsertListItem (panelDeviceSettings, DEVICE_RING_triggerSignal, -1, 
					"EXT", TRIGGER_SIGNAL_EXT);
				fillDigitalChannelNamesToRing (panelDeviceSettings, DEVICE_RING_triggerSignal);
				dev = GPIBDEVICE_ptr (device);
				CONFIG_GPIB_deviceSettings_displayItems(device, activeSeq(), dev);
				InstallPopup (panelDeviceSettings);
			}
			break;
	}
	return 0;
}


int CVICALLBACK CONFIG_GPIB_abortDeviceSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}


int CVICALLBACK CONFIG_GPIB_doneDeviceSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    int device;
    t_gpibDevice *dev;


	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_gpibAddress, &device);
			dev = GPIBDEVICE_ptr (device);
			GetCtrlVal (panelDeviceSettings, DEVICE_RING_triggerSignal,
						&dev->triggerSignal);
			GetCtrlVal (panelDeviceSettings, DEVICE_RING_triggerSlope,
						&dev->triggerSlope);
			GetCtrlVal (panelDeviceSettings, DEVICE_NUMERIC_maxVoltage,
						&dev->maxVoltage);
			dev->minVoltage = -dev->maxVoltage;
			GetCtrlVal (panelDeviceSettings, DEVICE_RING_load, &dev->load);
			
						
			RemovePopup (0);
			GPIB_displayValues (activeSeq(), activeGpibCommandNo());
			break;
	}
	return 0;
}




int CVICALLBACK GPIB_parameterChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			CONFIG_getGpibSettings ();
			break;
	}
	return 0;
}







void CONFIG_SIGNALS_initPanel (void)
{
}


void CONFIG_SIGNALS_setParameters (t_sequence *seq)

{
//	SetCtrlVal (subPanelSignals, SIGNALS_STRING_20MHz, NDsignalName(ND_PFI_3));
}





int CVICALLBACK CONFIG_savePathBrowse_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	//int err;
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			if (DirSelectPopup ("", "Select Directory", 1, 1, filename)  <= 0) return 0;
			SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_autosavePath,
						filename);
            break;
		}
	return 0;
}



int CVICALLBACK CONFIG_imageFilePathBrowse_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	//int err;
	
	switch (event)
		{
		case EVENT_COMMIT:
		    if (FileSelectPopup (config->defaultPath, "*.txt", "*.txt",
								 "select file that contains image file extension",
								 VAL_SELECT_BUTTON, 0, 0, 1, 1, filename) <= 0) return 0;
			SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathImageFile,
						filename);
            break;
		}
	return 0;
}

int CVICALLBACK CONFIG_convertPathBrowse_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	//int err;
	
	switch (event)
		{
		case EVENT_COMMIT:
		    if (FileSelectPopup (config->defaultPath, "*.exe", "*.exe",
								 "select imagemagick convert",
								 VAL_SELECT_BUTTON, 0, 0, 1, 1, filename) <= 0) return 0;
			SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pathMagick,
						filename);
            break;
		}
	return 0;
}




int CVICALLBACK CONFIG_idleSequenceDataPath_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN]; 
	switch (event)
	{
		case EVENT_COMMIT:
			if (DirSelectPopup ("", "Select Directory", 1, 1, filename)  <= 0) return 0;
			SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_idleSeqDatPath,
						filename);
            break;
	}
	return 0;
}



int CVICALLBACK CONFIG_pifocFocusVoltageLogFilePath_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN]; 
	switch (event)
	{
		case EVENT_COMMIT:
			if (FileSelectPopup (config->defaultPath, "*.txt", "*.txt",
								 "select filename for pifoc voltage log",
								 VAL_SELECT_BUTTON, 0, 0, 1, 1, filename) <= 0) return 0;
			SetCtrlVal (subPanelGeneral, CONFIGgen_STRING_pifocLogPath,
						filename);
			break;
	}
	return 0;
}

int CVICALLBACK CONFIG_isOfTypeIdleChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int checked;
	if (event == EVENT_VAL_CHANGED) {
		GetCtrlVal(subPanelGeneral, CONFIGgen_CHECKBOX_isIdleSeq, &checked);
		SetCtrlAttribute(subPanelGeneral, CONFIGgen_NUMERIC_numFocusImage, ATTR_DIMMED, !checked);
		SetCtrlAttribute(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusCent, ATTR_DIMMED, !checked);
		SetCtrlAttribute(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusStep, ATTR_DIMMED, !checked);
		SetCtrlAttribute(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusBloc, ATTR_DIMMED, !checked);    
	}
	return 0;
}


int CVICALLBACK CONFIG_autoFocusSettingsChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double *vals;
	int numAutofocusImages;
	double autofocusCenterVoltage;
	double autofocusStepVoltage;
	char infoText[500];
	int i;
	
	if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');   

	if (event != EVENT_COMMIT) {
		return 0;	
	}
	infoText[0] = 0;
	
	GetCtrlVal(subPanelGeneral, CONFIGgen_NUMERIC_numFocusImage, &numAutofocusImages);
	GetCtrlVal(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusCent, &autofocusCenterVoltage);
	GetCtrlVal(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusStep, &autofocusStepVoltage); 
	SetCtrlAttribute(subPanelGeneral, CONFIGgen_NUMERIC_autoFocusCent, ATTR_INCR_VALUE, autofocusStepVoltage);
	
	vals = SEQUENCE_calculateAutoFocusValues(numAutofocusImages, autofocusCenterVoltage, autofocusStepVoltage);
	
	//vals = (double*)calloc(numAutofocusImages,sizeof(double));
	for (i=0; i < numAutofocusImages; i++) {
		//vals[i] = autofocusCenterVoltage-(double)(numAutofocusImages-1)/2.0*autofocusStepVoltage + i*autofocusStepVoltage;
		if (strlen(infoText) == 0) {
			sprintf(infoText,"%.3f", vals[i]);
		} else { 
			sprintf(infoText,"%s\n%.3f",infoText,vals[i]);
		}
	}
	free(vals);
	ResetTextBox (subPanelGeneral, CONFIGgen_TEXTMSG_autoFocusVals,infoText);

	return 0;
}

		
			
int CVICALLBACK CONFIG_BUTTON_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CONFIG_GENERAL_changeVisibility ();
			break;
	}
	return 0;
}


char *CONFIG_getPathSuffixToday (void)
{
	char *tmp;
	int month, day, year;
	int hours, minutes, seconds;
	
	tmp = getTmpStr();
	GetSystemDate (&month, &day, &year);
	GetSystemTime (&hours, &minutes, &seconds);
	
	// if before 7:00 am, switch back to previous day
	if (hours < 7) day--;
	if (day <= 0) { 
		month--;
		if (month <= 0) {
			month = 12;
			year--;
		}
		
		day = daysPerMonth (month, year);
	}
	
//	sprintf (c->dataPathToday, "%s\\%d\\%s\\%02d\\",
//			c->autoSaveSequencesPath, year, monthStr(month), day);

	sprintf (tmp, "%d\\%02d\\%02d\\",year, month, day);
	return tmp;
}



void CONFIG_createTodaysDataPath (t_config *c) 
{
	sprintf (c->dataPathToday, "%s\\_%s",
			c->autoSaveSequencesPath, CONFIG_getPathSuffixToday ());
}


int CONFIG_getCurrentStartNumber (void) 
{
	char searchStr[MAX_PATHNAME_LEN];
	char found[MAX_PATHNAME_LEN];
	int startNr;
	int help;
	char h[N_STARTNR_DIGITS + 2];
	int status;
	
	CONFIG_createTodaysDataPath (config);
	strcpy (searchStr, config->dataPathToday);
	strcat (searchStr, startSearchName);
	startNr = 0;
	status = GetFirstFile (searchStr, 1, 1, 1, 1, 1, 1, found);
	if (status == 0) {
		do {
			strncpy (h, found, N_STARTNR_DIGITS);
			h[N_STARTNR_DIGITS] = 0;
			if (!StrToInt (h, &help)) help = 0;
			if (help  > startNr) startNr = help;
		} while (GetNextFile (found) == 0);
		startNr ++;
		if (startNr <= config->startNr) {
			PostMessagePopupf("Error while determining start no","The old start number %03d is larger or equal the new start number %03d. Using %03d as next start no.",config->startNr,startNr,config->startNr+1); 
			startNr = config->startNr+1;
		}
	}
	else {
		startNr = 1;
/*		PostMessagePopupf ("File error!", "An error occurred when searching path\n'%s'.\n\n"
					   "Error code: %d, %s",
					   config->dataPathToday, status, getErrorFileOperation (status));
*/
	}
	config->startNr = startNr;
	return startNr;
}

int CVICALLBACK CONFIG_BTN_abort_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup (1);
			break;
	}
	return 0;
}

int CVICALLBACK CONFIG_BTN_adwinGetSystemInfo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HARDWARE_ADWIN_getSystemInfo ();
			break;
	}
	return 0;
}


int CVICALLBACK CONFIG_BTN_adwinBoot (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if (SEQUENCE_getStatus () != SEQ_STATUS_FINISHED) return 0;
			if (config->hardwareType == HARDWARE_TYPE_ADWIN) {
				if (panel==panelMain) control = MAIN_TEXTBOX_Output1;
				else control = BOARDPAR_TEXTBOX_adWinOutput;
				HARDWARE_ADWIN_boot (panel, control);
			}
			break;

	}
	return 0;
}

int CVICALLBACK BTN_updateAutofocusVals (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence* seq;
	switch (event)
	{
		case EVENT_COMMIT:
			seq = activeSeq();
			CONFIG_GENERAL_getValues (seq);
			CONFIG_displayAllSettings (seq);
			
			CONFIG_autoFocusSettingsChanged (panel, control, EVENT_COMMIT,0,0,0);
			if (seq->isOfTypeIdleSeq) {
				SEQUENCE_updateAutoFocusValues(seq);
			}
			setChanges (seq, 1);
			DIGITALBLOCKS_displayAllBlocks (seq);
			RemovePopup(1);
			
			break;
	}
	return 0;
}
