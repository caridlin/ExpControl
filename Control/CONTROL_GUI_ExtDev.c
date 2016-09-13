#include <windows.h> // for  Sleep 
#include <formatio.h>
#include <utility.h>
#include "toolbox.h"
#include "tools.h"   
#include <userint.h>
#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"
#include "imageProcessing.h"


#define EXTDEV_TABLE_ROW_HEIGHT 20
enum {
	EXTDEV_TAB_INDEX_TELNET,
	EXTDEV_TAB_INDEX_unknown
};


int panelExtDevSet = 0;
int EXTDEV_activeTableRow = 2;

int CVICALLBACK EXTDEV_abort_CB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}

// Table
enum {
	EXTDEV_TABLE_COL_id = 1,
	EXTDEV_TABLE_COL_name,
	EXTDEV_TABLE_COL_type,
	EXTDEV_TABLE_COL_info,
	EXTDEV_TABLE_COL_uniqueid,

	EXTDEV_TABLE_NCols
};



int EXTDEV_nrFromTableRow (int row)
{
	t_sequence *seq;
	t_ext_device *d;
	int i;
	int n;
	
	seq = activeSeq();
	n = ListNumItems (seq->lExtDevices);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lExtDevices, &d, i);
		if (d->positionInList == row) return i;
	}
	return 0;
	
}

void EXTDEV_TABLE_setActiveRow (int row)
{
	EXTDEV_activeTableRow = row;
};

t_ext_device *EXTDEV_ptr (t_sequence *seq, int extdevID)
{
    t_ext_device *d;
    if ((extdevID > (int)ListNumItems (seq->lExtDevices)) || (extdevID < 1)) return seq->noExternalDevice;
	ListGetItem (seq->lExtDevices, &d, extdevID);
	return d;
}
 //=======================================================================
//
// 	   returns pointer to external device with unique id 'ID'
//	   if no device is found, the "no device" is returned (if seq. exists)
//
//=======================================================================
t_ext_device *EXTDEV_ptrByID (t_sequence *seq, int ID)
{
	t_ext_device *extdev;
    int i;
    if (seq == NULL) return NULL;
	if (ID==0) return seq->noExternalDevice;
	for (i = 1; i <= ListNumItems (seq->lExtDevices); i++) {
		ListGetItem (seq->lExtDevices, &extdev, i);
		if (extdev->uniqueID==ID) return extdev;
	}
	
	return seq->noExternalDevice;
}

int activeExtdevNo (void) 
{
	return EXTDEV_nrFromTableRow (EXTDEV_activeTableRow);
}

void EXTDEV_TABLE_init (int panel, int control)
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
	
	SetCtrlAttribute (panel, control, ATTR_ENABLE_POPUP_MENU, 0);
	
	for (col = 1; col <  EXTDEV_TABLE_NCols; col++) {
		switch (col) {
			case EXTDEV_TABLE_COL_id:
				type = VAL_CELL_NUMERIC;
				name = "ID";
				width = 20;
				break;
			case EXTDEV_TABLE_COL_name:
				type = VAL_CELL_STRING;
				name = "Name";
				width = 250;
				break;
			case EXTDEV_TABLE_COL_type:
				type = VAL_CELL_STRING;
				name = "Type";
				width = 50;
				break;
			case EXTDEV_TABLE_COL_info:
				type = VAL_CELL_STRING;
				name = "Info";
				width = 450;
				break;
			case EXTDEV_TABLE_COL_uniqueid:
				type = VAL_CELL_NUMERIC;
				name = "uID";
				width = 80;
				break;
		   
		}
		InsertTableColumns (panel, control, -1, 1, VAL_USE_MASTER_CELL_TYPE);

		SetTableColumnAttribute (panel, control, col, ATTR_COLUMN_WIDTH, width);
		SetTableColumnAttribute (panel, control, col, ATTR_USE_LABEL_TEXT, 1);
		SetTableColumnAttribute (panel, control, col, ATTR_LABEL_TEXT, name);
		SetTableColumnAttribute (panel, control, col, ATTR_CELL_JUSTIFY,
								 VAL_CENTER_LEFT_JUSTIFIED);

	}
}

t_ext_device *activeExtdev () 
{
    if (ListNumItems (activeSeq()->lExtDevices) == 0) return NULL;
    return EXTDEV_ptr (activeSeq(), activeExtdevNo());
    
}

