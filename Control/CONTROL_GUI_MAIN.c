#include <windows.h> // for Sleep   
#include <formatio.h>
#include <utility.h>
#include <userint.h>
#include "UIR_ExperimentControl.h"
#include "INCLUDES_CONTROL.h"    
#include "INTERFACE_TCP_IP.h"
#include "CONTROL_HARDWARE_GPIB.h"
#include "CONTROL_HARDWARE.h"






clock_t mytime;



int menuSequenceList    = -1;  // popup-menu sequence list
int menuSequenceQueue   = -1;  // popup-menu sequence queue

int panelMain 			= -1;  // main window
int panelMain2			= -1;
int panelSweeps 		= 0;
int panelIdleWait		= 0;;
int errorUIR;



extern int threadFunctionId    = 0; // id of the main thread 		 TODO volatile ??
extern volatile int threadStopRequest    = 0; // set to one to request the thread to stop
CmtThreadLockHandle lockSequenceQueue = 0;
CmtThreadLockHandle lockStartButton = 0; 
CmtThreadLockHandle lockSetStatus = 0;

//int errorQueue;


extern int subPanelWFMFunc;
extern int subPanelWFMPoints;

extern int panelAddressing; 

int panelSequenceStartFrom = 0;
int idleSequenceEnabled = 0;   // if the idle sequence is activated and should run if no other sequence is running
t_adwinData *idleSequenceData = NULL;



enum {
	TAB1_INDEX_TIMEBASE,
	TAB1_INDEX_INFO,
	TAB1_INDEX_STARTNO
};
	

#define AUTOQUIT_FILENAME "AUTOQUIT.TXT"

int panelSequence		= -1;
int panelWaveforms      = -1;
int panelGpib			= -1;
int panelCCD			= -1;
int panelConfiguration 	= -1;
//int panelOutput		    = -1;  // output data

int panelExtDev			= -1;

int remoteKey = 0;



// concept: every sequence is added to the sequence queue before being executed (exception: idle sequence)
// all the sequences are copied to the sequence queue (deep copy) and are freed after execution
// idle seq is represented by NULL
ListType listOfWaitingSequences;


// running sequence != activeSequence !!!  
// running sequence == currently running sequence.
t_sequence *runningSequence = NULL;

int SEQUENCE_autoSaveSequence (t_sequence *s, int alwaysAutoSave);
t_sequence *SEQUENCE_fromShortcut (int key);

void SEQUENCE_displayStartNr (int currentStartNr);
int SEQUENCE_writeCommandToFile (const char *command);
void PARAMETERSWEEP_initListCtrl (int panel, int ctrl);
int PARAMETERSWEEPS_createTxtFile (const char *filename, t_sequence *s, int shortVersion);
int MAIN_startSequence (t_sequence *s, int freeSequence);
void SEQUENCE_startOrEnqueue(t_sequence *s);
void MAIN_handleSequenceFinished(void);
void SEQUENCE_startThread(void);


extern int panelWfmSmall;



/************************************************************************/
/*
/*    SEQUENCE_QUEUE stuff: insert,delete
/*
/************************************************************************/


void SEQUENCE_QUEUE_Insert(t_sequence* seq, int pos) {
	t_sequence* tmp;	  
	
	if (pos < -1 || pos > (int)ListNumItems(listOfWaitingSequences)+1) {
		tprintf("error inserting sequence to sequence queue\n");
		return;		 
	}
	
	if (seq == NULL || seq == SEQUENCE_ptr(config->idleSequenceNr)) {  // insert idle as NULL
		tmp = NULL;	
	} else {
		tmp = SEQUENCE_clone(seq);
	}
	CmtGetLock(lockSequenceQueue);
	ListInsertItem(listOfWaitingSequences,&tmp,pos);
	CmtReleaseLock(lockSequenceQueue);
}


// inserts numSeq sequences from sequence array seq, repeat these Blocks numRepetitions times
// insert every "idleEvery" sequence a idle sequence, idleEvery = numSeq means idle after each Block.

// seq is not allowed to contain NULL ir IDLE seqs
void SEQUENCE_QUEUE_InsertMulti(t_sequence** seq, int numSeq, int numRepetitions, int idleEvery) {
	t_sequence*** tmp;
	t_sequence* idle = NULL;
	int i, j, k;
	
	if (idleEvery < 1) {
		return;	
	}
	
	for (i = 0; i < numSeq; i++) { 
		if (seq[i] == NULL || seq[i] == SEQUENCE_ptr(config->idleSequenceNr)) {
			PostMessagePopupf("Insert aborted","It is not allowed to insert the idle sequence multiple times using this function.\n");
			return;	 
		}
	}
	
	tmp = (t_sequence***)calloc(numSeq,sizeof(t_sequence**));
	if (tmp == NULL) {
		tprintf("Memory overflow.\n");
		return;	
	}
	
	for (i = 0; i < numSeq; i++) {
		tmp[i] = SEQUENCE_clone_multi(seq[i],numRepetitions);
		if (tmp[i] == NULL) {
			tprintf("Memory overflow or bug.\n");
			return;	
		}
	}
	
	
	CmtGetLock(lockSequenceQueue);
	
	k = 0;
	for (j= 0; j < numRepetitions; j++) {
		for (i = 0; i < numSeq; i++) {
			tprintf("%d %d\n",i,j);
			ProcessDrawEvents();
			ListInsertItem(listOfWaitingSequences,&tmp[i][j],END_OF_LIST);
			if (k % idleEvery == idleEvery-1) {
				ListInsertItem(listOfWaitingSequences,&idle,END_OF_LIST);
			} 
			k++;
			
			
			
		}	
	}
		
	CmtReleaseLock(lockSequenceQueue);
	free(tmp);
}

void SEQUENCE_QUEUE_InsertIdle(int pos) {
	SEQUENCE_QUEUE_Insert(NULL, pos);	
}

void SEQUENCE_QUEUE_Delete(int pos) {
	t_sequence* tmp;
	
	CmtGetLock(lockSequenceQueue);
	if (pos < 1 || pos > (int)ListNumItems(listOfWaitingSequences)) {
	} else {
		ListRemoveItem(listOfWaitingSequences,&tmp,pos);
		if (tmp != NULL) {
			SEQUENCE_free(tmp);
			free(tmp);
		}
	}
	CmtReleaseLock(lockSequenceQueue);
}

void SEQUENCE_QUEUE_DeleteAll() {
	t_sequence* tmpSeq;
	int i;
	
	CmtGetLock(lockSequenceQueue); 
	for (i = ListNumItems (listOfWaitingSequences); i > 0; i--) {
		ListGetItem (listOfWaitingSequences, &tmpSeq, i);
		if (tmpSeq != NULL) {
			SEQUENCE_free(tmpSeq);
			free(tmpSeq);
		}
	}
	ListClear(listOfWaitingSequences);
	CmtReleaseLock(lockSequenceQueue); 
}

// attention: can return NULL !!!
void SEQUENCE_QUEUE_Get(t_sequence** seqPtr, int pos) {
	ListGetItem(listOfWaitingSequences,seqPtr,pos);
	if (*seqPtr == NULL) {
		*seqPtr = SEQUENCE_ptr(config->idleSequenceNr);	
	}
}


/************************************************************************/
/*
/*    initializaiton of panels etc. 
/*
/************************************************************************/

//=======================================================================
//
//    return panel handles 
//    (serves as interface to other files)
//
//=======================================================================




int MAIN_PanelMinimumWidth (void)
{
    return 250;
}



//=======================================================================
//
//    resize panel
//
//=======================================================================
void MAIN_resizePanel (t_sequence *seq)
{
    //Rect tabBounds;
    //int height, width;
    //int heightControlsBelow;
	
	// ------------------------------------------
	//    set size of listbox of sequences
	// -----------------------------------------
//	SetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences,
//					  ATTR_WIDTH, width);
	
/*	SetCtrlAttribute (panelMain, MAIN_DECORATION2, ATTR_TOP,
				      ctrlBottom (panelMain, MAIN_LISTBOX_Sequences));
	SetCtrlAttribute (panelMain, MAIN_DECORATION2, ATTR_LEFT,
				      panelWidth(panelMain)-ctrlWidth (panelMain, MAIN_DECORATION2));
	SetCtrlAttribute (panelMain, MAIN_DECORATION2, ATTR_HEIGHT,
				      panelHeight(panelMain)-ctrlTop (panelMain, MAIN_DECORATION2));

	SetCtrlAttribute (panelMain, MAIN_DECORATION1, ATTR_TOP,
				      ctrlBottom (panelMain, MAIN_LISTBOX_Sequences));
	SetCtrlAttribute (panelMain, MAIN_DECORATION1, ATTR_WIDTH,
				      ctrlLeft(panelMain, MAIN_DECORATION2)
					  - ctrlLeft(panelMain, MAIN_DECORATION1));
*/	// ------------------------------------------
	//    place output windows
	// -----------------------------------------
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output1, ATTR_LEFT, 0); 
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output1, ATTR_TOP, 
					  ctrlBottom (panelMain, MAIN_TREE_sweeps));
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output1, ATTR_HEIGHT, 
					  panelHeight(panelMain)-ctrlTop (panelMain, MAIN_TEXTBOX_Output1));
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output1, ATTR_LEFT, 0);
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output1,
					  ATTR_WIDTH, round(panelWidth(panelMain) / 2.5));

	setCtrlPos (panelMain, MAIN_TEXTBOX_Output2,
				ctrlTop(panelMain, MAIN_TEXTBOX_Output1),
				ctrlRight(panelMain, MAIN_TEXTBOX_Output1));
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output2,
					  ATTR_WIDTH,
					  ctrlLeft(panelMain, MAIN_DECORATION_5)
					  -ctrlRight(panelMain, MAIN_TEXTBOX_Output1));
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_Output2,
					  ATTR_HEIGHT,
					  ctrlHeight (panelMain, MAIN_TEXTBOX_Output1));
	GPIB_setOutputCtrl (panelMain, MAIN_TEXTBOX_Output2);
	setStdTextOut (panelMain, MAIN_TEXTBOX_Output1);
	// ------------------------------------------
	//    set panel attributes
	// -----------------------------------------
//	SetPanelAttribute (panelMain, ATTR_MIN_HEIGHT_FOR_SCALING,
//					   panelHeight(panelMain)+20);
//	SetPanelAttribute (panelMain, ATTR_MIN_WIDTH_FOR_SCALING, 500); 
	

}    



void MAIN2_resizePanel (t_sequence *seq)
{
	// ------------------------------------------
	//    resize tab control
	// -----------------------------------------
	SetCtrlAttribute (panelMain2, MAIN2_TAB0, ATTR_LEFT, 0);
	SetCtrlAttribute (panelMain2, MAIN2_TAB0, ATTR_HEIGHT,
					  panelHeight(panelMain2) - ctrlTop(panelMain2, MAIN2_TAB0) );
	SetCtrlAttribute (panelMain2, MAIN2_TAB0, ATTR_WIDTH, panelWidth(panelMain2));


    DIGITALBLOCKS_resizePanel ();
    WFM_resizePanel ();
	GPIB_resizePanel ();
	CONFIG_resizePanel ();
}    


//=======================================================================
//
//    init submenu for sequence manager
//
//=======================================================================
int MAIN_initMenus (void)
{
	
	menuSequenceList = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL5));
	if (menuSequenceList < 0) return -1;
	menuSequenceQueue = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANE10));
	if (menuSequenceQueue < 0) return -1;
	

	return 0;
}



//=======================================================================
//
//    init panel "Main"
//
//=======================================================================
int MAIN_initPanel (int outPanel, int outCtrl)
{
 	 int color;
	 //int h;
	 //int i;
	 //char str[20];
	 int top, left;
 	 
 	// char metaFont[] = "Tab Edit Meta Font 2";
 	 const char strInitializing[] = "Initializing user interface...";

    if (panelMain != -1) return panelMain;
	
 	setCtrlStr (outPanel, outCtrl, strInitializing);
	
	listOfWaitingSequences = ListCreate(sizeof (t_sequence*));
	  
	
    
	// ------------------------------------------
	//    load main panel
	// -----------------------------------------
	panelMain = LoadPanel (0, UIR_File, MAIN);
	
	SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	
	SetPanelAttribute(panelMain, ATTR_SCALE_CONTENTS_ON_RESIZE, FALSE);
	//SetPanelSize(panelMain,848,1674);
	//SetPanelPos(panelMain,44,67);
	
	//SetPanelPos (panelMain, 25, 5);
//	SetPanelSize (panelMain, screenHeight()-100, 
//				  (screenWidth()/3)*2);
	
	// show tasbbar button for this pabel
	SetPanelAttribute(panelMain, ATTR_HAS_TASKBAR_BUTTON, TRUE);
	//SetPanelAttribute(panelMain, ATTR_MINIMIZE_OTHER_PANELS, TRUE);
	
	color = VAL_PANEL_GRAY;
	GetPanelAttribute (panelMain, ATTR_BACKCOLOR, &color);
	
	SetCtrlAttribute (panelMain, MAIN_NUM_repetitions,
					  ATTR_TEXT_BGCOLOR, VAL_YELLOW);
	SetCtrlAttribute (panelMain, MAIN_TEXTBOX_description,
					  ATTR_TEXT_BGCOLOR, VAL_YELLOW);

	
	SetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences,
					  ATTR_TOP, panelMenuHeight(panelMain));
	
	SetCtrlAttribute (panelMain, MAIN_BTN_startSequence,
	                 ATTR_CMD_BUTTON_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelMain, MAIN_BTN_startSequence,
	                 ATTR_VISIBLE, 1);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence,
	                 ATTR_CMD_BUTTON_COLOR, VAL_RED);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence,
	                 ATTR_VISIBLE, 0);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence, ATTR_TOP,
					  ctrlTop (panelMain, MAIN_BTN_startSequence));
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence, ATTR_LEFT,
					  ctrlLeft (panelMain, MAIN_BTN_startSequence));
		
	SetCtrlAttribute (panelMain, MAIN_BTN_startIdle,
	                 ATTR_CMD_BUTTON_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelMain, MAIN_BTN_startIdle,
	                 ATTR_VISIBLE, 1);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle,
	                 ATTR_CMD_BUTTON_COLOR, VAL_RED);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle,
	                 ATTR_VISIBLE, 0);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle, ATTR_TOP,
					  ctrlTop (panelMain, MAIN_BTN_startIdle));
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle, ATTR_LEFT,
					  ctrlLeft (panelMain, MAIN_BTN_startIdle));

	
	
	EnableDragAndDrop (panelMain);	
	

	SetCtrlAttribute (panelMain, MAIN_LED_TCP_send,
					  ATTR_ON_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelMain, MAIN_LED_TCP_send,
					  ATTR_OFF_COLOR, VAL_BLACK);
	SetCtrlAttribute (panelMain, MAIN_LED_TCP_receive,
					  ATTR_ON_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelMain, MAIN_LED_TCP_receive,
					  ATTR_OFF_COLOR, VAL_BLACK);
	SetCtrlAttribute (panelMain, MAIN_LED_gpib,
					  ATTR_ON_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelMain, MAIN_LED_gpib,
					  ATTR_OFF_COLOR, VAL_BLACK);
	
	WATCHDOG_setLedCtrl2 (panelMain, MAIN_LED_watchdog);

	SetCtrlAttribute (panelMain, MAIN_NUMERICSLIDE_progress, ATTR_FILL_COLOR, VAL_GREEN);
	SetCtrlAttribute (panelSequence, SEQUENCE_SLIDE_progress2, ATTR_FILL_COLOR, VAL_GREEN);
	
	
	TCP_serverSetLed (panelMain, MAIN_LED_TCP_send, MAIN_LED_TCP_receive);
	
	top = ctrlBottom (panelMain, MAIN_DECORATION_2);
	left = 2;
	top = ctrlBottom (panelMain, MAIN_DECORATION_2);


#ifndef _CVI_DEBUG_   
	SetCtrlAttribute (panelMain, MAIN_NUMERIC_channel, ATTR_VISIBLE, 0);
	SetCtrlAttribute (panelMain, MAIN_NUMERIC_offset, ATTR_VISIBLE, 0);
#endif
	
	WATCHDOG_setLedCtrl1 (panelMain, MAIN_LED_watchdog);
	// ------------------------------------------
	//    init menubar
	// -----------------------------------------
 	setCtrlStrf (outPanel, outCtrl, "%s    %s", strInitializing, "MENUS");
	if (MAIN_initMenus () != 0) return -1;
    MAIN_initTriggerSources (panelMain, MAIN_RING_TriggerSource);
	SEQUENCE_initializeListBox ();
	PARAMETERSWEEP_initListCtrl (panelMain, MAIN_TREE_sweeps);
	if (config->panelBoundsMain.width > 0) {
		SetPanelPos (panelMain, config->panelBoundsMain.top, config->panelBoundsMain.left);
	}
	
   	MAIN_resizePanel (NULL);
	
	cls (panelMain, MAIN_TEXTBOX_Output1);
	
   	CONFIG_initPanel ();
	OUTPUT_showPanel (NULL);
	
	//
	//  idel wait
	//
	panelIdleWait = LoadPanel (0, UIR_File, IDLEWAIT);
	SetCtrlAttribute (panelIdleWait, IDLEWAIT_BTN_stopSequence,
	                 ATTR_CMD_BUTTON_COLOR, VAL_RED);
	

	SEQUENCE_startThread(); // start thread if not running    
	
	
	
	// load addressing panel here - but do not show it yet
	if (panelAddressing == 0) {
		panelAddressing = LoadPanel (0, UIR_File, ADDRESS);	
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH2,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH_voltages,ATTR_REFRESH_GRAPH,0); 
	}
	
	return 0;        
}										 


