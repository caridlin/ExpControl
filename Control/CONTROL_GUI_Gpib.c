#include "easytab.h"
#include <utility.h>
#include <userint.h>
#include "UIR_ExperimentControl.h"

/*============================================================================
/*
/*                   E x p e r i m e n t   C o n t r o l  
/*
/*----------------------------------------------------------------------------
/*
/*  Copyright (c) Stefan Kuhr, 1999 - 2003
/*					
/*----------------------------------------------------------------------------
/*                                                                            
*                                                                            
/*============================================================================*/
#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"
#include "CONTROL_GUI_ExtDev.h"

#include "CONTROL_HARDWARE_DAQmx.h"



extern int panelGpib;
extern int panelConfiguration;
extern int subPanelGpib;

int subPanelGpibWfm  = -1;
int subPanelGpibFreq = -1;
int subPanelGpibCommandString = -1;
int subPanelGpibPower = -1;
int subPanelGpibPulses = -1;
int panelNewCommand = -1;

void GPIB_commandString_setValues (t_gpibCommand *g);
void GPIB_freq_setValues (t_gpibCommand *g);
void GPIB_power_setValues (t_gpibCommand *g);
void GPIB_pulses_setValues (t_gpibCommand *g);
void GPIB_pulses_getValues(t_gpibCommand *g);
void GPIB_wfm_setValues (t_sequence *seq, t_gpibCommand *g);


enum {
	GPIB_TAB_INDEX_wfm,
	GPIB_TAB_INDEX_freq,
	GPIB_TAB_INDEX_power,
	GPIB_TAB_INDEX_pulses,
	GPIB_TAB_INDEX_commandStrings
};

//=======================================================================
//
//    returns the active gpib command
//
//=======================================================================
int activeGpibCommandNo(void)
{
	int nr;
	int nItems;
	
	GetNumListItems (panelGpib, GPIB_LISTBOX_GpibCommands, &nItems);
	if (nItems <= 0) return -1;
	GetCtrlVal (panelGpib, GPIB_LISTBOX_GpibCommands, &nr);
	return nr;
}

t_gpibCommand *activeGpibCommand () 
{
    t_gpibCommand *g;
    
    g = GPIBCOMMAND_ptr (activeSeq(), activeGpibCommandNo());
    if (g != NULL) g->device = GPIBDEVICE_ptr (g->gpibAddress);
    
    return g;
}


//=======================================================================
//
//     determine minimum height of panel 
//
//=======================================================================
int GPIB_panelMinimumHeight(void)
{
	return ctrlBottom (panelGpib, GPIB_TAB)+10;
}



//=======================================================================
//
//    resize panel "GPIB commands"
//
//=======================================================================
void GPIB_resizePanel (void)
{
	SetCtrlAttribute (panelGpib, GPIB_LISTBOX_GpibCommands,
					  ATTR_HEIGHT,
					  ctrlBottom (panelGpib, GPIB_TAB) 
					  - ctrlTop (panelGpib, GPIB_LISTBOX_GpibCommands)
					  );
}
//=======================================================================
//
//    display GpibCommand in panel
//
//=======================================================================


void GPIB_setDeviceLimits (t_gpibCommand *g)
{
	t_gpibDevice *dev;
	
	dev = GPIBDEVICE_ptr(g->gpibAddress);
	if (dev != NULL) {
/*		SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_nPoints,
						  ATTR_MIN_VALUE, dev->wfmMinPoints);
		SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_nPoints,
						  ATTR_MAX_VALUE, dev->wfmMaxPoints);
//		SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_duration,
						  //ATTR_MIN_VALUE, dev->minDuration_us);
//		SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_duration,
//						  ATTR_MAX_VALUE, dev->maxDuration_us);
*/	}
}

		   

void GPIB_showCommand (t_sequence *seq, t_gpibCommand *g)
{
    t_ext_device *sExtDev;
	if (g == NULL) return;

    if (g->name== NULL) 
    	SetCtrlAttribute (panelGpib, GPIB_STRING_Name, ATTR_CTRL_VAL, "");
    else
    	SetCtrlAttribute (panelGpib, GPIB_STRING_Name, ATTR_CTRL_VAL, g->name);
	GPIB_fillDevicesToList (panelGpib, GPIB_RING_device, g->commandType);
	EXTDEV_fillDevicesToList (panelGpib,GPIB_RING_ExtDevice, EXTDEV_TYPE_TELNET); //ToDo Sebastian Change Type
    SetCtrlVal (panelGpib, GPIB_RING_device, g->gpibAddress);
	sExtDev=EXTDEV_ptrByID(seq,g->extDeviceID);

	SetCtrlVal (panelGpib, GPIB_RING_ExtDevice, sExtDev->positionInList+1);
	SetCtrlVal (panelGpib, GPIB_CHECKBOX_GPIB,!g->noGPIBdevice);
	SetCtrlAttribute(panelGpib,GPIB_RING_device,ATTR_DIMMED,g->noGPIBdevice);
	SetCtrlAttribute(panelGpib,GPIB_RING_ExtDevice,ATTR_DIMMED,!g->noGPIBdevice);
    SetCtrlVal (panelGpib, GPIB_BTN_transBeforeEvery, g->transmitBeforeEveryRepetition);
    SetCtrlVal (panelGpib, GPIB_BTN_transBeforeFirst, g->transmitBeforeFirstRepetition);
    SetCtrlVal (panelGpib, GPIB_BTN_transAfterLast,   g->transmitAfterLastRepetition);
    SetCtrlVal (panelGpib, GPIB_BTN_transAfterEvery, g->transmitAfterEveryRepetition);
	SetCtrlVal (panelGpib, GPIB_BTN_transAfterStart, g->transmitAfterStartOfSequence);
	SetCtrlVal (panelGpib, GPIB_NUMERIC_maxDuration, g->maximumDuration_ms);

	switch (g->commandType) {
		// -----------------------------------------
		//    GPIB_DEVICETYPE_ARBWFM (arbitrary waveform)
		// -----------------------------------------
        case GPIB_COMMANDTYPE_ARBWFM:
			GPIB_wfm_setValues (seq, g);
            break;
		// -----------------------------------------
		//    GPIB_DEVICETYPE_FREQ (frequency syntheziser)
		// -----------------------------------------
        case GPIB_COMMANDTYPE_FREQ: 
			GPIB_freq_setValues (g);
	     	break;
			
		case GPIB_COMMANDTYPE_STRINGS: 
			GPIB_commandString_setValues (g);
			break;
		case GPIB_COMMANDTYPE_POWERSUPPLY:
			GPIB_power_setValues (g);
			break;
		case GPIB_COMMANDTYPE_PULSES:
			GPIB_pulses_setValues (g);    
			break;
   	}
   	GPIB_changeVisibility (g);
   	GPIB_setDeviceLimits (g);
	if (g->noGPIBdevice){
		sExtDev=EXTDEV_ptrByID(seq,g->extDeviceID);
		if (sExtDev==seq->noExternalDevice){
			SetCtrlVal (panelGpib, GPIB_RING_ExtDevice, 0);
		}
		else{
			SetCtrlVal (panelGpib, GPIB_RING_ExtDevice, sExtDev->positionInList+1);
		}
		SetCtrlAttribute (panelGpib, GPIB_RING_device, ATTR_DIMMED,1);
		SetCtrlAttribute (panelGpib, GPIB_RING_ExtDevice, ATTR_DIMMED,0);
	}
	else {
		SetCtrlVal (panelGpib, GPIB_RING_ExtDevice, 0);
		SetCtrlAttribute (panelGpib, GPIB_RING_device, ATTR_DIMMED,0);
		SetCtrlAttribute (panelGpib, GPIB_RING_ExtDevice, ATTR_DIMMED,1);
	}
}


