#include <ansi_c.h>
#include <analysis.h>
#include <NIDAQmx.h>

#include "tools.h"
#include "CONTROL_DataStructure.h"
#include "CONTROL_HARDWARE_DAQmx.h"


extern int gpibOutputPanel;
extern int gpibOutputCtrl;


int HARDWARE_DAQmx_displayError (int errorCode)
{
	int bufferSize;
	char *errorString;
	char *extendedErrorString;
	
	bufferSize = DAQmxGetErrorString (errorCode, 0, 0);
	errorString = (char *) malloc (bufferSize);
	DAQmxGetErrorString (errorCode, errorString, bufferSize);

	bufferSize = DAQmxGetExtendedErrorInfo (0, 0);
	extendedErrorString = (char *) malloc (bufferSize);
	DAQmxGetExtendedErrorInfo (extendedErrorString, bufferSize);
	
	PostMessagePopupf ("Error", "NIDAQmx returned an error.\n\n%s", extendedErrorString);
	
	free (errorString);
	free (extendedErrorString);
	
	return errorCode;
}






//**************************************************************************
//* This generated function configures your DAQmx task.                    *
//*                                                                        *
//* Follow these steps to use this generated function:                     *
//*   1) Define a task handle variable in your program.                    *
//*   2) Call the generated function.                                      *
//*   3) Use the returned task handle as a parameter to other DAQmx        *
//*      functions.                                                        *
//*   4) Clear the task handle when you finish.                            *
//*                                                                        *
//*         TaskHandle task = 0;                                           *
//*         CreateDAQTask(&task);                                          *
//*         <use the DAQmx task handle>                                    *
//*         DAQmxClearTask(task);                                          *
//*                                                                        *
//**************************************************************************
int32 HARDWARE_NIUSB_CreateTask_AnalogInput (TaskHandle *taskOut1, const char *channel)
{
	int32 DAQmxError = DAQmxSuccess;
    TaskHandle taskOut;
	int oldState;

	oldState = SetBreakOnLibraryErrors (0);
	DAQmxErrChk(DAQmxCreateTask("NIUSB_AnalogInput", &taskOut));

	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskOut, channel, "Spannung",
		DAQmx_Val_Diff, -10, 10, DAQmx_Val_Volts, ""));	

	DAQmxErrChk(DAQmxCfgSampClkTiming(taskOut, "", 
		48000, DAQmx_Val_Rising, 
		DAQmx_Val_FiniteSamps, 200));

    *taskOut1 = taskOut;
	SetBreakOnLibraryErrors (1);

Error:
	if (DAQmxError != 0) HARDWARE_DAQmx_displayError (DAQmxError);
	
	return DAQmxError;
}






int HARDWARE_NIUSB_createTasks (t_sequence *seq)
{
		
//	HARDWARE_NIUSB_CreateTask_AnalogInput (&analogIn_TaskHandle[0], "Dev3/ai0");
	
	return 0;
	
}


int HARDWARE_NIUSB_clearTasks (t_sequence *seq)
{
//	DAQmxClearTask(analogIn_TaskHandle[0]);	
	
	
	return 0;
}



int HARDWARE_NIUSB_readValue (TaskHandle th, double *value)
{
	//int error;
	int64 nSamples = 0;
	double *dataArray;
	int samplesRead;
	int32 DAQmxError = DAQmxSuccess;
	
	*value = 0;
	DAQmxErrChk(DAQmxGetTimingAttribute (th, DAQmx_SampQuant_SampPerChan, &nSamples, 0));
	dataArray = (double *) calloc (nSamples, sizeof (double));
	
	DAQmxErrChk(DAQmxReadAnalogF64 (th, DAQmx_Val_Auto, 10.0,
								DAQmx_Val_GroupByScanNumber, dataArray,
								nSamples, &samplesRead, 0));
  	Mean (dataArray, nSamples, value);
								
	return 0;	
	
Error:
	if (DAQmxError != 0) HARDWARE_DAQmx_displayError (DAQmxError);
	return DAQmxError;
	
}


int HARDWARE_NIUSB_TestDataAcquisition (const char *deviceStr, double *value)
{
	TaskHandle th;
	//int error;
	clock_t startTime;
	int32 DAQmxError = DAQmxSuccess;
	
	*value = 0;
	startTime = clock ();

	DAQmxError = HARDWARE_NIUSB_CreateTask_AnalogInput (&th, deviceStr);
	if (DAQmxError != DAQmxSuccess) goto Error2;

	DAQmxErrChk(DAQmxStartTask (th));

	HARDWARE_NIUSB_readValue (th, value);
	
	DAQmxErrChk(DAQmxClearTask(th));	
	
	pprintf (gpibOutputPanel, gpibOutputCtrl, "Value = %1.3f V  [%1.3f s]\n", *value, timeStop_s (startTime));
	return 0;

	
	
Error:
	if (DAQmxError != 0) HARDWARE_DAQmx_displayError (DAQmxError);

Error2:	
	DAQmxErrChk(DAQmxClearTask(th));	
	return DAQmxError;
}
