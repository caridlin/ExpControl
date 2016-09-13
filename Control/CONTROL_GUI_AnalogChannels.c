#include <utility.h>
#include <userint.h>
#include "UIR_ExperimentControl.h"

/*************************************************************************

   
   graphical user interface for editing analog channels
   
   
   (c) Stefan Kuhr 1999-2003
   
   Institut für Angewandte Physik, Universität Bonn
   
   
   

*************************************************************************/
#include "INCLUDES_CONTROL.h"    



int panelDACMaster = -1;  
int panelDACFrame  = -1;  
int panelDAO 	   = -1;  


#define AOColumns 4
int *DAC_Waveforms;

int panelAnalogChannels = -1;

t_digitalBlock *activeDigitalBlock = NULL;

// -----------------------------------------------------------
//    handles for subpanels
// -----------------------------------------------------------

int panelDACx[N_AO_CHANNELS];






/*int nDisplayDAC (void)
{
    int x;
    
    x = nAOBoards();
    if (x == 0) x = 1;
    return x*8;

}
*/



/************************************************************************/
/************************************************************************/
/*
/*    Initializaiton of panels, controls etc. 
/*
/************************************************************************/
/************************************************************************/




//=======================================================================
//
//    init DAC subpanels
//
//=======================================================================
/*
void initDACPanels (t_sequence *s)
{
    int i;
	int framethickness;
	int height, height2;
	int txtWidth;

    if (panelDACFrame == -1) {
        panelDACFrame  = LoadPanel (panelAnalogChannels, UIR_File, PANEL_FR_2);
        panelDACMaster = LoadPanel (panelDACFrame, UIR_File, DAC_MASTER);
        for (i = 0; i < N_AO_CHANNELS; i++) panelDACx[i] = -1;
        
        HidePanel (panelDACMaster);
		// -----------------------------------------------------------
		// -----------------------------------------------------------
	    //   configure "master panel"
	    // -----------------------------------------------------------
		// -----------------------------------------------------------
		GetPanelAttribute (panelDACMaster, ATTR_FRAME_THICKNESS,
	                       &framethickness);
	    setCtrlBoundingRect (panelDACMaster, DAC_MASTER_TEXTMSG_DAC_No,
	    					 0, 0, -1, -1);
		SetCtrlAttribute (panelDACMaster, DAC_MASTER_TEXTMSG_Hold,
						  ATTR_TEXT_BGCOLOR, VAL_MED_GRAY);
	    // rings with waveform names
	    GetCtrlAttribute (panelDACMaster, DAC_MASTER_DAC_Name, ATTR_HEIGHT, &height);
	    setCtrlBoundingRect (panelDACMaster, DAC_MASTER_RING_WFMs,
	    				     height, 0, -1, panelWidth(panelDACMaster));
	    GetCtrlAttribute (panelDACMaster, DAC_MASTER_RING_WFMs, ATTR_HEIGHT, &height2);
	    
	    setCtrlBoundingRect (panelDACMaster, DAC_MASTER_GRAPH_WFM,
	    				     height+height2, 0, 
	    				     panelHeight(panelDACMaster)-height-height2, 5+panelWidth(panelDACMaster));
		SetCtrlAttribute (panelDACMaster, DAC_MASTER_GRAPH_WFM,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
		SetCtrlAttribute (panelDACMaster, DAC_MASTER_GRAPH_WFM,
						  ATTR_EDGE_STYLE, VAL_FLAT_EDGE);

	    SetPanelPos (panelDACMaster, framethickness+1, framethickness-1);
    }

	// -----------------------------------------------------------
    //   generate child panels
	// -----------------------------------------------------------
    for (i=0; i < N_AO_CHANNELS; i++) {			 
        if ((panelDACx[i] <= 0) && (isAnalogChannelActive (s, i))) {
	        panelDACx[i] = DuplicatePanel (panelDACFrame, panelDACMaster,
	                                      str_AnalogChannelNames(i),
	                                      10, 10);
		    SetCtrlVal (panelDACx[i], DAC_MASTER_TEXTMSG_DAC_No,
					   strAdd (str_AnalogChannelNames(i), ":"));
			GetCtrlAttribute (panelDACx[i], DAC_MASTER_TEXTMSG_DAC_No, ATTR_WIDTH, &txtWidth);
			setCtrlBoundingRect (panelDACx[i], DAC_MASTER_DAC_Name,
								 0, txtWidth, -1, panelWidth(panelDACx[i])-txtWidth);
		}
	}
}   

*/


