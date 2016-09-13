#include <windows.h> // for sleep
#undef ERROR
#include <analysis.h>
#include "UIR_ExperimentControl.h"
//#include <dataacq.h>
#include <utility.h>
#include <userint.h>
//#include "nidaqex.h"
//#include "easyio.h"
#include <ansi_c.h>
//#include <NIDAQmx.h>


#include "INTERFACE_TCP_IP.h"
#include "INCLUDES_CONTROL.h"    
#include "CONTROL_HARDWARE.h"
#include "CONTROL_HARDWARE_GPIB.h"
#include "CONTROL_HARDWARE_DAQmx.h"
#include "CONTROL_HARDWARE_Adwin.h"    
#include "CONTROL_GUI_MAIN.h"
#include "CONTROL_GUI_LOOP_CONF.h"


// set counter Delay in units of 50 ns
// to account for propagation times in cables etc
#define COUNTER_DELAY 0
#define WATCHDOG_ERR -3
#define OUTPUT_20MHZ_TO_PFI3 0
#define NO_OUTPUT_DEVICE -4

extern clock_t mytime;
//int speedMode = 1;
#define REFRESH_RATE 10

extern int transmit_CMD_READY = 0;

#ifdef EXPERIMENTCONTROL
  //#define ignoreNoHardwareDetected 1
  #define ignoreNoHardwareDetected 0
#else
  #define ignoreNoHardwareDetected 0
#endif



#define SHOW_HARDWARE_PROTOCOL  0


//#define DISABLE_DOUBLE_BUFFERING 0



#define DIO_BUFFER_SIZE_SAMPLES (4*1024*1024)	   // * 4       
#define AO_BUFFER_SIZE_SAMPLES (2*1024*1024)	   // * 2*8       
#define NoError 0 //Sebastian I'm not sure if this works

unsigned globalSum = 0;
int TEST_panelGraph = -1;
const char TEST_wfmName[] = "DIG STEP";

int analogBoardType = ID_PCI_6713; // default 
								   // funciton AUTODETECT Hardware may change this


volatile int interruptDataTransfer = 0;



double globalAnalogVoltages[N_DAC_CHANNELS];
int globalDigitalValues[N_TOTAL_DIO_CHANNELS];

const int HARDWARE_TRIGGER_SOURCE[N_TRIGGER_SOURCES] = {
	 VAL_NO_TRIGGER, 
//	 ND_PFI_0,
//	 ND_PFI_1,
//	 ND_PFI_2,
	 //ND_PFI_3,
//	 ND_PFI_4,

//	 ND_PFI_5,
//	 ND_PFI_6,
//	 ND_PFI_7,
//	 ND_PFI_8,
//	 ND_PFI_9,
     
//	 RTSI_START_TRIG,
	 ADWIN_TRIGGER_DIGIO0
 //	 ADWIN_TRIGGER_DIGIO1
};


extern int gpibOutputPanel;
extern int gpibOutputCtrl;



//=======================================================================
//    test if analog channel is active
//=======================================================================
 int HARDWARE_isAnalogChannelActive (t_sequence *seq, int channel)
{
	
	if (channel < 8) return 1;
	// channel < 16 okay, if second board PCI 6713 detected
	if (channel < N_DAC_CHANNELS) return nAOBoards() == 2;

	return 0;
}





void HARDWARE_sendStoredValues (t_sequence *seq)
{
	int i;

    for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		if (HARDWARE_sendDirectDigitalValue (i, (short) globalDigitalValues[i]) != 0) return;	
	}

	if (seq == NULL) return;
	
	for (i = 0; i < N_DAC_CHANNELS; i++) {
	   if (HARDWARE_sendDirectAnalogValue (i, globalAnalogVoltages[i]) != 0) return;
	}
    
	
}

//=======================================================================
//
//    store the latest applied voltages
//
//=======================================================================

void HARDWARE_rememberLastValues (t_sequence *seq)
{
    int i;
    int n;
    int nBlocks;
	int device;
	t_analogSample *sample;
	t_analogSample *sampleArr;
	t_adwinData *ad;

    if (seq->outData == NULL) return;
    for (i = 0; i < N_DAC_CHANNELS; i++) {
		device = i / 8;
    	n = seq->outData->AO_nSamples[device];
		globalAnalogVoltages[i] = 0;         
		switch (config->hardwareType) {
			case HARDWARE_TYPE_NATINST:
				sampleArr = seq->outData->AO_Samples[device];
				if ((sampleArr != NULL) && (n > 0)) {
					sample = &sampleArr[n-1]; 
					globalAnalogVoltages[i] = DigitalToAnalog (sample->value[i % 8], NULL);
				}
				break;
			case HARDWARE_TYPE_ADWIN:
				ad = seq->outData->adwinData;
				if (ad != NULL) globalAnalogVoltages[i] = DigitalToAnalog (ad->AOvaluesAtEndOfSequence[i], digitizeParameters_ADWIN ());
				break;
		}
	}
    
    nBlocks = ListNumItems (seq->lDigitalBlocks);
    for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
    	switch (seq->DIO_channelStatus[i]) {
    		case DIO_CHANNELSTATUS_ALWAYSON:
    			globalDigitalValues[i] = 1 ^ seq->DIO_invertDisplay[i];
    			break;
    		case DIO_CHANNELSTATUS_ALWAYSOFF:
    			globalDigitalValues[i] = 0 ^ seq->DIO_invertDisplay[i];
    			break;
    		default:
        		globalDigitalValues[i] = DIGITALBLOCK_ptr(seq, nBlocks)->channels[i];
        }
    }
    
	DIGITALBLOCKS_displayAllDirectIOButtons  (seq);
	HARDWARE_sendStoredValues (seq);
}



//=======================================================================
//
//    reset voltages
//
//=======================================================================
void HARDWARE_resetStoredValues (void)
{
    int i;
    
    for (i = 0; i < N_DAC_CHANNELS; i++) globalAnalogVoltages[i] = 0;
    for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) globalDigitalValues[i] = 0;

//	sendStoredValues ();
}





/************************************************************************/
/*
/*    general functions 
/*
/************************************************************************/


void HARDWARE_setTransmit_CMD_READY (int state)
{
	transmit_CMD_READY = state;	
	
}






/*
int watchdogOn (void)
{
	int iStatus;
	short lineState;

	if (config->watchdogChannel < 0) return 0;
	iStatus = DIG_In_Line (deviceNrAO(0), 0, config->watchdogChannel, &lineState);
//	ReadFromDigitalLine (deviceNrAO(0), "0", config->watchdogChannel, 8, 0, state);
	displayErrorMSG (iStatus, deviceNrAO(0), "DIG_In_Line (watchdog)"); 
	SetCtrlVal (panelOutput(), OUTPUT_LED_watchdog, lineState);

	return lineState;
}
*/


//=======================================================================
//
//    check if hardware was detected
//
//=======================================================================
int hardwareDetected (void)
{
   return (deviceNrAO(0) >= 0) || (deviceNrDIO(0) >= 0) || (ignoreNoHardwareDetected) || (config->hardwareType == HARDWARE_TYPE_ADWIN);
}


//=======================================================================
//
//    check if sequence has non-zero duration
//
//=======================================================================
/*int outputDurationNull (t_outputData *k)
{
	return (k->durationLo == 0) && (k->durationHi == 0);
}
*/


//=======================================================================
//
//    get name of board
//
//=======================================================================
//char *get_NIDeviceName (int deviceID)
//{
//	#define N_DEVICE_NAMES       6
//	
//	static char *NI_Device_Name[N_DEVICE_NAMES] = 
//	    {"Invalid Device", 
//	     "PCI-DIO-32-HS",
//	     "PCI-6713",
//	     "PCI-6534",
//	     "PCI-6602",
//	     "PCI-6733"
//	     };
//
//	static short NI_Device_IDs[N_DEVICE_NAMES] = 
//	    { ID_INVALID_DEVICE, 
//	      ID_PCI_DIO_32_HS,
//	      ID_PCI_6713,
//		  ID_PCI_6534,
//		  ID_PCI_6602,
//		  ID_PCI_6733
//	};
//
//
//	static char help[30];
//    
//    int i;
//    
//    for (i = 0; i < N_DEVICE_NAMES; i++) {
//        if (deviceID == NI_Device_IDs[i]) return NI_Device_Name[i];
//    }
//	sprintf (help, "unknown device (%d)", deviceID);
//    return help;
//}



/************************************************************************/
/*
/*    error handling
/*
/************************************************************************/

//=======================================================================
//
//    create title for error messages 
//
//=======================================================================
const char *createErrorMSGTitle (int iStatus)
{
    static char strTitle[20];

    sprintf(strTitle, "NI-DAQ %s", (iStatus > 0)? "warning":"error");
	return strTitle;
}

//=======================================================================
//
//    create error message containing function- and board name
//
//=======================================================================

char *createErrorMSG (int iStatus, int deviceNr, char *strFuncName)
{
   char *strMsg = 0;

   if (iStatus != NoError)
   {
	strMsg = (char *) malloc(1000);

	// -----------------------------------------
	//    append board name and function name
	// -----------------------------------------
   sprintf (strMsg,
			"Board #%d (%s), [%s] returned %s %d.\n\nError description:\n",
			deviceNr, getBoardName(deviceNr), strFuncName, createErrorMSGTitle(iStatus), iStatus);
// -----------------------------------------
//    append error description
	// -----------------------------------------
    //   strcat (strMsg, (char*) GetNIDAQErrorString (iStatus));
	// -----------------------------------------
	//    break lines
	// -----------------------------------------
	strBreakLines (strMsg, 120);
           
   } 
   return strMsg;
}


//=======================================================================
//
//    handle NI-DAQ errors and warnings from boards
//    (a dialog box is displayed, if an error occured)
//
//    INPUT:    (i16) iStatus - status code returned from NI-DAQ function
//             (char*) strFuncName - function name associated with status
//
//    OUTPUTS:        0 no error (nidaqExRetType) 
//              iStatus  if error occured
//=======================================================================
//int errorCheck(int iStatus, int deviceNr, char *strFuncName)
//{
//    char *strMsg;
//
//    if (iStatus != NoError)
//    {
//		// -----------------------------------------
//		//    create error message
//		// -----------------------------------------
//		strMsg = createErrorMSG(iStatus, deviceNr, strFuncName);
//		strcat (strMsg, "\n\nData output is halted.\n\n");
//		strcat (strMsg, "Boards will be reconfigured.\n");
//		// -----------------------------------------
//		//    display message + initialize boards
//		// -----------------------------------------
//		//MessagePopup (createErrorMSGTitle(iStatus), strMsg);
//		PostMessagePopup(createErrorMSGTitle(iStatus), strMsg);  
//		
//		
//		HARDWARE_initializeBoards ();
//    }
//    return iStatus;
//}



void CVICALLBACK MainThread_MessageCB (void *mesg)
{
    if (mesg != NULL) PostMessagePopup ("Error", mesg);
	free (mesg);
	HARDWARE_SetGetInterruptedFlag (1);
}



