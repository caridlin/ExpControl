#ifndef GUI_WAVEFORM
#define GUI_WAVEFORM


#include "imageProcessing.h"
//#include "DATA_Structure.h"



t_waveform *activeWfm (void);

int activeWfmNo (void); 


void WFM_fillNamesToListbox (t_sequence *seq, int panel, int control, int color);

void WFM_fillStepNamesToListbox (t_sequence *seq, int panel, int control);

void WFM_displayName (int panel, int control, int wfmNr);


void setTableCellTime (int panel, int table, int row, int col, unsigned long time);
void setTableCellTimeDouble (int panel, int table, int x, int y, double dTime_ns);


int WFM_plot (int panel, int control, t_sequence *seq, t_waveform *wfm, 
				   t_digitizeParameters *p, int retainGraph);
				   





void WFM_resizePanel (void);

void WFM_initPanel (void);

int WFM_panelMinimumHeight(void);

void WFM_show (t_sequence *seq, int nr);

void WFM_edit (t_sequence *ctrl, int nr, int sort);


void WFM_TABLE_displayAllWfms (int panel, int control, t_sequence *seq, int activeNr, int sort);


void WFM_ADDR_showImages (t_image *img, t_waveform *wfm, int imageSize, const char *txt);	



//void WFM_ADDR_findPhase (t_image *img, t_waveform *wfm);

double WFM_ADDR_findFocusFromIdleSequence(t_sequence* seq);

void WFM_ADDR_findPhase (t_image *img, t_waveform *wfm, dPoint lastPhase, dPoint lastPhaseErr);


dPoint WFM_ADDR_calculateAddrVoltageOffset (t_image *img, t_waveform *wfm);



void WFM_ADDR_showPointListInGraph (int panel, int ctrl, ListType pointList, int type, int color, int showText);


void WFM_ADDR_writeDataToProtocolFile (char *filename, char *imageFilename, int repetition, dPoint phase, dPoint phaseErr, dPoint Uoffs, dPoint UAddrOffs, dPoint UTotal, dPoint lastPhaseOffset);


void WFM_ADDR_writePointListToFile (char *filename, ListType pointList);


int WFM_ADDR_allowStepParameter (int parameterID);


#endif
