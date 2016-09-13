#include "ExperimentControl.h"
#include <utility.h>
#include <userint.h>

#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"



extern int panelCCD;

#define MAX_CCDPanels 5
#define SEND_CAMERA_CONFIG_PAR_INDEX 72


int panelHandleCCD[MAX_CCDPanels] = {-1,-1};
int cameraAdded = 0;


const char *CCDSETTINGS_getImageProcessingTxt (int index)
{
	switch (index) {
		case CCD_IMAGEPROCESSING_NONE: return "none";
		case CCD_IMAGEPROCESSING_DIVIDE: return "bg substract + divide (3 pics per repetition)";
		case CCD_IMAGEPROCESSING_SUBTRACT: return "bg substract (even num pics per repetition)";
//		case CCD_IMAGEPROCESSING_SUM_REP: return "sum pictures of each repetition";
// case CCD_IMAGEPROCESSING_SUM_ALL: return "sum all pictures";
		case CCD_IMAGEPROCESSING_EXPONATE: return "exponate";
		case CCD_IMAGEPROCESSING_SUBTRACT_INVERSE: return "bg substract (even num pics per repetition, bg images reversed)";                
	}
	return "???";
}


const char *CCDSETTINGS_getCameraName (int index)
{
	switch (index) {
		case CCD_CAMERATYPE_IXON: return "Andor IXon";
		case CCD_CAMERATYPE_IKON: return "Andor IKon";
	}
	return "unkown";
}


const char *CCDSETTINGS_getFanModeTxt (int index)
{
	switch (index) {
		case CCD_FANMODE_HIGH: return "high";
		case CCD_FANMODE_LOW:  return "low";
		case CCD_FANMODE_OFF:  return "off";
	}
	return "???";
}


void CCDSETTINGS_fillNamesToListbox  (t_sequence *seq, int panel, int ctrl)
{
    int i;
	t_ccdSettings *c;
	char help[200];
	
	ClearListCtrl (panel, ctrl);
	for (i = 1; i <= ListNumItems (seq->lCCDSettings); i++) {
		ListGetItem (seq->lCCDSettings, &c, i);
		sprintf (help, "%d - %s", c->serialNo, c->description);
		InsertListItem (panel, ctrl, -1, help, i);
	}
}





void CCD_initPanel (void)
{
	int i, k;
	
	if (panelHandleCCD[0] <= 0) {
		panelHandleCCD[0] = LoadPanel (panelCCD, UIR_File, CCD);
		SetPanelPos (panelHandleCCD[0], 60, 5);
		DisplayPanel (panelHandleCCD[0]);
		for (i = 0; i < MAX_CCDPanels; i++) {
			if (i > 0) {
				panelHandleCCD[i] = DuplicatePanel (panelCCD, panelHandleCCD[0], "", 
					panelTop (panelHandleCCD[0]), panelRight (panelHandleCCD[i-1])+5);
				DisplayPanel (panelHandleCCD[i]);
			}
			
			ClearListCtrl (panelHandleCCD[i], CCD_RING_cameraType);
			for (k = 0; k < N_CCD_CAMERATYPES; k++) {
				InsertListItem (panelHandleCCD[i], CCD_RING_cameraType, -1, CCDSETTINGS_getCameraName(k), k);
			}

			ClearListCtrl (panelHandleCCD[i], CCD_RING_imageProcessing);
			for (k = 0; k < N_IMAGEPROCESSING_METHODS; k++) {
				InsertListItem (panelHandleCCD[i], CCD_RING_imageProcessing, -1, CCDSETTINGS_getImageProcessingTxt(k), k);
			
			}

		}
		
	}
}


void CCD_createRemoteFilename (char * buffer, const char *suffix, int serialNo)
{
	sprintf (buffer, "%s\\%d_remote%s", config->autoSaveSequencesPath, serialNo, suffix);
}