int cmtErrorCheck(int iStatus, int deviceNr, char *strFuncName)
{
    char *strMsg;

    if (iStatus != NoError)
    {
		// -----------------------------------------
		//    create error message
		// -----------------------------------------
		strMsg = createErrorMSG(iStatus, deviceNr, strFuncName);
		// -----------------------------------------
		//    display message 
		// -----------------------------------------
        PostDeferredCallToThread (MainThread_MessageCB, strMsg, CmtGetMainThreadID ());
			
//		PostMessagePopup (createErrorMSGTitle(iStatus), strMsg);
    }
    return iStatus;
}



int UpdateNPoints (int iStatus, int deviceNr, char *strFuncName)
{
    char *strMsg;

    if (iStatus != NoError)
    {
		// -----------------------------------------
		//    create error message
		// -----------------------------------------
		strMsg = createErrorMSG(iStatus, deviceNr, strFuncName);
		// -----------------------------------------
		//    display message 
		// -----------------------------------------
        PostDeferredCallToThread (MainThread_MessageCB, strMsg, CmtGetMainThreadID ());
			
//		PostMessagePopup (createErrorMSGTitle(iStatus), strMsg);
    }
    return iStatus;
}


//=======================================================================
//
//    display error message 
//
//=======================================================================
int displayErrorMSG(int iStatus, int deviceNr, char *strFuncName)
{
    if (iStatus != NoError)
	   PostMessagePopup (createErrorMSGTitle(iStatus),
					 createErrorMSG(iStatus, deviceNr, strFuncName));
    return iStatus;
}



//=======================================================================
//
//    display error message 
//
//=======================================================================
int displayCmtErrorMSG (int cmtStatusCode, char *strFuncName)
{
    char cmtErrorMsg[CMT_MAX_MESSAGE_BUF_SIZE];
	
	if (cmtStatusCode != 0) {
		CmtGetErrorMessage (cmtStatusCode, cmtErrorMsg);
		
		PostMessagePopupf ("Error" 
		   			  "Multithread function '%s'\nreturned error code %d:\n%s.",
					  strFuncName, cmtStatusCode, cmtErrorMsg);
	}		   
    return cmtStatusCode;
}





/************************************************************************/
/*
/*    direct I/O 
/*    (directly access digital and analog output channes)
/*
/************************************************************************/


//=======================================================================
//
//    write value to digital channel
//
//=======================================================================
int HARDWARE_sendDirectDigitalValue (int ch, short bit)
{
    //char *ports[4] = {"0", "1", "2", "3"};
    int iStatus = 0;
	//int device = 0;
    
    if (!hardwareDetected()) return 0;

	
	switch (config->hardwareType) {
//		case HARDWARE_TYPE_NATINST:
//			DisableBreakOnLibraryErrors ();
//		    if (config->invertDigitalOutput) bit = !bit;
//    
//			if ((ch < 0) || (ch > N_TOTAL_DIO_CHANNELS)) return 0;
//			device = ch / N_BOARD_DIO_CHANNELS;
//			ch %= N_BOARD_DIO_CHANNELS;
//		
//			if (deviceNrDIO(device) < 0) return 0;   
//	
//			iStatus = DIG_Grp_Config (deviceNrDIO(device), 1, 0, 0, 1);
//			if (iStatus == -10412) iStatus = 0;
//		    if (errorCheck(iStatus, deviceNrDIO(device), "DIG_Grp_Config")) return iStatus;
//	
//			iStatus = WriteToDigitalLine (deviceNrDIO(device), ports[ch / 8], ch % 8, 8,
//										  0, bit);
//			displayErrorMSG (iStatus, deviceNrDIO(device), "WriteToDigitalLine"); 
//			break;
		case HARDWARE_TYPE_ADWIN: 
			iStatus = HARDWARE_ADWIN_sendDirectDigitalValue (ch, bit);
			break;
	}
	
	return iStatus;
}
    
    		  
//=======================================================================
//
//    write value to analog channel
//
//=======================================================================
int HARDWARE_sendDirectAnalogValue (int ch, double analogValue)
{
    //char *ports[N_DAC_CHANNELS] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    int iStatus = 0;
    //int boardNr = 0;

    if (!hardwareDetected()) return 0;
	
	switch (config->hardwareType) {
//		case HARDWARE_TYPE_NATINST:
//			boardNr = ch / 8;
//			ch = ch % 8;
//	
//			// -----------------------------------------
//			//    write analog value
//			// -----------------------------------------
//			if (analogValue > 10.0) analogValue = 10.0;
//			if (analogValue < -10.0) analogValue = -10.0;
//			if (deviceNrAO(boardNr) < 0) return 0;
//			iStatus = AOUpdateChannel (deviceNrAO(boardNr), ports[ch], analogValue);
//			// -----------------------------------------
//			//    ignore "bad range error" 
//			// -----------------------------------------
//			if (iStatus != 10012) {
//				displayErrorMSG (iStatus, deviceNrAO(boardNr), "AOUpdateChannel"); 
//				return -1;
//			}
//			break;
		case HARDWARE_TYPE_ADWIN:
			iStatus = HARDWARE_ADWIN_sendDirectAnalogValue (ch, analogValue);
			break;
	}
	return iStatus;
	
}


















/**********************************************************************************/
/**********************************************************************************/
/*
/*   synchronize internal 20 MHz clocks between different boards
/*
/**********************************************************************************/
/**********************************************************************************/
   
/*==================================================================================

 FUNCTION: synchronize20MHzClocks
 
 The internal 20 MHz clock of the digital board will be the reference for 
 the other boards. It is transmitted on the RTSI_CLOCK signal line.

 (notes: functions "RTSI_Conn", "RTSI_Disconn", "RTSI_Clock"
 	  	 work for digital (DIO) board (PCI 6533 / PCI DIO-32-HS)
 		 and are NOT supported for analog (AO) board (PCI 6713, E-Series) 
         --> use "Select_Signal" instead (see NI-DAQ help, "RTSI_Conn")
        
 		 "Select_signal" (device, signal, source, polarity):
  		 ND_OUT_START_TRIGGER, ND_OUT_UPDATE, and ND_UPDATE_CLOCK_TIMEBASE 
         values do not apply to the E Series devices.
		 --> the only version that works is:
 		 	 Select_Signal (deviceNrAO(0), ND_RTSI_CLOCK, ND_BOARD_CLOCK, ND_DONT_CARE);
 		
 		 Board_clock can only be transmitted onto ND_RTSI_CLOCK signal line
 		 (not RTSI_1 .. RTSI_7)

/*==================================================================================*/

//int HARDWARE_synchronize20MHzClocks (t_sequence *seq)
//{
//    int iStatus;
//    int i;
//	int device;
//    
//	// ---------------------------------------------------------------------------
//	//    AO boards: disconnect all clock signals from RTSI_CLOCK 
//	// ---------------------------------------------------------------------------
//	for (i = 0; i < nAOBoards(); i++) {
//		iStatus = Select_Signal (deviceNrAO(i), ND_RTSI_CLOCK, ND_NONE,
//								 ND_DONT_CARE);
//	    if (errorCheck (iStatus, deviceNrAO(i), "Select_Signal (disconnect clock)")) 
//	    	return iStatus;
//	}
//	// ---------------------------------------------------------------------------
//	//    DIO board: disconnect clock signals from RTSI_CLOCK 
//	// ---------------------------------------------------------------------------
//	iStatus = RTSI_Clock (deviceNrDIO(0), 0, 0);
//    if (errorCheck (iStatus, deviceNrDIO(0), "RTSI_Clock (disconnect)")) return iStatus;
//
//	
//	
//	// ---------------------------------------------------------------------------
//	// 	  DIO board 0: send clock to RTSI_CLOCK
//	// ---------------------------------------------------------------------------
//	iStatus = RTSI_Clock (deviceNrDIO(0), 1, 1);
//    if (errorCheck (iStatus, deviceNrDIO(0), "RTSI_Clock (send clock signal to RTSI_CLOCK)")) return iStatus;
//	
//	// ---------------------------------------------------------------------------
//	// 	  all other DIO boards: receive RTSI_CLOCK
//	// ---------------------------------------------------------------------------
//	for (device = 1; device < nDIOBoards(); device++) {
//		iStatus = RTSI_Clock (deviceNrDIO(device), 1, 0);
//	    if (errorCheck (iStatus, deviceNrDIO(device), "RTSI_Clock (receive RTSI_CLOCK)")) return iStatus;
//	}
//	// ---------------------------------------------------------------------------
//	//    AO-boards: receive clock from RTSI_CLOCK 
//	// ---------------------------------------------------------------------------
//	for (i = 0; i < nAOBoards(); i++) {
//		iStatus = Select_Signal (deviceNrAO(i), ND_BOARD_CLOCK, ND_RTSI_CLOCK,
//								 ND_DONT_CARE);
//	    if (errorCheck (iStatus, deviceNrAO(i), "Select_Signal (receive clock signal from RTSI_CLOCK)")) return iStatus;
//	}
//
//
////	iStatus = COUNTER_setBoardClockToRTSI ();
//	
///*  alternative solution: AO board is master oscillator
//    // Analog Board: transmit clock on RTSI_CLOCK 
//	iStatus = Select_Signal (deviceNrAO(0), ND_RTSI_CLOCK, ND_BOARD_CLOCK, ND_DONT_CARE);
//    if (errorCheck (iStatus, "Select_Signal (send clock TO RTSI_CLOCK)")) return iStatus;
//	// Digital Board: receive clock from RTSI_CLOCK 
//	iStatus = RTSI_Clock (deviceNrDIO(0), 1, 0);
//    if (errorCheck (iStatus, "RTSI_Clock (receive clock from RTSI_CLOCK)")) return iStatus;
//*/
///*	iStatus = Select_Signal (deviceNrAO(0), ND_FREQ_OUT,
//							 ND_INTERNAL_MAX_TIMEBASE, 1);
//	if (errorCheck (iStatus, deviceNrAO(i), "Select_Signal (send Clock to PFI_0)")) {
////		QuitUserInterface (0);
//		return iStatus;
//	}
//*/	
//
//
//    return iStatus;
//}






/**********************************************************************************/
/**********************************************************************************/
/*
/*   synchronize the different boards
/*
/**********************************************************************************/
/**********************************************************************************/

#define VAL_DIO_REQ1 0
#define VAL_DIO_REQ2 1
#define VAL_DIO_ACK1 2
#define VAL_DIO_ACK2 3
#define VAL_DIO_STOPTRIG1 4
#define VAL_DIO_STOPTRIG2 5
#define VAL_DIO_PCLK1 6

/*==================================================================================

   FUNCTION: synchronizeStartStop()
   

   a) starting the sequence
	   	concept: AO-board #1 starts AO-board #2 and DIO-Board
            
	    ND_OUT_START_TRIGGER (from AO-board #1)  
	    	is transmitted to  
	    RTSI_START_TRIG  (= RTSI_5)
    
	    RTSI_START_TRIG       
	        is received by
	    ACK1 (DIO-board) and ND_OUT_START_TRIGGER (AO-board #2)
            
            
    b) stopping the sequence: 
            RTSI_STOP_TRIG (= RTSI_4) is received by VAL_DIO_STOPTRIG1 (DIO-board)
            (AO boards do not require stop trigger, whereas DIO boards needs stop 
             trigger, because it sometimes plays a sequence twice, might be a bug of NI)
		   
		    the stop signal is generated by the general purpose counter (GPCTR) of
		    AO board #1 (see function "configureGPCTR()")

/*==================================================================================*/



