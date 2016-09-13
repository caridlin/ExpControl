#ifndef DATA_LoadSave
#define DATA_LoadSave

#include "CONTROL_DataStructure.h"					
#include "inifile.h"

extern const char INI_TAG_CCDSETTINGS_enableCCD[],
	  	INI_TAG_CCDSETTINGS_serialNo[],
	  	INI_TAG_CCDSETTINGS_description[],
		INI_TAG_CCDSETTINGS_cameraType[],
		INI_TAG_CCDSETTINGS_shutter[],
		INI_TAG_CCDSETTINGS_cooler[],
		INI_TAG_CCDSETTINGS_fanMode[],
		INI_TAG_CCDSETTINGS_temperature[],
		INI_TAG_CCDSETTINGS_preamplifierGain[],
		INI_TAG_CCDSETTINGS_shiftSpeed[],
		INI_TAG_CCDSETTINGS_readoutRate[],
		INI_TAG_CCDSETTINGS_acquisition[],
		INI_TAG_CCDSETTINGS_picturesPerRepetition[],
		INI_TAG_CCDSETTINGS_exposureTime_ms[],
		INI_TAG_CCDSETTINGS_takeSubimage[],
		INI_TAG_CCDSETTINGS_enableEMCCD[],
		INI_TAG_CCDSETTINGS_EMCCDgain[],
		INI_TAG_CCDSETTINGS_subimageCenterX[],
		INI_TAG_CCDSETTINGS_subimageCenterY[],
		INI_TAG_CCDSETTINGS_subimageSizeX[],
		INI_TAG_CCDSETTINGS_subimageSizeY[],
		INI_TAG_CCDSETTINGS_binsize[],
		INI_TAG_CCDSETTINGS_evaluateSubimage[],
		INI_TAG_CCDSETTINGS_evaluateCenterX[],
		INI_TAG_CCDSETTINGS_evaluateCenterY[],
		INI_TAG_CCDSETTINGS_evaluateSizeX[],
		INI_TAG_CCDSETTINGS_evaluateSizeY[],
		INI_TAG_CCDSETTINGS_saveRawData[],
		INI_TAG_CCDSETTINGS_imageProcessing[],
		INI_TAG_CCDSETTINGS_dataPath[],
		INI_TAG_CCDSETTINGS_date[],
		INI_TAG_CCDSETTINGS_time[],
		INI_TAG_CCDSETTINGS_daytimeInSeconds[],
		INI_TAG_CCDSETTINGS_kineticSeriesLength[],
		INI_TAG_CCDSETTINGS_separateFilePerRepetition[],
		INI_TAG_CCDSETTINGS_nRepetitions[],
		INI_TAG_CCDSETTINGS_nCycles[],
		INI_TAG_CCDSETTINGS_saveData[],
		INI_TAG_CCDSETTINGS_pixelCount[],
		INI_TAG_CCDSETTINGS_sumPictures[],
		INI_TAG_CCDSETTINGS_fromIdleSequence[],
		INI_TAG_CCDSETTINGS_idleSequenceDataPath[],
		INI_TAG_CCDSETTINGS_seqNr[];

int WFM_pointlist_from_file(double** pointList,unsigned long* nPoints, const char* filename, double min, double max);
int WFM_loadFromFile (t_sequence *seq, t_waveform *new);

//int saveSequenceFile_OldPrgVersion (const char *filename, t_sequence *seq);


void Ini_GetTPoint (IniText ini, char *sectionName, t_point *p, int nr);
void Ini_PutTPoint (IniText ini, char *sectionName, t_point *p, int nr);


void Ini_GetTableConfig (IniText ini, t_tableConfig *c, int nr);
void Ini_PutTableConfig (IniText ini, t_tableConfig *c, int nr);


void Ini_PutCCDSettings (IniText ini, t_sequence *seq, int nr, int iniFileForCamera);


int Ini_PutSequence (IniText ini, t_sequence *seq);
int Ini_GetSequence (IniText ini, t_sequence *seq);


int Ini_writeFile (IniText ini, const char *filename);


void SEQUENCE_addVoltageZeroToFirstBlock (t_sequence *seq);


int SEQUENCE_load (const char *filename, t_sequence *seq);

int SEQUENCE_save (const char *filename, t_sequence *seq);

t_sequence* SEQUENCE_clone(t_sequence *seq);
t_sequence** SEQUENCE_clone_multi(t_sequence *seq, int num);

void eliminateIdenticalWaveforms (t_sequence *seq);






	#ifdef _CVI_DEBUG_
	   #define CONF_REG_SUBKEY "SOFTWARE\\ExperimentControlV2"
//	   #define CONF_REG_SUBKEY "SOFTWARE\\ExperimentControlV2_dbg"
	#else
	   #define CONF_REG_SUBKEY "SOFTWARE\\ExperimentControlV2"
	#endif

#define ROOT_KEY 1 // = HKEY_CURRENT_USER

#define CONF_REG_SUBKEY_EXE "SOFTWARE\\ExperimentControlV2"



void setLaunchState (int state);

int getLaunchState (void);







#endif


