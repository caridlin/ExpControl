#include <formatio.h>
#include <userint.h>
#include "UIR_ExperimentControl.h"


/*============================================================================
 *
 *                   E x p e r i m e n t   C o n t r o l  
 *
 *----------------------------------------------------------------------------
 *    
 *  Copyright (c) Stefan Kuhr, 1999 - 2004
 *					
 *----------------------------------------------------------------------------
 *                                                                            
 * Title:       CONTROL_DataStructure.c       
 *
 * Purpose:     basic functions to initialize, free etc. 
 *				the data structures										
 *                                                                            
 *============================================================================*/

#include <utility.h>    
//#include "nidaqcns.h"  
#include "tools.h"

#include "CONTROL_DataStructure.h"					
#include "CONTROL_HARDWARE_adwin.h"					
#include "CONTROL_GUI_waveform.h"
#include "CONTROL_LoadSave.h" 
#include "CONTROL_GUI_ExtDev.h"



const unsigned arr_VAL_Time[4] = 
    {VAL_s, VAL_ms, VAL_us, VAL_50ns};
    
const char *arr_VAL_TimeUnits[4] = 
    {"s", "ms", "µs", "ns"};
    

// global variable for program configuration
t_config *config = NULL;
    



const char* WFM_STEP_TYPE_STR[] = {"none" , "linear" , "list"};
//const char WFM_STEP_TYPE_NONE[]      = "none";
//const char WFM_STEP_TYPE_TO_LINEAR[] = "linear";
//const char WFM_STEP_TYPE_LIST[]      = "list";


const char UIR_File[] = "UIR_ExperimentControl.uir";
const char UIR_common[] = "UIR_common.uir";


//=======================================================================
//   returns timebase (in units of 50 ns) 
//   'timebase' is the parameter used in
//   DIG_Block_PG_Config (...) and WFM_ClockRate (...)
//=======================================================================


//static unsigned currentDataID = 1;


//=======================================================================
//   get name of analog channel
//   	0 < ch < 16: "DAC0", "DAC1"...
//      ch >= 16   : "GPIB0_CH1", "GPIB0_CH2", ...
//=======================================================================
char *str_AnalogChannelNames (int ch)
{
	static char help[20];

	if ((ch < N_DAC_CHANNELS) && (ch >= 0)) sprintf (help, "DAC%d", ch);
	else help[0] = 0;
	return help;
}




//=======================================================================
//   get name of digital channel
//   	0 < ch < 32: "A0", "A1", etc.
//=======================================================================
char *str_ChannelNames (int ch)
{
	static char chName[3] = "A0";
	
    if ((ch < 0) || (ch >= N_TOTAL_DIO_CHANNELS)) return "";
	chName[0] = 'A' + (char)(ch / 8);
	chName[1] = '0' + (char)(ch % 8);
	return chName;
}







//=======================================================================
//   
//    't_config': functions 
//  
//=======================================================================


/*
// ------------------------------------------
//    returns global configuration variable
// ------------------------------------------
t_config *config(void)
{
    return &globalConfig;
}
*/


// ==========================================
//     initialize hardware board parameters
// ==========================================
void CONFIG_initBoards (t_config *c)
{
	int i;

	// number of boards = 0
	c->nAnalogBoards    = 0;
	c->nDigitalBoards   = 0;
	
	// reset IDs
	
	for (i = 0; i < MAX_DIO_DEVICES; i++) c->digitalBoardID[i] = -1;
	for (i = 0; i < MAX_AO_DEVICES; i++) c->analogBoardID[i]  = -1;
   
    // init list of boards
    c->listOfBoards = ListCreate(sizeof (int));
}



// ==========================================
//     initialize data type 't_config'
// ==========================================
void CONFIG_init (t_config *c)
{
	int i;
	
	
	
	// default path for "Load sequence" and "save sequence"
	c->defaultPath = "";
	
	// check if data acquisition 
	// should be started
//	c->START_ACQUISITION = 0;
	c->CHECK_DISABLED 	 = 0;
	c->DEBUGMODE   	 = 0;
//	c->START_SESSIONMANAGER = 0;
	c->REMOTE 			= 0;
//	c->FAKEDATA         = 0;

#ifdef SMANAGER
	c->enableTCP		 = 1;
#else
	c->enableTCP		 = 0;
#endif
	c->launchDataSocketServer = 0;

	c->enableGPIB		 = 1;
	c->displayAbsoluteTime = 1;

//	c->maxDigitalBufferSize = 50 * MB;  // maximum buffer size in bytes
//	c->maxAnalogBufferSize  = 50 * MB;  // maximum buffer size in bytes

	c->loadAdwinBoardsFromConfig = 1; // change here if using hard coded or dynamically loaded adwin config
	
	c->autoSaveSequencesAfterEachStart = 0;
	c->autoSaveSequencesPath[0] = 0;
	c->dataPathToday[0] = 0;
	c->createFilenameForImages = 0;
	c->imageFilenameExtensionPath[0] = 0;
	c->idleSequenceDataPath[0] = 0;
	c->pifocFocusVoltageLogFilePath[0] = 0;
	c->pifocVoltageChannel = 1;
	c->lastPifocFocusVoltage = 0;
	
	c->pathMagickConvert[0] = 0;
	
	c->startNr = 0;
	
	c->buttonWidth  = 80;
	c->buttonHeight = 13;
	
	c->gpibBoardIndex = 1;
	CONFIG_initBoards(c);
	
	c->listOfSequences = ListCreate(sizeof (t_sequence*));
	c->ignorePresenceOfGpibLeCroyDevices = 0;
	c->nGpibDevices    = MAX_GPIBDEVICES;
	c->listOfGpibDevices = ListCreate(sizeof (t_gpibDevice*));
	c->activeSequenceNr = -1;
	c->idleSequenceNr = -1;
    
	c->watchdogActivate   = 0;
	c->watchdogAutoRepeat = 0;
	c->invertDigitalOutput = 0;
	

	c->splitterVPos = 0;
	c->splitterHPos = 0;
	c->hardwareType = HARDWARE_TYPE_ADWIN;//HARDWARE_TYPE_NATINST;
	c->hardwareTypeBooted = -1;
	
	c->panelBoundsMain = MakeRect (0,0,0,0);
	c->panelBoundsMain2 = MakeRect (0,0,0,0);
	c->panelMain2IsZoomed = 0;
	c->panelPosSimulate.x = -1;
	c->panelPosSimulate.y = -1;
	c->useTwoScreens = 0;
	
	for (i = 0; i < N_TABLE_CONFIGS; i++) TABLECONFIG_init (&c->tableConfig[i], i+1);
}



void CONFIG_free (t_config *c) 
{
	ListDispose (c->listOfSequences);
	GPIBDEVICE_deleteAll (c);
	ListDispose (c->listOfGpibDevices);
}





//=======================================================================
//
// 	  compare two sets of digitize parameters
//
//=======================================================================
int DIGITIZEPARAMETERS_compare (t_digitizeParameters *p1, t_digitizeParameters *p2)
{
	return memcmp (p1, p2, sizeof (t_digitizeParameters));
}






void POINT_PlotLine (int panel,int control,t_point *p ,t_point * pPrime , int couleur) {
	PlotLine (panel,control, p->timeStart_ns*1000,p->valueStart ,
			  pPrime->timeStart_ns*1000, pPrime->valueStart, VAL_DK_CYAN);

}




void POINT_init (t_point *p)
{
	p->varyTime = 0;
	p->timeStart_ns = 0;
	p->timeIncrement_ns = 0.0;
	p->varyValue = 0;
	p->valueStart = 0;
	p->valueIncrement = 0;
	p->stepRepetitions = 1;
	p->thisTime_ns = 0;
	p->thisValue = 0;
}


int POINT_isEqual  (t_point *p1, t_point *p2)
{
	return (p1->varyTime       == p2->varyTime) 
		  && (p1->timeStart_ns      == p2->timeStart_ns)
		  && (p1->timeIncrement_ns  == p2->timeIncrement_ns)
		  && (p1->varyValue		 == p2->varyValue)
		  && (p1->valueStart     == p2->valueStart)
		  && (p1->valueIncrement == p2->valueIncrement);
		  
}


/*************************************************************************** 

	t_step_param: Funktionen

****************************************************************************/                 

void STEP_PARAM_free(t_step_param* p) {
	if (p == NULL) return;
	if (p->list == NULL) return;

	free(p->list);
	p->list = NULL;
}

void STEP_PARAM_copy(t_step_param* dest, t_step_param* src) {
	dest->stepType = src->stepType;
	dest->from = src->from;
	dest->to = src->to;
	dest->listSize = src->listSize;
	dest->list = (double*)calloc(src->listSize,sizeof(double));
	memcpy (dest->list, src->list, src->listSize);
}

void STEP_PARAM_init(t_step_param* p, int type) {
	p->stepType = type;
	
	p->from = 0;
	p->to = 0;
	p->list = NULL;   // list is allocated on demand

}

void STEP_PARAM_getListString(char* result,t_step_param* p) {
	if (p->list == NULL || p->listSize == 0) {
		strcpy(result,"NULL");
		return;
	}
	strcpy(result,"Array");
	//implodeDoubleArray(result,p->list,p->listSize);
	//return strnewcopy(tmp,ret);
}



/***************************************************************************

     t_waveform: Funktionen
            
****************************************************************************/


extern const double WFM_ADDR_calibrationParametersDefault[WFM_ADDR_N_CALIBRATION_PARAMETERS];

extern const double WFM_ADDR_trajParametersDefault[WFM_ADDR_N_TRAJ_PARAMETERS];


// Waveform mit Default-Werten füllen
void WFM_init (t_waveform *wfm, int type)
{
   
	int i;
	
	wfm->name[0]     = 0;
	wfm->type        = type;
	wfm->hide        = 0;
	wfm->filename    = NULL;
	wfm->channel = WFM_CHANNEL_NONE;

	wfm->uScaleVary  = 0;
	wfm->offsetFrom	 = 0;
	wfm->offsetTo    = 0;
	wfm->offsetVary  = 0;
	wfm->durationVary =0;
	wfm->timebaseStretch = 1;

    wfm->strValueName [0] = 0;
    wfm->strValueUnits[0] = 0;
	wfm->transferFunction = 0;
	wfm->uList    = NULL;
	wfm->nPoints  = 0;
	wfm->points   = NULL;
	if ((type == WFM_TYPE_FILE) || (type == WFM_TYPE_HOLD)) {
		wfm->durationFrom_50ns = VAL_ms;
		wfm->durationTo_50ns   = VAL_ms; 
		wfm->nPoints  = 0;
		wfm->points   = NULL;
	    wfm->uScaleFrom = 10.0;	  
	    wfm->uScaleTo = 10.0;	  

	}
	else {
	    wfm->uScaleFrom = 0.0;	  
	    wfm->uScaleTo = 0.0;	  
		wfm->durationFrom_50ns = 0;
		wfm->durationTo_50ns   = 0; 
		wfm->nPoints  = 0;
		wfm->points   = NULL;
	}
	wfm->stepFrom = 0.0;
	wfm->stepTo   = 0.0;
	wfm->stepRep  = 1;
	wfm->stepChangeMode =  SWEEP_CHANGEMODE_REPETITION;
	wfm->stepAlternate = 0;
	
	wfm->add = WFM_ID_UNCHANGED;

//	wfm->DIG_Timebase = 0;
	wfm->DIG_NValues  = 0;
	wfm->DIG_values   = NULL;
	wfm->DIG_repeat   = NULL;
	
	wfm->digitize     = 0;
	wfm->digParameters.minVoltage      = -1.0E89;
	wfm->menuID = 0;
	
	wfm->uScaleThisRepetition = 0;
	wfm->offsetThisRepetition = 0;  
	wfm->durationThisRepetition_ns = 0;
	wfm->durationThisRepetition_50ns = 0;
	
	
	wfm->functionID = 0;
	wfm->functionParameters = NULL;
	wfm->functionParametersThisRepetition = NULL;
	wfm->functionReverseTime = 0;
	
	
	
	wfm->addrLinkWfm = 0;
	wfm->addrType = WFM_ADDR_TYPE_SINGLESITE;
	wfm->stepAddrMatrix = 0;
	wfm->addrMatricesPerRepetition = 1;
	wfm->addrMatrix = NULL;
	for (i=0; i < WFM_ADDR_MATRIX_NUM; i++){
		wfm->addDisorder[i]=0;
		wfm->disorderSeed[i]=1;
		wfm->disorderTyp[i]=0;
	}
	
	for (i = 0; i < WFM_ADDR_N_CALIBRATION_PARAMETERS; i++) {
		wfm->addrCalibration[i] = WFM_ADDR_calibrationParametersDefault[i];
		wfm->addrCalibrationStep[i] = 0;
		wfm->addrCalibrationStepTo[i] = 0;
		wfm->addrCalibrationThisRepetition[i] = 0;
	}
	for (i = 0; i < WFM_ADDR_N_TRAJ_PARAMETERS; i++) {
		wfm->addrTrajectParams[i] = WFM_ADDR_trajParametersDefault[i];
	}

	wfm->addrCenter = MakePoint (256, 256);
	wfm->addrTestFilename = NULL;
	wfm->addrTestFilename2 = NULL;
	wfm->addrEnablePhaseFeedback = 0;
	wfm->addrEnablePhaseFeedback2 = 0;
	wfm->addrCamera = 4656;
	//wfm->addrOffsetThisRepetition[0] = 0;
	//wfm->addrOffsetThisRepetition[1] = 0;
	wfm->addrAddLastOffset = 0;
	wfm->addrAddLastOffsetNotAtFirst = 0;
	wfm->addrImageSize = 100;
	wfm->addrDMDimage = NULL;
	
	wfm->isActive = 0;
	
	wfm->positionInTable = -1;
	wfm->positionInList = 1;
	wfm->usedBy[0] = 0;
	wfm->asked = 0;

}


t_waveform *WFM_new (t_sequence *seq)
{
	t_waveform *new;

    new = (t_waveform *) malloc (sizeof (t_waveform));
	if (seq != 0) ListInsertItem (seq->lWaveforms, &new, END_OF_LIST);
    return new;
}


// alle Inhalte der Wellenform freigeben
void WFM_free (t_waveform *w)
{
	int i, index;
	
	free (w->filename);
	free (w->DIG_values);
	free (w->DIG_repeat);
	free (w->uList);
	free (w->points);
	w->nPoints = 0;
	w->uList   = NULL;
	w->points  = NULL;

	if (w->functionParameters != NULL) {
		for (index = FUNCTION_nFunctions()-1; index >= 0; index--) {
			if (w->functionParameters[index] != NULL) {   
				for (i = 0; i < FUNCTION_ptr (index+1)->nParameters; i++) {
					STEP_PARAM_free(&(w->functionParameters[index][i]));
				}
				//STEP_PARAM_free(w->functionParameters[i]);
				free (w->functionParameters[index]);
			}
			
		}
	}
	
	/*
		
	f = FUNCTION_ptr (wfm->functionID);
	if (f == NULL) return;
	
	nFunc = FUNCTION_nFunctions();
	if (wfm->functionParameters == NULL) {
		// no arrays allocated yet
		wfm->functionParameters = (t_step_param **) calloc (nFunc, sizeof(t_step_param*));
	}
	
	index = wfm->functionID-1;
	for (index = 0; index < nFunc; index++) {
		if (wfm->functionParameters[index] == NULL) {
			wfm->functionParameters[index] = (t_step_param *) calloc (FUNCTION_ptr (index+1)->nParameters,sizeof(t_step_param));
			for (i = 0; i < FUNCTION_ptr (index+1)->nParameters; i++) {
				STEP_PARAM_init(&wfm->functionParameters[index][i], WFM_STEP_TYPE_NONE);
				wfm->functionParameters[index][i].from = f->defaultValues[i];
			}
		}
	}*/
	
	
	free (w->functionParameters);
	w->functionParameters = NULL;
	free (w->addrMatrix);
	w->addrMatrix = 0;
	free (w->addrTestFilename);
	w->addrTestFilename = NULL;
	free (w->addrTestFilename2);
	w->addrTestFilename2 = NULL;
	DMDIMAGE_free (w->addrDMDimage);
	w->addrDMDimage = NULL;
	
	free(w->functionParametersThisRepetition);
	w->functionParametersThisRepetition = NULL;
}



// FIXME
void WFM_duplicate (t_waveform *dest, t_waveform *source)
{
	int i;
	int index;
	
	// copy all data
 	memcpy (dest, source, sizeof (t_waveform));
    
    // assign new name
    dest->filename = NULL;
    dest->filename = strnewcopy (dest->filename, source->filename);
	
	// copy all functions
	if (source->uList != NULL) {
	    dest->uList	  = (double *) malloc (dest->nPoints * sizeof(double)); 
	    memcpy (dest->uList, source->uList, dest->nPoints * sizeof(double));
	}
	if (source->points != NULL) {
	    dest->points  = (t_point *) malloc (dest->nPoints * sizeof(t_point )); 
	    memcpy (dest->points, source->points, dest->nPoints * sizeof(t_point ));
	}
	
	dest->functionParameters = NULL;
	if (source->functionParameters != NULL) {
		dest->functionParameters = (t_step_param **) calloc (FUNCTION_nFunctions(), sizeof(t_step_param*));          
		for (index = FUNCTION_nFunctions()-1; index >= 0; index--) {
			if (source->functionParameters[index] != NULL) {   
				dest->functionParameters[index] = (t_step_param *) calloc (FUNCTION_ptr (index+1)->nParameters,sizeof(t_step_param));
				for (i = 0; i < FUNCTION_ptr (index+1)->nParameters; i++) {
					STEP_PARAM_copy(&(dest->functionParameters[index][i]),&(source->functionParameters[index][i]));
				}
			}
			
		}
	}
	/*
	i = dest->functionID -1;
	if ((source->functionParameters != NULL) && (i >= 0)) {
		WFM_FUNC_allocateMemory (dest);
		memcpy (dest->functionParameters[i], source->functionParameters[i], 
				sizeof(t_step_param)*FUNCTION_nParameters (dest->functionID));
	}*/
	if (source->addrMatrix != NULL) {
		WFM_ADDR_allocateMatrix (dest);
		memcpy (dest->addrMatrix, source->addrMatrix, WFM_ADDR_MATRIX_NUM * WFM_ADDR_MATRIXDIM * sizeof(unsigned int));
	}
	
	dest->addrTestFilename = strnewcopy (NULL, source->addrTestFilename);
	dest->addrTestFilename2 = strnewcopy (NULL, source->addrTestFilename2);
	dest->addrDMDimage = NULL; // DMD image not copied

//	dest->DIG_Timebase = 0;
	dest->DIG_NValues  = 0;
	dest->DIG_values   = NULL;
	dest->DIG_repeat   = NULL;

	dest->functionParametersThisRepetition = NULL;
}


void WFM_deleteWithReferences (t_sequence *seq, int nr)
{
    t_digitalBlock *d;
    t_gpibCommand *g;
	t_waveform *w;
	int n;
	int i;
	int ch;
	
	WFM_delete (seq, nr);
	// -----------------------------------------
    //    change all links to this waveform
	// -----------------------------------------
	n = ListNumItems (seq->lDigitalBlocks);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &d, i);
		d = DIGITALBLOCK_ptr(seq, i);
        for (ch = 0; ch < N_DAC_CHANNELS; ch++) { 
            if (d->waveforms[ch] == nr) {
                d->waveforms[ch] = WFM_ID_UNCHANGED;
                
            }
            if (d->waveforms[ch] > nr)
                d->waveforms[ch] --;
        }
    }
	n = ListNumItems (seq->lGpibCommands);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lGpibCommands, &g, i);
        if (g->waveform == nr) g->waveform = WFM_ID_UNCHANGED;
        if (g->waveform  > nr) g->waveform--;
    }

	n = ListNumItems (seq->lWaveforms);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &w, i);
        if (w->addrLinkWfm == nr) w->addrLinkWfm = WFM_ID_UNCHANGED;
        if (w->addrLinkWfm > nr) w->addrLinkWfm--;
		
    }
	
	
}


