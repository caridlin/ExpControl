#include <utility.h>
#include <gpib.h>

#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"

#include "GPIB_DRIVER_Agilent.h"



int GPIB_transmitFreq_Agilent33250A (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2);
int GPIB_transmitWaveform_Agilent33250A (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2);

int GPIB_transmitWaveform_Agilent33120A (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2);
int GPIB_transmitCommand_6612C (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2);
int GPIB_transmitCommand_E3643A (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2);


int GPIB_transmitWaveform_Keithley3390 (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2);


int GPIB_transmitWaveform_Agilent33220A (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2);

int GPIB_transmitFreq_AgilentE4432B (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2);  

const char *strOnOff[2] = {"OFF", "ON"}; 


const char *agilentLoadStr (int load)
{
	if (load == LOAD_OPEN) return "INF";
	if (load == 50) return "50";
	else return "";
}


//  =========================================================
//
//            Agilent 33220A
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_Agilent33220A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Agilent Technologies,33220A");
		dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
		dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
		dev->wfmMinPoints   = 8;
		dev->wfmMaxPoints   = 65536;
		dev->minVoltage     = -10.0;
		dev->maxVoltage     =  10.0;
		dev->minDigital     = -8191;
		dev->maxDigital		= 8191;
		dev->minDuration_us = 0.040;
		dev->nChannels		= 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;

		dev->minFrequency   = 0.001;
		dev->maxFrequency   = 20.0E6;
		dev->minAmplitude   = -36.0;
		dev->maxAmplitude   = +23.9;
		
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent33250A;
		dev->GPIB_transmitWaveform  = GPIB_transmitWaveform_Agilent33220A;
	}
	return dev;
}



//  =========================================================
//
//            Agilent 33120A
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_Agilent33120A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		strcpy (dev->strIDN, "HEWLETT-PACKARD,33120A");
		dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
//		dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
		dev->wfmMinPoints   = 8;
		dev->wfmMaxPoints   = 16000;
		dev->minVoltage     = -5.0;
		dev->maxVoltage     =  5.0;
		dev->minDigital     = -2047;
		dev->maxDigital		= 2047;
		dev->minDuration_us = 0.040;
		dev->nChannels		= 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		// initialize pointer to functions			
		dev->GPIB_transmitWaveform  = GPIB_transmitWaveform_Agilent33120A;
	}
	return dev;
}



//  =========================================================
//
//            Agilent N5182A
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_AgilentN5182A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Agilent Technologies, N5182A");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		dev->hasPulseOption = 1;

		dev->minFrequency   = 100.0E3;
		dev->maxFrequency   = 3.0E9;
		dev->minAmplitude   = -110;
		dev->maxAmplitude   = +16;
		
		dev->hasAM = 1;
		dev->minAMdepthPercent = 0.1;

		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent83751A;
	}
	return dev;
}



//  =========================================================
//
//            Agilent N5182B
//
//  =========================================================

// TODO not all commands checked!!!
t_gpibDevice *GPIB_deviceParameters_AgilentN5182B (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Agilent Technologies, N5182B");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		dev->hasPulseOption = 1;

		dev->minFrequency   = 100.0E3;
		dev->maxFrequency   = 3.0E9;
		dev->minAmplitude   = -140;
		dev->maxAmplitude   = +16;
		
		dev->hasAM = 1;
		dev->minAMdepthPercent = 0.1;

		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent5182B;
	}
	return dev;
}



//  =========================================================
//
//            Agilent E4432B
//			  (ESG-D3000B,Hewlett-Packard)
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_AgilentE4432B (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Hewlett-Packard, ESG-D3000B");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		dev->hasPulseOption = 1;

		dev->minFrequency   = 250.0E3;
		dev->maxFrequency   = 3.0E9;
		dev->minAmplitude   = -136;
		dev->maxAmplitude   = +20;
		
		dev->hasAM = 1;
		dev->minAMdepthPercent = 0.1;

		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_AgilentE4432B;//GPIB_transmitFreq_Agilent83751A; 
	}
	return dev;
}





//  =========================================================
//
//            Keithley 3390
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_Keithley3390 (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Keithley Instruments Inc.");
		dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
		dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
		dev->wfmMinPoints   = 8;
		dev->wfmMaxPoints   = 16000;
		dev->minVoltage     = -5.0;
		dev->maxVoltage     =  5.0;
		dev->minDigital     = -2047;
		dev->maxDigital		= 2047;
		dev->minDuration_us = 0.040;
		dev->nChannels		= 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;

		dev->minFrequency   = 0.001;
		dev->maxFrequency   = 50.0E6;
		dev->minAmplitude   = -10;
		dev->maxAmplitude   = +30;
		
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent33250A;
		dev->GPIB_transmitWaveform  = GPIB_transmitWaveform_Keithley3390;
	}
	return dev;
}


