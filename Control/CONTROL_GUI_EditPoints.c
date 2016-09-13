#include <formatio.h>
#include <utility.h>

#include "CONTROL_DataStructure.h"
#include "CONTROL_GUI_EditPoints.h"					

#include "UIR_common.h"	



/*#define NMenuItems 12
#define VAL_TABLE_MENUITEM_INSERT_ABOVE 0
#define VAL_TABLE_MENUITEM_INSERT_BELOW 1
#define VAL_TABLE_MENUITEM_DELETE 3
#define VAL_TABLE_MENUITEM_VARTIME_ON  5
#define VAL_TABLE_MENUITEM_VARTIME_OFF 6
#define VAL_TABLE_MENUITEM_VARVALUE_ON  8
#define VAL_TABLE_MENUITEM_VARVALUE_OFF 9
#define VAL_TABLE_MENUITEM_INCTIME 11

int menuItemIDList[NMenuItems];
*/
char *stringYesNo[2] = { stringNo, stringYes};

char *stringYesNo2[2] = { stringNo2, stringYes2};

int menuTable = -1;

extern t_controls WFM_controls;

enum {
	TABLE_COL_TIME_VARY = 1,
	TABLE_COL_TIME_START,
	TABLE_COL_TIME_INCREMENT,
	TABLE_COL_VALUE_VARY,
	TABLE_COL_VALUE_START,
	TABLE_COL_VOLTAGE,
	TABLE_COL_VALUE_INCREMENT,
	TABLE_COL_STEP_REP,
	
	TABLE_NCOLUMNS
};


/*
const char *tableMenuText[NMenuItems] = {
    "Insert rows above",
    "Insert rows below",
    NULL,
    "Delete rows",
    NULL,
    "variable time ON",
    "variable time OFF",
    NULL,
    "variable value ON",
    "variable value OFF",
    NULL,
    "Increment times for selection"
};
*/


void POINTS_DEBUG_display (const char *str, t_point *p, int nPoints)
{
	int i;
	
	DebugPrintf ("%s :",str);
    for (i = 0; i < nPoints; i++) {
		DebugPrintf ("%f ", p[i].timeStart_ns);
    }
	DebugPrintf ("\n");

}




void CONTROLS_init (t_controls *c)
{
	c->panel = 0;
	c->table = 0;
	c->tableRowHeight = 25;
	c->graph = 0;
	c->repetition = 0;
	c->invalid = 0;
	c->allowNegativeTimes = 0;
	
	c->timeMultiply = 1E3;  // ms: 1E6   탎: 1E3
	
	c->valueLabel = NULL;
	c->valueLabelIncr = NULL;
	c->valueMin = -10.0;
	c->valueMax = 10.0;
	c->valueDigits = 3;
	
	c->showStepRep = 1;
	c->voltageVisible = 1;
	
	c->f = NULL;
	c->lastPoints = NULL;
	c->nLastPoints = 0;
}



void TABLE_insertRow (t_controls *c, int position)
{
	int rows;
	int columns;
	Point p;
	double value;
	
	if (c->panel < 0) return;
	
	GetNumTableRows (c->panel, c->table, &rows);
	GetNumTableColumns (c->panel, c->table, &columns);
	if (columns <= 0) {
		return;
	}
	if (rows > 0) {
		GetActiveTableCell (c->panel, c->table, &p);
		p.x = TABLE_COL_TIME_START;
		GetTableCellVal (c->panel, c->table, p, &value);
	}
	else {
		value = 0;
		position = 1;
	}
	
	InsertTableRows (c->panel, c->table, position, 1, 
					 VAL_USE_MASTER_CELL_TYPE);
	//DEBUG
	GetNumTableRows (c->panel, c->table, &rows);

	SetTableRowAttribute (c->panel, c->table, position, 
						  ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
	SetTableRowAttribute (c->panel, c->table, position, 
						  ATTR_ROW_HEIGHT, c->tableRowHeight);
					 
	
    SetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_TIME_START, position), 
    				 value);
    SetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_VALUE_VARY, position), 
    				 stringYesNo[0]);
    SetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_TIME_VARY, position), 
    				 stringYesNo[0]);
		
	POINTS_tableToGraph (c, 1);
	CallCtrlCallback (c->panel, c->table, EVENT_COMMIT, 0, 0, NULL);
	
}



//=======================================================================
//
//    auto-adjust precision of time interval 
//
//=======================================================================
void TABLE_setCellTimeDouble (int panel, int table, Point p, double dTime)
{              
	int digits;
	char help[20];
	char *point;

	setTableCellValDouble (panel, table, p, dTime);
	Fmt (help, "%s<%f", dTime);
	point = strrchr (help, '.');
	if (point == NULL) digits = 1;
	digits = strlen (help) - (point - help) - 1;
	if (digits == 3) digits = 4;
	if (digits == 5) digits = 6;
	setTableCellAttribute (panel, table, p, ATTR_PRECISION, digits);
	
/*
    if ((time % VAL_ms) == 0)
        SetTableCellAttribute (panel, table, MakePoint (x, y), ATTR_PRECISION, 1);
    else
    if ((time % VAL_us) == 0)
        SetTableCellAttribute (panel, table, MakePoint (x, y), ATTR_PRECISION, 3);
    else
    	SetTableCellAttribute (panel, table, MakePoint (x, y), ATTR_PRECISION, 5); 
*/
}


void TABLE_setCellValueYesNo (int panel, int table, Point p, int value)
{
	char help[100];
	
	if ((value < 0) || (value > 1)) return;	
	GetTableCellVal (panel, table, p, help);
	if (strcmp (help, stringYesNo[value]) != 0) 
		SetTableCellAttribute (panel, table, p, ATTR_CTRL_VAL, stringYesNo[value]);
//	SetTableCellVal (panel, table, p, stringYesNo[value]);
	setTableCellAttribute (panel, table, p, ATTR_TEXT_COLOR,
						   value ? VAL_GREEN : VAL_RED);

}

