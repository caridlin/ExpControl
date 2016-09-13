#ifndef HARDWARES
#define HARDWARES

#include <NIDAQmx.h>


#define ID_INVALID_DEVICE   -1
#define ID_PCI_DIO_32_HS   211
#define ID_PCI_6713        263

#define ID_PCI_6534        216
#define ID_PCI_6602        232
#define ID_PCI_6733        350



#define N_OUTPUT_PORTS 4


#define VAL_ACK1_TRIGGER -255

#define N_TRIGGER_SOURCES 2		 //Used to be 12
extern const int HARDWARE_TRIGGER_SOURCE[N_TRIGGER_SOURCES];



#define ANALOG_MIN_SAMPLES 5000




void HARDWARE_setTransmit_CMD_READY (int state);

int HARDWARE_SetGetInterruptedFlag (int set);


int displayErrorMSG(int iStatus, int deviceNr, char *strFuncName);

//int errorCheck(int iStatus, int deviceNr, char *strFuncName);


int hardwareDetected (void);


char *timeStrSec64 (__int64 sec64);


//char *get_NIDeviceName (int deviceID);





void HARDWARE_resetStoredValues (void);

void HARDWARE_rememberLastValues (t_sequence *seq);

void HARDWARE_sendStoredValues (t_sequence *seq);

int HARDWARE_sendDirectDigitalValue (int ch, short bit);

int HARDWARE_sendDirectAnalogValue (int ch, double analogValue);

//int HARDWARE_setDigitalOutputDirection (void);

int HARDWARE_initializeBoards (void);

t_digitizeParameters *digitizeParameters_PCI67XX (int timebase);

//int HARDWARE_resetDigitalOutput (t_sequence *seq);

//int HARDWARE_resetAnalogOutput (t_sequence *seq);


int HARDWARE_sendAllGpibCommands (t_sequence *seq, int beginningOfSequence);

int HARDWARE_resetOutputs (t_sequence *seq);


//int HARDWARE_NI_detectBoards (int outPanel, int outCtrl);

int HARDWARE_stopDataOutput (void);

int sequenceStarted (void);

int startSequence (t_sequence *seq); 

char *HARDWARE_signalName (int i);

int HARDWARE_boot (int panel, int ctrl);


#endif
