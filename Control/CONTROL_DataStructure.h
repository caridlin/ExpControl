#if !defined(DATA_STRUCTURE)
#define DATA_STRUCTURE

#include "CONTROL_Functions.h"   

#include "toolbox.h"   
#include "CONTROL_HARDWARE_adwin.h"					
#include "general.h"
#include "tools.h"
#include <NIDAQmx.h>
#include "CONTROL_DMDIMAGE.h"
#include "CONTROL_DMDSHAPE.h"   
 



#define ProgramVersion 6.35 // last number should be svn version

#define SHOWTIMES 0


// max. number of digital channels
#define N_BOARD_DIO_CHANNELS 32
#define N_TOTAL_DIO_CHANNELS (MAX_DIO_DEVICES*32)

 	
	
// RTSI lines for triggering
// START_TRIGGER
#define RTSI_START_TRIG    ND_RTSI_5
#define RTSI_START_TRIG_NR         5

#define RTSI_STOP_TRIG     ND_RTSI_4
#define RTSI_STOP_TRIG_NR  		   4

#define PROTOCOLFILE_active 0
#define PROTOCOLFILE_name "protocol.txt"
	
#define TRIGGER_SLOPE_POS 0
#define TRIGGER_SLOPE_NEG 1 

#define TRIGGER_SIGNAL_EXT (-2)



#define SEQ_STATUS_GET_STATUS  -1
#define SEQ_STATUS_STARTED     1
#define SEQ_STATUS_FINISHED    0
#define SEQ_STATUS_INTERRUPTED 2
#define SEQ_STATUS_IDLE_STARTED 3



// hardware timebases (values for
// DIO_timebase, und AO_timebase
// in data structure t_sequence
#define VAL_HARDWARE_TIMEBASE_50ns  -3    
#define VAL_HARDWARE_TIMEBASE_1us   1
#define VAL_HARDWARE_TIMEBASE_10us  2
#define VAL_HARDWARE_TIMEBASE_100us 3
#define VAL_HARDWARE_TIMEBASE_1ms   4 


char *str_AnalogChannelNames (int ch);

char *str_ChannelNames (int ch);


extern const char UIR_File[];
extern const char UIR_common[];

#define kByte 1024



#define SEQUENCE_defaultFilename "untitled.seq"
#define SEQUENCE_fileSuffix       "*.seq"
#define SEQUENCE_fileSuffixP      ".seq"


#define DIO_CHANNELSTATUS_STANDARD 0
#define DIO_CHANNELSTATUS_ALWAYSON 1
#define DIO_CHANNELSTATUS_ALWAYSOFF 2


// keine Analogsequenz
//#define AS_NONE 0

// Trigger-Quellen vom Analog-Board
#define VAL_NO_TRIGGER -1


#define MAX_GPIBDEVICES 32


//#define MAX_EXTERNAL_AO_CHANNELS 2 
//#define N_LECROY_LW120_CHANNELS (2*MAX_GPIBDEVICES)

//#define N_AO_CHANNELS (N_DAC_CHANNELS+N_LECROY_LW120_CHANNELS+MAX_EXTERNAL_AO_CHANNELS)

//#define FIRST_EXTERNAL_AO_CHANNEL (N_DAC_CHANNELS+N_LECROY_LW120_CHANNELS)

#define MAX_EXT_DEV_NAME_LEN 80

//********************************************************************
//
//    data structure t_extDevice
//
//********************************************************************
typedef struct {
	char name[MAX_EXT_DEV_NAME_LEN];
	int  deviceTyp; // deviceTyp
	int port;     // Device port for TCP/IP connection
	char	address[256];	//IP adress
	int positionInList;   // number in List
	int positionInTable;   // number in Table
	unsigned int uniqueID;	//ID to connect the command with the right device.	
} t_ext_device;

// -------------------------------------
//    definition of DIO channels
//    on analog board
// -------------------------------------
//#define DIO_WATCHDOG          0

#define DAC_CHANNEL_COUNTS    7

#define VAL_NO_RESET_REPETITIONS 99999


//=============================================================
//
//    t_config
//
//	  all global configuration data
//
//=============================================================



// was 40
#define TABLECONFIG_NAME_LEN 100

typedef struct {
	char name[TABLECONFIG_NAME_LEN];
	int DIOvisible[N_TOTAL_DIO_CHANNELS];
	int AOvisible[N_DAC_CHANNELS];
} t_tableConfig;


void TABLECONFIG_init (t_tableConfig *c, int nr);



#define HARDWARE_TYPE_NATINST 0
#define HARDWARE_TYPE_ADWIN 1


#define N_TABLE_CONFIGS 3

typedef struct {
// ========================================
//    GENERAL settings 
// ========================================
	char *defaultPath;
	int enableGPIB;
	int enableTCP;
	int launchDataSocketServer; 

	int CHECK_DISABLED;
	int DEBUGMODE;		// is set to 1 by command line parameter "/DEBUG" 
	int REMOTE;         // is set to 1 by command line parameter "/REMOTE"
//	int FAKEDATA;
	
	int displayAbsoluteTime;
	int buttonWidth;
	int buttonHeight;
	
	int watchdogActivate;
	int watchdogAutoRepeat;
	int suppressProtocol;
	
	int autoSaveSequencesAfterEachStart;
	char autoSaveSequencesPath[MAX_PATHNAME_LEN];
	int createFilenameForImages;
	char imageFilenameExtensionPath[MAX_PATHNAME_LEN];
	char dataPathToday[MAX_PATHNAME_LEN];
	int startNr;
	
	int splitterVPos;
	int splitterHPos;
	
// ========================================
//    HARDWARE configuration 
// ========================================
	int hardwareType;
	ListType listOfBoards;   // list of all boards (NI type codes)
	int hardwareTypeBooted;

	int loadAdwinBoardsFromConfig;
	unsigned long nAnalogBoards;
	unsigned long nDigitalBoards;
	int digitalBoardID[MAX_DIO_DEVICES];
	int analogBoardID[MAX_AO_DEVICES];
	int invertDigitalOutput;

	
// ========================================
//    GPIB configuration
// ========================================
	int ignorePresenceOfGpibLeCroyDevices;
	int gpibBoardIndex;
	int nGpibDevices;
	ListType listOfGpibDevices;
	
// ========================================
//    SEQUENCES 
// ========================================
	ListType listOfSequences;
	int activeSequenceNr;
	int idleSequenceNr;
	int calibrationSequenceNr;

// ========================================
//    DISPLAY
// ========================================
	Rect panelBoundsMain;
	Rect panelBoundsMain2;
	int panelMain2IsZoomed;
	Point panelPosSimulate;
	int useTwoScreens;
	t_tableConfig tableConfig[N_TABLE_CONFIGS];
	
	

// ========================================
//    IDLE SEQ AND PIFOC AUTO FOCUS
// ========================================
	char pifocFocusVoltageLogFilePath[MAX_PATHNAME_LEN]; // log filename including path for pifoc focus voltages
	char idleSequenceDataPath[MAX_PATHNAME_LEN]; // path to which idle sequence images are saved
	int pifocVoltageChannel; // the channel number of the pifoc voltage 
	double lastPifocFocusVoltage; // the last valid pifoc focus voltage
	
// ========================================
//    DMD TOOLS
// ========================================
	char pathMagickConvert[MAX_PATHNAME_LEN]; // path to image magick convert for DMD       
// ========================================
//    DMD Settings
// ========================================	
	unsigned long DMDnDevId, DMDnSeqId; //needed for initialization of DMD. unsigned long is same as ALP_ID
	
} t_config;

