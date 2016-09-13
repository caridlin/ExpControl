#ifndef GUI_Gpib
#define GUI_Gpib


//#include "hardware_gpib.h"


int activeGpibCommandNo(void);

void GPIB_resizePanel (void);

int GPIB_panelMinimumHeight(void);

void GPIB_initPanel (void);

void GPIB_displayValues (t_sequence *seq, int nr);

void GPIB_changeVisibility (t_gpibCommand *g);

//void GPIB_calculateNPoints (t_gpibCommand *g);

void GPIB_fillDevicesToList (int panel, int control, int type);

void GPIB_fillWaveformNamesToAllRings (t_sequence *seq);

void GPIB_fillCommandNamesToListbox  (t_sequence *seq, int panel, int control, int activeValue, int simpleMode);


#endif
