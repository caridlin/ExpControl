#ifndef GUI_Simulate
#define GUI_Simulate




void SIMULATE_start (t_sequence *seq, int update);


void debugDisplayWaveform (t_waveform *wfm);

void debugDisplayOutSequence (t_outputData *out, int ch);

void debugDisplayAOBufferContents (short *outBuf, unsigned long NValues, int NChannels);


#endif
