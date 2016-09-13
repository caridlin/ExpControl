#ifndef CONTROL_INCLUDE
#define CONTROL_INCLUDE

/*
#include <cvirte.h>     
#include <analysis.h>
#include <ansi_c.h>
#include <userint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <gpib.h>
#include <formatio.h>


#include "nidaqex.h"
#include "nidaqcns.h" 

#include "toolbox.h"
#include "inifile.h"
*/

#include "CONTROL/tools.h"		   			// some useful functions
#include "CONTROL/UIR_common.h"


#include "CONTROL/CONTROL_DataStructure.h"					
#include "CONTROL/CONTROL_DataSocket.h"					
#include "CONTROL/CONTROL_Definitions.h"					

#include "CONTROL/CONTROL_HARDWARE.h"

#include "CONTROL/CONTROL_CalculateOutput.h"
#include "CONTROL/CONTROL_LoadSave.h"

#include "CONTROL/CONTROL_GUI_MAIN.h"
#include "CONTROL/CONTROL_GUI_DigitalBlocks.h"
#include "CONTROL/CONTROL_GUI_EditPoints.h"
#include "CONTROL/CONTROL_GUI_Waveform.h"

#include "CONTROL/CONTROL_GUI_Config.h"  
#include "CONTROL/CONTROL_GUI_Simulate.h"
#include "CONTROL/CONTROL_GUI_Gpib.h"
#include "CONTROL/CONTROL_GUI_CCD.h"

#include "CONTROL/CONTROL_GUI_Import.h"


#endif    