void ANALOGCHANNELS_calculateDACPanelPositions (t_sequence *seq, int *maxRight, int *maxBottom)
{
	int framethickness;
	
	int n, i;
	int top, left, width, height;
	
	GetPanelAttribute (panelDACMaster, ATTR_FRAME_THICKNESS,
                       &framethickness);
	n = 0;
	*maxRight = 0;
	*maxBottom = 0;
	width = panelWidth(panelDACMaster) + panelFrameThickness(panelDACMaster);
	height = panelHeight(panelDACMaster) + panelFrameThickness(panelDACMaster); 
	for (i = 0; i < N_AO_CHANNELS; i++) {
		if (isAnalogChannelActive(seq, i)) {
			top = framethickness + (n / AOColumns) * (panelHeight(panelDACMaster)+2*framethickness);
			left = framethickness + (n % AOColumns) * (panelWidth(panelDACMaster)+2*framethickness);
			if (top + height > *maxBottom) *maxBottom = top + height;
			if (left + width > *maxRight) *maxRight = left + width;
			SetPanelPos (panelDACx[i], top, left);
  			DisplayPanel (panelDACx[i]);
			n++;
  		}
  		else HidePanel (panelDACx[i]);
  	}
}




//=======================================================================
//
//    init panel "Analog Blocks"
//
//=======================================================================

int ANALOGCHANNELS_initPanel (t_sequence *seq, int blockNr)
{
    int maxRight, maxBottom;
    char *help;
    int scrollbarsize;


	if (panelAnalogChannels == -1) 
  		panelAnalogChannels = LoadPanel (0, UIR_File, ANALOG);

	help = getTmpString();
	
	if (blockNr == 0)
		strcpy (help, "Analog channels");
	else 
		sprintf (help, "Analog channels - Block no. %d", blockNr);
	SetPanelAttribute (panelAnalogChannels, ATTR_TITLE, help);
	
	initDACPanels (seq);
    ANALOGCHANNELS_calculateDACPanelPositions (seq, &maxRight, &maxBottom);

	// -----------------------------------------------------------
    //     initialize small waveform panels
	// -----------------------------------------------------------
    
    SetPanelPos (panelDACFrame, 10, 10);
    GetPanelAttribute (panelDACFrame,  ATTR_SCROLL_BAR_SIZE,    &scrollbarsize);
    SetPanelSize (panelDACFrame, maxBottom + 10, maxRight + scrollbarsize);
    
    SetCtrlAttribute (panelAnalogChannels, ANALOG_COMMANDBUTTON_Done, ATTR_TOP,
    				  panelBottom(panelDACFrame)+10);
    SetCtrlAttribute (panelAnalogChannels, ANALOG_COMMANDBUTTON_Done, ATTR_LEFT,
    				  panelRight(panelDACFrame)
    				  -ctrlWidth(panelAnalogChannels, ANALOG_COMMANDBUTTON_Done));
    
	// -----------------------------------------------------------
    //      determine waveform panel size
	// -----------------------------------------------------------
//	GetPanelAttribute (panelDACMaster, ATTR_FRAME_THICKNESS,    &framethickness);

/*    
    framePanelHeight = (1 + (N_AO_CHANNELS-1) / AOColumns) * (panelHeight(panelDACMaster)+2*framethickness);
    framePanelWidth  = framethickness + AOColumns * (panelWidth(panelDACMaster)+2*framethickness)
                     + scrollbarsize;
    SetPanelSize (panelDACFrame, framePanelHeight, framePanelWidth);
	// -----------------------------------------------------------
    //      get height of panel "ANALOG BLOCKS"
	// -----------------------------------------------------------
    mainPanelHeight = framePanelHeight;
    if (mainPanelHeight > screenHeight()-50) {
        mainPanelHeight = screenHeight()-50;
        framePanelHeight = mainPanelHeight;
    }
    SetPanelAttribute (panelAnalogChannels, ATTR_HEIGHT, mainPanelHeight+1);
	// -----------------------------------------------------------
    //      place panel with waveforms 
	// -----------------------------------------------------------

    SetPanelPos (panelDACFrame, 0, 10);
   */
   
    SetPanelAttribute (panelDACFrame, ATTR_PARENT_SHARES_SHORTCUT_KEYS, 1);
    SetPanelAttribute (panelDACMaster, ATTR_PARENT_SHARES_SHORTCUT_KEYS, 1);
	SetPanelAttribute (panelDACFrame, ATTR_ZPLANE_POSITION, 0);
    DisplayPanel (panelDACFrame);
    
    SetPanelSize (panelAnalogChannels, 
    			  ctrlBottom (panelAnalogChannels, ANALOG_COMMANDBUTTON_Done) + 10,
    			  ctrlRight  (panelAnalogChannels, ANALOG_COMMANDBUTTON_Done) + 10);
    SetPanelPos (panelAnalogChannels, 100, 100);
    
    return 0;
}