//=======================================================================
//
//    changes the visibility of controls 
//    depending on the waveform type
//
//=======================================================================
void GPIB_changeVisibility (t_gpibCommand *g)
{
    int activeTab;
    int commandType;
    
    if (g == NULL) commandType = GPIB_COMMANDTYPE_NONE;
    else commandType = g->commandType;
    
	SetTabPageAttribute (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_wfm,
						 ATTR_DIMMED,
						 commandType != GPIB_COMMANDTYPE_ARBWFM);
	SetTabPageAttribute (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_freq,
						 ATTR_DIMMED, 
						 commandType != GPIB_COMMANDTYPE_FREQ);
	SetTabPageAttribute (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_power,
						 ATTR_DIMMED, 
						 commandType != GPIB_COMMANDTYPE_POWERSUPPLY);
	SetTabPageAttribute (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_commandStrings ,
						 ATTR_DIMMED, 
						 commandType != GPIB_COMMANDTYPE_STRINGS);
	SetTabPageAttribute (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_pulses,
						 ATTR_DIMMED, 
						 commandType != GPIB_COMMANDTYPE_PULSES);
	switch (commandType) {
	    case GPIB_COMMANDTYPE_FREQ:    activeTab = GPIB_TAB_INDEX_freq;   break;
	    case GPIB_COMMANDTYPE_STRINGS: activeTab = GPIB_TAB_INDEX_commandStrings;   break;
	    case GPIB_COMMANDTYPE_ARBWFM:  activeTab = GPIB_TAB_INDEX_wfm;   break;
   	    case GPIB_COMMANDTYPE_POWERSUPPLY: activeTab = GPIB_TAB_INDEX_power;   break;
		case GPIB_COMMANDTYPE_PULSES: activeTab = GPIB_TAB_INDEX_pulses;   break;   
		default:
			activeTab = GPIB_TAB_INDEX_wfm;
	}

	SetActiveTabPage (panelGpib, GPIB_TAB, activeTab);
	// change visibility if no waveform is active
	SetCtrlAttribute (panelGpib, GPIB_TAB, ATTR_DIMMED, g == NULL);

    // -----------------------------------------
	//   dimm controls if no waveform present
	// -----------------------------------------
    SetAttributeForCtrls (panelGpib, ATTR_DIMMED, g == NULL, 0, 
    			        GPIB_LISTBOX_GpibCommands,
						GPIB_BTN_DeleteGPIBCommand,
						GPIB_STRING_Name,
						GPIB_RING_device,
						GPIB_BTN_transBeforeFirst,
						GPIB_BTN_transBeforeEvery,
						GPIB_BTN_transAfterEvery,
						GPIB_BTN_transAfterLast,
						GPIB_TEXTMSG_transmit2,
						0);

}


//=======================================================================
//
//    init panel "GPIB commands"
//
//=======================================================================
void GPIB_initPanel (void)
{
	// -----------------------------------------
    // load subpanels for different device types
	// -----------------------------------------
	
	
	GetPanelHandleFromTabPage (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_wfm,
							   &subPanelGpibWfm);
	GetPanelHandleFromTabPage (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_freq,
							   &subPanelGpibFreq);
	GetPanelHandleFromTabPage (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_power,
							   &subPanelGpibPower);
	GetPanelHandleFromTabPage (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_commandStrings,
							   &subPanelGpibCommandString);
	GetPanelHandleFromTabPage (panelGpib, GPIB_TAB, GPIB_TAB_INDEX_pulses,
							   &subPanelGpibPulses);
	
	// -----------------------------------------
    //   init popup panel 
	// -----------------------------------------
	panelNewCommand = LoadPanel (0, UIR_File, NEW_GPIB);
	ClearListCtrl (panelNewCommand, NEW_GPIB_RING_commandType);
	InsertListItem (panelNewCommand, NEW_GPIB_RING_commandType, -1, 
					"Arbitrary waveform", GPIB_COMMANDTYPE_ARBWFM);
	InsertListItem (panelNewCommand, NEW_GPIB_RING_commandType, -1, 
					"Step frequency", GPIB_COMMANDTYPE_FREQ);
	InsertListItem (panelNewCommand, NEW_GPIB_RING_commandType, -1, 
					"Command strings", GPIB_COMMANDTYPE_STRINGS);
	InsertListItem (panelNewCommand, NEW_GPIB_RING_commandType, -1, 
					"Power supply", GPIB_COMMANDTYPE_POWERSUPPLY);
	InsertListItem (panelNewCommand, NEW_GPIB_RING_commandType, -1, 
					"Pulses", GPIB_COMMANDTYPE_PULSES);
	// -----------------------------------------
	//    configure table and graph
	// -----------------------------------------
	SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_GRAPH_WFM,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
	// -----------------------------------------
	//    dimm non-used items
	// -----------------------------------------
    GPIB_changeVisibility (NULL);
    

}



const char *GPIB_commandTypeStr (int type)
{
	switch (type) {
		case GPIB_COMMANDTYPE_STRINGS:
			return "command strings";
		case GPIB_COMMANDTYPE_ARBWFM:
			return "arbitrary waveform";
		case GPIB_COMMANDTYPE_FREQ:
			return "frequency generation";
		case GPIB_COMMANDTYPE_POWERSUPPLY:
			return "power supply";
		case GPIB_COMMANDTYPE_PULSES:
			return "pulses";
		default:
			return "UNKNOWN";
	}
}

//=======================================================================
//
//    display waveform names in all rings
//
//=======================================================================
void GPIB_fillWaveformNamesToAllRings (t_sequence *seq)
{
    int i;
	t_waveform *wfm;
	
	// -----------------------------------------------------------
    //      clear list + insert items
	// -----------------------------------------------------------
 	ClearListCtrl (subPanelGpibWfm, GPIBwfm_RING_WFM);
	InsertListItem (subPanelGpibWfm, GPIBwfm_RING_WFM, -1, "NONE", WFM_ID_UNCHANGED);
	for (i=1; i <= ListNumItems (seq->lWaveforms); i++) 
	{
        ListGetItem (seq->lWaveforms, &wfm, i);
		if (wfm->channel == WFM_CHANNEL_GPIB) {
			InsertListItem (subPanelGpibWfm, GPIBwfm_RING_WFM, -1, wfm->name, i);
		}
    }
    if (activeGpibCommand() != NULL)
		 WFM_displayName (subPanelGpibWfm, GPIBwfm_RING_WFM, activeGpibCommand()->waveform);
}


