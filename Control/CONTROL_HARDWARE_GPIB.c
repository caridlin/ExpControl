#include "ExperimentControl.h"
#include <windows.h> // for Sleep 
#include <formatio.h>
#include <gpib.h>
#include <userint.h>
#include <utility.h>

#include "UIR_ExperimentControl.h"
#include "tools.h"    

#include "CONTROL_CalculateOutput.h"
#include "CONTROL_HARDWARE_Adwin.h"
#include "CONTROL_HARDWARE_GPIB.h"
#include "CONTROL_HARDWARE_DAQmx.h"
#include "CONTROL_HARDWARE.h"

#include "CONTROL_GUI_Config.h"
#include "CONTROL_GUI_GPIB.h"
#include "CONTROL_GUI_MAIN.h"


#include "GPIB_DRIVER_Anritsu.h"
#include "GPIB_DRIVER_Agilent.h"
#include "GPIB_DRIVER_LeCroy.h"
#include "GPIB_DRIVER_RohdeSchwarz.h"
#include "GPIB_DRIVER_ThurlbyThandar.h"
#include "GPIB_DRIVER_StanfordResearchSystems.h"
#include "GPIB_DRIVER_Berkeley.h" 
#include "GPIB_DRIVER_Marconi.h"
#include "CONTROL_GUI_ExtDev.h"											 
#include "EXTDEV_DRIVER_DDS.h"
#include "Adwin.h"

extern int panelMain;
int gpibOutputPanel   = -1;
int gpibOutputCtrl 	  = -1;
int gpibBoard = -1;
int panelDemag = -1;

int GPIB_showData = 1;

char lastGpibCommandSent[500] = "";


const char strGpibError[] = "GPIB error";
const char *strPosNeg[2] = {"POS", "NEG"};

const short RECV_TRIGGER_PAR_INDEX = 74;
const short SEND_TRIGGER_PAR_INDEX = 75;


ListType GPIB_listOfAvailableDevices = NULL;


void GPIB_addAvailableDevice (t_gpibDevice *dev)
{
	if (GPIB_listOfAvailableDevices == NULL) return;
    ListInsertItem (GPIB_listOfAvailableDevices, &dev, END_OF_LIST);
}



void GPIB_deviceInitAllAvailableDevices (void)
{
	if (GPIB_listOfAvailableDevices != NULL) return;
	GPIB_listOfAvailableDevices = ListCreate (sizeof (t_gpibDevice *));
    GPIB_addAvailableDevice (GPIB_deviceParameters_AgilentN5182A());
	GPIB_addAvailableDevice (GPIB_deviceParameters_AgilentN5182B());
	GPIB_addAvailableDevice (GPIB_deviceParameters_Agilent33250A());
    GPIB_addAvailableDevice (GPIB_deviceParameters_AnritsuMG3692A());
    GPIB_addAvailableDevice (GPIB_deviceParameters_Agilent33120A ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_Agilent33220A ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_Agilent83751A ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_RohdeSchwarzSML02 ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_LeCroyLW120 ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_ThurlbyThandarPL330DP ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_SRS_PS350 ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_Agilent6612C ());
    GPIB_addAvailableDevice (GPIB_deviceParameters_AgilentE3643A ());
	GPIB_addAvailableDevice (GPIB_deviceParameters_Keithley3390 ()); 
	GPIB_addAvailableDevice (GPIB_deviceParameters_SRS_SG384()); 
	GPIB_addAvailableDevice (GPIB_deviceParameters_AgilentE4432B());
	GPIB_addAvailableDevice (GPIB_deviceParameters_RohdeSchwarzSMY02());
	GPIB_addAvailableDevice (GPIB_deviceParameters_RohdeSchwarzSMY01());       
	GPIB_addAvailableDevice (GPIB_deviceParameters_BNC575());  
	
//    GPIB_addAvailableDevice (GPIB_deviceParameters_Marconi2030 ());

}

/*
void GPIB_deviceSetParameters (t_gpibDevice *dev)
{
	switch (dev->type) {
		case GPIBDEV_NONE:
			dev->name[0] = 0;
			break;
		case GPIBDEV_AGILENT_33120A:
/*			dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
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
			break;
/*		case GPIBDEV_AGILENT_33250A:
			dev->commandType1 	= GPIB_COMMANDTYPE_ARBWFM;
			dev->commandType2 	= GPIB_COMMANDTYPE_FREQ;
			dev->wfmMinPoints   = 1;
			dev->wfmMaxPoints   = 65535;
			dev->minVoltage     = -10.0;
			dev->maxVoltage     =  10.0;
			dev->minDigital     = -2047;
			dev->maxDigital		= 2047;
			dev->minDuration_us = 0.040;
			dev->nChannels		= 1;
			dev->maxDivide	    = 1;
			dev->hasExternalSCLK = 0;
			break;
		case GPIBDEV_LECROY_LW120:
			GPIB_deviceSetParameters_LeCroyLW120 (dev);
			break;
		case GPIBDEV_ANRITSU_MG3692A:
			dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
			dev->minFrequency   = 2.0E9;
			dev->maxFrequency   = 20.0E9;
			dev->minAmplitude   = -10;
			dev->maxAmplitude   = +30;
			dev->maxDivide	    = 4;
			break;
		case GPIBDEV_PL330DP:
			dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
			dev->minVoltage = 0;
			dev->maxVoltage = 32.0;
			dev->maxCurrent = 3;
			dev->nChannels = 2;
			break;
/*		case GPIBDEV_AGILENT_83751A:
			dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
			dev->minFrequency   = 20.0E6;
			dev->maxFrequency   = 20.0E9;
			dev->minAmplitude   = -10;
			dev->maxAmplitude   = +15;
			dev->enablePulseMode = 1;
			dev->enableExternalTrigger = 1;
			dev->maxDivide	    = 1;
			break;
		case GPIBDEV_ROHDESCHWARZ_SML02:
			dev->commandType1 	= GPIB_COMMANDTYPE_FREQ;
			dev->minFrequency   = 9.0E3;
			dev->maxFrequency   = 2.0E9;
			dev->minAmplitude   = -40;
			dev->maxAmplitude   = +15;
			dev->enablePulseMode = 0;
			dev->enableExternalTrigger = 1;
			dev->maxDivide	    = 1;
			dev->errorMessageCompatibleWithAgilent = 1;
			
			break;			
		case GPIBDEV_STANFORDRESEARCH_PS350:
			dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
			dev->minVoltage = 0.0;
			dev->maxVoltage = 5000.0;
			dev->maxCurrent = 5.25E-3;
			break;
		case GPIBDEV_AGILENT_6612C:
			dev->commandType1 	= GPIB_COMMANDTYPE_POWERSUPPLY;
			dev->minVoltage = 0;
			dev->maxVoltage = 20.0;
			dev->maxCurrent = 2;
			dev->errorMessageCompatibleWithAgilent = 1;
			
			break;
		default:
			DebugPrintf ("\nWarning! Function 'GPIB_deviceSetParameters()' not implemented for device %s", 
				dev == NULL ? "" : dev->name);			
			break;
	}
}
*/


int GPIB_outputPanel (void)
{
	return gpibOutputPanel;
}

int GPIB_outputCtrl (void)
{
	return gpibOutputCtrl;
}

