#ifndef calculate_Output
#define calculate_Output		 


//#define N_AO_BITS 12  // Anzahl Bits für Analog-AO

#include "CONTROL_DataStructure.h"
#include "imageProcessing.h"
#include "tools.h"

//int SEQUENCE_isAnalogChannelActive (t_sequence *seq, int nr);


unsigned SEQUENCE_nTicksStopCtr (t_sequence *seq);



int SEQUENCE_insertBlockNumbersAndReferences (t_sequence *seq, int *nTriggerDuringSeq);


int DIGITALBLOCK_getWaveformsAndDuration (t_sequence *seq, t_digitalBlock *b);             




void WFM_calculateOffsetAndDuration (t_waveform *wfm, int repetition, int nRepetitions, int allowNegativeTimes);



double WFM_getStepOutputVoltage (t_sequence *seq, t_waveform *wfm);


int WFM_calculateNPoints (t_waveform *wfm, unsigned long nDigitalPoints, 
					   unsigned long maxNValues, t_transferFunction *f,
					   double addVoltage, t_digitizeParameters *p, int fastMode); 



unsigned long WFM_digitize (t_sequence *seq, t_waveform *wfm, 
   							    t_digitizeParameters *p, int allowAddrLinkWfm);


int WFM_ADDR_matrixNumThisRepetition(t_waveform* wfm, int rep, int nRep, int picNum);

ListType WFM_ADDR_convertMatrixToImageCoords (t_waveform *wfm, t_image *img, int matrixNum);

ListType WFM_ADDR_convertVoltagesToImageCoords (t_waveform *wfm1, t_waveform *wfm2, dPoint offset);


ListType WFM_ADDR_trajCalculateImageCoords (t_waveform *wfm, t_image *img);


ListType WFM_ADDR_lastVoltagesList (t_waveform *wfm1, t_waveform *wfm2);

ListType WFM_ADDR_voltageListToImageCoords (t_waveform *wfm, ListType lastVoltagesList, dPoint Uoffset);




dPoint WFM_ADDR_ctrlVoltageToImage (t_waveform *wfm, double U1, double U2);

dPoint WFM_ADDR_imageToCtrlVoltage (t_waveform *wfm, dPoint p, dPoint *err);

t_waveform *WFM_ADDR_createWaveform (t_sequence *seq, t_waveform *wfm, double phi1, double phi2);


dPoint WFM_ADDR_voltageOffsetFromPhaseDifference (t_waveform *wfm, t_image *img, Point testPoint);

void WFM_ADDR_offsetThisRepetition (t_waveform *wfm, int repetition, int nRepetitions);


void OUTPUTDATA_getTimebases (t_sequence *seq);



void OUTPUTDATA_checkMemoryForAOValues (t_outputData *out, int ch, unsigned long bytesToAdd);

unsigned long OUTPUTDATA_appendBlockAOValues (t_outputData *out, t_digitalBlock *bl, 
                       	       int ch, 
                       	       int repetition, int nRepetitions);
                       	       

int OUTPUTDATA_calculate (t_sequence *seq, int firstRepetition, int simulate);

//unsigned long OUTPUTDATA_writeDIOBytesToOutputBuffer_OLD (t_sequence *s);

                       	       
unsigned long OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (t_outputData *o,
			  unsigned long **buffers, unsigned long bufferSizeSamples);

unsigned long OUTPUTDATA_writeAOBytesToOutputBuffer_DBLBUF (t_outputData *o,
			  short **buffers, unsigned long bufferSizeSamples);


//unsigned long OUTPUTDATA_writeAOBytesToOutputBuffer_OLD (t_outputData *o, int AOdevice, int progressDialog, int percStart, int percStop);


//int OUTPUTDATA_writeAOBytesToOutputBuffer_sweepVoltages (t_outputData *o, int AOdevice);


void OUTPUTDATA_putExternalAOChannelsIntoBuffer 
		(t_outputData *outData, int firstChannel, int lastChannel, 
		 char **dataBuffer, unsigned *dataBufferSize);

t_outputData *OUTPUTDATA_getExternalAOChannelsFromBuffer 
		(char *buffer, unsigned bufferSize, int nChannels);



long OUTPUTDATA_getFirstOutputValue (t_outputData *out, int ch, unsigned *position);


int OUTPUTDATA_isChannelConstant (t_outputData *out, int ch);


int OUTPUTDATA_hasChanged (t_outputData *out, int ch);

void OUTPUTDATA_rememberLastValues (t_outputData *out, int ch);

//int OUTPUTDATA_initOutputBuffers (t_outputData *o, int simulate);

//int OUTPUTDATA_areOutBuffersIdentical (t_outputData *o1, t_outputData *o2);

void OUTPUTDATA_displayBuffer (unsigned long *buffer, unsigned long bufSize);


void OUTDATA_ADWIN_displayDigitalValues (t_adwinData *a);

int OUTPUTDATA_ADWIN_initPhaseFeedback (t_sequence *seq);



#endif