//=======================================================================
//
//    fill waveform names to listbox
//
//=======================================================================
char *GPIB_createListNameEntry (t_gpibCommand *g)
{
	static char help[300];

	int bgcolor = VAL_WHITE;
	int fgcolor = VAL_BLACK;
	 
	if ((g->transmitBeforeFirstRepetition) 
		|| (g->transmitBeforeEveryRepetition)
		|| (g->transmitAfterEveryRepetition)
		|| (g->transmitAfterLastRepetition))
		bgcolor = MakeColor (100, 255, 130);
		
	help[0] = 0;
	if (g->name != NULL) {
		appendColorSpec (help, bgcolor, fgcolor);
		strcat (help, g->name);
		strcat (help, "                                                                  ");
//			AppendColorSpec (help, bgcolor, fgcolor);
	}
	
	return help;

}


void GPIB_fillCommandNamesToListbox  (t_sequence *seq, int panel, int control, int activeValue, int simpleMode)
{
    int i;
	t_gpibCommand *g;
   
    ClearListCtrl (panel, control);
    for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) {
		ListGetItem (seq->lGpibCommands, &g, i);
		if (g->commandType == GPIB_LOOP_TRIGGER || g->commandType == GPIB_RECV_LOOP_TRIGGER) {
			continue;	
		}
		if (simpleMode) {
			InsertListItem (panel, control, -1, g->name, i);
		}
		else {
			InsertListItem (panel, control, -1, GPIB_createListNameEntry (g), i);
		}
    }
    
	if (!simpleMode) {
	    if ((activeValue < 0) || (activeValue > (int)ListNumItems (seq->lGpibCommands)))
	    	activeValue = 0;
		SetCtrlVal (panel, control, activeValue);
	}
}











//=======================================================================
//
//    display values of gpib commands
//
//=======================================================================



void GPIB_displayValues (t_sequence *seq, int nr)
{
	t_gpibCommand *g;
	int type;
	
	if (seq == NULL) return;
	
	GPIB_fillWaveformNamesToAllRings (seq);
	GPIB_fillCommandNamesToListbox (seq, panelGpib, GPIB_LISTBOX_GpibCommands, nr, 0);
	g = activeGpibCommand ();
	if (g == 0) type = -1;
	else type = g->commandType;
	GPIB_fillDevicesToList (panelGpib, GPIB_RING_device, type);
	InsertListItem (panelGpib, GPIB_RING_device, -1, "NONE", -1);
	GPIB_showCommand (seq, g);
}





int CVICALLBACK GPIB_BTN_Reset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int address = 0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (ConfirmPopup ("Reset GPIB device",
							  "Do you really want to reset the device?")) {
				GetCtrlVal (panel, GPIB_RING_device, &address);
				if (address > 0) GPIB_sendReset (address);
			}
			break;
		}
	return 0;
}





//=======================================================================
//
//    parameters of GPIB-command changed (general parameters)
//
//=======================================================================
int CVICALLBACK GPIB_AttributeChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int help;
    t_gpibCommand *g;
	t_ext_device *oldExtDev,*newExtDev;
	
	int GPIBDevSelected;

    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
    if (event == EVENT_COMMIT) {
	    if (ListNumItems (activeSeq()->lGpibCommands) == 0) return 0;
		g = GPIBCOMMAND_ptr (activeSeq(), activeGpibCommandNo());
        switch (control) {
            case GPIB_RING_device:
	            help = g->gpibAddress;
	            GetCtrlVal (panel, control, &g->gpibAddress);
	            if (g->gpibAddress < 1) g->gpibAddress = help;
				if (help != g->gpibAddress) setChanges (activeSeq(), 1);
				g->device = GPIBDEVICE_ptr (g->gpibAddress);
				GPIB_showCommand (activeSeq(), g);
				ProcessDrawEvents ();
				break;
			case GPIB_RING_ExtDevice:
				oldExtDev=EXTDEV_ptrByID(activeSeq(),g->extDeviceID);
				GetCtrlVal (panel, control, &help);
				newExtDev=EXTDEV_ptr(activeSeq(),help);
				g->extDeviceID =newExtDev->uniqueID;
				ProcessDrawEvents ();
				break;
			case GPIB_CHECKBOX_GPIB:
				GetCtrlVal(panel,control,&GPIBDevSelected);
				if (GPIBDevSelected) {
					tprintf("GPIB devices selected");
					SetCtrlAttribute(panel,GPIB_RING_device,ATTR_DIMMED,0);
					SetCtrlAttribute(panel,GPIB_RING_ExtDevice,ATTR_DIMMED,1);
					g->gpibAddress = 1;
					g->noGPIBdevice=0;
				}
				else {
					SetCtrlAttribute(panel,GPIB_RING_device,ATTR_DIMMED,1);
					SetCtrlAttribute(panel,GPIB_RING_ExtDevice,ATTR_DIMMED,0);
					g->gpibAddress = 0;
					g->noGPIBdevice=1; 
				}
				g->device = GPIBDEVICE_ptr (g->gpibAddress);
				GPIB_showCommand (activeSeq(), g);
				ProcessDrawEvents ();
				break;
			case GPIB_BTN_transBeforeFirst:
	            GetCtrlVal (panel, control, &g->transmitBeforeFirstRepetition);
	            ReplaceListItem (panel, GPIB_LISTBOX_GpibCommands, 
	            	activeGpibCommandNo()-1, GPIB_createListNameEntry(g), activeGpibCommandNo());
	            setChanges (activeSeq(), 1); 
                break;
			case GPIB_BTN_transBeforeEvery:
	            GetCtrlVal (panel, control, &g->transmitBeforeEveryRepetition);
				if (g->transmitBeforeEveryRepetition) g->transmitBeforeFirstRepetition = 0;
	            ReplaceListItem (panel, GPIB_LISTBOX_GpibCommands, 
	            	activeGpibCommandNo()-1, GPIB_createListNameEntry(g), activeGpibCommandNo());
	            setChanges (activeSeq(), 1); 
                break;
			case GPIB_BTN_transAfterEvery:
	            GetCtrlVal (panel, control, &g->transmitAfterEveryRepetition);
				if (g->transmitAfterEveryRepetition) g->transmitAfterLastRepetition = 0;
	            ReplaceListItem (panel, GPIB_LISTBOX_GpibCommands, 
	            	activeGpibCommandNo()-1, GPIB_createListNameEntry(g), activeGpibCommandNo());
	            setChanges (activeSeq(), 1); 
                break;
			case GPIB_BTN_transAfterLast:
	            GetCtrlVal (panel, control, &g->transmitAfterLastRepetition);
	            ReplaceListItem (panel, GPIB_LISTBOX_GpibCommands, 
	            	activeGpibCommandNo()-1, GPIB_createListNameEntry(g), activeGpibCommandNo());
	            setChanges (activeSeq(), 1); 
                break;
			case GPIB_BTN_transAfterStart:
	            GetCtrlVal (panel, control, &g->transmitAfterStartOfSequence);
	            ReplaceListItem (panel, GPIB_LISTBOX_GpibCommands, 
	            	activeGpibCommandNo()-1, GPIB_createListNameEntry(g), activeGpibCommandNo());
	            setChanges (activeSeq(), 1); 
				break;
			case GPIB_NUMERIC_maxDuration:
				GetCtrlVal (panelGpib, GPIB_NUMERIC_maxDuration, &g->maximumDuration_ms);
	            setChanges (activeSeq(), 1); 
				break;
				
        }
		
    }
    return 0;
}


  //=======================================================================
