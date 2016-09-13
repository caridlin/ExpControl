#include <windows.h>  // for Sleep
#include <formatio.h>
#include <utility.h>
#include "Adwin.h"    
#include "tools.h"
#include "CONTROL_HARDWARE_adwin.h"					
#include "CONTROL_HARDWARE_GPIB.h"					
			
#include "CONTROL_GUI_Main.h"					
#include "CONTROL_GUI_Waveform.h"					
#include "CONTROL_DataStructure.h"
#include "CONTROL_CalculateOutput.h"
#include "UIR_ExperimentControl.h"

#define ADWIN_DEBUG 0

char adwinFilename[]=  "C:\\ADwin\\Adwin12.btl";

	
char FILENAME_PROCESS_DIRECTIO[] = "Adbasic_ExperimentControl_LP.TC2";
char FILENAME_PROCESS_OUTPUT1[] = "Adbasic_ExperimentControl.TC1";
	
	

int HARDWARE_SetGetInterruptedFlag (int set);   


#define N_DIRECT_IO_VALUES (MAX_DIO_DEVICES+N_DAC_CHANNELS)


//long ADWIN_directIOValues[N_DIRECT_IO_VALUES];




int outPanel;
int outCtrl;


extern int panelMain;
extern int panelAddressing;
extern int gpibOutputPanel;
extern int gpibOutputCtrl;




extern t_image *img;
extern t_image *imgPos;

extern t_waveform *imgWfm;
extern t_waveform *imgWfmPos;

const double TIMEBASE_FACTOR = (double)(ADWIN_TIMEBASE_DIGITAL) / VAL_ms;




void HARDWARE_ADWIN_addConfig (void)
{
	//char* digitalBoardIDs = "1,2,3";
	//char* analogBoardIDs = "4,5,6,7,8";
	//tprintf("called HARDWARE_ADWIN_addConfig\n");
	
	//tprintf("number of analog boards: %d\n",c->nAnalogBoards);  
	//tprintf("number of digital boards: %d\n",c->nDigitalBoards);  
	
	if (!config->loadAdwinBoardsFromConfig) {
		config->nAnalogBoards = 8;   
		config->nDigitalBoards = 3;
		config->analogBoardID[0] = ADWIN_AOUT1;
		config->analogBoardID[1] = ADWIN_AOUT2;
		config->analogBoardID[2] = ADWIN_AOUT3;
		config->analogBoardID[3] = ADWIN_AOUT4;
		config->analogBoardID[4] = ADWIN_AOUT5;
		config->analogBoardID[5] = ADWIN_AOUT6;
		config->analogBoardID[6] = ADWIN_AOUT7; 
		config->analogBoardID[7] = ADWIN_AOUT8; 
		config->digitalBoardID[0] = ADWIN_DIO1;
		config->digitalBoardID[1] = ADWIN_DIO2;
		config->digitalBoardID[2] = ADWIN_DIO3;
	}
	
}



int Set_Par_Err (short index, long value)
{
	int status; 
	
	status = Set_Par (index, value);
	if (status == 255) {
		PostMessagePopupf ("ADWIN error", "ADWIN: 'Set_Par()' returned an error. %s\n", 
				 Get_Last_Error_Text (Get_Last_Error ()));
		return -1;
	}
	return 0;
}



int SetData_Long_Err (short dataNo, long *data, long index, long count)
{
	int status; 
	
	status = SetData_Long (dataNo, data, index, count);
	if (status == 255) {
		PostMessagePopupf ("ADWIN error", "ADWIN: 'SetData_Long()' returned an error. %s\n", 
				 Get_Last_Error_Text (Get_Last_Error ()));
		return -1;
	}
	return 0;
}




int HARDWARE_ADWIN_getCycleNo (void)
{
	long par;
	
	par = Get_Par (ADWIN_P1_PAR_CYCLE_NO);
	return par;
}