int MAIN2_initPanel (int outPanel, int outCtrl)
{
 	 //int color;
	 //int h;
	 //int i;
	 //char str[20];
 	 
 	 //char metaFont[] = "Tab Edit Meta Font 2";
 	 const char strInitializing[] = "Initializing user interface...";

    if (panelMain2 != -1) return panelMain2;
	
 	setCtrlStr (outPanel, outCtrl, strInitializing);
    
	// ------------------------------------------
	//    load main panel
	// -----------------------------------------
	panelMain2 = LoadPanel (0, UIR_File, MAIN2);
	
	
	// show tasbbar button for this pabel
	SetPanelAttribute(panelMain2, ATTR_HAS_TASKBAR_BUTTON, TRUE);
	//SetPanelAttribute(panelMain2, ATTR_MINIMIZE_OTHER_PANELS, TRUE);

	// ------------------------------------------
	//    load subpanels:
	//    digital blocks, analog blocks, waveforms
	// -----------------------------------------
	
	GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_SEQUENCE,
							   &panelSequence);
	GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_WFM,
							   &panelWaveforms);
	GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_GPIB,
							   &panelGpib);
	GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_CCD,
							   &panelCCD);
	
	GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_WFM,
							   &panelWaveforms);
	
	setCtrlPos (panelMain2, MAIN2_TAB0, 0, 0);
	// ------------------------------------------
	//    init subpanels 
	// -----------------------------------------
 	setCtrlStrf (outPanel, outCtrl, "%s    %s", strInitializing, "DIGITALBLOCKS");
	DIGITALBLOCKS_initPanel ();
 	setCtrlStrf (outPanel, outCtrl, "%s    %s", strInitializing, "WAVEFORMS");
   	WFM_initPanel ();   
 	setCtrlStrf (outPanel, outCtrl, "%s    %s", strInitializing, "GPIB");
   	GPIB_initPanel ();
	CCD_initPanel ();	
	SetActiveTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_SEQUENCE);
	SetPanelAttribute (panelMain2,ATTR_MIN_HEIGHT_FOR_SCALING, 400);
	SetPanelAttribute (panelMain2,ATTR_MIN_WIDTH_FOR_SCALING, 500);
	if (config->panelBoundsMain2.width > 0) {
		setPanelBounds (panelMain2, config->panelBoundsMain2, 1);
	}
	if (config->panelMain2IsZoomed) {
		SetPanelAttribute (panelMain2, ATTR_WINDOW_ZOOM, VAL_MAXIMIZE);
	}
   	MAIN2_resizePanel (NULL);
//	DisplayPanel (panelMain2);
	return 0;        
}										 


//=======================================================================
//
//    main panel callback
//
//=======================================================================
int CVICALLBACK MAIN_panelChanged (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
//	if (event != EVENT_MOUSE_POINTER_MOVE) tprintf ("%s\n", eventStr (event, eventData1, eventData2));
	
	t_sequence *startSeq;
	char **filelist;
	int i;
	int panelMain2WindowZoom;
	int tmp;
	
//	if (panel != panelMain) {
//		exit(0);
//	}
		
     switch (event)
        {
        case EVENT_PANEL_SIZE:
		case EVENT_PANEL_MOVE:
			getPanelBounds (panelMain, &config->panelBoundsMain);
//			if (config->panelBoundsMain.width < 850) config->panelBoundsMain2.width = 850;
//			/if (config->panelBoundsMain.height < 550) config->panelBoundsMain2.height = 550;
//			setPanelBounds (panelMain2, config->panelBoundsMain2, 0);
			CONFIG_write (config);
			
		
			
            break;
		case EVENT_PANEL_MINIMIZE:
			/*GetPanelAttribute(panelMain2,ATTR_WINDOW_ZOOM,&panelMain2WindowZoom); 
			if (panelMain2WindowZoom != VAL_MINIMIZE) {
				SetPanelAttribute(panelMain2,ATTR_WINDOW_ZOOM,VAL_MINIMIZE);
				//SetActivePanel(panelMain);  
			}*/
			break;
		case EVENT_PANEL_RESTORE:
			/*GetPanelAttribute(panelMain2,ATTR_WINDOW_ZOOM,&panelMain2WindowZoom); 
			if (panelMain2WindowZoom != VAL_NO_ZOOM) {
				SetPanelAttribute(panelMain2,ATTR_WINDOW_ZOOM,VAL_NO_ZOOM);
				//SetActivePanel(panelMain);  
			}*/
			
			SetPanelPos(panelMain,44,67);
			SetPanelSize(panelMain,848,1674);

		//	PostDeferredCallToThread (MAIN_panelResizeToDefault, 0, CmtGetMainThreadID ());
			
			
			//GetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,&panelMainWindowZoom);
			//if (panelMainWindowZoom != VAL_NO_ZOOM) {
			//	SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM); 
				//setPanelBounds (panelMain, config->panelBoundsMain, 0);
			//}
			//return 1;
			break;
		case EVENT_GOT_FOCUS:
			displayPanel2 (panelMain2);
			DisplayPanel (panelMain);
			SetPanelAttribute (panelMain, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
			break;
        case EVENT_LOST_FOCUS:
            break;
        case EVENT_CLOSE:
			quitProgram (0);
			break;
   		case EVENT_KEYPRESS:
			switch (eventData1) {
				case VAL_F5_VKEY:
				case VAL_F6_VKEY:
				case VAL_F7_VKEY:
				case VAL_F8_VKEY:
				    if ((startSeq = SEQUENCE_fromShortcut(eventData1)) == NULL) break;
					startSeq->remote = 0;
					SEQUENCE_startOrEnqueue (startSeq);
					break;
			}
			
			break;
		case EVENT_FILESDROPPED:
			filelist = (char **) eventData1;
			i = 0;
			while (filelist[i] != NULL) {
				MAIN_loadSequence (filelist[i]);
				i++;
			}
			
			
			
			break;
		case EVENT_DISCARD:
			return 1;
			
        }
    return 0;
}



int CVICALLBACK MAIN2_panelChanged (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
     int zoom;
	 t_sequence *startSeq;
	 int panelMainWindowZoom;
     
     switch (event)
        {
        //case EVENT_PANEL_SIZING:
//			MAIN2_resizePanel (activeSeq());
//			break;
			
        case EVENT_PANEL_MOVE:
		case EVENT_PANEL_SIZE:
			GetPanelAttribute (panelMain2, ATTR_WINDOW_ZOOM, &zoom);
			config->panelMain2IsZoomed = (zoom == VAL_MAXIMIZE);
			if (zoom != VAL_MINIMIZE) {
//				if (!config->panelMain2IsZoomed) {
					getPanelBounds (panelMain2, &config->panelBoundsMain2);
					if (config->panelBoundsMain2.width < 850) config->panelBoundsMain2.width = 850;
					if (config->panelBoundsMain2.height < 550) config->panelBoundsMain2.height = 550;
					setPanelBounds (panelMain2, config->panelBoundsMain2, 0);
//				}
				MAIN2_resizePanel (activeSeq());
				SetActivePanel (panelMain2);
				CONFIG_write (config);
			}
			CONFIG_write (config);
            break;
        case EVENT_LOST_FOCUS:
			HidePanel (panelWfmSmall);
            break;
        case EVENT_CLOSE:
			quitProgram (0);
			break;
   		case EVENT_KEYPRESS:
			switch (eventData1) {
				case VAL_F5_VKEY:
				case VAL_F6_VKEY:
				case VAL_F7_VKEY:
				case VAL_F8_VKEY:
				case VAL_F9_VKEY:
				    if ((startSeq = SEQUENCE_fromShortcut(eventData1)) == NULL) break;
					startSeq->remote = 0;
					SEQUENCE_startOrEnqueue (startSeq);
					break;
				case VAL_CTRL_S_KEY:
					MENU_Save (0,0,0,0);
					break;
				case VAL_CTRL_O_KEY:
					MENU_open (0,0,0,0);
					break;
				case VAL_CTRL_I_KEY:
					MENU_ImportNames (0,0,0,0);
					break;
			}
			break;
		case EVENT_PANEL_MINIMIZE:
			/*GetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,&panelMainWindowZoom); 
			if (panelMainWindowZoom != VAL_MINIMIZE) {
				SetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,VAL_MINIMIZE);
				//SetActivePanel(panelMain2);
			}*/
			break;
		case EVENT_PANEL_RESTORE:
			/*GetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,&panelMainWindowZoom); 
			if (panelMainWindowZoom != VAL_NO_ZOOM) {
				SetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,VAL_NO_ZOOM);
				//SetActivePanel(panelMain2);
			}*/
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
//    Trigger sources 
//
//=======================================================================

void MAIN_initTriggerSources (int panel, int ctrl)
{
    int i;
    ClearListCtrl (panel, ctrl);
    for (i = 0; i < N_TRIGGER_SOURCES; i++) {
        InsertListItem (panel, ctrl, -1, HARDWARE_signalName (HARDWARE_TRIGGER_SOURCE[i]), HARDWARE_TRIGGER_SOURCE[i]);
    }
    SetCtrlVal (panel, ctrl, VAL_NO_TRIGGER);
}




//=======================================================================
//
//    show timebases for hardware boards 
//
//=======================================================================
void MAIN_displayTimebases (t_sequence *seq)
{
   	//unsigned long DIOtimebase, AOtimebase;

	
	SetCtrlVal (panelMain, MAIN_TEXTBOX_DIOTimebase, 
					   strTime(seq->DIO_timebase_50ns));
	SetCtrlVal (panelMain, MAIN_TEXTBOX_AOTimebase, 
					   strTime(seq->AO_timebase_50ns));
	SetCtrlVal (panelMain, MAIN_RING_TriggerSource,
			   seq->trigger);
}


//=======================================================================
//
//    display current filename in title
//
//=======================================================================
void MAIN_setFilenameToTitle (const char *filename, int changes)
{
	char help[MAX_PATHNAME_LEN];

#ifdef _CVI_DEBUG_        	
	sprintf (help, "DEBUG Experiment Control V%1.2f - %s -", ProgramVersion, __DATE__);
#else
	sprintf (help, "Experiment Control V%1.2f - %s -", ProgramVersion, __DATE__);
#endif
	strcat (help, filename);
	if (changes) strcat (help, " *");
	SetPanelAttribute (panelMain, ATTR_TITLE, help);
	
	sprintf (help, "%s%s", extractFilename (filename), changes ? " *" : "");
	SetPanelAttribute (panelMain2, ATTR_TITLE, help);
}


void MAIN_setMenuBarStatus (int dimmed)
{
	int handle = GetPanelMenuBar(panelMain);
	if (handle != 0) {
		SetMenuBarAttribute (handle, 0, ATTR_DIMMED, dimmed);
	}
}


//=======================================================================
//
//    set "change" attribute
//    (if changes are made, display "*" in title bar)
//
//=======================================================================
void setChanges (t_sequence *seq, int changes)
{
//    if (seq->changes == changes) return;
    if (seq == activeSeq()) {
		MAIN_setFilenameToTitle (seq->filename, changes);
		PARAMETERSWEEPS_updateDisplay (seq);
	}

    if (seq->changes != changes) {
        seq->changes = changes;
        SEQUENCE_displayList ();
    }
}


//=======================================================================
//
//   checks whether the active control should react to inputs
//   if sequence is being executed
//
//   return value: 1:protected,  0=not protected
//
//=======================================================================
int isCurrentControlProtected (void)
{
    int i;
    int panel, control;

	// ---------------------------------------------
	//   list of controls in main window
	// ---------------------------------------------
	#define nMainCtrls 1
    int protMainCtrls[nMainCtrls] = {
        MAIN_NUM_repetitions
    };

	// ---------------------------------------------
	//   get active control
	// ---------------------------------------------
    panel = GetActivePanel();
    control = GetActiveCtrl (panel);
//    printf ("panel: %d,  control: %d\n", panel, control);
    
	// ---------------------------------------------
	//   check if active control is in main window? 
	// ---------------------------------------------
    if (panel == panelMain) {
	    for (i = 0; i < nMainCtrls; i++) 
	        if (control == protMainCtrls[i]) return 1;
// DEBUG
//        for (i = 0; i < N_DIO_Channels; i++) 
//	        if (control == CTRL_DigitalChannelNames[i]) return 1;
	}
    
    return 0;

	#undef nMainCtrls 
}


/************************************************************************/
/************************************************************************/
/*
/*    sequence handling 
/*
/************************************************************************/
/************************************************************************/

int getActiveSeqNr (void)
{
    return config->activeSequenceNr;
}





// ---------------------------------------------
//   display active sequence  
// ---------------------------------------------
void MAIN_displayActiveSequence (void)
{
	
	t_sequence *seq;
	
	seq = activeSeq();
	if (seq == NULL) return;
	
	SetAttributeForCtrls (panelSequence, ATTR_VISIBLE, 0, 0,
						  SEQUENCE_TABLE_header2, 
					      SEQUENCE_TABLE_dac1,						  
						  SEQUENCE_TABLE_dac2, 
					      SEQUENCE_TABLE_digital1,						  
						  SEQUENCE_TABLE_digital2, 
						  0);
	ProcessDrawEvents();
//	DeleteTableColumns (panelSequence, SEQUENCE_TABLE_dac2, 1, -1);
//	DIGITALBLOCKS_insertTableColumns (panelSequence, 0, SEQUENCE_TABLE_dac2, 0, 
//										 1, ListNumItems (seq->lDigitalBlocks));

	
	EnableBreakOnLibraryErrors ();
	
	
//	DIGITALBLOCKS_resetAllTableAttributes (seq);

	DIGITALBLOCKS_displayAllBlocks (seq);

	
	if (seq->description != NULL) ResetTextBox (panelMain, MAIN_TEXTBOX_description, seq->description);
	else ResetTextBox (panelMain, MAIN_TEXTBOX_description, "");

	SetCtrlVal (panelMain, MAIN_NUM_repetitions,
				seq->nRepetitions);
	SetCtrlVal (panelSequence, SEQUENCE_NUMERIC_repOf,
				seq->nRepetitions);
	
	SetCtrlAttribute (panelMain, MAIN_NUM_copies,
					  ATTR_CTRL_VAL, seq->nCopies);  
	SetCtrlVal (panelMain, MAIN_STRING_seqShortName, seq->seqShortName);
	if (seq->filename != 0) {
		SetCtrlVal (panelMain, MAIN_STRING_activeSeqPath, seq->filename);              
	}
							  
	SetAttributeForCtrls (panelSequence, ATTR_VISIBLE, 1, 0,
						  SEQUENCE_TABLE_header2, 
					      SEQUENCE_TABLE_dac1,						  
						  SEQUENCE_TABLE_dac2, 
					      SEQUENCE_TABLE_digital1,						  
						  SEQUENCE_TABLE_digital2, 
						  0);
				
	GPIB_displayValues (seq, -1);  
	MAIN_resizePanel (seq);
	MAIN_displayTimebases (seq);
	CONFIG_displayAllSettings (seq);
	WFM_TABLE_displayAllWfms (panelWaveforms, WFM_TABLE_wfm, seq, 1, 1);
	CCD_displayAll (seq);	

//    WFM_fillNamesToListbox (seq, panelWaveforms, WFM_LISTBOX_Waveforms, 1);
	

    WFM_edit (seq, 1, 1);
	SEQUENCE_displayList ();      
	
}


int nSequences (void)
{
	return ListNumItems (config->listOfSequences);
}



// ---------------------------------------------
//   set 'nr' as active sequence  
// ---------------------------------------------
void SEQUENCE_setAsActive (int nr)
{
	 if (nr > nSequences() || (nr < 1)) nr = nSequences();
	 config->activeSequenceNr = nr;
	 MAIN_displayActiveSequence ();
}


void SEQUENCE_setAsIdle (int nr)
{
	 if (nr > nSequences() || (nr < 1)) nr = nSequences();
	 config->idleSequenceNr = nr;
	 SEQUENCE_displayList ();
}


void SEQUENCE_setAsCalibration (int nr)
{
	 if (nr > nSequences() || (nr < 1)) nr = nSequences();
	 if (config->idleSequenceNr == nr) config->idleSequenceNr = 0;
	 config->calibrationSequenceNr = nr;
	 SEQUENCE_displayList ();
}


// ---------------------------------------------
//   returns pointer to the active sequence = the sequence being edited
// ---------------------------------------------
t_sequence *activeSeq (void)
{
	return SEQUENCE_ptr(config->activeSequenceNr);
}    


// ---------------------------------------------
//   returns pointer to the running sequence = the sequence being executed
// ---------------------------------------------
t_sequence *runningSeq(void)
{
	return runningSequence;
}    



// ---------------------------------------------
//    checks if filename already exists
// ---------------------------------------------
int SEQUENCE_nrFromFilename (char *filename)
{
	int i;
    
    for (i = 1; i <= nSequences(); i++)
    {
		 if (CompareStrings (filename, 0, SEQUENCE_ptr(i)->filename, 0, 0) == 0)
		     return i;
	}
	
	return 0;
}



t_sequence *SEQUENCE_fromFilename (char *filename)
{
	int i;
	t_sequence *s;
    
    for (i = 1; i <= nSequences(); i++)
    {
		 s = SEQUENCE_ptr(i);
		 if (CompareStrings (filename, 0, s->filename, 0, 0) == 0)
		     return s;
	}
	return NULL;
}



// ---------------------------------------------
//   initialize listbox
// ---------------------------------------------
void SEQUENCE_initializeListBox (void) 
{
	
	DeleteListItem (panelMain, MAIN_LISTBOX_Sequences, 0, -1);
	SEQUENCE_displayList ();
}

// ---------------------------------------------
//   create list entry
// ---------------------------------------------
const char *SEQUENCE_createListEntry (t_sequence *seq)
{
	#define bufLen 400
    static char buf[bufLen+1];
	
	int bgcolor = VAL_WHITE;
	int fgcolor = VAL_BLACK;
	int textHeight, tab;
	//char help[200];
    
    if (seq == activeSeq()) {
       bgcolor = VAL_GREEN;
       fgcolor = VAL_BLACK;
    }
	
	strcpy (buf, "");
	if (seq == NULL || seq == SEQUENCE_ptr(config->idleSequenceNr)) {
       bgcolor = 0xFFE1FF;
       fgcolor = VAL_BLACK;
   	   appendColorSpec (buf, bgcolor, fgcolor);
		strcat (buf, "IDLE");
	}
	else if (seq == SEQUENCE_ptr(config->calibrationSequenceNr)) {
       bgcolor = VAL_LT_BLUE;
       fgcolor = VAL_BLACK;
   	   appendColorSpec (buf, bgcolor, fgcolor);
		strcat (buf, "CALB");
	}
	else {
	   appendColorSpec (buf, bgcolor, fgcolor);
	   switch (seq->shortcut) {
	        case VAL_F5_VKEY:
	        	strcat (buf, " F5 ");
	        	break;
	        case VAL_F6_VKEY:
	        	strcat (buf, " F6 ");
	        	break;
	        case VAL_F7_VKEY:
	        	strcat (buf, " F7 ");
	        	break;
	        case VAL_F8_VKEY:
	        	strcat (buf, " F8 ");
	        	break;
	        default: 
				if ((seq->startNr > 0) && (strcmp(seq->startDate, CONFIG_getPathSuffixToday())==0)) {
					strcat (buf, " ");
					strcat (buf, intToStr0 (3, seq->startNr));
				}	
				else strcat (buf, "     ");
	    }
        	
	    appendColorSpec (buf, bgcolor, fgcolor);
	}
    GetTextDisplaySize (" F8 ", VAL_EDITOR_META_FONT, &textHeight, &tab);

    appendJustification (buf, tab, LeftJustify);
    appendVLine (buf);
	if (seq != NULL) {
    	//strcat (buf, strAddLeadingSpaces (intToStr(seq->nRepetitions), 5));
		snprintf(buf,bufLen,"%s%5d", buf,seq->nRepetitions);
	} else {
		//strcat (buf, strAddLeadingSpaces (intToStr(1), 5));	
		snprintf(buf,bufLen,"%s%5d", buf,1);   
	}
    appendVLine (buf);
    
	if (seq != NULL && seq->readOnly) {
    	appendColorSpec (buf, VAL_LT_BLUE, fgcolor);
 		strcat (buf, "»");  
    	appendColorSpec (buf, bgcolor, fgcolor);
	}
	else strcat (buf, " ");

//	strcat (buf, seq->writeProtected ? " " : " ");
//	strncat (buf, extractFilename (seq->filename), 40);
	if (seq != NULL) {
		strncat (buf, seq->seqShortName, 40);
	}
	if (seq != NULL && seq->readOnly) {
    	appendColorSpec (buf, VAL_LT_BLUE, fgcolor);
 		strcat (buf, "«");  
    	appendColorSpec (buf, bgcolor, fgcolor);
	}
	else strcat (buf, " ");

	strcat (buf, (seq != NULL && seq->changes) ? " *" : "  ");
	strncat (buf, "                                     ", 50-strlen (buf));
    appendJustification (buf, tab+300, LeftJustify);
    appendVLine (buf);
	if (seq != NULL) {
		strcat (buf, seq->dateLastModified);	
	}
	appendVLine (buf);
	if (seq != NULL) {
    	strncat(buf, seq->filename, 50);   
	}
	//strncat (buf, extractDir (seq->filename), 50);

    if (seq != NULL && seq->changes) strcat (buf, " *");
    strncat (buf, "                                            ",400);
    return buf;
}



// ---------------------------------------------
//   Insert or overrides a list item
// ---------------------------------------------
void SEQUENCE_OverrideListItem (int panel, int ctrl, int index, const char *txt, int value)
{
	int nLines;
	GetNumListItems (panel, ctrl, &nLines);
	if (index < nLines) 
        ReplaceListItem (panel, ctrl, index, txt, value);
    else
        InsertListItem (panel, ctrl, index, txt, value);
}
 

// ---------------------------------------------
//   display the list of waiting sequences
// ---------------------------------------------
void SEQUENCE_displayQueue ()
{	 
	int i;
    int numVisibleLines, firstVisibleLine;
	int textHeight, tab;
	char blankItem[100];
	int nLines;
	t_sequence *seq;
	//int readOnly, dummy;
	
	CmtGetLock(lockSequenceQueue);
	
	firstVisibleLine = 0;
    
	GetNumListItems (panelMain, MAIN_LISTBOX_SequenceQueue, &nLines);
    for (i = 1; i <= ListNumItems (listOfWaitingSequences); i++)
    {
		SEQUENCE_QUEUE_Get(&seq,i);
		if (seq != NULL) {
			isFileWriteProtected (seq->filename, &seq->readOnly);
		}
        SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_SequenceQueue, i-1, 
		       SEQUENCE_createListEntry(seq), i);
//		 if (i == config->activeSequenceNr) DebugPrintf ("*");
//		 DebugPrintf ("%d:%s\n", i, extractFilename(SEQUENCE_ptr(i)->filename));
    }
	CmtReleaseLock(lockSequenceQueue);
	
//	DebugPrintf ("\n");
    
    // fill rest with blank lines
    GetTextDisplaySize (" F8 ", VAL_EDITOR_META_FONT, &textHeight, &tab);
	strcpy (blankItem, "    ");
    appendJustification (blankItem, tab, LeftJustify);
    appendVLine (blankItem);    
    strcat (blankItem, "     ");
    appendVLine (blankItem);    
    strcat (blankItem, "                                              ");
    GetCtrlAttribute (panelMain, MAIN_LISTBOX_SequenceQueue, ATTR_VISIBLE_LINES, &numVisibleLines);
    for (i = ListNumItems (listOfWaitingSequences); i < numVisibleLines; i++) {
        SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_SequenceQueue, i, blankItem, 0);
    }
	if (i < nLines) 
		DeleteListItem (panelMain, MAIN_LISTBOX_SequenceQueue, i, -1);
	
	
	SetCtrlAttribute (panelMain, MAIN_LISTBOX_SequenceQueue,
					  ATTR_FIRST_VISIBLE_LINE, firstVisibleLine);
	
	//GetCtrlAttribute (panelMain, MAIN_LISTBOX_SequenceQueue,
	//				  ATTR_FIRST_VISIBLE_LINE, &firstVisibleLine);
/*	if ((config->activeSequenceNr-1 > firstVisibleLine+numVisibleLines) || 
		(config->activeSequenceNr-1 < firstVisibleLine)) {
		if (nSequences() < numVisibleLines) firstVisibleLine = 0;
		else firstVisibleLine = min(config->activeSequenceNr+numVisibleLines/2, nSequences()-numVisibleLines);
		SetCtrlAttribute (panelMain, MAIN_LISTBOX_SequenceQueue,
						  ATTR_FIRST_VISIBLE_LINE, firstVisibleLine);
	}   */

}



// ---------------------------------------------
//   display the list of sequences
// ---------------------------------------------
void SEQUENCE_displayList()
{
    int i;
    int numVisibleLines, firstVisibleLine;
	int textHeight, tab;
	char blankItem[100];
	int nLines;
	t_sequence *seq;
	//int readOnly, dummy;
    
	GetNumListItems (panelMain, MAIN_LISTBOX_Sequences, &nLines);
    for (i = 1; i <= nSequences(); i++)
    {
		seq = SEQUENCE_ptr(i);
		isFileWriteProtected (seq->filename, &seq->readOnly);
        SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_Sequences, i-1, 
		       SEQUENCE_createListEntry (seq), i);
//		 if (i == config->activeSequenceNr) DebugPrintf ("*");
//		 DebugPrintf ("%d:%s\n", i, extractFilename(SEQUENCE_ptr(i)->filename));
    }
//	DebugPrintf ("\n");
    
    // fill rest with blank lines
    GetTextDisplaySize (" F8 ", VAL_EDITOR_META_FONT, &textHeight, &tab);
	strcpy (blankItem, "    ");
    appendJustification (blankItem, tab, LeftJustify);
    appendVLine (blankItem);    
    strcat (blankItem, "     ");
    appendVLine (blankItem);    
    strcat (blankItem, "                                              ");
    GetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences, ATTR_VISIBLE_LINES, &numVisibleLines);
    for (i = nSequences(); i < numVisibleLines; i++) {
        SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_Sequences, i, blankItem, 0);
    }
	if (i < nLines) 
		DeleteListItem (panelMain, MAIN_LISTBOX_Sequences, i, -1);
	
	GetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences,
					  ATTR_FIRST_VISIBLE_LINE, &firstVisibleLine);
	if ((config->activeSequenceNr-1 > firstVisibleLine+numVisibleLines) || 
		(config->activeSequenceNr-1 < firstVisibleLine)) {
		if (nSequences() < numVisibleLines) firstVisibleLine = 0;
		else firstVisibleLine = min(config->activeSequenceNr+numVisibleLines/2, nSequences()-numVisibleLines);
		SetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences,
						  ATTR_FIRST_VISIBLE_LINE, firstVisibleLine);
	}
	SEQUENCE_displayQueue();
}