//
//    parameters of GPIB-command changed (pulse generator)
//
//=======================================================================
int CVICALLBACK GPIBPulse_attributeChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    t_gpibCommand *g;
    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
    if (event == EVENT_COMMIT) {
	    if (ListNumItems (activeSeq()->lGpibCommands) == 0) return 0;
		g = GPIBCOMMAND_ptr (activeSeq(), activeGpibCommandNo());
		switch (control) {    
			case GPIBpulses_RING_PULSES:
				GPIB_showCommand (activeSeq(), g);
				break;
			case GPIBpulses_NUMERIC_DELAY_START:
			case GPIBpulses_NUMERIC_DELAY_STOP:
			case GPIBpulses_NUMERIC_LENGTH_START:  
			case GPIBpulses_NUMERIC_LENGTH_STOP:    
			case GPIBpulses_CHECKBOX_DELAY_ENABLE:
			case GPIBpulses_CHECKBOX_LENGT_ENABLE:
				
				GPIB_pulses_getValues(g);
				setChanges (activeSeq(), 1);
				GPIB_showCommand (activeSeq(), g);
				break;
		}
		
    }
    return 0;
}

//=======================================================================
//
//    parameters of GPIB-command changed (Arbitrary waveforms)
//
//=======================================================================
int CVICALLBACK GPIBWfm_attributeChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
	#define MAX_TIMEDIV_ns 100
    t_gpibCommand *g;

    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
    if (event == EVENT_COMMIT) {
	    if (ListNumItems (activeSeq()->lGpibCommands) == 0) return 0;
		g = GPIBCOMMAND_ptr (activeSeq(), activeGpibCommandNo());
        switch (control) {
            case GPIBwfm_NUMERIC_channel:
	            GetCtrlVal (panel, control, &g->channel);
	            g->channel--;
				break;
            case GPIBwfm_RING_WFM:
	            GetCtrlVal (panel, control, &g->waveform);
				break;
            case GPIBwfm_CHECKBOX_alwaysMaxPts:
	            GetCtrlVal (subPanelGpibWfm, GPIBwfm_CHECKBOX_alwaysMaxPts, &g->wfmAlwaysMaxPts);
				break;
			case GPIBwfm_NUMERIC_nPoints:
				GetCtrlVal (panel, control, &g->nPoints);
				break;
			case GPIBwfm_CHECKBOX_askForErrors:
				GetCtrlVal (panel, control, &g->askDeviceForErrors);
				break;
			case GPIBwfm_RING_transferMode:
				GetCtrlVal (panel, control, &g->wfmTransferMode);
				break;
	    }
		setChanges (activeSeq(), 1);
		GPIB_showCommand (activeSeq(), g);
    }
    return 0;
}


//=======================================================================
//
//    edit gpib command
//
//=======================================================================
void GPIB_editCommand (t_sequence *seq, int nr) 
{
	// -----------------------------------------
	//    display all names
	// -----------------------------------------
    GPIB_fillCommandNamesToListbox (seq, panelGpib, GPIB_LISTBOX_GpibCommands, nr, 0);
//    fillStepWaveformNamesToListbox (ctrl, panelWaveforms(), WFM_RING_add);
	// -----------------------------------------
	// 	 show actual waveform
	// -----------------------------------------
	GPIB_changeVisibility (activeGpibCommand());
	GPIB_showCommand (seq, activeGpibCommand());
	
}





//=======================================================================
//
//    name of gpib Command changed 
//
//=======================================================================
int CVICALLBACK GPIB_NameChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int nr;
    t_gpibCommand *g;

	// -----------------------------------------
	//   get current wavefrom
	// -----------------------------------------
    if (event == EVENT_COMMIT) {
    	if (ListNumItems (activeSeq()->lGpibCommands) == 0) return 0;
	    nr = activeGpibCommandNo ();
	    g = GPIBCOMMAND_ptr (activeSeq(), nr);
		// -----------------------------------------
		//   get name
		// -----------------------------------------
        GetCtrlVal (panelGpib, GPIB_STRING_Name, g->name);
		// -----------------------------------------
		//   update all displays
		// -----------------------------------------
        GPIB_fillCommandNamesToListbox (activeSeq(), panelGpib, GPIB_LISTBOX_GpibCommands, nr, 0);
		setChanges (activeSeq(), 1);
    }
    return 0;
}



//=======================================================================
//
//    new command
//
//=======================================================================
int GPIB_getFirstDevice (int type) 
{
	int i;
	t_gpibDevice *g;
	
	for (i=1; i < config->nGpibDevices; i++) 
	{
        g = GPIBDEVICE_ptr(i);
        if (g != NULL)  {
        	if (g->commandType1 == type) return i;
        	if (g->commandType2 == type) return i;
        }
    }
    return 1;
}


int CVICALLBACK GPIB_newCommand_Done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    t_gpibCommand *new;
    int nr;
    int type;

	switch (event)
		{
		case EVENT_COMMIT:
			// -----------------------------------------
	        //    create & init new waveform
			// -----------------------------------------
		    new = GPIBCOMMAND_new (activeSeq());
			GetCtrlVal (panelNewCommand, NEW_GPIB_RING_commandType, &type);
    		GPIBCOMMAND_init (new, type);
    		strcpy (new->name, "untitled");
			RemovePopup (0);
			new->gpibAddress = GPIB_getFirstDevice (type);
			// -----------------------------------------
	        //    edit waveform
			// -----------------------------------------
			nr = ListNumItems (activeSeq()->lGpibCommands);
			GPIB_editCommand (activeSeq(), nr);
			setChanges (activeSeq(), 1);
			SetActiveCtrl (panelGpib, GPIB_STRING_Name);
			break;
		}
	return 0;
}



int CVICALLBACK GPIB_newCommand (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			InstallPopup (panelNewCommand);
			break;
		}
	return 0;
}


int CVICALLBACK GPIB_newCommand_Abort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}



//=======================================================================
//
//    delete gpib command
//
//=======================================================================
void GPIB_deleteSelectedCommand (void)
{
    int nr;
    char h[200];

	if (ListNumItems (activeSeq()->lGpibCommands)  == 0) return;
	
	// -----------------------------------------
    //    delete GPIB command
    // -----------------------------------------
	sprintf (h, "Do you really want to delete GPIB command \n'%s'?", activeGpibCommand()->name);
	if (ConfirmPopup ("Delete GPIB command", h) == 0) return;
	nr =  activeGpibCommandNo();
	GPIBCOMMAND_delete (activeSeq(), nr);
	// -----------------------------------------
    //    show new GPIB command
	// -----------------------------------------
    if (nr == ListNumItems (activeSeq()->lGpibCommands)) nr --;
	GPIB_editCommand (activeSeq(), nr);
	setChanges (activeSeq(), 1);
}