//=======================================================================
//
//     determine minimum height of panel 
//
//=======================================================================
int ANALOGBLOCKS_panelMinimumHeight(t_sequence *seq)
{
	int max;
	int bottom;
	int i;
	
	max = 0;
	for (i = 0; i < N_AO_CHANNELS; i++) {
		if (isAnalogChannelActive(seq, i)) {
		    bottom = panelBottom (panelDACx[i])
		             + panelFrameThickness (panelDACx[i]);
		    if (bottom > max) max = bottom;
		}
	}
	return max;
}










/************************************************************************/
/************************************************************************/
/*
/*    Display of DAC-Channels (i.e. small windows containing waveforms)
/*
/************************************************************************/
/************************************************************************/

int getDACpanelHandle (int nr)
{
   return panelDACx[nr];
}


// calculates the number of the DAC channel from the panelID
int nrPanel (int panelID)
{
   int i = 0;
   
   while ((panelID != panelDACx[i]) && (i < N_AO_CHANNELS)) i++;
   return i;
}



//=======================================================================
//
//    display waveform name in DACx
//
//=======================================================================


/*
void displayWaveformChangeVisibility (int panel, int wfmType, int stepCopies)
{
   	
   	SetCtrlVal (panel, DAC_MASTER_TEXTMSG_stepTitle, 
   	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_stepTitle, ATTR_VISIBLE, wfmType == WFM_TYPE_STEP);
	SetCtrlAttribute (panel, DAC_MASTER_NUMERIC_stepFrom,  ATTR_VISIBLE, wfmType == WFM_TYPE_STEP);
	
	
	SetCtrlAttribute (panel, DAC_MASTER_NUMERIC_stepTo,    ATTR_VISIBLE, wfmType == WFM_TYPE_STEP); 
	SetCtrlAttribute (panel, DAC_MASTER_NUMERIC_stepRep,   ATTR_VISIBLE, wfmType == WFM_TYPE_STEP); 
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_stepRep,   ATTR_VISIBLE, 
					  (wfmType == WFM_TYPE_STEP) && !stepCopies ); 
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_Arrow,     ATTR_VISIBLE, wfmType == WFM_TYPE_STEP);
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_Hold,      ATTR_VISIBLE, wfmType == WFM_TYPE_HOLD);
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_NoGraph,   ATTR_VISIBLE, wfmType == -1);
	SetCtrlAttribute (panel, DAC_MASTER_NUMERIC_constant,  ATTR_VISIBLE, wfmType == WFM_TYPE_CONSTVOLTAGE);
    SetCtrlAttribute (panel, DAC_MASTER_GRAPH_WFM, ATTR_VISIBLE, 
                      (wfmType == WFM_TYPE_POINTS) || (wfmType == WFM_TYPE_FILE));
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_stepCopies,  ATTR_VISIBLE, 
					  (wfmType == WFM_TYPE_STEP) && stepCopies); 

}

*/