extern t_config *config;  

void CONFIG_initBoards (t_config *c);

void CONFIG_init (t_config *c);

void CONFIG_free (t_config *c);




//********************************************************************
//
//    t_outputData
//
//    (intermediate data format, see function 'createOutputData')
//    Here, the output data is stored in a 'compressed format'
//    which is to be expanded into the output buffers. 
//
//********************************************************************

#define MAX_CHANNEL_VECT 8


typedef struct {
	short value[MAX_CHANNEL_VECT];
} t_analogSample;


void ANALOGSAMPLE_init (t_analogSample *a);


typedef struct {
	__int64 time;
	unsigned int duration;
	unsigned long digValue[MAX_DIO_DEVICES];
} t_digitalEvent;					


int CVICALLBACK DIGITALEVENT_compare (void *e1, void *e2);

void DIGITALEVENT_init (t_digitalEvent *e);

void DIGITALEVENT_free (t_digitalEvent *e);

t_digitalEvent *DIGITALEVENT_new (ListType list, __int64 time, unsigned int duration, unsigned long *digValues);

void DIGITALEVENT_copyValues (t_digitalEvent *target, t_digitalEvent *source);


typedef struct {
    unsigned __int64 duration;       
    int nCopies;
// ========================================
//    DIGITAL output: parameters 
// ========================================
    unsigned long DIOdeltaT;       			//  timebase       
    unsigned __int64 DIO_totalSamples;  			//  total number of values 
// ========================================
//    DIGITAL output: 32 bit-values
// ========================================
	ListType lDigitalEvents; 				// List of digital events


	unsigned long numDIOValues;   			// number of different digital bit patterns  
    							   			// in DIO_Values, DIO_REPEAT
    unsigned long *DIO_Values[MAX_DIO_DEVICES];  // all different digital values
    unsigned long *DIO_Repeat ;  // number of repetitions for each
    							   			// digital value
//   	unsigned long *DIO_OutBuf[MAX_DIO_DEVICES];        

// ========================================
//    DIGITAL output: doubleBuffering
// ========================================
	int DIO_dblBuf;
	unsigned __int64 DIO_totalSamplesWritten;
	unsigned long DIO_repCnt;
	unsigned long DIO_valueNr;
	unsigned long DIO_copyNr;
	
	
// ========================================
//    ANALOG output: parameters 
// ========================================
    unsigned long AOdeltaT;		   			    // timebase 
    unsigned __int64 AO_totalSamples;   		    // total number of values

    int   AOnChannels[MAX_AO_DEVICES];			// number of channels 
    short AOchannelVect[MAX_AO_DEVICES][MAX_CHANNEL_VECT];	    // active channels 

// ========================================
//    ANALOG output: digitized 12/16 bit-values for each channel
// ========================================
    unsigned long maxAOValues[N_DAC_CHANNELS];// size of allocated memory
    unsigned long numAOValues[N_DAC_CHANNELS];// Anzahl Werte in 
                                             // AO_Values[0..N_DAC_Channels], AO_Repeat[CH]
    long *AO_Values[N_DAC_CHANNELS]; // 
    unsigned long *AO_ValueDuration[N_DAC_CHANNELS]; // duration of each value
	int	          AO_ShiftSamples[N_DAC_CHANNELS];

	int AO_nSamples[MAX_AO_DEVICES];
	t_analogSample *AO_Samples[MAX_AO_DEVICES];		   // combines AO Values for NI devices 
    unsigned long *AO_SampleRepeat[MAX_AO_DEVICES]; 

	
// ========================================
//    ANALOG output: retain last values
// ========================================
    unsigned long last_numAOValues[N_DAC_CHANNELS];// Anzahl Werte in 
                                                  // AO_Values[0..N_DAC_Channels], AO_Repeat[CH]
    long         *last_AO_Values[N_DAC_CHANNELS];  
    unsigned long *last_AO_ValueDuration[N_DAC_CHANNELS]; // duration of each value
    short last_constAOvoltage[N_DAC_CHANNELS];

// ========================================
//    NI: ANALOG output: doubleBuffering
// ========================================
	int AO_dblBuf;
	unsigned __int64 AO_totalSamplesWritten;
	unsigned long AO_repCnt[MAX_AO_DEVICES];
	unsigned long AO_valueNr[MAX_AO_DEVICES];
	unsigned long AO_copyNr[MAX_AO_DEVICES];
	
	
// ========================================
//	trigger
// =======================================
	long *triggerTimesDuringSeq;
	int nTriggerTimesDuringSeq;

// ========================================
//    ADWIN output Data 
// ========================================	
	t_adwinData *adwinData;

// ========================================
//    general  
// ========================================
    int variableOutput;		// flag if there is any block with 
    					    // variable time or stepped output
	int nActiveDACChannels;
	t_digitizeParameters *digParameters;
	int simulate;
} t_outputData;


void OUTPUTDATA_freeDigitalEvents (t_outputData *o);

//void OUTPUTDATA_freeBuffers (t_outputData *o);

void OUTPUTDATA_free (t_outputData *o, int freeLastValues);

void OUTPUTDATA_init (t_outputData *o, int initLastValues);

void OUTPUTDATA_resetCounters (t_outputData *o);






#define MAX_AXISNAME_LEN 80
#define MAX_UNIT_LEN 10




enum {
	WFM_TABLE_SORT_MODE_CHANNEL,
	WFM_TABLE_SORT_MODE_NAME,
	WFM_TABLE_SORT_MODE_NR,
	WFM_TABLE_SORT_MODE_NPOINTS,
	WFM_TABLE_SORT_MODE_TYPE,
	WFM_TABLE_SORT_MODE_USEDBY
};


enum {   //Sort modes for the external device list
	EXTDEV_TABLE_SORT_MODE_ID,
	EXTDEV_TABLE_SORT_MODE_NAME,
	EXTDEV_TABLE_SORT_MODE_TYPE,
	EXTDEV_TABLE_SORT_MODE_INFO,
};

//********************************************************************
//
//    t_sequence
//
//    stores complete information of a sequence
//
//********************************************************************
#define MAX_DATESTR_LEN 30

