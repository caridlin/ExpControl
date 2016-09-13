#include <windows.h>  // for Sleep
#include <utility.h>
#include <gpib.h>

#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"





int GPIB_transmitCommand_BNC575 (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2);



//const char *strOnOff[2] = {"OFF", "ON"}; 


//  =========================================================
//
//            BNC 575   
//
//  =========================================================

t_gpibDevice *GPIB_deviceParameters_BNC575 (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

    	strcpy (dev->strIDN, "BNC");
		dev->commandType1 	= GPIB_COMMANDTYPE_PULSES;
		dev->errorMessageCompatibleWithAgilent = 0;

		// initialize pointer to functions			
		//dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent83751A;
	}
	return dev;
}


// ============================================================
//
//	 pulse generator BNC 575
//
// ============================================================

int GPIB_transmitCommand_BNC575 (t_sequence *s, t_gpibCommand *g, int dummy, int dummy2)
{
    t_gpibDevice *d;
	int i;
    double tmp;
   	d = g->device;

	GPIB_open (g->gpibAddress, T10s);
	
	d = GPIBDEVICE_ptr (g->gpibAddress);

 	// clear device
 	
 	if (s->repetition == 0) {
 		ibclr (d->handle);

		// reset error queue
		//if (GPIB_send (g->gpibAddress, "*CLS") != 0) {
		//	return GPIB_errorMessage(g->gpibAddress);
		//}
	}
	
		if (GPIB_send (g->gpibAddress, "*RST") != 0) GPIB_errorMessage(g->gpibAddress);
		if (g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
		for (i=0;i<GPIB_COMMAND_PULSES_CHANNELS;i++){
			
			Sleep(5);
			if (GPIB_sendf (g->gpibAddress, ":PULS%d:STAT ON", i+1) != 0) { 
				return GPIB_errorMessage(g->gpibAddress);
			}
			Sleep(5);
			if (GPIB_sendf (g->gpibAddress, ":PULSE%d:POL NORM", i+1) != 0) { 
				return GPIB_errorMessage(g->gpibAddress);
			}
			Sleep(5);
			if (!g->stepPulseDelay[i]) {
				tmp = g->startPulseDelay[i]; 
			} else{
				tmp = g->startPulseDelay[i]+(g->stopPulseDelay[i]-g->startPulseDelay[i])/(s->nRepetitions-1)*(s->repetition);
			}
			if (GPIB_sendf (g->gpibAddress, ":PULS%d:DEL %e", i+1 , tmp) != 0) {
				return GPIB_errorMessage(g->gpibAddress);
			}
			Sleep(5); 
			
			if (!g->stepPulseLength[i]) {
				tmp = g->startPulseLength[i];
			} else {
				tmp = g->startPulseLength[i]+(g->stopPulseLength[i]-g->startPulseLength[i])/(s->nRepetitions-1)*(s->repetition);

			}
			if (GPIB_sendf (g->gpibAddress, ":PULS%d:WIDT %e", i+1 , tmp) != 0) {
				return GPIB_errorMessage(g->gpibAddress);
			}
		}
		if (GPIB_send (g->gpibAddress, ":PULS0:MODE SING") != 0) 
			return GPIB_errorMessage(g->gpibAddress);	 
		//if (GPIB_send (g->gpibAddress, ":PULS0:GAT:MOD PULS") != 0) 
		//	return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, ":PULS0:TRIG:MODE TRIG") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, ":PULS0:TRIG:LEV 2.5") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, ":PULS0:TRIG:EDGE RIS") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, ":PULS0:STATE ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		if (GPIB_send (g->gpibAddress, ":INST:STATE ON") != 0) 
			return GPIB_errorMessage(g->gpibAddress);		
		if (1 || g->askDeviceForErrors) {
			if (GPIB_getErr (g,1) != 0) return -1;
		}
	return 0;
}