int CVICALLBACK GPIB_deleteCommand_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
  	switch (event)
		{
		case EVENT_COMMIT:
		    GPIB_deleteSelectedCommand ();
		    setChanges (activeSeq(),1);
			break;
		}
	return 0;
}



//=======================================================================
//
//    listbox clicked 
//
//=======================================================================
int CVICALLBACK GPIB_listboxClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event) {
        case EVENT_VAL_CHANGED:
            GPIB_showCommand (activeSeq(), activeGpibCommand());
			SetActivePanel (panel);
            break;
        case EVENT_KEYPRESS:
        	if (eventData1 == VAL_FWD_DELETE_VKEY) {
        	    GPIB_deleteSelectedCommand ();
				SetActivePanel (panel);
        	    return 1;
        	}
        }
    return 0;
}





int CVICALLBACK GPIB_transmitCommand_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_gpibCommand *g;
	t_sequence *s;
	
	switch (event)
		{
		case EVENT_COMMIT:
			SetWaitCursor (1);
			GPIB_clearOutputCtrl ();
			s = activeSeq();
			s->repetition = 0;
			g = activeGpibCommand();
			if (g == NULL) goto ENDE;
			GPIBDEVICE_resetLastValues (GPIBDEVICE_ptr (g->gpibAddress), 1);
			g->frequencyFeedbackDataPath[0] = 0;
//			g->offsetCorrection = 1;
			s->firstRepetition = 1;
			if (GPIB_transmitCommand (s, g, 0, 1) != 0) goto ENDE;
			// wait if values have stabilized
			GPIB_transmitCommand (s, g, 1, 1);
ENDE:						
			SetWaitCursor (0);
			GPIB_showCommand (s, g);
			break;
		}
	return 0;
}




const int ctrlSend[GPIB_NCOMMANDSTRINGS] = {
	GPIBcom_CHECKBOX_sendCommand1,
	GPIBcom_CHECKBOX_sendCommand2,
	GPIBcom_CHECKBOX_sendCommand3,
	GPIBcom_CHECKBOX_sendCommand4,
};
int ctrlTextBox[GPIB_NCOMMANDSTRINGS] = {
	GPIBcom_TEXTBOX1,
	GPIBcom_TEXTBOX2,
	GPIBcom_TEXTBOX3,
	GPIBcom_TEXTBOX4
};
int ctrlReceive[GPIB_NCOMMANDSTRINGS] = {
	GPIBcom_RADIOBUTTON_reply1,
	GPIBcom_RADIOBUTTON_reply2,
	GPIBcom_RADIOBUTTON_reply3,
	GPIBcom_RADIOBUTTON_reply4,
};
int ctrlErrChk[GPIB_NCOMMANDSTRINGS] = {
	GPIBcom_RADIOBUTTON_askError1,
	GPIBcom_RADIOBUTTON_askError2,
	GPIBcom_RADIOBUTTON_askError3,
	GPIBcom_RADIOBUTTON_askError4,
};
	
	



void GPIB_commandStringGetCtrlValues (t_gpibCommand *g)
{
	int i;
	char *textBoxContent;
	int entryLength;
	
	if (g == NULL) return;
	
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		GetCtrlVal (subPanelGpibCommandString, ctrlSend[i],
			&g->commandStringSend[i]);
		
		GetCtrlAttribute (subPanelGpibCommandString, ctrlTextBox[i],
						  ATTR_STRING_TEXT_LENGTH, &entryLength);
		textBoxContent = (char *) malloc (entryLength+1);
		textBoxContent[0]=0;
	    GetCtrlVal (subPanelGpibCommandString, ctrlTextBox[i], textBoxContent);
		if ((g->commandString[i] == NULL) ||
		   (strcmp (g->commandString[i], textBoxContent) != 0))
			g->commandString[i] = strnewcopy (g->commandString[i], textBoxContent);
		GetCtrlVal (subPanelGpibCommandString, ctrlReceive[i],
			&g->commandStringReceive[i]);
		GetCtrlVal (subPanelGpibCommandString, ctrlErrChk[i],
			&g->commandStringErrorCheck[i]);
		free (textBoxContent);
	}

}



void GPIB_commandString_setValues (t_gpibCommand *g)
{
	int i;
	
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		SetCtrlVal (subPanelGpibCommandString, ctrlSend[i],
				    g->commandStringSend[i]);
		//ResetTextBox 		(panel, ctrl, "TEST\nTEST2");
		ResetTextBox (subPanelGpibCommandString, ctrlTextBox[i],
				    g->commandString[i]);
		//setCtrlStr (subPanelGpibCommandString, ctrlTextBox[i],
		//		    g->commandString[i]);
		SetCtrlVal (subPanelGpibCommandString, ctrlReceive[i],
					g->commandStringReceive[i]);
		SetCtrlVal (subPanelGpibCommandString, ctrlErrChk[i],
					g->commandStringErrorCheck[i]);
		SetCtrlAttribute (subPanelGpibCommandString, ctrlTextBox[i],
						  ATTR_DIMMED, !g->commandStringSend[i]);
		SetCtrlAttribute (subPanelGpibCommandString, ctrlReceive[i],
						  ATTR_DIMMED, !g->commandStringSend[i]);
		SetCtrlAttribute (subPanelGpibCommandString, ctrlErrChk[i],       
						  ATTR_DIMMED, !g->commandStringSend[i]);
	}

}


int CVICALLBACK GPIBcomand_attributeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			GPIB_commandStringGetCtrlValues (activeGpibCommand());
			GPIB_commandString_setValues (activeGpibCommand());
			break;
	}
	return 0;
}




