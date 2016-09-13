#include <gpib.h>

#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"

#include "GPIB_DRIVER_Anritsu.h"


int GPIB_transmitFreq_AnritsuMG3692A (t_sequence *seq, t_gpibCommand *g, int dummy1, int dummy2);
int GPIB_getDeviceOptions_AnritsuMG3692A (int gpibAddress);









t_gpibDevice *GPIB_deviceParameters_AnritsuMG3692A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "ANRITSU,MG3692");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->minFrequency   = 2.0E9;
		dev->maxFrequency   = 20.0E9;
		dev->minAmplitude   = -10;
		dev->maxAmplitude   = +30;
		dev->maxDivide	    = 4;
		
		// initialize functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_AnritsuMG3692A;
		dev->GPIB_getDeviceOptions = GPIB_getDeviceOptions_AnritsuMG3692A;
	}
	return dev;
}



int GPIB_getDeviceOptions_AnritsuMG3692A (int gpibAddress)
{
	//char optionStr[200];
	t_gpibDevice *d;
	//int i, j;
//	char *startCh;
	
	
	d = GPIBDEVICE_ptr (gpibAddress);
	if (d == NULL) return 0;

/*	GPIB_open (gpibAddress, T10s);

/*	if (GPIB_sendf (gpibAddress, "XP") != 0) 
		return GPIB_errorMessage(gpibAddress);

	if (GPIB_sendf (gpibAddress, "OSE") != 0) 
		return GPIB_errorMessage(gpibAddress);

	optionStr[0] = 0;
//	if (GPIB_sendf (gpibAddress, "*IDN?") != 0) 
	Sleep (50);  
	GPIB_receive (gpibAddress, optionStr, 200);
	
	// parse optionStr;
/*	printf ("device %d: %s\n", gpibAddress, optionStr);
	startCh = optionStr;

    while (startCh != NULL) {
	 	if (strncmp (startCh, "B3", 2) == 0) d->hasPulseOption = 1;
	 	startCh = strchr (startCh, ',');
	 	if (startCh != NULL) startCh++;
	}			    
*/
	d->hasPulseOption = 1;
	return 0;
	
	
}

int GPIB_transmitFreq_AnritsuMG3692A (t_sequence *seq, t_gpibCommand *g, int dummy1, int dummy2)
{
    double freq;
    char *strRFOnOff[2] = {"RF0", "RF1"};
    //char errorStr[ERRMSG_LEN];
    t_gpibDevice *d;

	d = g->device;
	d = GPIBDEVICE_ptr (g->gpibAddress);

//	GPIB_open (g->gpibAddress, T10s);
	if (d->handle < 0) {
		GPIB_open (g->gpibAddress, T10s);
		ibclr (d->handle);
	}
	
	if (g->enableStep) {
		// exit if no step necessary
//		if (seq->repetition % g->nStepRepetition != 0) return 0;
//	 	if (seq->repetition == 0) ibclr (g->device->handle);
	 	freq = GPIB_getStepFrequencyInHz (seq, g);
	}
	else freq = (g->stopFreq - g->startFreq) / 2 + g->startFreq + g->addFreqOffset * g->freqOffsetMHz * 1E6 ;

	
	if (d->last_divide != g->divideFreq) {
		if (GPIB_sendf (g->gpibAddress, "FRS %d TMS", g->divideFreq) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_divide = g->divideFreq;
		
	}

	if (d->last_freq != freq) {
		if (GPIB_sendf (g->gpibAddress, "CF1 %2.12f GH", freq / 1.0E9) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_freq = freq;
			
	}

	if (g->enableStep) {
		if (g->stepTrigger != GPIB_COMMAND_FREQ_TRIGGER_CALC) 
		{
			PostMessagePopup ("Warning", "Trigger mode not supported for device\n");
			g->stepTrigger = GPIB_COMMAND_FREQ_TRIGGER_CALC;
		}
	}
	if (d->hasPulseOption) {
		if (d->last_pulseMode != g->pulseModeEnable) {
			if (GPIB_sendf (g->gpibAddress, "%s", g->pulseModeEnable ? "XP" : "P0") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->pulseModeEnable) {
				if (GPIB_send (g->gpibAddress, "EP1") != 0) 
					return GPIB_errorMessage(g->gpibAddress);
			}
		}
		d->last_pulseMode = g->pulseModeEnable;
		
	}
	
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, "L1 %2.3f DM", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_power = g->outputPower;
	}
	
	if (d->last_on != g->rfOn) {
		if (GPIB_sendf (g->gpibAddress, strRFOnOff[g->rfOn]) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_on = g->rfOn;
	}
	

	return 0;
}