//int HARDWARE_synchronizeStartStop (t_sequence *seq)
//{
//    int iStatus;
//    int waitForStopTrigger = 0;
//	int startTriggerSource = 1; // hardware Trigger 
//	int device;
//
//	// ---------------------------------------------------------------------------
//	//    DIO-board: clear all old RTSI connections
//	// ---------------------------------------------------------------------------
//	for (device = 0; device < nDIOBoards(); device++) {
//		iStatus = RTSI_Clear (deviceNrDIO(device));
//		if (errorCheck (iStatus, deviceNrDIO(device), "RTSI_Clear")) return iStatus;
//	// ---------------------------------------------------------------------------
//	//    DIO-board: configure trigger: enable hardware trigger
//	// ---------------------------------------------------------------------------
//		if (seq->outData->DIO_dblBuf) {
//			waitForStopTrigger = 0;
//			startTriggerSource = 1; // software
//		}
//		iStatus = DIG_Trigger_Config (deviceNrDIO(device), 1,
//									  startTriggerSource, 0,
//									  waitForStopTrigger, 0, 2, 0, 0);
//	    if (errorCheck (iStatus, deviceNrDIO(device), "DIG_Trigger_Config")) return iStatus;
//	}
//	
//	if (seq->trigger != VAL_NO_TRIGGER) {
//		// ---------------------------------------------------------------------------
//		//    AO-board #1: EXTERNAL trigger (PFI n)
//		// ---------------------------------------------------------------------------
//		iStatus = Select_Signal (deviceNrAO(0), ND_OUT_START_TRIGGER, seq->trigger, ND_LOW_TO_HIGH);
//	    if (errorCheck (iStatus, deviceNrAO(0), "Select_Signal (set external Trigger)")) return iStatus;
//	}
//	else {
//		// ---------------------------------------------------------------------------
//		//    AO-board #1: INTERNAL trigger
//		 // ---------------------------------------------------------------------------
//		iStatus = Select_Signal (deviceNrAO(0), ND_OUT_START_TRIGGER, ND_AUTOMATIC, ND_LOW_TO_HIGH);
//	    if (errorCheck (iStatus, deviceNrAO(0), "Select_Signal (set external Trigger)")) return iStatus;
//	}
//
//
//	// ---------------------------------------------------------------------------
//	//    AO-board #1: route "start" (ND_OUT_START_TRIGGER) to RTSI_START_TRIG 
//	// ---------------------------------------------------------------------------
//    iStatus = Select_Signal (deviceNrAO(0), RTSI_START_TRIG, ND_OUT_START_TRIGGER,
//                             ND_LOW_TO_HIGH);
//    if (errorCheck (iStatus, deviceNrAO(0), "Select_Signal (send START_TRIGGER)")) return iStatus;
//
//	
//	// ---------------------------------------------------------------------------
//	//    DIO-board: use RTSI_START_TRIG  as "start" trigger (ACK1)
//	// ---------------------------------------------------------------------------
//	for (device = 0; device < nDIOBoards(); device++) {
//	    iStatus = RTSI_Conn (deviceNrDIO(device), VAL_DIO_ACK1, RTSI_START_TRIG_NR, 0);
//	    if (errorCheck (iStatus, deviceNrDIO(device), "RTSI_Conn (receive ACK1)")) return iStatus;
//		// ---------------------------------------------------------------------------
//		//    DIO-board: use RTSI_STOP_TRIG  as "stop" trigger
//		// ---------------------------------------------------------------------------
//   		if (!seq->outData->DIO_dblBuf) {
//			iStatus = RTSI_Conn (deviceNrDIO(device), VAL_DIO_STOPTRIG1, RTSI_STOP_TRIG_NR, 0);
//		    if (errorCheck (iStatus, deviceNrDIO(device), "RTSI_Conn (receive ACK1)")) return iStatus;
//		}
//	}
//	// ---------------------------------------------------------------------------
//	//    AO-board #2: use RTSI_START_TRIG as "start" trigger
//	// ---------------------------------------------------------------------------
//    if (nAOBoards() == 2) {
//	    iStatus = Select_Signal (deviceNrAO(1), ND_OUT_START_TRIGGER, RTSI_START_TRIG, 
//	                           ND_LOW_TO_HIGH);
//	    if (errorCheck (iStatus, deviceNrAO(1), "Select_Signal (receive ND_OUT_START_TRIGGER Board 2)")) return iStatus;
//	}
//	// ---------------------------------------------------------------------------
//	//    AO-board #1: display start trigger at PFI_1
//	// ---------------------------------------------------------------------------
//	if (seq->trigger == VAL_NO_TRIGGER) {
////		iStatus = Select_Signal (deviceNrAO(0), ND_PFI_2, RTSI_START_TRIG, ND_DONT_CARE);
//						
//					
//	}
//    return 0;
//}



//==================================================================================
//  
//    configure DIO board 
//
//==================================================================================*/
//int HARDWARE_configureDigitalOutput (t_sequence *seq)
//{			 
//    int iStatus = 0;
//    //i32 lTimeout = 10;
//	int device;
//
////    iStatus = Set_DAQ_Device_Info (deviceNrDIO(0), ND_DATA_XFER_MODE_DIO_GR1,
////								   ND_UP_TO_1_DMA_CHANNEL);
////    if (errorCheck(iStatus, deviceNrDIO(0), "Set_DAQ_Device_Info")) return iStatus;
//
//	if (deviceNrDIO(0) < 0) return 0;
//	for (device = 0; device < nDIOBoards(); device++) {
//		// -----------------------------------------------------------
//	    // 		configure group of ports as output, with handshaking
//		// -----------------------------------------------------------
//	    iStatus = DIG_Grp_Config (deviceNrDIO(device), 1, N_OUTPUT_PORTS, 0, 1);
//	    if (errorCheck(iStatus, deviceNrDIO(device), "DIG_Grp_Config")) return iStatus;
//
//		// -----------------------------------------------------------
//	    // 		configure internally timed pattern generation with timebase
//	    //      and no external gating
//		// -----------------------------------------------------------
//		iStatus = DIG_Block_PG_Config (deviceNrDIO(device), 1,
//									   seq->outData->DIO_dblBuf ? 1 : 2, 0,
//									   -3, seq->DIO_timebase_50ns, 0);
//	    if (errorCheck (iStatus, deviceNrDIO(device), "DIG_Block_PG_Config")) return iStatus;
//		// -----------------------------------------------------------
//	    // 		configure double buffered output
//		// -----------------------------------------------------------
//		if (seq->outData->DIO_dblBuf) {
//			iStatus = DIG_DB_Config (deviceNrDIO(device), 1, 1, 1, 1);
//			if (errorCheck (iStatus, deviceNrDIO(device), "DIG_DB_Config")) return iStatus;
//		}
//
//	}
//	// -----------------------------------------------------------
//    // 		synchonize boards
//	// -----------------------------------------------------------
//    
//	
//	if (deviceNrAO(0) >= 0) {
//        iStatus = HARDWARE_synchronizeStartStop (seq);
//        if (iStatus == 0) iStatus = HARDWARE_synchronize20MHzClocks (seq);
//    }
//
//    return iStatus;
//}



//==================================================================================
//  
//    configure AO boards
//
//==================================================================================*/
//int HARDWARE_configureAnalogOutput (t_outputData *out)
//{
//    int iStatus;
//    int i;
//
//	if (!hardwareDetected ()) return 0;
//	
//	for (i = 0; i < nAOBoards(); i++) {
///*		// -----------------------------------------------------------
//	    // 		enable double buffered output
//		// -----------------------------------------------------------
//		iStatus = WFM_DB_Config (deviceNrAO(i), out->AOnChannels[i], out->AOchannelVect[i], out->AO_dblBuf, 1, 1);
//	    if (errorCheck (iStatus, deviceNrAO(i), "WFM_DB_Config")) return iStatus;
//*/		
//		// -----------------------------------------------------------
//	    // 		join all channels to a group
//		// -----------------------------------------------------------
//	    iStatus = WFM_Group_Setup (deviceNrAO(i), 
//	    	out->AOnChannels[i], out->AOchannelVect[i], 1);
//	    if (errorCheck (iStatus, deviceNrAO(i), "WFM_Group_Setup")) return iStatus;
//	}
//
//	return 0;
//
//}







/**********************************************************************************
/*
/*   initalization of boards (used after a the data output failed)
/*
/**********************************************************************************/



//int HARDWARE_setDigitalOutputDirection (void)
//{
//	int port;
//	int iStatus;
//	int device;
//
//	if (deviceNrDIO(0) < 0) return 0;
//	for (device = 0; device < nDIOBoards(); device++) {
//		for (port = 0; port < 4; port++) {
//			iStatus = DIG_Prt_Config (deviceNrDIO(device), port, 0, 1);
//			displayErrorMSG (iStatus, deviceNrDIO(device), "DIG_Prt_Config");    
//			if (iStatus != 0) return -1;
//		}
//	}
//	return 0;
//}

//==================================================================================
//  
//    initializes all boards
//
//==================================================================================*/
//int HARDWARE_NI_initializeBoards (void)
//{
//    short board;
//    int i;
//	int device;
//	int status;
//	
//	// -----------------------------------------------------------
//    // 		AO-boards
//	// -----------------------------------------------------------
//	for (i = 0; i < nAOBoards(); i++) {
//        tprintf ("Initializing DAQ-board #%d. ", deviceNrAO(i));
//		ProcessDrawEvents ();
//    	status = Init_DA_Brds (deviceNrAO(i), &board);
//    	tprintf ("Board type: %s (%d)\n", get_NIDeviceName(board), board);
////		if (status == 0) {
////			analogBoardType = board;	
////		}
//		ProcessDrawEvents ();
//	}
//	// -----------------------------------------------------------
//    // 		DIO-boards
//	// -----------------------------------------------------------
//	for (device = 0; device < nDIOBoards(); device++) {
//	    tprintf ("Initializing DIO-board #%d. ", deviceNrDIO(device));
//		ProcessDrawEvents ();
//	    Init_DA_Brds (deviceNrDIO(device), &board);
//	    tprintf ("Board type: %s (%d)\n", get_NIDeviceName(board), board);
//		ProcessDrawEvents ();
//	}
//    
//	HARDWARE_resetStoredValues ();
//	HARDWARE_setDigitalOutputDirection ();
//	HARDWARE_sendStoredValues(NULL);
//
//    return 0;
//}   


int HARDWARE_initializeBoards (void)
{
	switch (config->hardwareType) {
		case HARDWARE_TYPE_ADWIN: 
			if (HARDWARE_ADWIN_boot (0,0) != 0) return -1;
			HARDWARE_ADWIN_addConfig ();
//		case HARDWARE_TYPE_NATINST:
//			return HARDWARE_NI_initializeBoards ();
	}
	
	return 0;
		
}