void GPIB_freq_setValues (t_gpibCommand *g)
{
	double centerFreq, span;
	t_gpibDevice *dev;
	
	dev = GPIBDEVICE_ptr(g->gpibAddress);
						
	span       = (g->stopFreq - g->startFreq);
	centerFreq = g->startFreq + span / 2;

	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBTN_enableStep,
				g->enableStep);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_EXT_channel,
				(double) g->extDeviceChannel);
    // -----------------------------------------
	//   dimm controls if no waveform present
	// -----------------------------------------
    SetAttributeForCtrls (subPanelGpibFreq, ATTR_DIMMED, !g->enableStep, 0, 
						GPIBfreq_NUMERIC_startFreq,
						GPIBfreq_RING_startMultiply,
						GPIBfreq_NUMERIC_stopFreq,
						GPIBfreq_RING_stopMultiply,
						GPIBfreq_NUMERIC_span,
						GPIBfreq_RING_spanMultiply,
						GPIBfreq_NUMERIC_stepNRep,
						GPIBfreq_TEXTMSG_repetitions,
					    GPIBfreq_RINGSLIDE_changeFreq,
					    0);

	
	if (dev != NULL) {
		SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_divideFreq,
						  ATTR_MAX_VALUE, dev->maxDivide);
		SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_divideFreq,
						  ATTR_DIMMED, dev->maxDivide == 1);

		SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_outputPower,
						  ATTR_MIN_VALUE, dev->minAmplitude);
		SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_outputPower,
						  ATTR_MAX_VALUE, dev->maxAmplitude);
	}
	
	SetAttributeForCtrls (subPanelGpibFreq, ATTR_CTRL_MODE,
						  g->enterSpan ? VAL_INDICATOR : VAL_HOT, 0,
						  GPIBfreq_NUMERIC_startFreq,
						  GPIBfreq_RING_startMultiply,
						  GPIBfreq_NUMERIC_stopFreq,
						  GPIBfreq_RING_stopMultiply, 0);
	
	SetAttributeForCtrls (subPanelGpibFreq, ATTR_TEXT_BGCOLOR,
						  g->enterSpan ? VAL_OFFWHITE : VAL_WHITE, 0,
						  GPIBfreq_NUMERIC_startFreq,
						  GPIBfreq_NUMERIC_stopFreq, 0);
	

	SetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_startStop, !g->enterSpan);

	SetAttributeForCtrls (subPanelGpibFreq, ATTR_CTRL_MODE,
						  g->enterSpan ? VAL_HOT : VAL_INDICATOR, 0,
						  GPIBfreq_NUMERIC_centerFreq,
						  GPIBfreq_RING_centerMultiply,
						  GPIBfreq_NUMERIC_span,
						  GPIBfreq_RING_spanMultiply, 0);

	SetAttributeForCtrls (subPanelGpibFreq, ATTR_TEXT_BGCOLOR,
						  g->enterSpan ? VAL_WHITE : VAL_OFFWHITE, 0,
						  GPIBfreq_NUMERIC_centerFreq,
						  GPIBfreq_NUMERIC_span, 0);
	
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_centerSpan, g->enterSpan);
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_addFreqOffs, g->addFreqOffset);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_offset, ATTR_DIMMED, !g->addFreqOffset);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_TEXTMSG_MHz, ATTR_DIMMED, !g->addFreqOffset);
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_offset, g->freqOffsetMHz);
	
	
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_startFreq,
					  ATTR_PRECISION, 
					  (int) log10(g->startMultiply));
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_startFreq,   g->startFreq / g->startMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_startMultiply,  g->startMultiply);
	
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_stopFreq,
					  ATTR_PRECISION, (int) log10(g->stopMultiply));
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_stopFreq,    g->stopFreq / g->stopMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_stopMultiply,   g->stopMultiply);

	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_centerFreq,
					  ATTR_PRECISION, (int) log10(g->centerMultiply));
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_centerFreq,  centerFreq / g->centerMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_centerMultiply, g->centerMultiply);

	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_span,
					  ATTR_PRECISION, (int) log10(g->spanMultiply));
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_span,    	span / g->spanMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_spanMultiply,   g->spanMultiply);
	
//	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_nSteps,      g->nStepPoints);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_stepNRep,    g->nStepRepetition);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RINGSLIDE_changeFreq,
				g->stepTrigger);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_divideFreq, g->divideFreq);
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_fetchFreq, g->fetchFreq);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_RFon, g->rfOn);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_outputPower,
				g->outputPower);				
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_pulseMode,  g->pulseModeEnable);
//	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RINGSLIDE_logic,  g->pulseModeLogic);
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_freqDev, g->freqDeviation / g->freqDeviationMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_modDevMultiply, g->freqDeviationMultiply);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_freqMod, g->enableFM);
	
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_askForErrors, g->askDeviceForErrors);

	SetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_AM, g->enableAM);
	
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_amDepth, ATTR_DIMMED, !g->enableAM);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_TEXTMSG_pct, ATTR_DIMMED, !g->enableAM);
	SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_amDepth, g->AMdepthPercent);
	
	// feedback 1
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_feedback, ATTR_CTRL_VAL, g->enableFeedback[0]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput, ATTR_CTRL_VAL, g->feedbackAnalogInput[0]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackDev, ATTR_CTRL_VAL, g->feedbackFreqDeviation[0]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackOffs, ATTR_CTRL_VAL, g->feedbackOffset[0]);
	
	SetAttributeForCtrls (subPanelGpibFreq, ATTR_DIMMED,
						  !g->enableFeedback[0], 0,
						  GPIBfreq_STRING_feedbackInput,
						  GPIBfreq_NUMERIC_feedbackU,
						  GPIBfreq_TEXTMSG_feedback5,
						  GPIBfreq_TEXTMSG_feedback3,
						  GPIBfreq_NUMERIC_feedbackDev,
						  GPIBfreq_TEXTMSG_feedback1,
						  GPIBfreq_TEXTMSG_feedback2,
						  GPIBfreq_NUMERIC_feedbackOffs,
						  0);
	
	
	// feedback 2
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_feedback2, ATTR_CTRL_VAL, g->enableFeedback[1]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput2, ATTR_CTRL_VAL, g->feedbackAnalogInput[1]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackDev_2, ATTR_CTRL_VAL, g->feedbackFreqDeviation[1]);
	SetCtrlAttribute (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackOffs2, ATTR_CTRL_VAL, g->feedbackOffset[1]);
	
	SetAttributeForCtrls (subPanelGpibFreq, ATTR_DIMMED,
						  !g->enableFeedback[1], 0,
						  GPIBfreq_STRING_feedbackInput2,
						  GPIBfreq_NUMERIC_feedbackU_2,
						  GPIBfreq_TEXTMSG_feedback5_2,
						  GPIBfreq_TEXTMSG_feedback3_2,
						  GPIBfreq_NUMERIC_feedbackDev_2,
						  GPIBfreq_TEXTMSG_feedback1_2,
						  GPIBfreq_TEXTMSG_feedback2_2,
						  GPIBfreq_NUMERIC_feedbackOffs2,
						  0);

	
	#undef NDimmableItems
}




int CVICALLBACK GPIBfreq_attributeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {

	double freq, span;
	double channel;
	double oldMultiply;
	t_gpibCommand *g;
	t_gpibDevice *d;
	//int h;
	
	switch (event) {
		case EVENT_KEYPRESS:
			if (eventData1 == ',') FakeKeystroke ('.');
			break;
		case EVENT_COMMIT:
			g = activeGpibCommand();
			if (g == NULL) return 0;
			d = g->device;
			switch (control) {	
				case GPIBfreq_RADIOBTN_enableStep:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBTN_enableStep, &g->enableStep);
					if ((g->enableStep) && (!g->transmitAfterStartOfSequence)) {
						g->transmitBeforeEveryRepetition = 1;
						g->transmitBeforeFirstRepetition = 0;
					}
					else g->enterSpan = 1;
					GPIB_showCommand (activeSeq(), g);
					break;
				case GPIBfreq_NUMERIC_EXT_channel:
					 channel=1;
					 GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_EXT_channel, &channel);
					 g->extDeviceChannel=(unsigned int) channel;
					 GPIB_showCommand (activeSeq(), g);
					 break;
				case GPIBfreq_NUMERIC_startFreq:
				case GPIBfreq_NUMERIC_stopFreq:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_startFreq, &freq);
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_startMultiply,
								&g->startMultiply);
					g->startFreq = freq * g->startMultiply;
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_stopFreq, &freq);
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_stopMultiply,
								&g->stopMultiply);
					g->stopFreq = freq * g->stopMultiply;