typedef struct {
// ========================================
//    general info
// ========================================
   	char filename[MAX_PATHNAME_LEN];
	char *description;
	char dateLastModified[MAX_DATESTR_LEN];
   	double programVersionCreatedWith;
	int readOnly;			     // reflects state of file attribute (WP)
    int shortcut;				  // shortcut (F5..F8)
    							
    int maxDigitalChannel;		  // number of active digital channels
    int maxAnalogChannels;		  // number of active analog channels
	
    int trigger;	              // external trigger 
    							  // (possible values are VAL_NO_TRIGGER,
    							  //  NI_PFI_0, NI_PFI_1, ... NI_PFI_9);
    							     
    int triggerPolarity;
    int considerWatchdog;
    int generateRandomCounterData; // for debugging only
	char seqShortName[MAX_PATHNAME_LEN];
	int startNr;
	char startDate[MAX_DATESTR_LEN];  // date of latest Start

						 
// ========================================
//    DIGITAL output 
// ========================================
    char *DIO_channelNames[N_TOTAL_DIO_CHANNELS];	 // names of digital channels
    int DIO_channelStatus[N_TOTAL_DIO_CHANNELS];   // always on /off etc
    int DIO_invertDisplay[N_TOTAL_DIO_CHANNELS];
	int DIO_lockChannel[N_TOTAL_DIO_CHANNELS];
	int DIO_timebase_50ns;
    ListType lDigitalBlocks;      // list of digital blocks 
    							  // (see t_digitalBlock)

// ========================================
//    ANALOG output 
// ========================================
    char *AO_channelNames[N_DAC_CHANNELS];       // names of analog channels
	int AO_timebase_50ns;

    ListType lWaveforms;		 // list of waveforms (see t_waveform)
	int wfmTableSortMode;
	int wfmTableSortDescending;
	
	//External devices
	int extdevTableSortMode;
	int extdevTableSortDescending;
	ListType lExtDevices;	//list of external devices (see t_ext_device)
	t_ext_device *noExternalDevice;
	
    ListType lGpibCommands;
    ListType lTransferFunctions; // list of Transfer functions 
    							//	(used for calculation of waveforms)
	ListType lCCDSettings;		 // list of CCD settings
//   display
	int DAC_tableRowHeight[N_DAC_CHANNELS];

// ========================================
//    number of repetitions, runs etc.
// ========================================
    int nRuns;
    int nRepetitions;
    int runStart;
    int repetitionStart;
	int repetitionStop; // ignored if set to 0
	int nCopies;    



	// alternative: execute only a some specific repetitions
	unsigned nRedo;			   // number of repetitions
	int *redoRepetitions;  // vector (of nRedo elements) containing 
						   //  the repetitions to redo
	int *redoRuns;


// ========================================
//    data collection
// ========================================
	int remote;
	

	t_tableConfig tableConfig;
// =============================================
// =============================================
//    variables used during runtime only
// =============================================
// =============================================
	int changes;				 // true if changes made
	//int isRunning;
    
	t_outputData *outData;		 // pointer to generated output data
	
    
	int nExecutionsDone;
    int run;
    int repetition; // 0 for first repetition.
	
    int inLoop;				
    int loopsToDo;
    int loopStartNr;
	ListType lParameterSweeps;
	__int64 referenceTime;
	int panelSimulate;

	unsigned int *simulate_DIO_buffer[MAX_DIO_DEVICES];
	short *simulate_AO_buffer[MAX_AO_DEVICES];
	unsigned __int64 simulate_DIO_bufferSizeSamples;
	unsigned __int64 simulate_AO_bufferSizeSamples;
	int firstRepetition;
	
	t_tableConfig lastConfig;
	char currentPath[MAX_PATHNAME_LEN];
	void *phaseFeedbackWfm[2];
	
// ========================================
//    idle sequence and pifoc auto focus
// ========================================
	int enableFocusFeedback; // if focus feedback is enabled
	double manualPifocFocusVoltage; // pifoc focus voltage which can be manually set.
	
	int autoFocusNumImages;
	double autoFocusCenterVoltage;
	double autoFocusStepVoltage;
	int autoFocusFirstBlock;
	int isOfTypeIdleSeq; 
	
} t_sequence;




void SEQUENCE_init (t_sequence *seq);

void SEQUENCE_initSimulateBuffers (t_sequence *seq);


void SEQUENCE_free (t_sequence *seq);

void SEQUENCE_freeSimulateBuffers (t_sequence *seq);

t_sequence *SEQUENCE_new (void);

t_sequence *SEQUENCE_ptr (int nr);

void SEQUENCE_freeRedo (t_sequence *seq);		


void SEQUENCE_addDateLastModified (t_sequence *seq, int month, int day, int year);


int SEQUENCE_hasLoops (t_sequence *seq);


void SEQUENCE_detectLoops (t_sequence *seq, int maxBlocksPerLoop, int minmumPeriod, 
						   int progressDialog, int checkPanels);


int SEQUENCE_hasSweeps (t_sequence *seq);

int SEQUENCE_getNumberOfGpibCommandsDuringSequence (t_sequence *seq);


char *SEQUENCE_DACchannelName (t_sequence *seq, int ch);


int DIGITIZEPARAMETERS_compare (t_digitizeParameters *p1, t_digitizeParameters *p2);


int SEQUENCE_isIdleSequence(t_sequence *seq);

double* SEQUENCE_calculateAutoFocusValues(int autoFocusNumImages, double autofocusCenterVoltage, double autofocusStepVoltage);

void SEQUENCE_updateAutoFocusValues (t_sequence *seq);

int SEQUENCE_getSlicingFrequency(t_sequence *seq, double* freqOffsetMHz, double* centerFreqMHz);
	

//********************************************************************
//
//    WAVEFORMS
//
//********************************************************************


typedef struct {
	int varyTime;
	double timeStart_ns;
	double timeIncrement_ns;
	int varyValue;
	double valueStart;
	double valueIncrement;
	int stepRepetitions;

	double thisTime_ns;
	double thisValue;
} t_point;


void POINT_PlotLine (int panel,int control,t_point *p ,t_point * pPrime , int couleur);

void POINT_init (t_point *p);

int POINT_isEqual  (t_point *p1, t_point *p2);


// ========================================
//   different types of external devices
// ========================================
enum { EXTDEV_TYPE_NONE  = -1,
	   EXTDEV_TYPE_TELNET,
	   
      N_EXTDEVTypes
};



// ========================================
//   different types of waveforms
// ========================================
enum { WFM_TYPE_NONE  = -1,
	   WFM_TYPE_POINTS, 
       WFM_TYPE_FILE,
       WFM_TYPE_STEP,
       WFM_TYPE_HOLD,
       WFM_TYPE_CONSTVOLTAGE,
	   WFM_TYPE_FUNCTION,
	   WFM_TYPE_ADDRESSING,
 
      N_WFMTypes
};

// ========================================
//   default texts+ IDs
// ========================================
#define WFM_ID_UNCHANGED        0
#define WFM_ID_HOLD     		-2
#define WFM_ID_CONSTVOLTAGE     -3

#define strHold 	     "HOLD "
#define strUnchanged    "UNCHANGED"
#define strConstVoltage "CONSTANT VOLTAGE"



#define SWEEP_CHANGEMODE_REPETITION 0
#define SWEEP_CHANGEMODE_CURVE      1



#define MAX_WFM_NAME_LEN 80
#define MAX_VALUE_UNITS_LEN 20
#define MAX_VALUE_NAME_LEN 20
#define MAX_WFM_USEDBY_STR_LEN 40

