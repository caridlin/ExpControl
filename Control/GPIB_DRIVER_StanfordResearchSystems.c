#include <windows.h> // for Sleep  
#include <utility.h>
#include <gpib.h>

#include "tools.h"  

#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"

#include "GPIB_DRIVER_StanfordResearchSystems.h"
//#include "GPIB_DRIVER_Agilent.h"

#define MAX_RECEIVESTR_LEN 30



int GPIB_transmitCommand_Stanford (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2);
int GPIB_getSettings_Stanford (t_gpibCommand *g);
int GPIB_waitUntilStabilized_Stanford (t_gpibCommand *g);

//const char *strOnOff[2];// = {"OFF", "ON"}; 


t_gpibDevice *GPIB_deviceParameters_SRS_PS350(void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		strcpy (dev->strIDN, "StanfordResearchSystems,PS350");
		dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
		dev->minVoltage = 0.0;
		dev->maxVoltage = 5000.0;
		dev->maxCurrent = 5.25E-3;
		
		// initialize pointer to functions			
		dev->GPIB_transmitPowerSupply = GPIB_transmitCommand_Stanford;
		dev->GPIB_getDeviceSettings   = GPIB_getSettings_Stanford;
		dev->GPIB_waitUntilStabilized = GPIB_waitUntilStabilized_Stanford;
	}
	return dev;
}




