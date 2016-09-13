#include <gpib.h>
#include <utility.h>
#include <tools.h>

#include "CONTROL_DataStructure.h"    

#include "CONTROL_HARDWARE_GPIB.h"
#include "GPIB_DRIVER_LeCroy.h"

#ifdef EXPERIMENTCONTROL
int GPIB_transmitWaveform_LeCroy (t_sequence *seq, t_gpibCommand *g, int transmitData, int dummy2);
#endif

t_gpibDevice *GPIB_deviceParameters_LeCroyLW120  (void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		dev->type           = GPIBDEV_LECROY_LW120;
    	strcpy (dev->strIDN, "LeCroy,LW120");
		dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
		dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
		dev->wfmMinPoints   = 4;
		dev->wfmMaxPoints   = LECROY_MAX_POINTS;
		dev->wfmPointsMultiple = 4;  // nPoints has to be an integer multiple of 4
	
		dev->maxVoltage     =  10.0;
		dev->minVoltage     = -dev->maxVoltage;
		dev->minDigital     = 0;
		dev->maxDigital		= 16383;
		dev->minDuration_us = 0.200;
		dev->nChannels		= 2;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		// initialize pointer to functions			
#ifdef EXPERIMENTCONTROL
		dev->GPIB_transmitWaveform  = GPIB_transmitWaveform_LeCroy;
#endif
	}
	return dev;
}




t_digitizeParameters *digitizeParameters_LeCroyLW120 (int channel, unsigned long timebase)
{
	static t_digitizeParameters p;
	t_gpibDevice dev;
	t_gpibDevice *dev2;

	dev.type = GPIBDEV_LECROY_LW120;
	dev = *GPIB_deviceParameters_LeCroyLW120 ();
	
	if (channel > 0) {
		dev2 = GPIBDEVICE_ptrChannel (channel);
		if (dev2 != NULL) dev.maxVoltage = dev2->maxVoltage;
	}
	
	p.maxVoltage =  dev.maxVoltage;
	p.minVoltage 	= -p.maxVoltage;
	p.Vpp 			= p.maxVoltage - p.minVoltage;
	p.minDigital 	= dev.minDigital;
	p.maxDigital 	= dev.maxDigital;
	p.Dpp 			= p.maxDigital - p.minDigital;
	p.nPoints 		= 0;
	p.duration_us 	= dev.minDuration_us;
	p.timebase_50ns	= timebase;
	return &p;	
}




#ifdef EXPERIMENTCONTROL

const char *strIntExt[2] = {"INT", "EXT"};