int CCD_writeCommandFile (t_sequence *seq, int index, int autoSaveSequences)
{
	t_ccdSettings *c;
	IniText ini;
	int err;
	char filename[MAX_PATHNAME_LEN];
	char tmpFilename[MAX_PATHNAME_LEN];
	char dataDir[MAX_PATHNAME_LEN];
	
	c = CCDSETTINGS_ptr (seq, index);
	if (c == NULL) return 0;
	
	
	c->saveData = autoSaveSequences;
	if (autoSaveSequences) {
		// TEST this
		err = mkDirs (seq->currentPath);
		sprintf (c->dataPath, "%s\\%03d", seq->currentPath, seq->startNr > 0 ? seq->startNr : 0);    
	}
	else {
		sprintf (dataDir, "%s\\RemoteCCD\\%d", config->autoSaveSequencesPath, c->serialNo);
		err = mkDirs (dataDir);
		sprintf (c->dataPath, "%s\\%03d", dataDir, seq->startNr > 0 ? seq->startNr : 0);    
	}
	if (err != 0) {
		PostMessagePopupf ("FileError", "Could not create path for CCD settings %s\n%s", c->dataPath, getErrorMkDir (err));
		return err;	
	}

	if ((ini = Ini_New (0)) == 0) return -1;
	Ini_PutCCDSettings (ini, seq, index, 1);
	
	CCD_createRemoteFilename (tmpFilename, ".temp", c->serialNo);
	changeSuffix (filename, tmpFilename, ".ini");

	tprintf ("Writing %s\n", filename);

	err = Ini_writeFile(ini, tmpFilename);
	if (FileExists (filename, 0)) {
		err = DeleteFile (filename);
		if (err != 0) tprintf ("FileError %d. %s.", err, getErrorMkDir (err));
	}
	err = RenameFile (tmpFilename, filename);
	if (err != 0) tprintf ("FileError %d. %s.", err, getErrorMkDir (err));
	
	Ini_Dispose (ini);
	c->dataPath[0] = 0;

    return err;
}	

	

int CCD_removeCommandFile (t_sequence *seq, int index)
{
	char filename[MAX_PATHNAME_LEN];
	t_ccdSettings *c;
	int err;
	
	c = CCDSETTINGS_ptr (seq, index);
	if (c == NULL) return 0;

	CCD_createRemoteFilename (filename, ".ini", c->serialNo);
	err = DeleteFile (filename);
	if (err != 0) PostMessagePopupf ("FileError", "Could not delete '%s'.\n%s", filename, getErrorMkDir (err));

    return err;
}	

	



void CCD_writeAllCommandFiles (t_sequence *seq, int autoSave)
{
	int i;
	t_ccdSettings *c;
	
	for (i = 1; i <= ListNumItems(seq->lCCDSettings); i++) {
		ListGetItem (seq->lCCDSettings, &c, i);
		if (c->enableCCD) CCD_writeCommandFile (seq, i, autoSave);
	}
}

	

	
	

	
void CCD_getNumberOfPicturesPerRepetition (t_ccdSettings *c, int *nPics, int *controlMode)
{
	int nPictures;
	int ctrlMode;
	
	switch (c->imageProcessing) {
		case CCD_IMAGEPROCESSING_DIVIDE:
			nPictures = 3;
			ctrlMode = VAL_INDICATOR;
			break;
		case CCD_IMAGEPROCESSING_SUBTRACT:
			// TODO testing
			nPictures = c->picturesPerRepetition;
			ctrlMode = VAL_HOT;
			//nPictures = 2;
			//ctrlMode = VAL_INDICATOR;
			break;
		case CCD_IMAGEPROCESSING_SUBTRACT_INVERSE:
			// TODO testing
			nPictures = c->picturesPerRepetition;
			ctrlMode = VAL_HOT;
			//nPictures = 2;
			//ctrlMode = VAL_INDICATOR;
			break;
		
		default:
			nPictures = c->picturesPerRepetition;
			ctrlMode = VAL_HOT;
	}
	
	if (nPics != NULL) *nPics = nPictures;
	if (controlMode != NULL) *controlMode = ctrlMode;
		
}

	