void WFM_delete (t_sequence *seq, int nr)
{
	t_waveform *old;
	
	ListRemoveItem (seq->lWaveforms, &old, nr);
	WFM_free (old);
	free (old);
}

//=======================================================================
//
// 	   returns pointer to waveform "HOLD"
//
//=======================================================================
t_waveform *WFM_holdWaveform (void) 
{
    static t_waveform hold;
    static int firstcall = 1;
    static long DIG_VALUES[1] = { 0 };
    static unsigned long DIG_REPEAT[1] = { 1 };
    
    if (firstcall) {
        firstcall = 0;
        WFM_init (&hold, WFM_TYPE_HOLD);
        strcpy (hold.name, strHold);
        hold.durationFrom_50ns   = ULONG_MAX;
        hold.DIG_values = DIG_VALUES;
        hold.DIG_repeat = DIG_REPEAT;
        hold.DIG_NValues = 1;
    }
    return &hold;
}

//=======================================================================
//
// 	   returns pointer to waveform "CONSTANT VOLTAGE"
//
//=======================================================================
t_waveform *WFM_constWaveform (void) 
{
    static t_waveform c;
    static int firstcall = 1;
    static long DIG_VALUES[1] = { 0 };
    static unsigned long DIG_REPEAT[1] = { 1 };
    
    if (firstcall) {
        firstcall = 0;
        WFM_init (&c, WFM_TYPE_CONSTVOLTAGE);
        strcpy (c.name, strConstVoltage);
        c.durationFrom_50ns   = ULONG_MAX;
        c.DIG_values = DIG_VALUES;
        c.DIG_repeat = DIG_REPEAT;
        c.DIG_NValues = 1;
    }
    return &c;
}





//=======================================================================
//
// 	   returns pointer to waveform with number 'wfmID'
//
//=======================================================================
t_waveform *WFM_ptr (t_sequence *seq, int wfmID)
{
    t_waveform *w;
    
    if (seq == NULL) return NULL;
    switch (wfmID) {
    	case WFM_ID_UNCHANGED: return NULL;
    	case WFM_ID_CONSTVOLTAGE: return WFM_constWaveform();
    	case WFM_ID_HOLD: return WFM_holdWaveform ();
    }
	if ((wfmID > (int)ListNumItems (seq->lWaveforms)) || (wfmID < 1)) return NULL;
	ListGetItem (seq->lWaveforms, &w, wfmID);
	return w;
}



//=======================================================================
//
//    check if waveforms are equal
//
//=======================================================================
int WFM_isEqual (t_waveform *w1, t_waveform *w2)
{
    int i,j;
	int index, n;
    
    if ((w1 == NULL) || (w2 == NULL)) return 0;
    if (w1->type != w2 ->type) return 0;
	
	if (strcmp (w1->name, w2->name) != 0) return 0;
	if (w1->add != w2->add) return 0;
    switch (w1->type) {
        case WFM_TYPE_POINTS:
            if (w1->nPoints != w2 ->nPoints) return 0;
            for (i=0; i < w1->nPoints; i++) 
             	if (!POINT_isEqual (&w1->points[i], &w2->points[i])) return 0;
            return 1;
        case WFM_TYPE_FILE:
//			if (strcmp (w1->filename, w2->filename) != 0) return 0;
			if (w1->uScaleFrom != w2->uScaleFrom) return 0;
			if (w1->uScaleTo != w2->uScaleTo) return 0;
			if (w1->uScaleVary != w2->uScaleVary) return 0;
			if (w1->offsetFrom != w2->offsetFrom) return 0;
			if (w1->offsetTo != w2->offsetTo) return 0;
			if (w1->offsetVary != w2->offsetVary) return 0;
			if (w1->durationFrom_50ns!= w2->durationFrom_50ns) return 0;
			if (w1->durationTo_50ns!= w2->durationTo_50ns) return 0;
			if (w1->durationVary != w2->durationVary) return 0;
            for (i=0; i < w1->nPoints; i++) 
             	if (w1->uList[i] != w2->uList[i]) return 0;
            return 1;
        case WFM_TYPE_STEP:
			if (w1->stepFrom != w2->stepFrom) return 0;
			if (w1->stepTo != w2->stepTo) return 0;
			if (w1->stepRep != w2->stepRep) return 0;
			if (w1->stepChangeMode != w2->stepChangeMode) return 0;
			if (w1->stepAlternate != w2->stepAlternate) return 0;
			return 1;
		case WFM_TYPE_FUNCTION:
			if (w1->functionID != w2->functionID) return 0;
			if (w1->functionParameters != NULL) {
				index = w1->functionID-1;
				n = FUNCTION_nParameters (w1->functionID);
				//if (memcmp (w1->functionParameters[index], w2->functionParameters[index],
				//	sizeof(double)*n) != 0) return 0;
				for (i = 0; i < n; i++) {
					switch (w1->functionParameters[index][i].stepType) {
						case WFM_STEP_TYPE_TO_LINEAR:
							if (w1->functionParameters[index][i].to != w2->functionParameters[index][i].to) return 0;
							break;
						case WFM_STEP_TYPE_NONE:
							break;
						case WFM_STEP_TYPE_LIST:
							if (w1->functionParameters[index][i].listSize != w2->functionParameters[index][i].listSize) {
								return 0;	
							}
							for (j=0; j < w1->functionParameters[index][i].listSize; j++) {
								if ( w1->functionParameters[index][i].list[j] !=  w2->functionParameters[index][i].list[j]) return 0;	
							}
							break;
						default:
							return 0;
					}
				}
			}
			return 1;
		case WFM_TYPE_ADDRESSING:
			if (w1->addrCenter.x !=  w2->addrCenter.x) return 0;
			if (w1->addrCenter.y !=  w2->addrCenter.y) return 0;
			if (w1->addrMatrix == NULL && w2->addrMatrix != NULL) return 0;
			if (w1->addrMatrix != NULL && w2->addrMatrix == NULL) return 0; 
			if (w1->addrMatrix != NULL && w2->addrMatrix != NULL && memcmp (w1->addrMatrix, w2->addrMatrix, sizeof(unsigned int)*WFM_ADDR_MATRIXDIM*WFM_ADDR_MATRIX_NUM) != 0) return 0;
			if (memcmp (w1->addrCalibration, w2->addrCalibration, sizeof(double)*WFM_ADDR_N_CALIBRATION_PARAMETERS) != 0) return 0;
			return 1;
			
       default: return 1;
    }
}    





int WFM_nameExistsN (t_sequence *seq, const char *name, int startSearch, int reverseSearch)
{
	t_waveform *wfm;
	int n;
	int i;
	
	
	if (reverseSearch) {
		for (i = startSearch; i > 0; i--) {
			ListGetItem (seq->lWaveforms, &wfm, i);
			if (strcmp (wfm->name, name) == 0) return i;
		}
	}
	else {
		n = ListNumItems (seq->lWaveforms);
		for (i = startSearch; i <= n; i++) {
			ListGetItem (seq->lWaveforms, &wfm, i);
			if (strcmp (wfm->name, name) == 0) return i;
		}
	}
	return 0;
}






int WFM_checkForDuplicateNameAndRename (t_sequence *seq, char *name)
{
	//int i;
	int suffix;
	//t_waveform *wfm;
	int changed;
	
	changed = 0;
	
	suffix = 2;
	while (WFM_nameExistsN (seq, name, 1, 0)) {
		changeNameSuffix (name, MAX_WFM_NAME_LEN, intToStr (suffix));
		suffix ++;
		changed = 1;
	}
	return changed;
}



void WFM_checkForDuplicateNames (t_sequence *seq)
{
	int i;
	int suffix;
	t_waveform *wfm, *w2;
	int z, n;
	
	i = 0;
	n = ListNumItems (seq->lWaveforms);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &wfm, i);
		suffix = 2; 
		while ((z = WFM_nameExistsN (seq, wfm->name, i+1, 0))) {
			ListGetItem (seq->lWaveforms, &w2, z);
			changeNameSuffix (w2->name, MAX_WFM_NAME_LEN, intToStr (suffix));
			suffix ++;
		}
	}
}



//=======================================================================
//
//    finds waveform with name 'name'
//
//=======================================================================
int WFM_nrFromName (t_sequence *seq, const char *name, int searchEnd)
{
    int i;
    
    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
    	if (i > searchEnd) return 0;
    	if (strcmp (name, WFM_ptr (seq, i)->name) == 0) return i;
    }
    return 0;
}



//=======================================================================
//
//    finds waveform with name 'name'
//
//=======================================================================
t_waveform *WFM_ptrFromName (t_sequence *seq, const char *name)
{
    int i;
    t_waveform *wfm;
    
    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
    	wfm = WFM_ptr(seq, i);
    	if (strcmp (name, wfm->name) == 0) return wfm;
    }
    return NULL;
}




int WFM_hasVariableParameters(t_waveform *wfm)
{
    int i, hasVarVoltage;
	int n, index;
    
    if (wfm == NULL) 				  return 0;

    switch (wfm->type) {
    	case WFM_TYPE_POINTS:
    		if (wfm->points == NULL) return 0;
		    for (i = 0; i < wfm->nPoints; i++) {
		    	if ((wfm->points[i].varyTime) && (wfm->points[i].timeIncrement_ns != 0.0)) return 1;
		    	if ((wfm->points[i].varyValue) && (wfm->points[i].valueIncrement != 0.0)) return 1;
		    }
		    return 0;
		case WFM_TYPE_FILE:
			return (wfm->uScaleVary && (wfm->uScaleFrom != wfm->uScaleTo))
				|| (wfm->offsetVary && (wfm->offsetFrom != wfm->offsetTo))
				|| (wfm->durationVary && (wfm->durationFrom_50ns != wfm->durationTo_50ns));
		case WFM_TYPE_STEP:
			hasVarVoltage = (wfm->stepFrom != wfm->stepTo);
			return hasVarVoltage;
		case WFM_TYPE_FUNCTION:
			n = WFM_nFuncParams (wfm);
			index = wfm->functionID-1;
			if (index < 0) return 0;
			for (i = 0; i < n; i++) {
				if (wfm->functionParameters[index][i].stepType != WFM_STEP_TYPE_NONE) {
					if (wfm->functionParameters[index][i].stepType == WFM_STEP_TYPE_TO_LINEAR && wfm->functionParameters[index][i].from != wfm->functionParameters[index][i].to) return 1;
					if (wfm->functionParameters[index][i].stepType == WFM_STEP_TYPE_LIST && wfm->functionParameters[index][i].listSize > 1) return 1;
				}
			}
			return (wfm->durationVary && (wfm->durationFrom_50ns != wfm->durationTo_50ns));
		case WFM_TYPE_ADDRESSING:
			for (i=0; i < WFM_ADDR_N_CALIBRATION_PARAMETERS; i++) {
				if (wfm->addrCalibrationStep[i]) {
					return 1;
				}
			}
			return 0;//(wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi1] || wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi2]);
		default:
			return 0;
	}
	
}


int WFM_nrFromPtr (t_sequence *seq, t_waveform *wfm) 
{
    unsigned int i;
    
    if (wfm == NULL) return WFM_ID_UNCHANGED;
    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
    	if (wfm == WFM_ptr(seq, i)) return i;
    }
    return WFM_ID_UNCHANGED;
}



int WFM_nrFromMenuID (t_sequence *seq, int menuID)
{
	unsigned int i;
	t_waveform *wfm;
					
	for (i = ListNumItems (seq->lWaveforms); i > 0; i--) {
		ListGetItem (seq->lWaveforms, &wfm, i);
		if (wfm->menuID == menuID) return i;
	}
    return WFM_ID_UNCHANGED;
}


void WFM_addPositionInList (t_sequence *seq)
{
	int i;
	t_waveform *w;
	int n;
	
	n = ListNumItems (seq->lWaveforms);
		
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &w, i);
		w->positionInList = i;
	}
}



int WFM_findIdentical (t_sequence *seq, t_waveform *wfm)
{
	int i;
	t_waveform *w;
		
	for (i = ListNumItems (seq->lWaveforms); i > 0; i--) {
		ListGetItem (seq->lWaveforms, &w, i);
		if (WFM_isEqual (w, wfm)) return i;
	}
	
	return WFM_ID_UNCHANGED;
}


int WFM_nameExists (t_sequence *seq, char *name)
{
    int i;
    t_waveform *wfm;
	
    for (i = ListNumItems (seq->lWaveforms); i > 0 ; i--) 
    {
        ListGetItem (seq->lWaveforms, &wfm, i);
		if (strcmp (wfm->name, name) == 0) return i;
   	}
   	return 0;
}


void WFM_checkAllIfActive (t_sequence *seq)
{
    t_waveform *wfm;
	t_digitalBlock *d;
	t_gpibCommand *g;
    int i, ch;

	for (i = ListNumItems (seq->lWaveforms); i >= 1; i--) {
	    ListGetItem (seq->lWaveforms, &wfm, i);
		wfm->isActive = 0;
	}
    
    for (i = ListNumItems (seq->lDigitalBlocks); i >= 1; i--) {
	    ListGetItem (seq->lDigitalBlocks, &d, i);
		if (!d->disable) {
	        for (ch = 0; ch < N_DAC_CHANNELS; ch++) 
	        	if ((d->waveforms[ch] > 0) && (d->waveforms[ch] <= ListNumItems (seq->lWaveforms))) {
	        		wfm = WFM_ptr (seq, d->waveforms[ch]);
					if (wfm != NULL) wfm->isActive = 2;
				}
		}
    }
	
    for (i = ListNumItems (seq->lGpibCommands); i >= 1; i--) {
	    ListGetItem (seq->lGpibCommands, &g, i);
		if (g->commandType == GPIB_COMMANDTYPE_ARBWFM) {
			wfm = WFM_ptr(seq, g->waveform);
			if (wfm!= NULL) wfm->isActive = 1;
		}
			
    }
	
}




void WFM_FUNC_allocateMemory (t_waveform *wfm)
{
	t_function *f;
	int nFunc;
	int index;
	int i;
	
	f = FUNCTION_ptr (wfm->functionID);
	if (f == NULL) return;
	
	nFunc = FUNCTION_nFunctions();
	if (wfm->functionParameters == NULL) {
		// no arrays allocated yet
		wfm->functionParameters = (t_step_param **) calloc (nFunc, sizeof(t_step_param*));
	}
	
	index = wfm->functionID-1;
	for (index = 0; index < nFunc; index++) {
		if (wfm->functionParameters[index] == NULL) {
			wfm->functionParameters[index] = (t_step_param *) calloc (FUNCTION_ptr (index+1)->nParameters,sizeof(t_step_param));
			for (i = 0; i < FUNCTION_ptr (index+1)->nParameters; i++) {
				STEP_PARAM_init(&(wfm->functionParameters[index][i]), WFM_STEP_TYPE_NONE);
				wfm->functionParameters[index][i].from = f->defaultValues[i];
			}
		}
	}
}
	


int WFM_nFuncParams (t_waveform *wfm)
{
	return FUNCTION_nParameters (wfm->functionID);
}



char *WFM_name (t_sequence *seq, int nr)
{
	t_waveform *wfm;
	
	wfm = WFM_ptr (seq, nr);
	if (wfm != NULL) return wfm->name;
	else return "";
	
}


void WFM_addChannels (t_sequence *seq)
{
	t_digitalBlock *b;
	int n, i, ch;
	t_waveform *wfm;
	t_gpibCommand *g;
	
	n = ListNumItems (seq->lDigitalBlocks);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
			if (b->waveforms[ch] > 0) {
				wfm = WFM_ptr (seq, b->waveforms[ch]);
				if (wfm->channel == WFM_CHANNEL_NONE) wfm->channel = ch;
				else if (wfm->channel != ch) wfm->channel = WFM_CHANNEL_ALL;
			}
		}
	}
	
	n = ListNumItems (seq->lGpibCommands);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lGpibCommands, &g, i);
		if (g->commandType == GPIB_COMMANDTYPE_ARBWFM) {
			wfm = WFM_ptr (seq, g->waveform);
			if (wfm != NULL) {
				if (wfm->channel == WFM_CHANNEL_NONE) wfm->channel = WFM_CHANNEL_GPIB;
				else if (wfm->channel != WFM_CHANNEL_GPIB) wfm->channel = WFM_CHANNEL_ALL;   
			}
		}
	}
}


char *WFM_typeStr (t_waveform *wfm)
{
	switch (wfm->type) {
		case WFM_TYPE_POINTS:
			return "POINTS";
		case WFM_TYPE_FILE:
			return "FILE";
		case WFM_TYPE_STEP:
			return "STEP";
		case WFM_TYPE_HOLD:
			return "HOLD";
		case WFM_TYPE_CONSTVOLTAGE:
			return "CONST";
		case WFM_TYPE_FUNCTION:
			return "FUNCT";
		case WFM_TYPE_ADDRESSING:
			return "ADDR";
	}
	return "";
}





void WFM_fillInfos (t_sequence *seq)
{
	t_waveform *w;
	int i, n;
	t_digitalBlock *b;
	char addStr[100];
	int ch;
	t_gpibCommand *g;
	
	for (i = ListNumItems (seq->lWaveforms); i > 0; i--) 
	{
		ListGetItem (seq->lWaveforms, &w, i);
		w->usedBy[0] = 0;
    }
	
	n = ListNumItems (seq->lDigitalBlocks); 
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);	
		for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
			if (b->waveforms[ch] > 0) {
				ListGetItem (seq->lWaveforms, &w, b->waveforms[ch]);
				sprintf (addStr, "%sB%d", (w->usedBy[0] == 0) ? "" : ", ", i);
				if (strlen (w->usedBy)+strlen (addStr) < MAX_WFM_USEDBY_STR_LEN) strcat (w->usedBy, addStr);
			}
		}
	}
	
	n = ListNumItems (seq->lGpibCommands);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lGpibCommands, &g, i);	
		if (g->commandType == GPIB_COMMANDTYPE_ARBWFM) {
			if (g->waveform > 0) {
				ListGetItem (seq->lWaveforms, &w, g->waveform);
				sprintf (addStr, "%s GPIB %s", (w->usedBy[0] == 0) ? "" : ", ", g->name);
				if (strlen (w->usedBy)+strlen (addStr) < MAX_WFM_USEDBY_STR_LEN) strcat (w->usedBy, addStr);
			}
				
		}
	}
	
	
}


void WFM_ADDR_allocateMatrix (t_waveform *w)
{
	w->addrMatrix = (unsigned int*) calloc (WFM_ADDR_MATRIX_NUM*WFM_ADDR_MATRIXDIM, sizeof(unsigned int));
}


/*
void WFM_ADDR_changeAddrType(t_waveform *w, int addrType)
{
	if (w->addrType == addrType) return;
	switch (w->addrType) {
		case WFM_ADDR_TYPE_SINGLESITE:
			free(w->addrMatrix);
			w->addrMatrix = NULL;
			break;
		case WFM_ADDR_TYPE_TRAJECTORY:
			break;
		case WFM_ADDR_TYPE_DMD:
			DMDIMAGE_free(w->addrDMDimage);
			w->addrDMDimage = NULL;
			break;
	}
	switch (addrType) {
		case WFM_ADDR_TYPE_SINGLESITE:
			WFM_ADDR_allocateMatrix(w);
			break;
		case WFM_ADDR_TYPE_TRAJECTORY:
			break;
		case WFM_ADDR_TYPE_DMD:
			w->addrDMDimage = (t_DMDimage*) malloc (sizeof (t_DMDimage));
			DMDIMAGE_init(w->addrDMDimage);
			break;
	}
	w->addrType = addrType;
} */  


// n: matrix number
int WFM_ADDR_getMatrixValue (t_waveform *w, int n, int x, int y)
{
	unsigned int andVal;
	
	if (n < 0 || n >= WFM_ADDR_MATRIX_NUM) {
		return 0;	
	}
	
	if (w->addrMatrix == NULL) return 0;
	if (x > WFM_ADDR_MATRIXDIM) return 0;
	
	andVal = 1 << y;
	return (w->addrMatrix[n*WFM_ADDR_MATRIXDIM+x] & andVal) != 0;
}