int GPIB_transmitWaveform_LeCroy (t_sequence *seq, t_gpibCommand *g, int transmitData, int dummy2)
{
/*	double delay = 40; // ms
	int clsSent = 0;
	int instSelSent = 0;
	t_gpibDevice *d;
	int nErrors = 0;
	clock_t time;
	int error;
	int MAX_ERRORS = 3;
	int showMessage;


  	if (g->nPoints == 0) return 0;

	
	
	g->nPoints += xMod (g->nPoints, 4);
//bufferMinMax (g->dataBuffer, g->nPoints);

START:
	d = g->device;
	showMessage = nErrors >= MAX_ERRORS;
	
	if (d->handle > 0) {
		ibstop (d->handle);
		if (ibstaCheck (g->gpibAddress, "ibstop")  != 0) goto ERROR;
	}
		
		GPIB_open (g->gpibAddress, T3s);
		if (d->handle < 0) {
			if (ibstaCheck (g->gpibAddress, "GPIB_open (ibdev)")  != 0) goto ERROR;
		}
		
		error = ibonl (d->handle, 1);
		if (ibstaCheck (g->gpibAddress, "ibonl")  != 0) goto ERROR;

		error = ibclr (d->handle);
		if (ibstaCheck (g->gpibAddress, "ibclr")  != 0) goto ERROR;

		GPIB_send (g->gpibAddress, "*CLS");
	Sleep (delay);
//	if (GPIB_send (g->gpibAddress, "*CLS") != 0) goto ERROR;
	clsSent = 1;
// 	Sleep (delay);

    if (g->channel > 1) g->channel = 1;

	if (GPIB_sendf (g->gpibAddress, "INST:SEL %d;:INIT:CONT OFF", g->channel+1) != 0) goto ERROR;
	instSelSent = 1;
 	Sleep (2*delay);
	if (GPIB_getErr (g, showMessage) != 0) goto ERROR;


//	if (d->last_nCopies[g->channel] != g->nCopies) {
		if (GPIB_sendf (g->gpibAddress, "TRIG:BURS ON;COUN %d;SLOP POS;:TRIG:SOUR:ADV EXT", g->nCopies) != 0) goto ERROR;
		// rememberLastValue
		d->last_nCopies[g->channel] = g->nCopies;
	 	Sleep (delay);
		if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
//	}


//	if (d->last_externalSCLK != d->externalSCLK) {
		if (GPIB_sendf (g->gpibAddress, "FREQ:RAST:SOUR %s", strIntExt[d->externalSCLK]) != 0) goto ERROR;
	    d->last_externalSCLK = d->externalSCLK;
	 	Sleep (delay);
		if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
//	}
			
//LECROY_AmplitudeVolts		
//	if ((d->last_frequencySCLK != d->frequencySCLK) || (d->maxVoltage != d->last_maxVoltage)) {
		if (GPIB_sendf (g->gpibAddress, "APPL:USER 1,%1.0f,%1.3f, 0", d->frequencySCLK, d->maxVoltage) != 0) goto ERROR;
		d->last_frequencySCLK = d->frequencySCLK;
		d->last_maxVoltage    = d->maxVoltage;
	 	Sleep (delay);
		if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
//	}


//	if (transmitWaveform) {
	if (!instSelSent) {
		if (GPIB_sendf (g->gpibAddress, "INST:SEL %d", g->channel+1) != 0) goto ERROR;
		instSelSent = 1;
	 	Sleep (delay);
		if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
	}

	if (transmitData) {
		if (GPIB_sendf (g->gpibAddress, "TRAC:DEF 1, %ld;:TRAC:SEL 1;:FORM:BORD SWAP", g->nPoints) != 0) goto ERROR;
		Sleep (delay);
	    if (GPIB_getErr (g, showMessage) != 0) goto ERROR;

		if (g->nPoints > 25000) {
//		 	time = clock();
			// -----------------------------------------------
			//   use DMA transfer if #points > 30000 
			// -----------------------------------------------
			if (GPIB_send (g->gpibAddress, "*OPC?") != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (GPIB_receive (g->gpibAddress, NULL, 0) != 0) goto ERROR;
			Sleep (delay);
	//		GPIB_getErr (g,1);

			if (GPIB_send (g->gpibAddress, "DMA ON") != 0) goto ERROR;
			Sleep (100);

		// DMA transfer 
		 	if (GPIB_sendBinary (g->gpibAddress, NULL, g->dataBuffer, g->nPoints, 1) != 0) goto ERROR;
			Sleep (delay);
			if (GPIB_getErr (g, showMessage) != 0) goto ERROR;

//			tprintf ("duration: %d ms\n", timeStop (time));

		//ibstop (dPtr->handle);
		}
		else {
			//  standard transfer 
		 	if (GPIB_sendBinary (g->gpibAddress, "TRAC#", g->dataBuffer, g->nPoints, 0) != 0) goto ERROR;
			Sleep (delay);
			if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
		}
	}
//	}
	
 	Sleep (delay);
	
	if (GPIB_send (g->gpibAddress, "OUTP ON") != 0) goto ERROR;
 	Sleep (delay);
	if (GPIB_getErr (g, showMessage) != 0) goto ERROR;
	
	d->last_triggerSlope = d->triggerSlope;
	d->last_divideFreq[g->channel] = d->divideFreq[g->channel];
//	d->last_externalSCLK = d->externalSCLK;
	d->last_triggerSignal = d->triggerSignal;
	return 0;
	
ERROR:
    GPIBDEVICE_resetLastValues (d, 1);
    nErrors ++;
	GPIB_close (g->gpibAddress);
//	ibstop (d->handle);
//	d->handle = -1;
    if (nErrors <= MAX_ERRORS) {
   	    pprintf (GPIB_outputPanel(), GPIB_outputCtrl(), "\nError occured, retry command (%d).", nErrors);
    	goto START;
    }
    return GPIB_errorMessage(g->gpibAddress); 
    
ABORT:
    GPIBDEVICE_resetLastValues (d, 1);
 */	return -1;
 
	
	}
//=======================================================================
//    END  LeCroy
//=======================================================================

#endif