#define WFM_CHANNEL_ALL -1
#define WFM_CHANNEL_NONE -2
#define WFM_CHANNEL_GPIB -3

#define MAX_EXTDEV_NAME_LEN 80

// better to use here a odd number to have a real center
#define WFM_ADDR_MATRIXDIM 31
// lets have a fixed amount of 20 matrices
#define WFM_ADDR_MATRIX_NUM 20
//#define WFM_ADDR_N_CALIBRATION_PARAMETERS 19

enum {
	WFM_ADDR_CAL_a1x,
	WFM_ADDR_CAL_a1y,
	WFM_ADDR_CAL_b1x,
	WFM_ADDR_CAL_b1y,
	WFM_ADDR_CAL_U20,
	WFM_ADDR_CAL_a2x,
	WFM_ADDR_CAL_a2y,
	WFM_ADDR_CAL_b2x,
	WFM_ADDR_CAL_b2y,
	WFM_ADDR_CAL_U10,
	WFM_ADDR_CAL_theta1,
	WFM_ADDR_CAL_theta2,
	WFM_ADDR_CAL_lambda,
	WFM_ADDR_CAL_minThresh,
	WFM_ADDR_CAL_maxThresh,
	WFM_ADDR_CAL_minSigma,
	WFM_ADDR_CAL_maxSigma,
	WFM_ADDR_CAL_offsetU1,
	WFM_ADDR_CAL_offsetU2,
	WFM_ADDR_CAL_offsetPhi1,
	WFM_ADDR_CAL_offsetPhi2,
	WFM_ADDR_CAL_enableCal1,
	WFM_ADDR_CAL_enableCal2,
	WFM_ADDR_CAL_phaseFeedbackOff,
	WFM_ADDR_CAL_phaseErrThreshold,
	WFM_ADDR_CAL_DMDpxPerSiteX,			// scaling for the drawing of patterns
	WFM_ADDR_CAL_DMDpxPerSiteY,
	WFM_ADDR_CAL_DMDpxPerSiteXfeedback, // scaling for the feedback (negative value allowed)
	WFM_ADDR_CAL_DMDpxPerSiteYfeedback,
	WFM_ADDR_CAL_DMDpxPerSiteXbox,		// size of the box drawn to address a lattice site
	WFM_ADDR_CAL_DMDpxPerSiteYbox,
	WFM_ADDR_CAL_flipPhi,  // flip phi1 <-> phi2
	WFM_ADDR_CAL_DMDlattXangle,
	WFM_ADDR_CAL_DMDlattYangle,
	WFM_ADDR_CAL_DMDimgCenterX,  // image coordinates where the center of the matrix falls to
	WFM_ADDR_CAL_DMDimgCenterY,
	
	
	WFM_ADDR_N_CALIBRATION_PARAMETERS 

};	



enum {
    WFM_ADDR_TRAJ_DURATION_MS,
    WFM_ADDR_TRAJ_NPOINTS,
    WFM_ADDR_TRAJ_RADIUS1,
    WFM_ADDR_TRAJ_RADIUS2,
	WFM_ADDR_TRAJ_REVOLUTIONS,

	WFM_ADDR_N_TRAJ_PARAMETERS
};


enum {
	WFM_ADDR_TYPE_SINGLESITE,
	WFM_ADDR_TYPE_TRAJECTORY,
	WFM_ADDR_TYPE_DMD
};


enum {
	WFM_STEP_TYPE_NONE,
	WFM_STEP_TYPE_TO_LINEAR,
	WFM_STEP_TYPE_LIST
};
extern const char* WFM_STEP_TYPE_STR[];

enum{
	DMD_DISORDERTYPE_2D = 0,
	DMD_DISORDERTYPE_horizontal,
	DMD_DISORDERTYPE_vertical
};



//********************************************************************
//
//    data structure t_waveform
//
//********************************************************************


typedef struct {
	int  stepType; // WFM_STEP_TYPE_NONE, ...
	double from;
	double to;     // only for WFM_STEP_TYPE_TO_LINEAR
	double* list;  // only for WFM_STEP_TYPE_LIST
	unsigned long listSize; // size of list
} t_step_param;

// t_step_param Funktionen

void STEP_PARAM_getListString(char* result, t_step_param* p);

typedef struct {
// ========================================
//   general
// ========================================
    char name[MAX_WFM_NAME_LEN];
    int type;          // point sequence, file, step 
    int hide;
	int channel;       // channel where the waveform is played
    
// ========================================
//   WFM_TYPE_FILE
// ========================================
    char *filename;
    double uScaleFrom;	  // scaling factor voltage
    double uScaleTo;	  // scaling factor voltage
	int uScaleVary;
	
    double offsetFrom;	  // offset voltage
	double offsetTo;
	int offsetVary;
	int timebaseStretch;   // multiplication factor of timebase 
	
 // also for WFM_TYPE_FUNCTION
	__int64 durationFrom_50ns; // duration
	__int64 durationTo_50ns; // duration
	int durationVary;
    
// ========================================
//   WFM_TYPE_POINTS, WFM_TYPE_FILE
// ========================================
    int nPoints;            // #points 
    double *uList;   // list of values (from file)
	t_point *points;
    
    int transferFunction;
    char strValueName [MAX_VALUE_NAME_LEN];
    char strValueUnits[MAX_VALUE_UNITS_LEN];
    
// ========================================
// 	  WFM_TYPE_STEP: stepped output
// ========================================
    double stepFrom;         // start voltage
    double stepTo;			 // stop voltage
    int   stepRep;			 // step every n-th repetition
    int   stepChangeMode;
    int   stepAlternate;

// ========================================
//   WFM_TYPE_FUNCTION
// ========================================
	int functionID;
	t_step_param **functionParameters; // 2D array of *t_step_param:  first index: functionID, second index: parameter index (used to save all parameters for all function to avoid information loss if one selects another function and jumps back again)
	
	//double **functionParametersFrom;
	//double **functionParametersTo; // only use for step type WFM_STEP_TYPE_TO_LINEAR
	//int **functionParametersVary; // contains the step type WFM_STEP_TYPE_NONE,... for each function parameter
	int functionReverseTime;


// ========================================
//   WFM_TYPE_ADDRESSING
// ========================================
	int addrLinkWfm;// link to other waveform with this id
	int addrType;
	
	int stepAddrMatrix;
	int addrMatricesPerRepetition;
	int addrMatrixOffsetX; // shifts the position of the addr matrix on the DMD (in sites)
	int addrMatrixOffsetY;
	unsigned int *addrMatrix; //first dim num matrix, second dim column, third dim encoded in int
	double addrCalibration[WFM_ADDR_N_CALIBRATION_PARAMETERS];		  // parameters 0-3: slopes, 4-8: offsets, 9,10 U10, U20
	double addrCalibrationStepTo[WFM_ADDR_N_CALIBRATION_PARAMETERS];		  // parameters 0-3: slopes, 4-8: offsets, 9,10 U10, U20
	int addrCalibrationStep[WFM_ADDR_N_CALIBRATION_PARAMETERS];		  // parameters 0-3: slopes, 4-8: offsets, 9,10 U10, U20
	int addDisorder[WFM_ADDR_MATRIX_NUM];		  //parameter for each single site addressing pattern if disorder should be added.
	unsigned int disorderSeed[WFM_ADDR_MATRIX_NUM];		  //parameter for each single site addressing pattern for disorder amplitude.
	int disorderTyp[WFM_ADDR_MATRIX_NUM];
	
	double addrTrajectParams[WFM_ADDR_N_TRAJ_PARAMETERS];
	
	Point addrCenter;										
	char *addrTestFilename;
	char *addrTestFilename2;
	int addrEnablePhaseFeedback;
	int addrEnablePhaseFeedback2;
	int addrAddLastOffset;
	int addrAddLastOffsetNotAtFirst;
	int addrCamera;
	int addrImageSize;
	t_DMDimage *addrDMDimage;
	
	

	// runtime only
	//double addrOffsetThisRepetition[2];
	double addrCalibrationThisRepetition[WFM_ADDR_N_CALIBRATION_PARAMETERS]; // parameters 0-3: slopes, 4-8: offsets, 9,10 U10, U20  
	
	
// ========================================
//    stepped waveform to add
// ========================================
    int add;

// ========================================
// 	  digitized values
// ========================================
//    unsigned long DIG_Timebase; // timebase of last digitalization
    unsigned long DIG_NValues;	// #digitized values
    long 		  *DIG_values;  // digitized values
    unsigned long *DIG_repeat;  // number of repetitions for each value
    
// ========================================
// 	  auxilary variables
// ========================================
	int digitize;
	t_digitizeParameters digParameters;
	int menuID;				   // menu ID used when editing in DACs
	
//    double duration_ns;

	double uScaleThisRepetition;  
	double offsetThisRepetition;  
	double durationThisRepetition_ns;
	__int64 durationThisRepetition_50ns;
	double *functionParametersThisRepetition;  // function paramaters for this repetition for the currently selected function
	int isActive;
	int positionInTable;
	int positionInList;   // number in List
	char usedBy[MAX_WFM_USEDBY_STR_LEN];
	int asked;
} t_waveform;



