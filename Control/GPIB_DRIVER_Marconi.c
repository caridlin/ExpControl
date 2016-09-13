
#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE_GPIB.h"

#include "GPIB_DRIVER_Agilent.h" 




t_gpibDevice *GPIB_deviceParameters_Marconi2030 (void)
{
	//static t_gpibDevice *dev = NULL;

	return NULL;
		 /*
	if (dev == NULL) {
    	dev = (t_gpibDevice *) malloc (sizeof (t_gpibDevice));
		GPIBDEVICE_init (dev);

		NOT YET IMPLEMENTED
    	
    	strcpy (dev->strIDN, "MARCONI INSTRUMENTS LIMITED,2030");
    	strcpy (dev->strIDN2, "MARCONI INSTRUMENTS,2030");
		dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
		dev->minFrequency   = 10.0E3;
		dev->maxFrequency   = 1.35E9;
		dev->minAmplitude   = -40;
		dev->maxAmplitude   = +15;
		dev->enablePulseMode = 0;
		dev->enableExternalTrigger = 1;
		dev->maxDivide	    = 1;
		dev->errorMessageCompatibleWithAgilent = 1;
		
		// initialize pointer to functions			
		dev->GPIB_transmitFrequency = GPIB_transmitFreq_Agilent83751A;

	}
	return dev; */
}


/*                                                                           */
/* October 22, 1992:    Changed char rf_unit_type[3] to char                 */
/*                      rf_unit_type[4] in the function                      */
/*                      mi2030_setup_carrier, to resolve an                  */
/*                      "attempt to write beyond end of string" error.       */
/*                      Because of a change to the MI2030 firmware the       */
/*                      following   lines of the mi2030_init function        */
/*                      were changed.                                        */
/*                      FROM: Scan (cmd, "MARCONI INSTRUMENTS LIMITED,2030") */
/*                                if (NumFmtdBytes () != 32)  {              */
/*                      TO:   Scan (cmd, "MARCONI INSTRUMENTS");             */
/*                                if (NumFmtdBytes () != 18)  {              */
/*                                                                           */
/*                      Modified by:    JRO, LWILDP, NI, Austin, Texas       */
/*                                                                           */


/*
/* ========================================================================= */
/*  This function will configure the carrier signal of the generator,        */
/*  setting the frequency, phase, and RF level of the signal.                */
/* ========================================================================= */
/*
void mi2030_setup_carrier (int instrID, double value, int f_unit, double phase, 
                            double rf_value, int rf_unit, int on_off, 
                              double offset, int offset_on)
{
    char rf_unit_type[4];

    if (mi2030_device_closed (instrID) != 0)
        return;
    Breakpoint ();
    if (mi2030_invalid_integer_range (instrID, 1, mi2030_MAX_INSTR,  -1) != 0)
        return;
    if (mi2030_invalid_integer_range (f_unit, 0, 3,  -3) != 0)
        return;
    switch (f_unit)  {
    case 0:
        if (mi2030_invalid_real_range (value, 10000.0, 1350000000.0,  -2) != 0)
            return;
        break;
    case 1:
        if (mi2030_invalid_real_range (value, 10.0, 1350000.0,  -2) != 0)
            return;
        break;
    case 2:
        if (mi2030_invalid_real_range (value, 0.01, 1350.0,  -2) != 0)
            return;
        break;
    case 3:
        if (mi2030_invalid_real_range (value, 0.00001, 1.35,  -2) != 0)
            return;
        break;
    }
    if (mi2030_invalid_real_range (phase, -999.0, 999.0,  -4) != 0)
        return;
    if (mi2030_invalid_real_range (rf_value, rf_low[rf_unit], rf_high[rf_unit],  -5) != 0)
        return;
    if (mi2030_invalid_integer_range (rf_unit, 0, 6,  -6) != 0)
        return;
    if (mi2030_invalid_integer_range (on_off, 0, 1,  -7) != 0)
        return;
    if (mi2030_invalid_real_range (offset, -2.0, 2.0,  -8) != 0)
        return;
    if (mi2030_invalid_integer_range (offset_on, 0, 1,  -9) != 0)
        return;
    if (rf_unit < 4)
        Fmt (rf_unit_type, "%s<%s", "emf");
    else
        Fmt (rf_unit_type, "%s<%s", "pd");
    unit_rf = rf_unit;
    Fmt (cmd, "%s<imode normal;:cfrq:value %f %s;phase %f;:rflv:type %s;value %f %s;%s;OFFS:VALUE %f;%s;%s\r""\n", value
    , freq_units[f_unit], phase, rf_unit_type, rf_value, rf_units[rf_unit], on_off_str[on_off], offset, on_off_str[
    offset_on], enable[offset_on]);
    mi2030_write_msg (instrID, cmd, NumFmtdBytes ());
}
*/