int HARDWARE_configureOutputs (t_sequence *seq)
{
	switch (config->hardwareType) {
		case HARDWARE_TYPE_ADWIN:
			return HARDWARE_ADWIN_configureOutput ();
//		case HARDWARE_TYPE_NATINST:
//		 	if (HARDWARE_configureAnalogOutput (seq->outData) != 0) return -1;
//    		return HARDWARE_configureDigitalOutput (seq);
	}	

	return -1;

}		

int HARDWARE_resetOutputs (t_sequence *seq)
{
	switch (config->hardwareType) {
//		case HARDWARE_TYPE_NATINST: 
//			HARDWARE_resetDigitalOutput (seq);
//			return HARDWARE_resetAnalogOutput (seq);
	}
	return 0;
}






//=======================================================================
//
// 	  returns digitize parameters for NI-card PCI 6713
//
//    (timebase is in units of 50ns)
//
//=======================================================================
t_digitizeParameters *digitizeParameters_PCI67XX (int timebase)
{
	static t_digitizeParameters p;
	static int firstcall = 1;
	int status = -1;
	//unsigned long boardType;
	//int device;
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN) {
	 	p.timebase_50ns	= timebase;
		p.minVoltage 	=  -10.0;
		p.maxVoltage 	=  10.0;
		p.Vpp 			= p.maxVoltage - p.minVoltage;
		p.minDigital 	= -32768;  // for adwin
		p.maxDigital 	= 32767;
		p.Dpp           = p.maxDigital - p.minDigital;
		p.nPoints       = 0;
		return &p;
	}
	
	if (firstcall) {
		if (config->hardwareType == HARDWARE_TYPE_NATINST) {
			/*device = deviceNrAO(0);
			if (device > 0) status = Get_DAQ_Device_Info (device, ND_DEVICE_TYPE_CODE, &boardType);
			//errorCheck(status, deviceNrAO(0), "Get_DAQ_Device_Info");
			if (device > 0 && status == 0) analogBoardType = boardType;
			else analogBoardType = ID_PCI_6733;*/
		}
		firstcall = 0;	
	}
	
	p.offsetCorrection = 0;
	switch (analogBoardType) {
		case ID_PCI_6713:
			p.timebase_50ns	= timebase;
			// set max and min voltage for analog values
			p.minVoltage 	=  -10.0;
			p.maxVoltage 	=  10.0;
			p.Vpp 			= p.maxVoltage - p.minVoltage;
			// set max and min digital output values
			p.minDigital 	= -2047;  // for PCI6713
			p.maxDigital 	= 2047;
			p.Dpp           = p.maxDigital - p.minDigital;
			// reset number of points 
			// (nPoints != 0) wouls enforce digitalization with n points
			// (not used for NI boards)
			p.nPoints       = 0;
			break;
		case ID_PCI_6733:
			p.timebase_50ns	= timebase;
			p.minVoltage 	=  -10.0;
			p.maxVoltage 	=  10.0;
			p.Vpp 			= p.maxVoltage - p.minVoltage;
			p.minDigital 	= -32768;  // for PCI6733
			p.maxDigital 	= 32767;
			p.Dpp           = p.maxDigital - p.minDigital;
			p.nPoints       = 0;
			break;
		default:
			p.timebase_50ns	= timebase;
			p.minVoltage 	=  -10.0;
			p.maxVoltage 	=  10.0;
			p.Vpp 			= p.maxVoltage - p.minVoltage;
			p.minDigital 	= -32768;  // for PCI6733
			p.maxDigital 	= 32767;
			p.Dpp           = p.maxDigital - p.minDigital;
			p.nPoints       = 0;
	}

	return &p;	
}




//==================================================================================
//  
//    reset DIO-board (after execution of sequence)
//
//==================================================================================*/
//int HARDWARE_resetDigitalOutput (t_sequence *seq)
//{
//    int iStatus = 0;
//	int device;
//        
//    if (deviceNrDIO(0) < 0) return 0;
//	// -----------------------------------------------------------
//    // 	    disable timeouts
//	// -----------------------------------------------------------
//	for (device = 0; device < nDIOBoards(); device++) {
//	    iStatus = Timeout_Config(deviceNrDIO(device), -1);
//	    if (errorCheck (iStatus, deviceNrDIO(device), "Timeout_Config")) return iStatus;
//		// -----------------------------------------------------------
//	    // 	    reset output groups
//		// -----------------------------------------------------------
//		iStatus = DIG_Grp_Config (deviceNrDIO(device), 1, 0, 0, 1);
//		if (iStatus == -10412) iStatus = 0;
//		if (iStatus == -10609) iStatus = 0;
//	    if (errorCheck(iStatus, deviceNrDIO(device), "DIG_Grp_Config")) return iStatus;
//	}
//    
//    return iStatus;
//}



//==================================================================================
//  
//    reset AO-board (after execution of sequence)
//
//==================================================================================
//int HARDWARE_resetAnalogOutput (t_sequence *seq)
//{
//    int iStatus;
//	int i;
//        
//    for (i = 0; i < nAOBoards(); i++) {
//		// -----------------------------------------------------------
//	    // 	    reset GPCTR
//		// -----------------------------------------------------------
//	    iStatus = GPCTR_Control(deviceNrAO(i), ND_COUNTER_0, ND_RESET);
//	    if (errorCheck (iStatus, deviceNrAO(i), "GPCTR_Control/RESET")) return iStatus;
//	}
//    
//    return 0;
//}






void HARDWARE_sendAllGpibCommands_setFlag (int *commandTransmitted) 
{
	if (*commandTransmitted == 0) {
		*commandTransmitted = 1;
		if (!config->suppressProtocol) {
			tprintf ("Transmitting GPIB commands... ");
			ProcessDrawEvents ();
		}
	}
}
		


//==================================================================================
//  
//    send all GPIB commands
//    (subroutine of "startSequence" )
//
//==================================================================================
int HARDWARE_sendAllGpibCommands (t_sequence *seq, int beginningOfSequence) 
{
	int i;
	t_gpibCommand *g;
	int error;
	clock_t startTime;
	int commandTransmitted;
	
	if (!config->enableGPIB) return 0;
	error = 0;
	
	startTime = clock ();
	commandTransmitted = 0;
	
//	SetWaitCursor (1);
	for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) 
		if (!error) {
			g = GPIBCOMMAND_ptr(seq, i);
			if (beginningOfSequence) {
				if (((seq->repetition == 0) && (g->transmitBeforeFirstRepetition))
				  || (g->transmitBeforeEveryRepetition)) {
					error = GPIB_transmitCommand (seq, g, 0, 1);
					HARDWARE_sendAllGpibCommands_setFlag (&commandTransmitted);
				}
			}
			else {
				if (((seq->repetition == seq->nRepetitions-1) && (g->transmitAfterLastRepetition))
				  || (g->transmitAfterEveryRepetition)) {
					error = GPIB_transmitCommand (seq, g, 0, 1);
					HARDWARE_sendAllGpibCommands_setFlag (&commandTransmitted);
				}
			}
		}
		
	// eventually check the devices if the values have been set

	for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) 
		if (!error) {
			g = GPIBCOMMAND_ptr(seq, i);
			if (beginningOfSequence) {
				if (((seq->repetition == 0) && (g->transmitBeforeFirstRepetition))
				  || (g->transmitBeforeEveryRepetition)) {
					error = GPIB_transmitCommand (seq, g, 1, 1);
					HARDWARE_sendAllGpibCommands_setFlag (&commandTransmitted);
				}
			}
			else {
				if (((seq->repetition == seq->nRepetitions-1) && (g->transmitAfterLastRepetition))
				  || (g->transmitAfterEveryRepetition) ) {
					error = GPIB_transmitCommand (seq, g, 1, 1);
					HARDWARE_sendAllGpibCommands_setFlag (&commandTransmitted);
				}
			}
		}
	
//	SetWaitCursor (0);
	if ((!config->suppressProtocol) && commandTransmitted) 
		tprintf ("[%1.3f s]\n", timeStop_s (startTime));
	
	return error;
}






unsigned long sub (unsigned long time1, unsigned long time2)
{
	unsigned long deltaT;
	
	if (time1 > time2) {
		DEBUGOUT_printf ("Warning: Buffer overrun at start trigger\n");
		deltaT = (ULONG_MAX - time1) + time2;
		Breakpoint();
	}   
	else deltaT = time2 - time1;
	return deltaT;
}
	




//int HARDWARE_generatePulse (int device, int line, double duration_ms)
//{
//	int iStatus;
//	
//	iStatus = DIG_Line_Config (device, 0, line, 1);
//    if (errorCheck (iStatus, device, "DIG_Line_Config")) return iStatus;
//	iStatus = DIG_Out_Line (device, 0, line, 1);
//    if (errorCheck (iStatus, device, "DIG_Out_Line")) return iStatus;
//	Sleep (duration_ms);
//	iStatus = DIG_Out_Line (device, 0, line, 0);
//    if (errorCheck (iStatus, device, "DIG_Out_Line")) return iStatus;
//    
//    return 0;
//}






	// if set == -1 --> function gets Value
	// if set
int HARDWARE_SetGetInterruptedFlag (int set)
{
	int flag = 0;
	int *flagPtr;

	static int tsvHandle = 0;  // handle to thread save Variable
	
	if (tsvHandle == 0) {
		CmtNewTSV (sizeof(int), &tsvHandle);
		// init flag to 0;
		CmtGetTSVPtr (tsvHandle, &flagPtr);
		*flagPtr = 0;
		CmtReleaseTSVPtr (tsvHandle);
	}
	
	CmtGetTSVPtr (tsvHandle, &flagPtr);
	if (set == -1) {
		// get function Value
		flag = *flagPtr;
	}
	else { 
		*flagPtr = set;	
	}
	CmtReleaseTSVPtr (tsvHandle);
	
	return flag;
}



int HARDWARE_stopDataOutput (void)
{
	switch (config->hardwareType) {
		case HARDWARE_TYPE_ADWIN:
			return HARDWARE_ADWIN_stopDataOutput ();
//		case HARDWARE_TYPE_NATINST:
//			HARDWARE_SetGetInterruptedFlag (1);     
//			break;
	}
	return 0;
}



void HARDWARE_checkForDoubleBuffering (t_outputData *o)
{

/*	
	if (DISABLE_DOUBLE_BUFFERING) {
		o->DIO_dblBuf = 0;
		o->AO_dblBuf = 0;
		DEBUGOUT_printf ("\nDouble buffering disabled!!\n");
		return;
	}
*/
	
/*	
	
	o->DIO_dblBuf = (o->DIO_totalSamples > DIO_BUFFER_SIZE_SAMPLES);
	o->AO_dblBuf = (o->AO_totalSamples > AO_BUFFER_SIZE_SAMPLES);
*/
	
	o->DIO_dblBuf = 1;
	o->AO_dblBuf = 1;
	
	
	DEBUGOUT_printf  ("\nCheck for double buffering: DIO:%I64d samples ==> DBLBUF %s", o->DIO_totalSamples, o->DIO_dblBuf ? "enabled" : "disabled");
	DEBUGOUT_printf  ("\nCheck for double buffering: AO:%I64d samples ==> DBLBUF %s", o->AO_totalSamples, o->AO_dblBuf ? "enabled" : "disabled");
}




