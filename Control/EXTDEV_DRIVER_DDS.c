#include <windows.h> // for  Sleep 
#include <formatio.h>
#include <utility.h>
#include "toolbox.h"
#include "tools.h"   
#include <userint.h>
#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"
#include "imageProcessing.h"
#include "CONTROL_GUI_EXTDEV.h"
#include <cvintwrk.h>
#include <cvirte.h>
#include <userint.h>
#include <ansi_c.h>
#include "CONTROL_HARDWARE.h" 
#include "CONTROL_HARDWARE_GPIB.h"

extern int gpibOutputPanel;
extern int gpibOutputCtrl;



static void ProcessResponse(char *string)
{
	size_t	i, length;

	length = strlen (string);
	for (i = 0; i < length; ++i)
		if (!isprint(string[i]) && !isspace(string[i]))
			string[i] = ' ';
}

static void ReadResponse (int gTelnetHandle)
{
	int		error = 0;
	ssize_t	bytesRead, totalBytesRead = 0;
	char	response[128];

	// Read response from telnet server and update user interface
	while (1)
		{
		bytesRead = 0;
		error = InetTelnetRead (gTelnetHandle, response, sizeof (response) - 1,
			&bytesRead, 1000);
		if (bytesRead == 0 && totalBytesRead > 0)
			break;
		totalBytesRead += bytesRead;
		ProcessResponse (response);
	}
}



typedef struct { 
	t_sequence *seq;
	t_gpibCommand *g;
	t_ext_device *extdev;
} t_dds_tramit_frequency_data;



int CVICALLBACK EXTDEV_DDS_transmitFrequency (void *functionData)
{
	t_dds_tramit_frequency_data *data;
	int gTelnetHandle = -1;
	char command[256];
	unsigned int channel;
	double freqHz;
	t_sequence *seq;
	t_gpibCommand *g;
	t_ext_device *extdev;
	clock_t startTime;
	
	startTime = clock();
	
	data = (t_dds_tramit_frequency_data*)functionData;
	if (data == NULL) return -1;
	
	seq = data->seq;
	g = data->g;
	extdev = data->extdev;
	if (seq == NULL || g == NULL || extdev == NULL) {
		return -1;
	}
	free(data);
	
	channel = g->extDeviceChannel;

	//pprintf(gpibOutputPanel, gpibOutputCtrl, "Send Telnet command to device %s\n",extdev->name);
	if (!g->enableStep) {
		freqHz = (g->stopFreq - g->startFreq) / 2 + g->startFreq + g->addFreqOffset * g->freqOffsetMHz * 1E6 + g->summedFeedbackFrequencyOffset;
	}
	else{
		freqHz = GPIB_getStepFrequencyInHz (seq, g) + g->summedFeedbackFrequencyOffset;
	}
	
	sprintf(command,"setChannel(SingleTone,%d,1,%09.2f);",channel,freqHz);
	gTelnetHandle = InetTelnetOpen (extdev->address, (unsigned short)extdev->port, 0);
	
	if (gTelnetHandle<0){
		pprintf(gpibOutputPanel, gpibOutputCtrl, "Telnet connection failed. Error code:%d\n",gTelnetHandle);
		PostMessagePopupf ("Error", "Telnet connection failed.\n\nError code %d: %s",gTelnetHandle, InetGetErrorMessage (gTelnetHandle));
		return gTelnetHandle;
	}
	InetTelnetWrite (gTelnetHandle, command, "\r\n", -1, 0, 100);
	
	if (g->askDeviceForErrors) {
	}
	InetTelnetWrite (gTelnetHandle, "close();", "\r\n", -1, 0, 100);
	InetTelnetClose (gTelnetHandle);
	
	pprintf(gpibOutputPanel, gpibOutputCtrl, "Send Telnet command to device %s (%.f ms)\n%s\n",extdev->name, timeStop_s (startTime)*1000,command);

	return 0;
	
}

void EXTDEV_DDS_THREAD_transmitFrequency (t_sequence *seq, t_gpibCommand *g,t_ext_device *extdev)  
{
	t_dds_tramit_frequency_data* data;
	data = (t_dds_tramit_frequency_data*)malloc(sizeof(t_dds_tramit_frequency_data));
	data->seq = seq;
	data->g = g;
	data->extdev = extdev;
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, EXTDEV_DDS_transmitFrequency, data, NULL);
}