// -------------------------------------------------
//      error codes returned in "iberr"
// see http://www.ni.com/pdf/manuals/370428c.pdf Appendix C for error codes
// -------------------------------------------------
const char *iberrStr (void)
{
    char *help;
    help = getTmpString();
    
    switch (iberr) {
		// ibcnt den Fehler-Code 2 "Device not found".  
		case 0: 
			sprintf (help, "Operating system error.  The system-specific error code is ibcnt=%d.", ibcnt);
			return help;
		case 1: return "ECIC: Function requires GPIB-PC to be CIC.";
		case 2: return "ENOL: No listener on write function.";
		case 3: return "EADR: GPIB-PC addressed incorrectly.";
		case 4: return "EARG: Invalid function call argument.";
		case 5: return "ESAC: GPIB-PC not System Controller as required.";
		case 6: return "EABO: I/O operation aborted.";
		case 7: return "ENEB: Non-existent GPIB-PC board.";
		case 8: return "EDMA: Virtual DMA device error.";
		case 10: return "EOIP: I/O started before previous operation completed.";
		case 11: return "ECAP: Unsupported feature.";
		case 12: return "EFSO: File system error.";
		case 14: return "EBUS: Command error during device call.";
		case 15: return "Serial Poll status byte lost.";
		case 16: return "ESRQ: SRQ stuck in on position.";
		case 20: return "ETAB: Device list error during a FindLstn or FindRQS call.";
		case 21: return "ELCK: Address or board is locked.";
		case 22: return "EARM: ibnotify callback failed to rearm.";
		case 23: return "EHDL: Input handle is invalid."; 
		case 24: return "The GPIB-ENET was already on-line, and the default board configuration sent to it differs from than configuration\n"
                      "under which it was already operating. This is only a warning.  The board configuration has not been changed,\n"
                      "but the operation has otherwise completed successfully.";
		case 26: return "EWIP: ibwait already in progress.";
		case 27: return "ERST: event notification cancelled due to interface reset.";
		case 28: return "EPWR: interface power loss."; 
		case 200: return "The GPIB library was not linked. Dummy functions were linked instead.";
		case 201: 
			sprintf (help, "Error loading GPIB-32.DLL.  The MS Windows error code is %d.", ibcnt);
			return help;
		case 203: 
			sprintf (help, "Unable to find the function in GPIB-32.DLL. The MS Windows error code is %d.", ibcnt);
			return help;

		case 205: 
			sprintf (help, "Unable to find globals in GPIB-32.DLL. The MS Windows error code is %d.", ibcnt);
			return help;
		case 206: return "Not a National Instruments GPIB-32.DLL.";
		case 207: 
			sprintf (help, "Unable to acquire Mutex for loading DLL. The MS Windows error code is %d.", ibcnt);
			return help;
		case 210: return "Unable to register callback function with MS Windows.";
		case 211: return "The callback table is full.";
		default:
			return "";
//			sprintf (help, "unknown");
//			return help;
	}
}




// -------------------------------------------------
//      error codes returned by "ibsta"
// -------------------------------------------------

char *ibstaStr (void) 
{
	#define N_GPIB_ERRORS 13
	static int errorCodes[N_GPIB_ERRORS] = {  
		0x8000, //        GPIB error
        0x2000, //   END      END or EOS detected
        0x1000, //   SRQI     SRQ on
         0x800, //   RQS      Device requesting service
         0x100, //   CMPL     I/O completed
          
          0x80, //   LOK      GPIB-PC in Lockout State
		  0x40, //   REM      GPIB-PC in Remote State
          0x20, //   CIC      GPIB-PC is Controller-In-Charge
          0x10, //   ATN      Attention is asserted
           0x8, //   TACS     GPIB-PC is Talker
     
           0x4, //   LACS     GPIB-PC is Listener
           0x2, //   DTAS     GPIB-PC in Device Trigger State
           0x1 //    DCAS     GPIB-PC in Device Clear State
	};
	static char *errorStr[N_GPIB_ERRORS] = {
	    "GPIB error",                            
		"END (END or EOS detected)",
		"SRQI (SRQ on)",                           
		"RQS (Device requesting service)",      
		"CMPL (I/O completed)",
		"LOK (GPIB-PC in Lockout State)",
		"REM (GPIB-PC in Remote State)",           
		"CIC (GPIB-PC is Controller-In-Charge)",  
		"ATN (Attention is asserted)",            
		"TACS (GPIB-PC is Talker)",                
		"LACS (GPIB-PC is Listener)",
		"DTAS (GPIB-PC in Device Trigger State)",
		"DCAS (GPIB-PC in Device Clear State)"    
	};
	
	char *help;
	int i;
	
	help = getTmpString();  

	if (ibsta & errorCodes[0]) {
		sprintf (help, "\nGPIB error (iberr=%d) %s.", iberr, iberrStr());
	}
	for (i = 1; i < N_GPIB_ERRORS; i++) {
		if (ibsta & errorCodes[i]) {
			strcat (help, "\n");
			strcat (help, errorStr[i]);
		}
	}
	
	return help;	
	#undef N_GPIB_ERRORS 
}


    
int MessagePopupAbortSequence (char *format, ...)
{
	char help[500];
	
	va_list arg;

	va_start( arg, format );
    vsprintf( help, format, arg);
    va_end( arg );
    
	if (PostGenericMessagePopup (strGpibError, help, "Continue", "Abort sequence", "",
						NULL, 0, 0, VAL_GENERIC_POPUP_BTN2,
						VAL_GENERIC_POPUP_BTN2,
						VAL_GENERIC_POPUP_BTN2) == VAL_GENERIC_POPUP_BTN2) 
		return -1;
	return 0;
}



int ibstaCheck (int address, const char *command) 
{
	if (ibsta == 0) return 0;
	if ((ibsta & 0x100) == 0x100) return 0;
   	return MessagePopupAbortSequence ("GPIB device #%02d (%s) communication failed.\n\n"
   									  "Command \"%s\" returned error code (ibsta=0x%x).\n%s",	
   									  address, GPIBDEVICE_ptr(address)->name, 
   									  command, ibsta, ibstaStr() );
}





void GPIB_checkStatus (void)
{
    tprintf ("ibsta=%x; iberr=%d\n", ibsta, iberr);
}



int GPIB_errorMessage (int address)
{
   return MessagePopupAbortSequence ("GPIB device #%02d (%s) communication failed.\n\n"
   									 "Error code (iberr) = %d: \"%s\"\n\n"
   									 "Last command sent: \"%s\"\n",
   									address, GPIBDEVICE_ptr(address)->name, 
   									iberr, iberrStr(),lastGpibCommandSent);
}






int GPIB_showDeviceErrorMessage (int address, int errNo, char *str)
{
//	char help[ERRMSG_LEN+200];

	if (errNo != 0) {
		return MessagePopupAbortSequence  (
		        "GPIB device #%02d (%s) returned an error.\n"
				"(last string sent: \"%s\")\n\n"
				"Error code %d: %s\n",
				address, GPIBDEVICE_ptr(address)->name, 
				lastGpibCommandSent,
				errNo, str);
	}
	return errNo;
}




// ---------------------------------------------
//    sets the control where send_GPIB 
//    and receive_Gip display the commands
// ---------------------------------------------
void GPIB_setOutputCtrl (int panel, int ctrl)
{
    gpibOutputPanel = panel;
    gpibOutputCtrl  = ctrl;
}


