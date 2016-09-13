#include <utility.h>
#include <gpib.h>
#include <formatio.h>

#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"


int GPIB_transmitCommand_PL330DP (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2);



t_gpibDevice *GPIB_deviceParameters_ThurlbyThandarPL330DP(void)
{
	static t_gpibDevice *dev = NULL;
	
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		strcpy (dev->strIDN, "THURLBY-THANDAR,PL330DP");
		dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
		dev->minVoltage = 0;
		dev->maxVoltage = 32.0;
		dev->maxCurrent = 3;
		dev->nChannels = 2;
		
		// initialize pointer to functions			
		dev->GPIB_transmitPowerSupply = GPIB_transmitCommand_PL330DP;
	}
	return dev;
}



int GPIB_transmitCommand_PL330DP (t_sequence *s, t_gpibCommand *g, int dummy1, int dummy2)
{
    t_gpibDevice *d;
    
   	d = g->device;


	GPIB_open (g->gpibAddress, T30s);

	if (s->repetition == 0) {
		if (GPIB_sendf (g->gpibAddress, "OP%d 1", g->channel) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
	}
	// transmit max. current
	if (d->last_current[g->channel-1] != g->current) {
		if (GPIB_sendf (g->gpibAddress, "I%d %1.3f", g->channel, g->current) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_current[g->channel-1] = g->current;
	}
	
	// transmit voltage
	if (d->last_voltage[g->channel-1] != g->voltage) {
		if (GPIB_sendf (g->gpibAddress, "V%d %1.2f", g->channel, g->voltage) != 0) 
			return GPIB_errorMessage(g->gpibAddress);
		d->last_voltage[g->channel-1] = g->voltage;
	}
		
	
//	tprintf ("WARNING: function 'GPIB_transmitCommand_PL330DP ' not implemented.\n");
	return 0;
}



int GPIB_transmitVoltage_PL330DP (int gpibAddress, double voltage, double current, int channel, int switchOn)
{

	GPIB_open (gpibAddress, T30s);

	if (switchOn) {
		if (GPIB_sendf (gpibAddress, "OP%d 1", channel) != 0) 
			return GPIB_errorMessage(gpibAddress);
		// transmit max. current
		if (GPIB_sendf (gpibAddress, "I%d %1.3f", channel, current) != 0) 
				return GPIB_errorMessage(gpibAddress);
	}
	// transmit voltage
	if (GPIB_sendf (gpibAddress, "V%d %1.2f", channel, voltage) != 0) 
		return GPIB_errorMessage(gpibAddress);
		
	
//	tprintf ("WARNING: function 'GPIB_transmitCommand_PL330DP ' not implemented.\n");
	return 0;
}




void test_tplp (void) 
{
	char str[200];
	int dev;
	int brd;
	
	DisableBreakOnLibraryErrors ();
	brd = ibfind ("gpib0");
	brd = ibfind ("gpib1");
	brd = ibfind ("gpib2");
	
//	dev = OpenDev ("", "tt1906");
	dev = ibdev (1, 12, NO_SAD, T10s, 1, 0);
/*	Fmt (str, "*IDN?\012");
	ibwrt (dev, str, NumFmtdBytes ());
	ibrd (dev, str, 50);
	DebugPrintf (str);
	Sleep (100);
*/	Fmt (str, "OPALL 1\012");
	ibwrt (dev, str, NumFmtdBytes ());
	Fmt (str, "V1 10.2\012");
	ibwrt (dev, str, NumFmtdBytes ());
//	ibrd (dev, str, 50);
	DebugPrintf (str);
/*	Fmt (str, "V1?\012");
	ibwrt (dev, str, NumFmtdBytes ());
	ibrd (dev, str, 50);
	DebugPrintf (str);
	CloseDev (dev);
*/}   