int HARDWARE_ADWIN_transmitData (t_adwinData *a, int displayMessage)
{
	//int device;
	int maxValues = 0;
	clock_t startTime;
	
	unsigned int i;
	long* tmp;
	
	
	//tprintf("test0\n");ProcessDrawEvents();    
//	DEBUGOUT_initPanel ();
//	OUTDATA_ADWIN_displayDigitalValues (a);

	
	startTime = clock ();

	//tprintf("test1\n");ProcessDrawEvents();
	// transmit board configuration
	if (Set_Par_Err (ADWIN_P1_PAR_nAOmodules, config->nAnalogBoards) != 0) return -1;
	if (!config->suppressProtocol && displayMessage) tprintf ("Transmitting Data to AdwinSystem. ");

	//tprintf("test2\n");ProcessDrawEvents();    
	tmp = (long*)malloc(config->nAnalogBoards*sizeof(long));
	for (i=0;i<config->nAnalogBoards; i++) {
		tmp[i] = config->analogBoardID[i];	
	}
	if (SetData_Long_Err (ADWIN_P1_DATA_AOmodules, tmp, 1, config->nAnalogBoards) != 0) return 0;
	free(tmp);
	//tprintf("test3\n");ProcessDrawEvents();    
	
	if (Set_Par_Err (ADWIN_P1_PAR_nDIOmodules, config->nDigitalBoards) != 0) return -1;
	tmp = (long*)malloc(config->nDigitalBoards*sizeof(long));
	for (i=0;i<config->nDigitalBoards; i++) {
		tmp[i] = config->digitalBoardID[i];	
	}
	
	if (SetData_Long_Err (ADWIN_P1_DATA_DIOmodules, tmp, 1, config->nDigitalBoards) != 0) return 0;
	free(tmp);
	
	
	// transmit DIO data
	maxValues = Get_Par (ADWIN_P1_PAR_MAX_DIO_buffer);
	if (a->nDIOelements > maxValues) {
		PostMessagePopupf ("Error", "Buffersize of digital data (%d bytes) exceeds reserved memory size of ADWIN system (%d bytes).", a->nDIOelements*4, maxValues*4); 	
		return -1;
	}
	
	if (Set_Par_Err (ADWIN_P1_PAR_nDIOelements, a->nDIOelements) != 0) return -1;
	if (ADWIN_DEBUG) tprintf ("nDIOelements = %d\n", a->nDIOelements);
	if (SetData_Long_Err (ADWIN_P1_DATA_DIObuffer, a->DIObuffer, 1, a->nDIOelements) != 0) return -1;

	if (!config->suppressProtocol && displayMessage) tprintf ("DIO: %d kB", a->nDIOelements * 4 / 1024 +1);
	
	//
	if (Set_Par_Err (ADWIN_P1_PAR_MAXCYCLE_NO, a->maxCycleNo) != 0) return -1;
	if (ADWIN_DEBUG) tprintf ("nCycles = %d\n", a->maxCycleNo);

	// transmit AO data
	maxValues = Get_Par (ADWIN_P1_PAR_MAX_AO_buffer);
	if (a->nAOelements > maxValues) {
		PostMessagePopupf ("Error", "Buffersize of analog data (%d bytes) exceeds reserved memory size of ADWIN system (%d bytes).", a->nAOelements*4, maxValues*4); 	
		return -1;
	}
	if (!config->suppressProtocol && displayMessage) tprintf (", AO: %d kB.", a->nAOelements * 4 / 1024 +1);
	
	if (Set_Par_Err (ADWIN_P1_PAR_AOnElements, a->nAOelements) != 0) return -1;
	if (ADWIN_DEBUG) tprintf ("nAOelements = %d\n", a->nAOelements);
	if (SetData_Long_Err (ADWIN_P1_DATA_AObuffer, a->AObuffer, 1, a->nAOelements) != 0) return -1;
	if (SetData_Long_Err (ADWIN_P1_DATA_AOvaluesAtEndOfSequence, a->AOvaluesAtEndOfSequence, 1, N_DAC_CHANNELS) != 0) return -1;
	// transmit trigger settings
	if (Set_Par_Err (ADWIN_P1_PAR_trigger, a->trigger) != 0) return -1;
	maxValues = Get_Par (ADWIN_P1_PAR_MAX_TriggerTimes) - 1;
	if (a->nTriggerTimesDuringSeq > maxValues) {
		PostMessagePopupf ("Error", "Too many triggers (%d) within sequence. Maximum number of triggers is %d.", a->nTriggerTimesDuringSeq, maxValues); 	
		return -1;
	}
	if (Set_Par_Err (ADWIN_P1_PAR_N_triggerTimes, a->nTriggerTimesDuringSeq+1) != 0) return -1;
	if (SetData_Long_Err (ADWIN_P1_DATA_triggerTimesDuringSeq, a->triggerTimesDuringSeq, 1, a->nTriggerTimesDuringSeq+1) != 0) return -1;
	if (SetData_Long_Err (ADWIN_P1_DATA_analogOffsetsDuringSeq, a->AOoffsetsDuringSequence, 1, ADMIN_MAX_OFFSET_INDEX) != 0) return -1;
	if (!config->suppressProtocol && displayMessage) tprintf (" [%1.3f s]\n", timeStop_s (startTime));
	
	return 0;

}







int HARDWARE_ADWIN_transmitAnalogOffsetDouble (double value, int DACchannel)
{
	
	long lValue;
	int zero;
	int module, moduleChannel, index;
	int err = 0;
	
	zero = AnalogToDigital (0, digitizeParameters_ADWIN());
	//tprintf ("0=%d ", zero);
//	value = -5.0;
//	if (DACchannel == 38) value = -2.0;
	lValue = AnalogToDigital (value, digitizeParameters_ADWIN()) - zero;
	

	module = config->analogBoardID[DACchannel/8];
	moduleChannel = DACchannel % 8;
	index = (module << 4) + moduleChannel + 1;
	
	pprintf (panelAddressing, ADDRESS_TEXTBOX, "sending feedback value DAC%d = %1.3fV, index=%d\n", DACchannel, value, index);

	if (index > ADMIN_MAX_OFFSET_INDEX) return -1;
	err = SetData_Long_Err (ADWIN_P1_DATA_analogOffsetsDuringSeq, &lValue, index, 1);
	if (err != 0) return err;
	
	return err;
}




