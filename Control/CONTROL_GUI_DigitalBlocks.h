#ifndef GUI_DigitalBlocks
#define GUI_DigitalBlocks



#define NColors 12

extern const int colors[NColors]; 

// conversion unisgned long <-> float
unsigned long timeToInt(double t);

double ulToTime (unsigned __int64 time);

void setTime (int panel, int ctrl, unsigned __int64 uLtime, int sign);



void DIGITALBLOCKS_resizePanel (void);

int  DIGITALBLOCKS_initPanel (void);

void DIGITALBLOCKS_initDigitalChannelFields (void);

void DIGITALBLOCKS_insertTableColumns (int panel, int ctrlHeader, int ctrlDac, int ctrlDigital, int startCol, int nColumns);


void DIGITALBLOCKS_displayAllDirectIOButtons (t_sequence *seq);//, int nChannels, int *channels);

void DIGITALBLOCKS_displayDirectIOButton (t_sequence *seq, int channel);//, int state);

void DIGITALBLOCKS_displayHeaderInTable (t_sequence *seq, t_digitalBlock *b, int panel, int ctrlHeader);

void DIGITALBLOCKS_setBlockMode (t_sequence *seq, t_digitalBlock *b, int mode);

void DIGITALBLOCKS_displayGlobalAnalogVoltages (void);

void DIGITALBLOCKS_editVariableTime (t_digitalBlock *block, int top, int left);

void DIGITALBLOCKS_EDIT_deleteLoop (t_sequence *seq, int nr);

void DIGITALBLOCKS_displaySequenceDuration (t_sequence *seq);

//void DIGITALBLOCKS_resetAllTableAttributes (t_sequence *seq);

void DIGITALBLOCKS_displayAllBlocks (t_sequence *seq);

void BLOCK_editVariablePulse (t_digitalBlock *d, int channel);

void DIGITALBLOCKS_updateCorrespondingChannels(t_sequence *seq, int wfmNr);






#endif