//  =========================================================
//
//            Agilent 33250A
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_Agilent33250A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Agilent Technologies,33250A");
		dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
		dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
		dev->wfmMinPoints   = 8;
		dev->wfmMaxPoints   = 16000;
		dev->minVoltage     = -5.0;
		dev->maxVoltage     =  5.0;
		dev->minDigital     = -2047;
		dev->maxDigital		= 2047;
		dev->minDuration_us = 0.040;
		dev->nChannels		= 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;

		dev->minFrequency   = 0.001;
		dev->maxFrequency   = 80.0E6;
		dev->minAmplitude   = -10;
		dev->maxAmplitude   = +30;
		
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent33250A;
		dev->GPIB_transmitWaveform  = GPIB_transmitWaveform_Agilent33120A;
	}
	return dev;
}




//  =========================================================
//
//            Agilent 83751A
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_Agilent83751A (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "HEWLETT-PACKARD,83751A");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		
		dev->minFrequency   = 0.01E9;
		dev->maxFrequency   = 20.0E9;
		dev->minAmplitude   = -10;
		dev->maxAmplitude   = +30;

		dev->errorMessageCompatibleWithAgilent = 1;
		dev->hasPulseOption = 1;
		
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent83751A;
	}
	return dev;
}





t_gpibDevice *GPIB_deviceParameters_Agilent6612C(void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		strcpy (dev->strIDN, "AGILENT 6612C");
		dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
		dev->minVoltage = 0;
		dev->maxVoltage = 32.0;
		dev->maxCurrent = 3;
		dev->nChannels = 2;
		
		// initialize pointer to functions			
		dev->GPIB_transmitPowerSupply = GPIB_transmitCommand_6612C;
	}
	return dev;
}




t_gpibDevice *GPIB_deviceParameters_AgilentE3643A(void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		strcpy (dev->strIDN, "Agilent Technologies,E3643A");
		dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
		dev->minVoltage = 0;
		dev->maxVoltage = 35.0;
		dev->maxCurrent = 1.4;
		dev->nChannels = 1;
		
		// initialize pointer to functions			
		dev->GPIB_transmitPowerSupply = GPIB_transmitCommand_E3643A;
	}
	return dev;
}





int GPIB_showDeviceMessage_Agilent33250A (t_gpibCommand *g, char *str, int showMessage)
{
	int errNo = 0;
	char *comma;

	comma = strchr (str, ',');
	if (comma == NULL) return 0;
	if (comma == str) 
		return 0;
	sscanf (str, "%d", &errNo);
	if ((errNo != 0) && !showMessage) return -1;
	return GPIB_showDeviceErrorMessage (g->gpibAddress, errNo, comma+1);
}



//=======================================================================
//  BEGIN:   AgilentE4432B
//=======================================================================
int GPIB_transmitFreq_AgilentE4432B (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2)
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
	// -----------------------------------------
	//     sweep parameters
	// -----------------------------------------
	if (GPIB_sendf (g->gpibAddress, 
					"FREQ:STAR %2.10f GHZ;STOP %2.10f GHZ", 
					g->startFreq / 1.0E9, g->stopFreq / 1E09) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}

	if (GPIB_sendf (g->gpibAddress, 
					"SWE:POIN %d", 
					seq->nRepetitions / g->nStepRepetition) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}
	
	// -----------------------------------------
	//     trigger mode
	// -----------------------------------------
	switch (g->stepTrigger) {
		case GPIB_COMMAND_FREQ_TRIGGER_GPIB:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR BUS") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
		case GPIB_COMMAND_FREQ_TRIGGER_EXT:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR EXT") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
	}
	