void GPIB_clearOutputCtrl (void)
{
	cls (gpibOutputPanel, gpibOutputCtrl);
}


int GPIB_autodetectBoard (void)
{
	int tmp;

	SetBreakOnLibraryErrors (0);
	gpibBoard = -1;
 	if ((tmp=ibfind ("gpib0")) >= 0) gpibBoard = 0;
 	else if (ibfind ("gpib1") >= 0) gpibBoard = 1;
 	else if (ibfind ("gpib2") >= 0) gpibBoard = 2;
 	else if (ibfind ("gpib3") >= 0) gpibBoard = 3;
 	return gpibBoard;
}





int GPIB_identifyDevice (t_gpibDevice *dev, const char *strIDN)
{
	int i;
	int len;
	char help[200];
	char *comma1, *comma2;
	char idStr[2] = "";
	int nDev;
	t_gpibDevice *devPtr;
	//int handle;
	char tmpIDN[200];
	
	nDev = ListNumItems (GPIB_listOfAvailableDevices);
	strcpy (tmpIDN, strIDN);
	
	if (strIDN != NULL) {
		dev->type = GPIBDEV_UNKNOWN;
		for (i = 1; i <= nDev; i++) {
			ListGetItem (GPIB_listOfAvailableDevices, &devPtr, i);
			len = strlen (devPtr->strIDN);
			if (len > 0) {
				if (strncmp (strIDN, devPtr->strIDN, len) == 0) {
					dev->type = i;
					GPIBDEVICE_copySettings (dev, devPtr);
					strcpy (idStr, "*");
					break;
				}				 
			}
			len = strlen (devPtr->strIDN2);
			if ((len > 0) && (strncmp (strIDN, devPtr->strIDN2, len) == 0)) {
				dev->type = i;
				GPIBDEVICE_copySettings (dev, devPtr);
				strcpy (idStr, "*");
				break;
			}
		}

		comma1 = strchr (tmpIDN, ',');
		if (comma1 != NULL) {
			if (dev->type == GPIBDEV_UNKNOWN) {
				strncpy (dev->strIDN, tmpIDN, MAX_GPIBDEVICENAME_LEN-1);	
				dev->strIDN[MAX_GPIBDEVICENAME_LEN-1] = 0;
			}
			comma1[0] = 0;
			comma2 = strchr (comma1+1, ',');
			if (comma2 != NULL) comma2[0] = 0;
			sprintf (help, "%s%s, %s", idStr, comma1+1, tmpIDN);
			if (dev->type == GPIBDEV_UNKNOWN) {
				strcat (help, ", no driver");
			}
			strcpy (dev->name, help);
		}
	}
	return (dev->type == GPIBDEV_UNKNOWN);
}




void GPIB_open (int address, int timeout)
{
    t_gpibDevice *d;
    
    d = GPIBDEVICE_ptr(address);
	if (d == NULL) {
		tprintf("GPIB error: device pointer is NULL\n");
		return;
	}
	
    if (d->handle == -1) {
		//tprintf("GPIB info: open new device via ibdev (address: %d) (old handle was: %d)\n",address,d->handle);
	    DisableBreakOnLibraryErrors (); 
	    d->handle = ibdev (config->gpibBoardIndex, address, NO_SAD, timeout, 1, 0);   // 
		EnableBreakOnLibraryErrors (); 
		// ibclr(d->handle)    // should be used see http://www.ni.com/pdf/manuals/370963a.pdf page 99
		// see http://www.ni.com/pdf/manuals/370428c.pdf page 42
	}
	if (d->handle < 0) {
		/* Handle GPIB error here */
		tprintf("GPIB error: opening device failed (returned: %d)\n",d->handle);             
		if (iberr == EDVR) {
			/* bad boardindex or no devices
			 * available.
			 */
			tprintf("GPIB error: bad boardindex or no devices available.\n");             
		}
		else if (iberr == EARG) {
			/* The call succeeded, but at least one
			 * of pad,sad,tmo,eos,eot is incorrect.
			 */
			tprintf("GPIB error: The call succeeded, but at least one of pad,sad,tmo,eos,eot is incorrect.\n");
		}
		
	}
	//tprintf("GPIB info: device handle for address %d is %d\n",address,d->handle);  
}



int GPIB_close (int address)
{
	int tmp;
	
	tmp = GPIBDEVICE_ptr(address)->handle; 
	GPIBDEVICE_ptr(address)->handle = -1;
	if (tmp >= 0) {
		ibonl (tmp, 0);
	}
	
	
	//ibonl (GPIBDEVICE_ptr(address)->handle, 0);
	//GPIBDEVICE_ptr(address)->handle = -1;

	if (ibstaCheck (address, "ibonl")  != 0) return -1;
	return 0;
}




int GPIB_sendBinary (int address, char *commandPrefix, short *binaryData, int nPoints, int dma)  // the function that sends the string to the device 
{
	char help1[20];
	char help[200];
	char *dataStr;
	char *startPos;
	int err;
	int len;
	unsigned short last;
//	t_gpibCommand *g;

//	g = GPIBDEVICE_ptr(address);
	
	if (dma) {
		last = binaryData[nPoints-1];
// DEBUG!!!
		last |= 0x8000;
		memcpy (&binaryData[nPoints-1], &last, 2);

		err = ibwrta (GPIBDEVICE_ptr(address)->handle, binaryData, nPoints*2) & ERR;

		sprintf (help, "\n%02d: DMA transfer... [%d bytes]", address, nPoints*2);
		if (GPIB_showData) {
			pprintf (gpibOutputPanel, gpibOutputCtrl, help);
			GPIB_setLed (1, help+1);
			ProcessDrawEvents();
		}
		strcpy (lastGpibCommandSent, help+4);
		ibwait (GPIBDEVICE_ptr(address)->handle, CMPL | TIMO);
		return err;
	}
	else {
  		dataStr = (char *) malloc (nPoints*2+strlen(commandPrefix)+20);
		sprintf (help, "\n%02d: %s...[binary data, %d bytes]", address, commandPrefix, nPoints*2);
		if (GPIB_showData) {
			pprintf (gpibOutputPanel, gpibOutputCtrl, help);
			GPIB_setLed (1, help+1);
			ProcessDrawEvents();
		}
		strcpy (lastGpibCommandSent, help+4);
	    sprintf (help1, "%d", nPoints*2);
		sprintf (help, "%s%d%s", commandPrefix, strlen(help1), help1);
	    strcpy (dataStr, help);
	    len = strlen(dataStr);
	    startPos = dataStr + len;
		memcpy (startPos, binaryData, nPoints*2);
		
		err = ibwrt (GPIBDEVICE_ptr(address)->handle, dataStr, len+2*nPoints) & ERR;
		ibwait (GPIBDEVICE_ptr(address)->handle, CMPL | TIMO);
		free (dataStr);
		return err;
	}
	
}



void GPIB_setShowDataFlag (int show)
{
	GPIB_showData = show;
}

int GPIB_getShowDataFlag (void)
{
	return GPIB_showData;
}