int HARDWARE_ADWIN_outputFinished (short process)
{
	long status;
	
	status = Process_Status (process);
	return (status == 0);
}


const char *HARDWARE_ADWIN_errorText (int status, int cycle)
{
	static char help[300];
	
	switch (status) {
		case ADWIN_ERRORCODE_AO_BUFFER_EMPTY: 
			sprintf (help, "Analog buffer underrun at cycle %d", cycle);
			return help;
		case ADWIN_ERRORCODE_PROCESS_TOO_LONG:
			sprintf (help, "Process too long at cycle no. %d", cycle);
			return help;
	}
	return "";
	
}


int HARDWARE_ADWIN_displayPointsGenerated (void)
{
	long cycleNo;
	
	cycleNo = Get_Par (ADWIN_P1_PAR_CYCLE_NO);
	OUTPUT_displayPointsGenerated (cycleNo);
	return cycleNo;
}


int HARDWARE_ADWIN_isDataOutputFinished (void)
{
	return (Process_Status (ADWIN_PROCESS_DATA_OUT) != 1);
}


int HARDWARE_ADWIN_focusFeedback (t_adwinData *a)
{
	float focusPifocVoltage;
	int err;
	//
	// feedback for focusing
	// (before check for phaseFeedbackActive to let it run independently from this flag)
	//
	//tprintf("\nStart auto focus.\");  
	t_sequence* currentSeq = ((t_sequence*)a->callingSequence);
	if (currentSeq == NULL) {
		tprintf("Error in focusFeedback: sequence NULL\n");
		return -1;
	}
	if (currentSeq->enableFocusFeedback) {
		focusPifocVoltage = WFM_ADDR_findFocusFromIdleSequence(currentSeq);
	} else {
		//focusPifocVoltage = config->lastPifocFocusVoltage;
		focusPifocVoltage = currentSeq->manualPifocFocusVoltage;
	}
	if (focusPifocVoltage < 0 || focusPifocVoltage > 10) { // determination of new value failed, try to recover ...
		tprintf("\nPifoc focusing failed. Trying to use last used value.\n");
		ProcessDrawEvents (); 
		focusPifocVoltage = -1;
		/*file = OpenFile (config->pifocFocusVoltageLogFilePath, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
		if (file == -1) {
			tprintf ("File error: %s\n", GetFmtIOErrorString (GetFmtIOError()));
		} else {
			i = 0;
			while (ReadLine (file, line, L_LEN-1) >= 0) {
				sscanf (line, "%f",&focusPifocVoltage);
				i++;
			}
			CloseFile(file);
		}*/
		focusPifocVoltage = config->lastPifocFocusVoltage;
		if (focusPifocVoltage < 0 || focusPifocVoltage > 10) {
			tprintf("Old pifoc focus value invalid.\n");
			ProcessDrawEvents (); 
			focusPifocVoltage = 0;
		}
		err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (focusPifocVoltage, config->pifocVoltageChannel);
		tprintf("Using old pifoc voltage: %.3f\n", focusPifocVoltage);
	} else {
		//
		//   transmit voltage offset to adwin system
		//
		if (currentSeq->enableFocusFeedback) { 
			config->lastPifocFocusVoltage = focusPifocVoltage;
		}
		err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (focusPifocVoltage, config->pifocVoltageChannel);
		if (currentSeq->enableFocusFeedback) {   
			tprintf("Send new Pifoc focus voltage %.3f on analog channel DAC%d\n", focusPifocVoltage,config->pifocVoltageChannel); 
		} else {
			tprintf("Send manual Pifoc focus voltage %.3f on analog channel DAC%d\n", focusPifocVoltage,config->pifocVoltageChannel); 	
		}
	}
	if (err) {
		tprintf("Error during transmission of analog offset for auto focus.\n"); 
	}
	return err;
	
}

	/*
void CVICALLBACK MainThread_ShowAddressingPanel (void *callbackData)
{
	if (panelAddressing == 0) {
		panelAddressing = LoadPanel (0, UIR_File, ADDRESS);	
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH2,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH_voltages,ATTR_REFRESH_GRAPH,0); 
	}
	displayPanel2 (panelAddressing);
	SetPanelAttribute (panelAddressing, ATTR_FRAME_COLOR, VAL_RED);   
}  */


