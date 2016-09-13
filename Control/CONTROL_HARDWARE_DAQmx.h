#ifndef HARDWARE_DAQmx
#define HARDWARE_DAQmx




#include <NIDAQmx.h>


#define DAQmxErrChk(functionCall) {DAQmxError = (functionCall);  {if (DAQmxError < 0){goto Error;}}}



int32 HARDWARE_NIUSB_CreateTask_AnalogInput (TaskHandle *taskOut1, const char *channel);

int HARDWARE_NIUSB_readValue (TaskHandle th, double *value);



int HARDWARE_ANALOGin_createTasts (void);


int32 CreateNIUSB_AnalogInput(TaskHandle *taskOut1);

int HARDWARE_DAQmx_displayError (int errorCode);




int HARDWARE_NIUSB_TestDataAcquisition (const char *deviceStr, double *value);



#endif
