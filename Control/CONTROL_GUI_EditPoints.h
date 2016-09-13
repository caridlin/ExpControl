#ifndef GUI_EDIT_POINTS
#define GUI_EDIT_POINTS


#include "CONTROL_DataStructure.h"					
#include "tools.h"

#define TIME_MULTIPLY_ms 1.0E6
#define TIME_MULTIPLY_us 1.0E3

#define stringYes "ü"
#define stringNo "û"
#define fontYesNo "Wingdings"

#define stringYes2 "l"
#define stringNo2 "¡"




#define TABLE_NOT_CHANGE_COLORS 0x0002

typedef struct {
	int panel;
	int table;
	int tableRowHeight;	
	int graph;
	int repetition;
	int invalid;
	int allowNegativeTimes;
	
	double timeMultiply;  // ms: 1E6   µs: 1E3
	
	char *valueLabel;
	char *valueLabelIncr;
	double valueMin;
	double valueMax;
	int valueDigits;
	
	int showStepRep;
	int voltageVisible;
	
	t_transferFunction *f;
	t_point *lastPoints;
	int nLastPoints;
} t_controls;



void CONTROLS_init (t_controls *c);



int CVICALLBACK TABLE_processEvents_CB  (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

//int TABLE_processEvents (int panel, int control, int event,
// int eventData1, int eventData2, int allowEdit);

void TABLE_changeRow (int panel, int control, Point p, int setColors);

void TABLE_setColors (int panel, int control);

void TABLE_setCellTimeDouble (int panel, int table, Point p, double dTime);

int TABLE_isCellYesNo (int panel, int table, Point p);
																		   
int TABLE_isCellDimmed (int panel, int table, Point p);

void TABLE_setCellValueYesNo (int panel, int table, Point p, int value);

void TABLE_getCellValueYesNo (int panel, int table, Point p, int *value);


void TABLE_setCellValueYesNo2 (int panel, int table, Point p, int value);

void TABLE_getCellValueYesNo2 (int panel, int table, Point p, int *value);


void TABLE_setColumnAttributesYesNo (int panel, int table, int col);

void TABLE_setColumnAttributeStepType (int panel, int table, int col);  

void POINTS_defineControls (t_controls *c, int panel, int table, 
						    int graph, int repetition, int invalid, int tableRowHeight);

//void POINTS_defineControls (t_controls *c, int panel, int table, 
//						    int graph, int repetition, int invalid);




void POINTS_setParameterValues (t_controls *c, 
						   double timeMultiply,
						   char *valueLabel, 
						   char *valueLabelIncr,
						   double min, double max, int digits,
						   int showStepRep,
						   int voltageVisible);

void POINTS_createTableColumns (t_controls *c);


void POINTS_initControls(t_controls *c);

void POINTS_tableToGraph (t_controls *c, int updateTable);
					   
void POINTS_listOfPointsToTable (t_controls *c, t_point *p, int nPoints);

void POINTS_tableToListOfPoints (t_controls *c, t_point **points, int *nPoints, int saveAsLast);


int POINTS_consistent (t_point *p, int nPoints);

void POINTS_calculateValues (t_point *p, int nPoints, int repetition, int allowNegativeTimes);

int POINTS_plot (int panel, int graph, t_point *p, int nPoints, int repetition, double timeMultiply, int retainGraph);

void POINTS_showInvalid (int panel, int control, t_point *p, int nPoints);

void POINTS_showGraph (t_controls *c, t_point *p, int nPoints, int repetition);

void POINTS_duplicate (t_controls *c, t_point *p, int nPoints);


int CVICALLBACK POINTS_processEvents_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

void POINTS_shiftValues (t_point *p, int nPoints, double shift);

int POINTS_hasNegativeTime (t_point *p, int nPoints);


#endif