int HARDWARE_ADWIN_phaseFeedback (t_adwinData *a, int callNo)
{
	t_ccdImages *rawImg;
	static dPoint Uoffset, UAddrOffset, UTotal;
	t_waveform *wfm1, *wfm2;
	int err;
	int imageLoaded;
	static int firstcall = 1;
	char text[30];
	static char lastFilename[MAX_PATHNAME_LEN] = "";
	static dPoint lastPhases;	   // phases determined in last addressing shot
	static dPoint lastPhaseOffset; // not used for feedback (?), only for documentation and logging
	static dPoint lastPhasesErr;
	static ListType pointList = NULL;
	static ListType lastVoltagesList = NULL;
	#define L_LEN 300
	//char line[L_LEN];
	
	
	if (firstcall) {
		UAddrOffset = MakeDPoint (0,0);
		Uoffset = MakeDPoint (0,0);
		firstcall = 0;
		lastPhases = MakeDPoint (0,0);
		lastPhasesErr = MakeDPoint (0,0);
		lastPhaseOffset = MakeDPoint (0,0);
		
	}
	
	if (!a->phaseFeedbackActive) {
		if (panelAddressing > 0) HidePanel (panelAddressing);
		return -1;
	}
	
	
	if (panelAddressing == 0) {
		tprintf("FATAL ERROR: addressing panel not initialized!");   // cannot load panel here in wrong thread
	}
	displayPanel2 (panelAddressing);  
	SetPanelAttribute (panelAddressing, ATTR_FRAME_COLOR, VAL_RED);   
 
	/*
	if (panelAddressing == 0) {
		panelAddressing = LoadPanel (0, UIR_File, ADDRESS);	
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH2,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH_voltages,ATTR_REFRESH_GRAPH,0); 
	}
	displayPanel2 (panelAddressing);
	*/
	//PostDeferredCallToThread (MainThread_MessagePopup, null, CmtGetMainThreadID ()); 
	//PostDeferredCallToThreadAndWait (MainThread_MessagePopup, null, CmtGetMainThreadID ()); 

	
	wfm1 = (t_waveform *) a->phaseFeedbackWfm[0];
	wfm2 = (t_waveform *) a->phaseFeedbackWfm[1];
	
	if (wfm1 == NULL || wfm2 == NULL) {
		tprintf("################################################################\n");   
		tprintf("Fatal error in phase Feedback: phase feedback wfm1 or wfm2 is NULL\n");
		return -1;
	}	
	
	tprintf("Running phase feedback with waveforms \"%s\" and \"%s\"\n",wfm1->name,wfm2->name);
	
	
	if (callNo == 0) {
		pprintf(panelAddressing, ADDRESS_TEXTBOX, "\n-----  RUN nr. %d, rep. %d ----\n", a->startNr, a->repetition);
		if ((a->repetition == 1) && (wfm1->addrAddLastOffsetNotAtFirst)) {
			UTotal.x = 0;
			UTotal.y = 0;
			
		}
		else {
			pprintf(panelAddressing, ADDRESS_TEXTBOX,  "sum of offsets          : U1=%1.3f, U2=%1.3f\n", UTotal.x, UTotal.y);
			//
			//   transmit voltage offset to adwin system
			//
			err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (UTotal.x, wfm1->channel);
			if (err == 0) {
			    err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (UTotal.y, wfm2->channel);		
			}
			if (a->repetition == 1) { // TODO check this!!!!
				ListDispose (lastVoltagesList);
				lastVoltagesList = WFM_ADDR_lastVoltagesList (wfm1, wfm2);
				//lastPhaseOffset.x = wfm1->addrOffsetThisRepetition[0];
				//lastPhaseOffset.y = wfm1->addrOffsetThisRepetition[1];
				lastPhaseOffset.x = wfm1->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1];  
				lastPhaseOffset.y = wfm1->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2]; 
				
			}
		}

	}
	
	if (!FileExists (a->phaseFeedbackImageFilename, 0)) {
		tprintf("Phase feedback failed: phaseFeedbackImageFilename not found: \n");
		tprintf("    %s\n",a->phaseFeedbackImageFilename);
		return -1;
	}
	if (strcmp (a->phaseFeedbackImageFilename, lastFilename) == 0) return 0;
	

	//
	// init image file datasets
	//
	
	//  a) feedback from fluorescence image

	
	IMAGE_free (img);
	img = IMAGE_new ();
	if (imgWfm == NULL) {
		imgWfm = WFM_new (0);
		WFM_init (imgWfm, WFM_TYPE_ADDRESSING);
	}
	memcpy (imgWfm->addrCalibration, wfm1->addrCalibration, sizeof (double) * WFM_ADDR_N_CALIBRATION_PARAMETERS);

	IMAGE_free (imgPos);
	imgPos = IMAGE_new ();
	if (imgWfmPos == NULL) {
		imgWfmPos = WFM_new (0);
		WFM_init (imgWfmPos, WFM_TYPE_ADDRESSING);
	}
	memcpy (imgWfmPos->addrCalibration, wfm1->addrCalibration, sizeof (double) * WFM_ADDR_N_CALIBRATION_PARAMETERS);
	
	rawImg = CCDIMAGES_new ();
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Loading %s\n", a->phaseFeedbackImageFilename);
	
	
	imageLoaded = (CCDIMAGES_loadAndorSifFile (a->phaseFeedbackImageFilename, rawImg, 0) == 0);
	
	WFM_ADDR_writeDataToProtocolFile (a->phaseFeedbackProtocolFilename, a->phaseFeedbackImageFilename, a->repetition-1, lastPhases, lastPhasesErr, Uoffset, UAddrOffset, UTotal, lastPhaseOffset);

	if (imageLoaded) {
		strcpy (lastFilename, a->phaseFeedbackImageFilename);
		if (wfm1->addrEnablePhaseFeedback2) {
			//  feedback from position in addressing beam
			pprintf(panelAddressing, ADDRESS_TEXTBOX2, "\n-----  RUN nr. %d, rep. %d  ----\n", a->startNr, a->repetition);
			IMAGE_free (imgPos);
			imgPos = IMAGE_new ();
			IMAGE_assignFromCCDimage  (imgPos, rawImg, 0, -1, 0, 0, 0);
			sprintf (text, "%03d / %d", a->startNr, a->repetition == 1 ? 1: a->repetition-1);
			IMAGE_showInGraph (panelAddressing, ADDRESS_GRAPH3, imgPos, text);
			ProcessDrawEvents ();
			
			UAddrOffset = WFM_ADDR_calculateAddrVoltageOffset (imgPos, wfm1);
		}
		else {
			// feedback from single atoms in fluorescence picture
			if (a->repetition < 2) pprintf(panelAddressing, ADDRESS_TEXTBOX, "------------------------------------------------------------------------------------\n");
			IMAGE_assignFromCCDimage  (img, rawImg, 0, -1, 0, 0, 0);
			sprintf (text, "%03d / %d", a->startNr, a->repetition-1);
			WFM_ADDR_showImages (img, wfm1, wfm1->addrImageSize, text);

			// calculate + show list of pixel coordinates
			
			if (wfm1->addrType == WFM_ADDR_TYPE_DMD) {
				pointList = WFM_ADDR_convertMatrixToImageCoords (wfm1, img, WFM_ADDR_matrixNumThisRepetition(wfm1,a->repetition-2,a->maxCycleNo,0)); // write addr points for the sequence before.  
			} else {
				pointList = WFM_ADDR_voltageListToImageCoords (wfm1, lastVoltagesList, Uoffset); 
			}
			WFM_ADDR_writePointListToFile (changeSuffix (0, a->phaseFeedbackImageFilename, "_addrPoints.txt"), pointList);
			WFM_ADDR_showPointListInGraph (panelAddressing, ADDRESS_GRAPH1, pointList, VAL_SOLID_CIRCLE, VAL_GREEN,1);
			ListDispose (pointList); 
			
			// find new phases
			if (!wfm1->addrCalibration[WFM_ADDR_CAL_phaseFeedbackOff]) WFM_ADDR_findPhase (img, wfm1, lastPhases, lastPhasesErr);
			lastPhases.x = img->gridPhi[0];
			lastPhases.y = img->gridPhi[1];
			lastPhasesErr.x = img->gridPhiErr[0];
			lastPhasesErr.y = img->gridPhiErr[1];
			
			// set phases in DMDimage, if the waveform is of type DMD addressing
			if (wfm1->addrType == WFM_ADDR_TYPE_DMD && wfm1->addrDMDimage != NULL) {
				wfm1->addrDMDimage->phaseFeedbackPhase.x = img->gridPhi[0];
				wfm1->addrDMDimage->phaseFeedbackPhase.y = img->gridPhi[1];   
			}
			
			// remember voltages + phase offsets
			ListDispose (lastVoltagesList);
			lastVoltagesList = WFM_ADDR_lastVoltagesList (wfm1, wfm2);
			//lastPhaseOffset.x = wfm1->addrOffsetThisRepetition[0];
			//lastPhaseOffset.y = wfm1->addrOffsetThisRepetition[1];
			lastPhaseOffset.x = wfm1->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1];
			lastPhaseOffset.y = wfm1->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2];
		}
	} else {
		tprintf("################################################################\n");
		tprintf("Phase feedback error: image loading failed.");	
	}

	
	//
	//  calculate voltage offset for next repetition
	//
	if (wfm1->addrCalibration[WFM_ADDR_CAL_phaseFeedbackOff] != 0) Uoffset = MakeDPoint (0,0);
	else Uoffset = WFM_ADDR_voltageOffsetFromPhaseDifference (a->phaseFeedbackWfm[0], img, wfm1->addrCenter);
	
	pprintf(panelAddressing, ADDRESS_TEXTBOX,  "Voltage offsets (image): U1=%1.3f, U2=%1.3f\n", Uoffset.x, Uoffset.y);
	pprintf(panelAddressing, ADDRESS_TEXTBOX,  "Voltage offsets (addr) : U1=%1.3f, U2=%1.3f\n", UAddrOffset.x, UAddrOffset.y);
	UTotal.x = Uoffset.x + wfm1->addrAddLastOffset * UAddrOffset.x;
	UTotal.y = Uoffset.y + wfm1->addrAddLastOffset * UAddrOffset.y;
	
	if ((a->repetition == 1) && (wfm1->addrAddLastOffsetNotAtFirst)) {
		UTotal.x = 0;
		UTotal.y = 0;
	}
	pprintf(panelAddressing, ADDRESS_TEXTBOX,  "sum of offsets          : U1=%1.3f, U2=%1.3f\n", UTotal.x, UTotal.y);
	//
	//   transmit voltage offset to adwin system
	//
	err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (UTotal.x, wfm1->channel);
	if (err == 0) {
	    err = HARDWARE_ADWIN_transmitAnalogOffsetDouble (UTotal.y, wfm2->channel);		
	}
	
	
	CCDIMAGES_free (rawImg);
	free (rawImg);

	
	SetPanelAttribute (panelAddressing, ATTR_FRAME_COLOR, VAL_PANEL_GRAY);
	
	RefreshGraph (panelAddressing, ADDRESS_GRAPH1);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH2);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH_voltages);
	
	return 0;
}