void TABLE_getCellValueYesNo (int panel, int table, Point p, int *value)
{
	char str[20];

	GetTableCellVal (panel, table, p, str);
	*value = (strcmp(str, stringYesNo[1]) == 0);
}


void TABLE_setCellValueYesNo2 (int panel, int table, Point p, int value)
{
	char help[100];
	
	if ((value < 0) || (value > 1)) return;	
	GetTableCellVal (panel, table, p, help);
	if (strcmp (help, stringYesNo[value]) != 0) 
		SetTableCellAttribute (panel, table, p, ATTR_CTRL_VAL, stringYesNo2[value]);
//	SetTableCellVal (panel, table, p, stringYesNo[value]);
	setTableCellAttribute (panel, table, p, ATTR_TEXT_COLOR,
						   value ? VAL_BLUE : VAL_GRAY);
}


void TABLE_getCellValueYesNo2 (int panel, int table, Point p, int *value)
{
	char str[20];

	GetTableCellVal (panel, table, p, str);
	*value = (strcmp(str, stringYesNo2[1]) == 0);
}



int TABLE_isCellYesNo (int panel, int table, Point p)
{
	char font[50];
	GetTableCellAttribute (panel, table, p, ATTR_TEXT_FONT, font);
	return strcmp (font, fontYesNo) == 0;
}


int TABLE_isCellDimmed (int panel, int table, Point p)
{
	int dimmed;
	
	GetTableCellAttribute (panel, table, p, ATTR_CELL_DIMMED, &dimmed);
	return dimmed;
}


void TABLE_setColumnAttributeStepType (int panel, int table, int col) 
{
	int numRows;
	//int i;
	//Point cell;
	Rect cells = VAL_TABLE_COLUMN_RANGE(col);
	
	SetTableCellRangeAttribute (panel, table, cells,ATTR_CELL_TYPE, VAL_CELL_RING);
	SetTableCellRangeAttribute(panel, table, cells, ATTR_RING_ITEMS_UNIQUE, 1);
	
	//SetTableCellRangeAttribute (panel, table, VAL_TABLE_COLUMN_RANGE(col),
	//					ATTR_TEXT_FONT, fontYesNo);
	//SetTableCellRangeAttribute (panel, table, VAL_TABLE_COLUMN_RANGE(col),
	//					ATTR_TEXT_POINT_SIZE, 24);
	//SetTableCellRangeAttribute (panel, table, VAL_TABLE_COLUMN_RANGE(col),
	//	                ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
	//SetTableCellRangeAttribute (panel, table, VAL_TABLE_COLUMN_RANGE(col),
	//					ATTR_CELL_MODE, VAL_INDICATOR);

	SetTableColumnAttribute (panel, table, col, ATTR_CELL_TYPE,VAL_CELL_RING);
	SetTableColumnAttribute(panel, table, col, ATTR_RING_ITEMS_UNIQUE, 1);
	
	GetNumTableRows(panel,table,&numRows); 
	//for (i=1; i <= numRows; i++) {
	//	cell = MakePoint(col,i);
		InsertTableCellRangeRingItem (panel,table,cells,-1,WFM_STEP_TYPE_STR[WFM_STEP_TYPE_NONE]);
		InsertTableCellRangeRingItem (panel,table,cells,-1,WFM_STEP_TYPE_STR[WFM_STEP_TYPE_TO_LINEAR]);
		InsertTableCellRangeRingItem (panel,table,cells,-1,WFM_STEP_TYPE_STR[WFM_STEP_TYPE_LIST]);
	//}
	//SetTableColumnAttribute (panel, table, col, ATTR_TEXT_FONT, fontYesNo);
	//SetTableColumnAttribute (panel, table, col,
	//					   ATTR_TEXT_POINT_SIZE, 24);
	//SetTableColumnAttribute (panel, table, col, ATTR_CELL_JUSTIFY,
	//					   VAL_CENTER_CENTER_JUSTIFIED);
	//SetTableColumnAttribute (panel, table, col, ATTR_CELL_MODE, VAL_INDICATOR);	
		
}


