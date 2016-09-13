#include <cvirte.h>		
#include <userint.h>
#include "UIR_LoopExperimentControl.h"

static int panelHandle;

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "UIR_LoopExperimentControl.uir", PANEL)) < 0)
		return -1;
	
	DisplayPanel (panelHandle);
	SetPanelAttribute (panelHandle, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM); 
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}



void LaunchExperimentControl (void)
{
	errorCode = LaunchExecutableEx (commandStr, LE_HIDE, &handle);
	if (errorCode != 0) {
		PostMessagePopupf ("Error", "Function 'LaunchExecutableEx()' failed.\n\nError code %d: %s",
						errorCode, errorStringUtility (errorCode));
		return ERR_TOOLS_UNZIP_NOUNZIPPPER;
	}
	
	startTime = clock ();
	seconds = 0;
	while (!ExecutableHasTerminated (handle) && (seconds <= UNZIP_TIMEOUT_S)) {
		seconds = timeStop (startTime) / CLOCKS_PER_SEC;
	}
	RetireExecutableHandle (handle);



int CVICALLBACK PANEL_callback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface (0);
			break;
		}
	return 0;
}
