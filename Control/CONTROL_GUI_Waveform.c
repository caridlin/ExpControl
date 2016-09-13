#include <windows.h> // for  Sleep 
#include <analysis.h>
#include <formatio.h>
#include <utility.h>
#include "toolbox.h"
#include "tools.h"   
#include <userint.h>
#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"
#include "imageProcessing.h"


enum {
	WFM_TAB_INDEX_points,
	WFM_TAB_INDEX_fromFile,
	WFM_TAB_INDEX_function,
	WFM_TAB_INDEX_step,
	WFM_TAB_INDEX_addr
};

enum {
	WFM_ADDR_TAB_INDEX_singleSite,
	WFM_ADDR_TAB_INDEX_trajectories,
	WFM_ADDR_TAB_INDEX_DMD
};


enum {
	WFM_FUNC_TABLECOL_NAME = 1,
	WFM_FUNC_TABLECOL_VALUE_FROM,
	WFM_FUNC_TABLECOL_UNIT1,
	WFM_FUNC_TABLECOL_VARY,
	WFM_FUNC_TABLECOL_VALUE_TO,
	WFM_FUNC_TABLECOL_UNIT2
};


#define WFM_TABLE_ROW_HEIGHT 20  // was 30


int EDITWFM_oldTime = -1;  

extern int panelWaveforms;
extern int panelMain;
extern int panelMain2;
int subPanelWFMPoints = -1;
int subPanelWFMFile   = -1;
int subPanelWFMStep   = -1;
int subPanelWFMFunc   = -1;  
int subPanelWFMAddr   = -1;  
int subPanelWFMAddrSingleSite   = -1;  
int subPanelWFMAddrTraj     = -1;  
int subPanelWFMAddrDMD = -1;
int menuWfmAddrTable = -1;

int panelWfmType = 0;
int menuWfmTable  = 0;
int panelAddressing = 0;


int WFM_activeTableRow = 2;
int WFM_active = 0;



//
//  addressing phase feedback
//
t_image *img = NULL;
t_image *imgPos = NULL;
t_waveform *imgWfm = NULL;
t_waveform *imgWfmPos = NULL;


const double WFM_ADDR_calibrationParametersDefault[WFM_ADDR_N_CALIBRATION_PARAMETERS] = {
	-49.961, -71.707, 481.14, 553.95, 5.2, 51.524, -71.303, -26.832, 579.4, 4.8,
	-44.15, 45.55, 4.26667, 500, 10000, 1.5, 3.5, 0, 0, 0, 0, 1, 1, 0, 0.06, 
	7.64, 7.64,
	7.64, 7.64,
	7.65, 7.65,
	0,
	0,90,
	262.0,247.0,
};

const double WFM_ADDR_calibrationParametersMin[WFM_ADDR_N_CALIBRATION_PARAMETERS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	-90, -90, 3, 0, 0, 1, 1,
	0, 0, -5, -5, 0, 0,
	0, 0, 
	1, 1,
	-10, -10,
	1, 1,
	0,
	-90,-90,
	0,0,
};


const double WFM_ADDR_calibrationParametersMax[WFM_ADDR_N_CALIBRATION_PARAMETERS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	90, 90, 5, 1000, 10000, 10, 10,
	0, 0, 5, 5, 1, 1,
	1, 1, 
	20, 20,
	20, 20,
	20, 20,
	1,
	90,90,
	1023,1023,
};



const char *WFM_ADDR_calibrationParameterNames[WFM_ADDR_N_CALIBRATION_PARAMETERS] = {
		"a1x", "a1y", "b1x", "b1y", "U2_0", "a2x", "a2y", "b2x", "b2y", "U1_0",
		"theta1", "theta2", "lambda", "minThresh", "maxThresh", "minSigma", "maxSigma",
		"offset U1", "offset U2", "offset phi1", "offset phi2", "enable cal1", "enable cal2",
		"feedback OFF", "phaseErr threshold" ,
		"DMD px per site x", "DMD px per site y",
		"DMD px per site x feedback", "DMD px per site y feedback",
		"DMD px per site x box", "DMD px per site y box", 
		"DMD flip phi1 <-> phi2",
		"DMD latt x angle","DMD latt y angle",
		"DMD img center x","DMD img center y"
};



const double WFM_ADDR_trajParametersDefault[WFM_ADDR_N_TRAJ_PARAMETERS] = {
	100, 100, 120, 100, 5	
};

const char *WFM_ADDR_trajParameterNames[WFM_ADDR_N_TRAJ_PARAMETERS] = {
	"duration(ms)", "#Points", "r_start", "r_end", "revolutions"
};


//int plotHandle = 0;

#define POINTS_TABLE_ROW_HEIGHT 25

#define VAL_WAVEFORM_PLOT_COLOR VAL_RED

t_controls WFM_controls;
//static t_controls WFM_controlsTransferFunct;

// Table
enum {
	WFM_TABLE_COL_id = 1,
	WFM_TABLE_COL_name,
	WFM_TABLE_COL_channel,
	WFM_TABLE_COL_usedBy,
	WFM_TABLE_COL_type,
	WFM_TABLE_COL_graph,
	WFM_TABLE_COL_nPoints,
	

	WFM_TABLE_NCols
};




#define INDEX_WFM_CHANNEL_ALL   0
#define INDEX_WFM_CHANNEL_NONE  1
#define INDEX_WFM_CHANNEL_GPIB  2




/************************************************************************/
/*
/*    general functions 
/*
/************************************************************************/



int WFM_nrFromTableRow (int row)
{
	t_sequence *seq;
	t_waveform *w;
	int i;
	int n;
	
	seq = activeSeq();
	n = ListNumItems (seq->lWaveforms);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &w, i);
		if (w->positionInTable == row) return i;
	}
	return 0;
	
//	GetTableCellVal (panelWaveforms, WFM_TABLE_wfm,
//					 MakePoint (WFM_TABLE_COL_id, row), &wfmNr);
//	return w;
}



//=======================================================================
//
//    returns the active waveform no
//
//=======================================================================
int activeWfmNo (void) 
{
	return WFM_nrFromTableRow (WFM_activeTableRow);
}


//=======================================================================
//
//    returns the active waveform
//
//=======================================================================
t_waveform *activeWfm () 
{
    if (ListNumItems (activeSeq()->lWaveforms) == 0) return NULL;
    return WFM_ptr (activeSeq(), activeWfmNo());
    
}



//=======================================================================
//
//    changes the visibility of controls 
//    depending on the waveform type
//
//=======================================================================
void WFM_changeVisibility (int wfmType)
{
    int activeTab;
//    int wfmType;
    
   	// -----------------------------------------
	//    dimmable items (if no waveform in memory)
	// -----------------------------------------
    
//    if (wfm == NULL) wfmType = WFM_TYPE_NONE;
//    else wfmType = wfm->type;
    
    SetCtrlAttribute (panelWaveforms, WFM_RING_add, ATTR_VISIBLE, wfmType == WFM_TYPE_POINTS);
    
	activeTab = subPanelWFMPoints;
	switch (wfmType) {
	    case WFM_TYPE_POINTS: activeTab = WFM_TAB_INDEX_points; break;
	    case WFM_TYPE_FILE:   activeTab = WFM_TAB_INDEX_fromFile;   break;
	    case WFM_TYPE_STEP:   activeTab = WFM_TAB_INDEX_step;   break;
		case WFM_TYPE_FUNCTION: activeTab = WFM_TAB_INDEX_function; break;
		case WFM_TYPE_ADDRESSING: activeTab = WFM_TAB_INDEX_addr; break;
		default: activeTab = WFM_TAB_INDEX_points; break;
	}
	
	// change visibility 
	SetTabPageAttribute (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_points,
						 ATTR_DIMMED , (wfmType != WFM_TYPE_POINTS));
	
	SetTabPageAttribute (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_function,
						 ATTR_DIMMED , wfmType != WFM_TYPE_FUNCTION);
	
	SetTabPageAttribute (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_fromFile,
						 ATTR_DIMMED , wfmType != WFM_TYPE_FILE);
	
	SetTabPageAttribute (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_step,
						 ATTR_DIMMED , wfmType != WFM_TYPE_STEP);
	
	SetTabPageAttribute (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_addr,
						 ATTR_DIMMED , wfmType != WFM_TYPE_ADDRESSING);

	SetActiveTabPage (panelWaveforms, WFM_TAB0, activeTab);


    // -----------------------------------------
	//   dimm controls if no waveform present
	// -----------------------------------------
	SetAttributeForCtrls (panelWaveforms, ATTR_DIMMED, wfmType == WFM_TYPE_NONE, 0, 
		WFM_STRING_WFM_Name,    
		WFM_BTN_DeleteWaveform, 
		WFM_RING_add, 
		0);
	
}




void WFM_displayName (int panel, int control, int wfmNr)
{
    int color;
    
    if (wfmNr == WFM_ID_UNCHANGED) color = VAL_BLACK; 
        else color = VAL_RED;
    SetCtrlAttribute (panel, control, ATTR_TEXT_COLOR,
                      color);
    SetCtrlVal (panel, control, wfmNr);
}


/************************************************************************/
/*
/*    initializaiton of panels etc. 
/*
/************************************************************************/


//=======================================================================
//
//    resize panel
//
//=======================================================================
void WFM_resizePanel (void)
{
//	SetCtrlAttribute (panelWaveforms, WFM_LISTBOX_Waveforms, ATTR_HEIGHT,
//		 ctrlHeight (panelWaveforms, WFM_TAB0) 
//		 - ctrlTop (panelWaveforms, WFM_LISTBOX_Waveforms));  
	SetCtrlAttribute (panelWaveforms, WFM_TABLE_wfm, ATTR_LEFT, 0);
	SetCtrlAttribute (panelWaveforms, WFM_TABLE_wfm, ATTR_WIDTH, 
					max(0,ctrlLeft (panelWaveforms, WFM_TAB0)-10) );		
	SetCtrlAttribute (panelWaveforms, WFM_TABLE_wfm, ATTR_HEIGHT, 
					max(0,panelHeight (panelWaveforms) - ctrlTop (panelWaveforms, WFM_TABLE_wfm)) );		

}


//=======================================================================
//
//     determine minimum height of panel 
//
//=======================================================================
int WFM_panelMinimumHeight(void)
{
	return 300;
//	return ctrlBottom(panelWaveforms, WFM_BTN_Digitize) +10;
}


void WFM_setTransferFunction (t_transferFunction *f)
{
	
	int hasChanged = 0;
	
	if (WFM_controls.f != NULL) {
		TRANSFERFUNCT_free (WFM_controls.f);
		free (WFM_controls.f);
		WFM_controls.f = NULL;
		hasChanged = 1;
	}
	if (f == NULL) {
		POINTS_setParameterValues (&WFM_controls, TIME_MULTIPLY_ms,
									"voltage", 
									"voltage increment",
		 							-10.0, 10.0, 4, 1, 0);
		 if (hasChanged) POINTS_createTableColumns (&WFM_controls);
	}
	else {
		WFM_controls.f = TRANSFERFUNCT_copy (f);
		POINTS_setParameterValues (&WFM_controls, TIME_MULTIPLY_ms,
									f->yAxis, 
									"increment",
	 							    -100000.0, 100000.0, 3, 1, 1);      
	 	POINTS_createTableColumns (&WFM_controls);
	}
}


// channelName has to be at least of length MAX_PATHNAME_LEN
void WFM_TABLE_channelName (int wfmChannel, char* channelName)
{
	//char* tmp;
	char* name;
	t_sequence *seq;
	
	//tmp = calloc(500,sizeof(char));
		
	seq = activeSeq();
	
	switch (wfmChannel) {
		case WFM_CHANNEL_NONE:
			strcpy(channelName,"none");
			return;
		case WFM_CHANNEL_ALL:
			strcpy(channelName,"all");
			return;
		case WFM_CHANNEL_GPIB:
			strcpy(channelName,"GPIB");
			return;
		default:
			if (seq == NULL) {
				strcpy(channelName,str_AnalogChannelNames (wfmChannel)); // should never happen
				return;
			} else {	
				name = SEQUENCE_DACchannelName (seq,wfmChannel);
				if (name != NULL) {
					snprintf(channelName,MAX_PATHNAME_LEN,"%-6s - %s",str_AnalogChannelNames (wfmChannel),name);
					return;
				} else {
					strncpy(channelName,str_AnalogChannelNames (wfmChannel),MAX_PATHNAME_LEN);	
					return;
				}
				
			}
	}
}


int WFM_TABLE_channel_color(int channel) {
	if (channel >= 0) {
		return colors[channel % NColors];
	} else {
		return VAL_WHITE;	
	}
}


void WFM_TABLE_displayWfm (t_waveform *wfm, int panel, int control)
{
	Point cell;
	int textColor;
	int bgColor;
	char channelName[MAX_PATHNAME_LEN];
	
	if (wfm == NULL) return;
	
	if (wfm == activeWfm()) {
		bgColor = VAL_BLUE;
		textColor = VAL_WHITE;
	}
	else {
		bgColor = VAL_WHITE;
		textColor = VAL_BLACK;
	}
	
	cell = MakePoint (WFM_TABLE_COL_id, wfm->positionInTable);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, wfm->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);

	
	cell = MakePoint (WFM_TABLE_COL_name, wfm->positionInTable);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, wfm->name);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);

	cell = MakePoint (WFM_TABLE_COL_channel, wfm->positionInTable);
	
	WFM_TABLE_channelName (wfm->channel,channelName);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, channelName);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   VAL_LT_GRAY);// VAL_DK_GRAY2
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   WFM_TABLE_channel_color(wfm->channel));
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BOLD, TRUE); 

	cell = MakePoint (WFM_TABLE_COL_usedBy, wfm->positionInTable);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, wfm->usedBy);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);
	SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
								 VAL_CENTER_LEFT_JUSTIFIED);
	
	
	cell = MakePoint (WFM_TABLE_COL_type, wfm->positionInTable);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, WFM_typeStr (wfm));
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);

	cell = MakePoint (WFM_TABLE_COL_nPoints, wfm->positionInTable);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, intToStr (wfm->DIG_NValues));
	SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
						   VAL_CENTER_RIGHT_JUSTIFIED);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);
	
}

/*
void WFM_TABLE_initRingChannelNames (int panel, int control, Point cell)
{
	int i;
	
	DeleteTableCellRingItems (panel, control, cell, 0, -1);
	InsertTableCellRingItem (panel, control, cell, INDEX_WFM_CHANNEL_ALL , WFM_TABLE_channelName (WFM_CHANNEL_ALL));		   // index 0
	InsertTableCellRingItem (panel, control, cell, INDEX_WFM_CHANNEL_NONE, WFM_TABLE_channelName (WFM_CHANNEL_NONE));		   // index 1
	InsertTableCellRingItem (panel, control, cell, INDEX_WFM_CHANNEL_GPIB, WFM_TABLE_channelName (WFM_CHANNEL_GPIB));		   // index 2			

	for (i = 0; i < N_DAC_CHANNELS; i++) {
		InsertTableCellRingItem (panel, control, cell, -1, WFM_TABLE_channelName (i));
	}
}   */



void WFM_TABLE_initAllRows (int n, int panel, int control)
{
	int nRows;
	int i;
	Point cell;
	
	GetNumTableRows (panel, control, &nRows);
	if (n < nRows) DeleteTableRows (panel, control, n+1, -1);
	else if (n > nRows) InsertTableRows (panel, control, nRows+1, n-nRows, VAL_USE_MASTER_CELL_TYPE);
	for (i = nRows; i < n; i++) { 
		SetTableRowAttribute (panel, control, i+1, ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
		SetTableRowAttribute (panel, control, i+1, ATTR_ROW_HEIGHT, WFM_TABLE_ROW_HEIGHT);

		cell = MakePoint (WFM_TABLE_COL_id, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
		SetTableCellAttribute (panel, control, cell, ATTR_DATA_TYPE,
							   VAL_INTEGER);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_CENTER_JUSTIFIED);
		
		
 		cell = MakePoint (WFM_TABLE_COL_name, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_LEFT_JUSTIFIED);

		
		cell = MakePoint (WFM_TABLE_COL_channel, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
		//WFM_TABLE_initRingChannelNames (panel, control, cell);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_LEFT_JUSTIFIED);
		

		cell = MakePoint (WFM_TABLE_COL_type, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_CENTER_JUSTIFIED);
	
	}	

}


void WFM_TABLE_setActiveRow (int row)
{
	WFM_activeTableRow = row;
};





int WFM_TABLE_sortMode (int wfmSortMode, int setFlag)
{
	static int sortMode = WFM_TABLE_SORT_MODE_CHANNEL;
	
	if (setFlag) sortMode = wfmSortMode;	
	return sortMode;
}


int CVICALLBACK WFM_TABLE_compareFunction (void *wfm1, void *wfm2)
{
	t_waveform *w1, *w2;
	int result = 0;
	char channelName1[MAX_PATHNAME_LEN];
	char channelName2[MAX_PATHNAME_LEN];
	
	w1 = * ((t_waveform **) (wfm1));
	w2 = * ((t_waveform **) (wfm2));
	
	switch (WFM_TABLE_sortMode (0, 0)) {
		case WFM_TABLE_SORT_MODE_CHANNEL:
			WFM_TABLE_channelName (w1->channel,channelName1);
			WFM_TABLE_channelName (w2->channel,channelName2);
			if ((w1->channel >= 0) && (w2->channel >= 0)) {
				if (w1->channel < w2->channel) return -1;
				if (w1->channel > w2->channel) return 1;
			}
			else result = CompareStrings (channelName1, 0, channelName2, 0, 0);
			if (result == 0) result = CompareStrings (w1->name, 0, w2->name, 0, 0);
			break;
		case WFM_TABLE_SORT_MODE_NAME:
			return CompareStrings (w1->name, 0, w2->name, 0, 0);
		case WFM_TABLE_SORT_MODE_NR:
			if (w1->positionInList < w2->positionInList) return -1;
			if (w1->positionInList > w2->positionInList) return 1;
			return 0;			
		case WFM_TABLE_SORT_MODE_NPOINTS:
			if (w1->DIG_NValues < w2->DIG_NValues) return -1;
			if (w1->DIG_NValues > w2->DIG_NValues) return 1;
			return 0;			
		case WFM_TABLE_SORT_MODE_TYPE:
			if (w1->type < w2->type) return -1;
			if (w1->type > w2->type) return 1;
			return 0;			
		case WFM_TABLE_SORT_MODE_USEDBY:
			return CompareStrings (w1->usedBy, 0, w2->usedBy, 0, 0);
	}
	return result;
}



void WFM_TABLE_sort (t_sequence *seq)
{
    ListType lSortedWaveforms;		 
	int i, n;
	t_waveform *wfm;
	
//	lSortedWaveforms = ListCreate (sizeof(t_waveform *));
	WFM_addPositionInList (seq);
	lSortedWaveforms = ListCopy (seq->lWaveforms);
	WFM_TABLE_sortMode (seq->wfmTableSortMode, 1);
	ListQuickSort (lSortedWaveforms, WFM_TABLE_compareFunction);
	n = ListNumItems (lSortedWaveforms);
	for (i = 1; i <= n; i++) {
		ListGetItem (lSortedWaveforms, &wfm, i);
		if (seq->wfmTableSortDescending) 
			wfm->positionInTable = n - i + 1;
		else 
			wfm->positionInTable = i;
	}
	ListDispose (lSortedWaveforms);
}





void WFM_TABLE_displayAllWfms (int panel, int control, t_sequence *seq, int activeNr, int sort)
{
	int n, i;
	t_waveform *wfm;
//	Rect selection;
//	int err;

	WFM_fillInfos (seq);

	n = ListNumItems (seq->lWaveforms);
	
	WFM_TABLE_initAllRows (n, panel, control);
	if (sort) WFM_TABLE_sort (seq);

	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &wfm, i);
//		wfm->positionInTable = i;
//		wfm->positionInList = i;
		WFM_TABLE_displayWfm (wfm, panel, control);
	}
	
/*
	selection.top = 3;
	selection.left = 0;
	selection.width = 6;
	selection.height = 2;
	err = SetTableSelection (panel, control, selection);
*/	
	ProcessDrawEvents();
	CallCtrlCallback (panel, control, EVENT_TABLE_ROW_COL_LABEL_CLICK, 3, 0, 0);
}