//int HARDWARE_DBLBUF_waitUntilOutputFinished (t_outputData *o) 
//{
//    short wfmStopped = 0;
//	unsigned long wfmIterations = 0;
//	unsigned long wfmPointsGenerated= 0;
//	__int64 nGenerated;
//	int interrupted;
//	
//	int iStatus = 0;  
//	int t = 0;
//	
//	interrupted = 0;
//	
//	DEBUGOUT_printf ("\nWaiting until output finished...");
//	do {
//		// -----------------------------------------------------------
//	    // 	    get number of generated points
//		// -----------------------------------------------------------
//		iStatus = WFM_Check (deviceNrAO(0), 0, &wfmStopped, &wfmIterations,
//								 &wfmPointsGenerated);
//	// DEBUG        
//		t++;
//		if (t % 100000 == 0) { 
//			DEBUGOUT_printf ("WFM_Check: wfmStopped=%d, wfmIterations=%d, wfmPointsGenerated=%d\n",
//						 wfmStopped, wfmIterations, wfmPointsGenerated);
//		}
////		Sleep (500);
//        if (iStatus == -10803) iStatus = 0;
//        if (iStatus == -10608) {
//			iStatus = 0; // skip error "no transfer in progress"
//			wfmStopped = 1;
//		}
// 	    if (cmtErrorCheck (iStatus, deviceNrAO(0), "WFM_Check (B)")) goto STOP_OUTPUT;
//		interrupted = HARDWARE_SetGetInterruptedFlag (-1);
//		
//		nGenerated = AO_BUFFER_SIZE_SAMPLES * wfmIterations + wfmPointsGenerated;
//		OUTPUT_displayPointsGenerated (nGenerated);
//		Sleep (100);
////		SetCtrlVal (panelMain, MAIN_NUMERICSLIDE_progress, ui64ToDouble(nGenerated));
//    } while (!wfmStopped && !interrupted);
//
//	
//STOP_OUTPUT:
//	OUTPUT_displayPointsGenerated (o->AO_totalSamples);
//	DEBUGOUT_printf ("  ... FINISHED\n");
//
//	return iStatus;
//}




//int  CVICALLBACK HARDWARE_DBLBUF_outputThreadFunction (void *functionData)
//{
//	t_sequence *seq;
//	int iStatus = 0;
//	int i;
//	t_outputData *out;
//	int device;
//	
//	// -----------------------------------------------------------
//	//   DIGITAL: variables
//	// -----------------------------------------------------------
//	short *DIO_buffer[MAX_DIO_DEVICES];
//	short *DIO_halfBuffer[MAX_DIO_DEVICES]; 
//	short DIO_halfReady = 0;
//	unsigned __int64 DIO_samplesTransferred = 0;
//	unsigned DIO_samplesToTransfer;
//	unsigned __int64 DIO_samplesToTransfer64;
//	int DIO_createNewHalfBuf = 0;
//
//	short *AO_buffer[MAX_AO_DEVICES];
//	short *AO_halfBuffer[MAX_AO_DEVICES];
//	short AO_halfReady = 0;
//	unsigned __int64 AO_samplesTransferred = 0;
//	unsigned AO_samplesToTransfer;
//	unsigned __int64 AO_samplesToTransfer64;
//	int AO_createNewHalfBuf = 0;
////	int AO_nRepetitions;
//
//
//    short wfmStopped = 0;
//	unsigned long wfmIterations = 0;
//	unsigned long wfmPointsGenerated= 0;
//	__int64 nGenerated;
//	
//	int interrupted = 0;
//	int done = 0;
//	clock_t time;
//	unsigned long time2;
//	unsigned long blck = 0;
//
//	seq = (t_sequence *) functionData;
//	if (seq == NULL) return 0;
//	out = seq->outData;
//	if (out == NULL) return 0;
//	DisableBreakOnLibraryErrors ();
//	
//	// -----------------------------------------------------------
//	//   reset all buffers + counters
//	// -----------------------------------------------------------
//	for (i = 0; i < MAX_DIO_DEVICES; i++) {
//		DIO_buffer[i] = NULL;
//		DIO_halfBuffer[i] = NULL;
//	}
//	for (i = 0; i < MAX_AO_DEVICES; i++) {
//		AO_buffer[i] = NULL;
//		AO_halfBuffer[i] = NULL;
//	}				   
//	DIO_samplesTransferred	= 0;
//	DIO_samplesToTransfer64 = min (out->DIO_totalSamples, DIO_BUFFER_SIZE_SAMPLES);
//	DIO_samplesToTransfer = DIO_samplesToTransfer64;
//
//	AO_samplesTransferred	= 0;
//	AO_samplesToTransfer64 = min (out->AO_totalSamples, AO_BUFFER_SIZE_SAMPLES);
//	AO_samplesToTransfer = AO_samplesToTransfer64;
//
//	// -----------------------------------------------------------
//    // 	    allocate memory for output buffers
//	// -----------------------------------------------------------
//	DEBUGOUT_printf ("\n");
//	for (device = 0; device < nDIOBoards(); device++) {
//		DIO_buffer[device] = (short *) malloc (N_OUTPUT_PORTS * DIO_BUFFER_SIZE_SAMPLES);
//		DEBUGOUT_printf ("\nDIO: BufferSize: %u Samples", DIO_BUFFER_SIZE_SAMPLES);
//		DIO_halfBuffer[device] = (short *) malloc (N_OUTPUT_PORTS * DIO_BUFFER_SIZE_SAMPLES / 2);
//	}
//	for (device = 0; device < nAOBoards(); device++) {
//		AO_buffer[device] = (short *) malloc (2*8* AO_BUFFER_SIZE_SAMPLES);
//		DEBUGOUT_printf ("\nAO:  BufferSize: %u Samples", AO_BUFFER_SIZE_SAMPLES);
//		AO_halfBuffer[device] = (short *) malloc (8* AO_BUFFER_SIZE_SAMPLES);
//	}
//	
//
//	// -----------------------------------------------------------
//    // 	    configure all boards
//	// -----------------------------------------------------------
//	HARDWARE_checkForDoubleBuffering (seq->outData);
//	
//	
////	DEBUGOUT_printf ("\nHARDWARE_DBLBUF_outputThreadFunction: Samples to transfer: %d", DIO_samplesToTransfer);
//	
//	OUTPUTDATA_resetCounters (out);
//	
//
//	
//	time = clock ();
//	// -----------------------------------------------------------
//    // 	    DIGITAL OUT: write first buffer
//	// -----------------------------------------------------------
//	OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (out, (unsigned long **) DIO_buffer, DIO_samplesToTransfer);
//	DIO_samplesTransferred += DIO_samplesToTransfer64;
////	OUTPUTDATA_displayBuffer ((unsigned long *) DIO_buffer[0], DIO_samplesToTransfer);
//	// -----------------------------------------------------------
//    // 	    ANALOG OUT: write first buffer
//	// -----------------------------------------------------------
//	OUTPUTDATA_writeAOBytesToOutputBuffer_DBLBUF (out, AO_buffer, AO_samplesToTransfer);
//	AO_samplesTransferred += AO_samplesToTransfer64;
//	
//	time2 = timeStop (time);
//
//	// -----------------------------------------------------------
//    // 	    DIGITAL OUT: output first data
//	// -----------------------------------------------------------
//	for (device = 0; device < nDIOBoards(); device++) {
//		// -----------------------------------------------------------
//	    // 		configure internally timed pattern generation with timebase
//	    //      and no external gating
//		// -----------------------------------------------------------
//		iStatus = DIG_Block_PG_Config (deviceNrDIO(device), 1, 
//					 				   out->DIO_dblBuf ? 1 : 2,
//									   0,
//									   -3,
//									   seq->DIO_timebase_50ns, 0);
//	    if (cmtErrorCheck (iStatus, deviceNrDIO(device), "DIG_Block_PG_Config")) goto STOP_OUTPUT;
//		// -----------------------------------------------------------
//	    // 		configure double buffered output
//		// -----------------------------------------------------------
//		if (out->DIO_dblBuf) {
//			iStatus = DIG_DB_Config (deviceNrDIO(device), 1, 1, 1, 1);
//			if (cmtErrorCheck  (iStatus, deviceNrDIO(device), "DIG_DB_Config")) goto STOP_OUTPUT;
//		}
//
//    	iStatus = Timeout_Config(deviceNrDIO(device), -1);
//	    if (cmtErrorCheck (iStatus, deviceNrDIO(device), "Timeout_Config")) goto STOP_OUTPUT;
//		
//		iStatus = DIG_Block_Out (deviceNrDIO(device), 1, DIO_buffer[device], DIO_samplesToTransfer );
//	    if (cmtErrorCheck (iStatus, deviceNrDIO(device), "DIG_Block_Out")) goto STOP_OUTPUT;
//	}
//	DIO_createNewHalfBuf = 1;
//	time = clock ();
//	// -----------------------------------------------------------
//    // 	    ANANLOG OUT: output first data
//	// -----------------------------------------------------------
//
//	for (device = 0; device < nAOBoards(); device++) {
//		// -----------------------------------------------------------
//	    // 		enable double buffered output
//		// -----------------------------------------------------------
//		iStatus = WFM_DB_Config (deviceNrAO(device), out->AOnChannels[device], out->AOchannelVect[device], out->AO_dblBuf, 1, 1);
//	    if (cmtErrorCheck (iStatus, deviceNrAO(device), "WFM_DB_Config")) goto STOP_OUTPUT;
//
//    	iStatus = Timeout_Config(deviceNrAO(device), -1);
//	    if (cmtErrorCheck (iStatus, deviceNrAO(device), "Timeout_Config")) goto STOP_OUTPUT;
//		
////		if (out->AO_dblBuf) AO_nRepetitions = 0; 
////		else AO_nRepetitions = out->nCopies;
//		iStatus = WFM_Load (deviceNrAO(device), out->AOnChannels[device],
//							out->AOchannelVect[device], AO_buffer[device],
//							AO_samplesToTransfer * out->AOnChannels[device],
//							0, 0);
//	    if (cmtErrorCheck (iStatus, deviceNrAO(device), "WFM_Load")) goto STOP_OUTPUT;
//		// -----------------------------------------------------------
//	    // 	    set clock rate 
//		// -----------------------------------------------------------
//		iStatus = WFM_ClockRate (deviceNrAO(device), 1, 0, 1,
//								 seq->AO_timebase_50ns / VAL_us, 0);
//	    if (cmtErrorCheck  (iStatus, deviceNrAO(device), "WFM_ClockRate (1)")) return -1;
//	}
//	// -----------------------------------------------------------
//	// 		start AO boards 
//	//      -- in REVERSE order, since first AO device triggers all others
//    // -----------------------------------------------------------
//	for (device = nAOBoards()-1; device >= 0; device--) {
//		iStatus = WFM_Group_Control (deviceNrAO(device), 1, 1);
//	    if (cmtErrorCheck  (iStatus, deviceNrAO(device), "WFM_Group_Control")) return -1;
//	}
//	AO_createNewHalfBuf = out->AO_dblBuf;
//	
//
////	HARDWARE_DBLBUF_TEMP_startAnalogOut (seq);
//
//	
//	while (!interrupted && !done) {
//		interrupted = HARDWARE_SetGetInterruptedFlag (-1);
//		
//		// -----------------------------------------------------------
//		// 	    DIGITAL OUT: write next half buffer
//		// -----------------------------------------------------------
//		if (DIO_createNewHalfBuf) {
//			DIO_samplesToTransfer64 = min (DIO_BUFFER_SIZE_SAMPLES / 2, out->DIO_totalSamples - DIO_samplesTransferred);
//			DIO_samplesToTransfer = DIO_samplesToTransfer64;
//			OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (out, (unsigned long **) DIO_halfBuffer, DIO_samplesToTransfer);
//			DIO_createNewHalfBuf = 0;
//		}
//		
//
//		// -----------------------------------------------------------
//		// 	    Analog OUT: write next half buffer
//		// -----------------------------------------------------------
//		if (AO_createNewHalfBuf) {
//			AO_samplesToTransfer64 = min (AO_BUFFER_SIZE_SAMPLES / 2, out->AO_totalSamples - AO_samplesTransferred);
//			AO_samplesToTransfer = AO_samplesToTransfer64;
//			OUTPUTDATA_writeAOBytesToOutputBuffer_DBLBUF (out, AO_halfBuffer, AO_samplesToTransfer);
//			AO_createNewHalfBuf = 0;
//		}
//
//		timeStop (time);
//		
//		// -----------------------------------------------------------
//		// 	    DIGITAL OUT: check if half ready
//		// -----------------------------------------------------------
//		if (out->DIO_dblBuf) {
//			iStatus = DIG_DB_HalfReady(deviceNrDIO(0), 1, &DIO_halfReady);
//			if (iStatus == -10881) iStatus = 0;
//	    	if (cmtErrorCheck (iStatus, deviceNrDIO(0), "DIG_DB_HalfReady")) goto STOP_OUTPUT;
//		}
//		else 
//			DIO_halfReady = 0;
////		iStatus = DIG_Block_Check (deviceNrDIO(0), 1, &blck);
////	    if (cmtErrorCheck (iStatus, deviceNrDIO(0), "DIG_Block_Check")) goto STOP_OUTPUT;
//
//		
//		// -----------------------------------------------------------
//		// 	    ANALOG OUT: check if half ready
//		// -----------------------------------------------------------
//		if (out->AO_dblBuf) {
//			iStatus = WFM_DB_HalfReady(deviceNrAO(0), out->AOnChannels[0], out->AOchannelVect[0], &AO_halfReady);
//			if (iStatus == -10881) iStatus = 0;
//	    	if (cmtErrorCheck (iStatus, deviceNrAO(0), "WFM_DB_HalfReady")) goto STOP_OUTPUT;
////			DEBUGOUT_printf ("\nWFM_DB_halfReady: %d", AO_halfReady);
//
//		}
//		else 
//			AO_halfReady = 0;
////		iStatus = DIG_Block_Check (deviceNrDIO(0), 1, &blck);
////	    if (cmtErrorCheck (iStatus, deviceNrDIO(0), "DIG_Block_Check")) goto STOP_OUTPUT;
//
//		
//
///*		
//		if (DIO_halfReady || (print > 0)) {
//			DEBUGOUT_printf ("\nHalfready=%d(%d) SamplesLeft: %9I64d, samplesToTransfer=%d, done=%d (totalWritten=%I64u)", 
//						 DIO_halfReady, blck, samplesLeft, DIO_samplesToTransfer, done, out->DIO_totalSamplesWritten);
//			if (print > 0) print --;
//		}
//*/
//		if (interrupted) DEBUGOUT_printf ("\nTRANSFER INTERRUPTED!\n");
//
//		done =  (DIO_samplesToTransfer == 0) && (AO_samplesToTransfer == 0);
//
//		
//		// -----------------------------------------------------------
//		// 	    DIGITAL OUT: transfer next half buffer
//		// -----------------------------------------------------------
//		if ((DIO_samplesToTransfer != 0) && DIO_halfReady) {
//		  	for (device = 0; device < nDIOBoards(); device++) {
//				DEBUGOUT_printf ("\nDIG_DB_Transfer dev%d: %d Samples", device, DIO_samplesToTransfer);
//				iStatus = DIG_DB_Transfer(deviceNrDIO(device), 1, DIO_halfBuffer[device], DIO_samplesToTransfer );
//		    	if (cmtErrorCheck (iStatus, deviceNrDIO(device), "DIG_DB_Transfer")) goto STOP_OUTPUT;
//				
//			}
//			DIO_samplesTransferred  += DIO_samplesToTransfer64;  
//			DIO_createNewHalfBuf = 1;
//		}
//
//
//		if ((AO_samplesToTransfer != 0) && AO_halfReady) {
//		  	for (device = 0; device < nAOBoards(); device++) {
//				DEBUGOUT_printf ("\nWFM_DB_Transfer dev%d: %d Samples", device, AO_samplesToTransfer);
//				iStatus = WFM_DB_Transfer(deviceNrAO(device),out->AOnChannels[device], out->AOchannelVect[device], AO_halfBuffer[device], 
//									      AO_samplesToTransfer * out->AOnChannels[device]);
//		    	if (cmtErrorCheck (iStatus, deviceNrAO(device), "WFM_DB_Transfer")) goto STOP_OUTPUT;
//				
//			}
//			AO_samplesTransferred  += AO_samplesToTransfer64;  
//			AO_createNewHalfBuf = 1;
//		}
//
//		iStatus = WFM_Check (deviceNrAO(0), 0, &wfmStopped, &wfmIterations,
//								 &wfmPointsGenerated);
//        if (iStatus == -10803) {
//			done = 1;
//			iStatus = 0;
//		}
//        if (iStatus == -10608) {
//			iStatus = 0; // skip error "no transfer in progress"
//		}
// 	    if (cmtErrorCheck (iStatus, deviceNrAO(0), "WFM_Check (A)")) goto STOP_OUTPUT;
//
//		nGenerated = AO_BUFFER_SIZE_SAMPLES * wfmIterations + wfmPointsGenerated;
//		OUTPUT_displayPointsGenerated (nGenerated);
//		
//		
//		
//	}
//	
//	if (!interrupted) HARDWARE_DBLBUF_waitUntilOutputFinished (out);	
//
//STOP_OUTPUT:	
//	
//	for (device = 0; device < nDIOBoards(); device ++) {
//		// -----------------------------------------------
//	    // 	    DIO: halt ongoing data transfer
//		// -----------------------------------------------
//		iStatus = DIG_Block_Clear (deviceNrDIO(device), 1);
//	}
//	
//	for (device = 0; device < nAOBoards(); device++) {
//		// -----------------------------------------------
//	    // 	    AO: halt ongoing data transfer
//		// -----------------------------------------------
//		iStatus = WFM_Group_Control (deviceNrAO(device), 1, 0);
//	}
//
//
//	for (i = 0; i < MAX_DIO_DEVICES; i++) {
//		free (DIO_buffer[i]);
//		free (DIO_halfBuffer[i]);
//	}
//		
//	for (i = 0; i < MAX_AO_DEVICES; i++) {
//		free(AO_buffer[i]);
//		free(AO_halfBuffer[i]);
//	}
//	
//
//	DEBUGOUT_printf ("\nOutput Stopped!");
//	
//	return iStatus;
//
//	
//}
	


