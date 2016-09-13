#ifndef CONTROL_ADWIN
#define CONTROL_ADWIN


#include "general.h"					
// configuration



#define ADWIN_MAX_SLOTS 13


#define ADWIN_DIO1 1
#define ADWIN_DIO2 2
#define ADWIN_DIO3 3

#define ADWIN_AOUT1 4
#define ADWIN_AOUT2 5
#define ADWIN_AOUT3 6
#define ADWIN_AOUT4 7
#define ADWIN_AOUT5 8
#define ADWIN_AOUT6 9

#define ADWIN_AOUT7 10
#define ADWIN_AOUT8 11


#define ADWIN_PROCESS_DATA_OUT 1
#define ADWIN_PROCESS_DIRECT_IO 2


#define ADWIN_MAX_CYCLES 2147483646  

// =============================
//        digital output
// =============================

#define ADWIN_P1_PAR_CYCLE_NO		   	1	 // par_1
#define ADWIN_P1_PAR_MAXCYCLE_NO		2	 // par_2
#define ADWIN_P1_PAR_STOPFLAG          	3	 // par_3
#define ADWIN_P1_PAR_ERRORCODE          4	 // par_4
#define ADWIN_P1_PAR_nDIOelements    	5    // par_5
#define ADWIN_P1_PAR_DIOelementIndex	6    // par_6
#define ADWIN_P1_PAR_MAX_DIO_buffer 	7 	 // par_7
#define ADWIN_P1_PAR_nDIOmodules	    8 	 // par_8

#define ADWIN_P1_PAR_AOnElements		10  
#define ADWIN_P1_PAR_AOeventIndex 		11  
#define ADWIN_P1_PAR_nAOmodules			14 	
#define ADWIN_P1_PAR_MAX_AO_buffer		15

#define ADWIN_P2_PAR_outputFlag			20       // par_20
#define ADWIN_P2_PAR_outputModule  		21 	// par_21
#define ADWIN_P2_PAR_outputChannel  	22 	// par_22
#define ADWIN_P2_PAR_outputValue 		23 	// par_23
#define ADWIN_P1_PAR_trigger			26  // par_26
#define ADWIN_P1_PAR_N_triggerTimes	    28
#define ADWIN_P1_PAR_MAX_TriggerTimes   29


#define ADWIN_P1_DATA_DIObuffer 		1 // times are stored in data_1
#define ADWIN_P1_DATA_DIOmodules		2	// data_2
#define ADWIN_P1_DATA_AObuffer			3 // data_3
#define ADWIN_P1_DATA_localBuffer		4 // data_4
#define ADWIN_P1_DATA_AOvaluesAtEndOfSequence 5 // data 5
#define ADWIN_P1_DATA_AOmodules			6	// data_6
#define ADWIN_P1_DATA_triggerTimesDuringSeq 7
#define ADWIN_P1_DATA_analogOffsetsDuringSeq 8



#define ADWIN_P2_DATA_directIOValues 	12 // data_12



#define ADWIN_ERRORCODE_AO_BUFFER_EMPTY -1
#define ADWIN_ERRORCODE_PROCESS_TOO_LONG -2




// =============================
//        analog output
// =============================




#define ADWIN_NCOPY_ELEMENTS			32 // number of elements to be copied in each cycle
										   // from extended to local memory on adwin

#define ADWIN_TRIGGER_DIGIO0 1
#define ADWIN_TRIGGER_DIGIO1 2



// ================================================
//           Process 2: direct IO
// ================================================


#define ADWIN_P2_OUTPUT_ALL 1
#define ADWIN_P2_OUTPUT_SINGLE_CHANNEL_ANALOG 2
#define ADWIN_P2_OUTPUT_SINGLE_CHANNEL_DIGITAL 3






// ================================================
//      Process 3: data output
// ================================================



#define ADWIN_ERR_TIMEOUT -2


#define N_DAC_CHANNELS  (8*MAX_AO_DEVICES)

#define ADMIN_MAX_OFFSET_INDEX (ADWIN_MAX_SLOTS << 4)


#define ADWIN_TIMEBASE_ANALOG (2*VAL_us)
#define ADWIN_TIMEBASE_DIGITAL VAL_us


typedef struct {
	
	// organization of AO_BUFFER
    // 1 value = 2 longs
  	// long 1: timestamp
    // long 2: upper 16 bit: value, 
  	//    11111111 11111111 11111111 1111 1111
    //    | value         | 		 | DAC nr|
	long nAOelements;   // number of events in buffer (each events uses 2 longs)
	long *AObuffer;
	long AOvaluesAtEndOfSequence[N_DAC_CHANNELS]; 
	
	long nDIOelements;
	long *DIObuffer;    // odering: 1st long: timestamp, then one long per DIO module
	
	
	long maxCycleNo; 		// number of cycles (1 cycle = 1 µs)
	int trigger;
	
	long *triggerTimesDuringSeq;
	int nTriggerTimesDuringSeq;
	
	int nGpibCommands;
	void **gpibCommandPtr;
	int  *gpibCommandExecutionCycle;
	
	long AOoffsetsDuringSequence[ADMIN_MAX_OFFSET_INDEX];
// =======================================
//  phase feedback
// =======================================	

	int phaseFeedbackActive;
	char phaseFeedbackImageFilename[MAX_PATHNAME_LEN];
	char phaseFeedbackProtocolFilename[MAX_PATHNAME_LEN];
	double phaseFeedbackDeltaU[2];
	void *phaseFeedbackWfm[2];
	int repetition; // == seq->repetition+1
	int startNr;

//
// SEQUENCE sequence
//
	void *callingSequence;
	
} t_adwinData;



t_digitizeParameters *digitizeParameters_ADWIN (void);


void ADWINDATA_init (t_adwinData *a);

void ADWINDATA_free (t_adwinData *a);


void HARDWARE_ADWIN_addConfig (void);

int Set_Par_Err (short index, long value);

void HARDWARE_ADWIN_getSystemInfo (void);

int HARDWARE_ADWIN_boot (int panel, int ctrl);

int HARDWARE_ADWIN_configureOutput (void);


int HARDWARE_ADWIN_sendDirectAnalogValue (int dacChannel, double analogValue);

int HARDWARE_ADWIN_sendDirectDigitalValue (int channel, short bit);


int HARDWARE_ADWIN_transmitAnalogOffsetDouble (double values, int index);


void HARDWARE_ADWIN_initOutCtrl (int panel, int control);


int HARDWARE_ADWIN_dataOutput (t_adwinData *a, float maxWaitingTime_s, int waitUntilOutputFinished, int displayMessage);


int HARDWARE_ADWIN_stopDataOutput (void);

int HARDWARE_ADWIN_displayPointsGenerated (void);

int HARDWARE_ADWIN_isDataOutputFinished (void);


int HARDWARE_ADWIN_feedback (t_adwinData *a, int callNo);




#endif