void WFM_TABLE_init (int panel, int control)
{
	int col;
	int type;
	int width = 0;
	char *name = "";
	
	DeleteTableColumns (panel, control, 1, -1);

	SetCtrlAttribute (panel, control, ATTR_AUTO_EDIT, 1);
	SetCtrlAttribute (panel, control, ATTR_COLUMN_LABELS_VISIBLE, 1);
	SetCtrlAttribute (panel, control, ATTR_ROW_LABELS_VISIBLE, 0);
	SetCtrlAttribute (panel, control, ATTR_TABLE_MODE, VAL_GRID);
	SetCtrlAttribute (panel, control, ATTR_ENABLE_ROW_SIZING, 1);
	SetCtrlAttribute (panel, control, ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute (panel, control, ATTR_TABLE_BGCOLOR, VAL_PANEL_GRAY);
	SetCtrlAttribute (panel, control, ATTR_TEXT_POINT_SIZE, 13);
	
//	SetCtrlAttribute (panel, control, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
//	SetCtrlAttribute (panel, control, ATTR_VERTICAL_GRID_VISIBLE, 1);

	SetCtrlAttribute (panel, control, ATTR_ENABLE_POPUP_MENU, 0);
	
	for (col = 1; col <  WFM_TABLE_NCols; col++) {
		switch (col) {
			case WFM_TABLE_COL_id:
				type = VAL_CELL_NUMERIC;
				name = "ID";
				width = 20;
				break;
			case WFM_TABLE_COL_name:
				type = VAL_CELL_STRING;
				name = "Name";
				width = 250;
				break;
			case WFM_TABLE_COL_channel:
				type = VAL_CELL_STRING;
				name = "Channel";
				width = 50;
				break;
			case WFM_TABLE_COL_usedBy:
				type = VAL_CELL_STRING;
				name = "used by";
				width = 50;
				break;
			case WFM_TABLE_COL_type:
				type = VAL_CELL_RING;
				name = "type";
				width = 70;
				break;
			case WFM_TABLE_COL_graph:
				type = VAL_CELL_PICTURE;
				name = "";
				width = 80;
				break;
			case WFM_TABLE_COL_nPoints:
				type = VAL_CELL_NUMERIC;
				name = "nPoints";
				width = 80;
				break;

		}
		InsertTableColumns (panel, control, -1, 1, VAL_USE_MASTER_CELL_TYPE);
//		SetTableColumnAttribute (panel, control, col, ATTR_CELL_TYPE, type);
		SetTableColumnAttribute (panel, control, col, ATTR_COLUMN_WIDTH, width);
		SetTableColumnAttribute (panel, control, col, ATTR_USE_LABEL_TEXT, 1);
		SetTableColumnAttribute (panel, control, col, ATTR_LABEL_TEXT, name);
		SetTableColumnAttribute (panel, control, col, ATTR_CELL_JUSTIFY,
								 VAL_CENTER_LEFT_JUSTIFIED);

	}
}


void WFM_ADDR_tableToMatrix (int panel, int ctrl, t_waveform *w, int matrixNum)
{
	int i, j;
	int value;

	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		for (j = 0; j < WFM_ADDR_MATRIXDIM; j++) {
			TABLE_getCellValueYesNo2 (panel, ctrl, MakePoint (i+1, j+1), &value);
			WFM_ADDR_setMatrixValue (w, matrixNum, i, j, value );
			
		}
	}
}


void WFM_ADDR_matrixToTable (int panel, int ctrl, t_waveform *w, int matrixNum)
{
	int i, j;
	int value;
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		for (j = 0; j < WFM_ADDR_MATRIXDIM; j++) {
			value = WFM_ADDR_getMatrixValue (w, matrixNum, i, j);
			TABLE_setCellValueYesNo2 (panel, ctrl, MakePoint (i+1, j+1), value);
		}
	}
}

void WFM_ADDR_showDisorderPara(int panel,t_waveform *wfm, int matrixNum)
{
	SetCtrlVal(panel, ADDR_SS_CHECKBOX_disorder, wfm->addDisorder[matrixNum]);
	SetCtrlVal(panel, ADDR_SS_NUMERIC_disorder, wfm->disorderSeed[matrixNum]);
	SetCtrlVal(panel, ADDR_SS_RING_disorder, wfm->disorderTyp[matrixNum]);
}

int WFM_ADDR_allowStepParameter (int parameterID)
{
	switch (parameterID) {
		case WFM_ADDR_CAL_offsetPhi1:
		case WFM_ADDR_CAL_offsetPhi2:
		case WFM_ADDR_CAL_DMDpxPerSiteX:
		case WFM_ADDR_CAL_DMDpxPerSiteY:
		case WFM_ADDR_CAL_DMDpxPerSiteXbox:
		case WFM_ADDR_CAL_DMDpxPerSiteYbox:
			return 1;
	}
	return 0;
}




void WFM_ADDR_initTable (int panel, int ctrl)
{
	int colSize = 13;
	int rowSize = 12;
	int i;
	int centerX, centerY;
	
	DeleteTableRows (panel, ctrl, 1, -1);
	DeleteTableColumns (panel, ctrl, 1, -1);
	SetTableColumnAttribute (panel, ctrl, -1, ATTR_COLUMN_WIDTH, colSize);
	InsertTableColumns (panel, ctrl, -1, WFM_ADDR_MATRIXDIM, VAL_CELL_STRING);
	SetTableColumnAttribute (panel, ctrl, -1, ATTR_COLUMN_WIDTH, colSize);
	
	InsertTableRows (panel, ctrl, -1, WFM_ADDR_MATRIXDIM, VAL_CELL_STRING);
	SetTableRowAttribute (panel, ctrl, -1, ATTR_SIZE_MODE,
						  VAL_USE_EXPLICIT_SIZE);
	SetTableRowAttribute (panel, ctrl, -1, ATTR_ROW_HEIGHT, rowSize);
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		TABLE_setColumnAttributesYesNo (panel, ctrl, i+1);
		SetTableCellRangeAttribute (panel, ctrl, VAL_TABLE_COLUMN_RANGE(i+1), ATTR_TEXT_POINT_SIZE, 12);
		SetTableRowAttribute (panel, ctrl, i+1, ATTR_LABEL_POINT_SIZE, 8);
	}
	centerX = (WFM_ADDR_MATRIXDIM-1) / 2;
	centerY = (WFM_ADDR_MATRIXDIM-1) / 2;
	SetTableCellAttribute (panel, ctrl, MakePoint (centerX+1, centerY+1),
						   ATTR_TEXT_BGCOLOR, VAL_RED);
	SetCtrlAttribute (panel, ctrl, ATTR_ENABLE_ROW_SIZING, 0);
	SetCtrlAttribute (panel, ctrl, ATTR_ENABLE_COLUMN_SIZING, 0);

}


void WFM_ADDR_showCalibration (int panel, int ctrl, t_waveform *wfm)
{
	int i;
	#define bufLen 1000
	char buf[bufLen];
	
	setNumTableRows (panel, ctrl, WFM_ADDR_N_CALIBRATION_PARAMETERS, 13);
		
	for (i = 0; i < WFM_ADDR_N_CALIBRATION_PARAMETERS; i++) {
		SetTableCellAttribute (panel, ctrl, MakePoint (1, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
		snprintf(buf,bufLen,"%s=",WFM_ADDR_calibrationParameterNames[i]);
		SetTableCellAttribute (panel, ctrl, MakePoint (1, i+1), ATTR_CTRL_VAL, buf);
		SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CTRL_VAL, wfm->addrCalibration[i]);
		
		if (WFM_ADDR_calibrationParametersMin[i] < WFM_ADDR_calibrationParametersMax[i]) { 
			SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_MIN_VALUE, WFM_ADDR_calibrationParametersMin[i]);
			SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_MAX_VALUE, WFM_ADDR_calibrationParametersMax[i]);
			//tprintf("set min/max for addressing parameters\n");
			SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CHECK_RANGE, VAL_COERCE);  
		} else {
			SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CHECK_RANGE, VAL_IGNORE);  	
		}
		
		
		
		SetTableCellAttribute (panel, ctrl, MakePoint (3, i+1), ATTR_CELL_DIMMED, !WFM_ADDR_allowStepParameter (i));
		SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_TYPE, WFM_ADDR_allowStepParameter (i) ? VAL_CELL_NUMERIC : VAL_CELL_PICTURE);
		if (WFM_ADDR_allowStepParameter (i)) {
			TABLE_setCellValueYesNo (panel, ctrl, MakePoint (3, i+1), wfm->addrCalibrationStep[i]);
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CTRL_VAL, wfm->addrCalibrationStepTo[i]);
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_DIMMED, !wfm->addrCalibrationStep[i]);
		}
		else {
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_DIMMED, 1);
		}

	}
}


void WFM_ADDR_showTrajParameters (int panel, int ctrl, t_waveform *wfm)
{
	int i;
	#define bufLen 1000
	char buf[bufLen];
	
	setNumTableRows (panel, ctrl, WFM_ADDR_N_TRAJ_PARAMETERS, 13);
		
	for (i = 0; i < WFM_ADDR_N_TRAJ_PARAMETERS; i++) {
		SetTableCellAttribute (panel, ctrl, MakePoint (1, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
		snprintf(buf,bufLen,"%s=",WFM_ADDR_trajParameterNames[i]);
		SetTableCellAttribute (panel, ctrl, MakePoint (1, i+1), ATTR_CTRL_VAL, buf);
		SetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CTRL_VAL, wfm->addrTrajectParams[i]);

/*		SetTableCellAttribute (panel, ctrl, MakePoint (3, i+1), ATTR_CELL_DIMMED, !WFM_ADDR_allowStepParameter (i));
		SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_TYPE, WFM_ADDR_allowStepParameter (i) ? VAL_CELL_NUMERIC : VAL_CELL_PICTURE);
		if (WFM_ADDR_allowStepParameter (i)) {
			TABLE_setCellValueYesNo (panel, ctrl, MakePoint (3, i+1), wfm->addrCalibrationStep[i]);
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CTRL_VAL, wfm->addrCalibrationStepTo[i]);
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_DIMMED, !wfm->addrCalibrationStep[i]);
		}
		else {
			SetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CELL_DIMMED, 1);
		}
*/
	}
}



void WFM_ADDR_getCalibration (int panel, int ctrl, t_waveform *wfm)
{
	int i;
	
	if (wfm == NULL) return;
	for (i = 0; i < WFM_ADDR_N_CALIBRATION_PARAMETERS; i++) {
		GetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CTRL_VAL, &wfm->addrCalibration[i]);
		//tprintf("wfm->addrCalibration[%d]=%.3f\n",i,wfm->addrCalibration[i]);
		
		if (WFM_ADDR_allowStepParameter (i)) {
			TABLE_getCellValueYesNo (panel, ctrl, MakePoint (3, i+1), &wfm->addrCalibrationStep[i]);
			GetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CTRL_VAL, &wfm->addrCalibrationStepTo[i]);
		}
	}
}


void WFM_ADDR_getTrajParameters (int panel, int ctrl, t_waveform *wfm)
{
	int i;
	
	if (wfm == NULL) return;
	for (i = 0; i < WFM_ADDR_N_TRAJ_PARAMETERS; i++) {
		GetTableCellAttribute (panel, ctrl, MakePoint (2, i+1), ATTR_CTRL_VAL, &wfm->addrTrajectParams[i]);
/*		
		if (WFM_ADDR_allowStepParameter (i)) {
			TABLE_getCellValueYesNo (panel, ctrl, MakePoint (3, i+1), &wfm->addrCalibrationStep[i]);
			GetTableCellAttribute (panel, ctrl, MakePoint (4, i+1), ATTR_CTRL_VAL, &wfm->addrCalibrationStepTo[i]);
		}
*/	}
}




//=======================================================================
//
//    init panel
//
//=======================================================================
void WFM_initPanel (void) 
{
	Rect bounds;
	
	if (menuWfmAddrTable <= 0) {
		menuWfmAddrTable  = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL9));
	}

	
	// -----------------------------------------
    // load subpanels for different waveform types
	// -----------------------------------------
	
	// init panel only once
	if (subPanelWFMPoints >= 0) return;
	
	GetPanelHandleFromTabPage (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_points,
							   &subPanelWFMPoints);
	GetPanelHandleFromTabPage (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_function,
							   &subPanelWFMFunc);
	GetPanelHandleFromTabPage (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_fromFile,
							   &subPanelWFMFile);
	GetPanelHandleFromTabPage (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_step,
							   &subPanelWFMStep);
	GetPanelHandleFromTabPage (panelWaveforms, WFM_TAB0, WFM_TAB_INDEX_addr,
							   &subPanelWFMAddr);

	GetPanelHandleFromTabPage (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_singleSite,
						       &subPanelWFMAddrSingleSite);
	GetPanelHandleFromTabPage (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_trajectories,
						       &subPanelWFMAddrTraj);
	GetPanelHandleFromTabPage (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_DMD,
						       &subPanelWFMAddrDMD);

	
	// -----------------------------------------
	//    configure table and graph
	// -----------------------------------------
	POINTS_defineControls (&WFM_controls, subPanelWFMPoints,
						 WFM_wPTS_TABLE_points,
						 WFM_wPTS_GRAPH_Waveform,
						 WFM_wPTS_NUMERIC_repetition,
						 WFM_wPTS_TEXTMSG_invalid,
						 POINTS_TABLE_ROW_HEIGHT);
	WFM_setTransferFunction (NULL);
	POINTS_initControls (&WFM_controls);

	// -----------------------------------------------
	//    important: do not call "POINTS_initTable()"
	//    AFTER EasyTab_xxx.
	//    Changing the callBackData pointer (ATTR_CALLBACK_DATA)
	//    will result in "general protection faults"
	//	  due to a bug in the library EasyTab.h
	// -----------------------------------------------

/*	EasyTab_ConvertFromCanvas (panelWaveforms, WFM_CANVAS_WfmTYpes);
	EasyTab_AddPanels (panelWaveforms,  WFM_CANVAS_WfmTYpes, 1, 
					   subPanelWFMPoints, subPanelWFMFile, subPanelWFMStep, 0);
*/
	SetCtrlAttribute (subPanelWFMFile, WFM_wFILE_GRAPH_Waveform,
                          ATTR_ENABLE_ZOOM_AND_PAN, 1);
	SetCtrlAttribute (subPanelWFMFile, WFM_wFILE_GRAPH_Waveform,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
	SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_GRAPH_Waveform,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);

	GetCtrlBoundingRect (panelWaveforms, WFM_TAB0, &bounds.top,
							 &bounds.left, &bounds.height, &bounds.width);
//	EasyTab_GetBounds (panelWaveforms, WFM_CANVAS_WfmTYpes,
//					   VAL_EASY_TAB_EXTERIOR_BOUNDS, &bounds);
	SetCtrlAttribute (panelWaveforms, WFM_RING_add, ATTR_TOP, 
					  bounds.height + bounds.top + 5);

	FUNCTION_initializeAll ();
	FUNCTION_addAllNamesToRing (subPanelWFMFunc, WFM_FUNC_RING_functions);
	SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_TABLE, ATTR_AUTO_EDIT, 1);
	SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_TABLE,
					  ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_TABLE,
					  ATTR_ENABLE_ROW_SIZING, 0);
//	FUNCTION_createAllBitmaps (ctrlHeight (subPanelWFMFunc, WFM_FUNC_CANVAS),
//							   ctrlWidth (subPanelWFMFunc, WFM_FUNC_CANVAS));
								

	menuWfmTable  = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL6));
	WFM_TABLE_init (panelWaveforms, WFM_TABLE_wfm);

	//
	// addressing
	//
	WFM_ADDR_initTable (subPanelWFMAddrSingleSite, ADDR_SS_TABLE_SINGLESITE);
	
	TABLE_setColumnAttributesYesNo (subPanelWFMAddr, WFM_ADDR_TABLE_calibration, 3);

	// DMD stuff
	DMDSHAPE_initializeAll ();
	DMDSHAPE_addAllNamesToRing (subPanelWFMAddrDMD, ADDR_DMD_RING_shapes);
	SetCtrlAttribute (subPanelWFMAddrDMD, ADDR_DMD_TABLE_parameters, ATTR_AUTO_EDIT, 1);
	SetCtrlAttribute (subPanelWFMAddrDMD, ADDR_DMD_TABLE_parameters, ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute (subPanelWFMAddrDMD, ADDR_DMD_TABLE_parameters, ATTR_ENABLE_ROW_SIZING, 0);
	
	

	
	// -----------------------------------------
	//    dimm non-used items
	// -----------------------------------------
	
	
    WFM_changeVisibility (0);
    
}


void wfmShowNPoints (t_waveform *wfm)
{
//	unsigned long deltaT;
	
	WFM_calculateOffsetAndDuration (wfm, 0, 1, 0);
//	setTime (panelWaveforms, WFM_NUMERIC_duration, wfm->duration, 1);
//	deltaT = get_timeUnits (activeSeq()->AO_timeBase) * activeSeq()->AO_patternInterval;
//	SetCtrlVal (panelWaveforms, WFM_NUMERIC_nPoints, wfm->duration / deltaT);

}





/************************************************************************/
/*
/*    waveform type: WFM_POINTS
/*
/************************************************************************/



//=======================================================================
//
//    values changed in table
//
//=======================================================================
int CVICALLBACK WAVEFORMS_EditTable_Points (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//int wfmNo;
	t_waveform *k;
	t_sequence *seq;
	
 	
	
//	ChainCtrlCallback (c->panel, c->table, POINTS_processEvents_CB, c,
//					   "POINTS_processEvents");
//	ChainCtrlCallback (c->panel, c->table, TABLE_processEvents_CB, &allowEdit,
//					   "TABLE_processEvents");
	
 	
	if (POINTS_processEvents_CB (panel, control, event, callbackData, eventData1, eventData2) > 0)
 		return 1;
	if (TABLE_processEvents_CB (panel, control, event, (void *) 1, eventData1, eventData2) > 0)
 		return 1;
 		
		
		
//	DebugPrintf ("%s (WAVEFORMS_editPoints)\n", eventStr(event, eventData1, eventData2));
//	DebugPrintf ("\n");
	//if ((event == EVENT_KEYPRESS) && (GetKeyPressEventVirtualKey (eventData2) != VAL_ENTER_VKEY)) return 0;
	switch (event) {
		case EVENT_COMMIT:
			// -----------------------------------------
			//    get ptr to current waveform
			// -----------------------------------------
    		seq = activeSeq();
//    		DebugPrintf ("val changed");
//    		GetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, &nr);
			k = activeWfm ();
//			k = WFM_ptr (seq, nr);
			// -----------------------------------------
			//    convert table to waveform
			// -----------------------------------------
			if (k != NULL) {
				POINTS_tableToListOfPoints (&WFM_controls, &k->points, &k->nPoints, 1);
				POINTS_listOfPointsToTable (&WFM_controls, k->points, k->nPoints);  
				POINTS_tableToGraph (&WFM_controls, 0);
				// -----------------------------------------
				//    enforce digitalization at next start
				// -----------------------------------------
				k->digParameters.timebase_50ns = 0;
				wfmShowNPoints (k);
			}
    		setChanges (seq, 1);
			break;
	}
	return 0;
	
}