void CCD_displaySettings (t_sequence *seq, int index)
{
	t_ccdSettings *c;
	int p;
	int nPictures;
	int ctrlMode;
	
	if (index > (int)ListNumItems (seq->lCCDSettings)) return;
	
	ListGetItem (seq->lCCDSettings, &c, index);
	
	p = panelHandleCCD[index-1];
	SetCtrlVal (p, CCD_CHECKBOX_enable, c->enableCCD);
	SetAttributeForCtrls (p, ATTR_DIMMED, !(c->enableCCD && c->acquisition), 0,
				CCD_NUMERIC_nPictures,
				CCD_TEXTMSG1,
				CCD_NUMERIC_exposure,
				CCD_NUMERIC_preAmpGain,
				CCD_NUMERIC_shiftSpeed,
				CCD_CHECKBOX_subimage,
				CCD_RING_binning,
				CCD_CHECKBOX_evalSubimage,
				CCD_CHECKBOX_saveRaw,
				CCD_CHECKBOX_separateFile,
				CCD_RING_imageProcessing,
				CCD_CHECKBOX_pixelCount,
				0);
	SetAttributeForCtrls (p, ATTR_DIMMED, !c->enableCCD, 0,
				CCD_BINARYSWITCH_shutter,
				CCD_BINARYSWITCH_cooler,
				CCD_RINGSLIDE_fanMode,
				CCD_NUMERIC_temperature,
				CCD_CHECKBOX_acquisition,
				0);
	SetCtrlVal (p, CCD_NUMERIC_serialNo, c->serialNo);
	SetCtrlVal (p, CCD_STRING_description, c->description);

	SetCtrlVal (p, CCD_RING_cameraType, c->cameraType);
	SetCtrlVal (p, CCD_BINARYSWITCH_shutter, c->shutter);
	SetCtrlVal (p, CCD_BINARYSWITCH_cooler, c->cooler);
	SetCtrlVal (p, CCD_RINGSLIDE_fanMode, c->fanMode);
	SetCtrlVal (p, CCD_NUMERIC_temperature, c->temperature);
	SetCtrlVal (p, CCD_CHECKBOX_acquisition, c->acquisition);
	SetCtrlVal (p, CCD_NUMERIC_preAmpGain, c->preamplifierGain);
	SetCtrlVal (p, CCD_NUMERIC_shiftSpeed, c->shiftSpeed);
	SetCtrlVal (p, CCD_NUMERIC_readoutRate, c->readoutRate);

	
	SetAttributeForCtrls (p, ATTR_DIMMED, !c->enableCCD || (c->cameraType != CCD_CAMERATYPE_IXON), 0,
				CCD_CHECKBOX_enableEMCCD,
				CCD_NUMERIC_EMCCDgain,
				0);
	CCD_getNumberOfPicturesPerRepetition (c, &nPictures, &ctrlMode);
	
	SetCtrlVal (p, CCD_NUMERIC_nPictures, nPictures);
	SetCtrlAttribute (p, CCD_NUMERIC_nPictures, ATTR_CTRL_MODE, ctrlMode);
	SetCtrlAttribute (p, CCD_NUMERIC_nPictures, ATTR_TEXT_BGCOLOR,
					  ctrlMode == VAL_HOT ? VAL_WHITE : VAL_PANEL_GRAY);
		
	SetCtrlVal (p, CCD_NUMERIC_exposure, c->exposureTime_ms);
	SetCtrlVal (p, CCD_NUMERIC_EMCCDgain, c->EMCCDgain);
	SetCtrlVal (p, CCD_CHECKBOX_enableEMCCD, c->enableEMCCD);
	SetCtrlVal (p, CCD_CHECKBOX_subimage, c->takeSubimage);
	SetCtrlVal (p, CCD_NUMERIC_centerX, c->subimageCenterX);
	SetCtrlVal (p, CCD_NUMERIC_centerY, c->subimageCenterY);
	SetCtrlVal (p, CCD_NUMERIC_sizeX, c->subimageSizeX);
	SetCtrlVal (p, CCD_NUMERIC_sizeY, c->subimageSizeY);
	SetCtrlVal (p, CCD_RING_binning, c->binsize);
	SetCtrlVal (p, CCD_CHECKBOX_evalSubimage, c->evaluateSubimage);
	SetCtrlVal (p, CCD_NUMERIC_centerX_2, c->evaluateCenterX);
	SetCtrlVal (p, CCD_NUMERIC_centerY_2, c->evaluateCenterY);
	SetCtrlVal (p, CCD_NUMERIC_sizeX_2, c->evaluateSizeX);
	SetCtrlVal (p, CCD_NUMERIC_sizeY_2, c->evaluateSizeY);
	SetCtrlVal (p, CCD_CHECKBOX_saveRaw, c->saveRawData);
	SetCtrlVal (p, CCD_CHECKBOX_pixelCount, c->pixelCount);
	SetCtrlVal (p, CCD_RING_imageProcessing, c->imageProcessing);
	SetCtrlVal (p, CCD_CHECKBOX_sumPictures, c->sumPictures);

	
	SetCtrlVal (p, CCD_CHECKBOX_separateFile, c->separateFilePerRepetition);


	SetAttributeForCtrls (p, ATTR_DIMMED, !c->enableCCD || !c->takeSubimage, 0,
				CCD_NUMERIC_centerX,
				CCD_NUMERIC_centerY,
				CCD_NUMERIC_sizeX,
				CCD_NUMERIC_sizeY,
				0);
	SetAttributeForCtrls (p, ATTR_DIMMED, !c->enableCCD || !c->evaluateSubimage, 0,
				CCD_NUMERIC_centerX_2,
				CCD_NUMERIC_centerY_2,
				CCD_NUMERIC_sizeX_2,
				CCD_NUMERIC_sizeY_2,
				0);
}