void TABLE_setColumnAttributesYesNo (int panel, int table, int col)
{
	Rect cells = VAL_TABLE_COLUMN_RANGE(col);
	
	SetTableCellRangeAttribute (panel, table, cells,
      ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableCellRangeAttribute (panel, table, cells,
						ATTR_TEXT_FONT, fontYesNo);
	SetTableCellRangeAttribute (panel, table, cells,
						ATTR_TEXT_POINT_SIZE, 24);
	SetTableCellRangeAttribute (panel, table, cells,
		                ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
	SetTableCellRangeAttribute (panel, table, cells,
						ATTR_CELL_MODE, VAL_INDICATOR);

	SetTableColumnAttribute (panel, table, col, ATTR_CELL_TYPE,
							 VAL_CELL_STRING);
	SetTableColumnAttribute (panel, table, col, ATTR_TEXT_FONT, fontYesNo);
	SetTableColumnAttribute (panel, table, col,
						   ATTR_TEXT_POINT_SIZE, 24);
	SetTableColumnAttribute (panel, table, col, ATTR_CELL_JUSTIFY,
						   VAL_CENTER_CENTER_JUSTIFIED);
	SetTableColumnAttribute (panel, table, col, ATTR_CELL_MODE, VAL_INDICATOR);

}





void TABLE_setColors (int panel, int control)
{
	int nRows, nColumns;
	Point p;
	int height;
	//int a;
	int error;
	
	GetNumTableRows    (panel, control, &nRows);
	GetNumTableColumns (panel, control, &nColumns);

	if (control != GetActiveCtrl (panel)) {
		//SetTableCellRangeAttribute (panel, control, MakeRect (1, 1, nRows, nColumns),
		//						ATTR_TEXT_BGCOLOR, VAL_WHITE);
		SetTableCellRangeAttribute (panel, control, VAL_TABLE_ENTIRE_RANGE,ATTR_TEXT_BGCOLOR, VAL_WHITE);

		return;
	}
	
	error = GetActiveTableCell (panel, control, &p);
	if (p.x == 0 || p.y == 0 || error < 0) { // table is empty or error
		return;	
	}
	height = p.y - 1;
	if (height > 0) {
		SetTableCellRangeAttribute (panel, control, MakeRect (1, 1, height, nColumns),
								ATTR_TEXT_BGCOLOR, VAL_WHITE);
	}
	SetTableCellRangeAttribute (panel, control, MakeRect (p.y, 1, 1, nColumns),
								ATTR_TEXT_BGCOLOR, MakeColor(255,200,130));
	height = nRows - p.y;
	if (height > 0) {
		SetTableCellRangeAttribute (panel, control, MakeRect (p.y+1, 1, height, nColumns),
								ATTR_TEXT_BGCOLOR, VAL_WHITE);
	}
}

void TABLE_changeRow (int panel, int control, Point p, int setColors)
{
//	static Point last = {1, 1};
	int nRows, nColumns;
	//int height;
	
	GetNumTableRows    (panel, control, &nRows);
	GetNumTableColumns (panel, control, &nColumns);
	if (p.x < 1)     p.x = 1; 
	if (p.x > nColumns) p.x = nColumns;
	if (p.y < 1)     p.y = 1; 
	if (p.y > nRows) p.y = nRows;
	SetActiveTableCell (panel, control, p);
	if (setColors) TABLE_setColors (panel, control);
}


void CVICALLBACK TABLE_runPopupMenu (int panel, int control, int top, int left, void *callbackData)
{
//	int i;
	int k;
	Point p;
	Rect selection;
	int numPoints;
	t_controls *c;
	int panelInc;
	int choice;
	int error;
	
	// -----------------------------------------
	//    get active cell
	// -----------------------------------------
	error = GetActiveTableCell (panel, control, &p);
	if (error < 0) {
		return;	
	}
	
	GetTableSelection (panel, control, &selection);
	if (selection.height == 0) selection = MakeRect (p.y, p.x, 1, 1);
	GetNumTableRows (panel, control, &numPoints);

	// DEBUG
//	c  = (t_controls *) callbackData;
	

	
	c = &WFM_controls;
	
	choice = RunPopupMenu (menuTable, TABLE_MENU, panel,
						   top, left, 0, 0, 0, 0);
	switch (choice) {
	    case TABLE_MENU_INSERT_ABOVE: 
			// -----------------------------------------
			//      insert rows above  
			// -----------------------------------------
			if (p.y == 0) p.y = 1;
			TABLE_insertRow (c, p.y);
			break;
	    case TABLE_MENU_INSERT_BELOW: 
			// -----------------------------------------
			//      insert rows above  
			// -----------------------------------------
			if (p.y == 0) p.y = 1;
			TABLE_insertRow (c, p.y+1);
			break;
	    case TABLE_MENU_DELETE: 
			// -----------------------------------------
			//      delete rows
			// -----------------------------------------
	        if (p.y <= numPoints) {
				if (selection.top > 0) DeleteTableRows (panel,  control, selection.top, selection.height);
				  else DeleteTableRows (panel, control, p.y, 1);
				POINTS_tableToGraph (c, 1);
				CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, NULL);
	        }
	        break;
		// -----------------------------------------
		//      variable voltage
		// -----------------------------------------
	    case TABLE_MENU_VARTIME_ON: 
			for (k = 0; k < selection.height; k++) {
    			if (k+selection.top != 1) {
    				TABLE_setCellValueYesNo (panel, control, MakePoint (TABLE_COL_TIME_VARY, k+selection.top), 1);
    			}
				POINTS_tableToGraph (c, 1);
				CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, NULL);
			}		    						
	        break;
	    case TABLE_MENU_VARTIME_OFF: 
			for (k = 0; k < selection.height; k++) {
    			TABLE_setCellValueYesNo (panel, control, MakePoint (TABLE_COL_TIME_VARY, k+selection.top), 0);
				POINTS_tableToGraph (c, 1);
				CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, NULL);
			}		    						
	        break;
	    case TABLE_MENU_VARVALUE_ON: 
			for (k = 0; k < selection.height; k++) {
    			TABLE_setCellValueYesNo (panel, control, MakePoint (TABLE_COL_VALUE_VARY, k+selection.top), 1);
				POINTS_tableToGraph (c, 1);
				CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, NULL);
			}		    						
	        break;
	    case TABLE_MENU_VARVALUE_OFF: 
			for (k = 0; k < selection.height; k++) {
    			TABLE_setCellValueYesNo (panel, control, MakePoint (TABLE_COL_VALUE_VARY, k+selection.top), 0);
				POINTS_tableToGraph (c, 1);
				CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, NULL);
			}		    						
	        break;
	    case TABLE_MENU_INCTIME:
	    	panelInc = LoadPanel (0, UIR_common, POPUP_Incr);
			if (c->timeMultiply == TIME_MULTIPLY_us) 
				SetCtrlAttribute (panelInc, POPUP_Incr_NUMERIC_increment,
								  ATTR_LABEL_TEXT, "탎");
			else 
				SetCtrlAttribute (panelInc, POPUP_Incr_NUMERIC_increment,
								  ATTR_LABEL_TEXT, "ms");
			SetCtrlAttribute (panelInc, POPUP_Incr_COMMANDBUTTON_done,
							  ATTR_CALLBACK_DATA, c);
			InstallPopup (panelInc);
	    	break;
	}
	
}



