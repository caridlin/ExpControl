#ifndef HARDWARE_GPIB
#define HARDWARE_GPIB

#include "CONTROL_DataStructure.h"


// max frequency of extarnal sclk = 20 MHz
#define GPIB_MAX_FREQ_EXT_SCLK 20.0E6


#define ERRMSG_LEN 400



#define GPIB_ERR_NOT_SUPPORTED -12000
#define GPIB_ERR_GETSETTINGS   -12001

int GPIB_outputPanel (void);
int GPIB_outputCtrl (void);

const char *iberrStr (void);

int ibstaCheck (int address, const char *command);


double GPIB_getStepFrequencyInHz (t_sequence *seq, t_gpibCommand *g);

void GPIB_open (int address, int timeout);

int GPIB_close (int address);

int GPIB_send (int address, char *str);

int GPIB_getErr (t_gpibCommand *g, int showMessage);

int GPIB_showDeviceErrorMessage  (int address, int errNo, char *str);

int GPIB_sendf (int address, char* format, ... );

int GPIB_sendBinary (int address, char *commandPrefix, short *binaryData, int nPoints, int dma);

int GPIB_receive (int address, char *str, int maxLength) ;

int GPIB_errorMessage (int address);

void GPIB_deviceInitAllAvailableDevices (void);

int GPIB_identifyDevice (t_gpibDevice *dev, const char *strIDN);

void GPIB_setOutputCtrl (int panel, int ctrl);

void GPIB_clearOutputCtrl (void);

void GPIB_sendReset (int address);

int GPIB_autoDetectDevice (t_config *oldConfig, t_gpibDevice *dev, int i);

int GPIB_autodetectBoard (void);

t_digitizeParameters *GPIB_digitizeParameters (t_gpibCommand *g);

void GPIB_setLed (int state, char *text);

void triggerLoop(void);

void recvTriggerLoop(void);

int GPIB_transmitCommand (t_sequence *s, t_gpibCommand *g, 
							int waitUntilStabilized, int allowAutodetect);

void GPIB_setShowDataFlag (int show);

int GPIB_getShowDataFlag (void);

void GPIB_resetOutput (void);

void GPIB_resetAllLastValues (t_sequence *seq, int resetLeCroy, int resetWaveforms);

void GPIB_fillDevicesToList (int panel, int control, int type);

void EXTDEV_fillDevicesToList (int panel, int control, int type);

int GPIB_autodetectAllDevices (int panel, int control);    

void GPIB_displaySupportedDevices (int panel, int ctrl);

int GPIB_detectBoard (int outPanel, int outCtrl);

int GPIB_getFeedbackInputValue (t_sequence *seq, t_gpibCommand *g);

int GPIB_addFrequencyFeedbackToAllCommands (t_sequence *seq, const char *path);


#endif