// does a lot of feedback stuff (while sequence is already running)

int HARDWARE_ADWIN_feedback (t_adwinData *a, int callNo)
{
	t_waveform *waveformDMD;
	t_sequence *seq = a->callingSequence;
	
	if (callNo == 0) {
		HARDWARE_ADWIN_focusFeedback(a);
	}

	// DMD phase feedback maybe the std phase feeback is ok for that
	//HARDWARE_ADWIN_DMDphaseFeedback(a,callNo);
	
	// real phase feedback
	HARDWARE_ADWIN_phaseFeedback(a,callNo);
	
	// ===========================================================
	// 	   DMD 
	// ===========================================================
	waveformDMD = WFM_ADDR_findDMDwaveform (seq); 
	if (waveformDMD != NULL) {
		DMDIMAGE_THREAD_writeCommandFile (seq, waveformDMD, seq->repetition);
		//if (DMDIMAGE_THREAD_writeCommandFile (seq, waveformDMD, seq->repetition) != 0) {
		//	tprintf("Writing command files for DMD failed.\n");
		//}
	}
	
	return 0;	
}

	

int HARDWARE_ADWIN_waitUntilOutputFinished (t_adwinData *a)
{
	int status = 0;
	int interrupted = 0;
	//long *arr;
	#define N_VALUES 500000
	//int i, j;
	int phaseFeedbackDone = 0;
	int nGPIBCommandsExecuted = 0;
	t_gpibCommand *g;
	double time_s;
	unsigned int cycleNo;
	double duration_ms;
	int callNo = 0;
	
	//i = 0;

/*	if (a->nGpibCommands > 0) {
		tprintf ("GPIB commands during sequence:\n");
		for (i = 0; i < a->nGpibCommands; i++) {
			g = (t_gpibCommand *) a->gpibCommandPtr[i];
			time_s = 1.0*(a->gpibCommandExecutionCycle[i] * ADWIN_TIMEBASE_DIGITAL) / (1.0*VAL_s);
		}
	}
*/
	if (a->phaseFeedbackActive) tprintf ("--Phase feedback: Waiting for %s\n", a->phaseFeedbackImageFilename);
	
	// Sleep(100); // wait a bit - seems there was a race condition with the adwin system
	// HARDWARE_ADWIN_displayPointsGenerated is sometimes called before adwin system is ready without the delay
	// and returns some uninitialized field 
	callNo = 0;
	while (!interrupted && (!HARDWARE_ADWIN_isDataOutputFinished ())) {
		cycleNo = HARDWARE_ADWIN_displayPointsGenerated ();
		//
		// execute GPIB commands during sequence
		//
		
		if (nGPIBCommandsExecuted < a->nGpibCommands) {
			if (cycleNo > a->gpibCommandExecutionCycle[nGPIBCommandsExecuted]) {
				// execute GPIB_command
				g = (t_gpibCommand *) a->gpibCommandPtr[nGPIBCommandsExecuted ];
				// time_s = TIMEBASE_FACTOR * a->gpibCommandExecutionCycle[nGPIBCommandsExecuted];
				// pprintf (gpibOutputPanel, gpibOutputCtrl, "=============================================\n");
				// pprintf (gpibOutputPanel, gpibOutputCtrl, "GPIB command '%s': %2.3fs after start of seq.\n", g->name, time_s);
				// sketchy optimization to speed program triggers...sorry!
				if (g->commandType == GPIB_LOOP_TRIGGER)
				{
					triggerLoop();
				}
				else if (g->commandType == GPIB_RECV_LOOP_TRIGGER)
				{
					recvTriggerLoop();
				}
				else
				{
					GPIB_transmitCommand (a->callingSequence, g, 0, 0);
				}
				duration_ms = TIMEBASE_FACTOR * (HARDWARE_ADWIN_displayPointsGenerated () - cycleNo);
				pprintf (gpibOutputPanel, gpibOutputCtrl, "[Duration: < %1.2f ms] %s\n", duration_ms, (duration_ms > g->maximumDuration_ms) ? "EXCEEDS MAXIMUM DURATION!!" : "OK.");
				nGPIBCommandsExecuted ++;
			}
		}
		if (!phaseFeedbackDone) {
			phaseFeedbackDone = (HARDWARE_ADWIN_feedback (a, callNo) == 0);
			callNo ++;
		}
		//ProcessSystemEvents ();
		ProcessDrawEvents ();
		// Sleep ((int)(1000*delay));
	}
/*	tprintf ("%d values", i);
	for (j = 0; j < 20; j++) {
		tprintf ("%d ", arr[j]);	
		
	}
*/	if (!interrupted) {
		HARDWARE_ADWIN_displayPointsGenerated ();
	}

	status = Get_Par (ADWIN_P1_PAR_ERRORCODE);
	if (status != 0) PostMessagePopupf ("ADWIN error!", "Process %d was not executed correctly.\n%s.",
						 ADWIN_PROCESS_DATA_OUT, HARDWARE_ADWIN_errorText (status, Get_Par (ADWIN_P1_PAR_CYCLE_NO)));
						
//	free (arr);
	
	return status;
}