//=======================================================================
//
//    add waveform 
//
//=======================================================================
int CVICALLBACK WFM_addChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *w;
	int old;
	
	
	switch (event)
		{
		case EVENT_COMMIT:
 			w = activeWfm ();
 			if (w == NULL) return 0;
 			old = w->add;
			GetCtrlVal (panel, control, &w->add);
			if (w->add != old) {
				// force redigitalization
				w->digParameters.timebase_50ns = 0;
				setChanges (activeSeq(), 1);
			}
			break;
		}
	return 0;
}


/************************************************************************/
/*
/*    waveform type: WFM_FILE  
/*
/************************************************************************/

//=======================================================================
//
//    load a new waveform from file
//
//=======================================================================
int CVICALLBACK WFM_browse_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	int err;
	t_waveform *w;
	
	switch (event)
		{
		case EVENT_COMMIT:
			// -----------------------------------------
			//    get filename
			// -----------------------------------------
			w = activeWfm ();
		    if (FileSelectPopup (config->defaultPath, "*.WFM", "*.WFM",
								 "Load waveform file", VAL_LOAD_BUTTON, 0, 1, 1, 0,
								 filename) <= 0) return 0;
            w->filename = strnewcopy (w->filename, filename);
			// -----------------------------------------
			//    load waveform
			// -----------------------------------------
			if ((err = WFM_loadFromFile  (activeSeq(), w)) == 0) {
                SetCtrlVal (subPanelWFMFile, WFM_wFILE_STRING_Filename, w->filename);
				w->digParameters.timebase_50ns = 0;
			}
			// -----------------------------------------
			//    display  waveform
			// -----------------------------------------
		    WFM_show (activeSeq(), activeWfmNo());
			setChanges (activeSeq(), 1);
            break;
		}
	return 0;
}

//=======================================================================
//
//    attribute of waveform changed
//
//=======================================================================
int CVICALLBACK WFM_FILE_AttributeChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    //int nr;
    t_waveform *w;
    double dTime;
    double fhelp;
    unsigned long thelp;
	//int ihelp;
	t_sequence *seq;

    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
    if (event == EVENT_COMMIT) {
		seq = activeSeq();
	    if (ListNumItems (seq->lWaveforms) == 0) return 0;
//	    GetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, &nr);
		w = activeWfm ();
//			WFM_ptr (seq, nr);
        if (w == NULL) return 0;
		switch (control) {
			// -----------------------------------------
			//    amplitude
			// -----------------------------------------
            case WFM_wFILE_NUMERIC_amplitudeFrom:
	            GetCtrlVal (panel, control, &fhelp);
	            if (fhelp != w->uScaleFrom) {
	            	w->uScaleFrom = fhelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
                break;
            case WFM_wFILE_NUMERIC_amplitudeTo:
	            GetCtrlVal (panel, control, &fhelp);
	            if (fhelp != w->uScaleTo) {
	            	w->uScaleTo = fhelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
                break;
            case WFM_wFILE_RADIOBUTTON_amplVary:
	            GetCtrlVal (panel, control, &w->uScaleVary);
				WFM_show (seq, activeWfmNo());
				PARAMETERSWEEPS_updateDisplay (seq);			
				setChanges (seq, 1);
                break;
			// -----------------------------------------
			//    offset
			// -----------------------------------------
            case WFM_wFILE_NUMERIC_offsetFrom:
	            GetCtrlVal (panel, control, &fhelp);
	            if (fhelp != w->offsetFrom) {
	            	w->offsetFrom = fhelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
                break;
            case WFM_wFILE_NUMERIC_offsetTo:
	            GetCtrlVal (panel, control, &fhelp);
	            if (fhelp != w->offsetTo) {
	            	w->offsetTo = fhelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
                break;
            case WFM_wFILE_RADIOBUTTON_offsVary:
	            GetCtrlVal (panel, control, &w->offsetVary);
				WFM_show (seq, activeWfmNo());
				PARAMETERSWEEPS_updateDisplay (seq);			
				setChanges (seq, 1);
                break;
			// -----------------------------------------
			//    duration
			// -----------------------------------------
            case WFM_wFILE_NUMERIC_durationFrom:
	            GetCtrlVal (panel, control, &dTime);
	            thelp = timeToInt (dTime);   
	            if (thelp != w->durationFrom_50ns) {
		            w->durationFrom_50ns = thelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
				wfmShowNPoints (w);
                break;
            case WFM_wFILE_NUMERIC_durationTo:
	            GetCtrlVal (panel, control, &dTime);
	            thelp = timeToInt (dTime);   
	            if (thelp != w->durationTo_50ns) {
		            w->durationTo_50ns = thelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
				}
				wfmShowNPoints (w);
                break;
            case WFM_wFILE_RADIOBUTTON_durVary:
	            GetCtrlVal (panel, control, &w->durationVary);
				WFM_show (seq, activeWfmNo());
				PARAMETERSWEEPS_updateDisplay (seq);			
				setChanges (seq, 1);
                break;
        }
    }
    return 0;
}



/************************************************************************/
/*
/*    waveform type: WFM_STEP 
/*
/************************************************************************/

//=======================================================================
//
//    attibutes of 'step' waveform changed
//
//=======================================================================
int CVICALLBACK WFM_STEP_AttributeChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    t_waveform *w;
 
    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
	if (event == EVENT_COMMIT) {
	    if (ListNumItems (activeSeq()->lWaveforms) == 0) return 0;
//	    GetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, &nr);
	    w = activeWfm();//WFM_ptr (activeSeq(), nr);
        switch (control) {
            case WFM_wSTEP_NUMERIC_stepFrom:
                GetCtrlVal (panel, control, &w->stepFrom);
            	WFM_show (activeSeq(), activeWfmNo());
 				setChanges (activeSeq(), 1);
             break;
            case WFM_wSTEP_NUMERIC_stepTo:
                GetCtrlVal (panel, control, &w->stepTo);
                WFM_show (activeSeq(), activeWfmNo());
				setChanges (activeSeq(), 1);
                break;
            case WFM_wSTEP_NUMERIC_stepRep:
                GetCtrlVal (panel, control, &w->stepRep);
                WFM_show (activeSeq(), activeWfmNo());
				setChanges (activeSeq(), 1);
                break;
            case WFM_wSTEP_BTN_alternate:
                GetCtrlVal (panel, control, &w->stepAlternate);
                WFM_show (activeSeq(), activeWfmNo());
				setChanges (activeSeq(), 1);
				break;
            
        }
    }
    return 0;
}



/************************************************************************/
/*
/*    display waveforms
/*
/************************************************************************/


void POINTS_plotTransferFunction (int panel, int control, 
								  t_sequence *seq, t_waveform *wfm,
								  t_digitizeParameters *p)
{
	t_transferFunction *t;
    double *xList, *yList;
    int i;
	
	t = TRANSFERFUNCT_ptr (seq, wfm->transferFunction);
	if (t == NULL) return;

	if (wfm->nPoints == 0) return;
	xList = (double *) malloc (sizeof (double)*wfm->nPoints);
	yList = (double *) malloc (sizeof (double)*wfm->nPoints);
	// -------------------------------------------
	//    create list of points
	// -------------------------------------------
    for (i=0; i<wfm->nPoints; i++) {
         xList[i] = wfm->points[i].timeStart_ns;
         if (i > 0) xList[i] += xList[i-1];
         yList[i] = TRANSFERFUNCT_apply (t, wfm->points[i].valueStart);
         if (yList[i] > p->maxVoltage) yList[i] = p->maxVoltage;
         if (yList[i] < p->minVoltage) yList[i] = p->minVoltage;
 	}    
	// -------------------------------------------
	//    plot points
	// -------------------------------------------
 	PlotXY (panel, control, xList, yList,
            wfm->nPoints, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
            VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
    free (yList);
	free (xList);
}

//=======================================================================
//
//    plot waveform in graph
//
//=======================================================================
int WFM_plot (int panel, int control, t_sequence *seq, t_waveform *wfm, 
				   t_digitizeParameters *p, int retainGraph)
{
    double *xList, *yList;
    int i;
    char help[30];
	int nPlotPointsFunction = 501;
	t_function *f;
	int plotHandle = 0;
	
    
	if (p == NULL) p = digitizeParameters_PCI67XX(1);
    if (!retainGraph) DeleteGraphPlot (panel, control, -1, VAL_DELAYED_DRAW);
//    *pHandle = 0;
    if (wfm == NULL) {
		RefreshGraph (panel, control);
    	return 0;
	}
	WFM_calculateOffsetAndDuration (wfm, seq->repetition, seq->nRepetitions, 0);
	
	// -----------------------------------------
	//    assign list of x-values
	// -----------------------------------------
    switch (wfm->type) {
		// -----------------------------------------
		//    WFM_TYPE_POINTS
		// -----------------------------------------
        case WFM_TYPE_POINTS:
		case WFM_TYPE_ADDRESSING:
//            if (wfm->transferFunction != 0) {
 //           	POINTS_plotTransferFunction (panel, control, seq, wfm, p);
 //           }
            if (wfm->nPoints == 1) {
				// -----------------------------------------
				//    wfm consists of only ONE point
				// -----------------------------------------
				sprintf (help, "U_const = %1.3fV", wfm->points[0].valueStart);
				PlotText (panel, control, 0.0, wfm->points[0].valueStart, help, VAL_EDITOR_META_FONT,
						  VAL_WAVEFORM_PLOT_COLOR, VAL_TRANSPARENT);
				plotHandle = PlotLine (panel, control, 0, wfm->points[0].valueStart, 1000, wfm->points[0].valueStart,
						  VAL_WAVEFORM_PLOT_COLOR);
         	}
         	else {
				// -------------------------------------------
				//    wfm consists of MANY points (standard)
				// -------------------------------------------
				plotHandle = POINTS_plot (panel, control, wfm->points, wfm->nPoints, seq->repetition, WFM_controls.timeMultiply, retainGraph);
			}
            break;
		// -----------------------------------------
		//    WFM_TYPE_FILE
		// -----------------------------------------
    	case WFM_TYPE_FILE:
    		if (wfm->nPoints == 0) break;
    		xList = (double *) malloc (sizeof (double)*wfm->nPoints);
    		yList = (double *) malloc (sizeof (double)*wfm->nPoints);
			// -------------------------------------------
			//    create list of points
			// -------------------------------------------
            for (i=0; i<wfm->nPoints; i++) {
                 xList[i] = (wfm->durationThisRepetition_ns / TIME_MULTIPLY_ms * i) / (wfm->nPoints-1);
                 yList[i] = wfm->uList[i] * wfm->uScaleThisRepetition + wfm->offsetThisRepetition;
                 if (yList[i] > p->maxVoltage) yList[i] = p->maxVoltage;
                 if (yList[i] < p->minVoltage) yList[i] = p->minVoltage;
         	}    
			// -------------------------------------------
			//    plot points
			// -------------------------------------------
         	plotHandle = PlotXY (panel, control, xList, yList,
                    wfm->nPoints, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_WAVEFORM_PLOT_COLOR);
            free (yList);
    		free (xList);
            break;
        case WFM_TYPE_STEP:
			RefreshGraph (panel, control);
			break;
		case WFM_TYPE_FUNCTION:
    		f = FUNCTION_ptr (wfm->functionID);
			if (f == NULL) return 0;
    		xList = (double *) malloc (sizeof (double)*nPlotPointsFunction);
    		yList = (double *) malloc (sizeof (double)*nPlotPointsFunction);
			// -------------------------------------------
			//    create list of points
			// -------------------------------------------
            for (i=0; i<nPlotPointsFunction; i++) {
                 xList[i] = (wfm->durationThisRepetition_ns / TIME_MULTIPLY_ms * i) / (nPlotPointsFunction-1);
                 yList[i] = f->eval (xList[i], wfm->functionParametersThisRepetition, wfm->durationThisRepetition_ns / 1E6, wfm->functionReverseTime);
                 if (yList[i] > p->maxVoltage) yList[i] = p->maxVoltage;
                 if (yList[i] < p->minVoltage) yList[i] = p->minVoltage;
         	}    
			// -------------------------------------------
			//    plot points
			// -------------------------------------------
			plotHandle = PlotXY (panel, control, xList, yList, nPlotPointsFunction, VAL_DOUBLE,
					VAL_DOUBLE, VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
					VAL_WAVEFORM_PLOT_COLOR);
            free (yList);
    		free (xList);
			break;			
        	
    }
	return plotHandle;
}


void WFM_FUNC_displayParametersInTable (int panel, int ctrl, t_waveform *wfm)
{
	t_function *f;
	Point cell;
	//Rect cells;
	int i;
	int sizeChanged;
	int index;
	char stepTypeStr[100];
	int stepType;
	char tmp[800];
	
	
	//tprintf("WFM_FUNC_displayParametersInTable\n");
	
	
	f = FUNCTION_ptr (wfm->functionID);
	if ((f == NULL) || (wfm->functionParameters == NULL)) {
		DeleteTableRows (panel, ctrl, 1, -1);
		return;
	}
	
	sizeChanged = setNumTableRows (panel, ctrl, f->nParameters, 20);
	if (sizeChanged) {
		//tprintf("size changed\n"); 
		
		SetTableCellRangeAttribute (panel, ctrl, VAL_TABLE_COLUMN_RANGE(WFM_FUNC_TABLECOL_VALUE_FROM), ATTR_PRECISION, 4);
		

		//TABLE_setColumnAttributesYesNo (panel, ctrl, WFM_FUNC_TABLECOL_VARY);
		TABLE_setColumnAttributeStepType (panel, ctrl, WFM_FUNC_TABLECOL_VARY);
		
		
/*		cells = MakeRect (1, WFM_FUNC_TABLECOL_VARY, f->nParameters, 1);
		InsertTableCellRangeRingItem (panel, ctrl, cells, 0, "no");
		InsertTableCellRangeRingItem (panel, ctrl, cells, 1, "yes");
		SetTableCellRangeAttribute (panel, ctrl, cells,
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_CENTER_JUSTIFIED)
*/	
	}	
		
	index = wfm->functionID-1;
	if (index < 0) return;
	for (i = 0; i < f->nParameters; i++) {
		
		//tprintf("update column %d\n",i); 
		
		cell = MakePoint (WFM_FUNC_TABLECOL_NAME, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, f->parameterNames[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_FROM, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MAX_VALUE, f->maxValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MIN_VALUE, f->minValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, wfm->functionParameters[index][i].from);
		//SetTableCellAttribute (panel, ctrl, cell, ATTR_CELL_DIMMED,f->parameterType[i] != FUNCTION_PARAM_TYPE_STANDARD);	

		cell = MakePoint (WFM_FUNC_TABLECOL_UNIT1, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, f->parameterUnits[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VARY, i+1);
		
	//	TABLE_setCellValueYesNo (panel, ctrl, cell, wfm->functionParametersVary[index][i]);
		
		stepType = wfm->functionParameters[index][i].stepType;
		GetTableCellRingValueFromIndex(panel,ctrl,cell,stepType,stepTypeStr);
		SetTableCellVal(panel,ctrl,cell,stepTypeStr);
		
		// use "stepTo" column as multi-purpose:
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_TO, i+1); 
		SetTableCellAttribute(panel,ctrl,cell,ATTR_CELL_DIMMED,stepType==WFM_STEP_TYPE_NONE);
		switch (stepType) {
			
			case WFM_STEP_TYPE_LIST:
				SetTableCellAttribute(panel,ctrl,cell,ATTR_CELL_TYPE,VAL_CELL_STRING);
				SetTableCellAttribute(panel,ctrl,cell,ATTR_CELL_MODE,VAL_INDICATOR);
				STEP_PARAM_getListString(tmp,&wfm->functionParameters[index][i]);
				SetTableCellAttribute(panel,ctrl,cell,ATTR_CTRL_VAL,tmp);
				break;
			case WFM_STEP_TYPE_TO_LINEAR:
			case WFM_STEP_TYPE_NONE: 
				SetTableCellAttribute(panel, ctrl, cell, ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
				SetTableCellAttribute(panel, ctrl, cell, ATTR_PRECISION, 4);
				SetTableCellAttribute(panel, ctrl, cell, ATTR_MAX_VALUE, f->maxValue[i]);
				SetTableCellAttribute(panel, ctrl, cell, ATTR_MIN_VALUE, f->minValue[i]);
				SetTableCellAttribute(panel, ctrl, cell, ATTR_CTRL_VAL,  wfm->functionParameters[index][i].to);
				//tprintf("set val: index=%d, i=%d -> %.3f\n",index,i,wfm->functionParameters[index][i].to);
				SetTableCellAttribute(panel, ctrl, cell, ATTR_CELL_MODE, VAL_HOT);
			
			default:
				break;
				
			
		}
		 
		
		
		//SetTableCellAttribute (panel, ctrl, cell, ATTR_CELL_DIMMED,f->parameterType[i] != FUNCTION_PARAM_TYPE_STANDARD);

//		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, wfm->functionParametersVary[i]);
		

		cell = MakePoint (WFM_FUNC_TABLECOL_UNIT2, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, f->parameterUnits[i]);
		
		SetTableCellRangeAttribute(panel, ctrl, VAL_TABLE_ROW_RANGE (i+1) ,ATTR_CELL_DIMMED, f->parameterType[i] != FUNCTION_PARAM_TYPE_STANDARD);
		
	}				   

}



void WFM_FUNC_getParametersFromTable (int panel, int ctrl, t_waveform *wfm)
{
	int i;
	t_function *f;
	Point cell;
	int index;
	int cellType;
	t_step_param *currStep;
	
	char val[MAX_PATHNAME_LEN];
	int cellValLength;

	f = FUNCTION_ptr (wfm->functionID);
	if (f == NULL) return;
	
	index = wfm->functionID-1;
	for (i = 0; i < f->nParameters; i++) {
		
		currStep = &wfm->functionParameters[index][i]; 
		
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_FROM, i+1);
		GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &(currStep->from));
		cell = MakePoint (WFM_FUNC_TABLECOL_VARY, i+1);
//		GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &wfm->functionParametersVary[i]);
		//TABLE_getCellValueYesNo (panel, ctrl, cell, &wfm->functionParametersVary[index][i]);
		
		GetTableCellValLength(panel,ctrl,cell,&cellValLength);
		if (cellValLength >= MAX_PATHNAME_LEN) {
			tprintf("severe error in WFM_FUNC_table\n");
			return;
		}
		GetTableCellVal(panel,ctrl,cell, val);
		GetTableCellRingIndexFromValue(panel,ctrl,0,cell,&(currStep->stepType),val);
		//tprintf("found table ring val: %s\n",val);
		
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_TO, i+1);
		GetTableCellAttribute(panel,ctrl,cell,ATTR_CELL_TYPE,&cellType);
		if (cellType == VAL_CELL_NUMERIC) { // make sure not to load string as double
			GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &(currStep->to));
		}
	}
	
}




void WFM_ADDR_DMD_displayParametersInTable (int panel, int ctrl, t_DMDimage *d)
{
	t_DMDshape *s;
	Point cell;
	Rect cells;
	int i;
	int sizeChanged;
	
	
	if (d == NULL) return;
	s = DMDSHAPE_ptr (d->shapeID);
	if ((s == NULL) || (d->shapeParametersFrom == NULL)) {
		DeleteTableRows (panel, ctrl, 1, -1);
		return;
	}
	
	sizeChanged = setNumTableRows (panel, ctrl, s->nParameters, 20);
	if (sizeChanged) {
		cells = MakeRect (1, WFM_FUNC_TABLECOL_VALUE_FROM, VAL_TO_EDGE, 1);
		SetTableCellRangeAttribute (panel, ctrl, cells, ATTR_PRECISION, 4);
		cells = MakeRect (1, WFM_FUNC_TABLECOL_VALUE_TO, VAL_TO_EDGE, 1);
		SetTableCellRangeAttribute (panel, ctrl, cells, ATTR_PRECISION, 4);
		TABLE_setColumnAttributesYesNo (panel, ctrl, WFM_FUNC_TABLECOL_VARY);

/*		cells = MakeRect (1, WFM_FUNC_TABLECOL_VARY, f->nParameters, 1);
		InsertTableCellRangeRingItem (panel, ctrl, cells, 0, "no");
		InsertTableCellRangeRingItem (panel, ctrl, cells, 1, "yes");
		SetTableCellRangeAttribute (panel, ctrl, cells,
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_CENTER_JUSTIFIED)
*/	
	}	
		
	for (i = 0; i < s->nParameters; i++) {
		cell = MakePoint (WFM_FUNC_TABLECOL_NAME, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, s->parameterNames[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_FROM, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MAX_VALUE, s->maxValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MIN_VALUE, s->minValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, d->shapeParametersFrom[i]);

		cell = MakePoint (WFM_FUNC_TABLECOL_UNIT1, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, s->parameterUnits[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VARY, i+1);
		
		TABLE_setCellValueYesNo (panel, ctrl, cell, d->shapeParametersVary[i]);

//		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, wfm->functionParametersVary[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_TO, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MAX_VALUE, s->maxValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_MIN_VALUE, s->minValue[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, d->shapeParametersTo[i]);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CELL_DIMMED,
							   !d->shapeParametersVary[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_UNIT2, i+1);
		SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, s->parameterUnits[i]);
	}

}



void WFM_ADDR_DMD_getParametersFromTable (int panel, int ctrl, t_DMDimage *d)
{
	int i;
	t_DMDshape *s;
	Point cell;

	if (d == NULL) return;

	s = DMDSHAPE_ptr (d->shapeID);
	if (s == NULL) return;
	
	for (i = 0; i < s->nParameters; i++) {
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_FROM, i+1);
		GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &d->shapeParametersFrom[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VARY, i+1);
//		GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &wfm->functionParametersVary[i]);
		TABLE_getCellValueYesNo (panel, ctrl, cell, &d->shapeParametersVary[i]);
		cell = MakePoint (WFM_FUNC_TABLECOL_VALUE_TO, i+1);
		GetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, &d->shapeParametersTo[i]);
	}
	
}




typedef struct {
	t_waveform *wfm;
	t_sequence *seq;
	
} t_wfm_seq;

int CVICALLBACK THREAD_WFM_ADDR_DMD_displayShapeInGraph (void* functionData)
{
	t_wfm_seq *data = functionData;
	t_waveform *wfm = data->wfm;
	t_sequence *seq = data->seq;
	int repetition;
	int plotHandle;
	int bitmapID = 0;
	int picNum = 0;
	
	GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_NUMERIC_repetition, &repetition);
	WFM_ADDR_offsetThisRepetition (wfm, repetition-1, seq->nRepetitions);
	DMDIMAGE_calculateParametersThisRepetition (wfm->addrDMDimage, seq->nRepetitions, repetition-1);
	bitmapID = DMDIMAGE_createBitmap (wfm,seq->repetition,picNum);
	if (bitmapID == 0) {
		free(data); 
		return 0;
	}
	//CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, DMDIMAGE_THREAD_displayShapeInCanvas, wfm, NULL);
	
	plotHandle = PlotBitmap (subPanelWFMAddrDMD, ADDR_DMD_GRAPH_waveform, 0.0, 0.0, 
				ctrlWidth(subPanelWFMAddrDMD, ADDR_DMD_GRAPH_waveform), 
				ctrlHeight(subPanelWFMAddrDMD, ADDR_DMD_GRAPH_waveform), 0);
	SetCtrlBitmap (subPanelWFMAddrDMD, ADDR_DMD_GRAPH_waveform, plotHandle,bitmapID);
	DiscardBitmap(bitmapID);
	bitmapID = 0;
	free(data);
	return 0;
}


