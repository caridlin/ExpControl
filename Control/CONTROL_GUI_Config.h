#ifndef GUI_Config
#define GUI_Config




//int panelTCP (void);

  


void CONFIG_resizePanel (void);
void CONFIG_initPanel (void);
int CONFIG_panelMinimumHeight(void);


void CONFIG_initPanelGeneralSettings (void);

void CONFIG_initPanelGpib (void);

void CONFIG_GENERAL_setValues (t_sequence *seq);

void CONFIG_displayGpibSettings (void);

void CONFIG_displayAllSettings (t_sequence *seq);


void CONFIG_SIGNALS_initPanel (void);
void CONFIG_SIGNALS_setParameters (t_sequence *seq);


int deviceNrDIO (int nr);
int deviceNrAO (int nr);



char *getBoardName (int deviceNo);

//void detectBoards (int outPanel, int outCtrl);

void addBoard (int typeCode, int deviceNo);

void setDefaultDir (const char *path);		

int CONFIG_write (t_config *c);

int CONFIG_read (t_config *c, int readSequences);

char *CONFIG_getPathSuffixToday (void);

void CONFIG_createTodaysDataPath (t_config *c);

void CONFIG_displayExternalDevices (void);


#define N_STARTNR_DIGITS 3
#define startSearchName "???_*"
#define startDirName    "%03d_%s"
#define START_ZEROES_STR "   "

int CONFIG_getCurrentStartNumber (void);


enum {
	CONFIG_TAB_INDEX_general,
	CONFIG_TAB_INDEX_boardPar,
	CONFIG_TAB_INDEX_gpib,
	CONFIG_TAB_INDEX_tcpip,
	CONFIG_TAB_INDEX_extdev
};






#endif
