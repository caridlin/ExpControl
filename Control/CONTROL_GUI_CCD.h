#ifndef GUI_CCD
#define GUI_CCD



#include "CONTROL_Datastructure.h"




void CCD_resizePanel (t_sequence *seq);


void CCD_initPanel (void);

void CCD_displayAll (t_sequence *seq);

void CCDSETTINGS_fillNamesToListbox  (t_sequence *seq, int panel, int ctrl);

void CCD_writeAllCommandFiles (t_sequence *seq, int autoSave);

void CCD_getNumberOfPicturesPerRepetition (t_ccdSettings *c, int *nPics, int *controlMode);

void CCD_sendCommandFile (void);




#endif