void WFM_ADDR_DMD_displayShapeInGraph (t_waveform *wfm, t_sequence *seq)
{

	
	t_wfm_seq *data = malloc(sizeof(t_wfm_seq));
	data->wfm = wfm;
	data->seq = seq;
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, THREAD_WFM_ADDR_DMD_displayShapeInGraph, data, NULL);
	
}						


//=======================================================================
//
//    display waveform in panel
//
//=======================================================================
void WFM_show (t_sequence *seq, int nr)
{
    t_waveform *wfm;
   	t_transferFunction *f;
	unsigned int matrixNum;
	
	GetCtrlVal(subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_matrixNum, &matrixNum);

    
    wfm = WFM_ptr (seq, nr);
    if (wfm == NULL) {
		DeleteGraphPlot (subPanelWFMPoints, WFM_wPTS_GRAPH_Waveform, -1, VAL_IMMEDIATE_DRAW);
		DeleteGraphPlot (subPanelWFMFile, WFM_wPTS_GRAPH_Waveform, -1, VAL_IMMEDIATE_DRAW);
		WFM_changeVisibility (WFM_TYPE_NONE);
    	return;
    }
    WFM_changeVisibility (wfm->type);
    SetCtrlVal (panelWaveforms, WFM_STRING_WFM_Name, wfm->name);
    
    switch (wfm->type) {
		// -----------------------------------------
		//    WFM_TYPE_POINTS
		// -----------------------------------------
        case WFM_TYPE_POINTS:
			f = TRANSFERFUNCT_ptr (seq, wfm->transferFunction);
			WFM_setTransferFunction (f);
			POINTS_listOfPointsToTable (&WFM_controls, wfm->points, wfm->nPoints);
			
			GetCtrlVal (subPanelWFMPoints, WFM_wPTS_NUMERIC_repetition, &seq->repetition);
			seq->repetition--;
		    WFM_plot(subPanelWFMPoints, WFM_wPTS_GRAPH_Waveform, seq, wfm, NULL, 0);
			SetCtrlAttribute (subPanelWFMPoints, WFM_wPTS_TEXTMSG_invalid, ATTR_VISIBLE,
					  !POINTS_consistent (wfm->points, wfm->nPoints));
			// show transfer function
			SetCtrlAttribute (subPanelWFMPoints, WFM_wPTS_STRING_transferFct, 
							  ATTR_VISIBLE, f != NULL);
			SetCtrlVal (subPanelWFMPoints, WFM_wPTS_STRING_transferFct, 
				        TRANSFERFUNCT_str (f));
			SetCtrlAttribute (subPanelWFMPoints, WFM_wPTS_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
			SetCtrlVal (subPanelWFMPoints, WFM_wPTS_NUMERIC_timebaseStre, wfm->timebaseStretch);
			SetCtrlVal (subPanelWFMPoints, WFM_wPTS_NUMERIC_timebase,  1.0*(seq->AO_timebase_50ns*wfm->timebaseStretch) / VAL_us);
			break;
		// -----------------------------------------
		//    WFM_TYPE_FILE
		// -----------------------------------------
        case WFM_TYPE_FILE: 
			SetCtrlVal (subPanelWFMFile, WFM_wFILE_NUMERIC_nPoints, wfm->nPoints);
		    if (wfm->filename == NULL)
		        SetCtrlVal (subPanelWFMFile, WFM_wFILE_STRING_Filename, "");
		    else 
		        SetCtrlVal (subPanelWFMFile, WFM_wFILE_STRING_Filename, extractFilename (wfm->filename));

		    
			SetCtrlVal (subPanelWFMFile, WFM_wFILE_NUMERIC_amplitudeFrom, wfm->uScaleFrom);
		    SetCtrlVal (subPanelWFMFile, WFM_wFILE_NUMERIC_amplitudeTo, wfm->uScaleTo);
		    SetCtrlVal (subPanelWFMFile, WFM_wFILE_RADIOBUTTON_amplVary, wfm->uScaleVary);
			SetAttributeForCtrls (subPanelWFMFile, ATTR_DIMMED, !wfm->uScaleVary, 0,
								  WFM_wFILE_NUMERIC_amplitudeTo,
								  WFM_wFILE_TEXTMSG_amplFrom,
								  WFM_wFILE_TEXTMSG_amplTo, 0);
			
			SetCtrlVal (subPanelWFMFile, WFM_wFILE_NUMERIC_offsetFrom, wfm->offsetFrom);
		    SetCtrlVal (subPanelWFMFile, WFM_wFILE_NUMERIC_offsetTo, wfm->offsetTo);
		    SetCtrlVal (subPanelWFMFile, WFM_wFILE_RADIOBUTTON_offsVary, wfm->offsetVary);
			SetAttributeForCtrls (subPanelWFMFile, ATTR_DIMMED, !wfm->offsetVary, 0,
								  WFM_wFILE_NUMERIC_offsetTo,
								  WFM_wFILE_TEXTMSG_offsetFrom,
								  WFM_wFILE_TEXTMSG_offsetTo, 0);

		    setTime (subPanelWFMFile, WFM_wFILE_NUMERIC_durationFrom, wfm->durationFrom_50ns,1);
		    setTime (subPanelWFMFile, WFM_wFILE_NUMERIC_durationTo, wfm->durationTo_50ns,1);
		    SetCtrlVal (subPanelWFMFile, WFM_wFILE_RADIOBUTTON_durVary, wfm->durationVary);
			SetAttributeForCtrls (subPanelWFMFile, ATTR_DIMMED, !wfm->durationVary, 0,
								  WFM_wFILE_NUMERIC_durationTo,
								  WFM_wFILE_TEXTMSG_durationFrom,
								  WFM_wFILE_TEXTMSG_durationTo, 0);
			
			WFM_plot(subPanelWFMFile, WFM_wFILE_GRAPH_Waveform, seq, wfm, NULL, 0);
	     	break;
		// -----------------------------------------
		//    WFM_TYPE_STEP
		// -----------------------------------------
	    case WFM_TYPE_STEP:
			SetCtrlVal (subPanelWFMStep, WFM_wSTEP_NUMERIC_stepFrom,
						wfm->stepFrom);
			SetCtrlVal (subPanelWFMStep, WFM_wSTEP_NUMERIC_stepTo,
						wfm->stepTo);
			SetCtrlVal (subPanelWFMStep, WFM_wSTEP_NUMERIC_stepRep,
						wfm->stepRep);
			SetCtrlVal (subPanelWFMStep, WFM_wSTEP_BTN_alternate,
						wfm->stepAlternate);
			SetAttributeForCtrls (subPanelWFMStep, ATTR_VISIBLE,
								  !wfm->stepAlternate, 0, 
								  WFM_wSTEP_TEXTMSG_from,
								  WFM_wSTEP_TEXTMSG_to,
								  WFM_wSTEP_TEXTMSG_V1,
								  WFM_wSTEP_TEXTMSG_arrow,
								  WFM_wSTEP_TEXTMSG_V2,
								  WFM_wSTEP_TEXTMSG_msg1,
								  WFM_wSTEP_TEXTMSG_msg2,
								  0);
			SetAttributeForCtrls (subPanelWFMStep, ATTR_VISIBLE,
								  wfm->stepAlternate, 0, 
								  WFM_wSTEP_TEXTMSG_between,
								  WFM_wSTEP_TEXTMSG_and,
								  0);
			
			break;
		case WFM_TYPE_FUNCTION:
			SetCtrlVal (subPanelWFMFunc, WFM_FUNC_RING_functions, wfm->functionID);
			SetCtrlVal (subPanelWFMFunc, WFM_FUNC_TEXTMSG_function, FUNCTION_getFormula (wfm->functionID));
			WFM_FUNC_displayParametersInTable (subPanelWFMFunc, WFM_FUNC_TABLE, wfm);
			FUNCTION_displayBitmapInCanvas (subPanelWFMFunc, WFM_FUNC_CANVAS, wfm->functionID);
		    setTime (subPanelWFMFunc, WFM_FUNC_NUMERIC_durationFrom, wfm->durationFrom_50ns,1);
		    setTime (subPanelWFMFunc, WFM_FUNC_NUMERIC_durationTo, wfm->durationTo_50ns,1);
		    SetCtrlVal (subPanelWFMFunc, WFM_FUNC_RADIOBUTTON_durVary, wfm->durationVary);
			SetAttributeForCtrls (subPanelWFMFunc, ATTR_DIMMED, !wfm->durationVary, 0,
								  WFM_FUNC_NUMERIC_durationTo,
								  WFM_FUNC_TEXTMSG_durationFrom,
								  WFM_FUNC_TEXTMSG_durationTo, 0);
		    SetCtrlVal (subPanelWFMFunc, WFM_FUNC_RADIOBUTTON_reverseT, wfm->functionReverseTime);
			SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
            SetCtrlVal (subPanelWFMFunc, WFM_FUNC_NUMERIC_timebaseStre, wfm->timebaseStretch);
			SetCtrlVal (subPanelWFMFunc, WFM_FUNC_NUMERIC_timebase,  1.0*(seq->AO_timebase_50ns*wfm->timebaseStretch) / VAL_us);
			WFM_plot(subPanelWFMFunc, WFM_FUNC_GRAPH_Waveform, seq, wfm, NULL, 0);
			
			break;
		case WFM_TYPE_ADDRESSING:
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_RING_link, wfm->addrLinkWfm);
			SetAttributeForCtrls (subPanelWFMAddr, ATTR_DIMMED, wfm->addrLinkWfm != 0, 0, 
				WFM_ADDR_TAB,
				WFM_ADDR_TABLE_calibration,
				WFM_ADDR_NUMERIC_centerX,
				WFM_ADDR_NUMERIC_centerY,
				WFM_ADDR_STRING_testFile,
				WFM_ADDR_STRING_testFile2,
				WFM_ADDR_COMMANDBUTTON_browse,
				WFM_ADDR_COMMANDBUTTON_browse2,
				WFM_ADDR_COMMANDBUTTON_show,
				WFM_ADDR_COMMANDBUTTON_testPos,
				WFM_ADDR_NUMERIC_imageSize, 
				WFM_ADDR_CHECKBOX_enableFeedbk,
				WFM_ADDR_CHECKBOX_enableFeedb2,
				WFM_ADDR_NUMERIC_camera,
				0);
			
			WFM_ADDR_showCalibration (subPanelWFMAddr, WFM_ADDR_TABLE_calibration, wfm);
			
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_centerX, wfm->addrCenter.x);
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_centerY, wfm->addrCenter.y);
			if (wfm->addrTestFilename != NULL) SetCtrlVal (subPanelWFMAddr, WFM_ADDR_STRING_testFile, wfm->addrTestFilename);
			if (wfm->addrTestFilename2 != NULL) SetCtrlVal (subPanelWFMAddr, WFM_ADDR_STRING_testFile2, wfm->addrTestFilename2);
		    setTime (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_duration, wfm->durationFrom_50ns,1);
            SetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_enableFeedbk, wfm->addrEnablePhaseFeedback);
            SetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_enableFeedb2, wfm->addrEnablePhaseFeedback2);
            SetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_camera, wfm->addrCamera);
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_addOffset, wfm->addrAddLastOffset);
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_addOffsetNo1, wfm->addrAddLastOffsetNotAtFirst);
			SetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_imageSize, wfm->addrImageSize);
			
			switch (wfm->addrType) {
				case WFM_ADDR_TYPE_TRAJECTORY:
					WFM_ADDR_showTrajParameters (subPanelWFMAddrTraj, ADDR_TRAJ_TABLE_parameters, wfm);     
					break;
				case WFM_ADDR_TYPE_SINGLESITE:
					WFM_ADDR_matrixToTable (subPanelWFMAddrSingleSite, ADDR_SS_TABLE_SINGLESITE, wfm, matrixNum);
					WFM_ADDR_showDisorderPara(subPanelWFMAddrSingleSite,wfm, matrixNum);
					SetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_CHECKBOX_stepMatrix, wfm->stepAddrMatrix);
					SetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_picsPerRep, wfm->addrMatricesPerRepetition);
					break;
				case WFM_ADDR_TYPE_DMD:
					WFM_ADDR_matrixToTable (subPanelWFMAddrSingleSite, ADDR_SS_TABLE_SINGLESITE, wfm, matrixNum);
					WFM_ADDR_showDisorderPara(subPanelWFMAddrSingleSite,wfm, matrixNum);
					SetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_CHECKBOX_stepMatrix, wfm->stepAddrMatrix); 
					// WFM_ADDR_showDMDParameters()
					//WFM_ADDR_matrixToTable (subPanelWFMAddrSingleSite, ADDR_SS_TABLE_SINGLESITE, wfm, matrixNum);
					//SetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_CHECKBOX_stepMatrix, wfm->stepAddrMatrix);
					SetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_picsPerRep, wfm->addrMatricesPerRepetition); 
					if (wfm->addrDMDimage != NULL) {
						SetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_enableDMD, wfm->addrDMDimage->enableDMD);    
						SetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDfeedback, wfm->addrDMDimage->enableDMDPhaseFeedback);
						SetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDerrdiff, wfm->addrDMDimage->enableDMDErrorDiffusion);
						SetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDsmoothing, wfm->addrDMDimage->enableDMDSmoothing);
						
						SetCtrlAttribute (subPanelWFMAddrDMD, ADDR_DMD_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
						SetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_RING_shapes, wfm->addrDMDimage->shapeID);
						ResetTextBox (subPanelWFMAddrDMD, ADDR_DMD_TEXTBOX_description, DMDSHAPE_getDescription (wfm->addrDMDimage->shapeID));
						WFM_ADDR_DMD_displayParametersInTable (subPanelWFMAddrDMD, ADDR_DMD_TABLE_parameters, wfm->addrDMDimage);
						WFM_ADDR_DMD_displayShapeInGraph (wfm, seq);
					}
					break;
			}
			
			// set tab page visibility
			SetTabPageAttribute (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_singleSite,
								 ATTR_DIMMED, wfm->addrType != WFM_ADDR_TYPE_SINGLESITE && wfm->addrType != WFM_ADDR_TYPE_DMD);
	
			SetTabPageAttribute (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_trajectories,
								 ATTR_DIMMED, wfm->addrType != WFM_ADDR_TYPE_TRAJECTORY);
	
			SetTabPageAttribute (subPanelWFMAddr, WFM_ADDR_TAB, WFM_ADDR_TAB_INDEX_DMD,
								 ATTR_DIMMED, wfm->addrType != WFM_ADDR_TYPE_DMD);
			
			SetActiveTabPage (subPanelWFMAddr, WFM_ADDR_TAB, wfm->addrType); 
			
			//SetCtrlAttribute (subPanelWFMAddr, WFM_ADDR_TAB, );
			break;
			
			
   	}
	SetCtrlVal (panelWaveforms, WFM_RING_add, wfm->add);
	
	
	wfmShowNPoints (wfm);
	
	// update channels using this waveform
	DIGITALBLOCKS_updateCorrespondingChannels(seq, nr);
}