void WFM_ADDR_setMatrixValue (t_waveform *w,  int n, int x, int y, int value)
{
	unsigned int xorVal;
	int currentVal;
	
	if (n < 0 || n >= WFM_ADDR_MATRIX_NUM) {
		return;	
	}
	
	if (w == NULL) return;
	if (x > WFM_ADDR_MATRIXDIM) return;

	if (w->addrMatrix == NULL) {
		WFM_ADDR_allocateMatrix  (w);
	}
	currentVal = WFM_ADDR_getMatrixValue (w, n, x, y);
	if (currentVal != value) {
		xorVal = 1 << y;
		w->addrMatrix[n*WFM_ADDR_MATRIXDIM+x] = w->addrMatrix[n*WFM_ADDR_MATRIXDIM+x] ^ xorVal;
	}
}



t_waveform *WFM_ADDR_findDMDwaveform (t_sequence *seq)
{
    int i;
    t_waveform *wfm;
	
    for (i = ListNumItems (seq->lWaveforms); i > 0 ; i--) 
    {
        ListGetItem (seq->lWaveforms, &wfm, i);
		if (wfm->type == WFM_TYPE_ADDRESSING && wfm->addrType == WFM_ADDR_TYPE_DMD && (wfm->addrDMDimage != 0) && (wfm->addrDMDimage->enableDMD) && (wfm->addrLinkWfm == WFM_ID_UNCHANGED)) {
			return wfm;
		}
   	}
   	return NULL;
}



/***************************************************************************

     t_TransferFunction 
            
****************************************************************************/


void TRANSFERFUNCT_init (t_transferFunction *f)
{
	int i;
	
	f->type = TRANSFERFUNCT_TYPE_NONE;
	f->name[0] = 0;
	f->xAxis[0] = 0;
	f->yAxis[0] = 0;
	f->nParameters = 0;
	for (i = 0; i < MAX_TRANSFERFUNCT_PARAMETERS; i++) {
		f->parameters[i] =  0;
		f->parameterNames[i][0] = 'A'+(char)i;
		f->parameterNames[i][1] = 0;
	}
  	strcpy (f->xVariable, "x");
  	strcpy (f->yVariable, "y");
	
}


void TRANSFERFUNCT_free (t_transferFunction *f)
{
//	free (f->parameters);
}



t_transferFunction *TRANSFERFUNCT_copy (t_transferFunction *source)
{
	t_transferFunction *dest;
	
	if (source == NULL) return NULL;
	dest = 	(t_transferFunction *) malloc (sizeof (t_transferFunction));
	memcpy (dest, source, sizeof (t_transferFunction));
	return dest;
}




double TRANSFERFUNCT_apply (t_transferFunction *f, double value)
{
	double result = 0;
	double pot    = 1;
	int i;
	double a, b, c, d;
	
	switch (f->type) {
		case TRANSFERFUNCT_TYPE_POLYNOM:
			for (i = 0; i < f->nParameters; i++) {
				result += pot * f->parameters[i];
				pot *= value;
			}
			break;
		case TRANSFERFUNCT_TYPE_SQRT:
			if (f->nParameters < 2) return 0;
			if (f->parameters[0] == 0) return 0;
			result = value / f->parameters[0] + f->parameters[1];
			if (result >= 0) result = sqrt(result) + f->parameters[2];
			else result = 0;
			break;
		case TRANSFERFUNCT_TYPE_ABC_FORMULA:  
			// inverse of 
			// a x^2 + b x + (c' - value) = 0
			a = f->parameters[2];
			b = f->parameters[1];
			c = (f->parameters[0] - value);
			
			if (a == 0.0)  {
				if (b == 0.0) return 0;
				else return -c/b;
			}
			d = b*b - 4*a*c;	   
			if (d < 0) return 0;
//			if (a == 0) return 0;
			result = (-b + sqrt(d))/ (2*a);
			break;
		}
	return result;
}

	
											  
double TRANSFERFUNCT_applyInv (t_transferFunction *f, double value)
{
	double result = 0;
	double a, b, c, d;
	
	switch (f->type) {
		case TRANSFERFUNCT_TYPE_POLYNOM:
			// a x^2 + b x + c
			a = f->parameters[2];
			b = f->parameters[1];
			c = f->parameters[0];
			d = b*b - 4*a*c;
			if (d < 0) return 0;
			if (a < 0) return 0;
			return (-b + sqrt(d))/ (2*a);
		case TRANSFERFUNCT_TYPE_SQRT:
			result = sqr (value - f->parameters[2]) - sqr (f->parameters[1]);
			result *= f->parameters[0];
			break;
		case TRANSFERFUNCT_TYPE_ABC_FORMULA:
			// a x^2 + b x + c
			result = f->parameters[0] + f->parameters[1] * value + f->parameters[2] * sqr(value);
			break; 
			
	}
	return result;
}


t_transferFunction *TRANSFERFUNCT_new (t_sequence *s)
{
	t_transferFunction *f;
	
	f = (t_transferFunction *) malloc (sizeof (t_transferFunction));
    if (s != NULL) ListInsertItem (s->lTransferFunctions, &f, END_OF_LIST);
    TRANSFERFUNCT_init (f);
    return f;
	
}


t_transferFunction *TRANSFERFUNCT_ptr (t_sequence *s, int nr)
{
	t_transferFunction *f;
    
	if ((nr > (int)ListNumItems (s->lTransferFunctions)) || (nr < 1)) return NULL;
	ListGetItem (s->lTransferFunctions, &f, nr);
	return f;
}


int TRANSFERFUNCT_nr (t_sequence *s, const char *name)
{
	t_transferFunction *f;
	int i;

	for (i = 1; i <= ListNumItems (s->lTransferFunctions); i++) {
		ListGetItem (s->lTransferFunctions, &f, i);
		if (strcmp (f->name, name) == 0) return i;
	}
	
	return 0;
}


t_transferFunction *TRANSFERFUNCT_ptrFromName (t_sequence *s, const char *name)
{
	t_transferFunction *f;
	int i;

	for (i = 1; i < ListNumItems (s->lTransferFunctions); i++) {
		ListGetItem (s->lTransferFunctions, &f, i);
		if (strcmp (f->name, name) == 0) return f;
	}
	
	return NULL;
}


char *TRANSFERFUNCT_str (t_transferFunction *f)
{
	char *str;
	int i;
	char help[30];
	
	str = getTmpString ();
	if (f == NULL) {
		str[0] = 0;
		return str;
	}
	switch (f->type) {
		case (TRANSFERFUNCT_TYPE_POLYNOM):
			for (i = 0; i < f->nParameters; i++) {
				if (i == 0) strcat (str, doubleToStr (f->parameters[i]));
				else {
					if (i == 1) sprintf (help, "+%s*x", doubleToStr (f->parameters[i]));
					sprintf (help, "+%s*x^%d", doubleToStr (f->parameters[i]), i);
					strcat (str, help);
				}
			}
			break;
		case TRANSFERFUNCT_TYPE_SQRT:
			if (f->nParameters >= 2) 
				sprintf (str, "=SQRT(x / %s - %s) + %s", 
					 doubleToStr (f->parameters[0]),
					 doubleToStr (f->parameters[1]),
					 doubleToStr (f->parameters[2]));
			break;
		case TRANSFERFUNCT_TYPE_ABC_FORMULA:
			// a x^2 + b x + c
			break;
			
	}
	
	return str;
}



char *TRANSFERFUNCT_strInv (t_transferFunction *f)
{
	char *str;
	
	str = getTmpString ();
	if (f == NULL) {
		str[0] = 0;
		return str;
	}
	switch (f->type) {
		case (TRANSFERFUNCT_TYPE_POLYNOM):
			break;
		case TRANSFERFUNCT_TYPE_SQRT:
			break;
		case TRANSFERFUNCT_TYPE_ABC_FORMULA:
			// a x^2 + b x + c
			sprintf (str, "%s(%s)=%s + %s*%s + %s*%s^2",
					f->yVariable, f->xVariable,
					f->parameterNames[0], f->parameterNames[1], f->xVariable,
					f->parameterNames[2], f->xVariable);
			break;
	}
	
	return str;
}


void TRANSFERFUNCT_plotInv (t_transferFunction *f, int panel, int ctrl, double minX, double maxX)
{
	int nPoints = 201;
	int i;
	
	double *xArray, *yArray;
	
	xArray = (double *) malloc (sizeof(double) * nPoints);
	yArray = (double *) malloc (sizeof(double) * nPoints);
	
	for (i = 0; i < nPoints; i++) {
		xArray[i] = (maxX - minX) / (1.0*nPoints) * (1.0*i) + minX;
		yArray[i] = TRANSFERFUNCT_applyInv(f, xArray[i]);
	}
	PlotXY (panel, ctrl, xArray, yArray, nPoints, VAL_DOUBLE, VAL_DOUBLE, 
			VAL_THIN_LINE,VAL_EMPTY_SQUARE,VAL_SOLID,1,VAL_RED);
	
	free (xArray);
	free (yArray);
}


void TRANSFERFUNCT_plot (t_transferFunction *f, int panel, int ctrl, double minX, double maxX)
{
	int nPoints = 201;
	int i;
	
	double *xArray, *yArray;
	
	xArray = (double *) malloc (sizeof(double) * nPoints);
	yArray = (double *) malloc (sizeof(double) * nPoints);
	
	for (i = 0; i < nPoints; i++) {
		xArray[i] = (maxX - minX) / (1.0*nPoints) * (1.0*i) + minX;
		yArray[i] = TRANSFERFUNCT_apply (f, xArray[i]);
	}
	PlotXY (panel, ctrl, xArray, yArray, nPoints, VAL_DOUBLE, VAL_DOUBLE, 
			VAL_THIN_LINE,VAL_EMPTY_SQUARE,VAL_SOLID,1,VAL_RED);
	
	free (xArray);
	free (yArray);
}





void TRANSFERFUNCT_writeFormulasToTextbox (t_transferFunction *f, int panel, int ctrl)
{
	char *str;
	cls (panel, ctrl);
//	InsertTextBoxLine (panel, ctrl, -1, f->name);
	str = TRANSFERFUNCT_str(f);
	if (str[0]!= NULL) InsertTextBoxLine (panel, ctrl, -1, str);
	str = TRANSFERFUNCT_strInv(f);
	if (str[0]!= NULL) InsertTextBoxLine (panel, ctrl, -1, str);
}


/***************************************************************************

     t_DigitalBlock: Funktionen
            
****************************************************************************/

// Digital-Block mit Default-Werten füllen
void DIGITALBLOCK_reset (t_digitalBlock *block)
{
   int i;
   
   block->blockName[0] = 0;
   block->groupName[0] = 0;
   block->waitForExtTrigger = 0;
   block->doTriggerLoop = 0;
   block->doRecvTriggerLoop = 0;
   block->disable = 0;
   block->duration = VAL_ms;
   block->variableTime  =  0;
   block->incrementTime =  0;
   block->variableTimeExecuteRep = 1;
   block->variableTimeExecuteRepOffset = 0;
   block->variableTimeStepRep = 1;
   block->variableTimeModulo = 9999;
   block->isAbsoluteTimeReference = 0;
   block->gpibCommandNr = 0;
   block->absoluteTime = 0;
   block->calcDuration  = 0;
   block->blockMode = BMODE_Standard;
//   block->oldTime = 2;
/*   block->analogBlock = 0;
   block->rs232Sequence  = 0;
   block->gpibSequence   = 0;
*/   
   for (i=0; i < N_TOTAL_DIO_CHANNELS; i++) {
       block->channels[i] = 0;
	   block->alternate[i] = 0;
	   block->varyPulse[i] = 0;
	   block->varyPulseStartIncrement[i] = 0;
	   block->varyPulseStopIncrement[i] = 0;
	   block-> varyPulseStepRep[i]  = 1;
   }
//   block->analogBlockP =  NULL;
   block->seq = NULL;

   for (i=0; i < N_DAC_CHANNELS; i++) {
      block->waveforms[i] = WFM_ID_UNCHANGED;
      block->constVoltages[i] = 0.0;
      block->wfmP[i] = NULL; 
   }
   
//   DIGITALBLOCK_resetTableAttributes (block);
   
}



/*
void DIGITALBLOCK_resetTableAttributes (t_digitalBlock *b)
{
	int i;
	
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		b->tableDIO_CELL_FRAME_COLOR[i] = -1;	
		b->tableDIO_CELL_JUSTIFY[i] = -1;
		b->tableDIO_CELL_TYPE[i] = -1;
		b->tableDIO_TEXT_BGCOLOR[i] = -1;
		b->tableDIO_CELL_DIMMED[i] = -1;
		b->tableDIO_TEXT_COLOR[i] = -1;
	}

	for (i = 0; i < N_DAC_CHANNELS; i++) {
		b->tableDAC_CELL_TYPE[i] = -100;
		b->tableDAC_CELL_DIMMED[i] = -1;
		b->tableDAC_CELL_JUSTIFY[i] = -1;
		b->tableDAC_TEXT_BGCOLOR[i] = -1;
		b->tableDAC_CELL_MODE[i] = -1;
	}
}
*/


// neue Inhalte des DigitalBlocks anlegen
void DIGITALBLOCK_init (t_digitalBlock *block)
{
   DIGITALBLOCK_reset (block);
   block->tableColumnWidth = 0;
   block->blockNr = -1;
   block->groupNr = 0;
   block->isGroupExpanded = 0;
   block->nElementsInGroup = 0;
   block->bitmapPtrArr = NULL;
   block->groupDuration = 0;
   block->doTriggerLoop = 0;
   block->doRecvTriggerLoop = 0;
}


void DIGITALBLOCK_free_FAST (t_digitalBlock *b)
{
}



#define N_BITMAPS (N_TOTAL_DIO_CHANNELS + N_DAC_CHANNELS)


void DIGITALBLOCK_freeGroupBitmaps (t_digitalBlock *b)
{
	int i;
	
	if (b->bitmapPtrArr == NULL) return;

	for (i = 0; i < N_BITMAPS; i++) {
		if (b->bitmapPtrArr[i] != 0) DiscardBitmap (b->bitmapPtrArr[i]);
		b->bitmapPtrArr[i] = 0;
	}
	
}



void DIGITALBLOCK_createGroupBitmapPtr (t_digitalBlock *b)
{
//	DIGITALBLOCK_freeGroupBitmaps (b);
	if (b->bitmapPtrArr  == NULL) {
		b->bitmapPtrArr = (int *) calloc (N_BITMAPS, sizeof (int));
	}
}


// alle Inhalte des Digital-Blocks freigeben
void DIGITALBLOCK_free (t_digitalBlock *b)
{
    //int i;
	
	if (b->bitmapPtrArr != NULL) {
		DIGITALBLOCK_freeGroupBitmaps (b);
		free (b->bitmapPtrArr);
		b->bitmapPtrArr = NULL;
	}
}


// insert a new digital block at position nr
t_digitalBlock *DIGITALBLOCK_insert (t_sequence *seq, int nr)
{
	t_digitalBlock *new;
//	int x = sizeof (t_digitalBlock); 
    new = (t_digitalBlock *) malloc (sizeof (t_digitalBlock));
    ListInsertItem (seq->lDigitalBlocks, &new, nr);
	new->seq = seq;
    return new;
}


t_digitalBlock *DIGITALBLOCK_new (t_sequence *seq)
{
	return DIGITALBLOCK_insert (seq, END_OF_LIST);
}


void DIGITALBLOCK_delete (t_sequence *seq, int nr)
{
	t_digitalBlock *old;
	
	ListRemoveItem (seq->lDigitalBlocks, &old, nr);
	DIGITALBLOCK_free (old);
	free (old);
}


void DIGITALBLOCK_duplicate (t_digitalBlock *dest, t_digitalBlock *source)
{
 	// copy all data
 	memcpy (dest, source, sizeof (t_digitalBlock));
 	dest->blockNr = -1;
	dest->tableColumnWidth = 0;
	dest->bitmapPtrArr = 0;
	dest->gpibCommandNr = 0; // forget gpip command during sequence during copy
}


int DIGITALBLOCK_areIdentical (t_digitalBlock *b1, t_digitalBlock *b2) 
{
    if (b1->duration     		!= b2->duration     		) return 0;
    if (memcmp (b1->channels, b2->channels, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->waveforms, b2->waveforms, N_DAC_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->constVoltages, b2->constVoltages, N_DAC_CHANNELS*sizeof(double)) != 0) return 0;     
    
    if (b1->variableTime 		!= b2->variableTime 		) return 0;
    if (b1->incrementTime		!= b2->incrementTime		) return 0; 
    if (b1->variableTimeExecuteRep	!= b2->variableTimeExecuteRep	) return 0; 	
    if (b1->variableTimeStepRep	!= b2->variableTimeStepRep	) return 0; 
    if (b1->variableTimeModulo  != b2->variableTimeModulo) return 0;   
    if (b1->blockMode           != b2->blockMode)          return 0;

    if (memcmp (b1->varyPulse, b2->varyPulse, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->varyPulseStartIncrement, b2->varyPulseStartIncrement, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->varyPulseStopIncrement, b2->varyPulseStopIncrement, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->varyPulseStepRep, b2->varyPulseStepRep, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     

	return 1;
}



//=======================================================================
//
//   calculate the duration of the digital block
//
//=======================================================================
unsigned long DIGITALBLOCK_getDuration (t_digitalBlock *block, int repetitionNo)
{
    unsigned long ulongInc;
    int nRun;
    
	if (block->disable) return 0;
	
    if (block->variableTime) {
		// -----------------------------------------------------------
    	//   VARIABLE TIME:
        //    block is executed everey 'variableTimeRep' repetitions
	    // -----------------------------------------------------------
        if (((repetitionNo+1+block->variableTimeExecuteRepOffset) % (block->variableTimeExecuteRep)) != 0) return 0;
		// -----------------------------------------------------------
        // 	 imcrement time EVERY repetition
	    // -----------------------------------------------------------
		nRun   = repetitionNo /  block->variableTimeStepRep;
		if (block->variableTimeModulo) nRun = nRun % block->variableTimeModulo;
        if (block->incrementTime < 0) {
			// -----------------------------------------------------------
			// 	 negative increment time
			// -----------------------------------------------------------
			ulongInc = nRun * (- block->incrementTime);
			if (ulongInc > block->duration) ulongInc = block->duration;
			return block->duration - ulongInc;
        }   
        else {
			// -----------------------------------------------------------
			// 	 positive increment time
			// -----------------------------------------------------------
			ulongInc = nRun * block->incrementTime;
			return block->duration + ulongInc;
        }
    }
	// -----------------------------------------------------------
    //   return standard block duration
    // -----------------------------------------------------------
    else return block->duration;
}


//=======================================================================
//
//    calculate absolute times
//
//=======================================================================
__int64 DIGITALBLOCK_calculateAllAbsoluteTimes (t_sequence *seq, int repetition)
{
    t_digitalBlock *b;
    int i;
    int referenceBlock;
    __int64 referenceTime;
    __int64 absoluteTime;
    int loopRepetitions;
	int nBlocks;
	

	
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	if (nBlocks == 0) return 0;

	referenceBlock = -1;
	referenceTime  = 0;
	loopRepetitions = 1;
	// get reference Block
	for (i = 1; i <= nBlocks ; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		b->calcDuration = DIGITALBLOCK_getDuration(b, repetition);
		if (referenceBlock > 0) {
			// already found a reference block, disable absolute times for others
			b->isAbsoluteTimeReference = 0;			
		}
		if (!b->disable) {
	    	if (b->isAbsoluteTimeReference) {
				if ((b->blockMode == BMODE_InLoop) || (b->blockMode == BMODE_LoopEnd)) {
					// illegal reference time 
					referenceBlock = -1;
					b->isAbsoluteTimeReference = 0;
				}
				else {
					// ref. time permitted
					referenceBlock = i;
				}
			}
			if (referenceBlock < 0) {
				// no time reference yet found, att reference time
				if (b->blockMode > 0) loopRepetitions = b->blockMode;
				referenceTime +=  loopRepetitions * b->calcDuration;
				if (b->blockMode ==  BMODE_LoopEnd)   loopRepetitions = 1;
			}
		}
    }
    
    if (referenceBlock < 0) {
    	DIGITALBLOCK_ptr (seq, 1)->isAbsoluteTimeReference = 1;
    	referenceTime = 0;
    }
    
    absoluteTime = 0;
	loopRepetitions = 1;
	for (i = 1; i <= nBlocks; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (!b->disable) {
			b->absoluteTime = absoluteTime;
			if (b->blockMode > 0) loopRepetitions = b->blockMode;
			absoluteTime += loopRepetitions * b->calcDuration;
			if (b->blockMode ==  BMODE_LoopEnd) loopRepetitions = 1;
		}
	}					
	
	return referenceTime;
}





// 	   returns pointer digitalBlock with number 'ID'
t_digitalBlock *DIGITALBLOCK_ptr (t_sequence *seq, int ID)
{
    t_digitalBlock *a;
    
//    if (ID == 0) return NULL;
	if ((ID > (int)ListNumItems (seq->lDigitalBlocks)) || (ID < 1)) return NULL;
	ListGetItem (seq->lDigitalBlocks, &a, ID);
	a->blockNr = ID;
	return a;
}


int DIGITALBLOCK_hasWaveform (t_digitalBlock *b)
{
	int i;
	
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		if (b->waveforms[i] != 0) return 1;
	}
	
	return 0;								 
}


int DIGITALBLOCK_hasConstVoltage (t_digitalBlock *b)
{
	
	int i;
	
	
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		if (b->waveforms[i] == WFM_ID_CONSTVOLTAGE) return 1;
	}
	
	return 0;								 
}


