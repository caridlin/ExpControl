// ==============================================
//    commands for remote control
// ==============================================


// ------------------------------------------
//    commands sent by TCP/DATASOCKET client
// ------------------------------------------
// "LOAD filaname.seq" load sequence
#define TCP_CMD_LOAD     "LOAD"
// "RUN (.seqfile as binaryData)"  : displays + starts 
#define TCP_CMD_RUN      "RUN"

// "SEQUENCE (.seqfile as binaryData)"  : display sequence
#define TCP_CMD_SEQUENCE "SEQUENCE"	   

#define TCP_CMD_GPIB     "GPIB"
// "STOP" : stop sequence
#define TCP_CMD_STOP     "STOP"
#define TCP_CMD_DIGITAL_LINES "DIGITAL_LINES"

// "F5" : start sequence with shortcut F5, etc..
#define TCP_CMD_F5 		 "F5"
#define TCP_CMD_F6 		 "F6"
#define TCP_CMD_F7 		 "F7"
#define TCP_CMD_F8 		 "F8"
#define TCP_CMD_F9 		 "F9"
// "CLOSE filename.seq" : close sequence 
#define TCP_CMD_CLOSE    "CLOSE"


// ------------------------------------------
//    commands sent by ExperimentControl.exe
// ------------------------------------------
// "STOPPED" : send by ex
#define TCP_CMD_STOPPED  "STOPPED"
#define TCP_CMD_DATA     "DATA"
#define TCP_CMD_FILE     "FILE"
//#define TCP_CMD_FINISHED "FINISHED"

#define TCP_CMD_READY    "READY"
#define TCP_CMD_QUIT    "QUIT"    

#define TCP_CMD_MI6021_PROGRAM "MI6021_PROGRAM"
#define TCP_CMD_MI6021_START   "MI6021_START"
#define TCP_CMD_MI6021_STOP    "MI6021_STOP"
#define TCP_CMD_MI6021_READY   "MI6021_READY"
#define TCP_CMD_MI6021_RESET   "MI6021_RESET"
#define TCP_CMD_MI6021_CONSTVOLTAGE "MI6021_CONSTVOLTAGE"


enum {
	TCP_CMD_ID_LOAD,
	TCP_CMD_ID_RUN,      
	TCP_CMD_ID_SEQUENCE,
	TCP_CMD_ID_GPIB,
	TCP_CMD_ID_STOP,
	TCP_CMD_ID_DIGITAL_LINES,
	
	TCP_CMD_ID_F5,
	TCP_CMD_ID_F6,
	TCP_CMD_ID_F7,
	TCP_CMD_ID_F8,
	TCP_CMD_ID_F9,
	
	TCP_CMD_ID_CLOSE, 
	TCP_CMD_ID_STOPPED,
	TCP_CMD_ID_DATA,
	
	TCP_CMD_ID_FILE,
	TCP_CMD_ID_READY,
	TCP_CMD_ID_QUIT,
	
	N_TCP_CMD_STRINGS
};


extern char *TCP_cmdStr[N_TCP_CMD_STRINGS];






// ===========================
//
//    datasocket variables
//
// ===========================
#define DS_EXPCONTROL_PATH         "%s/Experiment_Control/%s"
#define DS_VAR_COMMAND             "command_str"
#define DS_VAR_UPDATED             "command_updated"
#define DS_VAR_RECEIVED            "command_received"
#define DS_VAR_WAITING_FOR_TRIGGER "waiting_for_trigger"

#define DS_ATTRIBUTE_SIZE   "size"
#define DS_MAX_BUFFERSIZE 100000






// maximum number of counts of the 
// counters of PCI-
//#define GPCTR_MAX_COUNTS ((1<<24)-1)     // 2^24 - 1   


int TCP_cmdStrID (const char *cmdStr);