//=======================================================================
//
//    show digitized waveform
//
//=======================================================================
int CVICALLBACK WFM_Digitize (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    unsigned long maxT;  //this variable might be unused at the end
    t_waveform *wfm;
    double length, t;
    int i;
    double *graphX, *graphY;
	t_digitizeParameters *p;
	t_sequence *seq;
    
    if (event == EVENT_COMMIT) {
		// -----------------------------------------
		//    get active wavefrom
		// -----------------------------------------
        wfm = activeWfm ();
        if (wfm == NULL) return 0;
//        WFM_show (activeSeq(), activeWfmNo());
		// -----------------------------------------
		//    calculate max. time
		// -----------------------------------------
		seq = activeSeq();
		switch (wfm->type) {
			case WFM_TYPE_POINTS: 
				GetCtrlVal (subPanelWFMPoints, WFM_wPTS_NUMERIC_repetition, &seq->repetition);
				break;
			case WFM_TYPE_FUNCTION:
				GetCtrlVal (subPanelWFMFunc, WFM_FUNC_NUMERIC_repetition, &seq->repetition);
				break;
			default:
				seq->repetition = 1;
		}
		
		seq->repetition--;
		OUTPUTDATA_getTimebases (seq);
//       deltaT = get_timeUnits (activeSeq()->AO_timeBase) * activeSeq()->AO_patternInterval;
        WFM_calculateOffsetAndDuration (wfm, seq->repetition, seq->nRepetitions, 0);
        maxT = (unsigned long)(seq->AO_timebase_50ns * (wfm->durationThisRepetition_50ns / seq->AO_timebase_50ns));
		if (wfm->durationThisRepetition_50ns % seq->AO_timebase_50ns != 0) {
			MessagePopup ("Warning!", "Waveform length is no integer multiple\nof hardware timebase!");
		    maxT += seq->AO_timebase_50ns;
		}
        WFM_show (seq, activeWfmNo());
		// -----------------------------------------
		//    digitize waveform
		// -----------------------------------------
		SetWaitCursor (1);
        WFM_digitize (seq, wfm, digitizeParameters_PCI67XX(seq->AO_timebase_50ns), 0);
        if (wfm->DIG_NValues == 0) {
//            DebugPrintf ("Warning! No Values created (timebase too long).\n");
			SetWaitCursor (0);
            return 0;
        }
		// -----------------------------------------
        //    calculate graph of digitized waveform
		// -----------------------------------------
        t = 0;
        graphX = (double *) malloc (sizeof (double) * wfm->DIG_NValues * 2);
        graphY = (double *) malloc (sizeof (double) * wfm->DIG_NValues * 2);
        p = digitizeParameters_PCI67XX (1);
        for (i=0; i<wfm->DIG_NValues; i++) {
	 		// -----------------------------------------
	        //    determine length
			// -----------------------------------------
     		if (wfm->DIG_NValues == 1) length = 1000.0;
     		else length = (1.0 * wfm->DIG_repeat[i] * seq->AO_timebase_50ns) / VAL_ms;
	 		// -----------------------------------------
	        //    create step-like graph
			// -----------------------------------------
    		graphX[2*i]   = t;
    		graphX[2*i+1] = t + length;
    		graphY[2*i]	  = DigitalToAnalog(wfm->DIG_values[i], p);
    		graphY[2*i+1] = graphY[2*i];
    		t += length;
    	}
		// -----------------------------------------
        //    plot digitized waveform
		// -----------------------------------------
		if (wfm->DIG_NValues > 0) {
//			Breakpoint();
			if (panel == subPanelWFMPoints) {
				PlotXY (subPanelWFMPoints, WFM_wPTS_GRAPH_Waveform,  graphX, graphY, wfm->DIG_NValues*2,
						VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE,
						VAL_SOLID, 1, VAL_DK_GREEN);
				RefreshGraph (subPanelWFMPoints, WFM_wPTS_GRAPH_Waveform);
			}
			if (panel == subPanelWFMFile) {
				PlotXY (subPanelWFMFile, WFM_wFILE_GRAPH_Waveform,  graphX, graphY, wfm->DIG_NValues*2,
						VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE,
						VAL_SOLID, 1, VAL_DK_GREEN);
				RefreshGraph (subPanelWFMFile, WFM_wFILE_GRAPH_Waveform);
			}
			if (panel == subPanelWFMFunc) {
				PlotXY (subPanelWFMFunc, WFM_FUNC_GRAPH_Waveform,  graphX, graphY, wfm->DIG_NValues*2,
						VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE,
						VAL_SOLID, 1, VAL_DK_GREEN);
				RefreshGraph (subPanelWFMFunc, WFM_FUNC_GRAPH_Waveform);
			}
			
		}
		// -----------------------------------------
        //    tidy up
		// -----------------------------------------
		free (graphX);
		free (graphY);
		SetWaitCursor (0);
    }
    return 0;
}

    

char *WFM_createListNameEntry (t_waveform *w, int active)
{
	static char help[300];

	int bgcolor;
	int fgcolor = VAL_BLACK;
	 
	switch (active) {
		case 1:
			bgcolor = MakeColor (200, 220, 255);
			break;
		case 2:
			bgcolor = MakeColor (100, 200, 255);
			break;
		default:
			bgcolor = VAL_WHITE;
	}
			
	help[0] = 0;
	if (w->name != NULL) {
		appendColorSpec (help, bgcolor, fgcolor);
		strcat (help, w->name);
	}
	strcat (help, strSpace50);
	
	return help;

}

 


//=======================================================================
//
//    fill waveform names to listbox
//
//=======================================================================
void WFM_fillNamesToListbox (t_sequence *seq, int panel, int control, int color)
{
    int i;
    t_waveform *w;
    int nWaveforms;
    
    ClearListCtrl (panel, control);
    
    if (color) {
    	// 
    	WFM_checkAllIfActive (seq);
    	nWaveforms = ListNumItems (seq->lWaveforms);
		for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
		    w = WFM_ptr (seq, i);
		    if (!w->hide) InsertListItem (panel, control, -1, 
		    				              WFM_createListNameEntry(w, w->isActive), i);
		}
	}
	else {
	    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
	        w = WFM_ptr(seq, i);
	        if (!w->hide) InsertListItem (panel, control, -1, w->name, i);
	    }
	}

	

}



void WFM_fillStepNamesToListbox (t_sequence *seq, int panel, int control)
{
    int i;
    t_waveform *w;
    
    ClearListCtrl (panel, control);
	InsertListItem (panel, control, -1, "NONE", WFM_ID_UNCHANGED);
    for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
        w = WFM_ptr(seq, i);
        if ((w->type == WFM_TYPE_STEP) && (!w->hide))
        	InsertListItem (panel, control, -1, WFM_ptr(seq,i)->name, i);
    }
}

/*
//=======================================================================
//
//    fill waveform names to listbox
//
//=======================================================================
void fillWaveformNamesToRing (t_sequence *seq, int panel, int control)
{
	int i;
	
	ClearListCtrl (oanelm, control);
	InsertListItem (panelWaveforms (), WFM_RING_add, -1, strUnchanged, WFM_ID_UNCHANGED);
	for (i=1; i <= ListNumItems (seq->lWaveforms); i++) 
	{
		InsertListItem (panelWaveforms (), WFM_RING_add, -1, WFM_ptr(seq, i)->name, i);
    }
}

*/



void WFM_ADDR_initLinkRing (int panel, int ctrl, t_sequence *seq)
{
	int i;
	t_waveform *w;
	
	ClearListCtrl (panel, ctrl);
	InsertListItem (panel, ctrl, -1, "NONE", 0);
	for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
		ListGetItem (seq->lWaveforms, &w, i);
		if (w->type == WFM_TYPE_ADDRESSING) {
			InsertListItem (panel, ctrl, -1, w->name, i);
		}
		
	}
}




//=======================================================================
//
//    edit waveform
//
//=======================================================================
void WFM_edit (t_sequence *seq, int nr, int sort) 
{
	t_waveform *wfm;
	int error;
	
	EnableBreakOnLibraryErrors ();
	// -----------------------------------------
	//    display all names
	// -----------------------------------------
	if (nr == 0) nr = min (ListNumItems (seq->lWaveforms), 1);
	if (nr > (int)ListNumItems(seq->lWaveforms)) nr = ListNumItems (seq->lWaveforms);
	if (sort) WFM_TABLE_sort (seq);
	wfm = WFM_ptr (seq, nr);
	if (wfm == NULL) {
		WFM_TABLE_displayAllWfms (panelWaveforms, WFM_TABLE_wfm, seq, -1, 0);       
		return;
	}
	SetCtrlAttribute (subPanelWFMFunc, WFM_FUNC_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
	SetCtrlAttribute (subPanelWFMPoints, WFM_wPTS_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
	
	WFM_TABLE_setActiveRow (wfm->positionInTable);

	WFM_fillStepNamesToListbox (seq, panelWaveforms, WFM_RING_add);
	
	WFM_ADDR_initLinkRing (subPanelWFMAddr,WFM_ADDR_RING_link, seq);

	// -----------------------------------------
	// 	 show actual waveform
	// -----------------------------------------
	// TODO if there is no waveform: display bug!
	WFM_TABLE_displayAllWfms (panelWaveforms, WFM_TABLE_wfm, seq, activeWfmNo(), 0);
	WFM_TABLE_displayWfm (activeWfm(), panelWaveforms, WFM_TABLE_wfm);
	WFM_show (seq, nr);
	error = SetActiveCtrl (panelWaveforms, WFM_TABLE_wfm);
	//tprintf("ERROR: SetActiveCtrl (panelWaveforms, WFM_TABLE_wfm) = %d\n",error);
	
}


//=======================================================================
//
//    delete waveform
//
//=======================================================================
void WFM_deleteSelected (void)
{
    int nr;
    t_waveform *wfm;
    t_sequence *seq;
    
    seq = activeSeq();

	if (ListNumItems (seq->lWaveforms)  == 0) return;
	
	// -----------------------------------------
    //    delete waveform
	// -----------------------------------------
	wfm = activeWfm();
	if (wfm == NULL) return;
	
	if (ConfirmPopupf ("Delete waveform", "Do you really want to delete waveform\n'%s'?", wfm->name) == 0) return;
	nr =  activeWfmNo();
	WFM_deleteWithReferences (seq, nr);
	// -----------------------------------------
    //    show new waveform
	// -----------------------------------------
    if (nr == ListNumItems (seq->lWaveforms)) nr --;
	WFM_edit (seq, nr, 1);
	PARAMETERSWEEPS_updateDisplay (seq);			
	setChanges (seq, 1);
}



void WFM_deleteUnused (t_sequence *seq)
{
	int i;
    int nr;
    t_waveform *wfm;
	char tmp[500];
	int changes = 0;

	nr =  activeWfmNo();
	WFM_fillInfos (seq);
	for (i = ListNumItems (seq->lWaveforms); i > 0; i--) {
		ListGetItem (seq->lWaveforms, &wfm, i);
		if (wfm->usedBy[0] == 0) {
			/*if (ConfirmPopupf ("Delete waveform", "Do you really want to delete waveform no. %d\n\n'%s'?", i, wfm->name) != 0) {
				WFM_deleteWithReferences (seq, i);
			    if (i == nr) WFM_TABLE_setActiveRow (1);
			}*/
			sprintf(tmp,"Do you really want to delete waveform no. %d\n\n'%s'?", i, wfm->name);
			
			switch (PostGenericMessagePopup ("Delete waveform", tmp, "Yes","No","Cancel",0,0,0,VAL_GENERIC_POPUP_BTN3,VAL_GENERIC_POPUP_NO_CTRL,VAL_GENERIC_POPUP_BTN3)) {
				case VAL_GENERIC_POPUP_BTN1: 
					WFM_deleteWithReferences (seq, i);
			    	if (i == nr) WFM_TABLE_setActiveRow (1);
					changes++;
					break;
				case VAL_GENERIC_POPUP_BTN2:
					break;
				case VAL_GENERIC_POPUP_BTN3:
					if (changes > 0) {
						WFM_edit (seq, nr, 1);
						PARAMETERSWEEPS_updateDisplay (seq);			
						setChanges (seq, 1);	
					} 
					return;
			}
		}
	}
	WFM_edit (seq, nr, 1);
	PARAMETERSWEEPS_updateDisplay (seq);			
	setChanges (seq, 1);
}





//=======================================================================
//
//    listbox clicked 
//
//=======================================================================
int CVICALLBACK WFM_listBoxClicked_CALLBACK (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    
    switch (event) {
        case EVENT_VAL_CHANGED:
            WFM_show (activeSeq(), activeWfmNo());
			SetActivePanel (panel);
            break;
        case EVENT_KEYPRESS:
        	if (eventData1 == VAL_FWD_DELETE_VKEY) {
        	    WFM_deleteSelected ();
				SetActivePanel (panel);
        	    return 1;
        	}
        }
    return 0;
}


//=======================================================================
//
//    name of waveform changed 
//
//=======================================================================
int CVICALLBACK WFM_MAIN_WFMNameChanged (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    t_waveform *w;
    t_sequence *seq;
	char name[MAX_WFM_NAME_LEN];

    seq = activeSeq();
    if (ListNumItems (seq->lWaveforms) == 0) return 0;
	// -----------------------------------------
	//   get current wavefrom
	// -----------------------------------------
    if (event == EVENT_COMMIT) {
//	    GetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, &nr);
	    w = activeWfm(); 
		// -----------------------------------------
		//   change name in data
		// -----------------------------------------
		
        GetCtrlVal (panelWaveforms, WFM_STRING_WFM_Name, name);
		w->name[0] = 0;
		if (WFM_checkForDuplicateNameAndRename (seq, name)) {
			PostMessagePopupf ("Waveform name changed", "Waveform name already exists.\n\nName has been changed to '%s'.", name);
        	SetCtrlVal (panelWaveforms, WFM_STRING_WFM_Name, name);
		}
		strcpy (w->name, name);
		// -----------------------------------------
		//   update all displays
		// -----------------------------------------
        WFM_TABLE_displayAllWfms (panelWaveforms, WFM_TABLE_wfm, seq, WFM_active, 0);
        WFM_fillStepNamesToListbox (seq, panelWaveforms, WFM_RING_add);
//		SetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, nr);
		setChanges (seq, 1);
    }
    return 0;
}


void WFMTYPE_initPanel (void) 
{
	if (panelWfmType > 0) return;
	panelWfmType = LoadPanel (0, UIR_File, WFMTYPE);
		
	ClearListCtrl (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "point sequence", WFM_TYPE_POINTS);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "function", WFM_TYPE_FUNCTION);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "waveform from file", WFM_TYPE_FILE);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "step output voltage", WFM_TYPE_STEP);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "addressing (single site)", WFM_TYPE_ADDRESSING+ 10*WFM_ADDR_TYPE_SINGLESITE);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "addressing (trajectories)", WFM_TYPE_ADDRESSING+ 10*WFM_ADDR_TYPE_TRAJECTORY);
	InsertListItem (panelWfmType, WFMTYPE_RINGSLIDE_wfmtype, -1, "addressing (DMD)", WFM_TYPE_ADDRESSING+10*WFM_ADDR_TYPE_DMD);
}

//=======================================================================
//
//    new waveform
//
//=======================================================================
int CVICALLBACK WFM_newWaveform_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			// -----------------------------------------
	        //    get type
			// -----------------------------------------
			WFMTYPE_initPanel ();
//			SetPanelPos (panelWfmType,
//						 panelTop (panelMain2) + ctrlTop (panelMain, MAIN2_TAB0) + ctrlTop (panelWaveforms, WFM_BTN_NewWFM) + 10,
//						 min (panelLeft (panelMain2) + ctrlLeft (panelMain, MAIN2_TAB0) + ctrlRight (panelWaveforms, WFM_BTN_NewWFM)+ 20, screenWidth()-200));
			InstallPopup (panelWfmType);
			break;
		}
	return 0;
}




int CVICALLBACK WFM_deleteWaveform_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
  	switch (event)
		{
		case EVENT_COMMIT:
		    WFM_deleteSelected ();
		    setChanges (activeSeq(),1);
			break;
		}
	return 0;
}



int CVICALLBACK WFM_deleteUnusedWaveforms_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
  	switch (event)
		{
		case EVENT_COMMIT:
			WFM_deleteUnused (activeSeq());
		    setChanges (activeSeq(),1);
			break;
		}
	return 0;
}




int CVICALLBACK WFM_tabChanged_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	if (event != EVENT_MOUSE_POINTER_MOVE) 
//	  DebugPrintf ("%s\n", eventStr(event,eventData1,eventData2));
	//t_waveform *wfm;
	
	/*switch (event)
	{
		case EVENT_LEFT_CLICK:
			break;
		
		case EVENT_ACTIVE_TAB_CHANGE:
			wfm = activeWfm ();
			if (wfm != NULL) WFM_changeVisibility (wfm->type);
			break;
	}*/
	return 1; // consume click to avoid tab change
}



int CVICALLBACK WFMTYPE_done_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    t_waveform *new;
    int nr;
    int type;
    t_sequence *seq;
	char newName[MAX_WFM_NAME_LEN];

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, WFMTYPE_RINGSLIDE_wfmtype, &type);
			RemovePopup (1);
			// -----------------------------------------
	        //    create & init new waveform
			// -----------------------------------------
			seq = activeSeq();
			
    		strcpy (newName, "untitled");
			WFM_checkForDuplicateNameAndRename (seq, newName);
			
		    new = WFM_new (seq);
    		WFM_init (new, type % 10);
			switch (type % 10) {
				case WFM_TYPE_FUNCTION:
					new->functionID = 1;
					WFM_FUNC_allocateMemory (new);
					new->durationFrom_50ns = 10 * VAL_ms;
					break;
				case WFM_TYPE_ADDRESSING:
					new->addrType = type / 10;
					switch (new->addrType) {
						case WFM_ADDR_TYPE_SINGLESITE:
							break;
						case WFM_ADDR_TYPE_TRAJECTORY:
							break;
						case WFM_ADDR_TYPE_DMD:
							new->addrDMDimage = (t_DMDimage*) malloc (sizeof (t_DMDimage));
							DMDIMAGE_init (new->addrDMDimage);
							break;
					}
					break;
			}
    		strcpy (new->name, newName);
			// -----------------------------------------
	        //    insert new name into all analog channels (DAC0-15)
			// -----------------------------------------

			nr = ListNumItems (seq->lWaveforms);
			// -----------------------------------------
	        //    edit waveform
			// -----------------------------------------
			new->positionInTable = nr;
			new->positionInList = nr;
			//WFM_TABLE_setActiveRow (new->positionInTable);
			WFM_edit (seq, nr, 0);
			setChanges (seq, 1);
			SetActiveCtrl (panelWaveforms, WFM_STRING_WFM_Name);
			break;
	}
	return 0;
}

int CVICALLBACK WFMTYPE_abort_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup (1);
			break;
	}
	return 0;
}




int CVICALLBACK WFM_FUNC_attributeChanged_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

    t_waveform *w;
    double dTime;
    //double fhelp;
    unsigned long thelp;
	int ihelp;
	t_sequence *seq;

    if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');
    if (event == EVENT_COMMIT) {
		seq = activeSeq();
	    if (ListNumItems (seq->lWaveforms) == 0) return 0;
//	    GetCtrlVal (panelWaveforms, WFM_LISTBOX_Waveforms, &nr);
		w = activeWfm ();//WFM_ptr (seq, nr);
        switch (control) {
			// -----------------------------------------
			//    duration
			// -----------------------------------------
            case WFM_FUNC_RING_functions:
				GetCtrlVal (panel, control, &ihelp);
				if (ihelp != w->functionID) {
					w->functionID = ihelp;
					WFM_FUNC_allocateMemory (w);
					w->digParameters.timebase_50ns = 0;
	            	WFM_show (seq, activeWfmNo());
					setChanges (seq, 1);
				}
				break;
			case WFM_FUNC_NUMERIC_durationFrom:
	            GetCtrlVal (panel, control, &dTime);
	            thelp = timeToInt (dTime);   
	            if (thelp != w->durationFrom_50ns) {
		            w->durationFrom_50ns = thelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					setChanges (seq, 1);
				}
				wfmShowNPoints (w);
                break;
            case WFM_FUNC_NUMERIC_durationTo:
	            GetCtrlVal (panel, control, &dTime);
	            thelp = timeToInt (dTime);   
	            if (thelp != w->durationTo_50ns) {
		            w->durationTo_50ns = thelp;
	            	WFM_show (seq, activeWfmNo());
					w->digParameters.timebase_50ns = 0;
					setChanges (seq, 1);
				}
				wfmShowNPoints (w);
                break;
            case WFM_FUNC_RADIOBUTTON_durVary:
	            GetCtrlVal (panel, control, &w->durationVary);
				WFM_show (seq, activeWfmNo());
				w->digParameters.timebase_50ns = 0;
				setChanges (seq, 1);
                break;
			case WFM_FUNC_RADIOBUTTON_reverseT:
	            GetCtrlVal (panel, control, &w->functionReverseTime);
				WFM_show (seq, activeWfmNo());
				w->digParameters.timebase_50ns = 0;
				setChanges (seq, 1);
				break;
			case WFM_FUNC_NUMERIC_timebaseStre:
	            GetCtrlVal (panel, control, &w->timebaseStretch);
				WFM_show (seq, activeWfmNo());
				w->digParameters.timebase_50ns = 0;
				setChanges (seq, 1);
				break;
		}
    }
	return 0;
}