// Thread which handles are running of sequences including idle sequence
int CVICALLBACK SEQUENCE_runningThread (void *functionData)
{
	
	
	while(1) {
		if (HARDWARE_ADWIN_isDataOutputFinished ()) {
			SEQUENCE_setStatus(SEQ_STATUS_FINISHED);
			runningSequence = NULL;
			/*if (runningSequence != NULL) { // cleanup if output is finished (should never happen)
				SEQUENCE_free(runningSequence);
				free(runningSequence);
				runningSequence = NULL; 
				tprintf("ERROR: cleaned up sequence, expected seq == NULL at this point\n");
			}  */
		}
		if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {  // here everything starts if nothing was running before
			if (threadStopRequest) { // this is a save place to stop the thread (no sequence running)  
				threadFunctionId = 0;
				return 0;	
			}
			MAIN_handleSequenceFinished();
		}
		//DebugPrintf("waiting...\n");
		
		
		Sleep(100); // ms
	}	
	
}


void CVICALLBACK ErrorQueueReadCallback (int queueHandle, unsigned int event, int value, void *callbackData)
{
    char data[500];
    CmtReadTSQData (queueHandle, data, 500, TSQ_INFINITE_TIMEOUT, 0);
}


void SEQUENCE_startThread()
{
	int error = 0;
	if (threadFunctionId != 0) {
		return;
	}
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, SEQUENCE_runningThread, NULL, &threadFunctionId);
	
	//tprintf("thread started with id %d\n",threadFunctionId);
	
	error = CmtNewLock(NULL, 0, &lockSequenceQueue);//OPT_TL_PROCESS_EVENTS_WHILE_WAITING
	if (error) PostMessagePopup ("Error", "Error creating new lock.");  
	error = CmtNewLock(NULL, 0, &lockStartButton);  //OPT_TL_PROCESS_EVENTS_WHILE_WAITING
	if (error) PostMessagePopup ("Error", "Error creating new lock.");
	error = CmtNewLock(NULL, 0, &lockSetStatus);//OPT_TL_PROCESS_EVENTS_WHILE_WAITING
	if (error) PostMessagePopup ("Error", "Error creating new lock.");
	
	// init thread safe queue for error messages
	

    //if (InitCVIRTE (0, argv, 0) == 0)
    //    return -1; /* out of memory */
    //if ((panelHandle = LoadPanel(0, "DAQDisplay.uir", PANEL)) < 0)
    //    return -1;
    /* create queue that holds 1000 doubles and grows if needed */
    //CmtNewTSQ(1000, sizeof(double), OPT_TSQ_DYNAMIC_SIZE, &errorQueue);
    //CmtInstallTSQCallback (errorueue, EVENT_TSQ_ITEMS_IN_QUEUE, 500, ErrorQueueReadCallback, 0, CmtGetCurrentThreadID(), NULL);
    //CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, DataAcqThreadFunction, NULL, NULL);
    //DisplayPanel (panelHandle);
    //RunUserInterface();


}



// ---------------------------------------------
//   append sequence list of 
// ---------------------------------------------
void SEQUENCE_startOrEnqueue(t_sequence *s)
{
	if (s == NULL) {
		return;
	}
	
	// enqueue
	SEQUENCE_QUEUE_Insert(s, END_OF_LIST); 
	SEQUENCE_displayList();
	
	ProcessDrawEvents();
	
}



// ---------------------------------------------
//   append sequence to list and activates it
// ---------------------------------------------
void SEQUENCE_appendToList (t_sequence *s, int displayList)
{
    ListInsertItem (config->listOfSequences, &s, END_OF_LIST);
    if (displayList) SEQUENCE_displayList ();
//	InsertListItem (panelMain, MAIN_LISTBOX_Sequences, -1, 
//	    extractFileName (s->filename), ListNumItems (listOfSequences));
//    SEQUENCE_setAsActive (nSequences());
}




//=======================================================================
//
//    set contents of sequence to system defaults
//
//=======================================================================
void setSystemDefaults (t_sequence *seq)
{    
    seq->maxDigitalChannel   = defaultNr_DIO_Channels;
    seq->DIO_timebase_50ns   = 20;
    seq->AO_timebase_50ns   = 100;
}


//=======================================================================
//
//    create an empty sequence  
//
//=======================================================================
t_sequence *SEQUENCE_createEmptySequence (void)
{
    t_digitalBlock *p;
    t_sequence *new;
    
   	// ---------------------------------------------
    //   create a new sequence
	// ---------------------------------------------
    new = SEQUENCE_new ();
	if (new == NULL) {
		return NULL;
	}
    SEQUENCE_init (new);
	// ---------------------------------------------
    //   set filename 
	// ---------------------------------------------
    strcpy (new->filename, SEQUENCE_defaultFilename);  
    setSystemDefaults (new);

	// ---------------------------------------------
    // create the first block
	// ---------------------------------------------
    p = DIGITALBLOCK_new (new);
    DIGITALBLOCK_init (p);
	strcpy (p->blockName, "Init");
    p = DIGITALBLOCK_new (new);
    DIGITALBLOCK_init (p);
	SEQUENCE_addVoltageZeroToFirstBlock (new);
    
    return new;
}





   
//=======================================================================
//
//    save sequence 
//
//=======================================================================
int MAIN_saveSequence (t_sequence *seq) 
{
    //char help[MAX_PATHNAME_LEN];
    int err;
    
    if (seq == NULL) return 0;
	
	
	if (strcmp (seq->filename, SEQUENCE_defaultFilename) == 0) {
        MENU_SaveAs (-1, -1, NULL, -1);
        return 0;
    }
    
    tprintf ("Saving %s.\n", seq->filename);
    if ((err = SEQUENCE_save (seq->filename, seq)) == 0) {
        seq->changes = 1;
        setChanges (seq, 0);
    }
    return err;
}



int MAIN_saveSequenceAs (t_sequence *seq) 
{
    char filename[MAX_PATHNAME_LEN];
	int error = 0;
	
	if (seq == NULL) return 0;

	if (FileSelectPopup (config->defaultPath, seq->filename, SEQUENCE_fileSuffix,
                         "Save sequence as", VAL_SAVE_BUTTON,
                         0, 0, 1, 1, filename) > 0) {
    	tprintf ("Saving %s.\n", filename);
        setDefaultDir (extractDir (filename));
    	if (SEQUENCE_nrFromFilename(filename) == 0) {
	    	error = SEQUENCE_save (filename, seq);
	    	strcpy (seq->filename, filename);  
	    	if (seq == activeSeq()) MAIN_setFilenameToTitle (filename, seq->changes);
	    	SEQUENCE_displayList ();
	    }
	    else {
    	    MessagePopup ("Error", "Cannot overwrite file.\n\nThe selected file is already opened by ExperimentControl.");
	    }
	    
    }
	return error;
}	