//=======================================================================
//
//    display waveform in DACx
//
//=======================================================================
void displayWaveform (t_sequence *seq, int panel, t_digitalBlock *b, int wfmNr, int DACNo)
{
    t_waveform *wfmP;
//    t_analogBlock *analogP;
    int wfmType = -1;
    int AO_voltageSweepsOnly;
    
	// -----------------------------------------------------------
    //      determine waveform type
	// -----------------------------------------------------------
    switch (wfmNr) {
        case WFM_ID_UNCHANGED:
        	wfmP = NULL;
        	wfmType = -1;
        	break;
        case WFM_ID_CONSTVOLTAGE:
        	wfmP = NULL;
        	wfmType = WFM_TYPE_CONSTVOLTAGE; 
        	break;
        default:
        	wfmP = WFM_ptr (seq, wfmNr);
	    	if (wfmP != NULL) wfmType = wfmP->type;
	    	break;
	}
	// -----------------------------------------------------------
    //      display + hide controls
	// -----------------------------------------------------------
//									 
	AO_voltageSweepsOnly  = seq->AO_voltageSweepsOnly && (DACNo < N_DAC_CHANNELS);
	
	if (wfmP != NULL) {
		if (AO_voltageSweepsOnly) {
	   	 	SetCtrlVal (panel, DAC_MASTER_TEXTMSG_stepTitle, 
	   	 				"Step output voltage");
			SetCtrlVal (panel, DAC_MASTER_TEXTMSG_arrow, "®");
		}
		else {
	   	 	SetCtrlVal (panel, DAC_MASTER_TEXTMSG_stepTitle, 
	   	 				wfmP->stepAlternate ? "Alternate" :"Step" );
			SetCtrlVal (panel, DAC_MASTER_TEXTMSG_arrow,
						wfmP->stepAlternate ? "«" : "®");
		}
	}
	
	SetCtrlVal (panel, DAC_MASTER_TEXTMSG_step, 
				AO_voltageSweepsOnly ? "copies" : "repetition");
   	
   	SetAttributeForCtrls (panel, ATTR_VISIBLE, 
   						  (wfmType== WFM_TYPE_STEP) || AO_voltageSweepsOnly,
   						  0, 
   						  DAC_MASTER_TEXTMSG_stepTitle, 
   						  DAC_MASTER_NUMERIC_stepFrom,  
   						  DAC_MASTER_NUMERIC_stepTo,    
   						  DAC_MASTER_NUMERIC_stepRep,
   						  DAC_MASTER_TEXTMSG_step,
   						  DAC_MASTER_TEXTMSG_arrow,     
   						  0);
	
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_Hold, ATTR_VISIBLE, 
					  (wfmType == WFM_TYPE_HOLD) && !AO_voltageSweepsOnly);
	SetCtrlAttribute (panel, DAC_MASTER_TEXTMSG_NoGraph,   ATTR_VISIBLE, 
					  (wfmType == WFM_TYPE_NONE) && !AO_voltageSweepsOnly);
	SetCtrlAttribute (panel, DAC_MASTER_NUMERIC_constant,  ATTR_VISIBLE, 
					  (wfmType == WFM_TYPE_CONSTVOLTAGE) && !AO_voltageSweepsOnly);
    SetCtrlAttribute (panel, DAC_MASTER_GRAPH_WFM, ATTR_VISIBLE, 
                      (wfmType == WFM_TYPE_POINTS) || (wfmType == WFM_TYPE_FILE));
                    

//	displayWaveformChangeVisibility (panel, wfmType, 0);
	// -----------------------------------------------------------
    //      display values
	// -----------------------------------------------------------
  	displayWaveformName (panel, DAC_MASTER_RING_WFMs, wfmNr);
    switch (wfmType) {
        case WFM_TYPE_STEP:
			SetCtrlVal (panel, DAC_MASTER_NUMERIC_stepFrom, wfmP->stepFrom);
			SetCtrlVal (panel, DAC_MASTER_NUMERIC_stepTo,   wfmP->stepTo);
			SetCtrlVal (panel, DAC_MASTER_NUMERIC_stepRep,  wfmP->stepRep);
			break;
		case WFM_TYPE_POINTS:
			plotWaveform (panel, DAC_MASTER_GRAPH_WFM, seq, wfmP, NULL);
			break;
		case WFM_TYPE_FILE:
			plotWaveform (panel, DAC_MASTER_GRAPH_WFM, seq, wfmP, NULL);
			break;
		case WFM_TYPE_CONSTVOLTAGE:
			SetCtrlVal (panel, DAC_MASTER_NUMERIC_constant, b->constVoltages[DACNo]);
			break;
	}	

    SetCtrlAttribute (panel, DAC_MASTER_GRAPH_WFM, ATTR_VISIBLE, 
                      (wfmType == WFM_TYPE_POINTS) || (wfmType == WFM_TYPE_FILE));
}