void WFM_init (t_waveform *wfm, int type);

void WFM_free (t_waveform *w);  

t_waveform *WFM_new (t_sequence *seq);

t_waveform *WFM_ptr (t_sequence *seq, int wfmID);

void WFM_duplicate (t_waveform *dest, t_waveform *source);

void WFM_delete (t_sequence *seq, int nr);

void WFM_deleteWithReferences (t_sequence *seq, int nr);


t_waveform *WFM_holdWaveform (void); 

int WFM_hasVariableParameters (t_waveform *wfm);

int WFM_isEqual (t_waveform *w1, t_waveform *w2);

int WFM_nrFromName (t_sequence *seq, const char *name, int searchEnd);

t_waveform *WFM_ptrFromName (t_sequence *seq, const char *name);
																	
int WFM_nrFromPtr (t_sequence *seq, t_waveform *wfm);

int WFM_nrFromMenuID (t_sequence *seq, int menuID);

int WFM_findIdentical (t_sequence *seq, t_waveform *wfm);

int WFM_nameExists (t_sequence *seq, char *name);

void WFM_checkAllIfActive (t_sequence *seq);

void WFM_FUNC_allocateMemory (t_waveform *wfm);

int WFM_nFuncParams (t_waveform *wfm);

void WFM_checkForDuplicateNames (t_sequence *seq);

int WFM_checkForDuplicateNameAndRename (t_sequence *seq, char *name);

char *WFM_name (t_sequence *seq, int nr);

void WFM_addChannels (t_sequence *seq); // upgrade from version 4.3 to 5.0

char *WFM_typeStr (t_waveform *wfm);

void WFM_addPositionInList (t_sequence *seq);

void WFM_fillInfos (t_sequence *seq);

void WFM_ADDR_allocateMatrix (t_waveform *w);

int WFM_ADDR_getMatrixValue (t_waveform *w, int matrixNum, int x, int y);

void WFM_ADDR_setMatrixValue (t_waveform *w, int matrixNum, int x, int y, int value);

t_waveform *WFM_ADDR_findDMDwaveform (t_sequence *seq);



/**************************************************
 *   
 *   t_transferFunction
 *
 **************************************************/
 
#define MAX_TRANSFERFUNCTNAME_LEN 50
#define MAX_TRANSFERFUNCT_AXIS_NAME_LEN 20
#define MAX_TRANSFERFUNCT_PARAMETERS 10 
#define MAX_TRANSFERFUNCT_PARAMETER_NAME_LEN 10 


enum {	
	TRANSFERFUNCT_TYPE_NONE,
	TRANSFERFUNCT_TYPE_POLYNOM,
	TRANSFERFUNCT_TYPE_SQRT,
	TRANSFERFUNCT_TYPE_ABC_FORMULA,
	
	N_TRANSFERFUNCT_TYPES
};

 
typedef struct {
 	 int type;
 	 char name[MAX_TRANSFERFUNCTNAME_LEN];
 	 char xAxis[MAX_TRANSFERFUNCT_AXIS_NAME_LEN];
 	 char yAxis[MAX_TRANSFERFUNCT_AXIS_NAME_LEN];
 	 int nParameters;
     double parameters[MAX_TRANSFERFUNCT_PARAMETERS];
     char parameterNames[MAX_TRANSFERFUNCT_PARAMETERS][MAX_TRANSFERFUNCT_PARAMETER_NAME_LEN];
  	 char xVariable[MAX_TRANSFERFUNCT_PARAMETER_NAME_LEN];
  	 char yVariable[MAX_TRANSFERFUNCT_PARAMETER_NAME_LEN];
//	 double defaultVoltage;
} t_transferFunction;


t_transferFunction *TRANSFERFUNCT_new (t_sequence *s);

void TRANSFERFUNCT_init (t_transferFunction *f);

void TRANSFERFUNCT_free (t_transferFunction *f);

t_transferFunction *TRANSFERFUNCT_copy (t_transferFunction *source);


double TRANSFERFUNCT_apply (t_transferFunction *f, double value);

double TRANSFERFUNCT_applyInv (t_transferFunction *f, double value);


t_transferFunction *TRANSFERFUNCT_ptr (t_sequence *s, int nr);

char *TRANSFERFUNCT_str (t_transferFunction *f);

int TRANSFERFUNCT_nr (t_sequence *s, const char *name);

t_transferFunction *TRANSFERFUNCT_ptrFromName (t_sequence *s, const char *name);

void TRANSFERFUNCT_plotInv (t_transferFunction *f, int panel, int ctrl, double minX, double maxX);

void TRANSFERFUNCT_plot (t_transferFunction *f, int panel, int ctrl, double minX, double maxX);

void TRANSFERFUNCT_writeFormulasToTextbox  (t_transferFunction *f, int panel, int ctrl);



//********************************************************************
//
//    Digital Blocks
//
//********************************************************************