//int HARDWARE_DBLBUF_dataOutput  (t_sequence *seq)
//{
//	int cmtOutputFunctionID;
//	static int isRunning = 0;
//	int status;
//	int daqThreadFunctionStatus;
//	int interrupted;
//	
//
//	if (deviceNrDIO(0) < 0) {
//		PostMessagePopup ("ERROR!", "No digital IO board found!");		
//		return 1;
//	}
//	if (isRunning) return 1;
//
//// ===========================================================
//// 	   init thread for data output
//// ===========================================================
//	isRunning = 1;
//	
//	status = CmtScheduleThreadPoolFunctionAdv (DEFAULT_THREAD_POOL_HANDLE,
//											   HARDWARE_DBLBUF_outputThreadFunction,
//											   seq,
//											   THREAD_PRIORITY_TIME_CRITICAL,
//											   NULL, 0, NULL, 0,
//											   &cmtOutputFunctionID);
//	if (status != 0) displayCmtErrorMSG (status, "CmtScheduleThreadPoolFunctionAdv");
//	
//	
//    status = CmtGetThreadPoolFunctionAttribute (DEFAULT_THREAD_POOL_HANDLE,
//                                   cmtOutputFunctionID,
//                                   ATTR_TP_FUNCTION_EXECUTION_STATUS,
//                                   &daqThreadFunctionStatus);
//	if (status != 0) displayCmtErrorMSG (status, "CmtGetThreadPoolFunctionAttribute");
//	
//	if (daqThreadFunctionStatus != kCmtThreadFunctionComplete) {
//		status = CmtWaitForThreadPoolFunctionCompletion
//						  (DEFAULT_THREAD_POOL_HANDLE, cmtOutputFunctionID,
//						   OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
//		if (status != 0) displayCmtErrorMSG (status, "CmtWaitForThreadPoolFunctionCompletion");
//	}
//	status = CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE,
//                                    		 cmtOutputFunctionID);
//	if (status != 0) displayCmtErrorMSG (status, "CmtReleaseThreadPoolFunctionID");
//	
//STOP_DATAOUTPUT:
//	
//	isRunning = 0;
//	
//	// check if sequence was interrupted
//	interrupted = HARDWARE_SetGetInterruptedFlag (-1);
//	// set flsag to 0
//	HARDWARE_SetGetInterruptedFlag (0);
//
//	return interrupted;
//
//}