// TODO which tables use this callback
int CVICALLBACK TABLE_parameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	t_sequence *seq;
	Point mouse;
	Point cell;
	int value;
	int cellType;
	//int stepType;
	char filename[MAX_PATHNAME_LEN];
	int i;
	t_step_param* currStep;
	
	switch (event)
	{
		case EVENT_KEYPRESS:
			switch (eventData1) {
				case ',':  FakeKeystroke ('.'); break;
			}
			break;
		case EVENT_COMMIT:
			wfm = activeWfm ();
			seq = activeSeq();
			if (wfm == NULL) return 0;
			WFM_FUNC_getParametersFromTable (panel, control, wfm);
			WFM_show (activeSeq (), activeWfmNo());
			PARAMETERSWEEPS_updateDisplay (seq);		
			// -----------------------------------------
			//    enforce digitalization at next start
			// -----------------------------------------
			wfm->digParameters.timebase_50ns = 0;
			
			setChanges (seq, 1);
			return 1;
		//case EVENT_LEFT_CLICK:
		//	break;
		case EVENT_LEFT_DOUBLE_CLICK:
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    GetTableCellFromPoint (panel, control, mouse, &cell); 
			if (cell.x == 0 || cell.y == 0) {
				return 0;	
			}
			if (1) { //event = EVENT_LEFT_CLICK 
				wfm = activeWfm ();
				GetTableCellAttribute(panel,control,MakePoint(WFM_FUNC_TABLECOL_VARY,cell.y),ATTR_CELL_TYPE,&cellType);
				if (cell.x == WFM_FUNC_TABLECOL_VALUE_TO && cellType == VAL_CELL_RING && wfm->functionParameters[wfm->functionID-1][cell.y-1].stepType == WFM_STEP_TYPE_LIST) {
					//GetTableCellVal(panel,control,MakePoint(WFM_FUNC_TABLECOL_VARY,cell.y),&
					//tprintf("showing edit window for list step\n");

					// load step values from file
				    if (FileSelectPopup (config->defaultPath, "*.WFM", "*.WFM",
										 "Load waveform file", VAL_LOAD_BUTTON, 0, 1, 1, 0,
										 filename) <= 0) return 0;
		            
					// -----------------------------------------
					//    load waveform
					// -----------------------------------------
					currStep = &(wfm->functionParameters[wfm->functionID-1][cell.y-1]);
					if (currStep->listSize > 0 && currStep->list != NULL) {
						free(currStep->list);
						currStep->list = NULL;
						currStep->listSize = 0;
					}
					if (WFM_pointlist_from_file(&(currStep->list),&(currStep->listSize),filename,-10,1000) != 0) {
		                PostMessagePopupf("Error","Error while loading points from file %s",filename);
					}
					tprintf("\nloaded data:\n");
					for (i=0; i < currStep->listSize; i++) {
						
						tprintf("%.3f\n",currStep->list[i]);
					}
					
					// -----------------------------------------
					//    display  waveform
					// -----------------------------------------
				    WFM_show (activeSeq(), activeWfmNo());
					setChanges (activeSeq(), 1);
				
				}
				
				
				break;	
			}
			// TODO
			TABLE_changeRow (panel, control, cell, 0);
			if (TABLE_isCellYesNo (panel, control, cell)) {
				TABLE_getCellValueYesNo (panel, control, cell, &value);
				TABLE_setCellValueYesNo (panel, control, cell, !value);
				CallCtrlCallback (panel, control, EVENT_COMMIT, cell.y, cell.x, 0);
				wfm = activeWfm ();
				wfm->digParameters.timebase_50ns = 0;
			}
			break;
		
			
	}
	return 0;
}

 int MENU_WFM_TABLE = 23187652;
 
typedef struct {
	int panel;
	int control;
	Point cell;
	char* value;
	int channel;
} t_wfm_table_popup_callback_data;

t_wfm_table_popup_callback_data *WFM_TABLE_popup_callback_data(int panel, int control, Point cell, int channel) {
	t_wfm_table_popup_callback_data *data = malloc(sizeof(t_wfm_table_popup_callback_data));
	data->panel = panel;
	data->control = control;
	data->cell = cell;
	data->channel = channel;
	return data;
}
 
void CVICALLBACK WFM_TABLE_channel_select_callback (int menuBarHandle, int menuItemID, void *callbackData, int panelHandle) {
	char channelName[MAX_PATHNAME_LEN];
	
	t_wfm_table_popup_callback_data* data = callbackData;
	WFM_TABLE_channelName (data->channel,channelName);  
	SetTableCellVal(data->panel,data->control,data->cell,channelName);
	if (activeWfm() != NULL) {
		activeWfm()->channel = data->channel;
		WFM_TABLE_displayWfm (activeWfm(), data->panel, WFM_TABLE_wfm);  
	}
	free(data);
}


Point WFM_TABLE_create_popup(int panel, int control, Point cell) {
	static int menu = 0;
	static int menu2 = 0;
	int i;
	char channelName[MAX_PATHNAME_LEN];
	
	if (menu != 0) {
		DiscardMenuBar(menu);	 // frees also Menu
	}
	menu = NewMenuBar (0); // zero is allowed here for popup menu
	menu2 = NewMenu(menu,"waveform table channel select popup",0);    
	
	// FIXME who needs to free all the MenuItems and the callbackData of non-called functions ???
	WFM_TABLE_channelName (WFM_CHANNEL_ALL,channelName);
	NewMenuItem (menu, menu2, channelName, -1, 0, WFM_TABLE_channel_select_callback, WFM_TABLE_popup_callback_data(panel,control,cell,WFM_CHANNEL_ALL));
	WFM_TABLE_channelName (WFM_CHANNEL_NONE,channelName);
	NewMenuItem (menu, menu2, channelName, -1, 0, WFM_TABLE_channel_select_callback, WFM_TABLE_popup_callback_data(panel,control,cell,WFM_CHANNEL_NONE));
	WFM_TABLE_channelName (WFM_CHANNEL_GPIB,channelName);
	NewMenuItem (menu, menu2, channelName, -1, 0, WFM_TABLE_channel_select_callback, WFM_TABLE_popup_callback_data(panel,control,cell,WFM_CHANNEL_GPIB));
	

	for (i = 0; i < N_DAC_CHANNELS; i++) {
		WFM_TABLE_channelName (i,channelName);
		NewMenuItem (menu, menu2, channelName, -1, 0, WFM_TABLE_channel_select_callback, WFM_TABLE_popup_callback_data(panel,control,cell,i));
	}

	return MakePoint(menu,menu2);
}

int CVICALLBACK WFM_TABLE_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//Rect selection;
	t_sequence *seq;
	int oldActiveWfm;
	Point cell;
	int nr;
	t_waveform *wfm;
	//char txt[50];
	//int first;
	int oldSortMode;
	//int choice;
	Point menu;
	int runState = VAL_SELECT_STATE;
	//int error;
	//Point p;
				 
//	DebugPrintf ("%s (WFM TABLE callback )\n", eventStr(event, eventData1, eventData2));
	 GetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, &runState);
	 
	 switch (event) {
		//case EVENT_MOUSE_WHEEL_SCROLL:
		//	tprintf("mouse wheel scroll\n");
		//	break;
		case EVENT_RIGHT_CLICK:
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
           	seq = activeSeq();
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			//:GetActiveTableCell (panel, control, &cell);
			
			if (cell.y > 0) {
				oldActiveWfm = WFM_nrFromTableRow (WFM_activeTableRow);
				WFM_activeTableRow = cell.y;
				WFM_TABLE_displayWfm (WFM_ptr (seq, oldActiveWfm), panelWaveforms, WFM_TABLE_wfm);
				WFM_TABLE_displayWfm (activeWfm(), panelWaveforms, WFM_TABLE_wfm);
				WFM_show (seq, activeWfmNo());
			}
			if (event == EVENT_RIGHT_CLICK && cell.y > 0 && cell.x == WFM_TABLE_COL_channel) {
				// do popup	
				menu = WFM_TABLE_create_popup(panel, control, cell);
				RunPopupMenu (menu.x, menu.y, panel,eventData1, eventData2, 0,0,0,0);
			}
			
//			DebugPrintf ("Nr selected: %d\n: ", activeWfmNo());
			break;
		case EVENT_COMMIT:
			if (eventData1 < 1) return 0;
			nr = WFM_nrFromTableRow (eventData1);			
			
//			DebugPrintf ("Nr selected: %d, %d\n: ", nr, eventData1);
			seq = activeSeq();
			wfm = WFM_ptr (seq, nr);
			if (wfm == NULL) return 0;
			cell = MakePoint (eventData2, eventData1);
//			DebugPrintf ("%s", wfm->name);
			/*switch (eventData2) { // column:
				case WFM_TABLE_COL_channel:
					GetTableCellVal (panel, control, cell, txt);
					GetTableCellRingIndexFromValue (panel, control, 0, cell, &wfm->channel,
													txt);
					switch (wfm->channel) {
						case INDEX_WFM_CHANNEL_ALL: 
							wfm->channel = WFM_CHANNEL_ALL;
							break;
						case INDEX_WFM_CHANNEL_NONE: 
							wfm->channel = WFM_CHANNEL_NONE;
							break;
						case INDEX_WFM_CHANNEL_GPIB: 
							wfm->channel = WFM_CHANNEL_GPIB;
							break;
						default:
							GetTableCellRingIndexFromValue (panel, control, 0, cell, &first,
										    WFM_TABLE_channelName (0));
							wfm->channel -= first;
					}
			}*/
			break;
		case EVENT_TABLE_ROW_COL_LABEL_CLICK:
			if (eventData1 != 0) return 0;
			seq = activeSeq();
			oldSortMode = seq->wfmTableSortMode;
			wfm = activeWfm ();
//			DebugPrintf ("active: listpos: %d;   tablepos: %d (%s)\n", activeWfm()->positionInList,activeWfm()->positionInTable, activeWfm()->name);
			if (wfm == NULL) return 0;
			switch (eventData2) {
				case WFM_TABLE_COL_id:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_NR;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				case WFM_TABLE_COL_name:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_NAME;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				case WFM_TABLE_COL_channel:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_CHANNEL;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				case WFM_TABLE_COL_usedBy:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_USEDBY;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				case WFM_TABLE_COL_type:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_TYPE;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				case WFM_TABLE_COL_graph:
					break;
				case WFM_TABLE_COL_nPoints:
					seq->wfmTableSortMode = WFM_TABLE_SORT_MODE_NPOINTS;
					if (seq->wfmTableSortMode == oldSortMode) {
						seq->wfmTableSortDescending = !seq->wfmTableSortDescending;
					}
					WFM_TABLE_sort (seq);
					WFM_TABLE_setActiveRow (wfm->positionInTable);
					break;
				
			}
			
//			DebugPrintf ("active: listpos: %d;   tablepos: %d (%s)\n", activeWfm()->positionInList,activeWfm()->positionInTable, activeWfm()->name);
			WFM_TABLE_displayAllWfms (panel, control, seq, activeWfmNo (), 0);
			SetTableSelection (panel, control, MakeRect (wfm->positionInTable, 1, 1, WFM_TABLE_NCols-1));
	 		break;
		/*case EVENT_KEYPRESS:// does not work (table never really "active"
			tprintf("keypress on wfm table\n");
			error = GetActiveTableCell (panel, control, &p);
			if (error < 0) {
				return 0;	
			}
			switch (eventData1 & VAL_VKEY_MASK) {
				case VAL_DOWN_ARROW_VKEY:
    				//if (runState == VAL_EDIT_STATE) {
					//	return 0;
    				//} else {
    					p.y += 1;
						TABLE_changeRow (panel, control, p, 1);
						
						if (p.y <= 0) return 1;
						oldActiveWfm = WFM_nrFromTableRow (WFM_activeTableRow);
						WFM_activeTableRow = p.y;
						WFM_TABLE_displayWfm (WFM_ptr (seq, oldActiveWfm), panelWaveforms, WFM_TABLE_wfm);
						WFM_TABLE_displayWfm (activeWfm(), panelWaveforms, WFM_TABLE_wfm);
						WFM_show (seq, activeWfmNo());
						//CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
						return 1;
    				//}
				case VAL_UP_ARROW_VKEY:
    				//if (runState == VAL_EDIT_STATE) {
					//	return 0;
					//} else {
						p.y -= 1;
						TABLE_changeRow (panel, control, p, 1);
						
						if (p.y <= 0) return 1; 
						oldActiveWfm = WFM_nrFromTableRow (WFM_activeTableRow);
						WFM_activeTableRow = p.y;
						WFM_TABLE_displayWfm (WFM_ptr (seq, oldActiveWfm), panelWaveforms, WFM_TABLE_wfm);
						WFM_TABLE_displayWfm (activeWfm(), panelWaveforms, WFM_TABLE_wfm);
						WFM_show (seq, activeWfmNo());
						//CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
						return 1;
					//}
    				
			}  */
			
	 }
	
	//SetActiveCtrl (panel, WFM_TABLE_wfm); // does not work??
	return 0;
}


int CVICALLBACK WFM_changeRepetitionPoints (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *k;
	
	switch (event)
	{
		case EVENT_COMMIT:
			k = activeWfm ();
			// -----------------------------------------
			//    convert table to waveform
			// -----------------------------------------
			if (k != NULL) {
				POINTS_tableToListOfPoints (&WFM_controls, &k->points, &k->nPoints, 1);
				POINTS_listOfPointsToTable (&WFM_controls, k->points, k->nPoints); 
				POINTS_tableToGraph (&WFM_controls, 0);
				// -----------------------------------------
				//    enforce digitalization at next start
				// -----------------------------------------
				k->digParameters.timebase_50ns = 0;
				wfmShowNPoints (k);
			}

			break;
	}
	return 0;
}



int CVICALLBACK WFM_changeRepetitionFunc (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *k;
	t_sequence *seq;
	
	switch (event)
	{
		case EVENT_COMMIT:
			k = activeWfm ();
			seq = activeSeq ();
			// -----------------------------------------
			//    convert table to waveform
			// -----------------------------------------
			if (k == NULL) return 0;
			GetCtrlVal (subPanelWFMFunc, WFM_FUNC_NUMERIC_repetition, &seq->repetition);
			seq->repetition --;
			WFM_plot(subPanelWFMFunc, WFM_FUNC_GRAPH_Waveform, seq, k, NULL, 0);

			

			break;
	}
	return 0;
}






void WFM_ADDR_showPointListInGraph (int panel, int ctrl, ListType pointList, int type, int color, int showText)
{
	dPoint p;
	int i;
		
	for (i = 1; i <= ListNumItems (pointList); i++) {
		ListGetItem (pointList, &p, i);
		PlotPoint (panel, ctrl, p.x, p.y, type, color);
		if (showText) {
			PlotText (panel, ctrl, p.x, p.y, intToStr (i), VAL_APP_META_FONT,
				  VAL_WHITE, VAL_TRANSPARENT);
		}
	}
}


void WFM_ADDR_showLineInGraph (int panel, int ctrl, ListType pointList, int color)
{
	dPoint p1, p2;
	int i;
		
	for (i = 1; i < ListNumItems (pointList); i++) {
		ListGetItem (pointList, &p1, i);
		ListGetItem (pointList, &p2, i+1);
		PlotLine (panel, ctrl, p1.x, p1.y, p2.x, p2.y, color);
	}
}




void WFM_ADDR_showImages (t_image *img, t_waveform *wfm, int imageSize, const char *txt)	
{
	int axisMin, axisMax;


	// x-axis
	axisMin = wfm->addrCenter.x - imageSize / 2;
	if (axisMin < 0) axisMin = 0;
	axisMax = wfm->addrCenter.x + imageSize / 2;
	if (axisMax > img->nPixel) axisMax = img->nPixel;
	SetAxisScalingMode (panelAddressing, ADDRESS_GRAPH1, VAL_BOTTOM_XAXIS, VAL_MANUAL, axisMin, axisMax);
	
	// y-axis
	axisMin = wfm->addrCenter.y - imageSize / 2;
	if (axisMin < 0) axisMin = 0;
	axisMax = wfm->addrCenter.y + imageSize / 2;
	if (axisMax > img->nPixel) axisMax = img->nPixel;
	SetAxisScalingMode (panelAddressing, ADDRESS_GRAPH1,
						VAL_LEFT_YAXIS, VAL_MANUAL, axisMin, axisMax);

	IMAGE_showInGraph (panelAddressing, ADDRESS_GRAPH2, img, txt);
	IMAGE_showInGraph (panelAddressing, ADDRESS_GRAPH1, img, txt);
	
	SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1, ATTR_ENABLE_ZOOM_AND_PAN, 1);

	RefreshGraph (panelAddressing, ADDRESS_GRAPH1);	// window update
	RefreshGraph (panelAddressing, ADDRESS_GRAPH2);
}


//
// find focus from images of the idle sequence
// return -1 on error or the determined pifoc voltage for focus
//
// parameter: seq that contains the data for the auto focus (typically idle seq, this is copied to real seq in startSequence)
double WFM_ADDR_findFocusFromIdleSequence(t_sequence *idleSeq)
{
	t_ccdImages *img;
	double focusVal;
	char filename[300];
	#define L_LEN 200
    char line[L_LEN];
	//float val;
	double *pifocVoltages;
	int err;
	double freqOffsetMHz,centerFreqMHz;
	
	//#define MAX_STEPS 100 
	//double pifocVoltages[MAX_STEPS]; // TODO
	int file; 
	tprintf("\nRunning auto focus using datapath: %s.\n",config->idleSequenceDataPath); 
	ProcessDrawEvents ();
	
	// load used voltages ...
	//sprintf(filename,"%s\\focus_voltages.txt",config->idleSequenceDataPath); // TODO filename hard coded
	//tprintf("Load pifoc voltages from idle sequence from %s.\n",filename); 
	//ProcessDrawEvents (); 
	
	//file = OpenFile (filename, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
	//if (file == -1) {
	//	tprintf ("File error: %s\n", GetFmtIOErrorString (GetFmtIOError()));
	//	return -1;
	//}
	//tprintf("\nFile loaded.\n",filename); 
	//ProcessSystemEvents (); 
	
	
	
	/*
	i = 0;
	while (ReadLine (file, line, L_LEN-1) >= 0 && i < MAX_STEPS) {
		//tprintf("\nReading line %d.\n",i); 
		//ProcessSystemEvents (); 
		
		sscanf (line, "%f",&val);
		//tprintf("\nScanned line %d.\n",i); 
		//ProcessSystemEvents (); 
		
		pifocVoltages[i] = val;
		i++;
	}
	CloseFile(file);	*/
	
	
	sprintf(filename,"%s\\000_subtr.sif",config->idleSequenceDataPath);   // TODO filename hard coded
	tprintf("Use images from %s for auto focus.\n",filename);
	ProcessDrawEvents (); 
	
	if (!FileExists (filename, 0)) {
		tprintf ("No idle sequence image found: %s\n",filename);
		return -1;
			
	}
	
	img = CCDIMAGES_new ();
	//pprintf(panelAddressing, ADDRESS_TEXTBOX, "Loading %s\n", wfm->addrTestFilename);
	if (CCDIMAGES_loadAndorSifFile (filename, img, 0) != 0) {
		return -1;	
	}
	
	// save fit array for debugging and checks
	sprintf(filename,"%s\\fit_data.txt",config->idleSequenceDataPath); // TODO filename hard coded 
	
	pifocVoltages = SEQUENCE_calculateAutoFocusValues(idleSeq->autoFocusNumImages, idleSeq->autoFocusCenterVoltage, idleSeq->autoFocusStepVoltage);
	focusVal = IMAGE_findFocus(img,pifocVoltages,idleSeq->autoFocusNumImages,filename);
	CCDIMAGES_free (img); 
	free(pifocVoltages);
	
	if (focusVal == -1) {
		tprintf("\nError: Pifoc focus voltage could not be determined (fit failed).\n");
		return -1;	
	}
	//tprintf("Successfully determined focus PIFOC voltage: %.3f.\n",focusVal);
	//ProcessDrawEvents ();
	
	// append focusVal to log file
	file = OpenFile (config->pifocFocusVoltageLogFilePath, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII);
	if (file == -1) {
		tprintf ("File error: %s\n", GetFmtIOErrorString (GetFmtIOError()));
	} else {
		err = SEQUENCE_getSlicingFrequency(idleSeq, &freqOffsetMHz, &centerFreqMHz);
		sprintf(line,"%.3f %s %s %.f %.f",focusVal,DateStr(),TimeStr(),freqOffsetMHz*1e6,centerFreqMHz*1e6);
		WriteLine(file,line,-1);
		CloseFile(file);
	}
	
	
	return focusVal;
}


