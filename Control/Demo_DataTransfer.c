#include <cvirte.h>		
#include <userint.h>
#include "dataskt.h"
#include "nidaqcns.h"  
#include "tools.h"

#include "UIR_DemoDataTransfer.h"

#include "CONTROL_DataStructure.h"
#include "CONTROL_LoadSave.h"
#include "CONTROL_Definitions.h"
#include "CONTROL_DataSocket_Client.h"


int panelHandle;

const char UIR_FILE[] = "UIR_DemoDataTransfer.uir";
const char datasocketServer[] = "dstp://localhost";



t_sequence *autoGenerateSequence (void)
{
	t_sequence *s;
	t_digitalBlock *block;
	
	s = SEQUENCE_new ();
	SEQUENCE_init (s);
	
    // set timebases 
    // timebase = DIO_timeBase + DIO_patternInterval;
    s->DIO_timeBase        = VAL_HARDWARE_TIMEBASE_50ns;
    s->AO_timeBase         = VAL_HARDWARE_TIMEBASE_50ns;
    s->DIO_patternInterval = 2;   // timebase = 100 ns = 2 x 50 ns
    s->AO_patternInterval  = 40;  // timebase = 2 µs
    // trigger source = PFI 0
    //s->trigger 			   = ND_PFI_0;
    
    // --------------------
    //    set filename
    // --------------------
    strcpy (s->filename, "testsequence.seq");
   
    // --------------------
    // set channel names
    // --------------------
    s->DIO_channelNames[CH_A3] = strAlloc ("Testchannel");
    
    // -----------------------
    //  insert digital blocks
    // -----------------------
    block = DIGITALBLOCK_new (s);
    DIGITALBLOCK_init (block);
    block->duration = 10 * VAL_ms;  // duration = 10 ms
    // set channel A3 to 1
    block->channels[CH_A3] = 1;
	
	return s;
}






int transmitSequence (t_sequence *seq, const char *commandStr)
{
	IniText ini;
	char *buffer = NULL;
	unsigned long bufferSize;

	// create sequence as "*.ini" structure
	ini = Ini_New (0);
	Ini_PutSequence (ini, seq);
	// write .ini structure to buffer
	if (Ini_WriteToBuffer (ini, &buffer, &bufferSize, commandStr, 0) == 0) {
		// send buffer to DataSocket server
		DATASOCKET_transferBuffer (buffer, bufferSize);
	}

	Ini_Dispose (ini);    	
	free (buffer);
	return 0;
}






int main (int argc, char *argv[])
{
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, UIR_FILE, PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	SetPanelAttribute (panelHandle, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
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



int CVICALLBACK callback_transferSequence (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *s;
	char commandStr[100];

	switch (event) {
		case EVENT_COMMIT:
			// transfer a simple command for test
//			DataSocket_transferBuffer (TCP_CMD_STOP, strlen (TCP_CMD_STOP)+1);
			
			s = autoGenerateSequence ();
	
			// create command String: start sequence immediately
			sprintf (commandStr, "%s %d %d %d", TCP_CMD_RUN, 0, 0, 0);
		//	if (launchExperimentControl () != 0) goto ERROR;
			transmitSequence (s, commandStr);
			
			SEQUENCE_free (s); // remove from queue before?
			break;
	}
	return 0;
}