int GPIB_transmitCommand_Stanford (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2)
{
	char receiveStr[MAX_RECEIVESTR_LEN];
    t_gpibDevice *d;
    int HV_on;
    
   	d = g->device;

	GPIB_open (g->gpibAddress, T30s);

	if (s->repetition == 0) {
		if (GPIB_send (g->gpibAddress, "*CLS") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, "HVON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		// check if HV could be switched on
		if (GPIB_send (g->gpibAddress, "*STB? 7") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_receive (g->gpibAddress, receiveStr, MAX_RECEIVESTR_LEN) != 0)
			return GPIB_errorMessage(g->gpibAddress);
		HV_on = strtol (receiveStr, NULL, 10);
		if (!HV_on) {		
			PostMessagePopupf ("GPIB command error", 
						  "GPIB command '%s', device '%s', GPIB address %d.\n\n"
					      "Please switch on the high voltage.\n\n"
					      "Sequence will be aborted.",
					       g->name, d->name, g->gpibAddress);
			return -1;
		}		
	}

	// transmit max. current
	if (d->last_current[0] != g->current) {
		if (GPIB_sendf (g->gpibAddress, "ILIM %2.6f", g->current) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_current[0] = g->current;
	}
	
	// transmit voltage
	if (d->last_voltage[0] != g->voltage) {
		if (GPIB_sendf (g->gpibAddress, "VSET %7.1f", g->voltage) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_voltage[0] = g->voltage;
	}

	return 0;
}




int GPIB_getSettings_Stanford (t_gpibCommand *g)
{
	char receiveStr[MAX_RECEIVESTR_LEN];
	double vLimit;
    t_gpibDevice *d;
    
   	d = g->device;
	
	if (d->settingsRecalledFromDevice) return 0;
	
	GPIB_open (g->gpibAddress, T30s);
	// get voltage limit from device
	if (GPIB_send (g->gpibAddress, "VLIM?") != 0) 
		return GPIB_errorMessage(g->gpibAddress);
	if (GPIB_receive (g->gpibAddress, receiveStr, MAX_RECEIVESTR_LEN) != 0)
		return GPIB_errorMessage(g->gpibAddress);
	vLimit = strtod (receiveStr, NULL);
	if (errno == ERANGE) return GPIB_ERR_GETSETTINGS;
	if (vLimit >= 0) {
		d->maxVoltage = vLimit;
		d->minVoltage = 0;
	}
	else {
		d->minVoltage = vLimit;
		d->maxVoltage = 0;
	}
	
	d->settingsRecalledFromDevice = 1;
	return 0;
}




int GPIB_waitUntilStabilized_Stanford (t_gpibCommand *g)
{
	#define MAX_RECEIVESTR_LEN 30
	#define MAX_TESTS 25
	char receiveStr[MAX_RECEIVESTR_LEN];
	int stable = 0;
	double vOut;
	double lastVOut;
	int i;
    t_gpibDevice *d;
    
   	d = g->device;
	
	if (!g->waitUntilStabilized) return 0;
	
	GPIB_open (g->gpibAddress, T30s);
	// get voltage limit from device
	
	i = 0;
	lastVOut = g->voltage;
	while ((!stable) && (i < MAX_TESTS)) {
		if (GPIB_send (g->gpibAddress, "VOUT?") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_receive (g->gpibAddress, receiveStr, MAX_RECEIVESTR_LEN) != 0)
			return GPIB_errorMessage(g->gpibAddress);
		vOut = strtod (receiveStr, NULL);
		stable = (fabs (vOut - lastVOut) < 2);
		if (!stable) {
			Sleep ((int)(1000*(i * 0.05 + 0.1)));
			i++;
		}
		lastVOut = vOut;
	}
	return 0;
}

//  ========================================================= 
// BEGIN SG384 synthesizer
//  =========================================================

int GPIB_fetchFreq_SRS_SG384 (t_gpibCommand *g,double freqHz)
{
	char currFreqStr[100]; 
	double currentFreq;
	double stepsize=50e3; //50kHz steps
	double sendfreq;
	int num_steps;
	int k;
	k=0;
	
	if (GPIB_send(g->gpibAddress,"FREQ?")!=0)
		return(GPIB_errorMessage(g->gpibAddress));
	GPIB_receive (g->gpibAddress, currFreqStr, 100);
	tprintf(currFreqStr);
	currentFreq = strtod (currFreqStr, NULL);
		
	num_steps=(int)RoundRealToNearestInteger (fabs(freqHz-currentFreq)/stepsize);
	if (num_steps!=0)
	{
		while  (k<num_steps+1){
			sendfreq=currentFreq+(freqHz-currentFreq)/num_steps*k;
			Sleep(5);
			if (GPIB_sendf (g->gpibAddress, 
								"FREQ %2.5fMHz", sendfreq / 1E6) != 0)
				return(GPIB_errorMessage(g->gpibAddress));
			k++;
		}
	}	
	Sleep(5); 
	return 0;
}	

int GPIB_transmitFreq_SRS_SG384 (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2)
{
    double freqHz;
    t_gpibDevice *d;  
	double amplPower;

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
			if (g->fetchFreq)
			{   GPIB_fetchFreq_SRS_SG384 (g,freqHz);
			}			
			if (GPIB_sendf (g->gpibAddress, 
							"FREQ %2.10fMHz", freqHz / 1E6) != 0) 
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
			if (g->fetchFreq)
			{   GPIB_fetchFreq_SRS_SG384 (g,freqHz);
			}
			if (GPIB_sendf (g->gpibAddress, 
							"FREQ %2.10fMHz", freqHz / 1E6) != 0) 
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
	/*if (GPIB_sendf (g->gpibAddress, 
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
	}	 */
	
	// -----------------------------------------
	//     trigger mode
	// -----------------------------------------
	/*switch (g->stepTrigger) {
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
	}*/
	

SET_OUTPUT:	

	if (d->hasPulseOption) {
		// -----------------------------------------
	    //   pulse mode
		// -----------------------------------------
	/*	if (d->last_pulseMode != g->pulseModeEnable) {
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
		}*/
	}
	
	
	
	
//	AMPLITUDE MODULATION: should work, but has to be tested!!

//	if (d->hasAM) {
		// -----------------------------------------
	    //   Amplitude Modulations
		// -----------------------------------------
/*		if ((d->last_AM != g->enableAM) || (d->last_AMdepthPercent != g->AMdepthPercent) || seq->firstRepetition){
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
		} */
//	}

	
	
	// -----------------------------------------
    //    power level 
	// -----------------------------------------
	if (d->last_power != g->outputPower) {
		if (GPIB_sendf (g->gpibAddress, 
						"AMPH %2.2f", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_sendf (g->gpibAddress, 
						"AMPR %2.2f", g->outputPower) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		
		amplPower = g->outputPower;
		if (amplPower > 14.96) {
			amplPower = 14.96;
			tprintf("SRS384 max output power for AMPL 14.96 dbm\n");
		}
		if (GPIB_sendf (g->gpibAddress, 
						"AMPL %2.2f", amplPower) != 0) 
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
						"ENBH %d", g->rfOn) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_sendf (g->gpibAddress, 
						"ENBR %d", g->rfOn) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_sendf (g->gpibAddress, 
						"ENBL %d", g->rfOn) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		d->last_on = g->rfOn;
	}
	
	return 0;
}


t_gpibDevice *GPIB_deviceParameters_SRS_SG384(void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "Stanford Research Systems,SG384");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 0;
		
		dev->hasPulseOption = 0;

		dev->minFrequency   = 950E3;
		dev->maxFrequency   = 4.05E9;
		dev->minAmplitude   = -110;
		dev->maxAmplitude   = +16;
		
		dev->hasAM = 0;
		dev->minAMdepthPercent = 0.1;

		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_SRS_SG384;
	}
	return dev;
}



//  ========================================================= 
// END SG384
//  ========================================================= 