void WFM_ADDR_findPhase (t_image *img, t_waveform *wfm, dPoint lastPhase, dPoint lastPhaseErr)
{
	int phaseErrorOk ;
	
	IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 0,0);
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Searching atoms...\n");
	ProcessDrawEvents();
	
//	IMAGE_findInterconnectedRegions (img, wfm->addrCalibration[WFM_ADDR_CAL_minThresh],
//								   wfm->addrCalibration[WFM_ADDR_CAL_maxThresh], 4, panelAddressing, ADDRESS_GRAPH2);

	IMAGE_findAllSingleAtoms_new (img, wfm->addrCalibration[WFM_ADDR_CAL_minThresh],
	//IMAGE_findAllSingleAtoms (img, wfm->addrCalibration[WFM_ADDR_CAL_minThresh],
								   wfm->addrCalibration[WFM_ADDR_CAL_maxThresh],
								   wfm->addrCalibration[WFM_ADDR_CAL_minSigma],
								   wfm->addrCalibration[WFM_ADDR_CAL_maxSigma]);
	
//	IMAGE_showPointListInGraph (panelAddressing, ADDRESS_GRAPH2, testlist);
//	return;							   
								   
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Found %i atoms.\n", ListNumItems (img->listOfAtoms));
	ProcessDrawEvents();
	phaseErrorOk = 1;
	if (wfm->addrCalibration[WFM_ADDR_CAL_phaseErrThreshold] > 0) {
		phaseErrorOk  = (img->gridPhiErr[0] < wfm->addrCalibration[WFM_ADDR_CAL_phaseErrThreshold])
					 && (img->gridPhiErr[1] < wfm->addrCalibration[WFM_ADDR_CAL_phaseErrThreshold]);
		if (!phaseErrorOk) {
			pprintf(panelAddressing, ADDRESS_TEXTBOX, "Phase error too large. ");
		}
	}
	
	if ((IMAGE_findPhase (img) == 1) || (!phaseErrorOk)) { 
		img->gridPhi[0] = lastPhase.x;	
		img->gridPhi[1] = lastPhase.y;	
		img->gridPhiErr[0] = lastPhaseErr.x;
		img->gridPhiErr[1] = lastPhaseErr.y;
		pprintf(panelAddressing, ADDRESS_TEXTBOX, "Keeping phases from previous shot: ");
		
	}
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "phi1= %1.3f +- %1.3f,  phi2= %1.3f +- %1.3f\n", 
			img->gridPhi[0], img->gridPhiErr[0], img->gridPhi[1], img->gridPhiErr[1]);
			
	IMAGE_showAtomsInGraph (panelAddressing, ADDRESS_GRAPH2, img);
	IMAGE_showAtomsInGraph (panelAddressing, ADDRESS_GRAPH1, img);

	IMAGE_showLattice (panelAddressing, ADDRESS_GRAPH1, img);
}



dPoint WFM_ADDR_calculateAddrVoltageOffset (t_image *img, t_waveform *wfm)
{
	t_atomData peak;
	dPoint Uaddr, UaddrErr, Udiff;
	int nPeaks;
	
	
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "----Fitting addressing beam picture...\n");
	IMAGE_findMinMax (img);
	//IMAGE_findAllSingleAtoms (img, img->max * 0.8, img->max * 1.4, 1.5, 7); // use the old single atom find routine here as the new one had some problems with these images ...
	IMAGE_fitSingleBeam (img, 1.0, 10);           //Changed from 1.5 to 1.0
	IMAGE_showAtomsInGraph (panelAddressing, ADDRESS_GRAPH3, img);
	
	nPeaks = ListNumItems (img->listOfAtoms);
	pprintf(panelAddressing, ADDRESS_TEXTBOX2, "%d peaks found.", nPeaks);
	
	if (nPeaks == 1) {
		// ok:
		ListGetItem (img->listOfAtoms, &peak, 1);
		pprintf(panelAddressing, ADDRESS_TEXTBOX2, " center = (%1.2f,%1.2f)\n", peak.x, peak.y);
		Uaddr = WFM_ADDR_imageToCtrlVoltage (wfm, MakeDPoint (peak.x, peak.y), &UaddrErr);
		Udiff.x = wfm->addrCalibration[WFM_ADDR_CAL_U10] - Uaddr.x;
		Udiff.y = wfm->addrCalibration[WFM_ADDR_CAL_U20] - Uaddr.y;
		pprintf (panelAddressing, ADDRESS_TEXTBOX2, "U = %1.3f +- %1.3f, %1.3f +- %1.3f\n", Uaddr.x, UaddrErr.x, Uaddr.y, UaddrErr.y);
		pprintf (panelAddressing, ADDRESS_TEXTBOX2, "difference: U1 = %1.3f  U2 = %1.3f\n", Udiff.x, Udiff.y);
		return Udiff;
		
	}
		
	return MakeDPoint (0,0);
	

}

/*

void WFM_ADDR_findPhase2 (t_image *img, t_waveform *wfm)
{
	IMAGE_findMinMax (img);
	
	IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 0,0);
	
	IMAGE_findAllSingleAtoms (img, img->max * 0.8, img->max * 1.4, 1.5, 7);
	IMAGE_findPhase (img, 1);    

	pprintf(panelAddressing, ADDRESS_TEXTBOX2, "phase: phi1= %1.3f +- %1.3f,  phi2= %1.3f +- %1.3f\n", 
			img->gridPhi[0], img->gridPhiErr[0], img->gridPhi[1], img->gridPhiErr[1]);
	
	IMAGE_showAtomsInGraph (panelAddressing, ADDRESS_GRAPH3, img);

}
*/


//
// test position feedback from addressing beam
//
int WFM_ADDR_showTestImageFile2 (t_sequence *seq, t_waveform *wfm)
{
	
	t_ccdImages *positionImg;
	
	
	if (wfm->addrTestFilename2 == NULL) return 0;
	if (panelAddressing == 0) {
		panelAddressing = LoadPanel (0, UIR_File, ADDRESS);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH2,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH_voltages,ATTR_REFRESH_GRAPH,0);  
	}
	DisplayPanel (panelAddressing);
	SetPanelAttribute (panelAddressing, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	
	
	positionImg = CCDIMAGES_new ();
	pprintf(panelAddressing, ADDRESS_TEXTBOX2, "Loading %s\n", wfm->addrTestFilename2);
	ProcessDrawEvents();
	
	if (CCDIMAGES_loadAndorSifFile (wfm->addrTestFilename2, positionImg, 0) != 0) {
		return -1;
	}
	tprintf("SIF file loaded\n");
	IMAGE_free (imgPos);
	imgPos = IMAGE_new ();
	
	IMAGE_assignFromCCDimage  (imgPos, positionImg, 0, -1, 0, 0, 0);
//	WFM_ADDR_showImages (imgPos, wfm, imageSize);
	
	
	tprintf("Showing SIF file in graph\n");
	IMAGE_showInGraph (panelAddressing, ADDRESS_GRAPH3, imgPos, "TEST");
	if (imgWfmPos == NULL) {
		imgWfmPos = WFM_new (0);
		WFM_init (imgWfmPos, WFM_TYPE_ADDRESSING);
	}
	memcpy (imgWfmPos->addrCalibration, wfm->addrCalibration, sizeof (double) * WFM_ADDR_N_CALIBRATION_PARAMETERS);

	// identify single atom in addressing beam
	
	tprintf("WFM_ADDR_calculateAddrVoltageOffset running ...\n");
	ProcessDrawEvents();
	WFM_ADDR_calculateAddrVoltageOffset (imgPos, imgWfmPos);
	
//	WFM_ADDR_findPhase2 (imgPos, imgWfmPos);
	
	CCDIMAGES_free (positionImg);
	
	RefreshGraph (panelAddressing, ADDRESS_GRAPH1);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH2);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH_voltages);
	return 0;
}




int WFM_ADDR_showTestImageFile (t_sequence *seq, t_waveform *wfm)
{

	t_ccdImages *rawImg;
	ListType pointList;
	t_waveform *wfm2;
	int h1 = 0;
	int h2 = 0;
	char hstr[500];
	dPoint Uoffset;
	float focusMeasure;
	int i;
	//int isVisible;

	if (wfm->addrTestFilename == NULL) return 0;
	if (panelAddressing == 0) {
		panelAddressing = LoadPanel (0, UIR_File, ADDRESS);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH2,ATTR_REFRESH_GRAPH,0);
		SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH_voltages,ATTR_REFRESH_GRAPH,0);  
	}

	//DisplayPanel (panelAddressing);

//
//     test addressing
//	
	wfm2 = WFM_ADDR_createWaveform (seq, wfm, 0, 0);
    h1 = WFM_plot(panelAddressing, ADDRESS_GRAPH_voltages, seq, wfm, NULL, 0);
	
	sprintf (hstr, "U1 - %s (%s)", str_AnalogChannelNames (wfm->channel), SEQUENCE_DACchannelName (seq, wfm->channel));
	hstr[29] = 0;
	if (h1 != 0) SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h1, ATTR_PLOT_LG_TEXT, hstr);
	//DisplayPanel (panelAddressing);
	
	
	if (wfm2 != NULL) {
	    h2 = WFM_plot(panelAddressing, ADDRESS_GRAPH_voltages, seq, wfm2, NULL, 1);
		sprintf (hstr, "U2 - %s (%s)", str_AnalogChannelNames (wfm2->channel), SEQUENCE_DACchannelName (seq, wfm2->channel));
	}
	hstr[29] = 0;
	if (h2 != 0) {
		SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2, ATTR_PLOT_LG_TEXT, hstr);
		SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2,
						  ATTR_TRACE_COLOR, VAL_GREEN);
	}	
	rawImg = CCDIMAGES_new ();
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Loading %s\n", wfm->addrTestFilename);
	ProcessDrawEvents();
	if (CCDIMAGES_loadAndorSifFile (wfm->addrTestFilename, rawImg, 0) != 0) {
		tprintf("Image loading failed.\n");
		return -1;
	}
	IMAGE_free (img);
	img = IMAGE_new ();
	if (imgWfm == NULL) {
		imgWfm = WFM_new (0);
		WFM_init (imgWfm, WFM_TYPE_ADDRESSING);
	}
	memcpy (imgWfm->addrCalibration, wfm->addrCalibration, sizeof (double) * WFM_ADDR_N_CALIBRATION_PARAMETERS);
	
	IMAGE_assignFromCCDimage  (img, rawImg, 0, -1, 0, 0, 0);
	WFM_ADDR_showImages (img, wfm, wfm->addrImageSize, "TEST");
	WFM_ADDR_findPhase (img, wfm, MakeDPoint (0,0), MakeDPoint (0,0));
	IMAGE_printAllSingleAtoms (panelAddressing, ADDRESS_TEXTBOX, img);
	
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Focussing test with %d images\n", rawImg->nImages);
	
	// Focusing test
	for (i = 0; i < rawImg->nImages; i++) {
		focusMeasure = IMAGE_focusMeasure (rawImg,i);
		pprintf (panelAddressing, ADDRESS_TEXTBOX, "Focus measure in image %d: %1.1f\n",i, focusMeasure); 
	}

	
	
	// feedback from addressing beam
//	if ()
		
	
	IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 
							 img->gridPhi[0] + wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi1],
							 img->gridPhi[1] + wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi2]);
	
	
	// plot U1, U2 with phase correction
	wfm2 = WFM_ADDR_createWaveform (seq, wfm, img->gridPhi[0], img->gridPhi[1]);

	h1 = WFM_plot(panelAddressing, ADDRESS_GRAPH_voltages, seq, wfm, NULL, 1);
	sprintf (hstr, "U1 with comp. - %s", str_AnalogChannelNames (wfm->channel));
	hstr[29] = 0;
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h1, ATTR_PLOT_LG_TEXT, hstr);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h1, ATTR_PLOT_LG_TEXT, hstr);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h1,
					  ATTR_TRACE_COLOR, VAL_ORANGE);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h1,
					  ATTR_LINE_STYLE, VAL_DASH);
	//DisplayPanel (panelAddressing);

	if (wfm2 != NULL) {
	    h2 = WFM_plot(panelAddressing, ADDRESS_GRAPH_voltages, seq, wfm2, NULL, 1);
		sprintf (hstr, "U2 with comp - %s", str_AnalogChannelNames (wfm2->channel));
	}
	hstr[29] = 0;
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2, ATTR_PLOT_LG_TEXT, hstr);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2,
					  ATTR_TRACE_COLOR, VAL_DK_GREEN);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2,
					  ATTR_LINE_STYLE, VAL_DASH);
	SetPlotAttribute (panelAddressing, ADDRESS_GRAPH_voltages, h2,
					  ATTR_PLOT_THICKNESS, 1);


	// calculate voltage offset
	Uoffset = WFM_ADDR_voltageOffsetFromPhaseDifference (wfm, img, wfm->addrCenter);
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Voltage offsets: U1=%1.3f, U2=%1.3f\n", Uoffset.x, Uoffset.y);

/*	Uoffset = WFM_ADDR_voltageOffset (wfm, img, MakePoint(wfm->addrCenter.x-100, wfm->addrCenter.y-100));
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Voltage offsets: U1=%1.3f, U2=%1.3f\n", Uoffset.x, Uoffset.y);

	Uoffset = WFM_ADDR_voltageOffset (wfm, img, MakePoint(wfm->addrCenter.x+100, wfm->addrCenter.y+100));
	pprintf(panelAddressing, ADDRESS_TEXTBOX, "Voltage offsets: U1=%1.3f, U2=%1.3f\n", Uoffset.x, Uoffset.y);
*/
	ProcessDrawEvents();
	
	// plot points with and without feedback

	switch (wfm->addrType) {
		case WFM_ADDR_TYPE_SINGLESITE:
		case WFM_ADDR_TYPE_DMD:  
			pointList = WFM_ADDR_convertMatrixToImageCoords (wfm, img, 0);  // just use first matrix here.
			WFM_ADDR_showPointListInGraph (panelAddressing, ADDRESS_GRAPH1, pointList, VAL_SOLID_CIRCLE, VAL_LT_BLUE,1);
			WFM_ADDR_showPointListInGraph (panelAddressing, ADDRESS_GRAPH2, pointList, VAL_SIMPLE_DOT, VAL_LT_BLUE,0);
			ListDispose (pointList);  
			break;
		case WFM_ADDR_TYPE_TRAJECTORY:
			pointList = WFM_ADDR_trajCalculateImageCoords (wfm, img);
			WFM_ADDR_showLineInGraph (panelAddressing, ADDRESS_GRAPH1, pointList, VAL_LT_BLUE);
			WFM_ADDR_showLineInGraph (panelAddressing, ADDRESS_GRAPH2, pointList, VAL_LT_BLUE);
			ListDispose (pointList);  
			break;
	}

	

//	IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
//							 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
//							 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 0,0);
//	pointList = WFM_ADDR_convertMatrixToImageCoords (wfm, img); 
//	WFM_ADDR_showPointListInGraph (panelAddressing, ADDRESS_GRAPH1, pointList, VAL_SIMPLE_DOT, VAL_GREEN,1);
//	ListDispose (pointList);
	CCDIMAGES_free (rawImg);

	
	
	
//    
// 	
	
	
	
//	WFM_ADDR_showWaveforms (panelAddressing, ADDRESS_GRAPH1, pointList);

	RefreshGraph (panelAddressing, ADDRESS_GRAPH1);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH2);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH_voltages);
	
	DisplayPanel (panelAddressing);
	SetPanelAttribute (panelAddressing, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	
	//GetPanelAttribute (panelAddressing,ATTR_VISIBLE,&isVisible);
	//if (!isVisible) {
	//	DisplayPanel (panelAddressing); 
	//}
	//SetPanelAttribute (panelAddressing,ATTR_VISIBLE,1); 
	
	return 0;
}



void WFM_ADDR_setMatrixValuesRect (int panel, int table, t_waveform *w, Rect selection, int value, int matrixNum)
{
	int i, j;
	
	for (i = selection.left; i < selection.left+selection.width; i++) {
		for (j = selection.top; j < selection.top+selection.height; j++) {
			WFM_ADDR_setMatrixValue (w, matrixNum, i-1, j-1, value);
		}
	}
}


	 
void WFM_ADDR_shiftMatrixValues (t_waveform *w, int matrixNum, int dx, int dy)
{
	int i;
	dy = -dy;
	dx = -dx;
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {	  // *WFM_ADDR_MATRIX_NUM
		if (dy > 0) {
			w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i] = w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i] << ((unsigned int)dy);    
		} else if (dy < 0) {
			w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i] = w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i] >> ((unsigned int)-dy);    
		}
	}
	if (dx > 0) {
		for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {          
			if (i+dx < 0) {
				w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i] = 0;
			} else {
				w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i]=w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+(i+dx)];
			}
		}
	} else if (dx < 0) {
		for (i = WFM_ADDR_MATRIXDIM-1; i >= 0; i--) {      
			if (i+dx >= WFM_ADDR_MATRIXDIM) {
				w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i]=0; 
			} else {
		    	w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+i]=w->addrMatrix[matrixNum*WFM_ADDR_MATRIXDIM+(i+dx)];
			}
		}
	}
	
	
}   



int CVICALLBACK WFM_ADDR_CHECKBOX_stepMatrix_clicked_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_TABLE_clicked_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Point mouse;
	Point p;
	int value;
	t_waveform *w;
	Rect selection;
	int matrixNum;
	GetCtrlVal(panel,ADDR_SS_NUMERIC_matrixNum,&matrixNum);
	
	switch (event)
	{
		case EVENT_COMMIT:
			w = activeWfm ();
			WFM_ADDR_tableToMatrix (panel, control, w, matrixNum);
			
			setChanges (activeSeq(), 1); 
			
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    GetTableCellFromPoint (panel, control, mouse, &p); 
			if (TABLE_isCellYesNo (panel, control, p)) {
				if (!TABLE_isCellDimmed (panel, control, p)) {
					TABLE_getCellValueYesNo2 (panel, control, p, &value);
					TABLE_setCellValueYesNo2 (panel, control, p, !value);
					w = activeWfm ();  			
					WFM_ADDR_tableToMatrix (panel, control, w, matrixNum);
					setChanges (activeSeq(), 1); 
				}
			}
			
			break;
			
		case EVENT_LEFT_CLICK:    			
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    GetTableCellFromPoint (panel, control, mouse, &p); 
			return 0;
		case EVENT_RIGHT_CLICK:
			GetTableSelection (panel, control, &selection);
			w = activeWfm ();  
			switch (RunPopupMenu (menuWfmAddrTable, MENU9_ADDR,
								   panel, eventData1, eventData2, 0, 0, 0, 0)) {
				case MENU9_ADDR_CLEAR:
					WFM_ADDR_setMatrixValuesRect (panel, control, w, selection, 0, matrixNum);
					WFM_ADDR_matrixToTable (panel, control, w, matrixNum);
					setChanges (activeSeq(), 1); 
					break;
				case MENU9_ADDR_SELECT:
					WFM_ADDR_setMatrixValuesRect (panel, control, w, selection, 1, matrixNum);
					WFM_ADDR_matrixToTable (panel, control, w, matrixNum);
					setChanges (activeSeq(), 1); 
					break;
					
			}
			return 1;
			
			
	}
	return 0;
}