int HARDWARE_ADWIN_startProcess (short process)
{
	int status;
	
	status = Start_Process (process);
	if (status == 255) {
		PostMessagePopupf ("Error", "ADWIN: Process %d could not be started. %s", 
				 process, Get_Last_Error_Text (Get_Last_Error ()));
		return -1;
	}
	return 0;
}



int HARDWARE_ADWIN_dataOutput (t_adwinData *a, float maxWaitingTime_s, int waitUntilOutputFinished, int displayMessage)
{
	clock_t startTime = 0;
	int timeout;
	int status;
	//int interrupted;
	
	
	if (a == NULL) return 0;
	OUTPUT_setMaxPointsGenerated (a->maxCycleNo);
	Stop_Process (ADWIN_PROCESS_DATA_OUT);
	
	timeout = 0;
	if (maxWaitingTime_s > 0) startTime = clock();
	// check if process is still running
	while (!timeout && ((status = Process_Status (ADWIN_PROCESS_DATA_OUT)) != 0)) {
		if (status == 255) {
			PostMessagePopupf ("Error", "ADWIN: Function 'Process_Status(%d) returned an error: %s", 
					 ADWIN_PROCESS_DATA_OUT, Get_Last_Error_Text (Get_Last_Error ()));
			return -1;
		}
		timeout = (maxWaitingTime_s > 0) && (timeStop_s (startTime) > maxWaitingTime_s);
//		interrupted = HARDWARE_SetGetInterruptedFlag (-1);
		Sleep(100);
	}
	if (!timeout) {
		if (HARDWARE_ADWIN_transmitData (a, displayMessage) != 0) return -1;
		if (HARDWARE_ADWIN_startProcess (ADWIN_PROCESS_DATA_OUT) != 0) return -1;
		if (waitUntilOutputFinished) {
			if (HARDWARE_ADWIN_waitUntilOutputFinished (a) != 0) return -1;
		}	
	}
	
	
	if (timeout) return ADWIN_ERR_TIMEOUT;
	
	return 0;
	
}