int GPIB_sendMultipleLines (t_gpibCommand *g, char *str, int errorCheck)
{
	char line[400];
	char *searchPos, *foundPos, *percentPos;
	int err;
	int n;
							 
	searchPos = str;
	foundPos = searchPos;
	err = 0;
	while ((foundPos != NULL) && (err == 0)) {
		foundPos = strchr (searchPos, '\n');
		if (foundPos != NULL) {
			n = foundPos-searchPos;
			strncpy (line, searchPos, n);
			line[n]=0;
		}
		else strcpy (line, searchPos);
		percentPos = strchr (line, '%');
		if (percentPos != NULL) percentPos[0] = 0;
		RemoveSurroundingWhiteSpace(line);
		if (strlen (line) > 0) {
			err = GPIB_send (g->gpibAddress, line);
			if (errorCheck) GPIB_getErr (g,1);
		}
		searchPos = foundPos + 1;
	}
	return err;
}


int GPIB_send (int address, char *str)  // the function that sends the string to the device 
{
    char help[200];
    t_gpibDevice *device;
    int err;
    int ret;
    
    if (str == NULL) return 0;
	if (GPIB_showData) {
	    sprintf (help, "\n%02d: ", address);
	    strncat (help, str, 100);
	    pprintf (gpibOutputPanel, gpibOutputCtrl, help);
	 }
	 
	strcpy (lastGpibCommandSent, str);
	 
    device = GPIBDEVICE_ptr(address);
//   strcpy (help, str);
///*   if (device->type == GPIBDEV_PL33DP) {
//      Fmt(help,"%s\010", str);
// 	   return ibwrt (device->handle, help, NumFmtdBytes()) & GPIB_ERR;
//   }*/
    err = ibwrt (device->handle, str, strlen(str));
	if (GPIB_showData) {
		GPIB_setLed (1, help+1);
		ProcessDrawEvents();
	}
    
   
	ret = ibwait (device->handle, CMPL | TIMO);   
	// wait for:
	// CMPL	0x100	I/O operation is complete. Useful for determining when an asynchronous io operation (ibrda(), ibwrta(), etc) has completed.
	// TIMO indicates that the last io operation or ibwait() timed out.
	// ERR is set if the last 'traditional' or 'multidevice' function call failed. The global variable iberr will be set indicate the cause of the error.
//	pprintf (gpibOutputPanel, gpibOutputCtrl, " (%04x) ", ret);

	return err  & ERR; 
   
}



int GPIB_sendf (int address, char* format, ... )
{
	char help[500];
	
	va_list arg;

	va_start( arg, format );
    vsprintf( help, format, arg);
    va_end( arg );
    
    return GPIB_send (address, help);
}



int GPIB_receive (int address, char *str, int maxLength)  
{
   int status;
   #define BUFSIZE 500
   
   char buffer[BUFSIZE];
   char help[300];
   
   
   if (str == NULL) {
   	   str = buffer;
   	   maxLength = BUFSIZE;
   }
  
   status = ibrd (GPIBDEVICE_ptr(address)->handle, str, maxLength) & ERR;
   if (status == 0) {
	   #if defined(_NI_mswin32_)  
       if (ibcntl > maxLength) str[maxLength-1] = 0;
       else str[ibcntl-1] = 0; 
		#else
	   if (ibcnt > maxLength) str[maxLength-1] = 0;
       else str[ibcnt-1] = 0;
		#endif
   }
   else {
       str[0] = 0;
   }
   if (GPIB_showData) {
	   sprintf (help, "  -->%02d: ", address);
	   strncat (help, str, 200);
	   pprintf (gpibOutputPanel, gpibOutputCtrl, help);
	   ProcessDrawEvents();
   }
   return status;
}



void GPIB_resetOutput (void)
{
	// TODO
	ResetTextBox (gpibOutputPanel, gpibOutputCtrl,""); 
	//DeleteTextBoxLines (gpibOutputPanel, gpibOutputCtrl, 0, -1);
}



// commands For Agilent88250A





int GPIB_autoDetectDevice (t_config *oldConfig, t_gpibDevice *dev, int i)
{
	// init the device with the address int address
	// and return the pointer to this device for 

    char strIDN[] = "*IDN?";
    #define IDNlen 200
    char strDeviceIDN[IDNlen];
    t_gpibDevice *oldDev;
    //int ch;
    int status = -1;

	DisableBreakOnLibraryErrors ();
	GPIB_open (i, T300ms);
	Sleep (100);
	if (GPIB_send (i, strIDN) == 0) 
	{
  		Sleep (50);
  		GPIB_receive (i, strDeviceIDN, IDNlen);
  		if (strDeviceIDN[0] == 0) {
			// try again if nothing received
			Sleep (100);
			if (GPIB_send (i, strIDN) == 0) {
  				GPIB_receive (i, strDeviceIDN, IDNlen);
			}
  		}
  		if (strDeviceIDN[0] == 0) {
			// try again if nothing received
			Sleep (200);
			if (GPIB_send (i, strIDN) == 0) {
  				GPIB_receive (i, strDeviceIDN, IDNlen);
			}
  		}

		status = GPIB_identifyDevice (dev, strDeviceIDN);
	    GPIB_close (i);
	    
	    if (dev->GPIB_getDeviceOptions) dev->GPIB_getDeviceOptions (i);

		if (oldConfig != NULL) {
		 	oldDev = GPIBDEVICE_ptrConfig (oldConfig, i);
			if ((oldDev!= NULL) && (oldDev->type == dev->type)) {
				switch (dev->type) {
					case GPIBDEV_LECROY_LW120:
//						dev->externalSCLK = oldDev->externalSCLK;
//						dev->frequencySCLK = oldDev->frequencySCLK;
						dev->triggerSlope = oldDev->triggerSlope;
//						for (ch = 0; ch < 2; ch++) {
//							dev->divideFreq[ch]             = oldDev->divideFreq[ch];
//							dev->displayAsAnalogChannel[ch] = oldDev->displayAsAnalogChannel[ch];
//						}
						dev->triggerSignal = oldDev->triggerSignal;
						break;
				}
			
			}
		 }
	}
	return status;
}

	




	
int GPIB_getErr (t_gpibCommand *g, int showMessage)
{
	char errorStr[ERRMSG_LEN];
	char strAskErr[] = ":SYST:ERR?";
	int err;
	t_gpibDevice *dev;
	
	dev = g->device;
	if (!g->askDeviceForErrors) return 0;
	if (dev->errorMessageCompatibleWithAgilent) {
		// error handling comatible with Agilent devices
		err = ibwrt (GPIBDEVICE_ptr(g->gpibAddress)->handle, strAskErr, strlen(strAskErr)) & ERR;
		ibwait (GPIBDEVICE_ptr(g->gpibAddress)->handle, CMPL);
		if (err != 0) {
			if (showMessage) return GPIB_errorMessage(g->gpibAddress);
			else return -1;
		}
	    Sleep (10);
	    if (GPIB_receive (g->gpibAddress, errorStr, ERRMSG_LEN) != 0) {
			if (showMessage) return GPIB_errorMessage(g->gpibAddress);
			else return -1;
		}		    	
	   	return GPIB_showDeviceMessage_Agilent33250A (g, errorStr, showMessage);
	}
	
	return 0;
}



