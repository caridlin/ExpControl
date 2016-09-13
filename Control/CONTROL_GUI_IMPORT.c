#include <userint.h>

/*============================================================================
/*
/*                   E x p e r i m e n t   C o n t r o l  
/*
/*----------------------------------------------------------------------------
/*
/*	Institut für Angewandte Physik, Universität Bonn
/*  Wegelerstr. 8, 53115 Bonn, Germany
/*  
/*----------------------------------------------------------------------------
/*    
/*  Copyright (c) Stefan Kuhr, 1999 - 2003
/*					
/*----------------------------------------------------------------------------
/*                                                                            
/* Title:       GUI_Import.c                                         
/* Purpose:     Import data from other sequences
/*                                                                            
/*============================================================================*/
#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"


int panelImport = -1;

t_sequence *importSequence = NULL;
int importSequenceFromFile = 0;



//=======================================================================
//
//    replace all channel names by the ones from another sequence
//
//=======================================================================
void IMPORT_importChannelNames (t_sequence *seqTo, t_sequence *seqFrom)
{
    int i;

	if (seqFrom == NULL) return;
	if (seqTo == NULL) return;
	
    for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
        seqTo->DIO_channelNames[i] = strnewcopy (seqTo->DIO_channelNames[i], seqFrom->DIO_channelNames[i]); 
		seqTo->DIO_invertDisplay[i] = seqFrom->DIO_invertDisplay[i];
	}
    for (i = 0; i < N_DAC_CHANNELS; i++)
        seqTo->AO_channelNames[i] = strnewcopy (seqTo->AO_channelNames[i], seqFrom->AO_channelNames[i]); 
	        
}


//=======================================================================
//
//    import all board parameters
//
//=======================================================================
void IMPORT_importBoardParameters (t_sequence *seqTo, t_sequence *seqFrom)
{
	if (seqFrom == NULL) return;
	if (seqTo == NULL) return;

	seqTo->DIO_timebase_50ns   = seqFrom->DIO_timebase_50ns;
	seqTo->AO_timebase_50ns    = seqFrom->AO_timebase_50ns;
}



//=======================================================================
//
//   adds "_" in front of name of waveform if the same name exists
//    
//=======================================================================
char *checkWfmName (t_sequence *seq, char *name)
{
     #define maxLen 100
     static char newName[maxLen]; 
     char help[maxLen];
     
 	if (seq == NULL) return name;
    if (WFM_nameExists (seq, name)) {
		 // copy string to the right
		 strcpy (help, "_");
		 strncat (help, name, Min(strlen(name)+1,maxLen-1));
		 strcpy (newName, help);
		 return checkWfmName (seq, newName);
     }
     else return name;
     #undef maxLen
}



/************************************************************************/
/*
/*    import of data from other sequences 
/*
/************************************************************************/

void IMPORT_importWaveforms (int panel, t_sequence *seqTo, t_sequence *seqFrom)
{
	int i, nWFMs;
	int checked;
	
						
	int *importWFM;		// saves new number of imported waveforms (in new list)
						// -1: do not import waveform
	int numberOfInitialWaveforms; // number of waveforms in current list;
	char *newName;
	int count;
//	t_analogBlock *analogP, *newAnalogB;
	t_waveform *wfmP, *newWfm;

	if (seqFrom == NULL) return;
	if (seqTo == NULL) return;

	numberOfInitialWaveforms = ListNumItems (seqTo->lWaveforms);
	
	// -----------------------------------------
	//    get number of waveforms to import
	// -----------------------------------------
	GetNumListItems (panel, IMPORT_LISTBOX_Waveforms, &nWFMs);
	importWFM = (int *) malloc (sizeof(int) * (nWFMs+1));
	// -----------------------------------------
	// 		initialize  -1: do not import
	// -----------------------------------------
	for (i = 1; i <= nWFMs; i++) importWFM[i] = -1;

	
	// -----------------------------------------
    //    check for waveforms to import
	// -----------------------------------------
	count = 1;
	for (i = 1; i <= nWFMs; i++) {
		IsListItemChecked (panel, IMPORT_LISTBOX_Waveforms, i-1, &checked);
		if (checked || (importWFM[i]!=-1)) {
		    importWFM[i] = count + numberOfInitialWaveforms;
//		    DebugPrintf ("WFM: old: #%d, new: #%d\n", i, importWFM[i]);
		    count ++;
		}
	}

//=======================================================================
//
//	duplicate waveforms
//    
//=======================================================================
	for (i = 1; i <= nWFMs; i++) {
		if (importWFM[i] > 0) {
		    wfmP  = WFM_ptr (seqFrom, i); 
		    newName = checkWfmName (seqTo, wfmP->name);
		    newWfm = WFM_new (seqTo);
		    WFM_duplicate (newWfm, wfmP);
			strcpy (newWfm->name, newName);
		}
	}
	free (importWFM);
	setChanges (seqTo,1);
	eliminateIdenticalWaveforms (seqTo);
}