int CVICALLBACK WFM_ADDR_showInImage_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_SS_getParameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned long thelp;
	double dTime;
	t_waveform *wfm;
	
	switch (event)
	{
	    case EVENT_KEYPRESS:
			if (eventData1 == ',') {
				FakeKeystroke ('.');
			}
			break;
		case EVENT_COMMIT:
			
			wfm = activeWfm();
			GetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_duration, &dTime);
	        thelp = timeToInt (dTime);   
	        if (thelp != wfm->durationFrom_50ns) wfm->durationFrom_50ns = thelp;
			wfm->digParameters.timebase_50ns = 0;
			
			
			GetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_CHECKBOX_stepMatrix, &wfm->stepAddrMatrix);
			GetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_picsPerRep, &wfm->addrMatricesPerRepetition);
			
			setChanges (activeSeq(), 1);	
			break;		
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_getParameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	Point mouse;
	Point cell;
	int value;
//	int addrType;
	
	//WFM_ADDR_SS_getParameters_CB (panel,control,event,callbackData,eventData1,eventData2);

	
	switch (event)
	{
	    case EVENT_KEYPRESS:
			if (eventData1 == ',') {
				FakeKeystroke ('.');
			}
			break;
		case EVENT_COMMIT:
			wfm = activeWfm();  
			
			WFM_ADDR_getCalibration (subPanelWFMAddr, WFM_ADDR_TABLE_calibration, wfm);

			if (wfm->addrType == WFM_ADDR_TYPE_DMD && wfm->addrDMDimage != NULL) {
				GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_enableDMD, &wfm->addrDMDimage->enableDMD);  
				GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDfeedback, &wfm->addrDMDimage->enableDMDPhaseFeedback); 
				GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDerrdiff, &wfm->addrDMDimage->enableDMDErrorDiffusion); 
				GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_CHECKBOX_DMDsmoothing, &wfm->addrDMDimage->enableDMDSmoothing);
			}
			
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_RING_link, &wfm->addrLinkWfm);
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_centerX, &wfm->addrCenter.x);
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_centerY, &wfm->addrCenter.y);
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_centerY, &wfm->addrCenter.y);
			
            GetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_enableFeedbk, &wfm->addrEnablePhaseFeedback);
            GetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_enableFeedb2, &wfm->addrEnablePhaseFeedback2);
            GetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_camera, &wfm->addrCamera);
            
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_addOffset, &wfm->addrAddLastOffset);
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_CHECKBOX_addOffsetNo1, &wfm->addrAddLastOffsetNotAtFirst);
			GetCtrlVal (subPanelWFMAddr, WFM_ADDR_NUMERIC_imageSize, &wfm->addrImageSize);
			
			GetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_CHECKBOX_stepMatrix, &wfm->stepAddrMatrix);
			GetCtrlVal (subPanelWFMAddrSingleSite, ADDR_SS_NUMERIC_picsPerRep, &wfm->addrMatricesPerRepetition); 
			
			setChanges (activeSeq(), 1);
			WFM_show (activeSeq(), activeWfmNo());   
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			if (control == WFM_ADDR_TABLE_calibration) {
				ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
								 &eventData1);
				mouse.x = eventData2 + ctrlLeft(panel, control);
				mouse.y = eventData1 + ctrlTop(panel, control);
			    GetTableCellFromPoint (panel, control, mouse, &cell); 
				TABLE_changeRow (panel, control, cell, 0);
				if (TABLE_isCellYesNo (panel, control, cell)) {
					TABLE_getCellValueYesNo (panel, control, cell, &value);
					TABLE_setCellValueYesNo (panel, control, cell, !value);
					CallCtrlCallback (panel, control, EVENT_COMMIT, cell.y, cell.x, 0);
					wfm = activeWfm ();
					wfm->digParameters.timebase_50ns = 0;
				}
			}
			break;
		case EVENT_ACTIVE_TAB_CHANGE:
			wfm = activeWfm();
			if (wfm != NULL && wfm->addrType == WFM_ADDR_TYPE_DMD && wfm->addrDMDimage != NULL) {
				WFM_ADDR_DMD_displayShapeInGraph(wfm,activeSeq());  	
			}
			break;
			
	}
	return 0;
}

int CVICALLBACK WFM_ADDR_selectTestFile_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	t_waveform *wfm;
	
	switch (event)
	{
		case EVENT_COMMIT:
			if (FileSelectPopup (config->dataPathToday, "*.sif", "*.sif",
							 "Select SIF file", VAL_LOAD_BUTTON, 0, 1, 1, 0,
							 filename) == VAL_EXISTING_FILE_SELECTED) {
			
				wfm = activeWfm();
				if (wfm != NULL) wfm->addrTestFilename = strnewcopy (wfm->addrTestFilename, filename);
				setChanges (activeSeq(), 1);
				WFM_show (activeSeq(), activeWfmNo ()); 
			}
			break;
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_selectTestFile2_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char filename[MAX_PATHNAME_LEN];
	t_waveform *wfm;
	
	switch (event)
	{
		case EVENT_COMMIT:
			if (FileSelectPopup (config->dataPathToday, "*.sif", "*.sif",
							 "Select SIF file", VAL_LOAD_BUTTON, 0, 1, 1, 0,
							 filename) == VAL_EXISTING_FILE_SELECTED) {
			
				wfm = activeWfm();
				if (wfm != NULL) wfm->addrTestFilename2 = strnewcopy (wfm->addrTestFilename2, filename);
				setChanges (activeSeq(), 1);
				WFM_show (activeSeq(), activeWfmNo ()); 
			}
			break;
	}
	return 0;
}



int CVICALLBACK WFM_ADDR_showImageFile_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	
	switch (event)
	{
		case EVENT_COMMIT:
			wfm = activeWfm();
			WFM_ADDR_showTestImageFile (activeSeq(), wfm);
			if (wfm->addrTestFilename == NULL) {
				return 0;
			}
			break;
	}
	return 0;
}



int CVICALLBACK WFM_ADDRESS_GRAPH1_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double x, y;
	dPoint U, Uerr;
	
//	DebugPrintf ("%s\n", eventStr (event,eventData1,eventData2));

	switch (event)
	{
		case EVENT_VAL_CHANGED:
			GetGraphCursor (panel, control, 1, &x, &y);
			U = WFM_ADDR_imageToCtrlVoltage (imgWfm, MakeDPoint (x,y), &Uerr); 
			pprintf (panelAddressing, ADDRESS_TEXTBOX, "[x = %1.1f, y = %1.1f], value = %1.1f,  U1 = %1.4f ± %1.4f,   U2 = %1.4f ± %1.4f\n",
					 x, y, IMAGE_getValue (img, x,y), U.x, Uerr.x, U.y, Uerr.y);
			break;
	}
	return 0;
}



int CVICALLBACK WFM_ADDR_test (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	dPoint p;
	double U1, U2;
	t_waveform *wfm;

	switch (event)
	{
		case EVENT_COMMIT:
			wfm = activeWfm();
			if (wfm == 0) return 0;
			if (wfm->type != WFM_TYPE_ADDRESSING) return 0;
			for (U1 = wfm->addrCalibration[WFM_ADDR_CAL_U10]-1.5; U1 < wfm->addrCalibration[WFM_ADDR_CAL_U10]+1.5; U1+= 0.02) {
				p = WFM_ADDR_ctrlVoltageToImage (wfm, U1, wfm->addrCalibration[WFM_ADDR_CAL_U20]);
				PlotPoint (panelAddressing, ADDRESS_GRAPH1, p.x, p.y, VAL_BOLD_X,
						   VAL_WHITE);
				PlotPoint (panelAddressing, ADDRESS_GRAPH2, p.x, p.y,
						   VAL_SIMPLE_DOT, VAL_WHITE);
			}

			for (U2 = wfm->addrCalibration[WFM_ADDR_CAL_U20]-1.5; U2 < wfm->addrCalibration[WFM_ADDR_CAL_U20]+1.5; U2+= 0.02) {
				p = WFM_ADDR_ctrlVoltageToImage (wfm, wfm->addrCalibration[WFM_ADDR_CAL_U10], U2);
				PlotPoint (panelAddressing, ADDRESS_GRAPH1, p.x, p.y, VAL_BOLD_X,
						   VAL_WHITE);
				PlotPoint (panelAddressing, ADDRESS_GRAPH2, p.x, p.y, VAL_SIMPLE_DOT,
						   VAL_WHITE);
			}

			break;
	}
	RefreshGraph (panelAddressing, ADDRESS_GRAPH1);
	RefreshGraph (panelAddressing, ADDRESS_GRAPH2); 
	//SetCtrlAttribute (panelAddressing, ADDRESS_GRAPH1,ATTR_REFRESH_GRAPH,1);
	return 0;
}

int CVICALLBACK WFM_ADDR_testPositionFeedback_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	
	switch (event)
	{
		case EVENT_COMMIT:
			wfm = activeWfm();
			WFM_ADDR_showTestImageFile2 (activeSeq(), wfm);
			if (wfm->addrTestFilename == NULL) return 0;
			break;
	}
	return 0;
}


void WFM_ADDR_writeDataToProtocolFile (char *filename, char *imageFilename, int repetition, dPoint phase, dPoint phaseErr, dPoint Uoffs, dPoint UAddrOffs, dPoint UTotal, dPoint lastPhaseOffset)
{
	int fileExists;
	int file;
	char line[400];

	fileExists = FileExists (filename, 0);
	file = OpenFile (filename, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII);
	if (file == -1) {
		//pprintf (panelAddressing, ADDRESS_TEXTBOX, "File error: %s\n", GetFmtIOErrorString (GetFmtIOError()));
		displayFileError (filename); 
		return;
	}
	if (!fileExists) {
		WriteLine (file, "repetition \tfilename \tphi1 \tdPhi1 \tphi2 \tdPhi2 \tphiOffset1 \tPhiOffset2 \tsumPhi1 \tsumPhi2 \tUoffset1 \tUoffset2 \tUAddrOffset1 \tUAddrOffset2 \tUSum1 \tUSum2",
				   -1);
	}
    // 				 1	  2   3		 4		  5		 6		  7		  8			9		10		 11		  12	 13		  14		15		 16		  17	   18
	sprintf (line, "%d \t%s %1.3f \t%1.3f \t%1.3f \t%1.3f \t%1.3f  \t%1.3f  \t%1.3f  \t%1.3f  \t%1.4f  \t%1.4f  \t%1.4f  \t%1.4f  \t%1.4f  \t%1.4f",
			 repetition, 
			 extractFilename (imageFilename),
	//		 ListNumItems (img->listOfAtoms),
			 
			 phase.x, //img->gridPhi[0], 
			 phaseErr.x, //img->gridPhiErr[0], 
			 phase.y, //img->gridPhi[1], 
			 phaseErr.y, //img->gridPhiErr[1],
			 lastPhaseOffset.x,
			 lastPhaseOffset.y,
			 phase.x + lastPhaseOffset.x,
			 phase.y + lastPhaseOffset.y,
			 Uoffs.x, Uoffs.y,
			 UAddrOffs.x, UAddrOffs.y,
			 UTotal.x, UTotal.y
			 );
			 
	WriteLine (file, line, -1);	
	CloseFile (file);
}



void WFM_ADDR_writePointListToFile (char *filename, ListType pointList)
{
	int file;
	char line[100];
	dPoint p;
	int i;
	
	file = OpenFile (filename, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);	
	if (file == -1) {
		displayFileError(filename);
		return;
	}
	for (i = 1; i <= ListNumItems (pointList); i++) {
		ListGetItem (pointList, &p, i);
		sprintf (line, "%1.2f \t%1.2f", p.x, p.y);
		WriteLine (file, line, -1);	
	}
	
	CloseFile (file);	
}

int CVICALLBACK WFM_ADDR_getParameters_entropy (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


int CVICALLBACK TESTfunction (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}


//=====================================================================

//                 DMD

//=====================================================================


int CVICALLBACK WFM_DMD_transmitToDMD_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int repetition;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_NUMERIC_repetition, &repetition);
			DMDIMAGE_THREAD_writeCommandFile (activeSeq(), activeWfm(), repetition-1);
			break;
	}
	return 0;
}

int CVICALLBACK WFM_DMD_transmitSeriesToDMD_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int repetition;
	t_waveform *wfm;
	t_sequence *seq;
	int i;
	
	switch (event)
	{
		case EVENT_COMMIT:
			wfm = activeWfm();
			seq = activeSeq();
			if (wfm == NULL || seq == NULL) return 0;
			
			GetCtrlVal (subPanelWFMAddrDMD, ADDR_DMD_NUMERIC_repetition, &repetition);
			SetCtrlAttribute(subPanelWFMAddrDMD, ADDR_DMD_BUTTON_transmitseries, ATTR_DIMMED, 1);
			tprintf ("Playing DMD series (%d repetitions)\n", seq->nRepetitions);
			ProcessDrawEvents();
			for (repetition = 0; repetition < seq->nRepetitions; repetition++) {
				tprintf ("Repetition %d:\n", repetition+1);
				DMDIMAGE_THREAD_writeCommandFile (seq, wfm, repetition);
				for (i=0; i <100; i++) {  // wait 5 seconds. 
					ProcessSystemEvents ();
					Sleep(50);
				}
				
			}
			SetCtrlAttribute(subPanelWFMAddrDMD, ADDR_DMD_BUTTON_transmitseries, ATTR_DIMMED, 0); 
			break;
	}
	return 0;
}





int CVICALLBACK WFM_DMD_attributeChanged_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *w;
	int ihelp;
	t_sequence *seq;
	t_DMDshape *s;
	int i;
	
	seq = activeSeq();
	w = activeWfm ();
	
	if ((event == EVENT_KEYPRESS) && (eventData1 == ',')) FakeKeystroke ('.');  
	switch (event)
	{
		case EVENT_COMMIT:
			if (w->addrDMDimage == NULL) return 0;
			GetCtrlVal (panel, control, &ihelp);
			if (ihelp != w->addrDMDimage->shapeID) {
				w->addrDMDimage->shapeID = ihelp;
				s = DMDSHAPE_ptr (w->addrDMDimage->shapeID);
				if (w->addrDMDimage->shapeParametersFrom == NULL) {
					DMDIMAGE_allocateMemory (w->addrDMDimage);
					for (i = 0; i < s->nParameters; i++) {
						w->addrDMDimage->shapeParametersFrom[i] = s->defaultValues[i];	
					}
				}
            	WFM_show (seq, activeWfmNo());
				setChanges (seq, 1);
			}

			break;
	}
	return 0;
}






int CVICALLBACK WFM_ADDR_DMD_changeRepetitionFunc (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			WFM_ADDR_DMD_displayShapeInGraph (activeWfm(), activeSeq());
			break;
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_DMD_TABLE_parameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	t_sequence *seq;
	Point mouse;
	Point cell;
	int value;
	
	switch (event)
	{
		case EVENT_KEYPRESS:
			switch (eventData1) {
				case ',':  FakeKeystroke ('.'); break;
			}
			break;
		case EVENT_COMMIT:
			wfm = activeWfm ();
			seq = activeSeq();
			if (wfm == NULL) return 0;
			WFM_ADDR_DMD_getParametersFromTable (panel, control, wfm->addrDMDimage);
			WFM_show (activeSeq (), activeWfmNo());
			PARAMETERSWEEPS_updateDisplay (seq);		
			setChanges (seq, 1);
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    GetTableCellFromPoint (panel, control, mouse, &cell); 
			if (cell.x == 0 || cell.y == 0){
				return 0;	
			}
			TABLE_changeRow (panel, control, cell, 0);
			if (TABLE_isCellYesNo (panel, control, cell)) {
				TABLE_getCellValueYesNo (panel, control, cell, &value);
				TABLE_setCellValueYesNo (panel, control, cell, !value);
				CallCtrlCallback (panel, control, EVENT_COMMIT, cell.y, cell.x, 0);
				//wfm = activeWfm ();
			}
			break;
	}
	return 0;
}


int CVICALLBACK WFM_ADDR_changeMatrix (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int matrixNum;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, ADDR_SS_NUMERIC_matrixNum, &matrixNum);
			WFM_ADDR_matrixToTable (panel, ADDR_SS_TABLE_SINGLESITE, activeWfm(), matrixNum);
			WFM_ADDR_showDisorderPara(panel,activeWfm(), matrixNum);
			break;
	}
	return 0;
}

int CVICALLBACK WFM_ADDR_shift (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int matrixNum;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, ADDR_SS_NUMERIC_matrixNum, &matrixNum);
			switch (control)
			{
				case ADDR_SS_COMMANDBUTTON_up:
					WFM_ADDR_shiftMatrixValues (activeWfm(),matrixNum, 0, 1);
					break;
				case ADDR_SS_COMMANDBUTTON_down:   
					WFM_ADDR_shiftMatrixValues (activeWfm(),matrixNum, 0, -1);
					break;
				case ADDR_SS_COMMANDBUTTON_left:      
					WFM_ADDR_shiftMatrixValues (activeWfm(),matrixNum, -1, 0);
					break;
				case ADDR_SS_COMMANDBUTTON_right:      
					WFM_ADDR_shiftMatrixValues (activeWfm(),matrixNum, 1, 0);
					break;
			
			}
			WFM_ADDR_matrixToTable (panel, ADDR_SS_TABLE_SINGLESITE, activeWfm(), matrixNum);
			WFM_ADDR_showDisorderPara(panel,activeWfm(), matrixNum);
			break;
	}
	return 0;
}

int CVICALLBACK WFM_ADDR_changeDisorder (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned int disorderSeed;
	int disorderEnable;
	t_waveform *wfm;
	t_sequence *seq;
	int matrixNum, disorderTyp;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, ADDR_SS_RING_disorder,&disorderTyp);
			GetCtrlVal(panel, ADDR_SS_NUMERIC_matrixNum, &matrixNum);
			GetCtrlVal(panel, ADDR_SS_CHECKBOX_disorder, &disorderEnable);
			GetCtrlVal(panel, ADDR_SS_NUMERIC_disorder, &disorderSeed);
			wfm = activeWfm ();
			seq = activeSeq();
			if (wfm->addDisorder[matrixNum]!=disorderEnable | wfm->disorderSeed[matrixNum]!=disorderSeed | wfm->disorderTyp[matrixNum]!=disorderTyp){
				setChanges (seq, 1);
			}
			wfm->addDisorder[matrixNum]=disorderEnable;
			wfm->disorderSeed[matrixNum]=disorderSeed;
			wfm->disorderTyp[matrixNum]=disorderTyp;
			break;
	}
	return 0;
}

int CVICALLBACK WAVEFORMS_Points_attributeChanged_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	t_sequence *seq;
	seq = activeSeq();
	wfm = activeWfm ();
	switch (event)
	{
		case EVENT_COMMIT:
			switch (control)
			{
				case WFM_wPTS_NUMERIC_timebaseStre:
					GetCtrlVal (panel, control, &wfm->timebaseStretch);
					WFM_show (seq, activeWfmNo());
					wfm->digParameters.timebase_50ns = 0;
					setChanges (seq, 1);
					break;
			}
			break;
			
	}
	return 0;
}