void GPIB_sendReset (int address)
{
	GPIB_open (address, T1s);
//	handle = ibdev (config->gpibBoardIndex, address, NO_SAD, T1s, 1, 0);
	
	GPIBDEVICE_resetLastValues (GPIBDEVICE_ptr (address), 1);
	
	if (GPIB_send (address, "*RST") != 0) GPIB_errorMessage(address);
	GPIB_close (address);
}












int GPIB_transmitCommandStrings (t_gpibCommand *g)
{
	int i;
	t_gpibDevice *d; 
	 
	
	GPIB_open (g->gpibAddress, T1s);
	
	// reset last values of this device
	d = GPIBDEVICE_ptr (g->gpibAddress);
	GPIBDEVICE_resetLastValues (d, 1);
	
	
	for (i = 0; i < GPIB_NCOMMANDSTRINGS; i++) {
		if (g->commandStringSend[i]) {
			if (GPIB_sendMultipleLines (g, g->commandString[i], g->commandStringErrorCheck[i]) != 0) 
				return GPIB_errorMessage(g->gpibAddress);
			if (g->commandStringReceive[i]) {
		    	if (GPIB_receive (g->gpibAddress, NULL, 0) != 0)
		   		return GPIB_errorMessage(g->gpibAddress);
		   	}
			if (g->commandStringErrorCheck[i]) {
				GPIB_getErr (g,1);
			}
		}
	}
	
	return 0;
}




void GPIB_writeDataToOutputBuffer (t_gpibCommand *c, 
	long *values, unsigned long *repeat, unsigned long nValues)
{
	unsigned long i;
	unsigned long n, idx, vNr;
	
	free (c->dataBuffer);
	if (nValues == 0) {
		c->dataBuffer = (short *) calloc (c->nPoints, sizeof (short));
		return;
	}
	else
		c->dataBuffer = (short *) malloc (sizeof (short) * c->nPoints);

    
    // "idx" is array index of destination
    idx = 0;
    // value nr to copy
    vNr  = 0;
    n    = 0;
    // number of repetitions for that value
    while (repeat[vNr] == 0) vNr++;
	// -----------------------------------------------
	//    get number of repetitions and value to copy
	// -----------------------------------------------
// -----------------------------------------------
//    write all samples
// -----------------------------------------------
    for (i = 0; i < c->nPoints; i++) {
		// -----------------------------------------------
		//    copy value to destination
		// -----------------------------------------------
        c->dataBuffer[idx] = values[vNr];
		// -----------------------------------------------
        //    increase #repetitions for that value
		// -----------------------------------------------
        n++;
        if (n == repeat[vNr]) {
            if (vNr < nValues-1) { 
				// -----------------------------------------------
				//    find new value
				// -----------------------------------------------
                vNr++;
    			while (repeat[vNr] == 0) vNr++;
				// -----------------------------------------------
				//    get number of repetitions and value to copy
				// -----------------------------------------------
            }
            // reset repetitions 
            n = 0;
        }
        // step to next place in buffer
        idx ++;
    }
   	return;
}




void GPIB_writeWaveformToOutputBuffer (t_gpibCommand *c, t_waveform *wfm)
{
//	int i;
//	int nr;
	
	GPIB_writeDataToOutputBuffer (c, 
	   wfm->DIG_values, wfm->DIG_repeat, wfm->DIG_NValues);
/*	DebugPrintf ("\n");
	for (i = 0; i < c->nPoints; i++) {
//		nr = ;
		DebugPrintf ("%d,", c->dataBuffer[i]);
//		if (i % 20 == 0) DebugPrintf ("\n");
	}
*/	
}




t_digitizeParameters *GPIB_digitizeParameters (t_gpibCommand *g)
{
	static t_digitizeParameters p;
	t_digitizeParameters *p1;
	t_gpibDevice *dev;

	dev = g->device;
	if (dev == NULL) {
		p1 = digitizeParameters_PCI67XX(1);
		memcpy (&p, p1, sizeof(t_digitizeParameters));
		p.nPoints = g->nPoints;
		p.duration_us = g->duration_us;
	}
	else {
		p.maxVoltage 	= dev->maxVoltage;
		p.minVoltage 	= -p.maxVoltage;
		p.Vpp 			= p.maxVoltage - p.minVoltage;
		p.minDigital 	= dev->minDigital;
		p.maxDigital 	= dev->maxDigital;
		p.Dpp 			= p.maxDigital - p.minDigital;
		if (g->nPoints > dev->wfmMaxPoints) 
			p.nPoints = dev->wfmMaxPoints;
		else p.nPoints = g->nPoints;
		if (g->duration_us < dev->minDuration_us) 
			p.duration_us = dev->minDuration_us;
		else p.duration_us = g->duration_us;
		p.offsetCorrection = g->offsetCorrection;
	}
	p.timebase_50ns	= 2;
	return &p;	
}





//=======================================================================
//
// 	  return the current voltage for "step output" 
//
//=======================================================================
double GPIB_getStepFrequencyInHz  (t_sequence *seq, t_gpibCommand *g) 
{
	double step; 
	int nSteps, nRun;
	 
	// -----------------------------------------------------------
    // 	  calculate number of voltage steps
	// -----------------------------------------------------------
	nSteps = (seq->nRepetitions / g->nStepRepetition - 1);
	if (seq->nRepetitions % g->nStepRepetition != 0) nSteps++;
	if (nSteps == 0) return g->startFreq + g->addFreqOffset * g->freqOffsetMHz * 1E6;
	// -----------------------------------------------------------
    // 	  calculate stepsize
	// -----------------------------------------------------------
	step   = (g->stopFreq - g->startFreq) / (1.0*nSteps);
	nRun   = seq->repetition /  g->nStepRepetition;
	// -----------------------------------------------------------
    // 	  calculate output voltage
	// -----------------------------------------------------------
	return g->startFreq + step * (1.0*nRun) + g->addFreqOffset * g->freqOffsetMHz * 1E6;
}







void GPIB_resetAllLastValues (t_sequence *seq, int resetLeCroy, int resetWaveforms)
{
	unsigned int i;
	t_gpibDevice *d;
	
	if (!config->enableGPIB) return;
	
	for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) {
		d = GPIBDEVICE_ptr (GPIBCOMMAND_ptr(seq, i)->gpibAddress); 
		if (d != NULL) {
			if (d->type == GPIBDEV_LECROY_LW120) {
				if (resetLeCroy) GPIBDEVICE_resetLastValues (d, resetWaveforms);  
			}
			else {
			    GPIBDEVICE_resetLastValues (d, 1);
			    //d->handle = -1;
				GPIB_close(GPIBCOMMAND_ptr(seq, i)->gpibAddress);
			}
		}
	}
}






int GPIB_transmittedDataChanged (t_gpibDevice *dev, int devCh, t_outputData *out, int outCh)
{
    if (dev->last_numAOValues[devCh] != out->numAOValues[outCh]) return 1;
    if (out->numAOValues[outCh] == 0) return 0;
    if (memcmp (dev->last_AOvalues[devCh], out->AO_Values[outCh], out->numAOValues[outCh]*sizeof(long))) return 1;
	if (memcmp (dev->last_AOvalueDuration[devCh], out->AO_ValueDuration[outCh], out->numAOValues[outCh]*sizeof(unsigned))) return 1;
	return 0;
}