void IMPORT_importGpibCommands (int panel, t_sequence *seqTo, t_sequence *seqFrom)
{
	int i;
	int checked;
	t_gpibCommand *commandSource, *commandDest;

	if (seqFrom == NULL) return;
	if (seqTo == NULL) return;

	// -----------------------------------------
    //    check for waveforms to import
	// -----------------------------------------
	for (i = 1; i <= ListNumItems (seqFrom->lGpibCommands); i++) {
		IsListItemChecked (panel, IMPORT_LISTBOX_gpibCommands, i-1, &checked);
		if (checked) {
			ListGetItem (seqFrom->lGpibCommands, &commandSource, i);
			commandDest = GPIBCOMMAND_new (seqTo); 
			GPIBCOMMAND_duplicate (commandDest, commandSource);
			if (commandSource->commandType == GPIB_COMMANDTYPE_ARBWFM) {
				if (commandSource->waveform > 0) 
					CheckListItem (panel, IMPORT_LISTBOX_Waveforms, commandSource->waveform-1, 0);
					
			}
		}
	}
	setChanges (seqTo,1);
}



void IMPORT_importCCDSettings (int panel, t_sequence *seqTo, t_sequence *seqFrom)
{
	int i;
	int checked;
	t_ccdSettings *source, *dest;
	int index;

	if (seqFrom == NULL) return;
	if (seqTo == NULL) return;

	// -----------------------------------------
    //    check for CCD settings to import
	// -----------------------------------------
	for (i = 1; i <= ListNumItems (seqFrom->lCCDSettings); i++) {
		IsListItemChecked (panel, IMPORT_LISTBOX_ccdSettings, i-1, &checked);
		if (checked) {
			ListGetItem (seqFrom->lCCDSettings, &source, i);
			index = CCDSETTINGS_getFromSerialNo (seqTo, source->serialNo);
			if ((index != 0) && (ConfirmPopupf ("Import CCD Settings", "CCD settings for camera %d already exist in current sequence.\n\nDo you want to overwrite these settings with the imported data?", source->serialNo))) {
				ListGetItem (seqTo->lCCDSettings, &dest, index);
			}
			else dest = CCDSETTINGS_new (seqTo); 
			CCDSETTINGS_duplicate (dest, source);
		}
	}
	setChanges (seqTo,1);
}







void IMPORT_dimmControls (void)
{
	int seqNr;
	char filename[MAX_PATHNAME_LEN];
	int doneDimmed;
	
	GetCtrlVal (panelImport, IMPORT_RING_sequences, &seqNr);
	GetCtrlVal (panelImport, IMPORT_STRING_filename, filename);
	doneDimmed = (seqNr == 0) && (strlen(filename) == 0);
	SetCtrlAttribute (panelImport, IMPORT_COMMANDBUTTON_Done, ATTR_DIMMED, doneDimmed); 
	
		
}



void IMPORT_initPanel (void)
{
	if (panelImport <= 0) {
		panelImport = LoadPanel (0, UIR_File, IMPORT);
/*		SetCtrlAttribute (panelImport, IMPORT_LISTBOX_Waveforms, ATTR_HEIGHT, 
						  ctrlTop (panelImport, IMPORT_CHECKBOX_ChannelNames)
					    - ctrlTop (panelImport, IMPORT_LISTBOX_Waveforms)-20);
*/	}
}



void IMPORT_freeSequence (void)
{
	if (importSequenceFromFile) {
		SEQUENCE_free (importSequence);	
		free (importSequence);
	}
	importSequence = NULL;
	importSequenceFromFile = 0;
}


void IMPORT_displayContentsOfSequence (t_sequence *seq)
{
	WFM_fillNamesToListbox (seq, panelImport, IMPORT_LISTBOX_Waveforms, 0);
	GPIB_fillCommandNamesToListbox  (seq, panelImport, IMPORT_LISTBOX_gpibCommands, 0, 1);
	CCDSETTINGS_fillNamesToListbox  (seq, panelImport, IMPORT_LISTBOX_ccdSettings);
}