int CVICALLBACK EXTDEV_done_CB (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
    t_ext_device *new;
   // int nr;
    int type;
    t_sequence *seq;
	char newName[MAX_EXT_DEV_NAME_LEN];
	char tmpPort[100];
//	int test;
	int subPanelTelnet;
	unsigned int dummy;
	
	GetPanelHandleFromTabPage (panel, EXTDEVSET_TAB_devtype, EXTDEV_TAB_INDEX_TELNET, &subPanelTelnet);

	switch (event)
	{
		case EVENT_COMMIT:
			type=EXTDEV_TYPE_TELNET;
			RemovePopup (0);
			// -----------------------------------------
	        //    create & init new external device
			// -----------------------------------------
			seq = activeSeq();
			GetCtrlVal (panel, EXTDEVSET_STRING_NAME, newName);
			EXTDEV_checkForDuplicateNameAndRename (seq, newName);
		    new = EXTDEV_new (seq);
    		EXTDEV_init (new, type);
			dummy=(unsigned int)time;
			EXTDEV_checkForDuplicateIdandChange (seq, &dummy);
			new->uniqueID=dummy; 
			tprintf("Unique ID:%d\n",new->uniqueID);
			switch (type) {
				case EXTDEV_TYPE_TELNET: 
					GetCtrlVal (subPanelTelnet, TELNET_STRING_IP, new->address);
					GetCtrlVal (subPanelTelnet, TELNET_STRING_PORT, tmpPort); 
					break;
			}
    		strcpy (new->name, newName);
			setChanges (seq, 1);
			CONFIG_displayExternalDevices();
			//SetActiveCtrl (CONFIGdev, CONFIGdev_LISTBOX_extDevices);
			break;
	}
	return 0;
}


void EXTDEV_initPanel (void) 
{
	if (panelExtDevSet > 0) return;
	panelExtDevSet = LoadPanel (0, UIR_File, EXTDEVSET);
}

char *EXTDEV_typeStr (t_ext_device *extdev)
{
	switch (extdev->deviceTyp) {
		case EXTDEV_TYPE_NONE:
			return "None";
		case EXTDEV_TYPE_TELNET:
			return "Telnet";
	}
	return "";
}


void EXTDEV_TABLE_displayEXTDEV (t_ext_device *extdev, int panel, int control)
{
	Point cell;
	int textColor;
	int bgColor;
	char uIDstring[50];
	
	if (extdev == NULL) return;
	
	if (extdev == activeExtdev()) {
		bgColor = VAL_BLUE;
		textColor = VAL_WHITE;
	}
	else {
		bgColor = VAL_WHITE;
		textColor = VAL_BLACK;
	}
	
	cell = MakePoint (EXTDEV_TABLE_COL_id, extdev->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, extdev->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);
	sprintf(uIDstring,"%d",extdev->uniqueID);
	cell = MakePoint (EXTDEV_TABLE_COL_uniqueid, extdev->positionInList);  //EXTDEV_TABLE_COL_uniqueid
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, uIDstring);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);

	
	cell = MakePoint (EXTDEV_TABLE_COL_name, extdev->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, extdev->name);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);
	
	cell = MakePoint (EXTDEV_TABLE_COL_type, extdev->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, EXTDEV_typeStr (extdev));
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);

	cell = MakePoint (EXTDEV_TABLE_COL_info, extdev->positionInList);
	SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, "extra information (soon)");
	SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
						   VAL_CENTER_RIGHT_JUSTIFIED);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR,
						   bgColor);
	SetTableCellAttribute (panel, control, cell, ATTR_TEXT_COLOR,
						   textColor);
	
	
}

int CVICALLBACK EXTDEV_newExtDev_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			EXTDEV_initPanel ();
			InstallPopup (panelExtDevSet);
			break;
	}
	return 0;
}

int EXTDEV_TABLE_sortMode (int extdevSortMode, int setFlag)
{
	static int sortMode = EXTDEV_TABLE_SORT_MODE_NAME;
	
	if (setFlag) sortMode = extdevSortMode;	
	return sortMode;
}

int CVICALLBACK extdev_TABLE_compareFunction (void *extdev1, void *extdev2)
{
	t_ext_device *w1, *w2;
	int result = 0;
//	char channelName1[MAX_PATHNAME_LEN];
//	char channelName2[MAX_PATHNAME_LEN];
	
	w1 = * ((t_ext_device **) (extdev1));
	w2 = * ((t_ext_device **) (extdev2));
	
	switch (EXTDEV_TABLE_sortMode (0, 0)) {
		case EXTDEV_TABLE_SORT_MODE_NAME:
			return CompareStrings (w1->name, 0, w2->name, 0, 0);
		case EXTDEV_TABLE_SORT_MODE_TYPE:
			if (w1->deviceTyp < w2->deviceTyp) return -1;
			if (w1->deviceTyp > w2->deviceTyp) return 1;
			return 0;									
	}
	return result;
}