int DIGITALBLOCK_hasVariablePulse (t_digitalBlock *b)
{
	int i;
	
	if (b->disable) return 0;
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		if ((b->varyPulse[i]) && (b->blockMode == 0)) return 1;
	}
	
	return 0;								 
}


int DIGITALBLOCK_getLoopRepetitions (t_sequence *seq, int nr)
{
	int i;
	t_digitalBlock *b;
	
	for (i = nr; i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->blockMode == BMODE_Standard) return 1;
		if (b->blockMode > 0) return b->blockMode;
	}
	return 1;
}



void DIGITALBLOCK_setLoop (t_sequence *seq, int loopStart, int nElements, int nCopies)
{
	int i, n;
	t_digitalBlock *b;
	
	n = ListNumItems (seq->lDigitalBlocks);
	if (nElements == 0) return;
	if ((loopStart < 1) || (loopStart > n)) return;
	if ((loopStart + nElements < 1) || (loopStart + nElements-1 > n)) return;
	
    ListGetItem (seq->lDigitalBlocks, &b, loopStart);
    b->blockMode = nCopies;
	for (i = 1; i < nElements-1; i++) {
	    ListGetItem (seq->lDigitalBlocks, &b, loopStart + i);
	    b->blockMode = BMODE_InLoop;
	}
    ListGetItem (seq->lDigitalBlocks, &b, loopStart + nElements - 1);
    b->blockMode = BMODE_LoopEnd;
}


void DIGITALBLOCK_removeBlocks (t_sequence *seq, int startBlock, int nBlocks, int checkPanels)
{
	int i;
	//t_digitalBlock *b;
	t_digitalBlock **oldBlocks;
	//int panelHidden;
	
	
	nBlocks = min(ListNumItems(seq->lDigitalBlocks) - startBlock, nBlocks);
	if (nBlocks == 0) return;
	oldBlocks = (t_digitalBlock **) malloc (sizeof (t_digitalBlock *) * nBlocks);

	ListRemoveItems (seq->lDigitalBlocks, oldBlocks, startBlock, nBlocks);

	if (checkPanels) {
	}
	
    for (i = nBlocks-1; i >= 0; i--) {
		DIGITALBLOCK_free_FAST (oldBlocks[i]);
		free (oldBlocks[i]);
	}
	free (oldBlocks);
}










/*******************************************************************************

      Data structure "t_sequence" 
      
********************************************************************************/


/*
void SEQUENCE_createVoltageSweepWaveforms (t_sequence *seq)
{
    int i;
    t_waveform *wfm;
    
    if (!seq->AO_voltageSweepsOnly) return;
    for (i = 0; i < N_DAC_CHANNELS; i++) {
    	wfm = WFM_ptr (seq, seq->voltageSweepWaveforms[i]);
    	if (wfm == NULL) {
			wfm = WFM_new (seq);
			WFM_init (wfm, WFM_TYPE_STEP);
			sprintf (wfm->name, "VOLTAGE SWEEP DAC %d", i);
			wfm->hide = 1;
			seq->voltageSweepWaveforms[i] = WFM_nrFromPtr (seq, wfm);
		}
	}
}
*/


void SEQUENCE_initSimulateBuffers (t_sequence *seq)
{
	int device;
	
	seq->simulate_DIO_bufferSizeSamples = 0;
	seq->simulate_AO_bufferSizeSamples = 0;
	for (device = 0; device < MAX_DIO_DEVICES; device++) {
		seq->simulate_DIO_buffer[device] = NULL;
	}
	for (device = 0; device < MAX_AO_DEVICES; device++) {
		seq->simulate_AO_buffer[device] = NULL;
	}
	
}





void SEQUENCE_init (t_sequence *seq)
{
    int i;
	unsigned int dummy;
    t_ext_device *new;
	
    seq->remote 			  = 0;
    seq->filename[0] 		  = 0;
	seq->description 		  = NULL;
	seq->dateLastModified[0]  = 0;
    seq->nExecutionsDone      = 0;
    seq->programVersionCreatedWith = ProgramVersion;
	seq->readOnly  			= 0;
	seq->startNr 			  = 0;
	seq->startDate[0] = 0;
    
    seq->changes			  = 0;
    seq->shortcut 			  = -1;
    seq->maxDigitalChannel    = 96;
    seq->maxAnalogChannels    = 32;
    seq->trigger			  = VAL_NO_TRIGGER;
//    seq->triggerPolarity      = ND_LOW_TO_HIGH;
    seq->considerWatchdog     = 1;
    seq->generateRandomCounterData = 0;
	seq->seqShortName[0]   = 0;
//    seq->maxAnalogChannel     = -1;

    // Parareter für DIO-Board initialisieren
    for (i=0; i<N_TOTAL_DIO_CHANNELS; i++) {
    	seq->DIO_channelNames[i] = NULL;
    	seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_STANDARD;
    	seq->DIO_invertDisplay[i] = 0;
		seq->DIO_lockChannel[i] = 0;
    }
	
    seq->DIO_timebase_50ns    = 20;
    seq->AO_timebase_50ns     = 100;
	
//    seq->DIO_timeBase         = -1;
//    seq->DIO_patternInterval  = 2;

    // Parameter für DAC-Board initialisieren
//    seq->AO_timeBase          = -1;
//    seq->AO_patternInterval   = 2;
    for (i=0; i < N_DAC_CHANNELS; i++) {
        seq->AO_channelNames[i] = NULL;
    }

	seq->lDigitalBlocks = ListCreate (sizeof(t_digitalBlock *));
	seq->lWaveforms 	= ListCreate (sizeof(t_waveform *));
	seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_CHANNEL;
	seq->wfmTableSortDescending = 0;
	
	seq->extdevTableSortMode = EXTDEV_TABLE_SORT_MODE_NAME;
	seq->extdevTableSortDescending = 0;
	seq->lExtDevices 	= ListCreate (sizeof(t_ext_device *));
	new = EXTDEV_new (seq);
    EXTDEV_init (new, EXTDEV_TYPE_NONE);
	strcpy (new->name, "No external device");
	dummy=clock();
	EXTDEV_checkForDuplicateIdandChange (seq, &dummy);
	new->uniqueID=dummy;
	seq->noExternalDevice=new;
	
	seq->lGpibCommands 	= ListCreate (sizeof(t_gpibCommand *));
	seq->lTransferFunctions = ListCreate (sizeof(t_transferFunction *));
	seq->lCCDSettings = ListCreate (sizeof(t_ccdSettings *));
	
    
    seq->startNr  			  = -1;
    seq->nRepetitions    = 1;
    seq->nCopies         = 1;
    seq->repetition      = 0;
    seq->nRuns           = 1;
    seq->run             = 0;
    seq->runStart        = 0;
    seq->repetitionStart = 0;
	seq->repetitionStop  = 0;
    
   	seq->outData         = NULL;
   	

	seq->nRedo = 0;
	seq->redoRepetitions = NULL;
	seq->redoRuns = NULL;

	seq->lParameterSweeps = ListCreate (sizeof(t_parameterSweep *));
	seq->referenceTime = 0;
	
    for (i=0; i < N_DAC_CHANNELS; i++) {
		seq->DAC_tableRowHeight[i] = 0;
	}
//	seq->DAC_tableHeight = 0;
	seq->panelSimulate = 0;
	//seq->isRunning = 0;

	
	SEQUENCE_addDateLastModified (seq, 0, 0, 0);
	
	SEQUENCE_initSimulateBuffers (seq);
	seq->firstRepetition = 0;
	
//	seq->doubleBuffering = 0;
	TABLECONFIG_init (&seq->tableConfig, 1);
	seq->currentPath[0] = 0;
	seq->phaseFeedbackWfm[0] = NULL;
	seq->phaseFeedbackWfm[1] = NULL;
	
	seq->manualPifocFocusVoltage = 0;
	seq->enableFocusFeedback = 1;
	
	
	seq->autoFocusNumImages = 7;
	seq->autoFocusCenterVoltage = 4.0;
	seq->autoFocusStepVoltage = 0.05;
	seq->autoFocusFirstBlock = 15;
	seq->isOfTypeIdleSeq = 0;
}; 


// ---------------------------------------------
//   create new sequence 
// ---------------------------------------------
t_sequence *SEQUENCE_new (void)
{
    t_sequence *new;
    
    new = (t_sequence *) malloc (sizeof(t_sequence));
    return new;
}



// ---------------------------------------------
//    get sequence 'nr' from list
// ---------------------------------------------
t_sequence *SEQUENCE_ptr (int nr)
{
	t_sequence *s;
	ListType l;
	
	l = config->listOfSequences;
	if ((nr < 1) || (nr > (int)ListNumItems (l))) return NULL;
	ListGetItem (l, &s, nr);
	return s;
}    


int SEQUENCE_hasLoops (t_sequence *seq)
{
    int i;
	t_digitalBlock *d;
	
	for (i = ListNumItems (seq->lDigitalBlocks); i >= 1; i--) {
		ListGetItem (seq->lDigitalBlocks, &d, i);
		if (d->blockMode != 0) return 1;
	}
	return 0;
}


int SEQUENCE_hasSweeps (t_sequence *seq)
{
	return (ListNumItems(seq->lParameterSweeps)	!= 0);
}


void SEQUENCE_freeSimulateBuffers (t_sequence *seq)
{
	int i;
	
	for (i = 0; i < MAX_DIO_DEVICES; i++) {
		free (seq->simulate_DIO_buffer[i]);
		seq->simulate_DIO_buffer[i] = NULL;
	}
		
	for (i = 0; i < MAX_AO_DEVICES; i++) {
		free(seq->simulate_AO_buffer[i]);
		seq->simulate_AO_buffer[i] = NULL;
	}
}

void SEQUENCE_free (t_sequence *seq)
{
    unsigned int i;
    t_digitalBlock *d;
    t_waveform *w;
    t_gpibCommand *g;
	t_ext_device *extdev;
    t_transferFunction *t;
	//t_parameterSweep *p;
	t_ccdSettings *ccd;
    
	
    if (seq == NULL) return;
	
	free (seq->description);
	seq->description = NULL;

	SEQUENCE_freeSimulateBuffers (seq);
	if (seq->panelSimulate > 0) {
		DiscardPanel (seq->panelSimulate);
		seq->panelSimulate = 0;
	}
    for (i=0; i<N_TOTAL_DIO_CHANNELS; i++) {
        free (seq->DIO_channelNames[i]);
    }
    for (i=0; i<N_DAC_CHANNELS; i++) {
        free (seq->AO_channelNames[i]);
    }

//    freeList (&seq->lDigitalBlocks);
//    freeList (&seq->lRS232Sequence);
//    freeList (&seq->lGPIBSequence);
	// ---------------------------------------------
	//   free digital blocks
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lDigitalBlocks); i++) {
		d = DIGITALBLOCK_ptr (seq, i);
		DIGITALBLOCK_free (d);
		free (d);
	}
	ListDispose (seq->lDigitalBlocks);
	seq->lDigitalBlocks = NULL;

	// ---------------------------------------------
	//   free waveforms
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
		w = WFM_ptr(seq, i);
		WFM_free (w);
		free (w);
	}
	ListDispose (seq->lWaveforms);
	seq->lWaveforms = NULL;
	// ---------------------------------------------
	//   free gpibCommands
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) {
		g = GPIBCOMMAND_ptr (seq, i);
		GPIBCOMMAND_free (g);
		free (g);
	}
	ListDispose (seq->lGpibCommands);
	seq->lGpibCommands = NULL;
// ---------------------------------------------
	//   free external devices
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lExtDevices); i++) {
		extdev = EXTDEV_ptr (seq, i);
		EXTDEV_free(extdev);
		free (extdev);
	}
	ListDispose (seq->lExtDevices);
	seq->lExtDevices = NULL;
	
	// ---------------------------------------------
	//   free transfer functions
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lTransferFunctions); i++) {
		t = TRANSFERFUNCT_ptr (seq, i);
		TRANSFERFUNCT_free (t);
		free (t);
	}
	ListDispose (seq->lTransferFunctions);
	seq->lTransferFunctions = NULL;

	// ---------------------------------------------
	//   free CCD settings
	// ---------------------------------------------
    for (i = 1; i <= ListNumItems (seq->lCCDSettings); i++) {
		ListGetItem (seq->lCCDSettings, &ccd, i);
		CCDSETTINGS_free (ccd);
		free (ccd);
	}
	ListDispose (seq->lCCDSettings);
	seq->lCCDSettings = NULL;
	
    OUTPUTDATA_free (seq->outData, 1);
   	free (seq->outData);
    seq->outData = NULL;
  	
	
	PARAMETERSWEEP_deleteAll (seq);
	ListDispose (seq->lParameterSweeps);
	seq->lParameterSweeps = NULL;
	
	SEQUENCE_freeRedo (seq);
	
	
}

void SEQUENCE_freeRedo (t_sequence *seq)
{
	free (seq->redoRepetitions);
	free (seq->redoRuns);
	seq->redoRepetitions = NULL;
	seq->redoRuns = NULL;
	seq->nRedo = 0;
}


void SEQUENCE_addDateLastModified (t_sequence *seq, int month, int day, int year)
{
	if (month == 0) {
		GetSystemDate (&month, &day, &year);
	}
	sprintf (seq->dateLastModified, "%02d-%02d-%04d", month, day, year);
}


            
            
int SEQUENCE_detectLoops_areDigitalBlocksIdentical (t_digitalBlock *b1, t_digitalBlock *b2) 
{
    if (b1->duration     		!= b2->duration     		) return 0;
    if (memcmp (b1->channels, b2->channels, N_TOTAL_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->waveforms, b2->waveforms, N_DAC_CHANNELS*sizeof(int)) != 0) return 0;     
    if (memcmp (b1->constVoltages, b2->constVoltages, N_DAC_CHANNELS*sizeof(double)) != 0) return 0;     
    
    if (b1->isAbsoluteTimeReference) return 0;
    if (b2->isAbsoluteTimeReference) return 0;
    // variable digital blocks in loops are not implemented
    if (b1->blockMode != BMODE_Standard) return 0;
    if (b2->blockMode != BMODE_Standard) return 0;
//    if (memcmp (b1->varyPulse, b2->varyPulse, N_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
//    if (memcmp (b1->varyPulseStartIncrement, b2->varyPulseStartIncrement, N_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
//    if (memcmp (b1->varyPulseStopIncrement, b2->varyPulseStopIncrement, N_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
//    if (memcmp (b1->varyPulseStepRep, b2->varyPulseStepRep, N_DIO_CHANNELS*sizeof(int)) != 0) return 0;     
    
    if (b1->variableTime 		!= b2->variableTime 		) return 0;
    if (b1->incrementTime		!= b2->incrementTime		) return 0; 
    if (b1->variableTimeExecuteRep		!= b2->variableTimeExecuteRep	) return 0; 	
    if (b1->variableTimeStepRep	!= b2->variableTimeStepRep	) return 0; 
    if (b1->variableTimeModulo  != b2->variableTimeModulo) return 0;   


	return 1;
}
            

int SEQUENCE_detectLoops_hasIdenticalBlockBefore (t_sequence *seq, int compareBlockID, 
											      int maxBlocksPerLoop, int minimumPeriod, int *identicalBlock)
{
	
	t_digitalBlock *compareBlock, *b;
	int i;
	int searchEnd;
	
	*identicalBlock = compareBlockID;
	searchEnd = max(compareBlockID-1-maxBlocksPerLoop,0);
	ListGetItem (seq->lDigitalBlocks, &compareBlock, compareBlockID);
	for (i = compareBlockID-1; i > searchEnd; i--) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (SEQUENCE_detectLoops_areDigitalBlocksIdentical (compareBlock, b)
			&& (compareBlockID - i >= minimumPeriod)) {
			*identicalBlock = i;
			return 1;
		}
	}
	return 0;
}


int SEQUENCE_detectLoops_areBlocksIdentical (t_sequence *seq, int startBlock1, int startBlock2, int nBlocksToCompare)
{

	t_digitalBlock *b1, *b2;
	int i, n;
	
	n = ListNumItems (seq->lDigitalBlocks);
	if ((startBlock1 < 1) || (startBlock1 + nBlocksToCompare-1 > n)) return 0;
	if ((startBlock2 < 1) || (startBlock2 + nBlocksToCompare-1 > n)) return 0;
	for (i = 0; i < nBlocksToCompare; i++) {
		ListGetItem (seq->lDigitalBlocks, &b1, startBlock1+i);
		ListGetItem (seq->lDigitalBlocks, &b2, startBlock2+i);
		if (!SEQUENCE_detectLoops_areDigitalBlocksIdentical  (b1, b2)) return 0;
	}
	return 1;
}



void SEQUENCE_detectLoops (t_sequence *seq, int maxBlocksPerLoop, int minimumPeriod, 
						   int progressDialog, int checkPanels)
{
	int searchBlockID, identicalBlockID;
	int period;
	int nCopies;
	int nBlocks;
	int n;
	
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	
	if (minimumPeriod < 0) minimumPeriod = 0;
	searchBlockID = nBlocks;
	while (searchBlockID > 1) {
		while (!SEQUENCE_detectLoops_hasIdenticalBlockBefore (seq, searchBlockID, 
															  maxBlocksPerLoop, minimumPeriod, 
															  &identicalBlockID) 
			   && (searchBlockID > 0)) {
			searchBlockID --;
		}
		if (searchBlockID < 1) {
			nBlocks = ListNumItems (seq->lDigitalBlocks);
			return;
		}
		period = searchBlockID - identicalBlockID;
		
		if (period > minimumPeriod) {
			nCopies = 0;
			while ((identicalBlockID >= 0) && SEQUENCE_detectLoops_areBlocksIdentical (seq, searchBlockID-period+1, identicalBlockID-period+1, period)){
				nCopies++;
				identicalBlockID -= period;
			}
	
			if (nCopies > 0) {
				if (progressDialog) {
					n = nBlocks - identicalBlockID;
					UpdateProgressDialog (progressDialog, (100 * n) / nBlocks ,0);
				}
				DIGITALBLOCK_setLoop (seq, searchBlockID-period+1, period, nCopies+1);
			    DIGITALBLOCK_removeBlocks (seq, identicalBlockID+1, period * nCopies, checkPanels);
				searchBlockID = identicalBlockID+1;
			}
		}
		searchBlockID--;
		n = nBlocks - searchBlockID;
		if (progressDialog && (n % 50 == 0) ) {
			UpdateProgressDialog (progressDialog, (100 * n) / nBlocks ,0);
		}
	}

	if (progressDialog) UpdateProgressDialog (progressDialog, 100, 0);
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	return;
}

            