#define BMODE_Standard  0
#define BMODE_LoopEnd   -2
#define BMODE_InLoop	-3
#define BMODE_MaxLoops  50000

#define MAX_DIGITALBLOCK_NAME_LEN 40

//********************************************************************
//
//    data structure t_digitalBlock
//
//********************************************************************

typedef struct {
    char blockName[MAX_DIGITALBLOCK_NAME_LEN];	// name
    char groupName[MAX_DIGITALBLOCK_NAME_LEN];	// group name 
	int groupNr;
	int isGroupExpanded;
	int waitForExtTrigger;
	int doTriggerLoop;
	int doRecvTriggerLoop;
	int disable;

// ========================================
// 	 times (all times in SMALLEST_UNITs [=50ns])
// ========================================
    unsigned long duration;  // block duration (as displayed)
    						  // if variable time is set, this is the startTime
    int variableTime;		  // flag, if time is variable
    int incrementTime;		  // increment per repetition (neg. values possible)
    int variableTimeExecuteRep;	// execute block every N repetitions
    int variableTimeExecuteRepOffset;	// execute block every N repetitions
    int variableTimeStepRep;   // step every N repetitions
    int variableTimeModulo;
    int isAbsoluteTimeReference;
    __int64 absoluteTime;
	int gpibCommandNr;

    unsigned long calcDuration;   // calculated duration of block 

// ========================================
// 	 Loops
// ========================================
    int blockMode;			  // 	0 : standard		
    						  //	N>= 1:	loop start (N = number of loops)
    						  //    -2   : loop end
    						  //    -3   : within loop
    											
   
// ========================================
//	  on / off
// ========================================
    int channels[N_TOTAL_DIO_CHANNELS];     // contents of digital channels (0 or 1)
    								  // if set to -1
	int alternate[N_TOTAL_DIO_CHANNELS];
	int varyPulse[N_TOTAL_DIO_CHANNELS];				  // is pulse variable (0 or 1)
	int varyPulseStartIncrement[N_TOTAL_DIO_CHANNELS];
	int varyPulseStopIncrement[N_TOTAL_DIO_CHANNELS];
	int varyPulseStepRep[N_TOTAL_DIO_CHANNELS];		  // increment every n repetitions
	

    int waveforms[N_DAC_CHANNELS];
    double constVoltages[N_DAC_CHANNELS];


// ========================================
// ========================================
//        changed during runtime only
// ========================================
// ========================================
    t_waveform *wfmP[N_DAC_CHANNELS];  // waveforms that are starting in this block
    int blockNr;				      // current number of this block
	t_sequence *seq;    			  // cross-reference sequence
	int tableColumnWidth;
	int nElementsInGroup;
	unsigned __int64 groupDuration;
	
	int *bitmapPtrArr;

} t_digitalBlock;
                        



void DIGITALBLOCK_reset (t_digitalBlock *block);

//void DIGITALBLOCK_resetTableAttributes (t_digitalBlock *b);

void DIGITALBLOCK_init (t_digitalBlock *block);

void DIGITALBLOCK_free (t_digitalBlock *b);

t_digitalBlock *DIGITALBLOCK_ptr (t_sequence *seq, int ID);

t_digitalBlock *DIGITALBLOCK_new (t_sequence *seq);

void DIGITALBLOCK_delete (t_sequence *seq, int nr);

t_digitalBlock *DIGITALBLOCK_insert (t_sequence *seq, int nr);

void DIGITALBLOCK_duplicate (t_digitalBlock *dest, t_digitalBlock *source);

unsigned long DIGITALBLOCK_getDuration (t_digitalBlock *block, int repetitionNo);

__int64  DIGITALBLOCK_calculateAllAbsoluteTimes(t_sequence *seq, int repetition);

int DIGITALBLOCK_areIdentical (t_digitalBlock *b1, t_digitalBlock *b2);


int DIGITALBLOCK_getLoopRepetitions (t_sequence *seq, int nr);

int DIGITALBLOCK_hasWaveform (t_digitalBlock *b);

int DIGITALBLOCK_hasConstVoltage (t_digitalBlock *b);

int DIGITALBLOCK_hasVariablePulse (t_digitalBlock *b);


void DIGITALBLOCK_freeGroupBitmaps (t_digitalBlock *b);

void DIGITALBLOCK_createGroupBitmapPtr (t_digitalBlock *b);




/**************************************************
 *   
 *  Data structure : GBIB-commands
 *
 **************************************************/
#define VAL_Hz 1.0
#define VAL_kHz 1.0E3
#define VAL_MHz 1.0E6
#define VAL_GHz 1.0E9

enum{
	GPIB_COMMANDTYPE_NONE = -1,
	GPIB_COMMANDTYPE_STRINGS = 1,
	GPIB_COMMANDTYPE_ARBWFM,
	GPIB_COMMANDTYPE_FREQ,
	GPIB_COMMANDTYPE_POWERSUPPLY,
	GPIB_COMMANDTYPE_PULSES,
	GPIB_LOOP_TRIGGER,
	GPIB_RECV_LOOP_TRIGGER
}; 


#define GPIB_COMMAND_FREQ_TRIGGER_GPIB 0
#define GPIB_COMMAND_FREQ_TRIGGER_EXT  1
#define GPIB_COMMAND_FREQ_TRIGGER_CALC 2

#define GPIB_COMMAND_PULSES_CHANNELS 8    

#define GPIB_NCOMMANDSTRINGS   4 

#define MAX_GPIBCOMMANDNAME_LEN 50
#define MAX_GPIB_ANALOGINPUTNAME_LEN 30

#define GPIB_COMMAND_WFM_TRANSFERMODE_STANDARD 			0
#define GPIB_COMMAND_WFM_TRANSFERMODE_INIT_AND_TRANSMIT 1
#define GPIB_COMMAND_WFM_TRANSFERMODE_TRANSMIT_ONLY 	2


#define GPIB_COMMANDNAME_NONE_STR "NONE"

#define N_FEEDBACK_CHANNELS 2