//=======================================================================
//
//    display waveform names in all rings
//
//=======================================================================
void ANALOGCHANNELS_fillWaveformNamesToAllRings (t_sequence *seq)

{
    int i, n;
    t_waveform *wfm;
    
	// -----------------------------------------------------------
    //      clear list + insert default items
	// -----------------------------------------------------------
    for (n=0; n < N_AO_CHANNELS; n++) {
    	if (isAnalogChannelActive (seq, n)) {
	        ClearListCtrl (panelDACx[n], DAC_MASTER_RING_WFMs);
		    InsertListItem (panelDACx[n], DAC_MASTER_RING_WFMs, -1, strUnchanged, WFM_ID_UNCHANGED);
		    InsertListItem (panelDACx[n], DAC_MASTER_RING_WFMs, -1, strConstVoltage,  WFM_ID_CONSTVOLTAGE);
		    InsertListItem (panelDACx[n], DAC_MASTER_RING_WFMs, -1, strHold,      WFM_ID_HOLD);
		}
    }
	// -----------------------------------------------------------
    //      insert items
	// -----------------------------------------------------------
	for (i=1; i <= ListNumItems (seq->lWaveforms); i++) 
	{
        for (n=0; n < N_AO_CHANNELS; n++) {
            wfm = WFM_ptr(seq, i);
            if (isAnalogChannelActive (seq, n) && !wfm->hide) 
            	InsertListItem (panelDACx[n], DAC_MASTER_RING_WFMs, -1, wfm->name, i);
        }
    }

/*	
	// -----------------------------------------------------------
    //      display waveforms
    // -----------------------------------------------------------
	if (ListNumItems (seq->lAnalogBlocks) != 0) {
    	GetCtrlVal (panelAnalogChannels, ANALOG_LISTBOX_AnalogBlocks, &nr);
    	a = ANALOGBLOCK_ptr(seq, nr);
   		if (a != NULL) {
	    	for (n = 0; n < N_AO_CHANNELS; n ++) {
	    		if (isAnalogChannelActive(n)) 
	    		   displayWaveformName (panelDACx[n], DAC_MASTER_RING_WFMs, a->waveforms[n]);
	    	}
    	}
   	}

*/
}



//=======================================================================
//
//    display analog block
//
//=======================================================================



int ANALOGCHANNELS_displayValues (t_sequence *seq, t_digitalBlock *b)
{
    int i;
	
    for (i=0; i < N_AO_CHANNELS; i++) {
		if (isAnalogChannelActive(seq, i)) {
			if (seq->AO_voltageSweepsOnly && (i < N_DAC_CHANNELS)) {
				displayWaveform (seq, panelDACx[i], b, seq->voltageSweepWaveforms[i], i);
			}
			else {
				if (b->waveforms[i] == NULL) {
					displayWaveform (seq, panelDACx[i], b, WFM_ID_UNCHANGED, i);
				}
				else {
					displayWaveform (seq, panelDACx[i], b, b->waveforms[i], i);
				}
			}
		}	   
	}

   
    return 0;
}



/*
int CVICALLBACK PanelAnalogBlockClicked (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			printf ("!AB!");
			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		}
	return 0;
}
*/





/************************************************************************/
/************************************************************************/
/*
/*    Callback functions
/*
/************************************************************************/
/************************************************************************/


int CVICALLBACK ANALOGCHANNELS_WaveformChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int wfmNr;
    int ch;
    
    switch (event)
        {
        case EVENT_COMMIT:
            GetCtrlVal (panel, control, &wfmNr);
			ch = nrPanel(panel);
	        activeDigitalBlock->waveforms[ch] = wfmNr;
            displayWaveform (activeSeq(), panel, activeDigitalBlock, wfmNr, ch);
            setChanges (activeSeq(),1);
            break;
        }
    return 0;
}