//=======================================================================
//
//   load sequence 
//
//=======================================================================
int MAIN_loadSequence (const char *filename)
{
//    char myFilename[MAX_PATHNAME_LEN];
    t_sequence *newSeq;
    int err;

    // check if changes to save
    newSeq = SEQUENCE_new ();
	if (newSeq == NULL) {
		return -12; // out of memory
	}
    if ((err = SEQUENCE_load (filename, newSeq)) == 0) {
        // append new sequence
        SEQUENCE_appendToList (newSeq, 1);
	    SEQUENCE_setAsActive (nSequences());
		CONFIG_write (config);
        // set new Data
//		MAIN_resetRepetitions ();
    }
    else {  // Fehler beim Laden
        // MessagePopup ("Error", "Load error");
        free (newSeq);
    }
    return err;
}



int MAIN_askForSaveChanges (t_sequence *seq)
{
	if (seq == NULL) return 0;
	if (seq->changes) {
	    switch (messagePopupSaveChanges (seq->filename)) {
			// ---------------------------------------------
			//   save before close
			// ---------------------------------------------
	        case VAL_GENERIC_POPUP_BTN1:
	            if (MAIN_saveSequence (seq) != 0) return -1;
	            break;
			// ---------------------------------------------
			//   cancel 
			// ---------------------------------------------
	        case VAL_GENERIC_POPUP_BTN3:
	            return -1;
		}            
	}
	return 0;
	
}

/* not needed any more - all sequences in queue were now copied so there is no need to delete them on close of some other seq.
int MAIN_removeSequenceFromQueue(t_sequence* s)
{
	int i;
    t_sequence *tmp;
	int numDeleted = 0;
	
	CmtGetLock(lockSequenceQueue);
	// remove closed sequence from queue.
	for (i=1; i <= ListNumItems(listOfWaitingSequences); i++) {   // this scales quadratically - but i do not care.
		ListGetItem(listOfWaitingSequences,&tmp,i);
		if (tmp == s) {
			ListRemoveItem(listOfWaitingSequences,0,i);
			i = 1;
			numDeleted++;
		}
	}
	CmtReleaseLock(lockSequenceQueue);
	return numDeleted;
}*/

//=======================================================================
//
//    close sequence
//
//=======================================================================
int MAIN_closeSequence (int seqNr, int askSaveChanges)
{
    t_sequence *s;	
    int isActiveSequence;
	int oldState;
	
	oldState = SetBreakOnLibraryErrors (0);
	RemovePopup (1);
	SetBreakOnLibraryErrors (oldState );
	if (seqNr == 0) return 0;
	s = SEQUENCE_ptr (seqNr);
	if (s == NULL) return 0;
	// ---------------------------------------------
	//   test if changes were made
	// ---------------------------------------------
    if (askSaveChanges) {
		if (MAIN_askForSaveChanges (s) != 0) return 0;
	}
	// ---------------------------------------------
	//   close active sequence 
	// ---------------------------------------------
    isActiveSequence = (s == activeSeq());
	if (config->idleSequenceNr == seqNr) config->idleSequenceNr = -1;
	if (seqNr < config->idleSequenceNr) config->idleSequenceNr--;

	ListRemoveItem (config->listOfSequences, 0, seqNr);
	//MAIN_removeSequenceFromQueue(s);
    SEQUENCE_free (s); 
    free (s);
    s = NULL;
	
    
	if (nSequences() == 0) {
		// ---------------------------------------------
		//   no sequences in memory 
		// ---------------------------------------------
	    SEQUENCE_appendToList (SEQUENCE_createEmptySequence (), 1);
		SEQUENCE_setAsActive (nSequences());

	}
	else {
		// ---------------------------------------------
		//   determine new active sequence 
		// ---------------------------------------------
		if (isActiveSequence) {
			SEQUENCE_setAsActive (seqNr-1);
		}
		else {
			// inactive sequence was closed
			if (seqNr < config->activeSequenceNr) config->activeSequenceNr--;
			if (seqNr < config->idleSequenceNr) config->idleSequenceNr--;
		 	if (config->activeSequenceNr <= 0) config->activeSequenceNr = 1;
			SEQUENCE_displayList ();   
		}
	}

    return 0;
}


int MAIN_closeAllSequences (int keepActiveSequence)
{
    int i;
    
	CONFIG_write (config);

	for (i = 1; i <= nSequences(); i++) {
		if (MAIN_askForSaveChanges (SEQUENCE_ptr (i)) != 0) return 1;
	}
	for (i = nSequences(); i > 0; i--) {
        if ((i != config->activeSequenceNr) && (i !=  config->idleSequenceNr)) {
	        if (MAIN_closeSequence (i, 0) != 0) return 1;
	        SEQUENCE_displayList ();   
	    }
	}

	if (!keepActiveSequence) {
		if (MAIN_closeSequence (config->activeSequenceNr, 0) != 0) return 1;
	}
	return 0;	
}



//=======================================================================
//
//    quit program 
//
//=======================================================================
int MAIN_quit ()
{ 
	if (MAIN_closeAllSequences (9) != 0) return 1;
	HidePanel (panelMain2);
	return 0;
}





//=======================================================================
//
//    assign shortcut key to sequence 
//
//=======================================================================
void SEQUENCE_setShortcut (t_sequence *seq, int key)
{
	t_sequence *s;
	int i;
	
	if (seq == NULL) return;
	for (i = 1; i <= nSequences(); i++) {
	   s = SEQUENCE_ptr (i);
	   if (s->shortcut == key) s->shortcut = -1;
	}
	seq->shortcut = key;
	SEQUENCE_displayList ();
}

//=======================================================================
//
//    return sequence with shortcut 'key'
//
//=======================================================================
t_sequence *SEQUENCE_fromShortcut (int key)
{
	t_sequence *s;
	int i;
	
	if (key == VAL_F9_VKEY) return activeSeq();
	
	for (i = 1; i <= nSequences(); i++) {
	   s = SEQUENCE_ptr (i);
	   if (s->shortcut == key) return s;
	}
	return NULL;
}







//=======================================================================
//
//    Sequence List clicked 
//
//=======================================================================
int CVICALLBACK Sequence_List_Clicked (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int seqNr;
    char txt[100];
    int numVisibleLines, firstVisibleLine, lineHeight;
    int lineClicked;
    int choice;
    int previous;
	t_sequence *seq;
	t_sequence *idleSeq;
	//t_sequence *tmp;
	int panelRename;
	char filename[MAX_PATHNAME_LEN];
	int keyModifiers;
	//int i;
    
    GetCtrlVal (panel, control, &seqNr);
    switch (event)
        {
		case EVENT_LEFT_CLICK:
			GetGlobalMouseState(NULL,NULL,NULL,NULL,NULL,&keyModifiers); //GetRelativeMouseState()  
			GetCtrlAttribute (panel, control, ATTR_VISIBLE_LINES,
				  &numVisibleLines);
			GetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE,
							  &firstVisibleLine);
            lineHeight = (ctrlHeight(panel,control) / numVisibleLines);
            lineClicked = 1 + firstVisibleLine + (eventData1 - ctrlTop (panel, control)) / lineHeight;
			if (lineClicked  > nSequences()) {
				return 0;
			}
			if (keyModifiers & VAL_MENUKEY_MODIFIER) { // strg key pressed during click
				seqNr = lineClicked;
				seq = SEQUENCE_ptr(seqNr);
				SEQUENCE_QUEUE_Insert(seq,END_OF_LIST);
				SEQUENCE_displayList ();
			}
			break;
        case EVENT_LEFT_DOUBLE_CLICK:
			if (seqNr > nSequences()) return 0;
			GetGlobalMouseState(NULL,NULL,NULL,NULL,NULL,&keyModifiers);
			if (keyModifiers != 0) return 0; // do not accept double click if modifier key is pressed.
     		previous = getActiveSeqNr ();
     		SEQUENCE_setAsActive (seqNr);
     		//if (seqNr != previous) {
	     	//	GPIB_clearOutputCtrl (); 
	     	//	stdCls ();
	     	//}
			//SetActivePanel (panelMain);
     		break;
        case EVENT_RIGHT_CLICK:
			if (seqNr > nSequences()) return 0;
			// ---------------------------------------------
			//   determine line of list box
			// ---------------------------------------------
			GetCtrlAttribute (panel, control, ATTR_VISIBLE_LINES,
							  &numVisibleLines);
			GetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE,
							  &firstVisibleLine);
            lineHeight = (ctrlHeight(panel,control) / numVisibleLines);
            lineClicked = 1 + firstVisibleLine + (eventData1 - ctrlTop (panel, control)) / lineHeight;
            if (lineClicked  > nSequences()) return 0; 
            SetCtrlVal (panel, control, lineClicked);
            seqNr = lineClicked;
			seq = SEQUENCE_ptr(seqNr);
            
			// ---------------------------------------------
			//   display filename in menu
			// ---------------------------------------------
            sprintf (txt, "Close  %s", extractFilename(seq->filename));
			SetMenuBarAttribute (menuSequenceList, MENU5_SEQLIST_CLOSE_SEQ,
								 ATTR_ITEM_NAME, txt);
			SetMenuBarAttribute (menuSequenceList,
								 MENU5_SEQLIST_READONLY, ATTR_CHECKED,
								 seq->readOnly);

			// ---------------------------------------------
			//   open popup menu
			// ---------------------------------------------
            choice = RunPopupMenu (menuSequenceList, MENU5_SEQLIST, panel, 
                eventData1, eventData2, 0,0,0,0);
			switch (choice) {
				case MENU5_SEQLIST_CLOSE_SEQ:
					MAIN_closeSequence (seqNr, 1);
					CONFIG_write (config);
					break;
				case MENU5_SEQLIST_CLOSE_ALL:
					MAIN_closeAllSequences (1);
					CONFIG_write (config);
					break;
				case MENU5_SEQLIST_RENAME:
					panelRename = LoadPanel (0, UIR_File, RENAME);
					changeSuffix (filename, extractFilename (seq->filename), "");
					SetCtrlVal (panelRename, RENAME_STRING, filename);
					SetPanelAttribute (panelRename, ATTR_CALLBACK_DATA, seq);
					InstallPopup (panelRename);
					CONFIG_write (config);
					break;
				case MENU5_SEQLIST_SAVE:
					MAIN_saveSequence (SEQUENCE_ptr (seqNr));
					CONFIG_write (config);
					break;
				case MENU5_SEQLIST_SAVEAS:
					MAIN_saveSequenceAs (SEQUENCE_ptr (seqNr));
					CONFIG_write (config);
					break;
				case MENU5_SEQLIST_SHOW:
					SEQUENCE_setAsActive (seqNr);
					break;
				case  MENU5_SEQLIST_IDLE:
					SEQUENCE_setAsIdle (seqNr);
					CONFIG_write (config);
					break;
				//case  MENU5_SEQLIST_CALIBRATION:
					//SEQUENCE_setAsCalibration (seqNr);
				//	break;
				case MENU5_SEQLIST_ADD_QUEUE:
					SEQUENCE_QUEUE_Insert(seq,END_OF_LIST);
					SEQUENCE_displayList ();
					break;
				case MENU5_SEQLIST_ADD_QUEUE_MULTI:
					idleSeq = SEQUENCE_ptr (config->idleSequenceNr); 
					/*for (i=0; i < 10; i++) {
						SEQUENCE_QUEUE_Insert(seq,END_OF_LIST);
						SEQUENCE_QUEUE_Insert(idleSeq,END_OF_LIST);  
					}*/
					SEQUENCE_QUEUE_InsertMulti(&seq, 1, 10, 1);
					SEQUENCE_displayList ();
					break;
				case MENU5_SEQLIST_READONLY:
					if (SetFileAttrs (seq->filename, !seq->readOnly, -1, -1, -1) == 0) {
						seq->readOnly = !seq->readOnly;
						SEQUENCE_displayList ();
					}
					break;
				case MENU5_SEQLIST_SHORTCUT_F5:
					SEQUENCE_setShortcut (SEQUENCE_ptr(seqNr), VAL_F5_VKEY);
					break;
				case MENU5_SEQLIST_SHORTCUT_F6:
					SEQUENCE_setShortcut (SEQUENCE_ptr(seqNr), VAL_F6_VKEY);
					break;
				case MENU5_SEQLIST_SHORTCUT_F7:
					SEQUENCE_setShortcut (SEQUENCE_ptr(seqNr), VAL_F7_VKEY);
					break;
				case MENU5_SEQLIST_SHORTCUT_F8:
					SEQUENCE_setShortcut (SEQUENCE_ptr(seqNr), VAL_F8_VKEY);
					break;
			}
        }
    return 0;
}



// returns -1 on error
int seqNrFromSeqPointer(t_sequence* seq)
{
	int i;
	t_sequence *tmp;
	for (i = 1; i <= nSequences(); i++)
    {
		tmp = SEQUENCE_ptr(i);
		if (seq == tmp) {
			return i;
		}
    }
	return -1;
}


//=======================================================================
//
//    Sequence Queue clicked 
//
//=======================================================================
int CVICALLBACK Sequence_Queue_Clicked (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
	//t_sequence *idleSeq;
    //int seqNr;
	int entryNr;
	//int i;
    //char txt[100];
    int numVisibleLines, firstVisibleLine, lineHeight;
    int lineClicked;
    int choice;
    //int previous;
	t_sequence *seq;
	//int panelRename;
	//char filename[MAX_PATHNAME_LEN];
	//int insertPos;
	//t_sequence *tmpSeq;
	
    GetCtrlVal (panel, control, &entryNr);
    
	switch (event)
        {
        /*case EVENT_LEFT_DOUBLE_CLICK:
			SEQUENCE_QUEUE_Get(&seq,entryNr);  
			seqNr = seqNrFromSeqPointer(seq);
			if (seqNr == -1) {
				tprintf("Error: sequence not open any more; TODO not implemented.\n");
				return 0;
			}
			
			if (seqNr > nSequences()) return 0;
     		previous = getActiveSeqNr ();
     		SEQUENCE_setAsActive (seqNr);
     		//if (seqNr != previous) {
	     	//	GPIB_clearOutputCtrl (); 
	     	//	stdCls ();
	     	//}
			//SetActivePanel (panelMain);
     		break;			 */
        case EVENT_RIGHT_CLICK:
			// ---------------------------------------------
			//   determine line of list box
			// ---------------------------------------------
			GetCtrlAttribute (panel, control, ATTR_VISIBLE_LINES,
							  &numVisibleLines);
			GetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE,
							  &firstVisibleLine);
            lineHeight = (ctrlHeight(panel,control) / numVisibleLines);
            lineClicked = 1 + firstVisibleLine + (eventData1 - ctrlTop (panel, control)) / lineHeight;
            if (lineClicked  > (int)ListNumItems(listOfWaitingSequences)) return 0; 
            SetCtrlVal (panel, control, lineClicked);
            entryNr = lineClicked;
			if ((entryNr < 1) || (entryNr > (int)ListNumItems (listOfWaitingSequences))) {
				tprintf("ERROR: sequence clicked does not exist\n");
				return 0; 
			}
			SEQUENCE_QUEUE_Get(&seq,entryNr);
			
			//if (seqNr == -1) {
			//	tprintf("Error: sequence not open any more; TODO not implemented.\n");
			//	return 0;
			//}
			
			//seq = SEQUENCE_ptr(seqNr);
            
		/*	// ---------------------------------------------
			//   display filename in menu
			// ---------------------------------------------
            sprintf (txt, "Close  %s", extractFilename(seq->filename));
			SetMenuBarAttribute (menuSequenceList, MENU5_SEQLIST_CLOSE_SEQ,
								 ATTR_ITEM_NAME, txt);
			SetMenuBarAttribute (menuSequenceList,
								 MENU5_SEQLIST_READONLY, ATTR_CHECKED,
								 seq->readOnly);
		*/
			// ---------------------------------------------
			//   open popup menu
			// ---------------------------------------------
            choice = RunPopupMenu (menuSequenceQueue, MENU10_SEQ_QUEUE, panel, 
                eventData1, eventData2, 0,0,0,0);
			switch (choice) {
				case MENU10_SEQ_QUEUE_REMOVE:
					SEQUENCE_QUEUE_Delete(entryNr);
					SEQUENCE_displayList();   
					break;
				case MENU10_SEQ_QUEUE_REMOVE_ALL:
					SEQUENCE_QUEUE_DeleteAll();	
					SEQUENCE_displayList();   
					break;
				case MENU10_SEQ_QUEUE_INSERT_IDLE_BEFORE:
					SEQUENCE_QUEUE_InsertIdle(entryNr);  
					SEQUENCE_displayList(); 
					break;
				case MENU10_SEQ_QUEUE_INSERT_IDLE_BEHIND:
					SEQUENCE_QUEUE_InsertIdle(entryNr+1);
					SEQUENCE_displayList(); 
					break;
				case MENU10_SEQ_QUEUE_EDIT_COPY:
					if (seq == NULL) {
						break;	
					}
					// append new sequence
			        SEQUENCE_appendToList (SEQUENCE_clone(seq), 0);
				    SEQUENCE_setAsActive (nSequences());
					CONFIG_write (config);
					// %ABC
					SEQUENCE_displayList(); 
					break;
				
			}
        }
    return 0;
	
}