int CCD_getIndexFromPanel (int panel)
{
	int i;
	
	for (i = 0; i < MAX_CCDPanels; i++) {
		if (panel == panelHandleCCD[i]) return i+1;
	}
	return 0;

}

void CCD_getSettings (t_sequence *seq, int panel, int *index)
{
	t_ccdSettings *c;
	int ctrlMode;

	*index = CCD_getIndexFromPanel (panel);
	if (*index > (int)ListNumItems (seq->lCCDSettings)) return;
	if (*index == 0) return;
	ListGetItem (seq->lCCDSettings, &c, *index);
	
	GetCtrlVal (panel, CCD_CHECKBOX_enable, &c->enableCCD);
	GetCtrlVal (panel, CCD_NUMERIC_serialNo, &c->serialNo);
	GetCtrlVal (panel, CCD_STRING_description, c->description);
	GetCtrlVal (panel, CCD_RING_cameraType, &c->cameraType);
	
	GetCtrlVal (panel, CCD_BINARYSWITCH_shutter, &c->shutter);
	GetCtrlVal (panel, CCD_BINARYSWITCH_cooler, &c->cooler);
	GetCtrlVal (panel, CCD_RINGSLIDE_fanMode, &c->fanMode);
	GetCtrlVal (panel, CCD_NUMERIC_temperature, &c->temperature);
	GetCtrlVal (panel, CCD_CHECKBOX_acquisition, &c->acquisition);
	
	GetCtrlVal (panel, CCD_NUMERIC_preAmpGain, &c->preamplifierGain);
	GetCtrlVal (panel, CCD_NUMERIC_shiftSpeed, &c->shiftSpeed);
	GetCtrlVal (panel, CCD_NUMERIC_readoutRate, &c->readoutRate);
	
	GetCtrlAttribute (panel, CCD_NUMERIC_nPictures, ATTR_CTRL_MODE, &ctrlMode);
	if (ctrlMode == VAL_HOT) GetCtrlVal (panel, CCD_NUMERIC_nPictures, &c->picturesPerRepetition);
	GetCtrlVal (panel, CCD_NUMERIC_exposure, &c->exposureTime_ms);
	GetCtrlVal (panel, CCD_NUMERIC_EMCCDgain, &c->EMCCDgain);
	GetCtrlVal (panel, CCD_CHECKBOX_enableEMCCD, &c->enableEMCCD);
	
	GetCtrlVal (panel, CCD_CHECKBOX_subimage, &c->takeSubimage);
	GetCtrlVal (panel, CCD_NUMERIC_centerX, &c->subimageCenterX);
	GetCtrlVal (panel, CCD_NUMERIC_centerY, &c->subimageCenterY);
	GetCtrlVal (panel, CCD_NUMERIC_sizeX, &c->subimageSizeX);
	GetCtrlVal (panel, CCD_NUMERIC_sizeY, &c->subimageSizeY);
	GetCtrlVal (panel, CCD_RING_binning, &c->binsize);
	GetCtrlVal (panel, CCD_CHECKBOX_evalSubimage, &c->evaluateSubimage);
	GetCtrlVal (panel, CCD_NUMERIC_centerX_2, &c->evaluateCenterX);
	GetCtrlVal (panel, CCD_NUMERIC_centerY_2, &c->evaluateCenterY);
	GetCtrlVal (panel, CCD_NUMERIC_sizeX_2, &c->evaluateSizeX);
	GetCtrlVal (panel, CCD_NUMERIC_sizeY_2, &c->evaluateSizeY);
	GetCtrlVal (panel, CCD_CHECKBOX_saveRaw, &c->saveRawData);
	GetCtrlVal (panel, CCD_CHECKBOX_pixelCount, &c->pixelCount);
	GetCtrlVal (panel, CCD_RING_imageProcessing, &c->imageProcessing);
	GetCtrlVal (panel, CCD_CHECKBOX_sumPictures, &c->sumPictures);
	GetCtrlVal (panel, CCD_CHECKBOX_separateFile, &c->separateFilePerRepetition);

}