typedef struct {
	char name[MAX_GPIBCOMMANDNAME_LEN];
	int channel;

	// ========================================
	// 	  GPIB address of the device
	// ========================================
	int gpibAddress;
// ========================================
// 	  command type (see HARRWARE_GPIB.h)
// ========================================
	int commandType;
	void *device;
//	t_gpibDevice *device;


	int transmitBeforeEveryRepetition;
	int transmitBeforeFirstRepetition;
	int transmitAfterEveryRepetition;
	int transmitAfterLastRepetition;
	int transmitAfterStartOfSequence;
	int askDeviceForErrors;
	double maximumDuration_ms;

// ========================================
// 	  Definitions for possible external devices 
// ========================================
	int noGPIBdevice; //is true if the device is an external device instead of an GPIB device. 
	unsigned int extDeviceID; //ID is usually generated out of seconds since 1900.	
	unsigned int extDeviceChannel;
// ========================================
// 	  TYPE "waveform" 
// ========================================
	int waveform;
	
	double duration_us;        // duration in µs
	int nPoints;
	int nCopies;
	int offsetCorrection;    // correction (binary values)
	short *dataBuffer;
	int wfmTransferMode;
	int wfmAlwaysMaxPts;

	
	
// ========================================
// 	  TYPE "frequency" 
// ========================================
	int enableStep;
	double startFreq;
	double stopFreq;
	
	int enterSpan;
	int addFreqOffset;
	double freqOffsetMHz;
	int fetchFreq; 
	int divideFreq;
	int nStepRepetition;
	int stepTrigger;
	double startMultiply;
	double stopMultiply;
	double centerMultiply;
	double spanMultiply;
	int rfOn;
	double outputPower;
	int pulseModeEnable;

	//	int pulseModeLogic;
	int enableFM;
	double freqDeviation;
	double freqDeviationMultiply;

	int enableAM;
	double AMdepthPercent;
	
	int enableFeedback[N_FEEDBACK_CHANNELS];
	char feedbackAnalogInput[N_FEEDBACK_CHANNELS][MAX_GPIB_ANALOGINPUTNAME_LEN];
	double feedbackFreqDeviation[N_FEEDBACK_CHANNELS];
	double feedbackOffset[N_FEEDBACK_CHANNELS];
	
// ========================================
// 	  TYPE "command strings"
// ========================================
	int commandStringSend	   [GPIB_NCOMMANDSTRINGS];
	char *commandString  	   [GPIB_NCOMMANDSTRINGS];
	int commandStringReceive   [GPIB_NCOMMANDSTRINGS];
	int commandStringErrorCheck[GPIB_NCOMMANDSTRINGS];
	
// ========================================
// 	  TYPE "powerSupply"
// ========================================
	double voltage;
	double current;
	
	int stepVoltage;
	double startVoltage;
	double stopVoltage;
	int stepCurrent;
	double startCurrent;
	double stopCurrent;
	int waitUntilStabilized;
	
	
// ========================================
// 	  TYPE "pulses" 
// ========================================
	
	double startPulseDelay[GPIB_COMMAND_PULSES_CHANNELS];// in units of s
	double stopPulseDelay[GPIB_COMMAND_PULSES_CHANNELS];// in units of s
	int    stepPulseDelay[GPIB_COMMAND_PULSES_CHANNELS];
	double startPulseLength[GPIB_COMMAND_PULSES_CHANNELS];// in units of s 
	double stopPulseLength[GPIB_COMMAND_PULSES_CHANNELS]; // in units of s                    
	int    stepPulseLength[GPIB_COMMAND_PULSES_CHANNELS];
// =====================================
//	    used during runtime only
// =====================================		
	int menuID;
	TaskHandle feedbackInputTaskHandle[N_FEEDBACK_CHANNELS];
	double feedbackInputValue[N_FEEDBACK_CHANNELS];
	double feedbackFrequencyOffset[N_FEEDBACK_CHANNELS];
	char frequencyFeedbackDataPath[MAX_PATHNAME_LEN];
	double summedFeedbackFrequencyOffset;
} t_gpibCommand;



void GPIBCOMMAND_init (t_gpibCommand *item, int type);

void GPIBCOMMAND_free (t_gpibCommand *item);

t_gpibCommand *GPIBCOMMAND_new_at_front (t_sequence *seq);

t_gpibCommand *GPIBCOMMAND_new_at_second (t_sequence *seq);

t_gpibCommand *GPIBCOMMAND_new (t_sequence *seq);

void GPIBCOMMAND_delete (t_sequence *seq, int nr);

t_gpibCommand *GPIBCOMMAND_ptr (t_sequence *seq, int ID);

void GPIBCOMMAND_duplicate (t_gpibCommand *dest, t_gpibCommand *source);

int GPIBCOMMAND_nrFromMenuID (t_sequence *seq, int menu, int menuID);

char *GPIBCOMMAND_name (t_sequence *seq, int nr);

const char *GPIBCOMMAND_strMultiply (double multiply);


/**************************************************
 *   
 *  Data structure : GBIB devcive Info
 *
 **************************************************/
#define N_CHANNELS 2
#define MAX_GPIBDEVICENAME_LEN 50 


#define LOAD_OPEN -1
#define DEFAULT_LOAD LOAD_OPEN

#define GPIBDEV_NONE            0
#define GPIBDEV_UNKNOWN         -1

#define GPIBDEV_LECROY_LW120    -3


#define GPIB_PULSE_LOGIC_NORMAL 1
#define GPIB_PULSE_LOGIC_INVERTED 0

#define GPIB_PULSE_MODE_OFF 	  0
#define GPIB_PULSE_MODE_ON 		  1
#define GPIB_PULSE_MODE_NOT_USED -1

typedef int (*GPIB_transmitCommandFunctionPtr)(t_sequence *seq, t_gpibCommand *gpibCommand,
										       int flag, int transmitAll);

typedef int (*GPIB_commandFunctionPtr)(t_gpibCommand *gpibCommand);

typedef int (*GPIB_deviceFunctionPtr)(int gpibAddress);



typedef struct {
    char name[MAX_GPIBDEVICENAME_LEN];
    char strIDN[MAX_GPIBDEVICENAME_LEN];
    char strIDN2[MAX_GPIBDEVICENAME_LEN];
    int type;
    int saveInfo;
	int settingsRecalledFromDevice;

    int commandType1;
    int commandType2;
	int handle;
// ========================================
// 	  TYPE "waveform" 
// ========================================
    unsigned long wfmMinPoints;
    unsigned long wfmMaxPoints;
    int wfmPointsMultiple;
    double minDuration_us;
    double maxDuration_us;
    double minVoltage;
	double maxVoltage;
	int minDigital;
	int maxDigital;
	int nChannels;
	int load;
	
/*	int hasExternalSCLK;	   
	int externalSCLK;      // external 
	double frequencySCLK;  // Hz
	int divideFreq[N_CHANNELS];
*/
	int triggerSlope;
	int triggerSignal;
	
// -------------------------------------------------------
//       last values
// (used to avoid unnecessary data transfer to devices)
// -------------------------------------------------------
	double last_maxVoltage;
	int last_externalSCLK;
	double last_frequencySCLK;  // Hz
	int last_divideFreq[N_CHANNELS];
	int last_triggerSlope;
	int last_triggerSignal;
	int last_load;
	unsigned long last_numAOValues[N_CHANNELS];       // Anzahl Werte in 
                                                    // AO_Values[0..N_DAC_Channels], AO_Repeat[CH]
    long          *last_AOvalues[N_CHANNELS];        // 
    unsigned long *last_AOvalueDuration[N_CHANNELS]; // duration of each value
    int last_nCopies[N_CHANNELS];

// ========================================
// 	  TYPE "frequency" 
// ========================================
	double minFrequency;
	double maxFrequency;
	int enableExternalTrigger;
	int hasPulseOption;
	
//	int pulseModeEnable;
//	int pulseModeLogic;  // 1 = normal (TTL HIGH = ON), 0 = invers (TTL LOW = ON)
	double minAmplitude;
	double maxAmplitude;
	int maxDivide;
	
	
	int hasAM;
	double minAMdepthPercent;
	double maxAMdepthPercent;
// ========================================
// 	  TYPE "power supply" 
// ========================================
	double maxCurrent;  // max. current in Amps

	int last_divide;
	double last_freq;
	double last_power;
	int last_on;
	int last_pulseMode;

	int last_FM;
	double last_FMfreq_Dev;
	int last_AM;
	double last_AMdepthPercent;

	double last_voltage[N_CHANNELS];
	double last_current[N_CHANNELS];
	
	
	// functions (type
	GPIB_transmitCommandFunctionPtr GPIB_transmitFrequency;
	GPIB_transmitCommandFunctionPtr GPIB_transmitWaveform;
	GPIB_transmitCommandFunctionPtr GPIB_transmitPowerSupply;
	GPIB_commandFunctionPtr GPIB_getDeviceSettings;
	GPIB_commandFunctionPtr GPIB_waitUntilStabilized;
	GPIB_deviceFunctionPtr GPIB_getDeviceOptions;
	int errorMessageCompatibleWithAgilent;
	
	
	
} t_gpibDevice;