// total number of gpib commands during seq including (!!) deactivated ones!!
int SEQUENCE_getNumberOfGpibCommandsDuringSequence (t_sequence *seq)
{
	t_digitalBlock *b;
	int i, n;
	
	n = 0;
	for (i = ListNumItems (seq->lDigitalBlocks); i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->gpibCommandNr > 0) n++;
	}
	return n;
}


char *SEQUENCE_DACchannelName (t_sequence *seq, int ch)
{
	if ((ch >= 0) && (ch < N_DAC_CHANNELS))	return seq->AO_channelNames[ch];
	return "";
}

int SEQUENCE_isIdleSequence(t_sequence *seq)
{	
	//tprintf("config->idleSequenceNr : %d,SEQUENCE_ptr(idle): %d, seq : %d\n"); 
	return seq != NULL && SEQUENCE_ptr (config->idleSequenceNr) == seq;
}

double* SEQUENCE_calculateAutoFocusValues(int autoFocusNumImages, double autoFocusCenterVoltage, double autoFocusStepVoltage) {
	double* vals;
	int i;
	
	vals = (double*)calloc(autoFocusNumImages,sizeof(double));
	for (i=0; i < autoFocusNumImages; i++) {
		vals[i] = autoFocusCenterVoltage-(double)(autoFocusNumImages-1)/2.0*autoFocusStepVoltage + i*autoFocusStepVoltage;
	}
	return vals;
}

// just takes the first gpib with "slicing" in its name - not totally fail safe ...
int SEQUENCE_getSlicingFrequency(t_sequence *seq, double* freqOffsetMHz, double* centerFreqMHz) {
	unsigned int i;
	char* ret;
	double span, centerFreq;
	
	
	
	t_gpibCommand *g;

	
	for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++)  {
		g = GPIBCOMMAND_ptr(seq, i);
		ret = strstr(g->name, "slicing");
		if (ret != NULL && ret != g->name && g->commandType == GPIB_COMMANDTYPE_FREQ) {
			span       = (g->stopFreq - g->startFreq);
			centerFreq = g->startFreq + span / 2;
			*freqOffsetMHz = g->freqOffsetMHz;
			*centerFreqMHz = centerFreq;
			return 0;
		}	
	}
	return -1; // not found
}


void SEQUENCE_updateAutoFocusValues (t_sequence *seq) {
	double *vals;
	t_digitalBlock **bl;
	int startBlock;
 	int pifocChannel;
	int currentIdx = 0;
	int nBlocks;
	//int i;
	int k,n;
	
	if (seq == NULL ||  seq->lDigitalBlocks == NULL) {
		return;	
	}
	if (!seq->isOfTypeIdleSeq) {
		return;	
	}
	
	nBlocks = ListNumItems (seq->lDigitalBlocks); 
	 
	vals = SEQUENCE_calculateAutoFocusValues(seq->autoFocusNumImages, seq->autoFocusCenterVoltage, seq->autoFocusStepVoltage);
	startBlock = seq->autoFocusFirstBlock-1;
	pifocChannel = config->pifocVoltageChannel;
	 
	if (pifocChannel < 0 || pifocChannel >= N_DAC_CHANNELS) {
		tprintf("ERROR: the pifoc channel = %d is out of range [%d;%d]\n",pifocChannel,0,N_DAC_CHANNELS-1);
		return;
	}
	if (startBlock < 0 || startBlock >= nBlocks) {
		tprintf("ERROR: the start block = %d is out of range [%d;%d]\n",startBlock,0,nBlocks-1);
		return;
	}
	
	
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	bl = (t_digitalBlock **) malloc (sizeof (t_digitalBlock*) * nBlocks);
	ListGetItems (seq->lDigitalBlocks, bl, startBlock+1, nBlocks-startBlock);
	
	tprintf("update pifoc auto focus values in idle seq\n");
	 
	n = 0; //0 or 1;  two times seq->autoFocusNumImages (real and bg images)
	for (k = 0; k < nBlocks-startBlock; k++) {
		if (currentIdx >= seq->autoFocusNumImages) {
			break;
		}
		if (bl[k]->waveforms[pifocChannel] == WFM_ID_CONSTVOLTAGE) {
			tprintf("write auto focus val #%d to block %d, channel DAC%d (val from %.3f -> %.3f)\n",currentIdx,k+startBlock+1,pifocChannel,bl[k]->constVoltages[pifocChannel],vals[currentIdx]); 
			bl[k]->constVoltages[pifocChannel] = vals[currentIdx];
			currentIdx++;
		}
		if (n==0 && currentIdx >= seq->autoFocusNumImages) {
			currentIdx = currentIdx % seq->autoFocusNumImages;
			tprintf("\n");
			n++;
		}											 
	}
	if (currentIdx != seq->autoFocusNumImages) {
		tprintf("ERROR: setting pifoc focus voltages failed: not enough constant voltage entries in channel %d starting from block %d\n",pifocChannel,startBlock);	
	}

	 
	free(vals);
	//DIGITALBLOCKS_displayAllAnalogChannels_fast (seq, bl, nBlocks, panelSequence, SEQUENCE_TABLE_dac2);   
}

/************************************************************************
 *
 *  Data structure "t_outputData"
 * 
 ************************************************************************/



void ANALOGSAMPLE_init (t_analogSample *a)
{
	memset (a->value, 0, sizeof (t_analogSample));	
}






void DIGITALEVENT_init (t_digitalEvent *e)
{
	memset (e, 0, sizeof (t_digitalEvent));
}




void DIGITALEVENT_free (t_digitalEvent *e)
{
}



//typedef int     (CVICALLBACK * CompareFunction)(void *item, void *itemOrArray); /* Typedef for ansi compatible comparison function  */	

int CVICALLBACK DIGITALEVENT_compare (void *e1, void *e2)
{
	t_digitalEvent *de1, *de2;
	
	de1 = * ((t_digitalEvent **) (e1));
	de2 = * ((t_digitalEvent **) (e2));
	if (de1->time < de2->time) return -1;
	if (de1->time > de2->time) return 1;
	return 0;
}
	

void DIGITALEVENT_copyValues (t_digitalEvent *target, t_digitalEvent *source)
{
	memcpy (&target->digValue, &source->digValue, 4 * MAX_DIO_DEVICES);  	
}


t_digitalEvent *DIGITALEVENT_new (ListType list, __int64 time, unsigned int duration, unsigned long *digValues) 
{
	int nElements;
	t_digitalEvent *e;
	t_digitalEvent *lastItem;
	
	e = (t_digitalEvent *) malloc (sizeof (t_digitalEvent));
	DIGITALEVENT_init (e);
	
	e->time = time;
	e->duration = duration;
	if (digValues != 0) memcpy (&e->digValue, digValues, 4 * MAX_DIO_DEVICES);

	nElements = ListNumItems (list);
	if (nElements == 0) {
		ListInsertItem (list, &e, END_OF_LIST);
		return e;
	}
	ListGetItem (list, &lastItem, nElements);
	if (e->time >= lastItem->time) {
		ListInsertItem (list, &e, END_OF_LIST);
		return e;
	}

	ListInsertInOrder (list, &e, DIGITALEVENT_compare);
	return e;
}




/*
void OUTPUTDATA_freeBuffers (t_outputData *o) 
{
    int i;

	for (i = 0; i < MAX_DIO_DEVICES; i++) {
		free (o->DIO_OutBuf[i]);
		o->DIO_OutBuf[i] = NULL;
	}
	for (i = 0; i < MAX_AO_DEVICES; i++) {
		free (o->AO_OutBuf[i]);
		o->AO_OutBuf[i] = NULL;
	}
}
*/

void OUTPUTDATA_freeDigitalEvents (t_outputData *o)
{
	t_digitalEvent *e;
	int i;

	for (i = ListNumItems (o->lDigitalEvents); i > 0; i--) {
		ListGetItem (o->lDigitalEvents, &e, i);
		DIGITALEVENT_free (e);
		free (e);
	}
	ListClear (o->lDigitalEvents);
}






void ADWINDATA_init (t_adwinData *a)
{
	int i;
	
	if (a == NULL) return;
	a->nAOelements = 0;
	a->AObuffer = NULL;
	
	a->nDIOelements = 0;
	a->DIObuffer  = NULL;
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		a->AOvaluesAtEndOfSequence[i] = 0;
	}

	for (i = 0; i < ADMIN_MAX_OFFSET_INDEX; i++) {
		a->AOoffsetsDuringSequence[i] = 0;
	}
	
	a->maxCycleNo = 0;
	a->trigger = VAL_NO_TRIGGER; 
	
	a->triggerTimesDuringSeq = NULL;
	a->nTriggerTimesDuringSeq = 0;
	
	a->nGpibCommands = 0;
	a->gpibCommandPtr = NULL;
	a->gpibCommandExecutionCycle = NULL;
	
	a->phaseFeedbackActive = 0;
	a->phaseFeedbackImageFilename[0] = 0;
	a->phaseFeedbackProtocolFilename[0] = 0;
	
	a->phaseFeedbackDeltaU[0] = 0;
	a->phaseFeedbackDeltaU[1] = 0;
	
	a->phaseFeedbackWfm[0] = 0;
	a->phaseFeedbackWfm[1] = 0;
	
	a->repetition = 0;
	a->startNr = 0;
	a->callingSequence = NULL;
}






void ADWINDATA_free (t_adwinData *a)
{
	//int i;
	
	if (a == NULL) return;
	
	free (a->AObuffer);
	free (a->DIObuffer);
 	free (a->triggerTimesDuringSeq); 
	free (a->gpibCommandPtr);
	free (a->gpibCommandExecutionCycle);
	a->nGpibCommands = 0;
	ADWINDATA_init (a);
}







void OUTPUTDATA_free (t_outputData *o, int freeLastValues)
{
    int i;
    
    if (o == NULL) return;
    
	OUTPUTDATA_freeDigitalEvents (o);
	ListDispose (o->lDigitalEvents);
	o->lDigitalEvents = NULL;

	for (i = 0; i < MAX_DIO_DEVICES; i++) {
		free (o->DIO_Values[i]);
	    o->DIO_Values[i] = NULL;
		o->AO_nSamples[i] = 0;
		free (o->AO_Samples[i]);
		o->AO_Samples[i] = NULL;
		free (o->AO_SampleRepeat[i]);
		o->AO_SampleRepeat[i] = NULL;
    }
    free (o->DIO_Repeat);
    o->DIO_Repeat = NULL;
    for (i=0; i<N_DAC_CHANNELS; i++) {
        free (o->AO_Values[i]);
        o->AO_Values[i] = NULL;
        free (o->AO_ValueDuration[i]);
        o->AO_ValueDuration[i] = NULL;
        o->numAOValues[i] = 0;

        free (o->last_AO_Values[i]);
        o->last_AO_Values[i] = NULL;
        free (o->last_AO_ValueDuration[i]);
        o->last_AO_ValueDuration[i] = NULL;
        o->last_numAOValues[i] = 0;
    }
    
//    OUTPUTDATA_freeBuffers (o);
	free (o->triggerTimesDuringSeq);
	o->triggerTimesDuringSeq = NULL;
	o->nTriggerTimesDuringSeq = 0;
	
	ADWINDATA_free (o->adwinData);
	free (o->adwinData);
	o->adwinData = NULL;
}



void OUTPUTDATA_resetCounters (t_outputData *o)
{
	int i;
	
	o->DIO_totalSamplesWritten = 0;
	o->DIO_repCnt = 0;
	o->DIO_valueNr = 0;
	o->DIO_copyNr = 0;

	o->AO_totalSamplesWritten = 0;
	for (i = 0; i < MAX_AO_DEVICES; i++) {
		o->AO_repCnt[i] = 0;
		o->AO_valueNr[i] = 0;
		o->AO_copyNr[i] = 0;
	}

}




void OUTPUTDATA_init (t_outputData *o, int initLastData)
{
    int i, k;
    unsigned long defaultMaxAOValues = 256;

	o->lDigitalEvents = ListCreate (sizeof(t_digitalEvent *));   
    o->numDIOValues      = 0;
    o->DIOdeltaT         = 0;
    for (i = 0; i < MAX_DIO_DEVICES; i++) {
		o->DIO_Values[i] = NULL;
	}
    o->DIO_Repeat = NULL;
	
	o->DIO_dblBuf = 0;
	

   	o->AOdeltaT      = 0;
    
    for (i=0; i<N_DAC_CHANNELS; i++) {
        o->maxAOValues[i] = defaultMaxAOValues;
        o->numAOValues[i] = 0;
        o->AO_Values[i]   = 
            (long *) malloc (sizeof(long)*o->maxAOValues[i]);
        o->AO_ValueDuration[i]   = 
            (unsigned long*) malloc (sizeof(unsigned long)*o->maxAOValues[i]);
	    o->AO_ShiftSamples[i] = 0;
	    
	    if (initLastData) {
	        o->last_numAOValues[i] = 0;
	        o->last_AO_Values[i] = NULL;
	        o->last_AO_ValueDuration[i] = NULL;
	        o->last_constAOvoltage[i] = 0;
	    }
    }
    

    o->duration           = 0;
    o->nCopies            = 1;
    o->AO_totalSamples     = 0;
    o->DIO_totalSamples    = 0;
    
    o->variableOutput     = 0;

	for (i = 0; i < MAX_AO_DEVICES; i++) {
    	o->AOnChannels[i]  = 0;	
	    for (k = 0; k < MAX_CHANNEL_VECT; k++) {
	    	o->AOchannelVect[i][k] = -1;
	    }
		o->AO_nSamples[i] = 0;
		o->AO_Samples[i] = NULL;
		o->AO_SampleRepeat[i] = NULL;
		
	}
//	for (i = 0; i < N_LECROY_CHANNELS; i++) o->LeCroy_OutBuf[i] = NULL;
//    o->configCounter = (t_configCounter *) malloc (sizeof (t_configCounter));
//    initConfigCounter (o->configCounter);
	
	o->AO_dblBuf = 0;
	
	OUTPUTDATA_resetCounters (o);
	
	
	o->adwinData = NULL;
	o->nActiveDACChannels = 0;
	o->digParameters = digitizeParameters_PCI67XX (VAL_us); 
	o->simulate = 0;
	
	o->triggerTimesDuringSeq = NULL;
	o->nTriggerTimesDuringSeq = 0;
	
}



/***************************************************************************

     t_gpibSequence: Funktionen
            
****************************************************************************/
void GPIBCOMMAND_init (t_gpibCommand *item, int type)
{
	int i;

	item->name[0]      = 0;
	item->gpibAddress  = 1;
	item->device       = NULL;
	item->commandType  = type;
	item->noGPIBdevice = 0;
	item->extDeviceID  = 0;
	item->extDeviceChannel = 1;

	item->waveform     = WFM_ID_UNCHANGED;
	item->channel      = 1;
	item->transmitBeforeFirstRepetition = 0;
	item->transmitBeforeEveryRepetition = 0;
	item->transmitAfterEveryRepetition  = 0;
	item->transmitAfterLastRepetition   = 0;
	item->transmitAfterStartOfSequence = 0;
	item->nPoints 	        = 10;
	item->nCopies			= 1;
	item->duration_us		= 10;
	item->dataBuffer        = NULL;
	item->offsetCorrection  = 0;
	item->wfmTransferMode 		= GPIB_COMMAND_WFM_TRANSFERMODE_TRANSMIT_ONLY;
	item->maximumDuration_ms = 10000;
//	GPIB_calculateTimebase (item);

	
	item->enterSpan		  = 1;
	item->addFreqOffset   = 0;
	item->freqOffsetMHz	  = 534.682611;
	
	item->enableStep      = 0;
	item->startFreq       = 0;
	item->stopFreq        = 0;
	item->nStepRepetition = 1;
	item->stepTrigger	  = GPIB_COMMAND_FREQ_TRIGGER_CALC;
	item->startMultiply	  = 1E6;
	item->stopMultiply	  = 1E6;
	item->centerMultiply  = 1E6;
	item->spanMultiply	  = 1E3;
	item->divideFreq 	  = 1;
	item->rfOn            = 1;
	item->fetchFreq       = 0;
	item->outputPower 	  = 0;
	item->pulseModeEnable = 1;
//	item->pulseModeLogic  = 1;

	
	item->enableFM = 0;
	item->freqDeviation = 0;
	item->freqDeviationMultiply = 1E3;
	item->askDeviceForErrors = 0;
	item->wfmAlwaysMaxPts = 1;
	
	
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		item->commandStringSend[i]	     = 0;
		item->commandString[i] 		     = NULL;
		item->commandStringReceive[i]    = 0;
		item->commandStringErrorCheck[i] = 0;
	}
	
	item->voltage = 0;
	item->current = 0;
	item->stepVoltage = 0;
	item->startVoltage = 0;
	item->stopVoltage = 0;
	item->stepCurrent = 0;
	item->startCurrent = 0;
	item->stopCurrent= 0;
	item->waitUntilStabilized = 0;
	item->enableAM = 0;
	item->AMdepthPercent = 100;
	
	item->menuID = 0;
	for (i = 0; i < N_FEEDBACK_CHANNELS; i++) {
		item->enableFeedback[i] = 0;
		item->feedbackAnalogInput[i][0] = 0;
		item->feedbackFreqDeviation[i] = 0;
		item->feedbackOffset[i] = 0;
		item->feedbackFrequencyOffset[i] = 0;
		item->feedbackInputTaskHandle[i] = NULL;
		item->feedbackInputValue[i] = 0;
	}
	item->summedFeedbackFrequencyOffset = 0;
	item->frequencyFeedbackDataPath[0] = 0;
	
	// TYPE pulses
	for (i=0; i < GPIB_COMMAND_PULSES_CHANNELS; i++) {
		item->startPulseDelay[i] = 0;
		item->stopPulseDelay[i] = 0;
		item->stepPulseDelay[i] = 0;
		item->startPulseLength[i] = 1e-6;
		item->stopPulseLength[i] = 1e-6;
		item->stepPulseLength[i] = 0;
	}
	
	
}


void GPIBCOMMAND_free (t_gpibCommand *item)
{
	int i;
	
	free (item->dataBuffer);
	item->dataBuffer = NULL;
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		free(item->commandString[i]);
		item->commandString[i] = NULL;
	}
}


t_gpibCommand *GPIBCOMMAND_new_at_front (t_sequence *seq)
{
	t_gpibCommand *new;

    new = (t_gpibCommand *) malloc (sizeof (t_gpibCommand));
    ListInsertItem (seq->lGpibCommands, &new, FRONT_OF_LIST);
    return new;
}

t_gpibCommand *GPIBCOMMAND_new_at_second (t_sequence *seq)
{
	t_gpibCommand *new;

    new = (t_gpibCommand *) malloc (sizeof (t_gpibCommand));
    ListInsertItem (seq->lGpibCommands, &new, FRONT_OF_LIST + 1);
    return new;
}

t_gpibCommand *GPIBCOMMAND_new (t_sequence *seq)
{
	t_gpibCommand *new;

    new = (t_gpibCommand *) malloc (sizeof (t_gpibCommand));
    ListInsertItem (seq->lGpibCommands, &new, END_OF_LIST);
    return new;
}


void GPIBCOMMAND_delete (t_sequence *seq, int nr)
{
	t_gpibCommand *old;
	int i;
	t_digitalBlock *b;
	
	ListRemoveItem (seq->lGpibCommands, &old, nr);
	GPIBCOMMAND_free (old);
	free (old);
	
	for (i = ListNumItems (seq->lDigitalBlocks); i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->gpibCommandNr == nr) b->gpibCommandNr = 0;
		else if (b->gpibCommandNr > nr) b->gpibCommandNr--;
	}
	
}