int OUTPUT_executeRepetition (t_sequence *seq, int firstRepetition)
{
	//int iStatus;
    int userInterrupted = 0;
	t_outputData *out;
	//int err;
	//int hasTransmitted = 0;
	//clock_t startTime;
	//t_waveform *waveformDMD;

	if (SHOWTIMES) tprintf ("\nTIME TEST: 'execute Repetition': %d ms\n", timeStop (mytime));

	if (SEQUENCE_getStatus() != SEQ_STATUS_STARTED) return 0;


	out = seq->outData;
	// -----------------------------------------------------------
    // 	    increase number of repetitions
	// -----------------------------------------------------------
	OUTPUT_setRepetitions (seq);        
	

// ===========================================================
// 	    VARIABLE BLOCK DURATIONS (from VERSION 1.3 on)
// ===========================================================
	
	if (out->variableOutput && !firstRepetition) {
		// -----------------------------------------------------------
	    // 	    recalculate all data    
		// -----------------------------------------------------------
		if (OUTPUTDATA_calculate (seq, firstRepetition, 0) < 0) goto ERROR_2;  
		out = seq->outData;
    }

// ===========================================================
// 	   Phase feedback
// ===========================================================
	OUTPUTDATA_ADWIN_initPhaseFeedback (seq);

// ===========================================================
// 	   Send configuration files
// ===========================================================
	// see what checkboxes are checked
	int ccdSendOnFirst;
	int ccdSendOnEvery;
	int confSendOnFirst;
	int confSendOnEvery;
	GetCtrlVal(CCD, CCD_CHECKBOX_FIRST, &ccdSendOnFirst);
	//GetCtrlVal(CCD, CCD_CHECKBOX_EVERY, &ccdSendOnEvery);
	GetCtrlVal(CCD, LCONFTAB_CHECKBOX_FIRST, &confSendOnFirst);
	GetCtrlVal(CCD, LCONFTAB_CHECKBOX_EVERY, &confSendOnEvery);
	
	// send our configuration files depending on what checkboxes are checked
	if (firstRepetition && ccdSendOnFirst || ccdSendOnEvery) {
		CCD_sendCommandFile();	
	}
	if (firstRepetition && confSendOnFirst || confSendOnEvery) {
		sendConfFile();
	}

// ===========================================================
// 	   DMD 
// ===========================================================
//	waveformDMD = WFM_ADDR_findDMDwaveform (seq);
//	if (waveformDMD != NULL) {
//		if (DMDIMAGE_writeCommandFile (seq, waveformDMD->addrDMDimage, seq->repetition) != 0) goto ERROR_2;
//	}

// ===========================================================
// 	   DATA OUTPUT #0 (send GpibCommands)
// ===========================================================
	GPIB_setShowDataFlag (!config->suppressProtocol);
	if (HARDWARE_sendAllGpibCommands (seq, 1) != 0) goto ERROR_3;

	if (SHOWTIMES) tprintf ("\nTIME TEST: GPIB processed: %d ms\n ", timeStop (mytime));
	GPIB_setShowDataFlag (1);
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: GPIB");

	// -----------------------------------------------
    // 	    init progress bar
	// -----------------------------------------------
	OUTPUT_setMaxPointsGenerated (out->AO_totalSamples);

// ===========================================================
// 	   DATA OUTPUT #1 (transmit data to devices)
// ===========================================================

	if (SHOWTIMES) tprintf ("\nTIME TEST: starting data output: %d ms\n", timeStop (mytime));

	
	
	switch (config->hardwareType) {
//		case HARDWARE_TYPE_NATINST:
//			if (HARDWARE_DBLBUF_dataOutput (seq) != 0) goto ERROR;
//			break;
		case HARDWARE_TYPE_ADWIN:
			if (HARDWARE_ADWIN_dataOutput (seq->outData->adwinData, -1, 1, 1) != 0) goto ERROR;
			if ((seq->repetition == (seq->nRepetitions-1)) && (seq->nRepetitions > 1)) {
				seq->repetition ++; // TODO why increment repetition and decrement again???
				OUTPUTDATA_ADWIN_initPhaseFeedback (seq);
				seq->repetition --;
				Sleep (5000);
				HARDWARE_ADWIN_feedback (seq->outData->adwinData, 1);
			}
			break;
	}
	
	if (transmit_CMD_READY) TCP_serverSendStrf (TCP_CMD_READY);
	
	DATASOCKET_setWaitingForTrigger (seq->trigger != VAL_NO_TRIGGER);


	// exit funcitonif no hardware present
	if (!hardwareDetected ()) {
		Sleep (500);
		//ProcessSystemEvents ();
		ProcessDrawEvents ();  
		return 0;
	}
	
	
// ===========================================================
// 	   WAIT UNTIL EVERYTHING IS DONE
// ===========================================================
	// -----------------------------------------------
    // 	    wait for analog board to finish
	// -----------------------------------------------

//	iStatus = HARDWARE_waitUntilOutputFinished (seq, &userInterrupted);
	DATASOCKET_setWaitingForTrigger (0);
    
//    if (iStatus == WATCHDOG_ERR) goto ERROR_WATCHDOG;

// ===========================================================
// 	   HALT data output
// ===========================================================
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: HARDWARE_haltDataOutput");
	
// ===========================================================
// 	   DATA OUTPUT #0 (send GpibCommands)
// ===========================================================
//	Sleep (100);
//	DEBUGOUT_printf ("t!");

#ifdef _CVI_DEBUG_   
	if (seq->nRepetitions == 1) {
		HARDWARE_resetOutputs (seq);
	}
#endif 



	if (!userInterrupted) {
		if (HARDWARE_sendAllGpibCommands (seq, 0) != 0) return -1;
	    // 	process system events (to enable STOP)
//	    ProcessSystemEvents ();
	}
	return 0;

ERROR:    
//	HARDWARE_haltDataOutput (out);
		
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: HARDWARE_stopCounters(ERROR)");
    
	return -1;
    
ERROR_WATCHDOG:
//	HARDWARE_haltDataOutput (out);
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: HARDWARE_stopCounters(WATCHDOG_ERR)");
    return WATCHDOG_ERR;
    
ERROR_2:
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: HARDWARE_stopCounters(ERROR 2)");
	return -1;	
	
ERROR_3:
	HARDWARE_resetOutputs (seq);
	PROTOCOLFILE_printf ("FUNC: startSequence_executeRepetition DONE: HARDWARE_stopCounters(GPIB_ERR)");
	return -1;
}



int sequenceStarted (void)
{
	return SEQUENCE_getStatus() == SEQ_STATUS_STARTED;
}



void HARDWARE_setStartAOVoltages (t_sequence *seq)
{
	t_digitalBlock *firstBlock;
	int i;
	t_waveform *wfm;
	double voltage;
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN) return;
	
	firstBlock = DIGITALBLOCK_ptr (seq, 1);
   	PROTOCOLFILE_printf ("FUNC:    setStartAOVoltages: first Block = %s", firstBlock->blockName);

	DIGITALBLOCK_getWaveformsAndDuration (seq, firstBlock );
	if (firstBlock == NULL) return;

	for (i = 0; i < N_DAC_CHANNELS; i++) {
		voltage = 0;
    	wfm = firstBlock->wfmP[i];
    	if (wfm != NULL) {
			if (HARDWARE_isAnalogChannelActive(seq, i) && (wfm != NULL)) {
	   			PROTOCOLFILE_printf ("FUNC:    setStartAOVoltages: wfm = %s", wfm->name); 
		    	switch (wfm->type) {
		    		case WFM_TYPE_CONSTVOLTAGE:
				        voltage = firstBlock->constVoltages[i];
				       	break;
				    case WFM_TYPE_STEP:
				        voltage = wfm->stepFrom;
				    	break;
				}
	       	}
		}
        HARDWARE_sendDirectAnalogValue (i, voltage);
	}
}



void setDigitalChannels_alwaysOnOff (t_sequence *seq)
{
	int i;
	
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		if (seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSON) 
			HARDWARE_sendDirectDigitalValue (i, 1);
		else if (seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSOFF) 
			HARDWARE_sendDirectDigitalValue (i, 0);
	}
}


int nPulses (t_sequence *seq, int ch)
{
	int multiply=1;
	int i, n;
	int oldState = 0;
	t_digitalBlock *d;
	int NPulses = 0;
	
    n = ListNumItems (seq->lDigitalBlocks);
    for (i = 1; i <= n; i++) {
    	d = DIGITALBLOCK_ptr (seq, i);
    	if (d->blockMode >= 1) 
    				multiply = d->blockMode;
    	if (d->channels[ch] != oldState) {
    		if (oldState == 0) NPulses+= multiply;
    		oldState = d->channels[ch];
    	}
    	if(d->blockMode==BMODE_LoopEnd)  multiply = 1;  
    }
    return NPulses;
}




/************************************************************************
 ************************************************************************
 *
 * 		startSequence()
 *
 * 		main function for data output
 *
 ************************************************************************
	 ************************************************************************/