/************************************************************************/
/************************************************************************/
/*
/*    Main Menu functions 
/*
/************************************************************************/
/************************************************************************/

//=======================================================================
//
//    Menu: New sequence 
//
//=======================================================================
void CVICALLBACK MENU_New (int menuBar, int menuItem, void *callbackData,
        int panel)
{
    SEQUENCE_appendToList (SEQUENCE_createEmptySequence (), 1);
	SEQUENCE_setAsActive (nSequences());
}

 
//=======================================================================
//
//    Menu: Save 
//
//=======================================================================
void CVICALLBACK MENU_Save (int menuBar, int menuItem, void *callbackData,
        int panel)
{
    CONFIG_write (config);
	MAIN_saveSequence (activeSeq());
}

 
//=======================================================================
//
//    Menu: Save As
//
//=======================================================================
void CVICALLBACK MENU_SaveAs (int menuBar, int menuItem, void *callbackData,
        int panel)
{
	MAIN_saveSequenceAs (activeSeq());
    CONFIG_write (config);
}


void CVICALLBACK MENU_SaveCopyAs (int menuBar, int menuItem, void *callbackData,
        int panel)
{
    char filename[MAX_PATHNAME_LEN];
    
    if (FileSelectPopup (config->defaultPath, activeSeq()->filename, SEQUENCE_fileSuffix,
                         "Save sequence as", VAL_SAVE_BUTTON,
                         0, 0, 1, 1, filename) > 0) {
    	tprintf ("Saving %s.\n", filename);
        setDefaultDir (extractDir (filename));
    	if (SEQUENCE_nrFromFilename(filename) == 0) {
	    	if (SEQUENCE_save (filename, activeSeq()) == 0) MAIN_loadSequence (filename);  
	    }
	    else {
    	    MessagePopup ("Error", "Cannot overwrite file.\n\nThe selected file is already opened by ExperimentControl.");
	    }
	    
    }
}


//=======================================================================
//
//    Menu: Open
//
//=======================================================================
void CVICALLBACK MENU_open (int menuBar, int menuItem, void *callbackData,
        int panel)
{
    char filename[MAX_PATHNAME_LEN];
    
    if (FileSelectPopup (config->defaultPath, SEQUENCE_fileSuffix, SEQUENCE_fileSuffix,
						 "Load sequence file", VAL_LOAD_BUTTON, 0, 1, 1, 0,
						 filename) > 0) {
        setDefaultDir (extractDir (filename));
        MAIN_loadSequence (filename);   
		CONFIG_write (config); 
    }
}

void CVICALLBACK MENU_Close (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	MAIN_closeSequence (config->activeSequenceNr, 1);    
}



void CVICALLBACK MENU_ImportNames (int menuBar, int menuItem, void *callbackData,
		int panel)
{
    IMPORT_showPanel ();
}


void CVICALLBACK Menu_ExportOldFormat (int menuBar, int menuItem, void *callbackData,
		int panel)
{
    char filename[MAX_PATHNAME_LEN];
    
    if (FileSelectPopup (config->defaultPath, activeSeq()->filename, SEQUENCE_fileSuffix,
                         "Save control sequence in old format as", VAL_SAVE_BUTTON,
                         0, 0, 1, 1, filename) > 0) {
    	tprintf ("Saving %s.\n", filename);
//	    saveSequenceFile_OldPrgVersion (filename, activeSeq());
    	if (SEQUENCE_nrFromFilename(filename) != 0) {
    	    MessagePopup ("Error", "Cannot overwrite file.\n\nThe selected file is already opened by ExperimentControl.");
	    }
	    
    }
    
	
}






void CVICALLBACK MENU_Simulate (int menuBar, int menuItem, void *callbackData,
		int panel)
{
    SIMULATE_start (activeSeq(), 0);
}

void CVICALLBACK MENU_InitBoards (int menuBar, int menuItem, void *callbackData,
		int panel)
{
    HARDWARE_initializeBoards ();
}


void CVICALLBACK MENU_detectLoops (int menuBar, int menuItem, void *callbackData,
		int panel)
{
    t_sequence *seq;
    
    seq = activeSeq();
    SEQUENCE_detectLoops (seq, ListNumItems (seq->lDigitalBlocks), 3, 0, 1);
	DIGITALBLOCKS_displayAllBlocks (seq);
    
    
}



void CVICALLBACK MENU_Test (int menuBar, int menuItem, void *callbackData,
		int panel)
{
//	performanceTest (activeSeq());	
}










/************************************************************************/
/************************************************************************/
/*
/*    Command buttons
/*
/************************************************************************/
/************************************************************************/



char* SEQUENCE_statusText(int status) {
	switch (status) { 	
		case SEQ_STATUS_STARTED: return "started";
		case SEQ_STATUS_FINISHED: return "finished";
		case SEQ_STATUS_INTERRUPTED: return "interrupted";
		case SEQ_STATUS_IDLE_STARTED: return "idle started";
		default: return "unknown";
	}
}

//=======================================================================
//
//    set status of sequence (STARTED, FINISHED, INTERRUPTED)
//
//=======================================================================
int SEQUENCE_setStatus (int newStatus)
{
	static volatile int status = SEQ_STATUS_FINISHED;
	char oldTxt[200];
	char newTxt[200];
	
	//CmtGetLock(lockSetStatus);
	
	switch (newStatus) {
	    case SEQ_STATUS_GET_STATUS:
			//CmtReleaseLock(lockSetStatus);    
	        return status;
//	        break;
		case SEQ_STATUS_FINISHED:
//	    	MAIN_setStartButton (1);        
	    	status = newStatus;
			//MAIN_setMenuBarStatus (0);
   			PROTOCOLFILE_printf ("      SEQUENCE_setStatus (SEQ_STATUS_FINISHED)");
	    	break;
		case SEQ_STATUS_STARTED:
//	    	MAIN_setStartButton (0);
			//MAIN_setMenuBarStatus (1);
	    	status = newStatus;
   			PROTOCOLFILE_printf ("      SEQUENCE_setStatus (SEQ_STATUS_STARTED)");
	    	break;
	    case SEQ_STATUS_INTERRUPTED:
	    	status = newStatus;
   			PROTOCOLFILE_printf ("      SEQUENCE_setStatus (SEQ_STATUS_INTERRUPTED)");
	    	break;
		case SEQ_STATUS_IDLE_STARTED:
			status = newStatus;
			//CmtReleaseLock(lockSetStatus);  
			SetCtrlVal(panelMain,MAIN_TEXTMSG_SequenceState,SEQUENCE_statusText(newStatus));
			return 0;
	}
	/*
	SetCtrlAttribute (panelMain, MAIN_BTN_startSequence,
	                 ATTR_VISIBLE, newStatus == SEQ_STATUS_FINISHED);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence,
	                 ATTR_VISIBLE, newStatus == SEQ_STATUS_STARTED);
	
	if (panelSequence > 0) {
		SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence,
	                 ATTR_VISIBLE, newStatus == SEQ_STATUS_FINISHED);
		SetCtrlAttribute (panelSequence, SEQUENCE_BTN_stopSequence,
	                 ATTR_VISIBLE, newStatus == SEQ_STATUS_STARTED);
	}   */
	
	
	//CmtReleaseLock(lockSetStatus);
	GetCtrlVal(panelMain,MAIN_TEXTMSG_SequenceState,oldTxt);
	strcpy(newTxt,SEQUENCE_statusText(newStatus));
	if (strcmp(oldTxt,newTxt)!=0) {
		SetCtrlVal(panelMain,MAIN_TEXTMSG_SequenceState,newTxt); 
	}
	return 0;
}





//=======================================================================
//
//    get status of sequence (STARTED, FINISHED, INTERRUPTED)
//
//=======================================================================
int SEQUENCE_getStatus (void)
{
    return SEQUENCE_setStatus(SEQ_STATUS_GET_STATUS);
}



void MAIN_startIdleSequence_changeCtrls (int isRunning, int disableAll)
{
	SetCtrlAttribute (panelMain, MAIN_BTN_startIdle, ATTR_VISIBLE, !isRunning );	
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle, ATTR_VISIBLE, isRunning);	
	SetCtrlAttribute (panelMain, MAIN_BTN_startIdle, ATTR_DIMMED, disableAll);	
	SetCtrlAttribute (panelMain, MAIN_BTN_stopIdle, ATTR_DIMMED, disableAll);	
	SetCtrlAttribute (panelMain, MAIN_COMMANDBUTTON_boot, ATTR_DIMMED, isRunning);	

}




void MAIN_startSequence_changeCtrls (int isRunning)
{
	int frameColor_running = VAL_RED;
	int frameColor_notrunning = VAL_DK_GRAY;
	int i, ctrl;
	
	
	CmtGetLock(lockStartButton);
	/*SetTabPageAttribute (panelMain2, MAIN2_TAB0, TAB0_INDEX_WFM,
						 ATTR_DIMMED, isRunning);
	SetTabPageAttribute (panelMain2, MAIN2_TAB0, TAB0_INDEX_GPIB,
						 ATTR_DIMMED,isRunning);
	SetTabPageAttribute (panelMain2, MAIN2_TAB0, TAB0_INDEX_CCD,
						 ATTR_DIMMED,isRunning);*/
	for (i = 0; i < 4; i++) { // frame color in tables shows that sequence is running
		switch (i) {
			case 0: ctrl = SEQUENCE_TABLE_dac1; break;
			case 1: ctrl = SEQUENCE_TABLE_dac2; break;
			case 2: ctrl = SEQUENCE_TABLE_digital1; break;
			case 3: ctrl = SEQUENCE_TABLE_digital2; break;
			default:ctrl = SEQUENCE_TABLE_dac1; 
		}
		SetTableCellRangeAttribute (panelSequence, ctrl, 
							   	VAL_TABLE_ENTIRE_RANGE, 
								ATTR_VERTICAL_GRID_COLOR, 
						  		isRunning ? frameColor_running : frameColor_notrunning);
		SetTableCellRangeAttribute (panelSequence, ctrl, 
							   	VAL_TABLE_ENTIRE_RANGE, 
								ATTR_HORIZONTAL_GRID_COLOR, 
						  		isRunning ? frameColor_running : frameColor_notrunning);
	}
	
	
	/*
	// tables as indicator
	SetAttributeForCtrls (panelSequence, ATTR_CTRL_MODE, 
						  isRunning ? VAL_INDICATOR : VAL_HOT, 0,
						  SEQUENCE_TABLE_header1,
						  SEQUENCE_TABLE_header2,
						  SEQUENCE_TABLE_dac1,
						  SEQUENCE_TABLE_dac2,
						  SEQUENCE_TABLE_digital1,
						  SEQUENCE_TABLE_digital2,
						  0);
	
	// other stuff as indicator
	SetAttributeForCtrls (panelMain, ATTR_CTRL_MODE, 
						  isRunning ? VAL_INDICATOR : VAL_HOT, 0,
						  MAIN_NUM_repetitions, 
						  MAIN_NUM_copies, 
						  MAIN_LISTBOX_Sequences,
						  MAIN_RING_TriggerSource, 
						  0);
	*/
	
	
	// update start/stop button
	SetCtrlAttribute (panelMain, MAIN_BTN_startSequence,
	                 ATTR_VISIBLE, !isRunning);
	SetCtrlAttribute (panelMain, MAIN_BTN_stopSequence,
	                 ATTR_VISIBLE, isRunning);
	
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence,
                 ATTR_VISIBLE, !isRunning);
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_stopSequence,
                 ATTR_VISIBLE, isRunning);
	
	
	 CmtReleaseLock(lockStartButton);
}


int MAIN_startSequence (t_sequence *s, int freeSequence)
{
    //int lastActivePanel;
    
    if (s == NULL) return 0;
    if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) return -1;
    // change status 
    // this to prevent another sequence to start
    // before all repetitions are finished
    SEQUENCE_displayList();
    SEQUENCE_setStatus (SEQ_STATUS_STARTED);
	//GetActiveTabPage (panelMain2, MAIN2_TAB0, &lastActivePanel);	  // ???
	
	SEQUENCE_displayStartNr(s->startNr); // TODO deref of null pinter at close.
	
	OUTPUT_showPanel (s);

	MAIN_startSequence_changeCtrls(1);
	
	SEQUENCE_writeCommandToFile ("START");
	ProcessDrawEvents();
	
	if (SEQUENCE_autoSaveSequence (s, 0) == 0) {
		CCD_writeAllCommandFiles (s, config->autoSaveSequencesAfterEachStart);

		runningSequence = s; // sequence already cloned before (on add into sequence queue)

		HARDWARE_SetGetInterruptedFlag (0);
		startSequence (s); // <--- really start sequence
		
		runningSequence = NULL; 
		
	} else {
		tprintf("Error auto save failed.\n");
	}

	SEQUENCE_writeCommandToFile ("STOP");
	
	MAIN_startSequence_changeCtrls (0);
	
	
								  
//	SetTabPageAttribute (panelMain2, MAIN2_TAB0, TAB0_INDEX_SEQUENCE,
//						 ATTR_DIMMED, 0);
//	SetActiveTabPage (panelMain2, MAIN2_TAB0, lastActivePanel);

    SEQUENCE_setStatus (SEQ_STATUS_FINISHED);
	DIGITALBLOCKS_displayGlobalAnalogVoltages ();
	ProcessDrawEvents();
	
	if (freeSequence) {
		SEQUENCE_free(s);
		free(s);
	}
	
//	MAIN_handleSequenceFinished();
	
//	DisplayPanel (panelMain2);
    return 0;
}




void MAIN_stopIdleSequence (void)
{
	//if (SEQUENCE_getStatus () != SEQ_STATUS_FINISHED) {  // FIXME
	//	HARDWARE_stopDataOutput ();
	//	SEQUENCE_setStatus(SEQ_STATUS_FINISHED); 
	//	runningSequence = NULL;
	//	tprintf ("Idle sequence stopped.\n");
	//}
	
	
	  
	if (!idleSequenceEnabled) {
		MAIN_startIdleSequence_changeCtrls (0, 0); // just to be sure   
		return;	
	}
	idleSequenceEnabled = 0; 
	if (SEQUENCE_isIdleSequence(runningSequence) && SEQUENCE_getStatus () == SEQ_STATUS_IDLE_STARTED) {
		HARDWARE_stopDataOutput ();
		SEQUENCE_setStatus(SEQ_STATUS_FINISHED); 
		runningSequence = NULL;
		tprintf ("Idle sequence stopped.\n");
	}
	MAIN_startIdleSequence_changeCtrls (0, 0); 
}


// start idle sequence as "normal sequence"
// just use isOfTypeIdleSeq-flag to distinguish?
// differences idle <.-> normal:
// -feedback
// -saving images, i.e. ccd ini file
// -pifoc voltage offset
//
void MAIN_startIdleSequence(void)
{
	t_sequence *idleSeq;
	//t_waveform *dmdWfm;
	
	if (!idleSequenceEnabled) return;
	
	/*if (config->hardwareType != HARDWARE_TYPE_ADWIN) {
		tprintf("Skip idle sequence: only implemented for Adwin.\n");
		return;
	}*/
	idleSeq = SEQUENCE_ptr (config->idleSequenceNr);
	
	if (idleSeq == NULL) {
		PostMessagePopupf ("Error", "No idle sequence selected.");
		MAIN_stopIdleSequence ();
		return;
	}
	
	// no focus feedback for idle sequence!
	idleSeq->enableFocusFeedback = 0;
	idleSeq->manualPifocFocusVoltage = 0;
	
	idleSeq->isOfTypeIdleSeq = 1; // not really necessary
	
	// TODO disable phase feedback - really needed ???   
	//dmdWfm = WFM_ADDR_findDMDwaveform (s);
	//if (dmdWfm != NULL) {
	//	dmdWfm->addrDMDimage;	
	//}
	
	
	
	runningSequence = idleSeq; // set idleSequence as currently running sequence.
	
	OUTPUT_showPanel (runningSequence);  
	ProcessDrawEvents();    
	
	CCD_writeAllCommandFiles (idleSeq, 0);
	startSequence(idleSeq);
	
	
	/*
	if (OUTPUTDATA_calculate (idleSeq, 1, 0) < 0) return;
	ADWINDATA_free (idleSequenceData);
	free (idleSequenceData);
	idleSequenceData = idleSeq->outData->adwinData;
	idleSeq->outData->adwinData = NULL;
	if (idleSequenceData == NULL) return;
	
	// disable external trigger - hardware 50Hz trigger
	idleSequenceData->trigger = VAL_NO_TRIGGER;
	//tprintf("idleSequenceData->trigger = %d\n", idleSequenceData->trigger);
	
//			free (idleSequenceData->triggerTimesDuringSeq); 
//			idleSequenceData->triggerTimesDuringSeq = NULL;
//			idleSequenceData->nTriggerTimesDuringSeq = 0;
	
//			SetCtrlVal (panelMain, MAIN_STRING_sequence, extractFilename (seq->filename));
//			tprintf ("Starting idle sequence...\n");
	
	
	// START IDLE SEQUENCE
	//MAIN_startIdleSequence_changeCtrls (1, 0);
	tprintf("Starting idle sequence ...\n");
	SetCtrlVal (panelMain, MAIN_STRING_sequence, "IDLE sequence");
	
	//NEW SK
	
	//idleSeq = (t_sequence *) idleSequenceData->callingSequence;
	// allow pictures being taken in idle sequence
	CCD_writeAllCommandFiles (idleSeq, config->autoSaveSequencesAfterEachStart);    
	
	// -----------------------------------------------------------
	//		run GPIB commands in idle sequence
    // 	    reset all last values from the frequency generators
	// -----------------------------------------------------------
	GPIB_resetAllLastValues (idleSeq, 0, 0);
	
	
	GPIB_setShowDataFlag (!config->suppressProtocol);
	if (HARDWARE_sendAllGpibCommands (idleSeq, 1) != 0) {
		HARDWARE_resetOutputs (idleSeq);
		tprintf("FUNC: MAIN_TIMER_idleSequence DONE: HARDWARE_stopCounters(GPIB_ERR)");
	}
	
	//if (HARDWARE_ADWIN_dataOutput (idleSequenceData, -1, 0, 0) != 0) return;
	SEQUENCE_setStatus(SEQ_STATUS_IDLE_STARTED);
	if (HARDWARE_ADWIN_dataOutput (idleSequenceData, -1, 1, 0) != 0) return;    // in threaded version we want to wait here	
	
	
	
	//runningSequence = NULL; // idle sequence starting finished. - the sequence is running now.
	//tprintf("Idle sequence finished.\n");
	//Sleep(1000); // TODO check if this is necessary
	*/
}