t_gpibCommand *GPIBCOMMAND_ptr (t_sequence *seq, int ID)
{
    t_gpibCommand *g;
    
    if (ID == GPIB_COMMANDTYPE_NONE) return NULL;
    if (seq == NULL) return NULL;
	if ((ID > (int)ListNumItems (seq->lGpibCommands)) || (ID < 1)) return NULL;
	ListGetItem (seq->lGpibCommands, &g, ID);
	return g;
}



void GPIBCOMMAND_duplicate (t_gpibCommand *dest, t_gpibCommand *source)
{
	int i;
	
	memcpy (dest, source, sizeof (t_gpibCommand));
	dest->dataBuffer = NULL;    
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		dest->commandString[i] = strnewcopy (NULL, source->commandString[i]);
	}
	
	
}



// ---------------------------------------------
//    checks if filename already exists
// ---------------------------------------------
t_gpibCommand *GPIBCOMMAND_ptrFromName (t_sequence *seq, char *name)
{
	unsigned int i;
	t_gpibCommand *g;
    
    for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++)
    {
		 g = GPIBCOMMAND_ptr (seq, i);
		 if (strcmp (g->name, name) == 0) return g;
	}
	
	return NULL;
}


int GPIBCOMMAND_nrFromMenuID (t_sequence *seq, int menu, int menuID)
{
	int i;
	t_gpibCommand *g;
	char name[200];
	
	if (menuID == 0) return -1;
	for (i = ListNumItems (seq->lGpibCommands); i > 0; i--) {
		ListGetItem (seq->lGpibCommands, &g, i);
		if (g->menuID == menuID) return i;
	}
	GetMenuBarAttribute (menu, menuID, ATTR_ITEM_NAME, name);
	if (strcmp (name, GPIB_COMMANDNAME_NONE_STR) == 0) return 0;
    return -1;
}


char *GPIBCOMMAND_name (t_sequence *seq, int nr)
{
	t_gpibCommand *g;
	
	g = GPIBCOMMAND_ptr (seq, nr);
	if (g == NULL) return "";
	else return g->name;
}


const char *GPIBCOMMAND_strMultiply (double multiply)
{
	//t_gpibCommand *g;
	
	if (multiply == VAL_Hz) return "Hz";
	if (multiply == VAL_kHz) return "kHz";
	if (multiply == VAL_MHz) return "MHz";
	if (multiply == VAL_GHz) return "GHz";
	return "";
}





/***************************************************************************

     t_gpibDevice: Funktionen
            
****************************************************************************/

void GPIBDEVICE_resetLastValues (t_gpibDevice *item, int resetWaveforms)
{
	int i;

	if (item == NULL) return;
	item->settingsRecalledFromDevice = 0;
	item->last_maxVoltage        = DBL_MAX;
	
	item->last_externalSCLK      = -1;
	item->last_frequencySCLK     = -1.0;  
	item->last_divideFreq[0]     = -1;
	item->last_divideFreq[1]     = -1;
	item->last_triggerSlope      = -1;
	item->last_triggerSignal     = -1;
		  
	for (i = 0; i < N_CHANNELS; i++) {
		if (resetWaveforms) {
			item->last_numAOValues[i]      = -1;
	    	item->last_AOvalues[i]        = NULL;        
	    	item->last_AOvalueDuration[i] = NULL;
	    }
		item->last_nCopies[i]          = -1;
    }
	
	item->last_divide = -1;
	item->last_freq  = DBL_MAX;
	item->last_power = DBL_MAX;
	item->last_on = -1;
	item->last_pulseMode = GPIB_PULSE_MODE_NOT_USED;
	item->last_load = -1;
	
	for (i = 0; i < N_CHANNELS; i++) {
		item->last_voltage[i] = DBL_MAX;
		item->last_current[i] = DBL_MAX;
	}
	item->last_FM = -1;
	item->last_FMfreq_Dev = DBL_MAX;
	
	item->last_AM = -1;
	item->last_AMdepthPercent = DBL_MAX;
	
}



int GPIBDEVICE_valuesChanged (t_gpibDevice *item, int i, int nCopies)
{
	if (item == NULL) return 0;
//	if (item->last_externalSCLK      !=   item->externalSCLK       ) return 1;
//	if (item->last_frequencySCLK     !=	  item->frequencySCLK      ) return 1;
//	if (item->last_divideFreq[i]     !=	  item->divideFreq[i]      ) return 1; 
	if (item->last_triggerSlope      !=	  item->triggerSlope       ) return 1; 
	if (item->last_nCopies[i]        !=   nCopies) return 1;
	return 0;	
}	



void GPIBDEVICE_copySettings (t_gpibDevice *target, t_gpibDevice *source)
{
	if (target == NULL) return;
	if (source == NULL) return;
	if (source->type != GPIBDEV_NONE) target->type = source->type;
	strcpy (target->strIDN, source->strIDN);
	strcpy (target->strIDN2, source->strIDN2);
	
	target->commandType1 			  	= source->commandType1; 	          
	target->commandType2 			  	= source->commandType2; 	          
	target->wfmMinPoints    		  	= source->wfmMinPoints;              
	target->wfmMaxPoints    		  	= source->wfmMaxPoints;              
	target->minVoltage      		  	= source->minVoltage;                
	target->maxVoltage      		  	= source->maxVoltage;                
	target->minDigital      		  	= source->minDigital;                
	target->maxDigital				  	= source->maxDigital;		          
	target->minDuration_us  		  	= source->minDuration_us;            
	target->nChannels				  	= source->nChannels;		          
	target->maxDivide	    		  	= source->maxDivide;	              
//	target->hasExternalSCLK 		  	= source->hasExternalSCLK;           
	target->hasPulseOption				= source->hasPulseOption;
	target->minFrequency 			  	= source->minFrequency;              
	target->maxFrequency   			  	= source->maxFrequency;              
	target->minAmplitude   			  	= source->minAmplitude;              
	target->maxAmplitude   			  	= source->maxAmplitude;              
//	target->pulseModeEnable			  	= source->pulseModeEnable;           
//	target->pulseModeLogic			  	= source->pulseModeLogic;           
	target->enableExternalTrigger	  	= source->enableExternalTrigger;     
	target->maxCurrent 				  	= source->maxCurrent;                
	target->GPIB_transmitFrequency	  	= source->GPIB_transmitFrequency;   
	target->GPIB_transmitWaveform	  	= source->GPIB_transmitWaveform;     
	target->GPIB_transmitPowerSupply  	= source->GPIB_transmitPowerSupply;  
	target->GPIB_getDeviceSettings	  	= source->GPIB_getDeviceSettings;    
	target->GPIB_waitUntilStabilized  	= source->GPIB_waitUntilStabilized;  
	target->GPIB_getDeviceOptions  		= source->GPIB_getDeviceOptions;  
	target->errorMessageCompatibleWithAgilent =source->errorMessageCompatibleWithAgilent; 
}


void GPIBDEVICE_init (t_gpibDevice *item)
{
   	item->handle 	  = -1;
    strcpy (item->name, "???");
   	strcpy (item->strIDN, "?");
   	item->strIDN2[0] = 0;
    item->type   		= GPIBDEV_NONE;
    item->saveInfo      = 1;
    item->commandType1 = GPIB_COMMANDTYPE_NONE;
    item->commandType2 = GPIB_COMMANDTYPE_NONE;
    item->wfmMinPoints   = 1;
    item->wfmMaxPoints   = 500000;
    item->wfmPointsMultiple = 1;
	item->minDuration_us = 0.100;
	item->maxDuration_us = 1.0E8;
	item->minVoltage     = 0;
	item->maxVoltage     = 0;
	item->minDigital     = 0;
	item->maxDigital	 = 0;
	item->nChannels		 = 1;
	item->load 			 = DEFAULT_LOAD;
	
//	item->hasExternalSCLK   = 1;
//	item->externalSCLK      = 1;
//	item->frequencySCLK     = 20.0E6;  
//	item->divideFreq[0]     = 1;
//	item->divideFreq[1]     = 1;
	item->triggerSlope      = TRIGGER_SLOPE_POS;
	item->triggerSignal     = TRIGGER_SIGNAL_EXT;
	item->hasPulseOption    = 0;
	
	item->hasAM = 0;
	item->minAMdepthPercent = 1;
	item->maxAMdepthPercent = 100;
	

	item->minFrequency   = 1;
	item->maxFrequency   = 100E9;
	item->enableExternalTrigger = 0;
//	item->pulseModeEnable  = 0;
//	item->pulseModeLogic   = 0;
	item->minAmplitude   = -50;
	item->maxAmplitude   = 40;
	item->maxDivide      = 10;
	
	item->maxCurrent = 0.010;
	item->errorMessageCompatibleWithAgilent = 1;
	
	item->GPIB_transmitFrequency = NULL;
	item->GPIB_transmitWaveform  = NULL;
	item->GPIB_transmitPowerSupply = NULL;
	item->GPIB_getDeviceSettings  = NULL;
	item->GPIB_waitUntilStabilized = NULL;
	item->GPIB_getDeviceOptions = NULL;
	
	
	GPIBDEVICE_resetLastValues (item, 1);

}


void GPIBDEVICE_free (t_gpibDevice *item)
{
	int i;
	
	for (i = 0; i < 2; i++) {
		free (item->last_AOvalues[i]);
    	item->last_AOvalues[i]   = NULL;        
    	free (item->last_AOvalueDuration[i]);
    	item->last_AOvalueDuration[i] = NULL;
    }
}

t_gpibDevice *GPIBDEVICE_new (t_config *c)
{
	t_gpibDevice *new;

    new = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
    GPIBDEVICE_init (new);
    ListInsertItem (c->listOfGpibDevices, &new, END_OF_LIST);
    return new;
}


void GPIBDEVICE_deleteAll (t_config *c)
{
	t_gpibDevice *old;
	int nr;
	int nTot;
	
	nTot = ListNumItems (c->listOfGpibDevices);
	for (nr = nTot; nr > 0; nr--) {
		ListRemoveItem (c->listOfGpibDevices, &old, nr);
		GPIBDEVICE_free (old);
		free (old);
	}
}


t_gpibDevice *GPIBDEVICE_ptrConfig (t_config *c, int address)
{
    t_gpibDevice *d;
    
	if ((address > (int)ListNumItems(c->listOfGpibDevices)) || (address < 1)) return NULL;
	ListGetItem (c->listOfGpibDevices, &d, address);
	return d;
}


t_gpibDevice *GPIBDEVICE_ptr (int address)
{
	return GPIBDEVICE_ptrConfig (config, address); 
}



t_gpibDevice *GPIBDEVICE_ptrChannel (int channel)
{
	return GPIBDEVICE_ptrConfig (config, (channel - N_DAC_CHANNELS) / 2 + 1); 
}



int GPIBDEVICE_isType (t_gpibDevice *d, int type)
{
	if (d == NULL) return 0;
	else return (d->commandType1 == type) || (d->commandType2 == type); 
}



char *GPIBDEVICE_nameFromAddress (int address)
{
	t_gpibDevice *dev;
	
	dev = GPIBDEVICE_ptr (address);
	if (dev == NULL) return "";
	return dev->name;
}






//=======================================================================
//
// 	  return a stepped output voltage
//
//=======================================================================
double getStepOutputVoltage (double stepFrom, double stepTo, 
						     int stepRep, int nRepetitions, int repetition)
{
	double step; 
	int nSteps, nRun;
	 
	// -----------------------------------------------------------
    // 	  calculate number of voltage steps
	// -----------------------------------------------------------
	if (stepRep == 0) {
		Breakpoint();
		return stepFrom;
	}
	nSteps = (nRepetitions / stepRep - 1);
	if (nRepetitions % stepRep != 0) nSteps++;
	if (nSteps == 0) return stepFrom;
	// -----------------------------------------------------------
    // 	  calculate stepsize
	// -----------------------------------------------------------
	step   = (stepTo - stepFrom) / (1.0*nSteps);
	nRun   = repetition /  stepRep;
	// -----------------------------------------------------------
    // 	  calculate output voltage
	// -----------------------------------------------------------
	return stepFrom + step * (1.0*nRun);
}






 /*****************************************************

  Zeiten addieren, in String verwandeln etc. 

****************************************************/

void dTime (unsigned long time, double *dtime, int *i)
{
    *i = 0;

    while ((time / arr_VAL_Time[*i] == 0) && (*i < 2)) (*i)++;
    if (time < VAL_MIN_ns) *dtime = (double) (time * VAL_MIN_ns) / 1000;
      else *dtime = (double) time / (double) arr_VAL_Time[*i];
}
    



char *strTime (unsigned long time_50ns)
{
    static char timestr[20];
    double dtime;
    int i;
      
    dTime (time_50ns, &dtime, &i);
    sprintf (timestr, "%1.3f", dtime);
    strcpy (timestr+5, " ");  
    strcat (timestr, arr_VAL_TimeUnits[i]);
    return timestr;
}



// ---------------------------------------------
//   return number of Analog Boards
// ---------------------------------------------
int nAOBoards (void)
{
#ifdef _CVI_DEBUG_
	if (config->nAnalogBoards == 0) return 2;
#endif
	return config->nAnalogBoards;
	
}

int nDIOBoards (void)
{
#ifdef _CVI_DEBUG_
	if (config->nDigitalBoards == 0) return 2;
#endif	
	return config->nDigitalBoards;
}
	 
	



double *calculateArrOfDigitizedValues (t_digitizeParameters *p)
{
	double *arr;
	int n;
	int i;
	//double analog;
	//int index;
	
	n = (p->Dpp);
	arr = (double *) calloc ((p->Dpp+1), sizeof (double));
	for (i = p->minDigital; i < p->maxDigital+1; i++) {
		arr[i - p->minDigital] = (p->Vpp * ((i - p->minDigital)) / (1.0*p->Dpp)) + p->minVoltage;
	}
	
	return arr;
}


//=======================================================================
//
// 	  analog to digital conversion
//
//=======================================================================

double DigitalToAnalog (long digital, t_digitizeParameters *p)
{
	static t_digitizeParameters *p_PCI67XX = NULL;
	static t_digitizeParameters *p_ADWIN;
	static double *arrPCI67XX;
	static double *arrADWIN;
	//int valInt;
	//double val;
/*	
	if (p_PCI67XX == NULL) {
		p_PCI67XX = digitizeParameters_PCI67XX(1);
		p_ADWIN = digitizeParameters_ADWIN();
		arrADWIN = calculateArrOfDigitizedValues (p_ADWIN);
		arrPCI67XX = calculateArrOfDigitizedValues (p_PCI67XX);
	}
*/	if (p == NULL) p = digitizeParameters_PCI67XX(1);
    if (digital < p->minDigital) digital = p->minDigital;
    if (digital > p->maxDigital) digital = p->maxDigital;
	if (p == p_ADWIN) return arrADWIN[digital];
	if (p == p_PCI67XX) return arrPCI67XX[digital];

	return ((p->Vpp * ((digital - p->minDigital)) / (1.0*p->Dpp)) + p->minVoltage);
		
}

	 


//=======================================================================
//
// 	  analog to digital conversion
//
//=======================================================================
long AnalogToDigital (double analog, t_digitizeParameters *p)
{
	long digital = 0;
	
	
	// use PCI6713 parameters unless otherwise specified
    if (p == NULL) p = digitizeParameters_PCI67XX(1);

	
    // limit max. output voltage
    if (analog < p->minVoltage) analog = p->minVoltage;
       else
    if (analog > p->maxVoltage) analog = p->maxVoltage;

	// return digital value
	if (p->Vpp != 0) {
		digital = RoundRealToNearestInteger (((analog - p->minVoltage) / p->Vpp * p->Dpp) + p->minDigital + p->offsetCorrection);
	} else {
		return p->minDigital + p->offsetCorrection;	// scaling bad - only offset
	}
	if (digital > p->maxDigital) 
		return p->maxDigital;
	if (digital < p->minDigital) 
		return p->minDigital;
	return digital;
}




/*
typedef struct {
	char name[MAX_SWEEPNAME_LEN];
	char quantity[MAX_AXISNAME_LEN]; // e.g. time/ frequency etc
	char plotAxisName[MAX_AXISNAME_LEN];
	char units[MAX_UNIT_LEN];

	double from;
	int dimension;
	double increment;

	int incrementAfterRepetitions;
	int resetAfterRepetitions;
} t_parameterSweep;

*/


//=======================================================================
//
// 	  PARAMETERSWEEPS
//
//=======================================================================


void PARAMETERSWEEP_init (t_parameterSweep *p)
{
	p->channelName[0] = 0;
	p->description[0] = 0;
	p->quantity[0] = 0;
	p->plotAxisName[0] = 0;
	p->units[0] = 0;
	
	p->from = 0;
	p->dimension = 0;
	p->increment = 0;
	p->incrementAfterRepetitions = 1;
	p->resetAfterRepetitions = 0;
	
	p->listSweep = NULL;
	p->numListSweepPoints = 0;
}



void PARAMETERSWEEP_free (t_parameterSweep *p)
{
	if (p != NULL) {
		if (p->listSweep != NULL) {
			free(p->listSweep);
			p->listSweep = NULL;
		}
		free(p);
	}
	
}


t_parameterSweep *PARAMETERSWEEP_new (t_sequence *s)
{
	t_parameterSweep *new;

	new = (t_parameterSweep *) malloc (sizeof (t_parameterSweep));
    PARAMETERSWEEP_init (new);
	ListInsertItem (s->lParameterSweeps, &new, END_OF_LIST);
	
    return new;
}



void PARAMETERSWEEP_deleteAll (t_sequence *s)
{
	int i;
	t_parameterSweep *p;
	
	for (i = ListNumItems (s->lParameterSweeps); i >= 1; i--) {
		ListGetItem (s->lParameterSweeps, &p, i); // copies list item - in our case only the pointer
		PARAMETERSWEEP_free (p);	
	}
	ListClear (s->lParameterSweeps);
}



