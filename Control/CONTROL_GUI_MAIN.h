#ifndef GUI_MAIN
#define GUI_MAIN


#ifdef SESSIONMANAGER_DATASTRUCTURE
    #define IS_ONE_PROGRAM 1
#else 
	#define IS_ONE_PROGRAM 0
#endif



enum {
	TAB0_INDEX_SEQUENCE,
	TAB0_INDEX_WFM,
	TAB0_INDEX_GPIB,
	TAB0_INDEX_CCD
};


#include "CONTROL_Datastructure.h"

int MAIN_PanelMinimumWidth  (void);

void MAIN_resizePanel (t_sequence *seq);
void MAIN2_resizePanel (t_sequence *seq);

int  MAIN_initPanel (int outPanel, int outCtrl);
int MAIN2_initPanel (int outPanel, int outCtrl);

int MAIN_quit (void);

int MAIN_loadSequence (const char *filename);

int MAIN_saveSequence (t_sequence *seq);

int MAIN_closeSequence  (int seqNr, int askSaveChanges);

void MAIN_displayTimebases (t_sequence *seq);

void MAIN_setFilenameToTitle (const char *filename, int changes);

void MAIN_setMenuBarStatus (int dimmed);

void MAIN_displayActiveSequence (void);

void MAIN_initTriggerSources (int panel, int ctrl);

int MAIN_getTriggerSource (void);

void MAIN_parseTCPString (char *buffer, unsigned long bufferSize);



__int64 OUTPUT_setElapsedTime (int reset);

__int64 OUTPUT_setTotalDuration (t_sequence *s);


void OUTPUT_setTimes (t_sequence *seq, int estimateTime, int timeFirstRepetition);

void OUTPUT_setRepetitions (t_sequence *s);

void OUTPUT_showPanel (t_sequence *s);

void OUTPUT_initPanel (void);

void OUTPUT_setMaxPointsGenerated (unsigned __int64 max64);

void OUTPUT_displayPointsGenerated (unsigned __int64 ui64Pts);

void OUTPUT_setSequenceInfo (t_sequence *s);


void DEBUGOUT_init (void);


double SEQUENCE_duration_ms (t_sequence *seq);

void SEQUENCE_initializeListBox (void);

void SEQUENCE_displayList (void);      

void SEQUENCE_appendToList (t_sequence *s, int displayList);

void SEQUENCE_setAsActive (int nr);

int nSequences (void);

t_sequence *activeSeq (void);

int getActiveSeqNr(void);


int SEQUENCE_setStatus (int newStatus);

int SEQUENCE_getStatus (void);

void SEQUENCE_closeAllDigitalBlockPanels (void);

void SEQUENCE_displayStartNr (int currentStartNr);

void quitProgram (int remoteQuit);


void setChanges (t_sequence *seq, int changes);



void CVICALLBACK MAIN_MENU_Quit(int menubar, int menuItem, void *callbackData, int panel);




void PARAMETERSWEEPS_updateDisplay (t_sequence *s);





#endif