// has to be called everywhere where a sequence is stopped/finished
// this function decides which sequence should run after the last finished sequence.
// FIXME: function stacked builds up, should not be a problem for realistic numbers of sequential runs
void MAIN_handleSequenceFinished(void) // t_sequence*s
{		
	t_sequence* seq;
	DebugPrintf("Sequence finished: decide which sequence to start next.\n");
	// TODO
	//static int callNo;
	//t_sequence *idleSeq;
	
				   
	if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) {
		tprintf("Error: expected sequence to be finished but it is not.");				
		return;
	}

	if (ListNumItems(listOfWaitingSequences) == 0) {
		
		if (!idleSequenceEnabled) {
			return;
		}
		
		// no sequences are waiting - start idle sequence.
		tprintf("Start idle sequence.\n"); 
		MAIN_startIdleSequence();
		/*
		switch (config->hardwareType) {
			case HARDWARE_TYPE_ADWIN:
				MAIN_startIdleSequence();
				break;
			default: tprintf("Skip idle sequence: only implemented for Adwin.\n");
		}*/
	} else {
		tprintf("Start sequence from queue.\n"); 
		
		CmtGetLock(lockSequenceQueue);
		ListRemoveItem(listOfWaitingSequences,&seq,1);
		CmtReleaseLock(lockSequenceQueue);
		
		SEQUENCE_displayList();
		ProcessDrawEvents();
		if (seq == NULL) {   // if idle sequence in queue just start it as idle sequence!
			MAIN_startIdleSequence();  	
		} else {
			MAIN_startSequence(seq, 1);
		}
	}
	
	
}



// TODO need to send the right number of camera triggers to avoid getting cam out of sync !!!  
void MAIN_stopSequence (void)
{
	
	if (!config->suppressProtocol) tprintf ("STOPPING SEQUENCE...  ");
	HARDWARE_stopDataOutput ();
	
    SEQUENCE_setStatus (SEQ_STATUS_INTERRUPTED);
	ProcessDrawEvents();
	//MAIN_handleSequenceFinished();
}



int CVICALLBACK BTN_remoteStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	DebugPrintf (eventStr(event,eventData1,eventData2));
	t_sequence *seq;
	
	switch (event) {
		case EVENT_GOT_FOCUS:
//			SetActivePanel(panelMain);
			SetActiveCtrl (panelMain, MAIN_BTN_startSequence);
			SetCtrlAttribute (panelMain, MAIN_COMMANDBUTTON_remote,
						  ATTR_VISIBLE, 1);
			seq = SEQUENCE_fromShortcut(remoteKey);
			if (seq != NULL) {
				if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
	 				SEQUENCE_startOrEnqueue (seq);
				}
				TCP_serverSendStrf (TCP_CMD_STOPPED);
			}
			
			break;
	}
	return 0;
}


//=======================================================================
//
//    stop sequence
//
//=======================================================================

int CVICALLBACK BTN_stopSequence (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
            if (SEQUENCE_getStatus() == SEQ_STATUS_STARTED) {
				/********************************/
                // sequence is running, so stop it
				/********************************/
				MAIN_stopSequence ();
			}
			break;
	}
	return 0;
}









//=======================================================================
//
//    "Start sequence"
//
//=======================================================================
int CVICALLBACK BTN_StartSequence (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
	t_sequence *startSeq;
	
	CONFIG_write (config); // write config on seq start
    
    switch (event)
        {
        case EVENT_COMMIT:
			
			startSeq = activeSeq();
			startSeq->remote = 0;
			
			SEQUENCE_startOrEnqueue(startSeq);
            break;
        }
    return 0;
}





int CVICALLBACK SEQUENCE_parametersChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	int entryLength; 
	char tmpShortName[MAX_PATHNAME_LEN]; 
	
	
	
	
	switch (event)
		{
		case EVENT_COMMIT:
			seq = activeSeq();
			if (panel == panelSequence) {
				GetCtrlVal (panelSequence, SEQUENCE_NUMERIC_repOf, &seq->nRepetitions );
				SetCtrlVal (panelMain, MAIN_NUM_repetitions, seq->nRepetitions);
			}
			else {
				GetCtrlVal (panelMain, MAIN_NUM_repetitions, &seq->nRepetitions );
				SetCtrlVal (panelSequence, SEQUENCE_NUMERIC_repOf, seq->nRepetitions);
			}
			SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
			SetCtrlAttribute (subPanelWFMPoints, WFM_wPTS_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
			GetCtrlVal (panelMain, MAIN_RING_TriggerSource, &seq->trigger);
			GetCtrlVal (panelMain, MAIN_NUM_copies, &seq->nCopies);
			GetCtrlVal (panelMain, MAIN_STRING_seqShortName, tmpShortName);
			strReplaceChar (tmpShortName, '\\', '_');
			strReplaceChar (tmpShortName, '/', '_');
			strReplaceChar (tmpShortName, '!', '_');
			strReplaceChar (tmpShortName, '>', '_');
			strReplaceChar (tmpShortName, '<', '_');
			strReplaceChar (tmpShortName, '?', '_');
			strReplaceChar (tmpShortName, '.', '_');
			strReplaceChar (tmpShortName, ':', '_');
			strReplaceChar (tmpShortName, '*', '_');
			strReplaceChar (tmpShortName, '#', '_');
			strReplaceChar (tmpShortName, '|', '_');
			strcpy(seq->seqShortName,tmpShortName); 
			SetCtrlVal (panelMain, MAIN_STRING_seqShortName, tmpShortName);
			
			
			
			PARAMETERSWEEPS_updateDisplay (seq);
			SEQUENCE_displayList ();
			DIGITALBLOCKS_displaySequenceDuration (seq);
			setChanges(seq, 1);
			if (control == MAIN_TEXTBOX_description) {
				GetCtrlAttribute (panelMain, MAIN_TEXTBOX_description,
								  ATTR_STRING_TEXT_LENGTH, &entryLength);
				free (seq->description);
				seq->description  = (char *) malloc (entryLength+1);
				seq->description[0]=0;
			    GetCtrlVal (panelMain, MAIN_TEXTBOX_description, seq->description);
			}
				
			break;
		}
	return 0;
}







int CVICALLBACK MAIN_tabClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	printf ("(%d,%d,%d)", event, eventData1, eventData2);

	switch (event)
		{
		case EVENT_ACTIVE_TAB_CHANGE:    /* eventData1 = old active tab */
                                         /* = new active tab */
			switch (eventData2) {
				case TAB0_INDEX_SEQUENCE:
					//DIGITALBLOCKS_displayAllBlocks (activeSeq());
					//SetActiveCtrl(panel,MAIN_BTN_startSequence); // set button active to avoid table being active which makes update slower
//					DIGITALBLOCKS_displayValues (seq);
					break;
				case TAB0_INDEX_WFM:
					WFM_edit (activeSeq(), activeWfmNo(), 1);
					break;
				case TAB0_INDEX_GPIB:
					GPIB_displayValues (activeSeq(), activeGpibCommandNo());
					break;
			}
			break;
		case EVENT_COMMIT:
			break;
	}
	return 0;
}





/************************************************************************/
/************************************************************************/
/*
/*    output panel (progress bar etc.) 
/*
/************************************************************************/
/************************************************************************/



//=======================================================================
//
//   display panel 
//
//=======================================================================
void OUTPUT_showPanel (t_sequence *s)
{
    SetCtrlVal (panelMain, MAIN_NUMERICSLIDE_progress, 0.0);
    SetCtrlVal (panelSequence, SEQUENCE_SLIDE_progress2, 0.0);
    
	SetCtrlAttribute (panelMain, MAIN_LED_watchdog,
					  ATTR_VISIBLE, config->watchdogActivate);
	SetCtrlVal (panelMain, MAIN_RADIOBUTTON_suppress, config->suppressProtocol);
    
    OUTPUT_setSequenceInfo (s);
}	


//=======================================================================
//
//   set maximum number of points, name etc
//
//=======================================================================
void OUTPUT_setMaxPointsGenerated (unsigned __int64 max64)
{
   double max;
	
   max = ui64ToDouble (max64);
   SetCtrlAttribute (panelMain, MAIN_NUMERICSLIDE_progress,
					 ATTR_MAX_VALUE, max);
   SetCtrlAttribute (panelSequence, SEQUENCE_SLIDE_progress2,
					 ATTR_MAX_VALUE, max);
   SetCtrlAttribute (panelMain, MAIN_NUMERIC_of,
					 ATTR_CTRL_VAL, max);
   SetCtrlAttribute (panelIdleWait, IDLEWAIT_NUMERICSLIDE,
					 ATTR_MAX_VALUE, max);
   
   OUTPUT_displayPointsGenerated (0);
}


void OUTPUT_setRepetitions (t_sequence *s)
{
	SetCtrlVal (panelSequence, SEQUENCE_NUMERIC_rep, s->repetition+1);
	SetCtrlVal (panelMain, MAIN_NUMERIC_nRep, s->repetition+1);
	SetCtrlVal (panelMain, MAIN_NUMERIC_run,  s->run+1);
}



void OUTPUT_displayPointsGenerated (unsigned __int64 ui64Pts)
{
    double dPoints;
	double max;
	double lastVal;
	
    
   dPoints = ui64ToDouble (ui64Pts);

   GetCtrlAttribute (panelMain, MAIN_NUMERICSLIDE_progress,
					 ATTR_MAX_VALUE, &max);
   if (dPoints > max) dPoints = max;
   GetCtrlAttribute (panelMain, MAIN_NUMERICSLIDE_progress, ATTR_CTRL_VAL, &lastVal);
   if (dPoints != lastVal) {
    	SetCtrlVal (panelMain, MAIN_NUMERICSLIDE_progress, dPoints);
 	    SetCtrlVal (panelIdleWait, IDLEWAIT_NUMERICSLIDE, dPoints);
 	    SetCtrlVal (panelSequence, SEQUENCE_SLIDE_progress2, dPoints);
		ProcessDrawEvents();
   }
   

	   		
}



__int64 OUTPUT_setElapsedTime (int reset)
{
	static clock_t startTime;  
	unsigned time_ms;  
	
	if (reset) startTime = clock ();
	time_ms = timeStop (startTime);
	SetCtrlVal (panelMain, MAIN_STRING_elapsed, timeStrSec64 (time_ms/1000));
	return time_ms;
}


	


__int64 OUTPUT_setTotalDuration (t_sequence *seq)
{
	__int64 totalDuration;
	
	if ((seq == NULL) || (seq->outData == NULL)) {
		SetCtrlAttribute (panelMain, MAIN_STRING_durationSeq, ATTR_CTRL_VAL, "");
		return 0;
	}
	totalDuration = (__int64) seq->nCopies * (__int64) seq->outData->duration;
	if (seq->redoRepetitions) {
		totalDuration *= (__int64) seq->nRedo;
	}
	else {
		totalDuration *= (__int64) seq->nRepetitions * (__int64) seq->nRuns;
	}
	SetCtrlAttribute  (panelMain, MAIN_STRING_durationSeq, ATTR_CTRL_VAL, timeStrSec64 (totalDuration / VAL_s));
	return totalDuration / VAL_ms;
}




void OUTPUT_setTimes (t_sequence *seq, int estimateTime, int timeFirstRepetition)
{
	__int64 nExecutions;
	__int64 elapsed;
	__int64 remaining;
	__int64 timeFirstRepetition64;
	static __int64 *oldTimeFirstRepetition;
	static __int64 *oldEstimated = NULL;
	static __int64 *estimated = NULL;
	
    _int64p (&estimated);
    _int64p (&oldEstimated);

    // remember time of first repetition
    _int64p (&oldTimeFirstRepetition);
    if (timeFirstRepetition >= 0) timeFirstRepetition64 = timeFirstRepetition;
    else timeFirstRepetition64 = *oldTimeFirstRepetition;
   	*oldTimeFirstRepetition = timeFirstRepetition64;

	
	if (seq->redoRepetitions) nExecutions = seq->nRedo;
	else nExecutions = seq->nRepetitions * seq->nRuns;

	elapsed = OUTPUT_setElapsedTime (0);      
					 
	if (estimateTime) {
		if (seq->nExecutionsDone == 0) {
			*estimated = OUTPUT_setTotalDuration (seq) + 100 * nExecutions;
			*oldEstimated = -1;
		}
		else {
			*estimated = ( (elapsed - timeFirstRepetition64) * nExecutions) / seq->nExecutionsDone + timeFirstRepetition64;
			if (*oldEstimated > 0) {
				// only show if estimate changes by more that 10 %
				if ((seq->nExecutionsDone % 10 == 0) || (seq->nExecutionsDone > (nExecutions * 10)/9) ) {
					if (abs64(*oldEstimated - *estimated) < (*estimated / 10)) *estimated = *oldEstimated;
				}
			}		 
			*oldEstimated = *estimated;
		}	
	}
	
	SetCtrlAttribute (panelMain, MAIN_STRING_durationTotal, ATTR_CTRL_VAL, timeStrSec64 (*estimated/1000));
	if (*estimated >= elapsed) remaining = *estimated/1000 - elapsed/1000;
	else remaining = 0;
	SetCtrlAttribute (panelMain, MAIN_STRING_remaining, ATTR_CTRL_VAL, timeStrSec64 (remaining));
}

	


void OUTPUT_setSequenceInfo (t_sequence *s)
{
	if (s == NULL) return;
	SetCtrlVal (panelMain, MAIN_STRING_sequence, extractFilename (s->filename));
	SetCtrlVal (panelMain, MAIN_STRING_running_seq, s->seqShortName);
	
	SetCtrlVal (panelMain, MAIN_NUMERIC_nRep, 0);
	SetCtrlVal (panelMain, MAIN_NUMERIC_run,  0);
	SetCtrlAttribute (panelMain, MAIN_NUMERIC_run,
					 ATTR_MAX_VALUE, s->nRuns);
	SetCtrlVal (panelMain, MAIN_NUMERIC_totalRun, s->nRuns);
	SetCtrlAttribute (panelMain, MAIN_NUMERIC_nRep, ATTR_MAX_VALUE, s->nRepetitions);
	SetCtrlVal (panelMain, MAIN_NUMERIC_nTotalRep,
			   s->nRepetitions);
	SetCtrlVal (panelMain, MAIN_STRING_gpibCommand, "");
	SetCtrlVal (panelMain, MAIN_LED_gpib, 0);
	SetCtrlVal (panelMain, MAIN_LED_TCP_receive, 0);
	SetCtrlVal (panelMain, MAIN_LED_TCP_send, 0);
	SetCtrlVal (panelMain, MAIN_NUMERICSLIDE_progress, 0.0);
	SetCtrlVal (panelSequence, SEQUENCE_SLIDE_progress2, 0.0);
	OUTPUT_setTotalDuration (s);
}






//=======================================================================
//
//   parser and handler for remote commands, arriving from TCP/IP Interface
//   see INTERFACE_TCP_IP for definitions of commands
//
//=======================================================================

//
//  LOAD "xxx.seq"  
//  RUN "xxx.seq" 
//  GPIB deviceNo "string"
//
//  DELETE "xxx.seq" 