void PARAMETERSWEEP_getAllFromDigitalBlocks (t_sequence *s)
{
	t_digitalBlock *b;
	t_parameterSweep *p;
	int nItems;
	int i;
	int nIncrements;
	int ch;
	

	// DigitalBlocks: variableTimes
	nItems = ListNumItems (s->lDigitalBlocks);
	for (i = 1; i <= nItems; i++) {
		ListGetItem (s->lDigitalBlocks, &b, i);
		if ((b->variableTime) && (!b->disable)) {
			p = PARAMETERSWEEP_new (s);	
			sprintf (p->channelName, "block %d (%s)", i, b->blockName);
			sprintf (p->description, "block nr. %d", i);
			strcpy (p->quantity, "duration");
			sprintf (p->plotAxisName, "duration block %d (ms)", i);
			strcpy (p->units, "ms");
			p->from = b->duration / (1.0 * VAL_ms);			
			p->dimension = 1;
			p->increment = b->incrementTime/ (1.0 * VAL_ms);			
			if (b->variableTimeModulo) 
				nIncrements = min(b->variableTimeModulo, (s->nRepetitions / b->variableTimeStepRep)-1);
			else nIncrements = (s->nRepetitions / b->variableTimeStepRep) -1;
			p->to = p->from + nIncrements * p->increment;
			p->incrementAfterRepetitions = b->variableTimeStepRep;
			p->resetAfterRepetitions = b->variableTimeModulo;
		}
	}
	// DigitalBlocks: variable Pulse duration
	
	for (i = 1; i <= nItems; i++) {
		ListGetItem (s->lDigitalBlocks, &b, i);
		if (!b->disable) {
			for (ch = 0; ch < s->maxDigitalChannel; ch++) {
				if (b->varyPulse[ch] && (b->varyPulseStartIncrement != 0)) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, "%s (%s)", str_ChannelNames (ch), STRVALID(s->DIO_channelNames[ch]));
					sprintf (p->description, "block %d", i);
					strcpy (p->quantity, "time pulse start");
					sprintf (p->plotAxisName, "pulse start block %d (ms)", i);
					strcpy (p->units, "ms");
					p->from = 0.0;			
					p->dimension = 1;
					p->increment = b->varyPulseStartIncrement[ch] / (1.0 * VAL_ms);			
	//				if (b->variableTimeModulo) 
	//					nIncrements = min(b->variableTimeModulo, (s->nRepetitions / b->variableTimeStepRep)-1);
	//				else 
					nIncrements = (s->nRepetitions / b->varyPulseStepRep[ch]) -1;
					p->to = p->from + nIncrements * p->increment;
					p->incrementAfterRepetitions = b->varyPulseStepRep[ch];
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				}
				if (b->varyPulse[ch] && (b->varyPulseStopIncrement[ch] != 0)) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, "%s (%s)", str_ChannelNames (ch), STRVALID(s->DIO_channelNames[ch]));
					sprintf (p->description, "block %d", i);
					strcpy (p->quantity, "time pulse stop");
					sprintf (p->plotAxisName, "pulse stop block %d (ms)", i);
					strcpy (p->units, "ms");
					p->from = 0.0;			
					p->dimension = 1;
					p->increment = b->varyPulseStopIncrement[ch] / (1.0 * VAL_ms);			
	//				if (b->variableTimeModulo) 
	//					nIncrements = min(b->variableTimeModulo, (s->nRepetitions / b->variableTimeStepRep)-1);
	//				else 
					nIncrements = (s->nRepetitions / b->varyPulseStepRep[ch]) -1;
					p->to = p->from + nIncrements * p->increment;
					p->incrementAfterRepetitions = b->varyPulseStepRep[ch];
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				}
			}
		}
	}
	
}




				
void PARAMETERSWEEP_appendFromWaveform (t_sequence *s, int wfmNr, const char *channelName, const char *description)
{
	t_parameterSweep *p;
	t_waveform *wfm;
	int i;
	int nIncrements;
	int index;
	t_function *f;
	t_step_param *currStep;
	
	
	wfm = WFM_ptr (s, wfmNr);
	if (wfm == NULL) return;
	
	switch (wfm->type) {
        case WFM_TYPE_POINTS:
    		if (wfm->points == NULL) break;
		    for (i = 0; i < wfm->nPoints; i++) {
  		    	if ((wfm->points[i].varyTime) && (wfm->points[i].timeIncrement_ns != 0.0)) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "time point %d", i);
					strcpy (p->units, "ms");
					sprintf (p->plotAxisName, "time point %d (%s)", i, p->units);
					p->from = wfm->points[i].timeStart_ns / 1E6;
					p->dimension = 1;
					nIncrements = (s->nRepetitions / wfm->points[i].stepRepetitions) - 1;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->increment = wfm->points[i].timeIncrement_ns / 1E6;
					p->to = p->from + nIncrements * p->increment;
					p->incrementAfterRepetitions = wfm->points[i].stepRepetitions;
				}
		    	if ((wfm->points[i].varyValue) && (wfm->points[i].valueIncrement != 0.0)) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "voltage point %d", i);
					strcpy (p->units, "V");
					sprintf (p->plotAxisName, "voltage point %d (%s)", i, p->units);
					p->from = wfm->points[i].valueStart;
					p->dimension = 1;
					nIncrements = (s->nRepetitions / wfm->points[i].stepRepetitions) - 1;
//								nIncrements = s->nRepetitions-1; // / 1; //wfm->stepRep;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->increment = wfm->points[i].valueIncrement;
					p->to = p->from + nIncrements * p->increment;
					p->incrementAfterRepetitions = wfm->points[i].stepRepetitions;
				}
			}
			break;
        case WFM_TYPE_FILE:
  		    	if (wfm->uScaleVary && (wfm->uScaleFrom != wfm->uScaleTo))  {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "amplitude");
					strcpy (p->units, "V");
					sprintf (p->plotAxisName, "amplitude (%s)", p->units);
					p->from = wfm->uScaleFrom;
					p->to = wfm->uScaleTo;
					p->dimension = 1;
					nIncrements = s->nRepetitions; // wfm->stepRep;
					if (nIncrements <= 1) p->increment = 0;
					else p->increment = (p->to - p->from) / (nIncrements-1);
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->incrementAfterRepetitions = 1;
				}
  		    	if (wfm->offsetVary && (wfm->offsetFrom != wfm->offsetTo))  {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "offset");
					strcpy (p->units, "V");
					sprintf (p->plotAxisName, "offset (%s)", p->units);
					p->from = wfm->offsetFrom;
					p->to = wfm->offsetTo;
					p->dimension = 1;
					nIncrements = s->nRepetitions; // wfm->stepRep;
					if (nIncrements <= 1) p->increment = 0;
					else p->increment = (p->to - p->from) / (nIncrements-1);
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->incrementAfterRepetitions = 1;
				}
  		    	if (wfm->durationVary && (wfm->durationFrom_50ns != wfm->durationTo_50ns))  {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "duration");
					strcpy (p->units, "ms");
					sprintf (p->plotAxisName, "offset (%s)", p->units);
					p->from = ui64ToDouble (wfm->durationFrom_50ns) * (50*1E-6);
					p->to = ui64ToDouble (wfm->durationTo_50ns) * (50*1E-6);
					p->dimension = 1;
					nIncrements = s->nRepetitions; // wfm->stepRep;
					if (nIncrements <= 1) p->increment = 0;
					else p->increment = (p->to - p->from) / (nIncrements-1);
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->incrementAfterRepetitions = 1;
				}
			
	
			break;
        case WFM_TYPE_STEP:
			if (wfm->stepFrom != wfm->stepTo) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				strcpy (p->quantity, "voltage");
				sprintf (p->plotAxisName, "voltage (V)");
				strcpy (p->units, "V");
				p->from = wfm->stepFrom;			
				p->dimension = 1;
//				if (b->variableTimeModulo) 
//					nIncrements = min(b->variableTimeModulo, s->nRepetitions / b->variableTimeStepRep);
//				else 
				p->to = wfm->stepTo;
				if (wfm->stepAlternate) {
					nIncrements = 2;
					p->resetAfterRepetitions = 2;
				}
				else {
					nIncrements = s->nRepetitions / wfm->stepRep;
					if (nIncrements <= 1) nIncrements = 2;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				}
				p->increment = (p->to - p->from) / (nIncrements-1);
				p->incrementAfterRepetitions = wfm->stepRep;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
			
			}
			break;
		case WFM_TYPE_FUNCTION:
			if (wfm->functionID == 0) break;
			index = wfm->functionID-1;
			f = FUNCTION_ptr (wfm->functionID);
			for (i = 0; i < f->nParameters; i++) {
				currStep = &wfm->functionParameters[index][i];
				if (((currStep->stepType == WFM_STEP_TYPE_TO_LINEAR) 
				&& (currStep->from != currStep->to))
				|| (currStep->stepType == WFM_STEP_TYPE_LIST && currStep->listSize > 1)) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "function: %s", f->parameterNames[i]);
					sprintf (p->plotAxisName, "%s %s", f->parameterNames[i], f->parameterUnits[i]);
					strcpy (p->units, f->parameterUnits[i]);
					p->from = wfm->functionParameters[index][i].from;			
					p->dimension = 1;
					p->to = currStep->to;	 
					nIncrements = s->nRepetitions / 1; //wfm->stepRep;
					if (nIncrements <= 1) nIncrements = 2;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->increment = (p->to - p->from) / (nIncrements-1);
					p->incrementAfterRepetitions = 1;//wfm->stepRep;
					
					if (currStep->stepType == WFM_STEP_TYPE_LIST && currStep->listSize > 1) {
						p->listSweep = (double*)calloc(currStep->listSize, sizeof(double));
						memcpy(p->listSweep,currStep->list, currStep->listSize*sizeof(double));
						p->numListSweepPoints = currStep->listSize;
					}
				}
			}
	    	if (wfm->durationVary && (wfm->durationFrom_50ns != wfm->durationTo_50ns))  {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "duration");
				strcpy (p->units, "ms");
				sprintf (p->plotAxisName, "offset (%s)", p->units);
				p->from = ui64ToDouble (wfm->durationFrom_50ns) * (50*1E-6);
				p->to = ui64ToDouble (wfm->durationTo_50ns) * (50*1E-6);
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				if (nIncrements <= 1) p->increment = 0;
				else p->increment = (p->to - p->from) / (nIncrements-1);
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;
			}
			break;
		case WFM_TYPE_ADDRESSING:
			if (wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi1]) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "offsetPhi1");
				strcpy (p->units, "");
				sprintf (p->plotAxisName, "offset phi1");
				p->from = wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi1];
				p->to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_offsetPhi1]; 
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				if (nIncrements <= 1) p->increment = 0;
				else p->increment = (p->to - p->from) / (nIncrements-1);
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;
			}
			if (wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi2]) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "offsetPhi2");
				strcpy (p->units, "");
				sprintf (p->plotAxisName, "offset phi2");
				p->from = wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi2];
				p->to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_offsetPhi2]; 
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				if (nIncrements <= 1) p->increment = 0;
				else p->increment = (p->to - p->from) / (nIncrements-1);
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;
			}
			if (wfm->addrCalibrationStep[WFM_ADDR_CAL_DMDpxPerSiteX]) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "DMDpxPerSiteX");
				strcpy (p->units, "px");
				sprintf (p->plotAxisName, "DMD px per site x");
				p->from = wfm->addrCalibration[WFM_ADDR_CAL_DMDpxPerSiteX];
				p->to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_DMDpxPerSiteX]; 
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				if (nIncrements <= 1) p->increment = 0;
				else p->increment = (p->to - p->from) / (nIncrements-1);
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;
			}
			if (wfm->addrCalibrationStep[WFM_ADDR_CAL_DMDpxPerSiteY]) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "DMDpxPerSiteY");
				strcpy (p->units, "px");
				sprintf (p->plotAxisName, "DMD px per site y");
				p->from = wfm->addrCalibration[WFM_ADDR_CAL_DMDpxPerSiteY];
				p->to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_DMDpxPerSiteY]; 
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				if (nIncrements <= 1) p->increment = 0;
				else p->increment = (p->to - p->from) / (nIncrements-1);
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;
			}
			if (wfm->stepAddrMatrix) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "single site pattern");
				strcpy (p->units, "");
				sprintf (p->plotAxisName, "pattern");
				p->from = 0;
				p->to = s->nRepetitions-1; 
				p->dimension = 1;
				nIncrements = s->nRepetitions; // wfm->stepRep;
				p->increment = 1;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->incrementAfterRepetitions = 1;	
			}
			break;
			
	} // switch
}


void PARAMETERSWEEP_appendFromDMDWaveform (t_sequence *s)
{
	t_parameterSweep *p;
	t_waveform *wfm;
	int i;
	int nIncrements;
	t_DMDimage *d;
	t_DMDshape *shape;

	wfm = WFM_ADDR_findDMDwaveform (s);
	if (wfm == NULL) return;
	d = wfm->addrDMDimage;
	if ((wfm->addrType != WFM_ADDR_TYPE_DMD) || (d == NULL)) return;
	shape = DMDSHAPE_ptr (d->shapeID);
	if (shape != NULL) {
		for (i = 0; i < shape->nParameters; i++) {
			if ((d->shapeParametersVary[i]) 
			&& (d->shapeParametersFrom[i] != d->shapeParametersTo[i])) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, "DMD");
				sprintf (p->description, "DMD image");
				sprintf (p->quantity, "DMD shape %s", shape->parameterNames[i]);
				sprintf (p->plotAxisName, "%s %s", shape->parameterNames[i], shape->parameterUnits[i]);
				strcpy (p->units, shape->parameterUnits[i]);
				p->from = d->shapeParametersFrom[i];			
				p->dimension = 1;
				p->to = d->shapeParametersTo[i];
				nIncrements = s->nRepetitions / 1; 
				if (nIncrements <= 1) nIncrements = 2;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->increment = (p->to - p->from) / (nIncrements-1);
				p->incrementAfterRepetitions = 1;
			}
		}
	}
}




void PARAMETERSWEEP_appendFromSweepGpib (t_sequence *s, t_gpibCommand *g, const char *channelName, const char *description)
{
	t_parameterSweep *p;
	int nIncrements;
	double multiply;
	int i;
	
	
	if (g == NULL) return;
	
	switch (g->commandType) {
		case GPIB_COMMANDTYPE_FREQ: 
			if (g->enableStep) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "frequency");
				if (g->enterSpan) multiply = g->centerMultiply; 
				else multiply = g->startMultiply;
				strcpy (p->units, GPIBCOMMAND_strMultiply (multiply));
				sprintf (p->plotAxisName, "frequency (%s)", p->units);
				p->from = g->startFreq / multiply;
				p->dimension = 1;
				nIncrements = (s->nRepetitions / g->nStepRepetition) - 1;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->to = g->stopFreq /multiply;
				if (nIncrements != 0) p->increment = (g->stopFreq - g->startFreq) / (multiply*nIncrements);
				p->incrementAfterRepetitions = g->nStepRepetition;
			}
			break;
		case GPIB_COMMANDTYPE_POWERSUPPLY:
			if (g->stepVoltage) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "voltage");
				strcpy (p->units, "V");
				sprintf (p->plotAxisName, "voltage (V)");
				p->from = g->startVoltage;
				p->dimension = 1;
				nIncrements = s->nRepetitions - 1; //(s->nRepetitions / g->nStepRepetition) - 1;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->to = g->stopVoltage;
				if (nIncrements != 0) p->increment = (p->to - p->from) / (nIncrements);
				p->incrementAfterRepetitions = 1; //g->nStepRepetition;
			}
			if (g->stepCurrent) {
				p = PARAMETERSWEEP_new (s);	
				sprintf (p->channelName, channelName);
				sprintf (p->description, description);
				sprintf (p->quantity, "current");
				strcpy (p->units, "A");
				sprintf (p->plotAxisName, "current (A)");
				p->from = g->startCurrent;
				p->dimension = 1;
				nIncrements = s->nRepetitions - 1; //(s->nRepetitions / g->nStepRepetition) - 1;
				p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
				p->to = g->stopCurrent;
				if (nIncrements != 0) p->increment = (p->to - p->from) / (nIncrements);
				p->incrementAfterRepetitions = 1; //g->nStepRepetition;
			}
			break;
		case GPIB_COMMANDTYPE_PULSES:
			for (i=0; i < GPIB_COMMAND_PULSES_CHANNELS; i++) {
				if (g->stepPulseDelay[i]) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "delay, channel %d", i+1);
					strcpy (p->units, "us");
					sprintf (p->plotAxisName, "time (us)");
					p->from = g->startPulseDelay[i]*1e6;
					p->dimension = 1;
					nIncrements = s->nRepetitions - 1; //(s->nRepetitions / g->nStepRepetition) - 1;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->to = g->stopPulseDelay[i]*1e6;
					if (nIncrements != 0) p->increment = (p->to - p->from) / (nIncrements);
					p->incrementAfterRepetitions = 1; //g->nStepRepetition;
				}
				if (g->stepPulseLength[i]) {
					p = PARAMETERSWEEP_new (s);	
					sprintf (p->channelName, channelName);
					sprintf (p->description, description);
					sprintf (p->quantity, "length, channel %d", i+1);
					strcpy (p->units, "us");
					sprintf (p->plotAxisName, "time (us)");
					p->from = g->startPulseLength[i]*1e6;
					p->dimension = 1;
					nIncrements = s->nRepetitions - 1; //(s->nRepetitions / g->nStepRepetition) - 1;
					p->resetAfterRepetitions = VAL_NO_RESET_REPETITIONS;
					p->to = g->stopPulseLength[i]*1e6;
					if (nIncrements != 0) p->increment = (p->to - p->from) / (nIncrements);
					p->incrementAfterRepetitions = 1; //g->nStepRepetition;
				}
			}
			break;
			
	}
}
	
	




void PARAMETERSWEEP_getAllFromWaveforms (t_sequence *s)
{
	t_digitalBlock *b;
	int nItems;
	int k;
	int ch;
	char channelName[MAX_SWEEPNAME_LEN];
	char description[MAX_SWEEPDESCRIPTION_LEN];
	t_waveform *w;

	WFM_checkAllIfActive (s);
	

	nItems = ListNumItems (s->lDigitalBlocks);
	for (k = 1; k <= nItems; k++) {
		ListGetItem (s->lDigitalBlocks, &b, k);
        for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
        	if ((b->waveforms[ch] > 0) && (b->waveforms[ch] <= ListNumItems (s->lWaveforms))) {
				ListGetItem (s->lWaveforms, &w, b->waveforms[ch]);
				if (w->isActive) {
					sprintf (channelName, "%s (%s)", str_AnalogChannelNames (ch), STRVALID(s->AO_channelNames[ch]));
					sprintf (description, "block %d, wfm '%s'", k, WFM_name (s, b->waveforms[ch]));
					PARAMETERSWEEP_appendFromWaveform (s, b->waveforms[ch], channelName, description);
				}
			}
		}
	}
	PARAMETERSWEEP_appendFromDMDWaveform (s);
}


void PARAMETERSWEEP_getAllFromGpibCommands (t_sequence *s)
{
	t_gpibCommand *g;
	int nItems;
	int i;
	char channelName[MAX_SWEEPNAME_LEN];
	char description[MAX_SWEEPDESCRIPTION_LEN];

	
	nItems = ListNumItems (s->lGpibCommands);
	for (i = 1; i <= nItems; i++) {
		ListGetItem (s->lGpibCommands, &g, i);
		if ((g->transmitBeforeEveryRepetition) || (g->transmitAfterStartOfSequence)) {
			switch (g->commandType) {
				case GPIB_COMMANDTYPE_ARBWFM:
					sprintf (channelName, "GPIB #%d: %s", g->gpibAddress, GPIBDEVICE_nameFromAddress(g->gpibAddress));
					sprintf (description, "CMD %s, wfm '%s'", g->name, WFM_name (s, g->waveform));
					PARAMETERSWEEP_appendFromWaveform (s, g->waveform, channelName, description);
					break;
				case GPIB_COMMANDTYPE_FREQ:
				case GPIB_COMMANDTYPE_POWERSUPPLY:
				case GPIB_COMMANDTYPE_PULSES:
					sprintf (channelName, "GPIB #%d: %s", g->gpibAddress, GPIBDEVICE_nameFromAddress(g->gpibAddress));
					sprintf (description, "CMD %s, wfm '%s'", g->name, WFM_name (s, g->waveform));
					PARAMETERSWEEP_appendFromSweepGpib (s, g, channelName, description);
					break;
			}
		}
	}
}





void PARAMETERSWEEP_getAllFromSequence (t_sequence *s)
{
	PARAMETERSWEEP_deleteAll (s);
	PARAMETERSWEEP_getAllFromDigitalBlocks (s);
	PARAMETERSWEEP_getAllFromWaveforms (s);
	PARAMETERSWEEP_getAllFromGpibCommands (s);
}



double PARAMETERSWEEPS_getValueForRepetition (t_parameterSweep *p, int repetition)
{
	double value;
	int n;

	if (p->listSweep == NULL) {
		n = repetition / p->incrementAfterRepetitions;
		n = n % p->resetAfterRepetitions;

		value = p->from + n * p->increment;
	} else {
		if (repetition < p->numListSweepPoints) {
			value = p->listSweep[repetition];
		} else {
			value = p->listSweep[p->numListSweepPoints-1];	
		}
	}
	return value;
}





void TABLECONFIG_init (t_tableConfig *c, int nr)
{
	int i;
	
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) c->DIOvisible[i] = 1;
	for (i = 0; i < N_DAC_CHANNELS; i++) c->AOvisible[i] = 1;
	
	sprintf (c->name, "Config %d", nr);
}



//// CCDSETTINGS


void CCDSETTINGS_init (t_ccdSettings *c)
{
	c->enableCCD = 0;
	c->serialNo = 0;
	c->description[0] = 0;
	c->cameraType = 1;
	c->shutter = 1;
	c->cooler = 1;
	c->fanMode = CCD_FANMODE_HIGH;
	c->acquisition = 1;
	c->temperature = -80;
	c->preamplifierGain = 3;
	c->shiftSpeed = 2;
	c->readoutRate = 2;
	c->picturesPerRepetition = 3;
	c->exposureTime_ms = 10;
	c->enableEMCCD = 0;
	c->EMCCDgain = 20;
	c->takeSubimage = 0;
	c->subimageCenterX = 0;
	c->subimageCenterY = 0;
	c->subimageSizeX = 0;
	c->subimageSizeY = 0;
	c->binsize = 1;
	c->evaluateSubimage = 0;
	c->evaluateCenterX = 0;
	c->evaluateCenterY = 0;
	c->evaluateSizeX  = 0;
	c->evaluateSizeY = 0;
	c->saveRawData = 0;
	c->imageProcessing = CCD_IMAGEPROCESSING_NONE;
	c->separateFilePerRepetition = 0;
	c->dataPath[0] = 0;
	c->pixelCount = 0;
	c->sumPictures = 0;
	c->saveData = 1;
}