void EXTDEV_addPositionInList (t_sequence *seq)
{
	int i;
	t_ext_device *d;
	int n,dummy;
	
	n = ListNumItems (seq->lExtDevices);
		
	dummy=1;
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lExtDevices, &d, i);
		if (d==seq->noExternalDevice){
		d->positionInList =0;	
		}
		else{
			
		if (seq->extdevTableSortDescending) 
			d->positionInList = n-1 - dummy + 1;
		else 
			d->positionInList = dummy;
		dummy++;
		}
	}

}

void EXTDEV_TABLE_sort (t_sequence *seq)
{
    ListType lSortedWaveforms;		 
	int i, n;
	int dummy;
	t_ext_device *extdev;
	
	EXTDEV_addPositionInList (seq);
	lSortedWaveforms = ListCopy (seq->lExtDevices);
	EXTDEV_TABLE_sortMode (seq->extdevTableSortMode, 1);
	ListQuickSort (lSortedWaveforms, extdev_TABLE_compareFunction);
	n = ListNumItems (lSortedWaveforms);
	dummy=1;
	for (i = 1; i <= n; i++) {
		ListGetItem (lSortedWaveforms, &extdev, i);
		if (extdev==seq->noExternalDevice){
		extdev->positionInList =0;	
		}
		else{
			
		if (seq->extdevTableSortDescending) 
			extdev->positionInList = n-1 - dummy + 1;
		else 
			extdev->positionInList = dummy;
		dummy++;
		}
	}
	ListDispose (lSortedWaveforms);
}


void EXTDEV_TABLE_initAllRows (int n, int panel, int control)
{
	int nRows;
	int i;
	Point cell;
	
	GetNumTableRows (panel, control, &nRows);
	if (n < nRows) DeleteTableRows (panel, control, n+1, -1);
	else if (n > nRows) InsertTableRows (panel, control, nRows+1, n-nRows, VAL_USE_MASTER_CELL_TYPE);
	for (i = nRows; i < n; i++) { 
		SetTableRowAttribute (panel, control, i+1, ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
		SetTableRowAttribute (panel, control, i+1, ATTR_ROW_HEIGHT, EXTDEV_TABLE_ROW_HEIGHT);

		cell = MakePoint (EXTDEV_TABLE_COL_id, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
		SetTableCellAttribute (panel, control, cell, ATTR_DATA_TYPE,
							   VAL_INTEGER);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_CENTER_JUSTIFIED);
		
		
 		cell = MakePoint (EXTDEV_TABLE_COL_name, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_LEFT_JUSTIFIED);

		
		cell = MakePoint (EXTDEV_TABLE_COL_type, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);

		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_LEFT_JUSTIFIED);
		

		cell = MakePoint (EXTDEV_TABLE_COL_info, i+1);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetTableCellAttribute (panel, control, cell, ATTR_CELL_JUSTIFY,
							  VAL_CENTER_CENTER_JUSTIFIED);
	
	}	

}


void EXTDEV_TABLE_displayAllExtDevs (int panel, int control, t_sequence *seq, int activeNr, int sort)
{
	int n, i;
	t_ext_device *dev;
	//t_ext_device *noDev;
	//EXTDEV_fillInfos (seq);	  //not sure if I need this

	n = ListNumItems (seq->lExtDevices);
	
	EXTDEV_TABLE_initAllRows (n-1, panel, control);
	if (sort) EXTDEV_TABLE_sort (seq);

	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lExtDevices, &dev, i);
		//tprintf("Pos. in list%d, Name:%s, ID:%d\n",i,dev->name,dev->uniqueID);
		if (seq->noExternalDevice!=dev) EXTDEV_TABLE_displayEXTDEV (dev, panel, control);
	}
	
	ProcessDrawEvents();
	CallCtrlCallback (panel, control, EVENT_TABLE_ROW_COL_LABEL_CLICK, 3, 0, 0);
}