//=======================================================================
//
//    name of DAC channel changed
//
//=======================================================================
int CVICALLBACK ANALOGCHANNELS_DACNameEdited (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    char help[100];
    int nr;
    
    switch (event)
        {
        case EVENT_COMMIT:
            GetCtrlVal (panel, control, help);
            nr = nrPanel(panel);
		    activeSeq()->AO_channelNames[nr] =
		        strnewcopy (activeSeq()->AO_channelNames[nr], help);
            setChanges (activeSeq(),1);
            break;
        case EVENT_LEFT_DOUBLE_CLICK:
            break;
        }
    return 0;
}




int CVICALLBACK ANALOGCHANNELS_WaveformConstVoltageChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    int ch;
	
  	switch (event)
		{
		case EVENT_COMMIT:
			ch = nrPanel (panel);
            GetCtrlVal (panel, control, &activeDigitalBlock->constVoltages[ch]);
			break;
		case EVENT_KEYPRESS:
			if (eventData1 == ',') FakeKeystroke ('.');
			break;
		}

	return 0;
}


int CVICALLBACK ANALOGCHANNELS_stepParametersChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int ch;
	t_sequence *seq;
	t_waveform *wfm;

	switch (event) {
		case EVENT_COMMIT:
			ch = nrPanel (panel);
			seq = activeSeq();
			if (seq->AO_voltageSweepsOnly && (ch < N_DAC_CHANNELS)) {
				wfm = WFM_ptr (seq, seq->voltageSweepWaveforms[ch]);
				if (wfm == NULL) Breakpoint();
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepFrom, &wfm->stepFrom);
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepTo  , &wfm->stepTo);
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepRep , &wfm->stepRep);
	            setChanges (seq, 1);
			}
			else {
				// change values in Waveform
				ch = nrPanel (panel);
				seq = activeSeq();
				wfm = WFM_ptr (seq, activeDigitalBlock->waveforms[ch]);
				if (wfm == NULL) Breakpoint();
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepFrom, &wfm->stepFrom);
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepTo  , &wfm->stepTo);
				GetCtrlVal (panelDACx[ch], DAC_MASTER_NUMERIC_stepRep , &wfm->stepRep);
            	setChanges (seq, 1);
				ANALOGCHANNELS_displayValues (seq, activeDigitalBlock);
			}
			break;
	}
	return 0;
}



void ANALOGCHANNELS_selectWaveforms (t_digitalBlock *b, t_sequence *seq)
{
	int i;

	if (b == NULL) return;
	insertBlockNumbersAndReferences (seq);
	
	ANALOGCHANNELS_initPanel (seq, b->blockNr);

	SEQUENCE_createVoltageSweepWaveforms (seq);    
    ANALOGCHANNELS_fillWaveformNamesToAllRings (seq);
    
    for (i=0; i < N_AO_CHANNELS; i++) {
		if (isAnalogChannelActive(seq, i)) {
		   SetCtrlAttribute (panelDACx[i], DAC_MASTER_RING_WFMs, 
		   					 ATTR_VISIBLE, 
		   					 !seq->AO_voltageSweepsOnly || (i >= N_DAC_CHANNELS));
		//		   SetCtrlAttribute (panelDACx[i], DAC_MASTER_RING_WFMs, 
		//		   					 ATTR_DIMMED, ListNumItems (seq->l) == 0);
		   if (seq->AO_channelNames[i] != NULL)
		       SetCtrlVal (panelDACx[i], DAC_MASTER_DAC_Name, seq->AO_channelNames[i]);
		   else
		       SetCtrlVal (panelDACx[i], DAC_MASTER_DAC_Name, "");
		}
	}
	
	activeDigitalBlock = b;
    ANALOGCHANNELS_displayValues (seq, b);
	InstallPopup (panelAnalogChannels );
}



int CVICALLBACK ANALOGCHANNELS_DONE (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
//			DIGITALBLOCKS_setColors (activeSeq(), activeDigitalBlock);
			activeDigitalBlock = NULL;
			break;
	}
	return 0;
}