void CCDSETTINGS_free (t_ccdSettings *c)
{
	
}


t_ccdSettings *CCDSETTINGS_new (t_sequence *seq)
{
	t_ccdSettings *new;

    new = (t_ccdSettings *) malloc (sizeof (t_ccdSettings ));
    ListInsertItem (seq->lCCDSettings, &new, END_OF_LIST);
    return new;
}


t_ccdSettings *CCDSETTINGS_ptr (t_sequence *seq, int index)
{
	t_ccdSettings *c;
	
	if ((index > (int)ListNumItems (seq->lCCDSettings)) || (index == 0)) return NULL;
	ListGetItem (seq->lCCDSettings, &c, index);
	return c;
}
	


void CCDSETTINGS_duplicate (t_ccdSettings *dest, t_ccdSettings *source)
{
	memcpy (dest, source, sizeof (t_ccdSettings));	
}




int CCDSETTINGS_getFromSerialNo (t_sequence *seq, int serialNo)
{
	int i;
	t_ccdSettings *c;
	
	for (i = ListNumItems (seq->lCCDSettings); i > 0; i--) {
		ListGetItem (seq->lCCDSettings, &c, i);
		if (c->serialNo == serialNo) return i;
	}
	return 0;
	
}


typedef struct {
	t_sequence *seq;
	t_waveform *dmdWaveform;
	int repetition;
} t_dmdimage_commandfile_data;



int DMDIMAGE_writeBinaryDMDPattern(char* filename, t_waveform *dmdWaveform, int repetition) {
	FILE* fid;
	int bitmapID;
	int picNum;
	t_DMDimage *d;
	
	
	if (dmdWaveform == NULL) return -1;
	d = dmdWaveform->addrDMDimage;
	if (d == NULL) return -1;
	
	
	fid=fopen(filename, "wb");
	if (!fid) {
		tprintf("Error creating file for DMD pattern filename: %s.\n",filename);
		return -1;
	}

	for (picNum=0; picNum < dmdWaveform->addrMatricesPerRepetition;picNum++) {
		bitmapID = DMDIMAGE_createBitmap(dmdWaveform,repetition, picNum);
		if (bitmapID == 0) {
			tprintf("Creation of bitmap failed (bitmapID=%d,repetition=%d,picNum=%d).\n",bitmapID,repetition,picNum);
			fclose(fid);
			return -1;
		}
		DMDIMAGE_prepareBitmapForDMD(bitmapID,fid,d->enableDMDErrorDiffusion,d->enableDMDSmoothing);  
		DiscardBitmap(bitmapID); 
	}
	
	
    fclose(fid);	
	return 0;
	
}

int DMDIMAGE_writeBinaryDMDPatternSinglePicture(char* filename, t_waveform *dmdWaveform, int repetition, int picNum) {
	FILE* fid;
	int bitmapID;
	t_DMDimage *d;
	
	
	if (dmdWaveform == NULL) return -1;
	d = dmdWaveform->addrDMDimage;
	if (d == NULL) return -1;
	
	
	fid=fopen(filename, "wb");
	if (!fid) {
		tprintf("Error creating file for DMD pattern filename: %s.\n",filename);
		return -1;
	}

	bitmapID = DMDIMAGE_createBitmap(dmdWaveform,repetition, picNum);
	if (bitmapID == 0) {
		tprintf("Creation of bitmap failed (bitmapID=%d,repetition=%d,picNum=%d) (single picture).\n",bitmapID,repetition,picNum);
		fclose(fid);
		return -1;
	}
	DMDIMAGE_prepareBitmapForDMD(bitmapID,fid,d->enableDMDErrorDiffusion,d->enableDMDSmoothing);  
	DiscardBitmap(bitmapID); 
	
    fclose(fid);	
	return 0;
}


int CVICALLBACK DMDIMAGE_writeCommandFile (void *functionData)
{
	t_dmdimage_commandfile_data* data;
	t_sequence *seq;
	t_waveform *dmdWaveform;
	int repetition;
	
	//IniText ini;
	int err = 0;
	char iniFilename[MAX_PATHNAME_LEN];
	//char bitmapFilename[MAX_PATHNAME_LEN];   //variable not set
	char tmpFilename[MAX_PATHNAME_LEN];
	//const char sectionName[] = "DMD";
	t_DMDshape *s;
	t_DMDimage *d;
	//int bitmapID = 0;
	//int picNum = 0;
	char slmDir[MAX_PATHNAME_LEN];
	char binaryDMDPatternFilename[MAX_PATHNAME_LEN];
	
	tprintf("Writing DMD image file for waveform \"%s\"...\n",dmdWaveform->name);
	ProcessDrawEvents();
	
	// unpack data
	data = (t_dmdimage_commandfile_data*)functionData;
	tprintf("DMD Test 1\n"); 
	if (data == NULL){
		tprintf("ERROR: No data transmitted for DMD image creation.\n");
		return -1;
	}
	tprintf("DMD Test 2\n"); 
	seq = data->seq;
	dmdWaveform = data->dmdWaveform;
	repetition = data->repetition;
	if (seq == NULL || dmdWaveform == NULL) {
		tprintf("ERROR: Data does not include sequence or DMD waveform.\n"); 
		return -1;
	}
	tprintf("DMD Test 3\n");
	free(data);
	
	// check data
	if (dmdWaveform == NULL) return -1;
	tprintf("DMD Test 4\n");
	d = dmdWaveform->addrDMDimage;
	if (d == NULL) return -1;
	tprintf("DMD Test 5\n");
	if (d->shapeID == 0) return -1;  //ToDo Sebastian: Why do we need these two lines? Ask Peter.
	s = DMDSHAPE_ptr(d->shapeID);
	tprintf("DMD Test 6\n");
	
	//WFM_ADDR_offsetThisRepetition (wfm->addrDMDimage, seq->nRepetitions, repetition); // already calculated before
	DMDIMAGE_calculateParametersThisRepetition (d, seq->nRepetitions, repetition);
	tprintf("DMD Test 7\n");
	

	//if ((ini = Ini_New (0)) == 0) return -1;
	
	sprintf(slmDir,"%s\\SLM",config->autoSaveSequencesPath);
	err = mkDirs (slmDir);
	tprintf("DMD Test 8\n");
	if (err) {
		tprintf("Error creating SLM target directory\n");	
	}
	tprintf("DMD Test 9\n");
	sprintf (tmpFilename, "%s\\SLM\\dmd%d.tmp", config->autoSaveSequencesPath,clock());
	changeSuffix (iniFilename, tmpFilename, ".ini");
	tprintf("DMD Test 10\n");
	
	
	// for new exe-file
	
	sprintf(binaryDMDPatternFilename,"%s\\SLM\\DMD_multi_%d.bin", config->autoSaveSequencesPath,dmdWaveform->addrMatricesPerRepetition);
	remove(binaryDMDPatternFilename);
	err = DMDIMAGE_writeBinaryDMDPattern(tmpFilename,dmdWaveform,repetition);
	if (err == 0) {
		if (FileExists (binaryDMDPatternFilename, 0)) {
			err = DeleteFile (binaryDMDPatternFilename);
			if (err != 0) tprintf ("FileError %d. %s.", err, getErrorMkDir (err));
		}
		err = RenameFile (tmpFilename, binaryDMDPatternFilename);
		if (err != 0) tprintf ("Error renaming to %s: %d. %s.\n", binaryDMDPatternFilename, err, getErrorFileOperation (err));	
		tprintf("DMD image file written to %s (tmpfile: %s).\n",binaryDMDPatternFilename,tmpFilename);
		ProcessDrawEvents();
	} else {
		remove(tmpFilename);
		tprintf("Error: writing binary DMD pattern failed.\n");
	}
	
	
	
	  /*
	// compatible to old exe-file
	sprintf(binaryDMDPatternFilename,"%s\\SLM\\DMD.bin", config->autoSaveSequencesPath);
	remove(binaryDMDPatternFilename);
	err = DMDIMAGE_writeBinaryDMDPatternSinglePicture(tmpFilename, dmdWaveform, repetition, 0);
	if (err == 0) {
		err = RenameFile (tmpFilename, binaryDMDPatternFilename);
		if (err != 0) tprintf ("Error renaming to %s: %d. %s.\n", binaryDMDPatternFilename, err, getErrorFileOperation (err));
		tprintf("DMD image files written (old format).\n");
		ProcessDrawEvents();
	} else {
		remove(tmpFilename);	
	}   */
	
	
	
	/*if ( dmdWaveform->addrMatricesPerRepetition <= 1) {
		bitmapID = DMDIMAGE_createBitmap(dmdWaveform,repetition,0);
		sprintf(binaryDMDPatternFilename,"%s\\SLM\\DMD.bin", config->autoSaveSequencesPath);
		DMDIMAGE_prepareBitmapForDMD(bitmapID,binaryDMDPatternFilename,d->enableDMDErrorDiffusion,d->enableDMDSmoothing);
		
		//sprintf (bitmapFilename, "%s\\dmd_image.bmp", config->autoSaveSequencesPath);
		// tprintf ("Writing bitmap for DMD: %s\n", bitmapFilename);
		//err = SaveBitmapToBMPFile(bitmapID, bitmapFilename);
		//if (err != 0) tprintf ("FileError %d. %s.\n", err, getErrorMkDir (err));
		//DiscardBitmap(bitmapID);
		
		//Ini_PutString(ini, sectionName, "file", bitmapFilename);          
	} else {	
		
		
		for (picNum=0; picNum < dmdWaveform->addrMatricesPerRepetition;picNum++) {
			bitmapID = DMDIMAGE_createBitmap(dmdWaveform,repetition, picNum);
			sprintf(binaryDMDPatternFilename,"%s\\SLM\\DMD_%d.bin", config->autoSaveSequencesPath, picNum);
			DMDIMAGE_prepareBitmapForDMD(bitmapID,binaryDMDPatternFilename,d->enableDMDErrorDiffusion,d->enableDMDSmoothing);  
			//sprintf (bitmapFilename, "%s\\dmd_image_%d.bmp", config->autoSaveSequencesPath,picNum);
			
			//tprintf ("Writing bitmap for DMD: %s\n", bitmapFilename);
			//err = SaveBitmapToBMPFile(bitmapID, bitmapFilename);
			//if (err != 0) tprintf ("FileError %d. %s.\n", err, getErrorMkDir (err));
			//DiscardBitmap(bitmapID);
			
			//Ini_PutString(ini, sectionName, "file", bitmapFilename);          
		}
	} */
	
	
	/*
	Ini_PutInt(ini, sectionName, "smoothEdges", d->smoothEdges);
	if (d->shapeParametersThisRepetition != NULL && s->nParameters >= 4) {
		Ini_PutDouble (ini, sectionName, "centerX", d->shapeParametersThisRepetition[0]); 
		Ini_PutDouble (ini, sectionName, "centerY", d->shapeParametersThisRepetition[1]);
		Ini_PutDouble (ini, sectionName, "scaleFactor", d->shapeParametersThisRepetition[2]);
		Ini_PutDouble (ini, sectionName, "rotationAngle", d->shapeParametersThisRepetition[3]);
	}*/
	
	

/*	tprintf ("Writing inifile for DMD: %s\n", iniFilename);
	err = Ini_writeFile(ini, tmpFilename);
	if (FileExists (iniFilename, 0)) {
		err = DeleteFile (iniFilename);
		if (err != 0) tprintf ("FileError %d. %s.\n", err, getErrorMkDir (err));
	}
	err = RenameFile (tmpFilename, iniFilename);
	if (err != 0) tprintf ("FileError %d. %s.\n", err, getErrorMkDir (err));
	
	Ini_Dispose (ini);
*/

    return err;
}

void DMDIMAGE_THREAD_writeCommandFile (t_sequence *seq, t_waveform *dmdWaveform, int repetition) 
{
	t_dmdimage_commandfile_data* data;
	data = (t_dmdimage_commandfile_data*)malloc(sizeof(t_dmdimage_commandfile_data));
	data->seq = seq;
	data->dmdWaveform = dmdWaveform;
	data->repetition = repetition;
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, DMDIMAGE_writeCommandFile, data, NULL);
}


/*
 * returns new bitmap, needs to be discarded by calling function!
 * for repetition and certain picture in the repetition.
 *
 * return 0 for error (invalid bitmap ID)
 */
int DMDIMAGE_createBitmap (t_waveform *wfm, int repetition, int picNum)
{
	t_DMDimage *d;
	t_DMDshape *s;
	int DMDpanel = 0;
	int canvas = 0;
	int bitmapID = 0;
	int err;
	
	if (wfm == NULL) {
		tprintf("DMDIMAGE_createBitmap failed: wfm NULL\n");
		return 0;
	}
	d = wfm->addrDMDimage;
	
	if (d == NULL) {
		tprintf("DMDIMAGE_createBitmap failed: addrDMDimage NULL\n");
		return 0;
	}
	
	s = DMDSHAPE_ptr (d->shapeID);
	/*if (s == NULL) {  // just do nothing for shapeID null...
		tprintf("DMDIMAGE_createBitmap failed: shapeID invalid\n");    
		return 0;
	}*/
	
	DMDpanel = NewPanel (0, "", 0, 0, d->bitmapSize.x, d->bitmapSize.y);
	canvas = NewCtrl (DMDpanel, CTRL_CANVAS, "", 0, 0);
	SetCtrlAttribute (DMDpanel, canvas, ATTR_HEIGHT, d->bitmapSize.y);
	SetCtrlAttribute (DMDpanel, canvas, ATTR_WIDTH, d->bitmapSize.x);
	SetCtrlAttribute (DMDpanel, canvas, ATTR_DRAW_POLICY,VAL_UPDATE_IMMEDIATELY);
	SetCtrlAttribute (DMDpanel, canvas, ATTR_PEN_FILL_COLOR, VAL_WHITE);
	SetCtrlAttribute (DMDpanel, canvas, ATTR_PICT_BGCOLOR, VAL_WHITE);  
		
//	DisplayPanel (DMDpanel);
	CanvasClear (DMDpanel, canvas, VAL_ENTIRE_OBJECT);
	//CanvasDrawRect (DMDpanel, canvas, MakeRect(1,1,d->bitmapSize.x,d->bitmapSize.y), VAL_DRAW_FRAME_AND_INTERIOR);
	if (s != NULL && s->plotFunc != 0) {
		(s->plotFunc) (d->shapeParametersThisRepetition, DMDpanel, canvas, wfm, repetition, picNum);
	}
//	CanvasUpdate (DMDpanel, canvas, VAL_ENTIRE_OBJECT);
	err = GetCtrlBitmap (DMDpanel, canvas, 0, &bitmapID);
	if (err < 0) {
		tprintf("GetCtrlBitmap failed with err %d\n",err);	
	}
	DiscardCtrl(DMDpanel,canvas);
	DiscardPanel(DMDpanel);
	
	
	return bitmapID;
//	SaveBitmapToBMPFile (d->bitmapID, "D:\\Test.bmp");
		
}





/*
void DMDIMAGE_displayShapeInCanvas (t_waveform *wfm, int repetition)
{
	t_DMDimage *d = wfm->addrDMDimage;
	t_DMDshape *s;
	static int DMDpanel = 0;
	static int canvas = 0;
	
	s = DMDSHAPE_ptr (d->shapeID);
	if (s == NULL) return;
	
	if (DMDpanel == 0) {
		DMDpanel = NewPanel (0, "", 0, 0, d->bitmapSize.x, d->bitmapSize.y);
		canvas = NewCtrl (DMDpanel, CTRL_CANVAS, "", 0, 0);
		SetCtrlAttribute (DMDpanel, canvas, ATTR_HEIGHT, d->bitmapSize.y);
		SetCtrlAttribute (DMDpanel, canvas, ATTR_WIDTH, d->bitmapSize.x);
		SetCtrlAttribute (DMDpanel, canvas, ATTR_DRAW_POLICY,
						  VAL_UPDATE_IMMEDIATELY);
		SetCtrlAttribute (DMDpanel, canvas, ATTR_PEN_FILL_COLOR, VAL_BLACK);
		
	}		
//	DisplayPanel (DMDpanel);
	CanvasClear (DMDpanel, canvas, VAL_ENTIRE_OBJECT);
	(s->plotFunc) (d->shapeParametersThisRepetition, DMDpanel, canvas, wfm, repetition);
	if (d->bitmapID != 0) 
		DiscardBitmap (d->bitmapID);
	d->bitmapID = 0;
	GetCtrlBitmap (DMDpanel, canvas, 0, &d->bitmapID);
	
//	SaveBitmapToBMPFile (d->bitmapID, "D:\\Test.bmp");
		
}	 */

////////////////////////////////////////////
//
// Functions for external devices
//
////////////////////////////////////////////
t_ext_device *EXTDEV_new (t_sequence *seq)
{
	t_ext_device *new;

    new = (t_ext_device *) malloc (sizeof (t_ext_device));
	if (seq != 0) ListInsertItem (seq->lExtDevices, &new, END_OF_LIST);
    return new;
}


void EXTDEV_init (t_ext_device *extdev, int deviceTyp)
{
   	
	extdev->name[0]     = 0;
	extdev->deviceTyp        = deviceTyp;
	extdev->port	=2602;
	strcpy(extdev->address,"130.183.96.88");
	extdev->positionInList 	=1;
	extdev->positionInTable	=1;
	extdev->uniqueID=1; 
}


int EXTDEV_nameExistsN (t_sequence *seq, const char *name, int startSearch, int reverseSearch)
{
	t_ext_device *extdev;
	int n;
	int i;
	
	
	if (reverseSearch) {
		for (i = startSearch; i > 0; i--) {
			ListGetItem (seq->lExtDevices, &extdev, i);
			if (strcmp (extdev->name, name) == 0) return i;
		}
	}
	else {
		n = ListNumItems (seq->lExtDevices);
		for (i = startSearch; i <= n; i++) {
			ListGetItem (seq->lExtDevices, &extdev, i);
			if (strcmp (extdev->name, name) == 0) return i;
		}
	}
	return 0;
}




int EXTDEV_checkForDuplicateNameAndRename (t_sequence *seq, char *name)
{
	//int i;
	int suffix;
	//t_ext_device *extdev;
	int changed;
	
	changed = 0;
	
	suffix = 2;
	while (EXTDEV_nameExistsN (seq, name, 1, 0)) {
		changeNameSuffix (name, MAX_EXTDEV_NAME_LEN, intToStr (suffix));
		suffix ++;
		changed = 1;
	}
	return changed;
}

int EXTDEV_IdExists (t_sequence *seq, unsigned int ID, int startSearch, int reverseSearch)
{
	t_ext_device *extdev;
	int n;
	int i;

	if (reverseSearch) {
		for (i = startSearch; i > 0; i--) {
			ListGetItem (seq->lExtDevices, &extdev, i);
			if (extdev->uniqueID==ID) return i;
		}
	}
	else {
		n = ListNumItems (seq->lExtDevices);
		for (i = startSearch; i <= n; i++) {
			ListGetItem (seq->lExtDevices, &extdev, i);
			if (extdev->uniqueID==ID) tprintf("ID is equal: %d",i);
			if (extdev->uniqueID==ID) return i;
		}
	}
	return 0;
}

int EXTDEV_checkForDuplicateIdandChange (t_sequence *seq, unsigned int *id)
{
	int add;
	int changed;

	changed = 0;
	add = 1;
	while (EXTDEV_IdExists (seq, *id, 1, 0)) {
		*id=*id+1;
		add ++;
		changed = 1;
	}
	return changed;
}