int CVICALLBACK TABLE_processEvents_CB  (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//Rect r;
	Point p, mouse;
	int runState = VAL_SELECT_STATE;
	//int ctrlMode;
	int allowEdit = 0;
	int displayColors = 1;
	int value;
	int error;

	//GetTableSelection (panel, control, &r);
	error = GetActiveTableCell (panel, control, &p);
	if (error < 0) {
		return 0;	
	}
	//if (p.x > 0 && p.y > 0) {
	//	GetTableCellAttribute (panel, control, p, ATTR_CELL_MODE, &ctrlMode);
	//}
	GetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, &runState);
	allowEdit     = ((int) callbackData) & 0x0001;
	displayColors = (((int) callbackData) & TABLE_NOT_CHANGE_COLORS) == 0;
	

//	DebugPrintf ("%s (TABLE_processEvents)\n", eventStr(event, eventData1, eventData2));
//	DebugPrintf ("mouseAbs=(%d,%d), control=(%d,%d), active= (%d,%d)", eventData1, eventData2, panel, control, p.x, p.y);
//	DebugPrintf (", selection=(%d,%d,%d,%d)", r.top, r.left, r.height, r.width);
//	DebugPrintf (", ctrlMode=(%s,%s)\n", ctrlMode == VAL_HOT ? "HOT" : "INDICATOR",
//										 runState == VAL_EDIT_STATE ? "EDIT" : "SELECT");

 	switch (event) {
	    case EVENT_KEYPRESS:
			if (eventData1 == ',') {
				FakeKeystroke ('.');
				break; // TODO in another case this break did not lead to break of switch - check it.
			}
			switch (eventData1 & VAL_VKEY_MASK) {
				case VAL_TAB_VKEY:
					if ((eventData1 & VAL_SHIFT_MODIFIER) == 0) 
						p.x +=1;
					else 
						p.x -=1;
					TABLE_changeRow (panel, control, p, displayColors);
    				if (runState == VAL_EDIT_STATE) {
						CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
					}					
					return 1;
				case VAL_ENTER_VKEY:
    				if (runState == VAL_EDIT_STATE) {
	    				SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_SELECT_STATE);
    					p.y += 1;
						TABLE_changeRow (panel, control, p, displayColors);
						CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
						return 0;
    				}
    				else {
						if (allowEdit) SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_EDIT_STATE);
						if (displayColors) TABLE_setColors (panel, control);
	    				return 0;
	    			}
				case VAL_LEFT_ARROW_VKEY:
					if (runState == VAL_EDIT_STATE) return 0;
					p.x -= 1;
					TABLE_changeRow (panel, control, p, displayColors);
					return 1;
				case VAL_RIGHT_ARROW_VKEY:
					if (runState == VAL_EDIT_STATE) return 0;
					p.x += 1;
					TABLE_changeRow (panel, control, p, displayColors);
					return 1;
				case VAL_DOWN_ARROW_VKEY:
    				if (runState == VAL_EDIT_STATE) return 0;
    				else {
    					p.y += 1;
						TABLE_changeRow (panel, control, p, displayColors);
						CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
						return 1;
    				}
				case VAL_UP_ARROW_VKEY:
    				if (runState == VAL_EDIT_STATE) return 0;
    				else {
						p.y -= 1;
						TABLE_changeRow (panel, control, p, displayColors);
						CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
						return 1;
    				}
				case VAL_HOME_VKEY:
					p.y = 0;
					TABLE_changeRow (panel, control, p, displayColors);
					CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
					return 1;
				case VAL_END_VKEY:
					p.y = INT_MAX;
					TABLE_changeRow (panel, control, p, displayColors);
					CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
					return 1;
				case VAL_PAGE_UP_VKEY:
					p.y -= 10;
					TABLE_changeRow (panel, control, p, displayColors);
					CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
					return 1;
				case VAL_PAGE_DOWN_VKEY:              
					p.y += 10;
					TABLE_changeRow (panel, control, p, displayColors);
					CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
					return 1;
				default:
/*					if (ctrlMode != VAL_HOT) {
	    				SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_SELECT_STATE);
						return 1;
					}
*/					// swallow key event
//					if ((eventData1 >= '0') && (eventData1 <= '9')) {
    				GetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, &runState);
    				if ((runState != VAL_EDIT_STATE) && allowEdit) {
	    				SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_EDIT_STATE);
	    			}
	    			return 0;
			}
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    error = GetTableCellFromPoint (panel, control, mouse, &p);
			if (error < 0) {
				break;
			}
			TABLE_changeRow (panel, control, p, displayColors);
			if (p.x >0 && p.y > 0 && TABLE_isCellYesNo (panel, control, p)) {
				if (!TABLE_isCellDimmed (panel, control, p)) {
					TABLE_getCellValueYesNo (panel, control, p, &value);
					TABLE_setCellValueYesNo (panel, control, p, !value);
					CallCtrlCallback (panel, control, EVENT_COMMIT, 0, 0, 0);
				}
			}
			else if (allowEdit) {
				SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_EDIT_STATE);
			}
			break;
		case EVENT_LEFT_CLICK:
			ConvertMouseCoordinates (panel, control, 1, 1, &eventData2,
							 &eventData1);
			mouse.x = eventData2 + ctrlLeft(panel, control);
			mouse.y = eventData1 + ctrlTop(panel, control);
		    GetTableCellFromPoint (panel, control, mouse, &p); 
			TABLE_changeRow (panel, control, p, displayColors);
			if (allowEdit) {
				SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE, VAL_EDIT_STATE);
			}
			return 0;
		case EVENT_RIGHT_CLICK:
			TABLE_runPopupMenu (panel, control, eventData1, eventData2, callbackData);
			break;
	
		}
	return 0;
}






