#if !defined(WATCHDOGH)
#define WATCHDOGH

#include "inifile.h" 
#include "NIDAQmx.h"

#define STATUS_ALERT 1


#define WATCHDOG_PROGRAM_VERSION 1.03

#define DETECTIONMODE_FIXED 1
#define DETECTIONMODE_DIFFERENCE 2


#define FP_BUFFER_SIZE 256

#define MIN_NPOINTS 20
#define DEFAULT_NPOINTS 500
#define DEFAULT_NPREALERTPOINTS 5
//#define DATA_BUFFER_SIZE 500

#define DATA_SHIFT_BUFFER 100

//#define CHANNEL_MODE_WATCHDOG 1
//#define CHANNEL_MODE_MONITOR 2


#define MAX_MODULENAME_LEN 30
#define MAX_CHANNELNAME_LEN 50
#define MAX_UNITNAME_LEN 30


enum {
	MODULETYPE_USB,
	MODULETYPE_FIELDPOINT
} ;


typedef struct {
	int alarmState;
	int active;
	int watch;
	char name[MAX_CHANNELNAME_LEN];
//	int locked;
//	int alert;
//	int preAlert;
	int monitorChannel; 
		// channel independent of watchdog

	int detectionMode;
	double thresholdFixedUpper;
	double thresholdFixedLower;
	double thresholdDiff;
	int thresholdDiffAverages;
	int nPoints;
	int nPreAlertPoints;
	int nAverage;    // n running averages
	
	char channel[MAX_CHANNELNAME_LEN];
	int timeInterval;
//	int changeDetect;
	
// ===============================
//     channel mode: MONITOR
// ===============================
//	double lastMinutes;
	double displaySeconds;
	double saveMinutes;
	int changeUnits;
	double scaleVolts;
	double scaleToUnit;
	char scaleUnit[MAX_UNITNAME_LEN];
	int saveData;
	char saveFilename[MAX_PATHNAME_LEN];
	Rect window;

// ======================================
//       values during runtime 
// ======================================
	// -------------------------
	//     timestamps
	// -------------------------
	double firstTime;
	double lastTime;
	double lastValue;
	int valuesToGo;
	int preAlertValuesToDo;
	// -------------------------
	//       data display
	// -------------------------
	int panel;
	int updateGraph;

	// -------------------------------
	//    data
	// -------------------------------
	int nAveragedValues;
	double sum;
	__int64 nValuesRead;
	int dataBufferSize;
	int posPlotStart;

	double *dataBuffer;
	double *timeBuffer;
	int *statusBuffer;
	double *thresholdUp;
	double *thresholdDown;
	
	// -------------------------------
	//    communication with
	//    fieldpoint module
	// -------------------------------
	TaskHandle taskHandle;
	int status;
} t_channel;


void CHANNEL_init (t_channel *c);



#define MAX_RESOURCENAME_LEN 50
// ============================================
//      t_config
// ============================================
typedef struct {
	char filename[MAX_PATHNAME_LEN];
	char filenameIAK[MAX_PATHNAME_LEN];
	char dataSocketServer[MAX_PATHNAME_LEN];
	char FP_resourceName[MAX_RESOURCENAME_LEN];
	
	int beep;
	int beeperActive;
	int soundActive;
	char soundFilename[MAX_PATHNAME_LEN];

	char moduleBeeper  [MAX_MODULENAME_LEN];
	char chBeeper  [MAX_CHANNELNAME_LEN];
	
	int panelWidth, panelHeight;
	
	int nChannels;
	t_channel **channels;
	
} t_config;


//t_config *config (void);


void CONFIG_init (t_config *w);
int CONFIG_save (t_config *c);
int CONFIG_load (t_config *c);
void CONFIG_setDefault (t_config  *w);
void CONFIG_allocate (t_config *w, int nChannels);

void CONFIG_setValues (t_config *c);
void CONFIG_getValues (t_config *c);
void CONFIG_setValues_dimItems (void);
void CONFIG_displayChannelsInTree (t_config *c, int panel, int ctrl);


void CHANNEL_config_initPanel (void) ;
void CHANNEL_config_setValues (t_channel *ch, int nr);

int FP_setUnlocked (t_config *c);

int NIUSB_showError (const char *functionName, const char *module, const char *channel, int status);



int CHANNEL_open (t_config *w, t_channel *c);
void CHANNEL_updateGraph (t_channel *c);


/*
void Ini_GetWatchdogConfig (IniText ini, t_config *w);
void Ini_PutWatchdogConfig (IniText ini, t_config *w);
*/



void printState (void);
void setLED (int panel, int ctrl, int status);

int DS_setStatus (void);

void WATCHDOG_checkUnlockedState (t_config *c);
void WATCHDOG_updateDisplay (t_config *c);
void WATCHDOG_initAllPanels (t_config *w);
void WATCHDOG_discardAllPanels (t_config *w);
int WATCHDOG_connectToDataSocketServer (const char *serverName);


void ALERT_displayPanels (int status);


//void WATCHDOG_initPanel (t_config *w) ;

#endif


