
#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"
#include "gpib.h"
#include "GPIB_DRIVER_Agilent.h" 



//  ========================================================= 
// BEGIN SG384 synthesizer
//  ========================================================= 

int GPIB_transmitFreq_SMY (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2)
{
    double freqHz;
    t_gpibDevice *d;        

	GPIB_open (g->gpibAddress, T10s);

	if (g->enableStep && (seq->repetition % g->nStepRepetition != 0)) 
		return 0; 		// exit if no step necessary
	
	d = GPIBDEVICE_ptr (g->gpibAddress);

 	// clear device
 	
 	if (seq->repetition == 0) {
 		ibclr (d->handle);

		// reset error queue
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	}

	GPIB_getFeedbackInputValue (seq, g);
	
	if (!g->enableStep) {
		// -----------------------------------------
	    //   CW mode
		// -----------------------------------------
		freqHz = (g->stopFreq - g->startFreq) / 2 + g->startFreq + g->addFreqOffset * g->freqOffsetMHz * 1E6 + g->summedFeedbackFrequencyOffset;
		if (d->last_freq != freqHz) {  
			if (GPIB_sendf (g->gpibAddress, 
							"RF %2.10fMHz", freqHz / 1E6) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			d->last_freq = freqHz;
		}
		
		goto SET_OUTPUT; 
	}

	
 	
 	if (g->stepTrigger == GPIB_COMMAND_FREQ_TRIGGER_CALC) {
		// -----------------------------------------
		//   calculate frequency manually
		// -----------------------------------------
 		freqHz = GPIB_getStepFrequencyInHz (seq, g) + g->summedFeedbackFrequencyOffset;
		if (d->last_freq != freqHz) {  
			if (GPIB_sendf (g->gpibAddress, 
							"RF %2.10fMHz", freqHz / 1E6) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			d->last_freq = freqHz;
		}
		goto SET_OUTPUT; 
 	}
	
	if (seq->repetition != 0) {
		// -----------------------------------------------------
		//   not first run --> trigger if necessary, then exit
		// -----------------------------------------------------
		if (g->stepTrigger == GPIB_COMMAND_FREQ_TRIGGER_GPIB) {
			if (GPIB_send (g->gpibAddress, 
							"*TRG")  != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
		}
		return 0;
	}


SET_OUTPUT:	
	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"LEVEL %2.2fDBM", g->outputPower) != 0) {
			return GPIB_errorMessage(g->gpibAddress);
		}
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_power = g->outputPower;
	}
	// -----------------------------------------
    //    output on/on
	// -----------------------------------------
	if (d->last_on != g->rfOn) {
		if (g->rfOn) {
		if (GPIB_sendf (g->gpibAddress,"LEVEL:ON") != 0)
			return GPIB_errorMessage(g->gpibAddress);
		} else {
		if (GPIB_sendf (g->gpibAddress,"LEVEL:OFF") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		}
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_on = g->rfOn;
	}
	
	return 0;
}


int GPIB_getDeviceOptions_RohdeSchwarzSML02 (int gpibAddress)
{
	char optionStr[200];
	t_gpibDevice *d;
	char *startCh;
	
	
	d = GPIBDEVICE_ptr (gpibAddress);
	if (d == NULL) return 0;

	GPIB_open (gpibAddress, T10s);

	if (GPIB_sendf (gpibAddress, "*OPT?") != 0) 
		return GPIB_errorMessage(gpibAddress);

	GPIB_receive (gpibAddress, optionStr, 200);
	
	// parse optionStr;
	startCh = optionStr;

    while (startCh != NULL) {
	 	if (strncmp (startCh, "B3", 2) == 0) d->hasPulseOption = 1;
	 	startCh = strchr (startCh, ',');
	 	if (startCh != NULL) startCh++;
	}			    
	return 0;
	
	
}

t_gpibDevice *GPIB_deviceParameters_RohdeSchwarzSMY01 (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "ROHDE&SCHWARZ,SMY01");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->minFrequency   = 9.0E3;
		dev->maxFrequency   = 1.04E9;
		dev->minAmplitude   = -40;
		dev->maxAmplitude   = +15;
//		dev->pulseModeEnable = 0;
//		dev->pulseModeLogic = 0;
		dev->enableExternalTrigger = 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_SMY;
	}
	return dev;
}

t_gpibDevice *GPIB_deviceParameters_RohdeSchwarzSMY02 (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "ROHDE&SCHWARZ,SMY02");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->minFrequency   = 9.0E3;
		dev->maxFrequency   = 2.08E9;
		dev->minAmplitude   = -40;
		dev->maxAmplitude   = +15;
//		dev->pulseModeEnable = 0;
//		dev->pulseModeLogic = 0;
		dev->enableExternalTrigger = 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_SMY;
	}
	return dev;
}



t_gpibDevice *GPIB_deviceParameters_RohdeSchwarzSML02 (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "ROHDE&SCHWARZ,SML02");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->minFrequency   = 9.0E3;
		dev->maxFrequency   = 2.0E9;
		dev->minAmplitude   = -40;
		dev->maxAmplitude   = +15;
//		dev->pulseModeEnable = 0;
//		dev->pulseModeLogic = 0;
		dev->enableExternalTrigger = 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent83751A;
		dev->GPIB_getDeviceOptions = GPIB_getDeviceOptions_RohdeSchwarzSML02 ;
	}
	return dev;
}