void POINTS_defineControls (t_controls *c, int panel, int table, 
						    int graph, int repetition, int invalid, int tableRowHeight)
{
	c->panel      = panel;
	c->table      = table;
	c->graph      = graph;
	c->repetition = repetition;
	c->invalid    = invalid;
	c->tableRowHeight = tableRowHeight;
	c->f		 = NULL;
}


void POINTS_setParameterValues (t_controls *c,
						   double timeMultiply,
						   char *valueLabel, 
						   char *valueLabelIncr,
						   double min, double max, int digits, 
						   int showStepRep,
						   int voltageVisible)
{
	c->timeMultiply   = timeMultiply;
	c->valueLabel     = valueLabel;
	c->valueLabelIncr = valueLabelIncr;
	c->valueMin       = min;
	c->valueMax       = max;
	c->valueDigits    = digits;
	c->showStepRep    = showStepRep;
	c->voltageVisible = voltageVisible;
}





void POINTS_tableToGraph (t_controls *c, int updateTable)
{
	t_point *p;
	int nPoints;
	int repetition;

	if (c == NULL) return;
	p = NULL;
	
	POINTS_tableToListOfPoints (c, &p, &nPoints, 1);
	if (c->repetition >= 0) GetCtrlVal (c->panel, c->repetition, &repetition);
	else repetition = 1;
	POINTS_showGraph (c, p, nPoints, repetition-1);
//	POINTS_showGraph (c, c->lastPoints, c->nLastPoints, repetition);
	if (updateTable) {
//		POINTS_listOfPointsToTable (c, p, nPoints);
		//SetActiveCtrl (c->panel, c->table);
		TABLE_setColors (c->panel, c->table);
	}

	free (p);
}




void POINTS_createTableColumns (t_controls *c)
{
	int columns, rows, nColumnsVisible;
	int cellType, width, align;
	int cellMode;
    int decimalPlaces;
    double min;
    double max;
    int dataType;
    int i;
    char *label;

	if (c == NULL) return;

//    SetCtrlAttribute (c->panel, c->table, ATTR_VISIBLE, 0);
    deleteAllTableColumns (c->panel, c->table);
	nColumnsVisible = TABLE_NCOLUMNS-2+c->showStepRep;
	GetNumTableColumns (c->panel, c->table, &columns);
	if (columns == nColumnsVisible) return;
	GetNumTableRows (c->panel, c->table, &rows);
	if (rows > 0) DeleteTableRows (c->panel, c->table, 1, -1);
	DeleteTableColumns (c->panel, c->table, 1, -1);
	for (i = 1; i < TABLE_NCOLUMNS-1+c->showStepRep; i++) {
		#ifdef _CVI_DEBUG_
//		DisplayPanel (c->panel);
		#endif
		width = 80;
		align = VAL_CENTER_CENTER_JUSTIFIED;
		cellType = VAL_CELL_NUMERIC;
		cellMode = VAL_HOT;
		decimalPlaces = 1;
		min = 0;
		if (c->timeMultiply == TIME_MULTIPLY_us) max = 1E6;
		else max = 1E5;
		dataType = VAL_DOUBLE;
		switch (i) {
			case TABLE_COL_TIME_VARY: 
			  	cellType = VAL_CELL_STRING;
			  	width = 40;
			  	label = "vary";
			  	break;
			case TABLE_COL_TIME_START:
				if (c->timeMultiply == TIME_MULTIPLY_us) label = "time [탎]";
				else label = "time [ms]";
				if (c->allowNegativeTimes) 
				 min = -1E6;
				break;
			case TABLE_COL_TIME_INCREMENT:
				if (c->timeMultiply == TIME_MULTIPLY_us) label = "increment per repetition [탎]";
				else label = "increment per repetition [ms]";
				min = -1E6;
				break;
			case TABLE_COL_VALUE_VARY:
//			  	cellType = VAL_CELL_PICTURE;
			  	cellType = VAL_CELL_STRING;
			  	width = 40;
			  	label = "vary";
			  	align = VAL_CENTER_CENTER_JUSTIFIED;
			  	break;
			case TABLE_COL_VALUE_START:
			  	label         = c->valueLabel;
				decimalPlaces = c->valueDigits;
				min 		  = c->valueMin;
				max 		  = c->valueMax;
				break;
			case TABLE_COL_VOLTAGE:
			  	label         = c->voltageVisible ? "V": "";
				min 		  = -10.0;
				max 		  = +10.0;
				decimalPlaces = 4;
				width         = c->voltageVisible ? 40 : 1;
				cellMode = VAL_INDICATOR;
				break;
			case TABLE_COL_VALUE_INCREMENT:
			  	label = c->valueLabelIncr; 
				decimalPlaces = c->valueDigits;
				min 		  = c->valueMin;
				max 		  = c->valueMax;
				break;
			case TABLE_COL_STEP_REP:
				dataType = VAL_INTEGER;
				min = 1;
				max = 10000;
				label = "step every N repetitions";
				break;
			default:
				label = "";
		}
		InsertTableColumns (c->panel, c->table, i, 1, cellType);
		SetTableColumnAttribute (c->panel, c->table, i,
								 ATTR_COLUMN_WIDTH, width);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_LABEL_TEXT, label);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_CELL_TYPE, cellType);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_CELL_MODE,
								 cellMode);
		SetTableColumnAttribute (c->panel, c->table, i,
								 ATTR_TEXT_BGCOLOR,
								 cellMode == VAL_HOT ? VAL_WHITE : VAL_MED_GRAY);
		if (cellType == VAL_CELL_NUMERIC) {
			SetTableColumnAttribute (c->panel, c->table, i,
								     ATTR_SHOW_INCDEC_ARROWS, cellMode == VAL_HOT);
			SetTableColumnAttribute (c->panel, c->table, i, ATTR_DATA_TYPE, dataType);
			if (dataType == VAL_DOUBLE) {
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_INCR_VALUE, 0.1);
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_MAX_VALUE,
										 max);
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_MIN_VALUE,
										 min);
			}
			else {
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_INCR_VALUE, 1);
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_MAX_VALUE,
										 (int) max);
				SetTableColumnAttribute (c->panel, c->table, i, ATTR_MIN_VALUE,
										 (int) min);
			}
			SetTableColumnAttribute (c->panel, c->table, i, ATTR_PRECISION,
									 decimalPlaces);
			SetTableColumnAttribute (c->panel, c->table, i,
									 ATTR_SHOW_INCDEC_ARROWS, 1);
			SetTableColumnAttribute (c->panel, c->table, i,
									 ATTR_CHECK_RANGE, VAL_COERCE);
		}
		if (cellType == VAL_CELL_STRING) {
			TABLE_setColumnAttributesYesNo (c->panel, c->table, i);
		}