void GPIB_setLed (int state, char *text) 
{
	SetCtrlVal (panelMain, MAIN_LED_gpib, state);
	if (text != NULL) 
		SetCtrlVal (panelMain, MAIN_STRING_gpibCommand, text);
	else 
		SetCtrlAttribute (panelMain, MAIN_STRING_gpibCommand, ATTR_CTRL_VAL,  "");
	
//	ProcessDrawEvents ();
}



  


int GPIB_transmitCommand_wfm (t_sequence *s, t_gpibCommand *g)
{
	int error = 0;
	t_waveform *w;
	t_gpibDevice *dev;
	
	dev = g->device;
	
	w = WFM_ptr (s, g->waveform);
    WFM_calculateOffsetAndDuration (w, s->repetition, s->nRepetitions, 0);
	if (w == NULL) return 0;
    g->duration_us = w->durationThisRepetition_ns / 1000;
//	GPIB_calculateNPoints (g);
	if (g->duration_us < dev->minDuration_us) 
		g->duration_us = dev->minDuration_us;

	if (g->nPoints > dev->wfmMaxPoints) {
		PostMessagePopupf ("GPIB command error", "Too many points.\n\nWaveform '%s' of GPIB command '%s' has %d points,\n"
				      "but device '%s' can only charge waveforms of <= %d points.\n\n"
				      "You may shorten the waveform or decrease the SCLK timebase (see\n"
				      "button 'Device settings').",
				      w->name, g->name, g->nPoints, dev->name, dev->wfmMaxPoints);
		return -1;						    
//				g->nPoints = g->device->wfmMaxPoints;
	}
		

	WFM_digitize (s, w, GPIB_digitizeParameters(g), 0);

	if (dev->GPIB_transmitWaveform == NULL) error = GPIB_ERR_NOT_SUPPORTED;
	else {
		GPIB_writeWaveformToOutputBuffer (g, w);
		dev->GPIB_transmitWaveform (s, g, 1, 0);
	}
	return error;
}




int GPIB_transmitCommand_powerSupply (t_sequence *s, t_gpibCommand *g, int waitUntilStabilized)
{
	int error = 0;
	t_gpibDevice *dev;
	
	dev = g->device;
	if (waitUntilStabilized) {
		if (dev->GPIB_waitUntilStabilized != NULL) {
			error =  dev->GPIB_waitUntilStabilized (g);
		}
		return error;
	}
	//calculate actual voltage
	g->voltage = g->startVoltage;
	if ((g->stepVoltage) && (s->nRepetitions > 1)) {
		g->voltage += (g->stopVoltage - g->startVoltage) / (s->nRepetitions-1) * s->repetition;
	}
	// calculate actual current
	g->current = g->startCurrent;
	if ((g->stepCurrent) && (s->nRepetitions > 1)) {
		g->current += (g->stopCurrent - g->startCurrent) / (s->nRepetitions-1) * s->repetition;
	}
	
	if (dev->GPIB_transmitPowerSupply == NULL) {
		return GPIB_ERR_NOT_SUPPORTED;
	}

	// get settings from devices
	if (dev->GPIB_getDeviceSettings != NULL) {
		error = dev->GPIB_getDeviceSettings(g);
	}
	
	// check for overrange
	if ((g->voltage > dev->maxVoltage) || (g->voltage < dev->minVoltage)) {
		PostMessagePopupf ("GPIB command error", "Voltage out of range.\n\n"
					  "GPIB command '%s', device '%s', GPIB address %d.\n\n"
				      "You programmed U = %1.3f V, but\n"
				      "device supports voltages from %1.3f V to %1.3f V.",
				       g->name, dev->name, g->gpibAddress,
				       g->voltage, dev->minVoltage, dev->maxVoltage);
		return -1;						    
	}
	if ((g->current > dev->maxCurrent) ) {
		PostMessagePopupf ("GPIB command error", "Current out of range.\n\n"
					  "GPIB command '%s', device '%s', GPIB address %d.\n\n"
				      "You programmed I = %1.3f mA, but\n"
				      "device supports a maximum current of %1.3f mA.",
				       g->name, dev->name, g->gpibAddress,
				       g->current *1000 , dev->maxCurrent * 1000);
		return -1;						    
	}

	
	error = dev->GPIB_transmitPowerSupply (s, g, 0, 0);
	return error;
}

int GPIB_transmitCommand_pulses(t_sequence *s, t_gpibCommand *g)
{
	 int error = 0;
	 GPIB_transmitCommand_BNC575 (s, g, 0, 0);    
	 return error;
}



    
int GPIB_MessagePopupDeviceNotSupported (char *format, ...)
{
	char help[500];
	
	va_list arg;

	va_start( arg, format );
    vsprintf( help, format, arg);
    va_end( arg );
    
	if (PostGenericMessagePopup (strGpibError, help, "Continue", "Abort sequence", "",
						NULL, 0, 0, VAL_GENERIC_POPUP_BTN2,
						VAL_GENERIC_POPUP_BTN2,
						VAL_GENERIC_POPUP_BTN2) == VAL_GENERIC_POPUP_BTN2) 
		return -1;
	return 0;
}
//=======================================================================
//
//    display external device names in all rings
//
//=======================================================================
void EXTDEV_fillDevicesToList (int panel, int control, int type)
{
    unsigned int i;
    t_ext_device *d;
	unsigned int ID;
    char h[200];
    int *selected;
    int flag = 0;
	t_sequence *seq;
    seq = activeSeq();
    selected = (int *) calloc (ListNumItems (seq->lExtDevices), sizeof(int));
    
	// -----------------------------------------------------------
    //      clear list + insert items
	// -----------------------------------------------------------
	ClearListCtrl (panel, control);
	d=seq->noExternalDevice;
	ID=d->uniqueID;
	sprintf (h, "%s: %s", EXTDEV_typeStr(d), d->name);
	InsertListItem (panel, control, -1, h, 0);
	
	for (i=1; i <= ListNumItems (seq->lExtDevices); i++) 
	{
        d = EXTDEV_ptr(seq,i);
        if (d != NULL)  {
				//tprintf("Pos. in list%d, Name:%s, ID:%d\n",i,d->name,d->uniqueID);
        	if (((d->deviceTyp == type) ||  (type == EXTDEV_TYPE_NONE)) && (d!=seq->noExternalDevice) ) {
	        	sprintf (h, "%s: %s", EXTDEV_typeStr(d), d->name);
	        	//if (d->hasPulseOption) strcat (h, ", opt: PULSE");
	        	InsertListItem (panel, control, -1, h, i);
	        	selected[i-1] = 1;
	        	flag = 1;
	        }
        }
    }
	/*
	if (flag)  
		InsertListItem (panel, control, -1, "", -2);    
    for (i=1; i < ListNumItems (seq->lExtDevices); i++) 
	{
        d = EXTDEV_ptr(seq,i);
        if (d != NULL)  {
        	if (!selected[i-1]) {
	        	sprintf (h, "%s: %s", EXTDEV_typeStr(d), d->name);
	        	InsertListItem (panel, control, -1, h, i);
	        	selected[i-1] = 1;
	        }
        }
    }
	*/
	free (selected);
}