int IMPORT_loadSequence (const char *filename)
{
	int err;
	
	IMPORT_freeSequence ();
	
	importSequence = SEQUENCE_new ();
	if (importSequence == NULL) {
		return -12; // out of memory
	}
    if ((err = SEQUENCE_load (filename, importSequence)) == 0) {
		SetCtrlVal (panelImport, IMPORT_STRING_filename, filename);
		IMPORT_displayContentsOfSequence (importSequence);
		importSequenceFromFile = 1;

		SetCtrlVal (panelImport, IMPORT_RING_sequences, 0);
	}
	return err;
}




void IMPORT_fillSequenceNamesToRing (int panel, int ctrl, t_sequence *excludeSeq)
{
	char help[500];
	int i;
	t_sequence *seq;
	
	ClearListCtrl (panel, ctrl);
	InsertListItem (panel, ctrl, -1, "FROM FILE", 0);
	
	for (i = 1; i <= nSequences (); i++) {
		seq = SEQUENCE_ptr(i);
		if (seq != excludeSeq) {
			sprintf (help, "%s (%s)", extractFilename(seq->filename), extractDir(seq->filename));
			InsertListItem (panel, ctrl, -1, help, i);
		}
	}
}


void IMPORT_clearAllLists (void)
{
    ClearListCtrl (panelImport, IMPORT_LISTBOX_Waveforms);
    ClearListCtrl (panelImport, IMPORT_LISTBOX_gpibCommands);
    ClearListCtrl (panelImport, IMPORT_LISTBOX_ccdSettings);
}
				

void IMPORT_showPanel (void)
{
	IMPORT_initPanel ();	
	IMPORT_freeSequence ();
	IMPORT_clearAllLists ();

	IMPORT_fillSequenceNamesToRing (panelImport, IMPORT_RING_sequences, activeSeq());
	SetCtrlVal (panelImport, IMPORT_STRING_filename, "");
	IMPORT_dimmControls ();
	InstallPopup (panelImport);
}

	
	
							  



int CVICALLBACK IMPORT_BTN_Abort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
        	RemovePopup (panelImport);
			IMPORT_freeSequence ();
			break;
		}
	return 0;
}




int CVICALLBACK IMPORT_BTN_Done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			if (checked (panelImport, IMPORT_CHECKBOX_ChannelNames))
			    IMPORT_importChannelNames (activeSeq(), importSequence);
			IMPORT_importGpibCommands (panelImport, activeSeq(), importSequence);
			IMPORT_importWaveforms (panelImport, activeSeq(), importSequence);
			IMPORT_importCCDSettings (panelImport, activeSeq(), importSequence);
			if (checked (panelImport, IMPORT_CHECKBOX_BoardParams))
			    IMPORT_importBoardParameters (activeSeq(), importSequence);
			MAIN_displayTimebases (activeSeq());    
        	RemovePopup (panelImport);
			IMPORT_freeSequence ();
	 		MAIN_displayActiveSequence ();
			break;
		}
	return 0;
}






int CVICALLBACK IMPORT_BTN_Browse (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	
	switch (event)
	{
		case EVENT_COMMIT:
		    if (FileSelectPopup (config->defaultPath, SEQUENCE_fileSuffix, SEQUENCE_fileSuffix,
								 "Import sequence data", VAL_LOAD_BUTTON, 0, 1, 1, 0,
								 filename) > 0) {
		        setDefaultDir (extractDir (filename));
				IMPORT_loadSequence (filename);
				IMPORT_dimmControls ();
			}
			break;
	}
	return 0;
}



int CVICALLBACK IMPORT_SEQUENCE_changed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int seqSelected;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, control, &seqSelected);
			if (seqSelected == 0) {
				// sequence from File
				IMPORT_freeSequence ();
				//if (importSequence == NULL) {
				IMPORT_clearAllLists ();
				//}
			}
			else {
				IMPORT_freeSequence ();
				importSequence = SEQUENCE_ptr (seqSelected);
				IMPORT_displayContentsOfSequence (importSequence);
			}
			IMPORT_dimmControls ();
			SetCtrlVal (panelImport, IMPORT_STRING_filename, "");
			break;
	}
	return 0;
}