SET_OUTPUT:	

	if (d->hasPulseOption) {  // disable pulsed 
		if (GPIB_sendf (g->gpibAddress, 
						"PULM:STAT 0") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	}
	


// 	Frequency modulation : should work, but has to be tested!!
	
	if ((d->last_FM != g->enableFM) || (d->last_FMfreq_Dev != g->freqDeviation) || seq->firstRepetition) {
		if (GPIB_sendf (g->gpibAddress, 
						"FM:STAT %s", strOnOff[g->enableFM]) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		if (g->enableFM) {
			if (GPIB_sendf (g->gpibAddress, 
							"FM:SOUR EXT1") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_sendf (g->gpibAddress, 
							"FM:EXT1:COUP DC") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			if (GPIB_sendf (g->gpibAddress, 
							"FM:DEV %2.10f", g->freqDeviation) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
		}
		d->last_FM = g->enableFM;
		d->last_FMfreq_Dev = g->freqDeviation;
	}
	
	
	
//	AMPLITUDE MODULATION: should work, but has to be tested!!

	if (d->hasAM) {
		// -----------------------------------------
	    //   Amplitude Modulations
		// -----------------------------------------
		if ((d->last_AM != g->enableAM) || (d->last_AMdepthPercent != g->AMdepthPercent) || seq->firstRepetition){
			if (GPIB_sendf (g->gpibAddress, 
							"AM:STAT %s", strOnOff[g->enableAM]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}

			if (g->enableAM) {
				if (GPIB_sendf (g->gpibAddress, 
								"AM:SOUR EXT2;:AM %1.2fPCT", g->AMdepthPercent) != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (GPIB_sendf (g->gpibAddress, 
							"AM:EXT2:COUP DC") != 0) 
					return GPIB_errorMessage(g->gpibAddress);   
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}
				
			}
			d->last_AM = g->enableAM;
			d->last_AMdepthPercent = g->AMdepthPercent;
		}
	}
		
	if (d->hasPulseOption) {
		if (g->enableAM && strOnOff[g->pulseModeEnable])
			tprintf("conflict: cannot enable pulse mode and AM at the same time\n");
		else {	
			// -----------------------------------------
		    //   pulse mode
			// -----------------------------------------
			if (d->last_pulseMode != g->pulseModeEnable) {
				if (GPIB_sendf (g->gpibAddress, 
								"PULM:STAT %s", strOnOff[g->pulseModeEnable]) != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}

				if (g->pulseModeEnable) {
					if (GPIB_send (g->gpibAddress, 
									"PULM:SOUR EXT2") != 0) 
						return GPIB_errorMessage(g->gpibAddress);
					if (g->askDeviceForErrors) {
						if (GPIB_getErr (g,1) != 0) return -1;
					}
				}
				d->last_pulseMode = g->pulseModeEnable;
			}
		}
	}
		

	
	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"POW:LEV %2.2fdBm", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_power = g->outputPower;
	}
	// -----------------------------------------
    //    output on/on
	// -----------------------------------------
	if (d->last_on != g->rfOn) {
		if (GPIB_sendf (g->gpibAddress, 
						"OUTP:STAT %s", strOnOff[g->rfOn]) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_on = g->rfOn;
	}
	
	return 0;
}
//=======================================================================
//  END:    AgilentE4432B
//=======================================================================



//=======================================================================
//  BEGIN:   Agilent83751A
//=======================================================================
int GPIB_transmitFreq_Agilent83751A (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2)
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
		return 0;		// FIXME TODO why return here ??? REMOVED
	}
	// -----------------------------------------
	//     sweep parameters
	// -----------------------------------------
	if (GPIB_sendf (g->gpibAddress, 
					"FREQ:STAR %2.10f GHZ;STOP %2.10f GHZ", 
					g->startFreq / 1.0E9, g->stopFreq / 1E09) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}

	if (GPIB_sendf (g->gpibAddress, 
					"SWE:POIN %d", 
					seq->nRepetitions / g->nStepRepetition) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}
	
	// -----------------------------------------
	//     trigger mode
	// -----------------------------------------
	switch (g->stepTrigger) {
		case GPIB_COMMAND_FREQ_TRIGGER_GPIB:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR BUS") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
		case GPIB_COMMAND_FREQ_TRIGGER_EXT:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR EXT") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
	}
	

SET_OUTPUT:	

	if (d->hasPulseOption) {
		// -----------------------------------------
	    //   pulse mode
		// -----------------------------------------
		if (d->last_pulseMode != g->pulseModeEnable) {
			if (GPIB_sendf (g->gpibAddress, 
							"PULM:STAT %s", strOnOff[g->pulseModeEnable]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}

			if (g->pulseModeEnable) {
				if (GPIB_send (g->gpibAddress, 
								"PULM:SOUR EXT") != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}
			}
			d->last_pulseMode = g->pulseModeEnable;
		}
	}
	
	


// 	Frequency modulation : should work, but has to be tested!!
	
	if ((d->last_FM != g->enableFM) || (d->last_FMfreq_Dev != g->freqDeviation) || seq->firstRepetition) {
		if (GPIB_sendf (g->gpibAddress, 
						"FM:STAT %s", strOnOff[g->enableFM]) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		if (g->enableFM) {
			if (GPIB_sendf (g->gpibAddress, 
							"FM:SOUR EXT") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			if (GPIB_sendf (g->gpibAddress, 
							"FM:DEV %2.10f", g->freqDeviation) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
		}
		d->last_FM = g->enableFM;
		d->last_FMfreq_Dev = g->freqDeviation;
	}
	
	
	
//	AMPLITUDE MODULATION: should work, but has to be tested!!

//	if (d->hasAM) {
		// -----------------------------------------
	    //   Amplitude Modulations
		// -----------------------------------------
		if ((d->last_AM != g->enableAM) || (d->last_AMdepthPercent != g->AMdepthPercent) || seq->firstRepetition){
			if (GPIB_sendf (g->gpibAddress, 
							"AM:STAT %s", strOnOff[g->enableAM]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}

			if (g->enableAM) {
				if (GPIB_sendf (g->gpibAddress, 
								"AM:SOUR EXT;:AM %1.2fPCT", g->AMdepthPercent) != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}
				
			}
			d->last_AM = g->enableAM;
			d->last_AMdepthPercent = g->AMdepthPercent;
		}
//	}

	
	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"POW:LEV %2.2fdBm", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_power = g->outputPower;
	}
	// -----------------------------------------
    //    output on/on
	// -----------------------------------------
	if (d->last_on != g->rfOn) {
		if (GPIB_sendf (g->gpibAddress, 
						"OUTP:STAT %s", strOnOff[g->rfOn]) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_on = g->rfOn;
	}
	
	return 0;
}
//=======================================================================
//  END:    Agilent83751A
//=======================================================================
	



//=======================================================================
//  BEGIN:   Agilent5182B
//=======================================================================
// changes compared to  Agilent83751A:
// -  Agilent5182B has to ext mod input, so we need to label them EXT1 for AM and EXT2 form FM
int GPIB_transmitFreq_Agilent5182B (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2)
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
							"FREQ:CW %2.10fGHZ", freqHz / 1E9) != 0) 
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
		return 0;		// FIXME TODO why return here ??? REMOVED
	}
	// -----------------------------------------
	//     sweep parameters
	// -----------------------------------------
	if (GPIB_sendf (g->gpibAddress, 
					"FREQ:STAR %2.10f GHZ;STOP %2.10f GHZ", 
					g->startFreq / 1.0E9, g->stopFreq / 1E09) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}

	if (GPIB_sendf (g->gpibAddress, 
					"SWE:POIN %d", 
					seq->nRepetitions / g->nStepRepetition) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}
	
	// -----------------------------------------
	//     trigger mode
	// -----------------------------------------
	switch (g->stepTrigger) {
		case GPIB_COMMAND_FREQ_TRIGGER_GPIB:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR BUS") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
		case GPIB_COMMAND_FREQ_TRIGGER_EXT:
			if (GPIB_sendf (g->gpibAddress, "SWE:TRIG:SOUR EXT") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			break;
	}
	

SET_OUTPUT:	

	if (d->hasPulseOption) {
		// -----------------------------------------
	    //   pulse mode
		// -----------------------------------------
		if (d->last_pulseMode != g->pulseModeEnable) {
			if (GPIB_sendf (g->gpibAddress, 
							"PULM:STAT %s", strOnOff[g->pulseModeEnable]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}

			if (g->pulseModeEnable) {
				if (GPIB_send (g->gpibAddress, 
								"PULM:SOUR EXT") != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}
			}
			d->last_pulseMode = g->pulseModeEnable;
		}
	}
	
	


// 	Frequency modulation : should work, but has to be tested!!
	
	if ((d->last_FM != g->enableFM) || (d->last_FMfreq_Dev != g->freqDeviation) || seq->firstRepetition) {
		if (GPIB_sendf (g->gpibAddress, 
						"FM:STAT %s", strOnOff[g->enableFM]) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		if (g->enableFM) {
			if (GPIB_sendf (g->gpibAddress, 
							"FM:SOUR EXT2") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
			if (GPIB_sendf (g->gpibAddress, 
							"FM:DEV %2.10f", g->freqDeviation) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}
		}
		d->last_FM = g->enableFM;
		d->last_FMfreq_Dev = g->freqDeviation;
	}
	
	
	
//	AMPLITUDE MODULATION: should work, but has to be tested!!

//	if (d->hasAM) {
		// -----------------------------------------
	    //   Amplitude Modulations
		// -----------------------------------------
		if ((d->last_AM != g->enableAM) || (d->last_AMdepthPercent != g->AMdepthPercent) || seq->firstRepetition){
			if (GPIB_sendf (g->gpibAddress, 
							"AM:STAT %s", strOnOff[g->enableAM]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->askDeviceForErrors) {
				if (GPIB_getErr (g,1) != 0) return -1;
			}

			if (g->enableAM) {
				if (GPIB_sendf (g->gpibAddress, 
								"AM:SOUR EXT1;:AM %1.2fPCT", g->AMdepthPercent) != 0) 
					return GPIB_errorMessage(g->gpibAddress);
				if (g->askDeviceForErrors) {
					if (GPIB_getErr (g,1) != 0) return -1;
				}
				
			}
			d->last_AM = g->enableAM;
			d->last_AMdepthPercent = g->AMdepthPercent;
		}
//	}

	
	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"POW:LEV %2.2fdBm", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_power = g->outputPower;
	}
	// -----------------------------------------
    //    output on/on
	// -----------------------------------------
	if (d->last_on != g->rfOn) {
		if (GPIB_sendf (g->gpibAddress, 
						"OUTP:STAT %s", strOnOff[g->rfOn]) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_on = g->rfOn;
	}
	
	return 0;
}
//=======================================================================
//  END:    Agilent5182B
//=======================================================================
	






int GPIB_transmitWaveform_Agilent33220A (t_sequence *seq, t_gpibCommand *g, int dummy1, int dummy2)
{
    char help[50];

    t_gpibDevice *d;        
	double voltage;
	
	
	GPIB_open (g->gpibAddress, T10s);

	d = GPIBDEVICE_ptr (g->gpibAddress);

 	// clear device
 	
 	if (seq->repetition == 0) { // added as Agilent33220A has problems and the other agilent devices not
 		ibclr (d->handle);

		// reset error queue
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	}  
	

	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);
	}

	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
		if (GPIB_sendf (g->gpibAddress, "DISP:TEXT 'please wait...'") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	}
	
	
	
//	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;

	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) 
		|| (g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_INIT_AND_TRANSMIT)		
		|| (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
		


	if (g->duration_us == 0) g->duration_us = 1;
	sprintf (help, "FREQ %3.10f", 1 / (1E-6 * g->duration_us)); 
	if (GPIB_send (g->gpibAddress, help) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}

	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "BURS:STAT OFF") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			GPIB_getErr (g, 1);
		}
	
		if (GPIB_send (g->gpibAddress, "FUNC:SHAP DC") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	

	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
		
//		if (GPIB_sendf (g->gpibAddress, "OUTP:LOAD %s", agilentLoadStr(50)) != 0) 

		if (GPIB_sendf (g->gpibAddress, "OUTP:LOAD %s", agilentLoadStr(d->load)) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	
	

	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
		voltage = d->maxVoltage;
		if (voltage == 10) voltage = 9.999; 
		if (GPIB_sendf (g->gpibAddress, "VOLT:AMPL %1.3f;:VOLT:OFFS 0", voltage) != 0) 
	
//		if (GPIB_sendf (g->gpibAddress, "VOLT:AMPL 9.999;:VOLT:OFFS 0") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	//	if (g->askDeviceForErrors;) {
			if (GPIB_getErr (g,1) != 0) return -1;
	//	}

	}
		


/*	
    DebugPrintf ("%d data points\n", g->nPoints);
	for (i = 0; i < 40; i++) {
		DebugPrintf ("%05d: %06d\n", i, g->dataBuffer[i]);
	}
	for (i = max(0, g->nPoints-100); i < g->nPoints; i++) {
		DebugPrintf ("%05d: %06d\n", i, g->dataBuffer[i]);
	}
*/
	
	
 	if (GPIB_sendBinary (g->gpibAddress, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #",
 		g->dataBuffer, g->nPoints, 0) != 0) {
		return GPIB_errorMessage(g->gpibAddress);
	}
	if (g->askDeviceForErrors) {
		if (GPIB_getErr (g,1) != 0) return -1;
	}

//	if (GPIB_send (g->gpibAddress, "DATA:COPY MYWFM1, VOLATILE") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;


	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "BURS:NCYC 1") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	
	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "TRIG:SLOP POS;:TRIG:SOUR EXT") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}

	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "FUNC:USER VOLATILE") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "FUNC USER") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}

	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "BURS:STAT ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	