//=======================================================================
//
//    display device names in all rings
//
//=======================================================================
void GPIB_fillDevicesToList (int panel, int control, int type)
{
    int i;
    t_gpibDevice *g;
    char h[200];
    int *selected;
    int flag = 0;
    
    selected = (int *) calloc (config->nGpibDevices, sizeof(int));
    
	// -----------------------------------------------------------
    //      clear list + insert items
	// -----------------------------------------------------------
	ClearListCtrl (panel, control);
	InsertListItem (panel, control, -1, "No GPIB Device", 0);
	for (i=1; i < config->nGpibDevices; i++) 
	{
        g = GPIBDEVICE_ptr(i);
        if (g != NULL)  {
        	if ((g->commandType1 == type) || (g->commandType2 == type) || (type == -1)) {
	        	sprintf (h, "%02d: %s", i, g->name);
	        	if (g->hasPulseOption) strcat (h, ", opt: PULSE");
	        	InsertListItem (panel, control, -1, h, i);
	        	selected[i-1] = 1;
	        	flag = 1;
	        }
        }
    }
	if (flag)  
		InsertListItem (panel, control, -1, "", -2);
	for (i=1; i < config->nGpibDevices; i++) 
	{
        g = GPIBDEVICE_ptr(i);
        if (g != NULL)  {
        	if (!selected[i-1]) {
	        	sprintf (h, "%02d: %s", i, g->name);
	        	if (g->hasPulseOption) strcat (h, ", opt: PULSE");
	        	InsertListItem (panel, control, -1, h, i);
	        	selected[i-1] = 1;
	        }
        }
    }
  
    
    free (selected);
}



void triggerLoop(void)
{
	Set_Par_Err(SEND_TRIGGER_PAR_INDEX, 1);
}

void recvTriggerLoop(void)
{
	while (Get_Par(RECV_TRIGGER_PAR_INDEX) != 1);
	// receive an image, telling Drogon that system control is ready to receive the image by resetting our trigger
	char image[30000];
	Set_Par_Err(RECV_TRIGGER_PAR_INDEX, 0);
	recv(ConnectSocket, image, 30000, 0);
	
	// write the image to file
	FILE *imageFile;
	imageFile = fopen("sequences\\testImage.pgm", "w");
	fprintf(imageFile, "%s", image);
	fclose(imageFile);
}


int GPIB_transmitCommand (t_sequence *s, t_gpibCommand *g, int waitUntilStabilized, int allowAutodetect)
{
	char helpStr[500];
	int error = 0;
	t_gpibDevice *d;
	t_ext_device *extdev;
	clock_t startTime;
	
	startTime = clock ();
	if (g == NULL) return 0;
	if (s == NULL) return 0;
	// 
	if (waitUntilStabilized && (!g->waitUntilStabilized)) return 0;
	
//	tprintf ("GPIB%02d transmitting command: %s.",g->gpibAddress, g->name);

	if (GPIB_showData) {
		pprintf (gpibOutputPanel, gpibOutputCtrl, 
				 "\n--- repetiton %d, command: %s ----------------------------------------------\n",
				 s->repetition+1, g->name);
	}	
	
	// If an external device is selected this first code block is evaluated.
	if (g->noGPIBdevice){
		extdev=EXTDEV_ptrByID(s,g->extDeviceID);
		if (extdev!= NULL){
			switch (extdev->deviceTyp){
				case EXTDEV_TYPE_NONE:
					break;
				case EXTDEV_TYPE_TELNET:
					if (g->commandType== GPIB_COMMANDTYPE_FREQ){
												    
						//startTime = clock(); 
		//#ifdef _CVI_DEBUG_        	
						
						EXTDEV_DDS_THREAD_transmitFrequency(s,g,extdev);
						//tprintf ("duration of EXTDEV_DDS_transmitFrequency: %1.3f s\n", timeStop_s (startTime)); 
					}
					break;
			}
		}
		// -------------------------------------------------------------------		
		//   COMMAND-TYPE: loop trigger
		// -------------------------------------------------------------------
		if (g->commandType == GPIB_LOOP_TRIGGER)
		{
			triggerLoop();
		}
		else if (g->commandType == GPIB_RECV_LOOP_TRIGGER)
		{
			recvTriggerLoop();
		}
	}
	
	else{
		
	d = GPIBDEVICE_ptr (g->gpibAddress);
	g->device = d;
	if (d == NULL) return 0;		
	// device was not yet identified
	// probably switched on later
	if (d->type == GPIBDEV_NONE) {
		if (GPIB_autoDetectDevice (config, g->device, g->gpibAddress) == 0) {
			// new device successfully detected
			CONFIG_write (config);
		}
	}
//	GPIB_transmitFreq = d->function_transmitFreq;
	
	switch (g->commandType) {
// -------------------------------------------------------------------		
//   COMMAND-TYPE: arbitrary waveform
// ------------------------------------------------------------------		
		case GPIB_COMMANDTYPE_ARBWFM:
			error = GPIB_transmitCommand_wfm (s, g);
			break;

// -------------------------------------------------------------------		
//   COMMAND-TYPE: frequency
// -------------------------------------------------------------------		
		case GPIB_COMMANDTYPE_FREQ:
            if (d->GPIB_transmitFrequency == NULL) error = GPIB_ERR_NOT_SUPPORTED;
            else error = d->GPIB_transmitFrequency (s, g, 0, 0);
//			error = GPIB_transmitCommand_freq (s, g);
			break;
// -------------------------------------------------------------------		
//   COMMAND-TYPE: command strings
// -------------------------------------------------------------------		
		case GPIB_COMMANDTYPE_STRINGS:
			error = GPIB_transmitCommandStrings (g);
			break;
// -------------------------------------------------------------------		
//   COMMAND-TYPE: power supply
// -------------------------------------------------------------------		
		case GPIB_COMMANDTYPE_POWERSUPPLY:
		    error = GPIB_transmitCommand_powerSupply (s, g, waitUntilStabilized);
			break;
// -------------------------------------------------------------------		
//   COMMAND-TYPE: power supply
// -------------------------------------------------------------------	
		case GPIB_COMMANDTYPE_PULSES:
			error = GPIB_transmitCommand_pulses (s, g);  
			break;
	}

	GPIB_setLed (0, 0);

    if (error == GPIB_ERR_NOT_SUPPORTED) {
    	sprintf (helpStr, 
    			"ERROR: GPIB command '%s' could not be transmitted.\n"
    			"GPIB device #%02d (%s) does not support the selected command type "
    			"or the device is not yet included in the device list.\n%s"
				"\nCommand is not transmitted.",
	   			g->name,
	   			g->gpibAddress, GPIBDEVICE_ptr(g->gpibAddress)->name,
	   			allowAutodetect ? "\nYou should try 'Autodetect devices' to update the device list.\n" : "");
		switch (PostGenericMessagePopup (strGpibError, helpStr,
									 allowAutodetect ? "Autodetect devices" : "",
									 "Ignore (command is not transmitted)",
									 "Abort sequence", NULL, 0, 0,
									 VAL_GENERIC_POPUP_BTN1,
									 VAL_GENERIC_POPUP_BTN1,
									 VAL_GENERIC_POPUP_BTN3)) {
			case VAL_GENERIC_POPUP_BTN1:
				if (!allowAutodetect) return -1;
				SetWaitCursor(1);
				GPIB_autodetectAllDevices (-1, -1);//panelMain, MAIN_TEXTBOX_Output2);
				SetWaitCursor(0);
				return GPIB_transmitCommand (s, g, waitUntilStabilized, 0);
			case VAL_GENERIC_POPUP_BTN2:
				error = 0;
				break;
			case VAL_GENERIC_POPUP_BTN3:
				return -1;
		}
    }
	
	if (GPIB_showData) {    
		pprintf (gpibOutputPanel, gpibOutputCtrl, "\n---- END ---- [%1.3f s]\n", timeStop_s (startTime));
	}
	}
	
ENDE:
	return error;

}