int CVICALLBACK EXTDEV_TABLE_callback (int panel, int control, int event,
									   void *callbackData, int eventData1, int eventData2)
{
	//Rect selection;
	t_sequence *seq;
	int oldActiveExtdev;
	Point cell;
	int nr;
	t_ext_device *extdev;
	int oldSortMode;
//	Point menu;
	int runState = VAL_SELECT_STATE;

				 
	 GetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, &runState);
	 
	 switch (event) {
		case EVENT_RIGHT_CLICK:
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:
           	seq = activeSeq();
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);		
			if (cell.y > 0) {
				 oldActiveExtdev = EXTDEV_nrFromTableRow (EXTDEV_activeTableRow);
				EXTDEV_activeTableRow = cell.y;
				EXTDEV_TABLE_setActiveRow(cell.y); 
				tprintf("test: %d",cell.y);
				EXTDEV_TABLE_displayEXTDEV (EXTDEV_ptr (seq, oldActiveExtdev), panelExtDevSet, CONFIGdev_TABLE_extdev);
				EXTDEV_TABLE_displayEXTDEV (activeExtdev(), panelExtDevSet, CONFIGdev_TABLE_extdev);
				//EXTDEV_show (seq, activeWfmNo());
			}
			
			break;
		case EVENT_COMMIT:
			if (eventData1 < 1) return 0;
			nr = EXTDEV_nrFromTableRow (eventData1);			
			
			seq = activeSeq();
			extdev = EXTDEV_ptr (seq, nr);
			if (extdev == NULL) return 0;
			cell = MakePoint (eventData2, eventData1);

			break;
		case EVENT_TABLE_ROW_COL_LABEL_CLICK:
			if (eventData1 != 0) return 0;
			seq = activeSeq();
			 oldSortMode = seq->extdevTableSortMode;
			 extdev = activeExtdev();
			 if (extdev == NULL) return 0;
			switch (eventData2) {
				case EXTDEV_TABLE_COL_id:
					seq->extdevTableSortMode = EXTDEV_TABLE_SORT_MODE_ID;
					if (seq->extdevTableSortMode == oldSortMode) {
						seq->extdevTableSortDescending = !seq->extdevTableSortDescending;
					}
					EXTDEV_TABLE_sort (seq);
					EXTDEV_TABLE_setActiveRow (extdev->positionInTable);
					break;
				case EXTDEV_TABLE_COL_name:
					seq->extdevTableSortMode = EXTDEV_TABLE_SORT_MODE_NAME;
					if (seq->extdevTableSortMode == oldSortMode) {
						seq->extdevTableSortDescending = !seq->extdevTableSortDescending;
					}
					EXTDEV_TABLE_sort (seq);
					EXTDEV_TABLE_setActiveRow (extdev->positionInTable);
					break;
				case EXTDEV_TABLE_COL_type:
					seq->extdevTableSortMode = EXTDEV_TABLE_SORT_MODE_TYPE;
					if (seq->extdevTableSortMode == oldSortMode) {
						seq->extdevTableSortDescending = !seq->extdevTableSortDescending;
					}
					EXTDEV_TABLE_sort (seq);
					EXTDEV_TABLE_setActiveRow (extdev->positionInTable);
					break;
				case EXTDEV_TABLE_COL_info:
					seq->extdevTableSortMode = EXTDEV_TABLE_SORT_MODE_INFO;
					if (seq->extdevTableSortMode == oldSortMode) {
						seq->extdevTableSortDescending = !seq->extdevTableSortDescending;
					}
					EXTDEV_TABLE_sort (seq);
					EXTDEV_TABLE_setActiveRow (extdev->positionInTable);
					break;
				
				
			}
			GetTableCellFromPoint (panel, control,MakePoint (eventData2,eventData1), &cell);
			EXTDEV_TABLE_displayAllExtDevs (panel, control, seq, -1, 0);
			SetTableSelection (panel, control, MakeRect (extdev->positionInTable, 1, 1, EXTDEV_TABLE_NCols-1));
	 		break;
		
			
	 }
	
	return 0;
}

void EXTDEV_free (t_ext_device *w)
{
	
}

void EXTDEV_delete (t_sequence *seq, int nr)
{
	t_ext_device *old;
	
	ListRemoveItem (seq->lExtDevices, &old, nr);
	EXTDEV_free (old);
	free (old);
}

void EXTDEV_deleteWithReferences (t_sequence *seq, int nr)
{
	
	EXTDEV_delete (seq, nr);
	// -----------------------------------------
    //    change all links to this external devie ToDo Sebastian see WFM_deleteWithReferences
	// -----------------------------------------

	
	
}

void EXTDEV_deleteSelected (void)
{
    int nr;
    t_ext_device *extdev;
    t_sequence *seq;
    
    seq = activeSeq();

	if (ListNumItems (seq->lExtDevices)  == 0) return;
	
	// -----------------------------------------
    //    delete waveform
	// -----------------------------------------
	extdev = activeExtdev();
	if (extdev == NULL) return;
	
	if (ConfirmPopupf ("Delete external device", "Do you really want to delete the external device\n'%s'?", extdev->name) == 0) return;
	nr =  activeExtdevNo();
	EXTDEV_deleteWithReferences (seq, nr);
	// -----------------------------------------
    //    show new waveform
	// -----------------------------------------
    if (nr == ListNumItems (seq->lExtDevices)) nr --;
	WFM_edit (seq, nr, 1);			
	setChanges (seq, 1);
}

int CVICALLBACK EXTDEV_deleteExtdev_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
  	switch (event)
		{
		case EVENT_COMMIT:
		    EXTDEV_deleteSelected ();
		    setChanges (activeSeq(),1);
			EXTDEV_TABLE_displayAllExtDevs (panel,CONFIGdev_TABLE_extdev,activeSeq(), -1,1);
			break;
		}
	return 0;
}