//	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) || (seq->firstRepetition)) {
	
		if (GPIB_sendf (g->gpibAddress, "DISP:TEXT 'GPIB#%d %s\nWfm: %s - %d'", g->gpibAddress, strMaxN (g->name,10), strMaxN (WFM_name (seq, g->waveform),10), seq->repetition+1) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
//	}
	
	if ((g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD) 
		|| (g->wfmTransferMode == GPIB_COMMAND_WFM_TRANSFERMODE_INIT_AND_TRANSMIT)		
		|| (seq->firstRepetition)) {
	
		if (GPIB_send (g->gpibAddress, "OUTP ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}

	return 0;
}
// END Agilent 33120A


/*
int GPIB_transmitWaveform_Agilent33220A (t_sequence *dummy, t_gpibCommand *g, int dummy2)
{
    char help[50];

    t_gpibDevice *d;        
	int i;


	GPIB_open (g->gpibAddress, T10s);

	d = GPIBDEVICE_ptr (g->gpibAddress);
	
	if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);

	
//	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	


	if (g->duration_us == 0) g->duration_us = 1;
	sprintf (help, "FREQ %3.10f", 1 / (1E-6 * g->duration_us)); 
	if (GPIB_send (g->gpibAddress, help) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	
	if (GPIB_send (g->gpibAddress, "BURS:STAT OFF") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	GPIB_getErr (g, 1);
	if (GPIB_send (g->gpibAddress, "FUNC:SHAP DC") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	
	if (GPIB_sendf (g->gpibAddress, "VOLT:AMPL 9.999") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_sendf (g->gpibAddress, "OUTP:LOAD %s", agilentLoadStr(d->load)) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

//	if (GPIB_send (g->gpibAddress, "VOLT:OFFS 0.0") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;

    DebugPrintf ("%d data points\n", g->nPoints);
	for (i = 0; i < 40; i++) {
		DebugPrintf ("%05d: %06d\n", i, g->dataBuffer[i]);
	}
	for (i = max(0, g->nPoints-100); i < g->nPoints; i++) {
		DebugPrintf ("%05d: %06d\n", i, g->dataBuffer[i]);
	}
	
 	if (GPIB_sendBinary (g->gpibAddress, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #",
 		g->dataBuffer, g->nPoints, 0) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "DATA:COPY MYWFM1, VOLATILE") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "BURS:NCYC 1") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "TRIG:SLOP POS;:TRIG:SOUR EXT") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "FUNC:USER MYWFM1") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "FUNC USER") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "BURS:STAT ON") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "OUTP ON") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	return 0;
}
// END Agilent 33120A
	
*/


int GPIB_transmitWaveform_Keithley3390 (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2)
{
    char help[50];

	GPIB_open (g->gpibAddress, T10s);
	if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);

//	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;

//	if (GPIB_send (g->gpibAddress, "BM:STAT OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	GPIB_getErr (g, 1);
	if (GPIB_send (g->gpibAddress, "FUNC:SHAP DC") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (g->duration_us == 0) g->duration_us = 1;
	sprintf (help, "FREQ %3.10f", 1 / (1E-6 * g->duration_us)); 
	if (GPIB_send (g->gpibAddress, help) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	

	if (GPIB_send (g->gpibAddress, "VOLT:AMPL 10.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "VOLT:OFFS 0.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

 	if (GPIB_sendBinary (g->gpibAddress, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #",
 		g->dataBuffer, g->nPoints, 0) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


//	if (GPIB_send (g->gpibAddress, "BM:NCYC 1") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "TRIG:SLOP POS;:TRIG:SOUR EXT") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "FUNC:USER VOLATILE;:FUNC:SHAP USER") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

//	if (GPIB_send (g->gpibAddress, "BM:STAT ON") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;
	

	return 0;
}
// END Agilent 33120A



int GPIB_transmitWaveform_Agilent33120A (t_sequence *dummy, t_gpibCommand *g, int dummy1, int dummy2)
{
    char help[50];

	GPIB_open (g->gpibAddress, T10s);
	if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);

//	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);
//	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "BM:STAT OFF") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	GPIB_getErr (g, 1);
	if (GPIB_send (g->gpibAddress, "FUNC:SHAP DC") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (g->duration_us == 0) g->duration_us = 1;
	sprintf (help, "FREQ %3.10f", 1 / (1E-6 * g->duration_us)); 
	if (GPIB_send (g->gpibAddress, help) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	

	if (GPIB_send (g->gpibAddress, "VOLT:AMPL 10.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "VOLT:OFFS 0.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

 	if (GPIB_sendBinary (g->gpibAddress, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #",
 		g->dataBuffer, g->nPoints, 0) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "BM:NCYC 1") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "TRIG:SLOP POS;:TRIG:SOUR EXT") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "FUNC:USER VOLATILE;:FUNC:SHAP USER") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "BM:STAT ON") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	

	return 0;
}
// END Agilent 33120A








int GPIB_transmitFreq_Agilent33250A (t_sequence *seq, t_gpibCommand *g, int dummy1, int dummy2)
{
//    char help[50];


   	double freq;
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
	
	// -----------------------------------------
    //    general settings 
	// -----------------------------------------
 	if (seq->repetition == 0) {
		if (GPIB_send (g->gpibAddress, "FUNC SIN;:BURS:STAT OFF") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
		// set output load to 50 Ohms, 
		if (GPIB_send (g->gpibAddress, "OUTP:LOAD 50") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
		// set power units to DBM
		if (GPIB_send (g->gpibAddress, "VOLT:UNIT DBM") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
	}
	
	// frequency feedback
	GPIB_getFeedbackInputValue (seq, g);

	if (!g->enableStep) {
		// -----------------------------------------
	    //   CW mode
		// -----------------------------------------
		freq = (g->stopFreq - g->startFreq) / 2 + g->startFreq + g->addFreqOffset * g->freqOffsetMHz * 1E6 + g->summedFeedbackFrequencyOffset;
		if (d->last_freq != freq) {  
			if (freq > d->maxFrequency) freq = d->maxFrequency;
			if (freq < d->minFrequency) freq = d->minFrequency;
			
			if (GPIB_sendf (g->gpibAddress, 
							"FREQ:CW %2.10fMHZ", freq/ 1E6) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
			d->last_freq = freq;
		}
		
		goto SET_OUTPUT; 
	}

	
 	
 	if (g->stepTrigger == GPIB_COMMAND_FREQ_TRIGGER_CALC) {
		// -----------------------------------------
		//   calculate frequency manually
		// -----------------------------------------
 		freq = GPIB_getStepFrequencyInHz (seq, g) + g->summedFeedbackFrequencyOffset;
		if (d->last_freq != freq) {  
			if (GPIB_sendf (g->gpibAddress, 
							"FREQ:CW %2.10fMHZ", freq / 1E6) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
			d->last_freq = freq;
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
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		return 0;
	}
	// -----------------------------------------
	//     sweep parameters
	// -----------------------------------------
	if (GPIB_sendf (g->gpibAddress, 
					"FREQ:STAR %2.10f MHZ;STOP %2.10f MHZ", 
					g->startFreq / 1.0E6, g->stopFreq / 1E6) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

/*
	if (GPIB_sendf (g->gpibAddress, 
					"SWE:POIN %d", 
					seq->nRepetitions / g->nStepRepetition) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
*/	
	// -----------------------------------------
	//     trigger mode
	// -----------------------------------------
	switch ((g->stepTrigger) && (seq->repetition == 0)) {
		case GPIB_COMMAND_FREQ_TRIGGER_GPIB:
			if (GPIB_sendf (g->gpibAddress, "TRIG:SOUR BUS") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			
			if (GPIB_getErr (g,1) != 0) return -1;
			break;
		case GPIB_COMMAND_FREQ_TRIGGER_EXT:
			if (GPIB_sendf (g->gpibAddress, "TRIG:SOUR EXT") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
			break;
	}
	

SET_OUTPUT:	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------


	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"VOLT:UNIT DBM;:VOLT %2.3f", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
		d->last_power = g->outputPower;
	}
	
	if (d->last_pulseMode != g->pulseModeEnable) {
		if (g->pulseModeEnable) {
			if (GPIB_sendf (g->gpibAddress, 
						"AM:STAT OFF;:BURS:MODE GAT;:BURS:STAT ON") != 0) 				// ;:BURS:NCYC INF
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		else if (g->pulseModeEnable) {
			if (GPIB_sendf (g->gpibAddress, 
							"BURS:STAT OFF") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_pulseMode = g->pulseModeEnable;
	}
		

	
	if ((d->last_AM != g->enableAM) || (d->last_AMdepthPercent != g->AMdepthPercent) || seq->firstRepetition) {
		if (GPIB_getErr (g,1) != 0) return -1;
		if (g->enableAM) {
			if (GPIB_sendf (g->gpibAddress, 
							"BURS:STAT OFF;:AM:STAT OFF;:AM:SOUR EXT;:AM:DEPT %d", g->AMdepthPercent) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		}
		else { 		
			if (GPIB_sendf (g->gpibAddress, 
						"AM:STAT OFF") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		}

		d->last_AM = g->enableAM;
		d->last_AMdepthPercent = g->AMdepthPercent;
	}
	
	
	if ((d->last_FM != g->enableFM) || (d->last_FMfreq_Dev != g->freqDeviation) || seq->firstRepetition) {
		if (GPIB_sendf (g->gpibAddress, 
						"FM:STAT %s", strOnOff[g->enableFM]) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
		if (g->enableFM) {
			if (GPIB_sendf (g->gpibAddress, 
							"FM:SOUR EXT") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
			if (GPIB_sendf (g->gpibAddress, 
							"FM:DEV %2.10f", g->freqDeviation * 5.0) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_FM = g->enableFM;
		d->last_FMfreq_Dev = g->freqDeviation;
	}
	// -----------------------------------------
    //    output on/on
	// -----------------------------------------
	if (d->last_on != g->rfOn) {
		if (GPIB_sendf (g->gpibAddress, 
						"OUTP %s", strOnOff[g->rfOn]) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_getErr (g,1) != 0) return -1;
		d->last_on = g->rfOn;
	}
	
	return 0;


}





int GPIB_transmitWaveform_Agilent33250A (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2)
{
    char help[50];
    t_gpibDevice *d;
    
   	d = g->device;

	GPIB_open (g->gpibAddress, T30s);
	if (s->repetition == 0) {
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);
	}

//	if (GPIB_send (g->gpibAddress, "OUTP OFF") != 0) 
//		return GPIB_errorMessage(g->gpibAddress);

	
	if (GPIB_send (g->gpibAddress, "BURS:STAT OFF") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "FUNC DC") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "DATA:DEL:ALL") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	
/*	if (GPIB_sendf (g->gpibAddress, "VOLT:UNIT VPP;:VOLT:OFFS 0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
*/
	if (GPIB_sendf (g->gpibAddress, "VOLT:UNIT VPP;:VOLT 0.1;:OUTP:LOAD %s", d->load == LOAD_OPEN ? "INF" : intToStr (d->load)) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_sendf (g->gpibAddress, "VOLT:OFFS 0;:VOLT %1.3f", d->maxVoltage-d->minVoltage) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	
	sprintf (help, "PULS:PER %2.10f", 1E-6 * g->duration_us); 
	if (GPIB_send (g->gpibAddress, help) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	
/*
	if (GPIB_send (g->gpibAddress, "VOLT:LOW -5.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "VOLT:HIGH 5.0") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
*/
/*
    sprintf (help, "%d", g->nPoints*2);
	sprintf (dataStr, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #%d%s", strlen(help), help);
    len = strlen(dataStr);
    startPos = dataStr + len;
	memcpy (startPos, g->dataBuffer, g->nPoints*2);
*/ 	if (GPIB_sendBinary (g->gpibAddress, "FORM:BORD SWAP;:DATA:DAC VOLATILE, #",
 						 g->dataBuffer, g->nPoints, 0) != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "BURS:MODE TRIG;:BURS:NCYC 1") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;
	if (GPIB_send (g->gpibAddress, "TRIG:SLOP POS;:TRIG:SOUR EXT") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;


	if (GPIB_send (g->gpibAddress, "FUNC:USER VOLATILE;:FUNC USER") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	if (GPIB_send (g->gpibAddress, "BURS:STAT ON;:OUTP ON") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_getErr (g,1) != 0) return -1;

	
	return 0;
       
}
// END Agilent 33250A


// ============================================================
//
//	 power supply Agilent 6612C
//
// ============================================================

int GPIB_transmitCommand_6612C (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2)
{
    t_gpibDevice *d;
    
   	d = g->device;

	GPIB_open (g->gpibAddress, T30s);
	if (s->repetition == 0) {
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, "OUTP ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	
	// transmit max. current
	if (d->last_current[0] != g->current) {
		if (GPIB_sendf (g->gpibAddress, "CURR %2.6f", g->current) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_current[0] = g->current;
	}
	
	// transmit voltage
	if (d->last_voltage[0] != g->voltage) {
		if (GPIB_sendf (g->gpibAddress, "VOLT %2.5f", g->voltage) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_voltage[0] = g->voltage;
	}



//	tprintf ("WARNING: function 'GPIB_transmitCommand_6612C' not implemented.\n");
	return 0;
}








// ============================================================
//
//	 power supply Agilent E3643A
//
// ============================================================

int GPIB_transmitCommand_E3643A (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2)
{
    t_gpibDevice *d;
    
   	d = g->device;

	GPIB_open (g->gpibAddress, T30s);
	if (s->repetition == 0) {
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, "OUTP ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	}
	
	// transmit max. current
	if (d->last_current[0] != g->current) {
		if (GPIB_sendf (g->gpibAddress, "CURR %2.6f", g->current) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_current[0] = g->current;
	}
	
	// transmit voltage
	if (d->last_voltage[0] != g->voltage) {
		if (GPIB_sendf (g->gpibAddress, "VOLT %2.5f", g->voltage) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_voltage[0] = g->voltage;
	}



//	tprintf ("WARNING: function 'GPIB_transmitCommand_6612C' not implemented.\n");
	return 0;
}