int CVICALLBACK GPIB_TEST (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
//			test_tplp ();
			break;
	}
	return 0;
}




void GPIB_displaySupportedDevices (int panel, int ctrl)
{
	int i;
	t_gpibDevice *devPtr;
	
	DeleteTextBoxLines (panel, ctrl, 0, -1);
	for (i = 1; i <= ListNumItems (GPIB_listOfAvailableDevices); i++) {
		ListGetItem (GPIB_listOfAvailableDevices, &devPtr, i);
		InsertTextBoxLine (panel, ctrl, -1, devPtr->strIDN);
	}
}





int GPIB_autodetectAllDevices (int panel, int control)
{
    t_gpibDevice *dev;
    int i;
    t_config oldConfig;

	CONFIG_init (&oldConfig);
	CONFIG_read (&oldConfig, 0);
	GPIB_resetOutput ();
	GPIBDEVICE_deleteAll (config);
	ProcessDrawEvents ();
	for (i = 1; i < config->nGpibDevices; i++) {
		dev = GPIBDEVICE_new (config);
		GPIB_autoDetectDevice (&oldConfig, dev, i);
		if ((panel > 0) && (control > 0)) {
			GPIB_fillDevicesToList (panel, control, -1);
		}
		ProcessDrawEvents ();
	}
	CONFIG_free (&oldConfig);
	CONFIG_write (config);
	GPIB_displayValues (activeSeq(), activeGpibCommandNo());
		
	
	return 0;
}



int GPIB_detectBoard (int outPanel, int outCtrl)
{
	config->gpibBoardIndex = GPIB_autodetectBoard ();
    if (config->gpibBoardIndex >= 0) {
    	if (outPanel > 0) {
        	pprintf (outPanel, outCtrl, "Found NI GPIB board (GPIB%d)\n", config->gpibBoardIndex );
		}
	}
	return 0;
}


int GPIB_feedbackEnabled (t_gpibCommand *g) 
{
	int i;
	
	for (i = 0; i < N_FEEDBACK_CHANNELS; i++)
		if (g->enableFeedback[i]) return 1;
		
	return 0;
}



int GPIB_addFrequencyFeedbackToAllCommands (t_sequence *seq, const char *path)
{
	int i;
	t_gpibCommand *g;
					
	for (i = ListNumItems (seq->lGpibCommands); i > 0; i--) {
		ListGetItem (seq->lGpibCommands, &g, i);
		if (GPIB_feedbackEnabled (g)) {
			if (path == NULL) g->frequencyFeedbackDataPath[0] = 0;
			else {
				sprintf (g->frequencyFeedbackDataPath, "%s_%s_frequencyFeedbackData.txt", path, g->name);
				strReplaceChar (g->frequencyFeedbackDataPath, '/', '_');
			}
		} else g->frequencyFeedbackDataPath[0] = 0;

	}
    return -1;
}




int GPIB_frequencyFeedbackAppendValuesToFile (t_sequence *seq, t_gpibCommand *g) 
{
	char line[1000];
	char adds[1000];
	int file;
	int i;

	if (g->frequencyFeedbackDataPath[0] == 0) return 0;
	
	file = OpenFile (g->frequencyFeedbackDataPath, VAL_WRITE_ONLY, VAL_APPEND, VAL_ASCII);
	if (file < 0) {
		displayFileError (g->frequencyFeedbackDataPath);
		return -1;
	}
	if (seq->repetition == 0) {
		sprintf (line, "repetition");
		for (i = 0; i < N_FEEDBACK_CHANNELS; i++) {
			if (g->enableFeedback[i]) {
				sprintf (adds, "\t%s\tfreqOffset%d", g->feedbackAnalogInput[i], i);
				strcat (line, adds);
			}
		}
		strReplaceChar (line, '/', '_');
		WriteLine (file, line, -1);
//		sprintf (line, " \t");
		line[0] = 0;
		for (i = 0; i < N_FEEDBACK_CHANNELS; i++) {
			if (g->enableFeedback[i]) {
				strcat (line, "\tV \t kHz");
			}
		}
		WriteLine (file, line, -1);
	}	
	
	sprintf (line, "%d", seq->repetition+1);
	for (i = 0; i < N_FEEDBACK_CHANNELS; i++) {
		if (g->enableFeedback[i]) {
			sprintf (adds, "\t%1.5f\t%1.5f", g->feedbackInputValue[i], g->feedbackFrequencyOffset[i]/1000);
			strcat (line, adds);
		}
	}
	WriteLine (file, line, -1);
	CloseFile (file);
	return 0;
	
}





int GPIB_getFeedbackInputValue (t_sequence *seq, t_gpibCommand *g)
{
	//int error = 0;
	int32 DAQmxError = DAQmxSuccess;
	clock_t startTime;
	int i;

	startTime = clock ();
	
	if (g == NULL) return 0;
	
	for (i = 0; i < N_FEEDBACK_CHANNELS; i++) g->feedbackFrequencyOffset[i] = 0;
	g->summedFeedbackFrequencyOffset = 0;


	for (i = 0; i < N_FEEDBACK_CHANNELS; i++) {
		if (g->enableFeedback[i]) {
			DAQmxError = HARDWARE_NIUSB_CreateTask_AnalogInput (&g->feedbackInputTaskHandle[i], g->feedbackAnalogInput[i]);
			if (DAQmxError != DAQmxSuccess) goto Error2;
			DAQmxErrChk(DAQmxStartTask (g->feedbackInputTaskHandle[i]));
			HARDWARE_NIUSB_readValue (g->feedbackInputTaskHandle[i], &g->feedbackInputValue[i]);
			DAQmxErrChk(DAQmxClearTask(g->feedbackInputTaskHandle[i]));	
			g->feedbackFrequencyOffset[i]= (g->feedbackInputValue[i] - g->feedbackOffset[i]) * g->feedbackFreqDeviation[i] * 1E3;
			pprintf (gpibOutputPanel, gpibOutputCtrl, "\n::::feedback: In=%1.5fV, dev=%1.5fkHz; [%1.3f s]", g->feedbackInputValue[i], g->feedbackFrequencyOffset[i]/1E3, timeStop_s (startTime));
			g->summedFeedbackFrequencyOffset += g->feedbackFrequencyOffset[i];
		}
	}

	GPIB_frequencyFeedbackAppendValuesToFile (seq, g);

	
	return 0;

	
	
Error:
	if (DAQmxError != 0) HARDWARE_DAQmx_displayError (DAQmxError);

Error2:	

	
#ifndef _CVI_DEBUG_        	
	DAQmxErrChk(DAQmxClearTask(g->feedbackInputTaskHandle[i]));	
#endif	
	return DAQmxError;
	
	
}