int startSequence (t_sequence *seq)
{ 
    int iStatus = 0;
    int firstRepetition = 1;
    int i;
    int error;
	int loopStop;
	t_sequence *idleSeq;

	int successful;    
	clock_t startTime;
	clock_t timeFirstRepetition = 0;
//	if (seq->remote) SetSleepPolicy (VAL_SLEEP_MORE);
	
	
	if (!seq->isOfTypeIdleSeq) {
		idleSeq = SEQUENCE_ptr (config->idleSequenceNr);
	
		if (idleSeq != NULL) {
			seq->autoFocusNumImages = idleSeq->autoFocusNumImages;
			seq->autoFocusFirstBlock = idleSeq->autoFocusFirstBlock; // not needed
			seq->autoFocusCenterVoltage = idleSeq->autoFocusCenterVoltage;
			seq->autoFocusStepVoltage = idleSeq->autoFocusStepVoltage;
		}	
		
	}
	
	seq->considerWatchdog = config->watchdogActivate; // take over global watchdog setting          


	if (SHOW_HARDWARE_PROTOCOL) DEBUGOUT_initPanel ();
	
   	PROTOCOLFILE_printf ("\n ++++FUNC: startSequence START seq=%08x", seq);

	if (seq->panelSimulate) {
		DiscardPanel (seq->panelSimulate);
	}
	seq->panelSimulate = 0;
	
	SEQUENCE_setStatus(SEQ_STATUS_STARTED);
	
	stdCls();
//	GPIB_clearOutputCtrl ();
   	seq->nExecutionsDone = 0;
	
	if (SHOWTIMES) tprintf ("TIME TEST: 'Start sequence called': %d ms)\n", timeStop (mytime));

	// -----------------------------------------------------------
    // 	    EXIT, if no hardware present
	// -----------------------------------------------------------
/*	if (!hardwareDetected()) {
		tprintf ("No hardware detected!!");
		for (i = 0; i < 100; i++) {
			Sleep(500);
			if (SEQUENCE_getStatus() != SEQ_STATUS_STARTED) i=100;
			ProcessSystemEvents();
		}
		//MessagePopup ("Error!", "No hardware detected for signal output!");
		return 0;
	}	
*/
	DisableBreakOnLibraryErrors ();


	// -----------------------------------------------------------
    // 	    check if redo repetitions are correct
	// -----------------------------------------------------------
   	for (i = 0; i < seq->nRedo; i++) 
   		if (seq->redoRepetitions[i] > seq->nRepetitions) 
   			SEQUENCE_freeRedo (seq);   

	
	// -----------------------------------------------------------
    // 	    get trigger source
	// -----------------------------------------------------------
//    seq->trigger = MAIN_getTriggerSource();
	// -----------------------------------------------------------
    // 	    set filename to popup window
	// -----------------------------------------------------------
//	OUTPUT_setSequenceInfo (extractFilename (seq->filename));
	tprintf ("Starting %d repetitions of sequence '%s'\n",
			 seq->nRedo ? seq->nRedo : seq->nRepetitions, 
			 extractFilename (seq->filename));
	// -----------------------------------------------------------
    // 	    set digital channels which are "always on/ off"
	// -----------------------------------------------------------
	setDigitalChannels_alwaysOnOff (seq);
   	PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'setDigitalChannels'");
 	HARDWARE_setStartAOVoltages (seq);
   	PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'setAOVoltages'");

	// -----------------------------------------------------------
    // 	    calculate output data
	// -----------------------------------------------------------
	if (seq->nRedo > 0) seq->repetition = seq->redoRepetitions[0];
	else seq->repetition = seq->repetitionStart;
    
    if (OUTPUTDATA_calculate (seq, 1, 0) < 0) return -1;
   	PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'calculateOutputData '");
	
	if (SHOWTIMES) tprintf ("\nTIME TEST: 'calculateOutputData': %d ms) ", timeStop (mytime));
	// -----------------------------------------------------------
    // 	    init output buffers
	// -----------------------------------------------------------
//    if (OUTPUTDATA_initOutputBuffers (seq->outData, 0) != 0) return -1;
	// -----------------------------------------------------------
    // 	    EXIT, if output was not correctly generated
	// -----------------------------------------------------------
    if (seq->outData->duration == 0) { 
	    PostMessagePopup ("Error", "Total time = 0. No output could be generated.");
    	goto ERROR;
    }
//!!!!!!!!!!!!!   
//   DEBUGOUT_printf ("%d: ", seq->nRepetitions);

//	configureGPCTR_FREQ (seq);
	
	// -----------------------------------------------------------
    // 	    configure watchdog
	// -----------------------------------------------------------
	if (watchdogConfigure () != 0) goto ERROR;
	

	// -----------------------------------------------------------
    // 	    configure all boards
	// -----------------------------------------------------------
//	HARDWARE_checkForDoubleBuffering (seq->outData);
	if (HARDWARE_configureOutputs (seq) != 0) goto ERROR;


	// -----------------------------------------------------------
    // 	    reset all last values from the frequency generators
	// -----------------------------------------------------------
	GPIB_resetAllLastValues (seq, 0, 0);
   	PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'GPIB_resetAllLastValues  '");
	

	// -----------------------------------------------------------
    // 	    output the sequence "seq->nRepetitions" times
	// -----------------------------------------------------------
	
//	DEBUGOUT_printf ("%d Repetitions\n", seq->nRepetitions);
//	for (i = 0; i < n

	OUTPUT_setElapsedTime (1);   
	OUTPUT_setTimes (seq, 1, 0);
	TCP_setDisplayProtocol (!config->suppressProtocol);
	
	
	seq->firstRepetition  = 1;
	if (seq->nRedo != 0) {
		// --------------------------------------------
		//      redo specific repetitions
		// --------------------------------------------	
    	for (i = 0; i < seq->nRedo; i++) {
    		seq->repetition = seq->redoRepetitions[i];
    		seq->run        = seq->redoRuns[i];
		   	successful = 0;
		   	while (!successful && sequenceStarted()) {
	 			// wait until watchdog is switched off
	 			//while (watchdogOn(seq) && sequenceStarted()) ProcessSystemEvents();
				// start sequence
		 		switch (OUTPUT_executeRepetition (seq, firstRepetition)) {
		 			case -1: 
		 				goto ERROR;  	
		 			case -2: 
		 				goto THE_END;
		 			case WATCHDOG_ERR: 
				 		successful = !config->watchdogAutoRepeat;
				 		if (!successful) GPIB_resetAllLastValues (seq, 1, 0);
						break;			 				
					default:
						successful = 1;
		 		}
		 		// set flag
		 		firstRepetition = 0;
		   	}
			seq->firstRepetition = 0;
   			seq->nExecutionsDone ++;
			OUTPUT_setTimes (seq, 1, 0);
			
			//ProcessSystemEvents();
    	}
    	SEQUENCE_freeRedo (seq);
	}
	else {
		// --------------------------------------------
		//      standard execution mode
		// --------------------------------------------	
 		startTime = clock();

		loopStop = seq->repetitionStop == 0 ? seq->nRepetitions : seq->repetitionStop;
		for (seq->run = seq->runStart; seq->run < seq->nRuns; seq->run ++) {
	    	for (seq->repetition = seq->repetitionStart; seq->repetition < loopStop; ) {
			   	successful = 0;
			   	if (!sequenceStarted()) goto THE_END;
			   	while (!successful && sequenceStarted()) {
		 			// wait until watchdog is switched off
		 			while (watchdogOn(seq) && sequenceStarted()) {
		 				if (config->suppressProtocol) ProcessSystemEvents();
			 			WATCHDOG_handshake ();
			 		}
			 		error = OUTPUT_executeRepetition (seq, firstRepetition);
			 		if (firstRepetition) timeFirstRepetition = timeStop (startTime);
			 		switch (error) {
			 			case -1: 
							// Aborted:
							if (!config->suppressProtocol) tprintf ("ABORTED.\n");
			 				goto ERROR;  	
			 			case -2: 
			 				goto THE_END;
			 			// watchdog intterupted
			 			case WATCHDOG_ERR: 
					 		successful = !config->watchdogAutoRepeat;
					 		if (!successful) GPIB_resetAllLastValues (seq, 1,0);
					 		WATCHDOG_handshake ();
					 		tprintf ("Watchdog alarm detected. %s\n", config->watchdogAutoRepeat ? "Redo repetition" : "");
							break;			 				
						default:
							successful = 1;
							if (!config->suppressProtocol){ 
								tprintf ("DONE.\n");
							}
						
			 		}
			 		firstRepetition = 0;
			   	}
	//		   	TEST_displayPoint (seq, globalSum);
				seq->firstRepetition = 0;
				
			   	seq->repetitionStart = 0;
				seq->nExecutionsDone ++;
				if ((!config->suppressProtocol) || (seq->repetition % REFRESH_RATE == 0)) {
					OUTPUT_setTimes (seq, 1, timeFirstRepetition);
				}
   				PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'repetition %d', seq=%08x", seq->repetition, seq);
				
//				ProcessSystemEvents();
   				PROTOCOLFILE_printf ("FUNC: startSequence DONE: 'ProcessSystemEvents()'");
				
				if (successful) seq->repetition ++;
			 }
		}
	}
	if (config->suppressProtocol) ProcessSystemEvents();
	
THE_END:
	// -----------------------------------------------
    // 	   reset boards
	// -----------------------------------------------
#ifdef _CVI_DEBUG_   
	
	if (seq->nRepetitions != 1) {
		PROTOCOLFILE_printf ("      seq=%08x", seq);
		HARDWARE_resetOutputs (seq);
	}
	
#else 
	HARDWARE_resetOutputs (seq);
#endif
//DEBUG
//    HARDWARE_stopCounters (seq, 1);
	// -----------------------------------------------
    // 	   remember last analog voltages
	// -----------------------------------------------
	HARDWARE_rememberLastValues (seq);
	if (!hardwareDetected()) return 0;

	// -----------------------------------------------
    // 	    disable timeouts
	// -----------------------------------------------
//	if (config->hardwareType == HARDWARE_TYPE_NATINST) {
//	    iStatus = Timeout_Config(deviceNrDIO(0), -1);
//	    if (errorCheck (iStatus, deviceNrDIO(0), "Timeout Config")) goto ERROR;
//	}
	
	// -----------------------------------------------
    // 	    free buffers
	// -----------------------------------------------
ERROR:
	HARDWARE_resetOutputs (seq);
	HARDWARE_rememberLastValues (seq);
	
   	PROTOCOLFILE_printf (" ++++FUNC: startSequence END, errorcode = %d\n", iStatus);

	
    return (iStatus == 0);

//	if (seq->remote) SetSleepPolicy (VAL_SLEEP_MORE);
}




char *timeStrSec64 (__int64 sec64)
{
	__int64 hour64, min64;
	int hour, min, sec;
	char *help;
	
	help = getTmpString();
	
	hour64 = sec64 / 3600;
	min64 = (sec64 / 60) % 60;
	sec64 = sec64 % 60;
	hour = (int)hour64;
	min  = (int)min64;
	sec  = (int)sec64;
	sprintf (help, "%02d:%02d:%02d", hour, min, sec);
	return help;
	 
					 
}




//=======================================================================
//
//    get names of signals
//
//=======================================================================
char *HARDWARE_signalName (int i) 
{
    switch (i) {
        case -1: return "None";
//        case ND_PFI_0: return "PFI0 (NI)";
//        case ND_PFI_1: return "PFI1 (NI)";
//        case ND_PFI_2: return "PFI2 (NI)";
//        case ND_PFI_3: return "PFI3 (NI)";
//        case ND_PFI_4: return "PFI4 (NI)";
//        case ND_PFI_5: return "PFI5 (NI)";
//        case ND_PFI_6: return "PFI6 (NI)";
//        case ND_PFI_7: return "PFI7 (NI)";
//        case ND_PFI_8: return "PFI8 (NI)";
//        case ND_PFI_9: return "PFI9 (NI)";
//        case ND_RTSI_0: return "RTSI0 (NI)";
//        case ND_RTSI_1: return "RTSI1 (NI)";
//        case ND_RTSI_2: return "RTSI2 (NI)";
//        case ND_RTSI_3: return "RTSI3 (NI)";
//        case ND_RTSI_4: return "RTSI4 (NI)";
//        case ND_RTSI_5: return "RTSI5 (NI)";
//        case ND_RTSI_6: return "RTSI6 (NI)";
		case ADWIN_TRIGGER_DIGIO0: return "Dig I/O 0 (ADWIN)";
		case ADWIN_TRIGGER_DIGIO1: return "Dig I/O 1 (ADWIN)";
//        case VAL_ACK1_TRIGGER: return "ACK1";
	    default: return " ";
    }
//    return "";
}



// ---------------------------------------------
//   reset and find all boards
// ---------------------------------------------

//int HARDWARE_NI_detectBoards  (int outPanel, int outCtrl)
//{
//    int deviceNo = 1;
//    unsigned long typeCode = 0;
//    short board;
//    int status;
//    
//	pprintf (outPanel, outCtrl, "Detecting NI DAQ-Boards...\n");
//
//	CONFIG_initBoards (config);
//    do {
//  		status = Init_DA_Brds (deviceNo, &board);
//        if (status == 0)
//           status = Get_DAQ_Device_Info (deviceNo, ND_DEVICE_TYPE_CODE, &typeCode);
//        if (status == 0) {
//           pprintf (outPanel, outCtrl, "found %s\n", get_NIDeviceName(typeCode));
//           addBoard (typeCode, deviceNo);
//  		}
////  		else if (outPanel > 0) appendText (outPanel, outCtrl, ": not found");
//		deviceNo++;
//    } while (status == 0);
//	
//	return status;
//}	



int HARDWARE_boot (int panel, int ctrl) 
{
	
	pprintf (panel, ctrl, "====================================================================\n");
	
	switch (config->hardwareType) {
		case HARDWARE_TYPE_ADWIN: 
			HARDWARE_ADWIN_addConfig ();
			if (HARDWARE_ADWIN_boot (panel, ctrl) != 0) return -1;
			break;
//		case HARDWARE_TYPE_NATINST:
//		    if (!config->CHECK_DISABLED) {
//				// ------------------------------------------
//				//    detect and initialize all boards
//				// ------------------------------------------
//				HARDWARE_NI_detectBoards (panel, ctrl);
//			}
//		    else {
//				// ------------------------------------------
//				//    DEBUG_MODE: manually insert boards
//				// ------------------------------------------
//				addBoard (ID_PCI_DIO_32_HS, 1);
//		        addBoard (ID_PCI_6713, 2);
//		//        addBoard (ID_PCI_6713, 3);
//		    }
//			HARDWARE_setDigitalOutputDirection ();
//			break;
	}
	config->hardwareTypeBooted = config->hardwareType;
	GPIB_detectBoard (panel, ctrl);
	HARDWARE_sendStoredValues(0);
	pprintf (panel, ctrl, "====================================================================\n");
	
	return 0;
}