//					if (g->stopFreq < g->startFreq) {
//						freq = g->startFreq;
//						g->startFreq = g->stopFreq;
//						g->stopFreq = freq;
//					}
					break;
				case GPIBfreq_NUMERIC_centerFreq:
				case GPIBfreq_NUMERIC_span:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_centerFreq, &freq);
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_centerMultiply,
								&g->centerMultiply);
					freq *= g->centerMultiply;
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_span, &span);
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_spanMultiply,
								&g->spanMultiply);
					span *= g->spanMultiply;
					g->startFreq = freq - 0.5 * span; 
					g->stopFreq  = freq + 0.5 * span;
					break;
				case GPIBfreq_RING_startMultiply:
					oldMultiply = g->startMultiply;
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_startMultiply,
								&g->startMultiply);
					g->startFreq *= g->startMultiply / oldMultiply;
					break;
				case GPIBfreq_RING_stopMultiply:
					oldMultiply = g->stopMultiply;
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_stopMultiply,
								&g->stopMultiply);
					g->stopFreq *= g->stopMultiply / oldMultiply;
					break;
				case GPIBfreq_RING_centerMultiply:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_centerMultiply,
								&g->centerMultiply);
					break;
				case GPIBfreq_RING_spanMultiply:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_spanMultiply,
								&g->spanMultiply);
					break;
				case GPIBfreq_CHECKBOX_startStop:
//					GetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_startStop,
//								&h);
					g->enterSpan = 0;
					
					break;
				case GPIBfreq_CHECKBOX_centerSpan:
					g->enterSpan = 1;
//					GetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_centerSpan,
//								&g->enterSpan);
					break;
				case GPIBfreq_CHECKBOX_fetchFreq:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_fetchFreq,
								&g->fetchFreq);
					break;	
				case GPIBfreq_CHECKBOX_addFreqOffs:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_addFreqOffs,
								&g->addFreqOffset);
					break;
				case GPIBfreq_NUMERIC_offset:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_offset,
								&g->freqOffsetMHz);
					break;
				
				case GPIBfreq_NUMERIC_stepNRep:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_stepNRep,
								&g->nStepRepetition);
					break;
				case GPIBfreq_NUMERIC_divideFreq:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_divideFreq,
								&g->divideFreq);
					break;
				case GPIBfreq_RINGSLIDE_changeFreq:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RINGSLIDE_changeFreq, &g->stepTrigger);
					if (!d->enableExternalTrigger) 
						g->stepTrigger = GPIB_COMMAND_FREQ_TRIGGER_CALC;
					break;
				case GPIBfreq_RADIOBUTTON_RFon:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_RFon,  &g->rfOn);
					break;
				case GPIBfreq_RADIOBUTTON_pulseMode:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_pulseMode,  &g->pulseModeEnable);
					break;
				case GPIBfreq_NUMERIC_outputPower:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_outputPower, &g->outputPower);
					break;
				case GPIBfreq_NUMERIC_freqDev:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_freqDev, &g->freqDeviation);
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_modDevMultiply, &g->freqDeviationMultiply);
					g->freqDeviation *= g->freqDeviationMultiply;
					break;
				case GPIBfreq_RING_modDevMultiply:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RING_modDevMultiply, &g->freqDeviationMultiply);
					break;
				case GPIBfreq_RADIOBUTTON_freqMod:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_freqMod, &g->enableFM);
					break;
				case GPIBfreq_RADIOBUTTON_AM:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_AM, &g->enableAM);
					break;
				case GPIBfreq_NUMERIC_amDepth:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_amDepth, &g->AMdepthPercent);
					break;
				case GPIBfreq_CHECKBOX_askForErrors:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_CHECKBOX_askForErrors, &g->askDeviceForErrors);
					break;
				case GPIBfreq_RADIOBUTTON_feedback:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_feedback, &g->enableFeedback[0]);
					break;
				case GPIBfreq_STRING_feedbackInput:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput, g->feedbackAnalogInput[0]);
					break;
				case GPIBfreq_NUMERIC_feedbackDev:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackDev, &g->feedbackFreqDeviation[0]);
					break;
				case GPIBfreq_NUMERIC_feedbackOffs: 	
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackOffs, &g->feedbackOffset[0]);
					break;
				case GPIBfreq_RADIOBUTTON_feedback2:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_RADIOBUTTON_feedback2, &g->enableFeedback[1]);
					break;
				case GPIBfreq_STRING_feedbackInput2:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput2, g->feedbackAnalogInput[1]);
					break;
				case GPIBfreq_NUMERIC_feedbackDev_2:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackDev_2, &g->feedbackFreqDeviation[1]);
					break;
				case GPIBfreq_NUMERIC_feedbackOffs2: 	
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackOffs2, &g->feedbackOffset[1]);
					break;
				default:
					return 0;
			}
			GPIB_freq_setValues (g);
			setChanges (activeSeq(), 1);
			break;

	}
	return 0;
}


void GPIB_power_setValues (t_gpibCommand *g)
{
	t_gpibDevice *d;
	int dimmed;
	
	if (g == NULL) return;
	d = g->device;

	// set limits
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uMin, d->minVoltage);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uMax, d->maxVoltage);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_iMax, d->maxCurrent * 1000);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_channel, g->channel);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_NUMERIC_channel, ATTR_DIMMED, d->nChannels <= 1);
	
	SetCtrlVal (subPanelGpibPower, GPIBpow_RADIOBTN_enableStepU, g->stepVoltage);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uStart, g->startVoltage);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_NUMERIC_uStop,
					  ATTR_DIMMED, !g->stepVoltage);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_TEXTMSG_V2,
					  ATTR_DIMMED, !g->stepVoltage);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uStop, g->stopVoltage);
	
	// current
	SetCtrlVal (subPanelGpibPower, GPIBpow_RADIOBTN_enableStepI, g->stepCurrent);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_iStart, g->startCurrent *1E3);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_NUMERIC_iStop,
					  ATTR_DIMMED, !g->stepCurrent);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_TEXTMSG_mA2,
					  ATTR_DIMMED, !g->stepCurrent);

	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_iStop, g->stopCurrent * 1E3);
	SetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_stepNRep, g->nStepRepetition);
	
	dimmed = (d == NULL) || (d->GPIB_waitUntilStabilized == NULL);
	SetCtrlAttribute (subPanelGpibPower, GPIBpow_CHECKBOX_waitStabiliz, ATTR_DIMMED, dimmed);
	
	SetCtrlVal (subPanelGpibPower, GPIBpow_CHECKBOX_waitStabiliz, g->waitUntilStabilized);
	
	SetCtrlVal (subPanelGpibPower, GPIBpow_CHECKBOX_askForErrors, g->askDeviceForErrors);
	
}