int  MAIN_remoteLoadSequence (char *filename)
{
	int seqNr;
	int error;
	t_sequence *s;

	RemovePopup (1);
	seqNr = SEQUENCE_nrFromFilename (filename);
    if (seqNr > 0) {
		// ---------------------------------------------
    	//   close sequence (without check for changes)
		// ---------------------------------------------
    	s = SEQUENCE_ptr (seqNr);
		ListRemoveItem (config->listOfSequences, 0, seqNr);
	    //MAIN_removeSequenceFromQueue(s); 
		SEQUENCE_free (s); 
	    free (s);
	}
	// ---------------------------------------------
	//   load sequence
	// ---------------------------------------------
	error = MAIN_loadSequence (filename);
	if (nSequences() == 0) {
		//   check if no sequences in memory 
	    SEQUENCE_appendToList (SEQUENCE_createEmptySequence (), 1);
		SEQUENCE_setAsActive (nSequences());
	}
	return error;

}

int MAIN_remoteStopRun (void);


/*
int MAIN_remoteStartRun (char *buffer, unsigned long bufferSize)
{
	t_run *r;
	t_sequence *s;
	
	while ((SEQUENCE_getStatus() != SEQ_STATUS_FINISHED)) {
		ProcessSystemEvents ();
	}
	r = (t_run *) malloc (sizeof (t_run));
	RUN_init (r);
	Ini_ReadFromBuffer (r->ini, buffer, bufferSize);
	Ini_DisplayContents (r->ini);
	Ini_GetRun (r->ini, r);
	s = SEQUENCE_fromFilename (r->sequenceFilename);
//	s->run = r;
	SEQUENCE_startOrEnqueue (s);
	s->run = NULL;
	RUN_free (r);	
	return 0;
}
*/


t_sequence *MAIN_remoteGetSequence (char *buffer, unsigned long bufferSize)
{
	t_sequence *s, *new;
	IniText ini;
	int seqNr;
	int activeSeqClosed = 0;
	
	RemovePopup (1);
	new = SEQUENCE_new ();
	if (new == NULL) {
		return NULL;
	}
	SEQUENCE_init (new);

	ini = Ini_New (0);
	Ini_ReadFromBuffer (ini, buffer, bufferSize);
//	Ini_DisplayContents (ini);
	Ini_GetSequence (ini, new);
	Ini_Dispose (ini);
	
	PROTOCOLFILE_printf ("FUNC: MAIN_remoteGetSequence: seq=%08x", new);
	

	seqNr = SEQUENCE_nrFromFilename (new->filename);
    if (seqNr != 0) {
		// ---------------------------------------------
    	//   close sequence (without check for changes)
		// ---------------------------------------------
    	s = SEQUENCE_ptr (seqNr);
    	if (s == activeSeq()) {
    		activeSeqClosed = 1;
    	}
		ListRemoveItem (config->listOfSequences, 0, seqNr);
	    //MAIN_removeSequenceFromQueue(s); 
		SEQUENCE_free (s); 
	    free (s);
	}
	SEQUENCE_appendToList (new, 1);
	
	if (activeSeqClosed) SEQUENCE_setAsActive (nSequences());
	return new;
}



void MAIN_remoteButton (int shortcutKey) 
{
	remoteKey = shortcutKey;
	
	if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) return;
	
	RemovePopup (1);
	SetActiveCtrl (panelMain, MAIN_BTN_startSequence);
	ProcessSystemEvents();
	
	SetCtrlAttribute (panelMain, MAIN_COMMANDBUTTON_remote,
					  ATTR_VISIBLE, 1);
	SetActiveCtrl (panelMain, MAIN_COMMANDBUTTON_remote);
}


void MAIN_setDigitalLines (char *stateStr)
{
	short state;
	char *parseStart, *parseEnd;
	int i;
	
	parseStart = stateStr;
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		state = (short)strtol (parseStart, &parseEnd, 10);
		if (parseEnd != NULL) parseStart = parseEnd + 1;
		else parseStart = NULL;
//    	if (config->invertDigitalOutput) state = !state;
		HARDWARE_sendDirectDigitalValue (i, state);
	}
	
}


void MAIN_parseTCPString (char *buffer, unsigned long bufferSize)
{
	char *parts[MAX_COMMANDSTRING_PARTS];
	int nParts;
	unsigned long l;
	t_sequence *s;
	int nr;
	int cmdID;

    cutCommandString (buffer, parts, &nParts);
// ---------------------------------------------
//     LOAD sequence
// ---------------------------------------------
   	l = strlen(buffer)+1;
   	cmdID = TCP_cmdStrID (parts[0]);
   	PROTOCOLFILE_printf ("FUNC: MAIN_parseTCPString, Received Command: %s", parts[0]);
	HARDWARE_setTransmit_CMD_READY (0);			
   	switch (cmdID) {
   		case TCP_CMD_ID_LOAD:
			if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
    	 		MAIN_remoteLoadSequence (parts[1]) ;
    	 	}
    	 	break;
    	case TCP_CMD_ID_RUN:
	    	// start sequence
			if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
		    	if (SHOWTIMES) mytime = clock ();
				s = MAIN_remoteGetSequence (&buffer[l], bufferSize-l);
				s->startNr		   = -1;		
				s->runStart        = 0;
				s->repetitionStart = 0;
				s->remote          = 1;
				if (nParts >= 2) StrToInt (parts[1], &s->repetitionStart);
				if (nParts >= 3) StrToInt (parts[2], &s->runStart);
				if (nParts >= 4) StrToInt (parts[3], &s->startNr);
		
				SEQUENCE_setShortcut (s, VAL_F5_VKEY);
		//		SetActivePanel (panelMain);
				MAIN_remoteButton (VAL_F5_VKEY);
			}
			break;
		case TCP_CMD_ID_STOP:
			if (SEQUENCE_getStatus()== SEQ_STATUS_STARTED) {
				MAIN_stopSequence ();
			}
			break;
		case TCP_CMD_ID_SEQUENCE:
			if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
	    		MAIN_remoteGetSequence (&buffer[l], bufferSize-l);
	    	}
    		break;
    	case TCP_CMD_ID_CLOSE:
			if (strlen (parts[1]) != 0) {
				// get number from sequence name
				if (SEQUENCE_getStatus() != SEQ_STATUS_STARTED) {
					nr = SEQUENCE_nrFromFilename (buffer+strlen(parts[0])+1);
					MAIN_closeSequence (nr, 0);    
				}
			}
			break;
		case TCP_CMD_ID_F5:
			HARDWARE_setTransmit_CMD_READY (1);			
			MAIN_remoteButton (VAL_F5_VKEY);
			break;
		case TCP_CMD_ID_F6:
			HARDWARE_setTransmit_CMD_READY (1);			
			MAIN_remoteButton (VAL_F6_VKEY);
			break;
		case TCP_CMD_ID_F7:
			HARDWARE_setTransmit_CMD_READY (1);			
			MAIN_remoteButton (VAL_F7_VKEY);
			break;
		case TCP_CMD_ID_F8:
			HARDWARE_setTransmit_CMD_READY (1);			
			MAIN_remoteButton (VAL_F8_VKEY);
			break;
		case TCP_CMD_ID_F9:
			HARDWARE_setTransmit_CMD_READY (1);			
			MAIN_remoteButton (VAL_F9_VKEY);
			break;
    	case TCP_CMD_ID_DIGITAL_LINES:
    		MAIN_setDigitalLines (parts[1]);
    		break;
    	case TCP_CMD_ID_QUIT:
    		QuitUserInterface(0);
    		break;
    }
}	 
	 
	 
	 
 // TRANSFER "xxx.seq"
// DELETE "xxx.seq"
// LOAD "example.seq"
//
// START 500 test.seq
//
// EXECUTE 1 circRamps.seq


int CVICALLBACK OUTPUT_suppressChanged_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panel, control, &config->suppressProtocol);
			TCP_setDisplayProtocol (!config->suppressProtocol);
			break;
	}
	return 0;
}




void SEQUENCE_moveinQueue (int oldIndex, int direction)
{
	int newIndex;
	t_sequence *newItem, *oldItem;
	int n;
	
	CmtGetLock(lockSequenceQueue);
	
	n = ListNumItems(listOfWaitingSequences);
	if ((oldIndex < 1) || (oldIndex > n)) return;
	newIndex = oldIndex + direction;
	if ((newIndex < 1) || (newIndex > n)) return;
	ListGetItem (listOfWaitingSequences, &oldItem, oldIndex);
	ListGetItem (listOfWaitingSequences, &newItem, newIndex);
	ListReplaceItem (listOfWaitingSequences,  &newItem, oldIndex);
	ListReplaceItem (listOfWaitingSequences,  &oldItem, newIndex);
	
	CmtReleaseLock(lockSequenceQueue); 
	
    SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_SequenceQueue, oldIndex-1, 
	       SEQUENCE_createListEntry (newItem), oldIndex);
    SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_SequenceQueue, newIndex-1, 
	       SEQUENCE_createListEntry (oldItem), newIndex);
	SetCtrlAttribute (panelMain, MAIN_LISTBOX_SequenceQueue,
					  ATTR_CTRL_INDEX, newIndex-1);
}


void SEQUENCE_moveinList (t_sequence *s, int oldIndex, int direction)
{
	int newIndex;
	t_sequence *newItem, *oldItem;
	int n;
	
	n = nSequences();
	if ((oldIndex < 1) || (oldIndex > n)) return;
	newIndex = oldIndex + direction;
	if ((newIndex < 1) || (newIndex > n)) return;
	if (oldIndex == config->activeSequenceNr) config->activeSequenceNr = newIndex;	
	if (oldIndex == config->idleSequenceNr) {
		config->idleSequenceNr = newIndex;
	} else if (newIndex == config->idleSequenceNr) {
		config->idleSequenceNr = oldIndex;        
	}
	ListGetItem (config->listOfSequences, &oldItem, oldIndex);
	ListGetItem (config->listOfSequences, &newItem, newIndex);
	ListReplaceItem (config->listOfSequences,  &newItem, oldIndex);
	ListReplaceItem (config->listOfSequences,  &oldItem, newIndex);
    SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_Sequences, oldIndex-1, 
	       SEQUENCE_createListEntry (newItem), oldIndex);
    SEQUENCE_OverrideListItem (panelMain, MAIN_LISTBOX_Sequences, newIndex-1, 
	       SEQUENCE_createListEntry (oldItem), newIndex);
	SetCtrlAttribute (panelMain, MAIN_LISTBOX_Sequences,
					  ATTR_CTRL_INDEX, newIndex-1);
//	for (i = 0; i < n; i++)  {
//		SetTreeItemAttribute (panelSMain, SMAN_TREE_sessions, i, ATTR_SELECTED, i == (newIndex-1));
//	}
//	SetCtrlAttribute (panelSMain, SMAN_TREE_sessions,
//					  ATTR_CTRL_INDEX, newIndex-1);
}

int CVICALLBACK SEQUENCE_moveInList_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nr;
	int i;
	
	switch (event)
		{
		case EVENT_COMMIT:
			nr = getActiveSeqNr();
			switch (control) {
				case MAIN_COMMANDBUTTON_up:
					SEQUENCE_moveinList (activeSeq(), nr, -1);
					break;
				case MAIN_COMMANDBUTTON_down:
					SEQUENCE_moveinList (activeSeq(), nr, 1);
					break;
				case MAIN_COMMANDBUTTON_top:
					for (i = nr; i > 1; i--) {
						SEQUENCE_moveinList (activeSeq(), i, -1);
						Sleep(50);
						ProcessDrawEvents();
					}
					break;
				case MAIN_COMMANDBUTTON_bottom:
					for (i = nr; i <= nSequences(); i++) {
						SEQUENCE_moveinList (activeSeq(), i, 1);
						Sleep(50);
						ProcessDrawEvents();
					}
					break;
			}
			CONFIG_write (config);  
			break;
		}
	return 0;
}

int CVICALLBACK SEQUENCE_moveInQueue_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nr;
	int i;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, MAIN_LISTBOX_SequenceQueue, &nr);  
			switch (control) {
				case MAIN_COMMANDBUTTON_up2:
					SEQUENCE_moveinQueue (nr, -1);
					break;
				case MAIN_COMMANDBUTTON_down2:
					SEQUENCE_moveinQueue (nr, 1);
					break;
				case MAIN_COMMANDBUTTON_top2:
					for (i = nr; i > 1; i--) {
						SEQUENCE_moveinQueue (i, -1);
						Sleep(50);
						ProcessDrawEvents();
					}
					break;
				case MAIN_COMMANDBUTTON_bottom2:
					for (i = nr; i <= ListNumItems(listOfWaitingSequences); i++) {
						SEQUENCE_moveinQueue (i, 1);
						Sleep(50);
						ProcessDrawEvents();
					}
					break;
			}
			//CONFIG_write (config);  
			break;
		}
	return 0;
}









void SEQUENCE_displayStartNr (int currentStartNr) 
{
	SetCtrlAttribute (panelMain, MAIN_NUMERIC_currentStart, ATTR_DIMMED, 
					  !config->autoSaveSequencesAfterEachStart );
	SetCtrlAttribute (panelMain, MAIN_TEXTMSG_startNr, ATTR_DIMMED, 
					  !config->autoSaveSequencesAfterEachStart);
	
	SetCtrlVal (panelMain, MAIN_RADIOBUTTON_autoSave, config->autoSaveSequencesAfterEachStart);
	if (currentStartNr >= 0) {
		SetCtrlVal (panelMain, MAIN_NUMERIC_currentStart, currentStartNr);
		SetCtrlVal (panelMain, MAIN_NUMERIC_nextStart, currentStartNr+1);
	}
	
	
}	
	


int SEQUENCE_writeCommandToFile (const char *command)
{
	char filename[MAX_PATHNAME_LEN];
	int file;
	
	if (!config->createFilenameForImages) return 0;
	
	changeSuffix (filename, config->imageFilenameExtensionPath, ".cmd");
	
	file = OpenFile (filename, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
	if (file < 0) {
		Sleep (50);
		file = OpenFile (filename, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
		if (file < 0) {
			tprintf ("Error opening %s.", filename);
			return -1;
		}
	}
	WriteFile (file, command, strlen(command)+1);
	CloseFile (file);
	return 0;
}



double SEQUENCE_duration_ms (t_sequence *seq)
{
    t_digitalBlock *b;

    b = DIGITALBLOCK_ptr (seq, ListNumItems (seq->lDigitalBlocks));
	if (b != NULL) {
		return ulToTime(b->absoluteTime + b->duration);
	}
	else return 0;
}



int SEQUENCE_autoSaveSequence (t_sequence *s, int alwaysAutoSave)
{
	char filename[MAX_PATHNAME_LEN];
	char sweepFilename[MAX_PATHNAME_LEN];
	char sweepFilenameShort[MAX_PATHNAME_LEN];
	char sequencePath[MAX_PATHNAME_LEN];
	char imageFilenameExtension[MAX_PATHNAME_LEN];
	int status;
	int file;
	
	
	
	if ((!config->autoSaveSequencesAfterEachStart) && (!alwaysAutoSave)) {   // FIXME why is the GPIB feedback done here???
		GPIB_addFrequencyFeedbackToAllCommands (s, 0);	
		return 0;
	}

	CONFIG_getCurrentStartNumber ();
	s->startNr = config->startNr;
	strcpy (s->startDate, CONFIG_getPathSuffixToday());
	if (s->startNr == 0) {
		PostMessagePopupf("Error", "Error: Auto save sequences failed.\nCould not determine start number.\n\n"
					  "You could unselect 'auto save sequences after each start' in the Configuration menu and go on.");
		return -1; // error occurred
	}
	SEQUENCE_displayStartNr (s->startNr);

	RemoveSurroundingWhiteSpace (s->seqShortName);
	if (s->seqShortName[0] == 0) {
		strncpy (s->seqShortName, extractFilename(s->filename), 30);
	}
	changeSuffix (s->seqShortName, s->seqShortName, "");

	sprintf (sequencePath, startDirName, s->startNr, s->seqShortName);
	
	strcpy (s->currentPath, config->dataPathToday);
	strcat (s->currentPath, sequencePath);
//	changeSuffix (s->currentPath, filename, "\\");
	
	status = mkDirs (s->currentPath);
	if (status != 0) {
		PostMessagePopupf ("File error!", "An error occurred when trying to create directory\n'%s'.\n\n"
				   	   "Error code: %d, %s",
				   		config->dataPathToday, status, getErrorFileOperation (status));
		return -1;
	}
	
	strcpy (filename, s->currentPath);
	strcat (filename,"\\");
	strcpy (sweepFilename, filename);

	strcat (filename, extractFilename (s->filename));
	
	SEQUENCE_save (filename, s);
	SetFileAttrs (filename, 1, -1, -1, -1);
	
	strcat (sweepFilename, intToStr0(N_STARTNR_DIGITS,s->startNr));
	
	strcpy (sweepFilenameShort, sweepFilename); 
	GPIB_addFrequencyFeedbackToAllCommands (s, sweepFilename);
	strcat (sweepFilename, "_sweeps.txt");
	strcat (sweepFilenameShort, "_sweepsShort.txt");
	
	PARAMETERSWEEPS_createTxtFile (sweepFilename, s, 0);	
	PARAMETERSWEEPS_createTxtFile (sweepFilenameShort, s, 1);	
	SEQUENCE_displayList ();	
	
	if (!config->createFilenameForImages) return 0;
	
	if (config->imageFilenameExtensionPath[0] == 0) {
		PostMessagePopupf ("File error!", "Could not create file that contains image file extension.\nNo file selected.");
		return -1;
	}
			
	file = OpenFile (config->imageFilenameExtensionPath, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
	if (file == -1) {
		displayFileError (config->imageFilenameExtensionPath);
		return -1;
	}
	
	strcpy (imageFilenameExtension, CONFIG_getPathSuffixToday());
	strcat (imageFilenameExtension, sequencePath);
	strcat (imageFilenameExtension, "\\");
	strcat (imageFilenameExtension, sequencePath);
//	strcat (imageFilenameExtension, "");

	WriteFile (file, imageFilenameExtension, strlen(imageFilenameExtension)+1);
	CloseFile (file);
	

	return 0;
}		
	

int CVICALLBACK MAIN_autosaveSequences (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, control, &config->autoSaveSequencesAfterEachStart);
			CONFIG_getCurrentStartNumber ();
			SEQUENCE_displayStartNr (config->startNr-1);
			CONFIG_write(config);
			break;
	}
	return 0;
}



int CVICALLBACK MAIN_Timer_autoQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
            if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) return 0;
			if (FileExists (AUTOQUIT_FILENAME, 0)) {
				if (DeleteFile (AUTOQUIT_FILENAME) == 0) {
				    CONFIG_write (config);
					QuitUserInterface (0);		
				}
			}
			break;
	}
	return 0;
}