//		SetTableColumnAttribute (panel, table, i, ATTR_CELL_MODE,
//					df(valueID)->isEditable ? VAL_HOT : VAL_INDICATOR);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_USE_LABEL_TEXT, 1);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_LABEL_WRAP_MODE, VAL_WORD_WRAP);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_CELL_JUSTIFY, align);
		SetTableColumnAttribute (c->panel, c->table, i, ATTR_CELL_JUSTIFY, align);
		
	}


}




void POINTS_initControls (t_controls *c)
{
	//int i;
	char *label;
	//static int allowEdit = 1;
	static int panelDummy = -1;

    // init table
//	SetCtrlAttribute (c->panel, c->table, ATTR_CALLBACK_DATA, c);

/* DEBUG!!!
	ChainCtrlCallback (c->panel, c->table, POINTS_processEvents_CB, c,
					   "POINTS_processEvents");
	ChainCtrlCallback (c->panel, c->table, TABLE_processEvents_CB, &allowEdit,
					   "TABLE_processEvents");
*/
 
	// -----------------------------------------
    //     hide built-in menus
	// -----------------------------------------
    HideBuiltInCtrlMenuItem (c->panel, c->table, VAL_GOTO);
    HideBuiltInCtrlMenuItem (c->panel, c->table, VAL_SEARCH);
    HideBuiltInCtrlMenuItem (c->panel, c->table, VAL_SORT);
	// -----------------------------------------
	// 		create new menus
	// -----------------------------------------
	if (panelDummy == -1) {
		panelDummy = LoadPanel (0, UIR_common, TMENU);
		menuTable = GetPanelMenuBar (panelDummy);
	}
	
/*
	for (i = 0; i < NMenuItems; i++) {
		if (tableMenuText[i] == NULL)
		    menuItemIDList[i] = NewCtrlMenuSeparator (c->panel, c->table, -1);
		else 
		    menuItemIDList[i] = NewCtrlMenuItem (c->panel, c->table, tableMenuText[i],
								-1, POINTS_tableRightClicked, c);
	}
*/
	
	// -----------------------------------------
	//    create table columns 
	// -----------------------------------------
	POINTS_createTableColumns (c);
		
	
	// -----------------------------------------
    // init graph
	// -----------------------------------------
    SetCtrlAttribute (c->panel, c->graph,
                          ATTR_ENABLE_ZOOM_AND_PAN, 1);
	SetCtrlAttribute (c->panel, c->graph,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
	if (c->timeMultiply == TIME_MULTIPLY_us) label = "time [탎]";
	else label = "time [ms]";
	SetCtrlAttribute (c->panel, c->graph, ATTR_XNAME, label);
	// -----------------------------------------
    //   init 
	// -----------------------------------------
	if (c->repetition >= 0) 
//		SetCtrlAttribute (c->panel, c->repetition, ATTR_CALLBACK_DATA, c);
	  ;
}


void POINTS_displayPoint (t_controls *c, t_point *p, int i)
{
    double dVal;
    //int old;

    TABLE_setCellValueYesNo (c->panel, c->table, MakePoint (TABLE_COL_TIME_VARY, i+1), 
    				 		 p[i].varyTime);
	dVal = fRound (p[i].timeStart_ns);
	TABLE_setCellTimeDouble (c->panel, c->table, MakePoint (TABLE_COL_TIME_START, i+1), 
    				 		 dVal / c->timeMultiply);
	dVal = fRound (p[i].timeIncrement_ns);
    TABLE_setCellTimeDouble (c->panel, c->table, MakePoint (TABLE_COL_TIME_INCREMENT, i+1), 
    				 		 dVal / c->timeMultiply);
	TABLE_setCellValueYesNo (c->panel, c->table, MakePoint (TABLE_COL_VALUE_VARY, i+1), 
    				 p[i].varyValue);	    
    setTableCellValDouble (c->panel, c->table, MakePoint (TABLE_COL_VALUE_START, i+1), 
    				 p[i].valueStart);

    if (c->f != NULL) {
    	setTableCellValDouble (c->panel, c->table, MakePoint (TABLE_COL_VOLTAGE, i+1), 
    				 TRANSFERFUNCT_apply (c->f, p[i].valueStart));
    }
    setTableCellValDouble (c->panel, c->table, MakePoint (TABLE_COL_VALUE_INCREMENT, i+1), 
    				 p[i].valueIncrement);
    if (c->showStepRep) {
		setTableCellAttribute (c->panel, c->table, 
							   MakePoint (TABLE_COL_STEP_REP, i+1), 
							   ATTR_DATA_TYPE, VAL_INTEGER);
	    setTableCellValInt (c->panel, c->table, MakePoint (TABLE_COL_STEP_REP, i+1), 
	    				 p[i].stepRepetitions);
		setTableCellAttribute (c->panel, c->table,
					   MakePoint (TABLE_COL_STEP_REP, i+1), 
					   ATTR_CELL_DIMMED, !(p[i].varyValue || p[i].varyTime));
	}
	if (!c->allowNegativeTimes) {
		setTableCellAttribute (c->panel, c->table,
						   MakePoint (TABLE_COL_TIME_VARY, i+1), 
						   ATTR_CELL_DIMMED, i == 0);
		setTableCellAttribute (c->panel, c->table,
						   MakePoint (TABLE_COL_TIME_START, i+1), 
						   ATTR_CELL_DIMMED, i == 0);
		setTableCellAttribute (c->panel, c->table,
						   MakePoint (TABLE_COL_TIME_INCREMENT, i+1), 
						   ATTR_CELL_DIMMED, (!p[i].varyTime) || (i == 0));
		setTableCellAttribute (c->panel, c->table,
						   MakePoint (TABLE_COL_VALUE_INCREMENT, i+1), 
						   ATTR_CELL_DIMMED, !p[i].varyValue);
	}
}


void POINTS_duplicate (t_controls *c, t_point *p, int nPoints)
{
	if (p == NULL) {
		free (c->lastPoints);
		c->lastPoints = NULL;
		c->nLastPoints = 0;
		return;
	}
	if (nPoints != c->nLastPoints) {
		free (c->lastPoints);
		c->lastPoints = (t_point *) malloc (sizeof (t_point) * nPoints);
	}
	memcpy (c->lastPoints, p, sizeof (t_point) * nPoints);
	c->nLastPoints = nPoints;
}




void POINTS_listOfPointsToTable (t_controls *c, t_point *p, int nPoints)
{
    int i;
	
	setNumTableRows (c->panel, c->table, nPoints, c->tableRowHeight);
    
    for (i = 0; i < nPoints; i++) {
		POINTS_displayPoint (c, p, i);
//		ProcessDrawEvents();
    }
    if ((nPoints > 0) && (!c->allowNegativeTimes)) {
		setTableCellValDouble (c->panel, c->table, 
						 MakePoint (TABLE_COL_TIME_START, 1), 0.0);
		setTableCellValDouble (c->panel, c->table, 
						 MakePoint (TABLE_COL_TIME_INCREMENT, 1), 0.0);
	}
	
	POINTS_duplicate (c, p, nPoints);
}



void POINTS_tableToListOfPoints (t_controls *c, t_point **points, int *nPoints, int saveAsLast)
{
	t_point *p;
	int i;
	int numPoints = 0;
	//double test;
	double timeAdd;
	
	if (c->panel == 0) return;
	GetNumTableRows (c->panel, c->table, &numPoints);
	// -----------------------------------------
	//    assign memory
	// -----------------------------------------
	*nPoints = numPoints;
	*points = (t_point *) realloc (*points, sizeof(t_point) * numPoints);
	
	timeAdd = 0;
	for (i = 0; i < *nPoints; i++) {
		p = &(*points)[i];
    	POINT_init (p);
		TABLE_getCellValueYesNo (c->panel, c->table, MakePoint (TABLE_COL_TIME_VARY, i+1), 
								 &p->varyTime);
    	GetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_TIME_START, i+1), 
    					 &p->timeStart_ns);
/*		test = fRound (1.5);
		test = fRound (1.4);
		test = fRound (100000000.0);
		test = fRound (100000000.9);
*/		
		p->timeStart_ns = fRound (c->timeMultiply * p->timeStart_ns + timeAdd);
//		p->timeStart_ns = RoundRealToNearestInteger (c->timeMultiply * p->timeStart_ns);
//		p->timeStart_ns = fRound (p->timeStart_ns + timeAdd);

		if ((i < c->nLastPoints) && (timeAdd == 0)) {
			if (p->timeStart_ns != c->lastPoints[i].timeStart_ns) 
				timeAdd = p->timeStart_ns - c->lastPoints[i].timeStart_ns;
		}
		
		
	    TABLE_setCellTimeDouble (c->panel, c->table, MakePoint (TABLE_COL_TIME_START, i+1), 
	    				 		 p->timeStart_ns / c->timeMultiply);
		ProcessDrawEvents();

    	GetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_TIME_INCREMENT, i+1), 
    					 &p->timeIncrement_ns);
		p->timeIncrement_ns = fRound (c->timeMultiply * p->timeIncrement_ns);    					 