void CCD_displayAll (t_sequence *seq)
{
	int i;
	
	for (i = 0; i < MAX_CCDPanels; i++) {
		SetPanelAttribute (panelHandleCCD[i], ATTR_VISIBLE, ListNumItems (seq->lCCDSettings) > i);
		CCD_displaySettings (seq, i+1);	
		if (ListNumItems (seq->lCCDSettings) > i) DisplayPanel (panelHandleCCD[i]) ;
			else HidePanel (panelHandleCCD[i]);
		
	}
	SetCtrlAttribute (panelCCD, CCDTAB_COMMANDBUTTON_add, ATTR_DIMMED, 
			ListNumItems (seq->lCCDSettings) >= MAX_CCDPanels);
	
		
}

int CVICALLBACK CCD_editParameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	int index;
	
	switch (event)
	{
		case EVENT_COMMIT:
			seq = activeSeq();
			CCD_getSettings (seq, panel, &index);
			CCD_displaySettings (seq, index);
			setChanges(seq, 1);   
			break;
	}
	return 0;
}



int CVICALLBACK CCD_addCamera_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	
	cameraAdded++;
	
	switch (event)
	{
		case EVENT_COMMIT:
			seq = activeSeq();
			if (ListNumItems (seq->lCCDSettings) < MAX_CCDPanels) {
				CCDSETTINGS_init (CCDSETTINGS_new (seq));
			}
			CCD_displayAll (seq);
			break;
	}
	return 0;
}

int CVICALLBACK CCD_removeCamera_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_ccdSettings *c;
	t_sequence *seq;
	int index;
	
	cameraAdded--;
	
	switch (event)
	{
		case EVENT_COMMIT:
			seq = activeSeq();
			index = CCD_getIndexFromPanel (panel);
			c = CCDSETTINGS_ptr (seq, index);
			if (c == NULL) return 0;
			if (!ConfirmPopupf ("Remove CCD settings", "Do you really want to remove the settings for\n" 
				"CCD camera %d (%s)?", c->serialNo, c->description)) return 0;
			ListRemoveItem (seq->lCCDSettings, 0, index);
			CCDSETTINGS_free (c);
			free (c);
			CCD_displayAll (seq); 
			break;
	}
	return 0;
}


int CVICALLBACK CCD_writeCommandFile_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	int index;

	switch (event)
	{
		case EVENT_COMMIT:
			seq = activeSeq();
			index = CCD_getIndexFromPanel (panel);
			CCD_writeCommandFile (seq, index, 0);
			break;
	}
	return 0;
}

int CVICALLBACK CCD_removeCommandFile_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CCD_removeCommandFile (activeSeq(), CCD_getIndexFromPanel (panel));
			break;
	}
	return 0;
}

void CCD_sendCommandFile (void)
{	
	//printf("cameraAdded %d", cameraAdded);
	if (cameraAdded == 0)
	{
		return;
	}
	// tell Drogon to expect a camera configuration file
	Set_Par_Err(SEND_CAMERA_CONFIG_PAR_INDEX, 1);

	// have to practically rewrite Ini_PutCCDSettings because we can't access INI file as a string...:(
	char configFile[2000];
	char *confIndex = configFile;
	int nPics;

	t_ccdSettings *c;
	ListGetItem (activeSeq()->lCCDSettings, &c, CCD_getIndexFromPanel(CCD));
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_serialNo, c->serialNo);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_cameraType, c->cameraType);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_shutter, c->shutter);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_cooler, c->cooler);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_fanMode, c->fanMode);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_temperature, c->temperature);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_acquisition, c->acquisition);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_preamplifierGain, c->preamplifierGain);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_shiftSpeed, c->shiftSpeed);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_readoutRate, c->readoutRate);
	CCD_getNumberOfPicturesPerRepetition (c, &nPics, 0);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_picturesPerRepetition, nPics);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_dataPath, c->dataPath);    
	if (activeSeq()->isOfTypeIdleSeq) {
		confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_fromIdleSequence, 1);
		confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_idleSequenceDataPath, config->idleSequenceDataPath);
		confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_seqNr, 0);
	} else {
		confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_seqNr, activeSeq()->startNr); 
	}
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_date, dateStr('.'));
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_time, timeStr());
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_daytimeInSeconds, daytimeInSeconds());
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_nRepetitions, c->separateFilePerRepetition ? 1 : activeSeq()->nRepetitions);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_nCycles, c->separateFilePerRepetition? activeSeq()->nRepetitions : 1);
	confIndex += sprintf(confIndex, "%s %d\n", INI_TAG_CCDSETTINGS_saveData, c->saveData);

	// send Drogon the camera configuration file we just generated
	send(ConnectSocket, configFile, 2000, 0);
}

int CVICALLBACK CCD_sendCommandFile_CB (int panel, int control, int event,
										void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CCD_sendCommandFile();
			break;
	}
	return 0;
}