void GPIB_power_getValues (t_gpibCommand *g)
{
    t_gpibDevice *d;
	//int dimmed;
	
	if (g == NULL) return;
	d = g->device;
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_channel, &g->channel);
	// voltage
	GetCtrlVal (subPanelGpibPower, GPIBpow_RADIOBTN_enableStepU, &g->stepVoltage);
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uStart, &g->startVoltage);
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_uStop, &g->stopVoltage);
	// current			
	GetCtrlVal (subPanelGpibPower, GPIBpow_RADIOBTN_enableStepI, &g->stepCurrent);
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_iStart, &g->startCurrent);
	g->startCurrent /= 1E3;
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_iStop, &g->stopCurrent);
	g->stopCurrent /= 1E3;
	
	GetCtrlVal (subPanelGpibPower, GPIBpow_NUMERIC_stepNRep, &g->nStepRepetition);
	
	GetCtrlVal (subPanelGpibPower, GPIBpow_CHECKBOX_waitStabiliz, &g->waitUntilStabilized);

	GetCtrlVal (subPanelGpibPower, GPIBpow_CHECKBOX_askForErrors, &g->askDeviceForErrors);
	
	
	GPIB_power_setValues (g);
	setChanges (activeSeq(), 1);
}


void GPIB_pulses_setValues (t_gpibCommand *g)
{
	int channel;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_RING_PULSES, &channel);
	--channel;
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_DELAY_START, g->startPulseDelay[channel]*1e6);   //display in 탎
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_DELAY_STOP, g->stopPulseDelay[channel]*1e6);	   //display in 탎
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_CHECKBOX_DELAY_ENABLE, g->stepPulseDelay[channel]);	
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_LENGTH_START, g->startPulseLength[channel]*1e6);	   //display in 탎 
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_LENGTH_STOP, g->stopPulseLength[channel]*1e6);	   //display in 탎
	SetCtrlVal (subPanelGpibPulses, GPIBpulses_CHECKBOX_LENGT_ENABLE, g->stepPulseLength[channel]);	
	
}

void GPIB_pulses_getValues (t_gpibCommand *g)
{
	int channel;
	double tmp;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_RING_PULSES, &channel);  
	--channel; 
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_DELAY_START, &tmp);   //display in 탎
	g->startPulseDelay[channel] = tmp/1e6;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_DELAY_STOP, &tmp);	   //display in 탎
	g->stopPulseDelay[channel] = tmp/1e6;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_CHECKBOX_DELAY_ENABLE, &g->stepPulseDelay[channel]);
	
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_LENGTH_START, &tmp);	   //display in 탎
	g->startPulseLength[channel] = tmp/1e6;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_NUMERIC_LENGTH_STOP, &tmp);	   //display in 탎
	g->stopPulseLength[channel] = tmp/1e6;
	GetCtrlVal (subPanelGpibPulses, GPIBpulses_CHECKBOX_LENGT_ENABLE, &g->stepPulseLength[channel]);
	
}



void GPIB_wfm_setValues (t_sequence *seq, t_gpibCommand *g)
{

    t_waveform *w;
    double timeResolution_us;
    //char help[50];
    t_gpibDevice *d;

    w = WFM_ptr (seq, g->waveform);
    d = g->device;
    WFM_calculateOffsetAndDuration (w, seq->repetition, seq->nRepetitions, 0);
    if (w != NULL) 
		
	g->duration_us = w->durationThisRepetition_ns / 1E3;
    SetCtrlVal (subPanelGpibWfm, GPIBwfm_NUMERIC_duration,  g->duration_us / 1000.0);


    SetCtrlVal (subPanelGpibWfm, GPIBwfm_RING_WFM, 
    			g->waveform);
    SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_GRAPH_WFM,        ATTR_DIMMED, w == NULL);
    SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_duration, ATTR_DIMMED, w == NULL);
    SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_nPoints,  ATTR_DIMMED, w == NULL);
    SetCtrlVal       (subPanelGpibWfm, GPIBwfm_NUMERIC_channel, g->channel+1);
    

	if (g->wfmAlwaysMaxPts) {
		if (d != NULL) g->nPoints = d->wfmMaxPoints;
	}

	SetCtrlVal (subPanelGpibWfm, GPIBwfm_NUMERIC_nPoints, g->nPoints);
	
	SetCtrlVal (subPanelGpibWfm, GPIBwfm_CHECKBOX_alwaysMaxPts, g->wfmAlwaysMaxPts);
    SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_nPoints, ATTR_DIMMED, g->wfmAlwaysMaxPts);
	

	if ((w != NULL) && (g->nPoints != 0)) {
		timeResolution_us = w->durationThisRepetition_ns / (1000.0 * g->nPoints);
	}
	else 
		timeResolution_us = 0;
	
	SetCtrlAttribute (subPanelGpibWfm, GPIBwfm_NUMERIC_resolution,
					  ATTR_CTRL_MODE, VAL_INDICATOR);
	SetCtrlVal (subPanelGpibWfm, GPIBwfm_NUMERIC_resolution,  timeResolution_us);
	
	
	
	SetCtrlVal (subPanelGpibWfm, GPIBwfm_CHECKBOX_askForErrors, g->askDeviceForErrors);
	
	SetCtrlVal (subPanelGpibWfm, GPIBwfm_RING_transferMode,
				g->wfmTransferMode);
	
	
	WFM_plot (subPanelGpibWfm, GPIBwfm_GRAPH_WFM, seq, w, GPIB_digitizeParameters (g), 0);

}





int CVICALLBACK GPIBpower_attributeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_gpibCommand *g;

	switch (event) {
		case EVENT_KEYPRESS:
			if (eventData1 == ',') FakeKeystroke ('.');
			break;
		case EVENT_COMMIT:
			g = activeGpibCommand();
			if (g == NULL) return 0;
			GPIB_power_getValues (g);
			
			break;
	}
	return 0;
}




int CVICALLBACK GPIB_TabChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return 1; // omit all events
	
	/*int dimmed;
	
	switch (event)
	{
		case EVENT_ACTIVE_TAB_CHANGE: */
/* eventData1 = old active tab */
/* eventData2 = new active tab */			
	/*		GetTabPageAttribute (panel, control, eventData2, ATTR_DIMMED, &dimmed);
			tprintf("tab %d has dimmed = %d\n", eventData2, dimmed);
			if (!dimmed) { //SetActiveTabPage (panel, control, eventData1);
				return 0;
			}
	}
	tprintf("passed switch with dimmed = %d\n",dimmed);
	return 1; */
}



int CVICALLBACK GPIB_autodetect_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CONFIG_displayAllSettings (activeSeq());
			InstallPopup (panelConfiguration);
			SetActiveTabPage (panelConfiguration, CONFIG_TAB, CONFIG_TAB_INDEX_gpib);
			GPIB_autodetectAllDevices (subPanelGpib, CONFIGgpib_LISTBOX_gpibDevices);   
			break;
	}
	return 0;
}




int CVICALLBACK GPIB_freq_TestDataAcquisition_CB(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double value;
	char channel[40];
		
	switch (event)
	{
		case EVENT_COMMIT:
			switch (control) {
				case GPIBfreq_COMMANDBUTTON_test1:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput, channel);
					HARDWARE_NIUSB_TestDataAcquisition (channel, &value);
					SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackU, value);
					break;
				case GPIBfreq_COMMANDBUTTON_test2:
					GetCtrlVal (subPanelGpibFreq, GPIBfreq_STRING_feedbackInput2, channel);
					HARDWARE_NIUSB_TestDataAcquisition (channel, &value);
					SetCtrlVal (subPanelGpibFreq, GPIBfreq_NUMERIC_feedbackU_2, value);
					break;
			}
			break;
	}
	return 0;
}