int CVICALLBACK HARDWARE_ADWIN_outputThreadFunction (void *functionData);






t_digitizeParameters *digitizeParameters_ADWIN (void)
{
	static t_digitizeParameters p;
	static int firstcall = 1;

	if (firstcall) {
		firstcall = 0;
		p.offsetCorrection = 0;
		p.timebase_50ns	= ADWIN_TIMEBASE_ANALOG;  // 
			// set max and min voltage for analog values
		p.minVoltage 	=  -10.0;
		p.maxVoltage 	=  10.0;
		p.Vpp 			= p.maxVoltage - p.minVoltage;
			// set max and min digital output values
		p.minDigital 	= 0;  //for PCI6713
		p.maxDigital 	= 65535;
		p.Dpp           = p.maxDigital - p.minDigital;
		// reset number of points 
		// (nPoints != 0) wouls enforce digitalization with n points
		// (not used for NI boards)
		p.nPoints       = 0;
	}

	return &p;	
}





int HARDWARE_ADWIN_startProcessDirectIO (void)
{
	int status;
	
	if (Process_Status (ADWIN_PROCESS_DIRECT_IO) == 0) {
//		tprintf ("ADWIN: Starting process %d.\n", ADWIN_PROCESS_DIRECT_IO);
		status = Start_Process (ADWIN_PROCESS_DIRECT_IO);
		if (status == 255) {
			PostMessagePopupf ("Error", "ADWIN: Process %d could not be started. %s", 
					 ADWIN_PROCESS_DIRECT_IO, Get_Last_Error_Text (Get_Last_Error ()));
			return -1;
		}
	}
	
	return 0;
	
}






int HARDWARE_ADWIN_waitForDirectIOFlag (float timeout_s)
{
	clock_t startTime = 0;
	int timeout;
	
	int status;

	timeout = 0;
	startTime = clock();
	do  {
		status = Get_Par (ADWIN_P2_PAR_outputFlag);
		if (status == 255) {
			tprintf ("ADWIN: 'Get_Par()' returned an error.  %s", 
					 Get_Last_Error_Text (Get_Last_Error ()));
			return -1;
		}
		if (status != 0) Sleep (10);
		if (timeout_s > 0) timeout = (timeStop_s (startTime) > timeout_s);
	}
	while ((status != 0) && (!timeout));
	if (timeout) return -1;
	return 0;	
}