//		p->timeIncrement_ns = RoundRealToNearestInteger (c->timeMultiply * p->timeIncrement_ns);
	    TABLE_setCellTimeDouble (c->panel, c->table, MakePoint (TABLE_COL_TIME_INCREMENT, i+1), 
	    				 		 p->timeIncrement_ns / c->timeMultiply);
		
		TABLE_getCellValueYesNo (c->panel, c->table, MakePoint (TABLE_COL_VALUE_VARY, i+1), 
						 		 &p->varyValue);
    	GetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_VALUE_START, i+1), 
    					 &p->valueStart);
    	GetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_VALUE_INCREMENT, i+1), 
    					 &p->valueIncrement);
	    if (c->showStepRep) {
		    GetTableCellVal (c->panel, c->table, MakePoint (TABLE_COL_STEP_REP, i+1), 
		    				 &p->stepRepetitions);
		}
		else p->stepRepetitions = 1;
	}
	if (saveAsLast) POINTS_duplicate (c, *points, *nPoints);
}




int POINTS_consistent (t_point *p, int nPoints)
{
	int i;
	
	for (i = 1; i < nPoints; i++) {
		if (p[i].timeStart_ns < p[i-1].timeStart_ns) return 0;   		
	}
	
	return 1;

}



void POINTS_calculateValues (t_point *p, int nPoints, int repetition, int allowNegativeTimes)
{
	int i;
	int nRun;
	
	// first step calculate variable times
	for (i = 0; i < nPoints; i++) {
		if (p[i].stepRepetitions == 0)  p[i].stepRepetitions = 1;
		nRun = repetition / p[i].stepRepetitions;
		p[i].thisTime_ns = p[i].timeStart_ns;
		if (p[i].varyTime) p[i].thisTime_ns += nRun* p[i].timeIncrement_ns;
		if (!allowNegativeTimes && (p[i].thisTime_ns < 0)) 
			p[i].thisTime_ns = 0;
		p[i].thisValue = p[i].valueStart;
		if (p[i].varyValue) p[i].thisValue += nRun * p[i].valueIncrement;
	}

	if (!POINTS_consistent (p, nPoints)) return;
	// second step: check for time overlaps
	for (i = 0; i < nPoints-1; i++) {
//		if ((i > 0) && (p[i].thisTime < p[i-1].thisTime)) p[i].thisTime = p[i-1].thisTime;
		if (p[i].thisTime_ns > p[i+1].thisTime_ns) p[i].thisTime_ns = p[i+1].thisTime_ns;
	}
}