void GPIBDEVICE_resetLastValues (t_gpibDevice *item, int resetWaveforms);

void GPIBDEVICE_copySettings (t_gpibDevice *target, t_gpibDevice *source);

void GPIBDEVICE_init (t_gpibDevice *item);

int GPIBDEVICE_valuesChanged (t_gpibDevice *item, int i, int nCopies);

void GPIBDEVICE_free (t_gpibDevice *item);

t_gpibDevice *GPIBDEVICE_new (t_config *c);

void GPIBDEVICE_deleteAll (t_config *c);

t_gpibDevice *GPIBDEVICE_ptr (int address);

t_gpibDevice *GPIBDEVICE_ptrChannel (int channel);

t_gpibDevice *GPIBDEVICE_ptrConfig (t_config *c, int address);

int GPIBDEVICE_isType (t_gpibDevice *d, int type);

char *GPIBDEVICE_nameFromAddress (int address);





//unsigned long get_timeUnits (int timebase);


double getStepOutputVoltage (double stepFrom, double stepTo, 
						     int stepRep, int nRepetitions, int repetition);




void dTime (unsigned long time, double *dtime, int *i);


char *strTime (unsigned long time);

int nAOBoards (void);
int nDIOBoards (void);


t_digitizeParameters *digitizeParameters_PCI67XX (int timebase);

double DigitalToAnalog (long digital, t_digitizeParameters *p);
long AnalogToDigital (double analog, t_digitizeParameters *p);


#define MAX_SWEEPNAME_LEN 200
#define MAX_SWEEPDESCRIPTION_LEN 200

typedef struct {
	char channelName[MAX_SWEEPNAME_LEN];
	char description[MAX_SWEEPDESCRIPTION_LEN];
	char quantity[MAX_AXISNAME_LEN]; // e.g. time/ frequency etc
	char plotAxisName[MAX_AXISNAME_LEN];
	char units[MAX_UNIT_LEN];

	double from;
	double to;
	int dimension;
	double increment;
	
	double* listSweep;
	int numListSweepPoints;

	int incrementAfterRepetitions;
	int resetAfterRepetitions;
} t_parameterSweep;


void PARAMETERSWEEP_init (t_parameterSweep *p);

void PARAMETERSWEEP_free (t_parameterSweep *p);


void PARAMETERSWEEP_deleteAll (t_sequence *s);

void PARAMETERSWEEP_getAllFromSequence (t_sequence *s);

double PARAMETERSWEEPS_getValueForRepetition (t_parameterSweep *p, int repetition);





#define MAX_CCDDESCRIPTION_LEN 80

enum {
	CCD_CAMERATYPE_IXON,
	CCD_CAMERATYPE_IKON,
	
	N_CCD_CAMERATYPES
};


enum {
	CCD_FANMODE_HIGH,
	CCD_FANMODE_LOW,
	CCD_FANMODE_OFF,
	
	N_CCD_FANMODES
};


enum {
	CCD_IMAGEPROCESSING_NONE,
	CCD_IMAGEPROCESSING_DIVIDE,   // = 1
	CCD_IMAGEPROCESSING_SUBTRACT,
	CCD_IMAGEPROCESSING_EXPONATE,
	
//	CCD_IMAGEPROCESSING_SUM_ALL,
//	CCD_IMAGEPROCESSING_SUM_REP,
	CCD_IMAGEPROCESSING_SUBTRACT_INVERSE,  
	
	N_IMAGEPROCESSING_METHODS
};

typedef struct {
	int enableCCD;
	int serialNo;
	char description[MAX_CCDDESCRIPTION_LEN];
	int shutter;
	int cooler;
	int fanMode;
	int temperature;
	int acquisition;
	int preamplifierGain;
	int shiftSpeed;
	int readoutRate;
	int cameraType;
	int picturesPerRepetition;
	int exposureTime_ms;
	int enableEMCCD;
	int EMCCDgain;
	int takeSubimage;
	int subimageCenterX;
	int subimageCenterY;
	int subimageSizeX;
	int subimageSizeY;
	int binsize;
	int evaluateSubimage;
	int evaluateCenterX;
	int evaluateCenterY;
	int evaluateSizeX;
	int evaluateSizeY;
	int saveRawData;
	int imageProcessing;
	int separateFilePerRepetition;
	int pixelCount;
	int sumPictures;
	char dataPath[MAX_PATHNAME_LEN];
	int saveData;
} t_ccdSettings;


void CCDSETTINGS_init (t_ccdSettings *c);

void CCDSETTINGS_free (t_ccdSettings *c);

t_ccdSettings *CCDSETTINGS_new (t_sequence *seq);

t_ccdSettings *CCDSETTINGS_ptr (t_sequence *seq, int index);

void CCDSETTINGS_duplicate (t_ccdSettings *dest, t_ccdSettings *source);

														
int CCDSETTINGS_getFromSerialNo (t_sequence *seq, int serialNo);




#define MAX_VARIABLENAME_LEN 30
							   
typedef struct {
	char name[MAX_VARIABLENAME_LEN];
	double value;
} t_variable;

void VARIABLE_init (t_variable *v);


//int DMDIMAGE_writeCommandFile (t_sequence *seq, t_waveform *dmdWaveform, int repetition);
void DMDIMAGE_THREAD_writeCommandFile (t_sequence *seq, t_waveform *dmdWaveform, int repetition);

int DMDIMAGE_createBitmap (t_waveform *wfm, int repetition, int picNum);

//int CVICALLBACK DMDIMAGE_THREAD_displayShapeInCanvas(void* functionData);

t_ext_device *EXTDEV_new (t_sequence *seq);
void EXTDEV_init (t_ext_device *extdev, int deviceTyp);
int EXTDEV_nameExistsN (t_sequence *seq, const char *name, int startSearch, int reverseSearch);
int EXTDEV_checkForDuplicateNameAndRename (t_sequence *seq, char *name);
int EXTDEV_checkForDuplicateIdandChange (t_sequence *seq, unsigned int *id);
#endif // DATA_STRUCTURE