int CVICALLBACK OUTPUT_panelCallback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	
	switch (event)
	{
		case EVENT_KEYPRESS:
			switch (eventData1) {
				case VAL_F9_VKEY:
					SetActivePanel (panelMain);
					BTN_StartSequence (panelMain, MAIN_BTN_startSequence, EVENT_COMMIT,
						0, 0, 0);
					break;
			}
			break;
			
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		case EVENT_PANEL_MOVE:
			break;
	}
	return 0;
}

void CVICALLBACK MENU_SequenceStart (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	BTN_StartSequence (panelMain, MAIN_BTN_startSequence, EVENT_COMMIT,
						0, 0, 0);
}


void CVICALLBACK MENU_SequenceStartFrom (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	t_sequence *seq;
	
	if (panelSequenceStartFrom <= 0) {
		panelSequenceStartFrom = LoadPanel (0, UIR_File, STARTFROM);
		SetCtrlAttribute (panelSequenceStartFrom, STARTFROM_BTN_startSequence,
		                 ATTR_CMD_BUTTON_COLOR, VAL_GREEN);
		
	}
	seq = activeSeq();
	SetCtrlVal (panelSequenceStartFrom, STARTFROM_NUMERIC_from, seq->repetition);
	SetCtrlAttribute (panelSequenceStartFrom, STARTFROM_NUMERIC_to,
					  ATTR_MAX_VALUE, seq->nRepetitions);
	SetCtrlAttribute (panelSequenceStartFrom, STARTFROM_NUMERIC_from,
					  ATTR_MAX_VALUE, seq->nRepetitions);
	SetCtrlVal (panelSequenceStartFrom, STARTFROM_NUMERIC_to, seq->nRepetitions);
	InstallPopup (panelSequenceStartFrom);
}


int CVICALLBACK PANEL_STARTFROM_callback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			RemovePopup (0);
			break;
		case EVENT_KEYPRESS:
			if (eventData1 == VAL_ESC_VKEY) RemovePopup (0);
			break;

	}
	return 0;
}



int CVICALLBACK STARTFROM_BTN_StartSequence (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	int h;
	
	switch (event)
	{
		case EVENT_COMMIT:
			if (SEQUENCE_getStatus() == SEQ_STATUS_FINISHED) {
				seq = activeSeq();
				seq->remote = 0;
				GetCtrlVal (panelSequenceStartFrom, STARTFROM_NUMERIC_from, &h);
				seq->repetitionStart = h-1;
				GetCtrlVal (panelSequenceStartFrom, STARTFROM_NUMERIC_to, &seq->repetitionStop);
				RemovePopup (0);
				SEQUENCE_startOrEnqueue (seq);
				seq->repetitionStart = 0;
				seq->repetitionStop = 0;
			}

			break;
	}
	return 0;
}





enum {
	PARAMETERSWEEP_LIST_COL_NAME = 0,
	PARAMETERSWEEP_LIST_COL_QUANTITY,
	PARAMETERSWEEP_LIST_COL_DESCRIPTION,
	PARAMETERSWEEP_LIST_COL_FROM,
	PARAMETERSWEEP_LIST_COL_TO,
	PARAMETERSWEEP_LIST_COL_INCREMENT,
	PARAMETERSWEEP_LIST_COL_INCREMENT_REP,
	PARAMETERSWEEP_LIST_COL_RESET_REP,
	
	N_PARAMETERSWEEP_COLS
};

	


void PARAMETERSWEEPS_displaySweepsInList (t_sequence *s, int panel, int ctrl)
{
	int n;
	int i;
	t_parameterSweep *p;
	char help[50];
	
	n = ListNumItems (s->lParameterSweeps);
	setNumTreeItems (panel, ctrl, n);
	for (i = 1; i <= n; i++) {
		ListGetItem (s->lParameterSweeps, &p, i);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_NAME, ATTR_LABEL_TEXT, p->channelName);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_NAME,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_LEFT_JUSTIFIED);

				
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_QUANTITY, ATTR_LABEL_TEXT, p->quantity);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_QUANTITY,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_LEFT_JUSTIFIED);

		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_DESCRIPTION, ATTR_LABEL_TEXT, p->description);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_DESCRIPTION,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_LEFT_JUSTIFIED);

		sprintf (help, "%1.4f %s ", p->from, p->units);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_FROM, ATTR_LABEL_TEXT, help);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_FROM,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);

		sprintf (help, "%1.4f %s ", p->to, p->units);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_TO, ATTR_LABEL_TEXT, help);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_TO,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);

		sprintf (help, "%1.4f %s ", p->increment, p->units);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_INCREMENT, ATTR_LABEL_TEXT, help);
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_INCREMENT,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);

		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_INCREMENT_REP, ATTR_LABEL_TEXT, 
				 intToStr (p->incrementAfterRepetitions));
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_INCREMENT_REP,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);

		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_RESET_REP, ATTR_LABEL_TEXT, 
			p->resetAfterRepetitions == VAL_NO_RESET_REPETITIONS ? "" : intToStr (p->resetAfterRepetitions));
		SetTreeCellAttribute (panel, ctrl, i-1, PARAMETERSWEEP_LIST_COL_RESET_REP,
							  ATTR_LABEL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
	}
	
}



void PARAMETERSWEEP_initListCtrl (int panel, int ctrl)
{
	int PARAMETERSWEEP_listColWidth[N_PARAMETERSWEEP_COLS] = {
		170, 120, 120, 120, 120, 120, 40, 40
	};
	char *PARAMETERSWEEP_listColLabels[N_PARAMETERSWEEP_COLS] = {
		"channel/device name", "quantity", "where?", "from", "to", "increment", "every\nN reps", "reset\nevery\nN reps"
	};
	int i;
	
	ClearListCtrl (panel, ctrl);
	setNumTreeColumns (panel, ctrl, N_PARAMETERSWEEP_COLS);
	SetCtrlAttribute (panel, ctrl, ATTR_COLUMN_LABELS_HEIGHT, 50);
	SetCtrlAttribute (panel, ctrl, ATTR_SHOW_CONNECTION_LINES, 0);
	
	for (i = 0; i < N_PARAMETERSWEEP_COLS; i++) {
		SetTreeColumnAttribute (panel, ctrl, i, ATTR_COLUMN_WIDTH, PARAMETERSWEEP_listColWidth[i]);
		SetTreeColumnAttribute (panel, ctrl, i, ATTR_LABEL_TEXT,
								PARAMETERSWEEP_listColLabels[i]);
	}
	
}



void PARAMETERSWEEPS_updateDisplay (t_sequence *s) 
{
	if (s == NULL) return;
	
	PARAMETERSWEEP_getAllFromSequence (s); 
	SetCtrlAttribute (panelMain, MAIN_TREE_sweeps, ATTR_DIMMED, 
					  ListNumItems(s->lParameterSweeps) == 0 );
	PARAMETERSWEEPS_displaySweepsInList (s, panelMain, MAIN_TREE_sweeps);
	
}



void PARAMETERSWEEPS_initDisplayWindow (void)
{
	if (panelSweeps <= 0) panelSweeps = LoadPanel (0, UIR_File, SWEEPS);
}



int PARAMETERSWEEPS_createTxtFile (const char *filename, t_sequence *s, int shortVersion) 
{
	#define lineLen 2000
	char line[lineLen];
	char help[200], help1[200];
	char *appendPos;
	int file;
	int i,n,r;
 	t_parameterSweep *p;
	double value;

	file = OpenFile (filename, VAL_WRITE_ONLY, VAL_TRUNCATE,
					 VAL_ASCII);
	if (file < 0) {
		displayFileError (filename);
		return -1;
	}
			
	PARAMETERSWEEP_getAllFromSequence (s);
	
	

	n = ListNumItems (s->lParameterSweeps);

	
	if (!shortVersion) {
		// ----------------------
		//    header				
		// ----------------------
		strcpy (line, "repetition");
		appendPos = line + strlen (line);
		for (i = 1; i <= n; i++) {
			ListGetItem (s->lParameterSweeps, &p, i);
			strcpy (help1, p->channelName);
			strcpy (help1+20, "...");
			sprintf (help, "\t%s-%s (%s)", p->quantity, help1, p->units);
			strncpy (appendPos, help, lineLen - (appendPos-line)-2);
			appendPos += strlen (help);
		}
		line[lineLen-1] = 0;
		WriteLine (file, line, -1);

		// ----------------------
		//    units
		// ----------------------
		strcpy (line, "");
		appendPos = line + strlen (line);
		if (n == 0) {
			sprintf (help, "\trepetition");
			strncpy (appendPos, help, lineLen - (appendPos-line)-2);
			appendPos += strlen (help);
		}
		
		for (i = 1; i <= n; i++) {
			ListGetItem (s->lParameterSweeps, &p, i);
			sprintf (help, "\t%s", p->units);
			strcpy (appendPos, help);
			appendPos += strlen (help);
		}
		line[lineLen-1] = 0;
		WriteLine (file, line, -1);
	}
	
	// ----------------------
	//    sweep params
	// ----------------------
	if (shortVersion) {
		for (r = 0; r < s->nRepetitions; r++) {
			if (ListNumItems (s->lParameterSweeps) > 0) {
				ListGetItem (s->lParameterSweeps, &p, 1);
				value =  PARAMETERSWEEPS_getValueForRepetition (p, r);
			}
			else value = 0;
			sprintf (line, "%1.9f", value);
			WriteLine (file, line, -1);
		}
	}
	else {
		for (r = 0; r < s->nRepetitions; r++) {
			strcpy (line, intToStr (r+1));
			appendPos = line + strlen (line);
			if (n == 0) {
				sprintf (help, "\t%d", r+1);
				strncpy (appendPos, help, lineLen - (appendPos-line)-2);
				appendPos += strlen (help);
			}
			else {
				for (i = 1; i <= n; i++) {
					ListGetItem (s->lParameterSweeps, &p, i);
					value =  PARAMETERSWEEPS_getValueForRepetition (p, r);
					sprintf (help, "\t%1.6f", value);
					strncpy (appendPos, help, lineLen - (appendPos-line)-2);
					appendPos += strlen (help);
				}	
			}
			line[lineLen-1] = 0;
			WriteLine (file, line, -1);
		}
	}

	CloseFile (file);
	return 0;
	
}


void PARAMETERSWEEPS_displayForEachRepetitionInTable (t_sequence *s, int panel, int table) 
{
	t_parameterSweep *p;
	int i;
	int r;
	char help[200], help1[200];
	double value;
	
	if (s->nRepetitions > 1000) return;
	DeleteTableRows (panel, table, 1, -1);
	DeleteTableColumns (panel, table,  1, -1);
	InsertTableColumns (panel, table,  -1, ListNumItems (s->lParameterSweeps),
						VAL_CELL_NUMERIC);
	SetTableColumnAttribute (panel, table, -1,
							 ATTR_LABEL_WRAP_MODE, VAL_WORD_WRAP);

	SetTableColumnAttribute (panel, table, -1, ATTR_USE_LABEL_TEXT,
							 1);
	SetTableColumnAttribute (panel, table, -1, ATTR_SIZE_MODE,
							 VAL_USE_EXPLICIT_SIZE);
	SetTableColumnAttribute (panel, table, -1, ATTR_COLUMN_WIDTH,
							 120);
	InsertTableRows (panel, table, 1, s->nRepetitions,
					 VAL_CELL_NUMERIC);
	SetTableColumnAttribute (panel, table, -1, ATTR_COLUMN_WIDTH,
							 120);
	for (i = 1; i <= ListNumItems (s->lParameterSweeps); i++) {
		ListGetItem (s->lParameterSweeps, &p, i);
		strcpy (help1, p->channelName);
		strcpy (help1+20, "...");
		sprintf (help, "%s - %s (%s)", p->quantity, help1, p->units);
		SetTableColumnAttribute (panel, table, i, ATTR_LABEL_TEXT,
						   help);
		for (r = 0; r < s->nRepetitions; r++) {
			value =  PARAMETERSWEEPS_getValueForRepetition (p, r);
			SetTableCellAttribute (panel, table, MakePoint (i, r+1),
								   ATTR_CTRL_VAL, value);
			SetTableCellAttribute (panel, table, MakePoint (i, r+1),
								   ATTR_CELL_MODE, VAL_INDICATOR);
			SetTableCellAttribute (panel, table, MakePoint (i, r+1), 
								   ATTR_PRECISION, 4);


		}
	}
	
}



void CVICALLBACK MENU_config (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	CONFIG_displayAllSettings (activeSeq());
	InstallPopup (panelConfiguration);
}



int CVICALLBACK MAIN_showSweepParameter_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int isVisible;
	switch (event)
	{
		case EVENT_COMMIT:
			PARAMETERSWEEPS_initDisplayWindow (); 
			GetPanelAttribute(panelSweeps, ATTR_VISIBLE, &isVisible);
			if (isVisible) {
				HidePanel (panelSweeps); 	
			} else {
				PARAMETERSWEEPS_displayForEachRepetitionInTable (activeSeq(), panelSweeps, SWEEPS_TABLE);
				DisplayPanel (panelSweeps);
			}
			break;
	}
	return 0;
}



int CVICALLBACK MAIN_RENAMESEQ_done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	char filename[MAX_PATHNAME_LEN];
	char newName[MAX_PATHNAME_LEN];
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			strcpy (filename, extractDir (seq->filename));
			GetCtrlVal (panel, RENAME_STRING, newName);
			strcat (filename, newName);
			strcat (filename, SEQUENCE_fileSuffixP);
			strcpy (seq->filename, filename);
			SEQUENCE_displayList ();
			seq->changes = 1;
			if (seq == activeSeq()) MAIN_setFilenameToTitle (seq->filename, seq->changes);
			RemovePopup (0);
			DiscardPanel (panel);
			break;
	}
	return 0;
}


int CVICALLBACK MAIN_RENAMESEQ_abort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup (0);
			DiscardPanel (panel);
			break;
	}
	return 0;
}



/*
int MAIN_showTiming (void)
{
	int val;
	
	GetCtrlVal (panelMain, MAIN_BUTTON_showTiming, &val);
	return val;
}
*/

int CVICALLBACK MAIN_saveSequenceNow_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SEQUENCE_autoSaveSequence (activeSeq(), 1);
			break;
	}
	return 0;
}

int CVICALLBACK MAIN_SWEEP_close_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (panel);
			break;
	}
	return 0;
}


int CVICALLBACK MAIN_browseSeq2_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


int CVICALLBACK MAIN_startIdle_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			idleSequenceEnabled = 1;
			MAIN_startIdleSequence_changeCtrls (1, 0);
			break;
	}
	return 0;
}


int CVICALLBACK MAIN_stopIdle_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			MAIN_stopIdleSequence();
			
			break;
	}
	return 0;
}



int CVICALLBACK MAIN_ADWIN_testOffsetSend_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int ch;
	double offset;
	int error;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelMain, MAIN_NUMERIC_channel, &ch);
			GetCtrlVal (panelMain, MAIN_NUMERIC_offset, &offset);
			error = HARDWARE_ADWIN_transmitAnalogOffsetDouble (offset, ch);			
			tprintf ("Transmitted %1.3f on ch %d. Error = %d\n", offset, ch, error);
			break;
	}
	return 0;
}



int CVICALLBACK BTN_stopWaitingForIdle_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			return 1;
	}
	return 0;
}

int CVICALLBACK BTN_skipWaitingForIdle_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK WFM_Rescue_CALLBACK (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SetPanelAttribute(panelMain,ATTR_WINDOW_ZOOM,VAL_NO_ZOOM);
			SetPanelSize(panelMain,848,1674);
			SetPanelPos(panelMain,44,67);
			break;
	}
	return 0;
}