int HARDWARE_ADWIN_sendDirectAnalogValue (int dacChannel, double analogValue)
{
	long value;
	
//	AnalogToDigital (analogValue, digitizeParameters_ADWIN());
	
	if ((dacChannel < 0) || (dacChannel >= N_DAC_CHANNELS)) return 0;
	value = AnalogToDigital (analogValue, digitizeParameters_ADWIN());
//	ADWIN_directIOValues[MAX_DIO_DEVICES+dacChannel] = value;
	//DebugPrintf ("value=%d\n", value);
	
	HARDWARE_ADWIN_startProcessDirectIO ();
	if (HARDWARE_ADWIN_waitForDirectIOFlag (1) != 0) return -1;

	if (Set_Par_Err (ADWIN_P2_PAR_outputValue, value) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputModule, config->analogBoardID[dacChannel / 8]) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputChannel, (dacChannel % 8)+1) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputFlag, ADWIN_P2_OUTPUT_SINGLE_CHANNEL_ANALOG) != 0) return -1;
	
	return 0;

}


	
	

int HARDWARE_ADWIN_sendDirectDigitalValue (int channel, short bit)
{
	//long value;
	
	//int shift;
	//int index;
	//unsigned long andVal;
	
	
	if ((channel < 0) || (channel >= N_TOTAL_DIO_CHANNELS)) return 0;
	
	HARDWARE_ADWIN_startProcessDirectIO ();
	if (HARDWARE_ADWIN_waitForDirectIOFlag (0.1) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputValue, bit) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputModule, config->digitalBoardID[channel / 32]) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputChannel, channel % 32) != 0) return -1;
	if (Set_Par_Err (ADWIN_P2_PAR_outputFlag, ADWIN_P2_OUTPUT_SINGLE_CHANNEL_DIGITAL) != 0) return -1;

	return 0;	
}






void HARDWARE_ADWIN_initOutCtrl (int panel, int control)
{
	outPanel = panel;
	outCtrl = control;
}


void HARDWARE_ADWIN_getSystemInfo (void)
{
	pprintf (outPanel, outCtrl, "Info:\n");
	
	pprintf (outPanel, outCtrl, "Memory:\n");
	pprintf (outPanel, outCtrl, "PM_LOCAL: %d kB\n", Free_Mem (1) / 1024);
	pprintf (outPanel, outCtrl, "EM_LOCAL: %d kB\n", Free_Mem (2) / 1024);
	pprintf (outPanel, outCtrl, "DM_LOCAL: %d kB\n", Free_Mem (3) / 1024);
	pprintf (outPanel, outCtrl, "DRAM_EXTERN: %d kB\n", Free_Mem (4) / 1024);
}




int HARDWARE_ADWIN_boot (int panel, int ctrl)
{
	int status;
	int error;
	char dateStr[100];
	char timeStr[100];
	
	pprintf (panel, ctrl, "Booting Adwin...    ");
	
	// Boot System
	ProcessDrawEvents ();
	status = Boot (adwinFilename, 0);
	if (status == 8000) {
		pprintf (panel, ctrl, "OK.\n");
		error = 0;
	}
	else {
		error = Get_Last_Error ();
		pprintf (panel, ctrl, "%s\n", Get_Last_Error_Text (error));
		goto ENDE;
	}

	// Load Process1: Output
	getDateAndTimeFromFile (FILENAME_PROCESS_OUTPUT1, dateStr, timeStr);
	
	pprintf (panel, ctrl, "Loading process \"%s\" (%s, %s)...    ", extractFilename (FILENAME_PROCESS_OUTPUT1), dateStr, timeStr);
	ProcessDrawEvents ();
	status = Load_Process (FILENAME_PROCESS_OUTPUT1);
	if (status == 1) {
		pprintf (panel, ctrl, "OK.\n");
		error = 0;
	}
	else {
		error = Get_Last_Error ();
		pprintf (panel, ctrl, "%s\n", Get_Last_Error_Text (error));
		goto ENDE;
	}
	
	// Load Process Digital IO
	getDateAndTimeFromFile (FILENAME_PROCESS_DIRECTIO, dateStr, timeStr);
	pprintf (panel, ctrl, "Loading process \"%s\" (%s, %s)...    ", extractFilename (FILENAME_PROCESS_DIRECTIO), dateStr, timeStr);
	ProcessDrawEvents ();
	status = Load_Process (FILENAME_PROCESS_DIRECTIO);
	if (status == 1) {
		pprintf (panel, ctrl, "OK.\n");
		error = 0;
	}
	else {
		error = Get_Last_Error ();
		pprintf (panel, ctrl, "%s\n", Get_Last_Error_Text (error));
		goto ENDE;
	}
	
	status = HARDWARE_ADWIN_startProcessDirectIO ();
	
ENDE:	
	return status;
}



int HARDWARE_ADWIN_configureOutput (void)
{
	return 0;	
}

void HARDWARE_ADWIN_test (void)
{
		
}


int HARDWARE_ADWIN_stopDataOutput (void)
{
	Set_Par_Err (ADWIN_P1_PAR_STOPFLAG, 1);	
	return 0;	
}