int POINTS_plot (int panel, int graph, t_point *p, int nPoints, int repetition, double timeMultiply, int retainGraph)
{
	double *xValues;
	double *yValues;
	int i;
	int plotHandle = 0;
	
	if (nPoints == 0) return 0;
	
	xValues = (double *) malloc (sizeof(double) * nPoints);
	yValues = (double *) malloc (sizeof(double) * nPoints);
	
	POINTS_calculateValues (p, nPoints, repetition, 1);
	for (i = 0; i < nPoints; i++) {
		xValues[i] = p[i].thisTime_ns / timeMultiply;
		yValues[i] = p[i].thisValue;
	}
    if (!retainGraph) DeleteGraphPlot (panel, graph, -1, VAL_DELAYED_DRAW);
    
	SetCtrlAttribute (panel, graph, ATTR_DIMMED, nPoints <= 0);
    
    plotHandle = PlotXY (panel, graph, xValues, yValues,
        nPoints, VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
        VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
	
	free (xValues);
	free (yValues);
	
	return plotHandle;

}

void POINTS_showInvalid (int panel, int control, t_point *p, int nPoints)
{
	SetCtrlAttribute (panel, control, ATTR_VISIBLE,
					  !POINTS_consistent (p, nPoints));
}


void POINTS_showGraph (t_controls *c, t_point *p, int nPoints, int repetition)
{
	POINTS_plot (c->panel, c->graph, p, nPoints, repetition, c->timeMultiply, 0);
	POINTS_showInvalid (c->panel, c->invalid, p, nPoints);
}


//=======================================================================
//
//    values changed in table
//
//=======================================================================
int CVICALLBACK POINTS_processEvents_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_controls *c = NULL;
 	Rect selection;
 	Point p;
 	int dimmed;
 	
 	
/* 	c = (t_controls *) callbackData;
 	if (TABLE_processEvents (panel, control, event, eventData1, eventData2, 1) > 0) {
		TABLE_setColors (panel, control);
//		POINTS_tableToGraph (c, 0);
 		return 0;
 	}
 	
*/	
	
	if ((panel == WFM_controls.panel) && (control == WFM_controls.table))
		c = &WFM_controls;
	else 
		return 0;
	switch (event)
		{
		case EVENT_RIGHT_CLICK:
			GetTableSelection (panel, control, &selection);
			if (selection.height == 0) {
				GetActiveTableCell (panel, control, &p);
				selection = MakeRect (p.y, p.x, 1, 1);
			}
			dimmed = (selection.left != TABLE_COL_TIME_START)
					|| (selection.width != 1);
			SetMenuBarAttribute (menuTable, TABLE_MENU_INCTIME, ATTR_DIMMED,
								  dimmed);
			break;
		case EVENT_KEYPRESS:
			if (eventData1 == VAL_ENTER_VKEY) {
				POINTS_tableToGraph (c, 0);			
			}

//			if (eventData1 == ',') FakeKeystroke ('.');
			break;
		case EVENT_LEFT_CLICK: 
		case EVENT_COMMIT:
		case EVENT_VAL_CHANGED:
		case EVENT_LOST_FOCUS:

			POINTS_tableToGraph (c, 0);
			break;
		}
	TABLE_setColors (panel, control);
	return 0;
	
}






int CVICALLBACK POINTS_POPUP_incr_done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_controls *c;
	double incr;
	double value;
	Rect selection;
	Point p;
	int i;
	
	c = &WFM_controls;
	switch (event) {
		case EVENT_COMMIT:
			if (control == POPUP_Incr_COMMANDBUTTON_done) {
//				c = (t_controls *) callbackData;
				GetCtrlVal (panel, POPUP_Incr_NUMERIC_increment, &incr);
				GetTableSelection (c->panel, c->table, &selection);
				if (selection.height == 0) {
					GetActiveTableCell (c->panel, c->table, &p);
					selection = MakeRect (p.y, p.x, 1, 1);
					if (p.x == 0 || p.y  == 0) {
						RemovePopup(0);
						return 0;
					}
				}
				for (i = selection.top; i < selection.top + selection.height; i++) {
					GetTableCellVal (c->panel, c->table, 
									 MakePoint (TABLE_COL_TIME_START , i), &value);
					SetTableCellVal (c->panel, c->table, 
									 MakePoint (TABLE_COL_TIME_START , i), value + incr);
				}
				POINTS_tableToGraph (c, 0);
				CallCtrlCallback (c->panel, c->table, EVENT_COMMIT, 0, 0, NULL);

			}
			RemovePopup (0);
			break;
	}
	return 0;
}


void POINTS_shiftValues (t_point *p, int nPoints, double shift)
{
	int i;
	
	for (i = 0; i < nPoints; i++) {
		p[i].timeStart_ns += shift;
	}
}


int POINTS_hasNegativeTime (t_point *p, int nPoints)
{
	int i;
	
	for (i = 0; i < nPoints; i++) {
		if (p[i].thisTime_ns < 0) return 1;
	}

	return 0;
}

