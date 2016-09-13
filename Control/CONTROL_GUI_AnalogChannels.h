#ifndef GUI_ANALOG
#define GUI_ANALOG

//int nDisplayDAC (void);



int isAnalogChannelActive (t_sequence *seq, int nr);


void ANALOGBLOCKS_calculateDACPanelPositions (void);


int getDACpanelHandle (int nr);

void ANALOGCHANNELS_fillWaveformNamesToAllRings (t_sequence *seq);

void ANALOGCHANNELS_selectWaveforms (t_digitalBlock *b, t_sequence *seq);



void displayWaveformName (int panel, int control, int wfmNr);



//void showAnalogBlock (t_sequence *seq, int nr);

//Int ANALOGBLOCKS_displayValues (t_sequence *seq, int nr);



#endif

