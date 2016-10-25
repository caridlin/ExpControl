#include <utility.h>
#include <userint.h>
#include "UIR_ExperimentControl.h"
#include "INCLUDES_CONTROL.h"    

#define MAX_DISPLAY_DIGITAL_BLOCKS 180

// see http://www.farb-tabelle.de/de/farbtabelle.htm

#define VAL_COLOR_SLATEBLUE1       0x00836FFF
#define VAL_COLOR_DAC_WAVEFORMS     VAL_GREEN
#define VAL_COLOR_VARIABLE_TIME     0x009ACD32
#define VAL_COLOR_DAC_CONSTVOLATGE  0x0076EE00
#define VAL_COLOR_DAC_HOLD  		0x007CFC00

//#define SPLITTER_BOTTOM_POS(buttonHeight) (N_DAC_CHANNELS*((buttonHeight+1))-5)
										  

#define SPLITTER_BOTTOM_POS(buttonHeight) (N_DAC_CHANNELS*((buttonHeight+1)))

#define COLOR_DAC_CHANNELS 0


const int colors[NColors] = { 
   VAL_DK_RED,   
   VAL_RED, 
   VAL_ORANGE,
   VAL_MAGENTA,
   VAL_DK_YELLOW,    
   0x0000CD00, //VAL_GREEN,
   VAL_DK_GREEN,          
   VAL_DK_CYAN,                     
   VAL_BLUE, 
   VAL_DK_BLUE,                     
   VAL_DK_MAGENTA,     
   VAL_DK_GRAY3 //VAL_DK_GRAY
};


extern int globalDigitalValues[N_TOTAL_DIO_CHANNELS];
extern double globalAnalogVoltages[N_DAC_CHANNELS];



const char str_None[] = "NONE";


extern int panelMain;
extern int panelMain2;

extern int panelMainInfo;
extern int panelSequence;

int panelWfmSmall = -1;
int panelVariableTime    	= -1;  // small poupu window "variable time"
int panelVariablePulse		= -1;
int panelLoop = -1;


int menuBlock			 = -1;     // popup menu 
								   // (appears when right click on block no.)
int menuChannelProperties = -1;
int menuDACChannelProperties = -1;
int menuPulseProperties = -1;
int menuDacProperties = -1;

ListType blockClipboard = NULL;
ListType blockClipboardWaveforms = NULL;

int nextGroupNr = 1;
int sendingTrigger = 0;


enum {
	BLOCK_TABLE_COL_directIO = 1,
	BLOCK_TABLE_COL_DIOName,
	BLOCK_TABLE_COL_channelName,

	BLOCK_TABLE_NspecialCols
};
	

int BLOCK_TABLE_COL_widths[BLOCK_TABLE_NspecialCols] =  {
	0,
	45,
	45,
	150,
	
	} ;
	

#define BLOCK_TABLE_COL_WIDTH_normal 50
#define BLOCK_TABLE_COL_WIDTH_inGroup 1

enum {
	BLOCK_TABLE_HEADER_ROW_blockNo = 1,
//	BLOCK_TABLE_HEADER_ROW_properties,
	BLOCK_TABLE_HEADER_ROW_name,
	BLOCK_TABLE_HEADER_ROW_duration,
	BLOCK_TABLE_HEADER_ROW_absTime,
	BLOCK_TABLE_HEADER_ROW_gpib,

	BLOCK_TABLE_HEADER_NRows
};

#define TABLE_ROW_HEIGHT_HEADER_blockNo 25
#define TABLE_ROW_HEIGHT_HEADER_name 40
#define TABLE_ROW_HEIGHT_HEADER_duration 25
#define TABLE_ROW_HEIGHT_HEADER_absTime 15
#define TABLE_ROW_HEIGHT_HEADER_gpib 14



#define BLOCK_COL_invisible -2

#define TABLE_COLOR_channel_inactive  VAL_PANEL_GRAY
#define TABLE_COLOR_directIO_inactive VAL_PANEL_GRAY
#define TABLE_COLOR_channel_alwaysOn  VAL_LT_GRAY
#define TABLE_COLOR_channel_alwaysOff VAL_MED_GRAY
#define TABLE_COLOR_header_bkgrnd VAL_MED_GRAY

#define TABLE_COLOR_blockDisabled VAL_MED_GRAY



const int TABLE_COLOR_BUTTON_INACTIVE [2] // even and odd channels 
= {
	VAL_LT_GRAY,
	VAL_LT_GRAY2
};


const int TABLE_COLOR_BUTTON_INACTIVE_LOCKED [2] // even and odd channels 
= {
	VAL_LT_GRAY4,
	VAL_LT_GRAY3
};


const int TABLE_COLOR_BUTTON_INACTIVE_DAC [2] // even and odd channels 
= {
	VAL_LT_GRAY,
	VAL_DK_GRAY2
};


const int TABLE_COLOR_header_bkgrnd_groups[2] = {
	VAL_BLUE,
	VAL_GREEN
};


enum {
	DAC_DISPLAYMODE_name,
	DAC_DISPLAYMODE_compact,
	DAC_DISPLAYMODE_graph
} ;


// ------------------------------------------
//    memorizes the block for which
//    the variable duration is edited
// -----------------------------------------
t_digitalBlock *variableTimeBlock  = NULL;  
t_digitalBlock *loopBlock  = NULL;     
int variableTimeChannel = -1;


// ------------------------------------------
//    controls
// -----------------------------------------



void DIGITALBLOCKS_insertTableColumns (int panel, int ctrlHeader, int ctrlDac, 
									  int ctrlDigital, int startCol, int nColumns);


void DIGITALBLOCKS_deleteTableColumns (int panel, int ctrlHeader, int ctrlDac, 
									  int ctrlDigital, int startCol, int nColumns);

void DIGITALBLOCKS_displayInTable (t_sequence *seq, int blockNr, int panel, 
								  int ctrlHeader, int ctrlDac, int ctrlDigital);

void TABLE_initAll (void);


//void DIGITALBLOCKS_displayDIOchannelsAlwaysOnOff (t_sequence *seq);

void DIGITALBLOCKS_displayAllAbsoluteTimes(t_sequence *seq);

int DIGITALBLOCKS_isFirstOfCompressedGroup (t_digitalBlock *b);

void DIGITALBLOCKS_displayDigitalChannel_fast (t_sequence *seq, t_digitalBlock **bl, int nBlocks, int panel, int ctrlDigital, int channel);



const char *arrow (int value) 
{
	 if (value < 0) return "<--";
	 if (value > 0) return "-->";
	 return "   ";
}



/************************************************************************/
/************************************************************************/
/*
/*    Initializaiton of panels, controls etc. 
/*
/************************************************************************/
/************************************************************************/


void DIGITALBLOCKS_setTableComlumWidth_channelNames (void)
{
	int width, totalWidth;
	int nameWidth;
	int i;
	
	totalWidth = 0;
	for (i = 1; i < BLOCK_TABLE_COL_channelName; i++) {
		GetTableColumnAttribute (panelSequence, SEQUENCE_TABLE_header1, i, ATTR_COLUMN_ACTUAL_WIDTH,
								 &width);
		totalWidth += width;
	}
 
	nameWidth = config->splitterVPos - totalWidth - 6;
	SetTableColumnAttribute (panelSequence, SEQUENCE_TABLE_header1,
							 BLOCK_TABLE_COL_channelName, ATTR_COLUMN_WIDTH,
							 nameWidth);
	SetTableColumnAttribute (panelSequence, SEQUENCE_TABLE_dac1,
							 BLOCK_TABLE_COL_channelName, ATTR_COLUMN_WIDTH,
							 nameWidth);
	SetTableColumnAttribute (panelSequence, SEQUENCE_TABLE_digital1,
							 BLOCK_TABLE_COL_channelName, ATTR_COLUMN_WIDTH,
							 nameWidth);

}



void DIGITALBLOCKS_resizeProgressBar (void)
{
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERICSLIDE_progress, ATTR_LEFT, 
					  ctrlLeft (panelSequence, SEQUENCE_TABLE_dac2) + 100);
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERICSLIDE_progress, ATTR_TOP, 
					  ctrlTop (panelSequence, SEQUENCE_TABLE_dac2)-ctrlHeight (panelSequence,SEQUENCE_NUMERICSLIDE_progress)+4 );
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERICSLIDE_progress, ATTR_WIDTH, 
					  ctrlWidth (panelSequence, SEQUENCE_TABLE_dac2));
	
}

//=======================================================================
//
//    resize panel "Digital Blocks"
//
//=======================================================================

void DIGITALBLOCKS_resizePanel (void)
{
	//int nVisibleDACRows_default = 4;
	int width1;
	int bottomMax, topPos;
	

	width1 = BLOCK_TABLE_COL_widths[BLOCK_TABLE_COL_directIO] 
						+ BLOCK_TABLE_COL_widths[BLOCK_TABLE_COL_DIOName] 
						+ BLOCK_TABLE_COL_widths[BLOCK_TABLE_COL_channelName] + 6;

	if (config->splitterVPos == 0) {
		config->splitterVPos = width1;
	}
	
	// size and position of header  1
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header1,
					  ATTR_NUM_VISIBLE_ROWS, BLOCK_TABLE_HEADER_NRows-1);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header1,
					  ATTR_LEFT, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header1,
					  ATTR_TOP, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header1,
					  ATTR_WIDTH, config->splitterVPos);
	
	// splitter vertical:
//	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER, ATTR_FRAME_COLOR,
//					  VAL_DK_CYAN);
	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER,
					  ATTR_LEFT, config->splitterVPos);
	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER,
					  ATTR_TOP, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER,
					  ATTR_HEIGHT, panelHeight (panelSequence));
	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER,
					  ATTR_RIGHT_RANGE, 350);
	SetCtrlAttribute (panelSequence, SEQUENCE_VSPLITTER,
					  ATTR_LEFT_RANGE, max(0,width1 - 50));
	

	// size and position of header  2
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2,
					  ATTR_NUM_VISIBLE_ROWS, BLOCK_TABLE_HEADER_NRows-1);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2,
					  ATTR_LEFT, ctrlRight(panelSequence, SEQUENCE_VSPLITTER));
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2,
					  ATTR_TOP, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2,
					  ATTR_WIDTH, max(0,panelWidth (panelSequence) - config->splitterVPos - VAL_LARGE_SCROLL_BARS));

	
	// dac table  1
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac1,
					  ATTR_LEFT, 1); // Left whole table 
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac1,
					  ATTR_TOP, ctrlBottom (panelSequence, SEQUENCE_TABLE_header1) + 10); // Top Whole table
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac1,
					  ATTR_WIDTH, config->splitterVPos); // Whole table width 

	// timing table
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2,
					  ATTR_LEFT, ctrlRight(panelSequence, SEQUENCE_VSPLITTER) + 10); // Left whole table
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2,
					  ATTR_TOP, ctrlBottom (panelSequence, SEQUENCE_TABLE_header1) + 10); //Top whole table
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2,
					  ATTR_WIDTH, max(0,panelWidth (panelSequence) - config->splitterVPos - 200));
	
	if (config->splitterHPos > 0) {
		SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac1, ATTR_HEIGHT, config->splitterHPos);
		SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HEIGHT, 
						 min(config->splitterHPos, SPLITTER_BOTTOM_POS(config->buttonHeight)-5));
	}
	else {
		SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HEIGHT, 
					 	  SPLITTER_BOTTOM_POS(config->buttonHeight)-5);
		GetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HEIGHT, &config->splitterHPos);
	}

	// splitter  horiz
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER, ATTR_FRAME_COLOR,
					  VAL_DK_CYAN);
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_LEFT, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_TOP, ctrlBottom (panelSequence, SEQUENCE_TABLE_dac1));
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_WIDTH, panelWidth(panelSequence));
	bottomMax = ctrlTop(panelSequence, SEQUENCE_TABLE_dac2)+SPLITTER_BOTTOM_POS(config->buttonHeight);
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_BOTTOM_RANGE, bottomMax);

	GetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_TOP, &topPos);
	SetCtrlAttribute (panelSequence, SEQUENCE_SPLITTER,
					  ATTR_TOP, min(topPos,bottomMax-5));

	
	// table for digital Channels 1
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital1, ATTR_LEFT, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital1, ATTR_TOP,
					  ctrlBottom (panelSequence, SEQUENCE_SPLITTER));  
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital1, ATTR_WIDTH,
					  config->splitterVPos ); 
	SetPanelAttribute(panelSequence, ATTR_CONFORM_TO_SYSTEM_THEME, 0); // for color of buttons
	
	
	// table for digital Channels 2
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_LEFT, ctrlRight(panelSequence, SEQUENCE_VSPLITTER));
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_TOP,
					  ctrlBottom (panelSequence, SEQUENCE_SPLITTER));  
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_WIDTH,
					  max(0,panelWidth (panelSequence) -config->splitterVPos));  
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_HIDE_HILITE, 1);

	
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_HEIGHT, 
		max(0,panelHeight (panelSequence) - ctrlTop (panelSequence, SEQUENCE_TABLE_digital2)));
	SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital1, ATTR_HEIGHT, 
		max(0,ctrlHeight(panelSequence, SEQUENCE_TABLE_digital2) - VAL_LARGE_SCROLL_BARS));

	// avoid editing
	//SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_TABLE_RUN_STATE, VAL_SELECT_STATE);
	
	DIGITALBLOCKS_setTableComlumWidth_channelNames ();
	
	DIGITALBLOCKS_resizeProgressBar ();

	

}



void DIGITALBLOCKS_insertGpibCommandsToMenu (t_sequence *seq, int gpibCommandNr)
{
	static int subMenu = MENU3_BLKEDIT_GPIB_SUBMENU;
	t_gpibCommand *g;
	int i;
	
	DiscardSubMenu (menuBlock, subMenu);
	subMenu = NewSubMenu (menuBlock, MENU3_BLKEDIT_GPIB);
	NewMenuItem (menuBlock, subMenu, GPIB_COMMANDNAME_NONE_STR, -1, 0, 0, 0);
	for (i = 1; i <= ListNumItems (seq->lGpibCommands); i++) {
		ListGetItem (seq->lGpibCommands, &g, i);
		if (g->transmitAfterStartOfSequence && g->commandType != GPIB_LOOP_TRIGGER && g->commandType != GPIB_RECV_LOOP_TRIGGER) {
			g->menuID = NewMenuItem (menuBlock, subMenu, g->name, -1, 0, 0, 0);
			// SetMenuBarAttribute (menuBlock, g->menuID, ATTR_CHECKED, i == gpibCommandNr);
		}
	}
}

void DIGITALBLOCKS_insertWaveformsToMenu (t_sequence *seq, int channel)
{
	static int subMenu = MENU4_DAC_WAVEFORM_SUBMENU;
	t_waveform *wfm;
	int i;
	int err;
	
	if (subMenu != 0) {
		err = DiscardSubMenu (menuDacProperties, subMenu);
		if (err) {
			tprintf("Error discarding submenu\n");	
		}
	}
	subMenu = NewSubMenu (menuDacProperties, MENU4_DAC_WAVEFORM);
	for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
		ListGetItem (seq->lWaveforms, &wfm, i);
		if ((wfm->channel == channel) || (wfm->channel == WFM_CHANNEL_ALL)) {
			wfm->menuID = NewMenuItem (menuDacProperties, subMenu, wfm->name, -1,
								   0, 0, 0);
		} 
		else wfm->menuID = 0;
	}
}

	



t_digitalBlock *DIGITALBLOCKS_getFirstOfGroup (t_sequence *seq, int nr)
{
	t_digitalBlock *p1, *p2;
	int i;
	
	p1 = DIGITALBLOCK_ptr (seq, nr);
	if (p1 == NULL) return NULL;
	
	for (i = nr-1; i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &p2, i);
		if (p2->groupNr != p1->groupNr) return p1;
		p1 = p2;
	}
	if (p1->groupNr != 0) return p1;
	
	return NULL;
}



int DIGITALBLOCKS_createBitmapForChannel (t_sequence *seq, t_digitalBlock *start, int ch, int panel, int ctrl, int analogCh)
{

	t_digitalBlock *b;
	int i;
	double xLeft, xWidth;
	int state = 0; // TODO check if zero is ok
	unsigned durationStart;
	int color;
	int drawEqualSizes = 1;
	int error;
	
	if (start->bitmapPtrArr == NULL) DIGITALBLOCK_createGroupBitmapPtr (start);

	CanvasStartBatchDraw (panel, ctrl);
	//SetCtrlAttribute(panel,ctrl,ATTR_PICT_BGCOLOR,VAL_OFFWHITE); // already set at creation
	CanvasClear (panel, ctrl, VAL_ENTIRE_OBJECT);
	
	/*SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_OFFWHITE);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_OFFWHITE);
	CanvasDrawRect (panel, ctrl,
					MakeRect(0,0,config->buttonHeight, start->tableColumnWidth),
					VAL_DRAW_FRAME_AND_INTERIOR);
    */
	//SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, colors[ch % NColors]);
	b = start;
	durationStart = 0;
	if (analogCh) color = VAL_COLOR_DAC_WAVEFORMS;
	else color = colors[ch % NColors];
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, color );
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, color );
	for (i = start->blockNr; i < start->blockNr+start->nElementsInGroup; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (analogCh) state = b->waveforms[ch] != 0; 
		else {
			switch (seq->DIO_channelStatus[ch]) {
				case DIO_CHANNELSTATUS_ALWAYSON: 
					state = !seq->DIO_invertDisplay[ch];
					break;
				case DIO_CHANNELSTATUS_ALWAYSOFF:
					state = seq->DIO_invertDisplay[ch];
					break;
				case DIO_CHANNELSTATUS_STANDARD:
					state = b->channels[ch] ^ seq->DIO_invertDisplay[ch];
					break;
			}
		}
		if (state) {
			if (drawEqualSizes) {
				xWidth = (1.0*start->tableColumnWidth) / (1.0*start->nElementsInGroup);
				xLeft = (i-start->blockNr) * xWidth;
			}
			else if (start->groupDuration != 0) {
				xLeft = ((1.0*durationStart) * start->tableColumnWidth) / ui64ToDouble (start->groupDuration);
				xWidth = ((1.0*b->duration) * start->tableColumnWidth) / ui64ToDouble (start->groupDuration);
			} else{
				xLeft = 0;
				xWidth = 0;
			}
			CanvasDrawRect (panel, ctrl,
							MakeRect (0, xLeft, 1/*config->buttonHeight*/, xWidth+1),
							VAL_DRAW_FRAME_AND_INTERIOR);
		}
		durationStart += b->duration;
	}
	CanvasEndBatchDraw (panel, ctrl);
	
	
	if (analogCh) ch += N_TOTAL_DIO_CHANNELS;
	if (start->bitmapPtrArr[ch] > 0) {
		DiscardBitmap (start->bitmapPtrArr[ch]);
		start->bitmapPtrArr[ch] = 0;
	}
	error = GetCtrlBitmap (panel, ctrl, 0, &start->bitmapPtrArr[ch]);
	return start->bitmapPtrArr[ch];
}

	


void DIGITALBLOCKS_createBitmaps (t_sequence *seq, t_digitalBlock *b, int DACchannel, int DIOchannel)
{
	static int canvas = 0;
	int i;
	int bid;
	
	

	if ((b->groupNr > 0) && (!b->isGroupExpanded) && (b->nElementsInGroup > 0)) {
		DIGITALBLOCK_createGroupBitmapPtr (b);  
		if (canvas == 0) {	
			canvas = NewCtrl (panelSequence, CTRL_CANVAS, "", 5000, 2000);
			SetCtrlAttribute(panelSequence,canvas,ATTR_PICT_BGCOLOR,VAL_OFFWHITE);
		}
		SetCtrlAttribute (panelSequence, canvas, ATTR_HEIGHT, 1 /*config->buttonHeight*/); 
		SetCtrlAttribute (panelSequence, canvas, ATTR_YSCALING, (double)config->buttonHeight);// not clear if it saves space to set height=1 if yscale is large instead
		SetCtrlAttribute (panelSequence, canvas, ATTR_WIDTH,b->tableColumnWidth);
//		ProcessDrawEvents();
		if (DACchannel >= 0) {
			DIGITALBLOCKS_createBitmapForChannel (seq, b, DACchannel, panelSequence, canvas, 1);
	    }
		if (DACchannel == -2) { 
			for (i = 0; i < N_DAC_CHANNELS; i++) {
				DIGITALBLOCKS_createBitmapForChannel (seq, b, i, panelSequence, canvas, 1);
			}
		}	
		
			
		if (DIOchannel >= 0) {
			DIGITALBLOCKS_createBitmapForChannel (seq, b, DIOchannel, panelSequence, canvas, 0);
		}
		if (DIOchannel == -2) {
			for (i = 0; i < seq->maxDigitalChannel; i++) {
				bid = DIGITALBLOCKS_createBitmapForChannel (seq, b, i, panelSequence, canvas, 0);
			}
#ifdef _CVI_DEBUG_        	
//			tprintf ("all digital BMPS created. last ID = %d", bid);
#endif			

		}	

	}
}




void DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (t_sequence *seq)
{
	int nBlocks;
	//int column;
	t_digitalBlock *b;
	int i;
	int newGroupNr = 1;
	int groupNr;
	t_digitalBlock *firstOfGroup;
	//int groupDuration = 0;
	
	nBlocks = ListNumItems (seq->lDigitalBlocks);

	for (i = 1; i <= nBlocks; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		b->blockNr = i;
		b->tableColumnWidth = BLOCK_TABLE_COL_WIDTH_normal;
		firstOfGroup = b;
		firstOfGroup->groupDuration = b->duration;
		firstOfGroup->nElementsInGroup = 0;
		if (b->groupNr != 0) {
			groupNr = b->groupNr;
			while ((b->groupNr == groupNr) && (i <= nBlocks)) {
				b->groupNr = newGroupNr;
				i++;
				if (i <= nBlocks) {
					ListGetItem (seq->lDigitalBlocks, &b, i);
					b->blockNr = i;
					b->nElementsInGroup = 0;
					if ((b->groupNr == groupNr) && (!firstOfGroup->isGroupExpanded)) b->tableColumnWidth = BLOCK_TABLE_COL_WIDTH_inGroup;
					else b->tableColumnWidth = BLOCK_TABLE_COL_WIDTH_normal;
					if (b->groupNr == groupNr) 
						firstOfGroup->groupDuration += b->duration;
				}
			}
			firstOfGroup->nElementsInGroup = i - firstOfGroup->blockNr;
			if (firstOfGroup->nElementsInGroup == 1) {
				// group consists of 1 element only? --> delete group
				firstOfGroup->groupNr = 0;
				firstOfGroup->isGroupExpanded = 0;
				firstOfGroup->nElementsInGroup = 0;
			}
			else {
				if (!firstOfGroup->isGroupExpanded) firstOfGroup->tableColumnWidth = BLOCK_TABLE_COL_WIDTH_normal + 20;
			}
			newGroupNr++;
			if (i <= nBlocks) 
				i--;
		}
	}
}





void DIGITALBLOCKS_smallWfmInitPanel (void)
{
	int height;
	
	if (panelWfmSmall > 0) return;
	
	panelWfmSmall = LoadPanel (panelSequence, UIR_File, WFMSMALL);
	SetCtrlAttribute (panelWfmSmall, WFMSMALL_STRING_name, ATTR_WIDTH, panelWidth (panelWfmSmall));
	height = ctrlHeight (panelWfmSmall, WFMSMALL_STRING_name);
	setCtrlBoundingRect (panelWfmSmall, WFMSMALL_GRAPH_WFM, 
						 height, 0, 
						 panelHeight(panelWfmSmall)-height,
						 panelWidth(panelWfmSmall));
	SetCtrlAttribute (panelWfmSmall, WFMSMALL_GRAPH_WFM,
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
	enableCommaInNumeric (panelWfmSmall, WFMSMALL_NUMERIC_stepFrom);
	enableCommaInNumeric (panelWfmSmall, WFMSMALL_NUMERIC_stepTo);
	SetCtrlAttribute (panelWfmSmall, WFMSMALL_NUMERIC_stepFrom,
					  ATTR_AUTO_EDIT, 1);
}									 



void DIGITALBLOCKS_smallWfmShowPanel (int top, int left, t_sequence *seq, int wfmNr)
{
	t_waveform *wfm, *oldWfm;
	int visible;
	
	wfm = WFM_ptr (seq, wfmNr);
	if (wfm == NULL) return;
	SetAttributeForCtrls (panelWfmSmall, ATTR_VISIBLE,
						  (wfm->type == WFM_TYPE_POINTS)|| (wfm->type == WFM_TYPE_FILE)|| (wfm->type == WFM_TYPE_FUNCTION),
						  0, WFMSMALL_GRAPH_WFM, WFMSMALL_STRING_name, 0);

	SetAttributeForCtrls (panelWfmSmall, ATTR_VISIBLE,
						  wfm->type == WFM_TYPE_STEP, 0,
						  WFMSMALL_NUMERIC_stepFrom,
						  WFMSMALL_NUMERIC_stepTo,
						  WFMSMALL_NUMERIC_stepRep,
						  WFMSMALL_TEXTMSG_stepTitle,
						  WFMSMALL_TEXTMSG_arrow,
						  WFMSMALL_TEXTMSG_repetitions,
						  WFMSMALL_COMMANDBUTTON_done,
						  0);
    switch (wfm->type) {
        case WFM_TYPE_STEP:
			SetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepFrom, wfm->stepFrom);
			SetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepTo,   wfm->stepTo);
			SetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepRep,  wfm->stepRep);
			break;
		case WFM_TYPE_POINTS:
		case WFM_TYPE_FILE:
		case WFM_TYPE_FUNCTION:
			WFM_plot (panelWfmSmall, WFMSMALL_GRAPH_WFM, seq, wfm, NULL, 0);
			SetCtrlVal (panelWfmSmall, WFMSMALL_STRING_name, wfm->name);
			break;
		case WFM_TYPE_CONSTVOLTAGE:
			break;
		
	}	
	SetPanelAttribute (panelWfmSmall, ATTR_TITLE, wfm->name);
	
	
	GetPanelAttribute (panelWfmSmall, ATTR_CALLBACK_DATA, &oldWfm);
	GetPanelAttribute (panelWfmSmall, ATTR_VISIBLE, &visible);
	if ((oldWfm != wfm) || !visible) SetPanelPos (panelWfmSmall, top, left);

	SetPanelAttribute (panelWfmSmall, ATTR_CALLBACK_DATA, wfm);
	DisplayPanel (panelWfmSmall);
}


//=======================================================================
//
//    Init panel "Digital Blocks"
//
//=======================================================================
int DIGITALBLOCKS_initPanel (void)
{
 	// -----------------------------------------
	//    load panels & menus
	// -----------------------------------------
	if (menuChannelProperties <= 0) {
		menuChannelProperties = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL1));
		if (menuChannelProperties < 0) return -1;

		menuDACChannelProperties = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL7));
		if (menuDACChannelProperties < 0) return -1;

		
		menuPulseProperties  = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL2));
		if (menuPulseProperties  < 0) return -1;

		menuBlock = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL3));
		if (menuBlock < 0) return -1;

	
		menuDacProperties = GetPanelMenuBar (LoadPanel (0, UIR_File, MENUPANEL4));
		if (menuDacProperties < 0) return -1;
	
	
		if (panelSequence == 0) {
			GetPanelHandleFromTabPage (panelMain2, MAIN2_TAB0, 0, &panelSequence);
		}
	}
	
	TABLE_initAll  ();
	
	SetCtrlAttribute (panelSequence, SEQUENCE_DECORATION1, ATTR_LEFT, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_DECORATION1, ATTR_TOP, 0);
	SetCtrlAttribute (panelSequence, SEQUENCE_DECORATION1, ATTR_WIDTH, 
					  iArraySum (BLOCK_TABLE_COL_widths, 1,BLOCK_TABLE_COL_DIOName)+30); //BLOCK_TABLE_COL_DIOName);
	SetCtrlAttribute (panelSequence, SEQUENCE_DECORATION1, ATTR_HEIGHT, 
					  ctrlHeight (panelSequence, SEQUENCE_TABLE_header1));
	
	// start+stop button
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence, ATTR_LEFT,4);
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence, ATTR_WIDTH,
					  ctrlWidth (panelSequence, SEQUENCE_DECORATION1)-10);
	//SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence, ATTR_TOP, 10);

	
	
  	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_startSequence,
	                 ATTR_CMD_BUTTON_COLOR, VAL_GREEN);

//	copyCtrlBoundingRect (panelSequence, SEQUENCE_BTN_startSequence,
//					      panelSequence, SEQUENCE_DECORATION1);
	copyCtrlBoundingRect (panelSequence, SEQUENCE_BTN_stopSequence,
						  panelSequence, SEQUENCE_BTN_startSequence);	 
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_stopSequence,
	                 ATTR_CMD_BUTTON_COLOR, VAL_RED);
	SetCtrlAttribute (panelSequence, SEQUENCE_BTN_stopSequence,
	                 ATTR_VISIBLE, 0);
	
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_rep, ATTR_TOP, ctrlBottom (panelSequence, SEQUENCE_BTN_startSequence)); 
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_rep, ATTR_LEFT, ctrlLeft (panelSequence, SEQUENCE_BTN_startSequence)); 
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_repOf, ATTR_TOP, ctrlBottom (panelSequence, SEQUENCE_BTN_startSequence)); 
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_repOf, ATTR_LEFT, ctrlRight (panelSequence, SEQUENCE_BTN_startSequence)-
		ctrlWidth (panelSequence, SEQUENCE_NUMERIC_repOf)); 
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_rep, ATTR_TEXT_BGCOLOR, VAL_YELLOW);
	SetCtrlAttribute (panelSequence, SEQUENCE_NUMERIC_repOf, ATTR_TEXT_BGCOLOR, VAL_YELLOW);

	
	
	SetCtrlAttribute (panelSequence, SEQUENCE_SLIDE_progress2, ATTR_TOP,
					  ctrlBottom (panelSequence, SEQUENCE_NUMERIC_repOf));
	SetCtrlAttribute (panelSequence, SEQUENCE_SLIDE_progress2, ATTR_LEFT,
					  ctrlLeft (panelSequence, SEQUENCE_BTN_startSequence)-2);
	SetCtrlAttribute (panelSequence, SEQUENCE_SLIDE_progress2, ATTR_WIDTH,
					  ctrlWidth (panelSequence, SEQUENCE_BTN_startSequence)+4);

	

	DIGITALBLOCKS_smallWfmInitPanel ();
	
    if (panelVariableTime < 0) 
    	panelVariableTime = LoadPanel (panelSequence, UIR_File, VARTIME);

	return 0;
}






void DIGITALBLOCKS_flipChannel (t_sequence *seq, int ch)
{
	int i;
    t_digitalBlock *b;
	int nBlocks;

	
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	if (nBlocks == 0) return;

	for (i = 1; i <= nBlocks; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->channels[ch] == 0) b->channels[ch] = 1;
		else if (b->channels[ch] == 1) b->channels[ch] = 0;
	}
	
	if (seq->DIO_channelStatus[ch] == DIO_CHANNELSTATUS_ALWAYSON) seq->DIO_channelStatus[ch] = DIO_CHANNELSTATUS_ALWAYSOFF;
	else if (seq->DIO_channelStatus[ch] == DIO_CHANNELSTATUS_ALWAYSOFF) seq->DIO_channelStatus[ch] = DIO_CHANNELSTATUS_ALWAYSON;
	
}





//=======================================================================
//
//    displays all digital channel names 
//
//=======================================================================

void DIGITALBLOCKS_displayChannelNames (t_sequence *seq)
{
	int i;
	int color;
	Point cell;

	
	for (i = 0; i < seq->maxDigitalChannel; i++) {
		cell = MakePoint (BLOCK_TABLE_COL_channelName, i+1);
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital1, cell, ATTR_CTRL_VAL, 
							   seq->DIO_channelNames[i] == NULL ? "" : seq->DIO_channelNames[i]);
		
		if (seq->DIO_lockChannel[i])  color = VAL_LT_BLUE1; //TABLE_COLOR_BUTTON_INACTIVE_LOCKED[i%2];
		else color = TABLE_COLOR_BUTTON_INACTIVE[i%2];
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital1, cell, ATTR_TEXT_BGCOLOR,
							   color);	
		
		
		cell = MakePoint (BLOCK_TABLE_COL_DIOName, i+1);
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital1, cell,
							   ATTR_TEXT_BGCOLOR,
							   seq->DIO_invertDisplay[i] ? VAL_CYAN : TABLE_COLOR_BUTTON_INACTIVE[i%2]);
	}
	 tprintf("%d\n",N_DAC_CHANNELS);   
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		
		cell = MakePoint (BLOCK_TABLE_COL_channelName, i+1);
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_dac1, cell,
							   ATTR_TEXT_BGCOLOR,
							   TABLE_COLOR_BUTTON_INACTIVE[i%2]);

		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_dac1, cell, ATTR_CTRL_VAL, 
							   seq->AO_channelNames[i] == NULL ? "" : seq->AO_channelNames[i]);
		
	}
		  
	
}
    

       
//=======================================================================
//
//    sets direct IO Buttons to the values specified in "b"  
//
//=======================================================================

void DIGITALBLOCKS_displayDirectIOButton (t_sequence *seq, int channel)
{
    int state;
	int color;
	
	state = globalDigitalValues[channel];
	if (seq != NULL) state ^= seq->DIO_invertDisplay[channel];
	
	if (state == 1) {
		color = colors[channel % NColors];
	}
	else {
		color = TABLE_COLOR_BUTTON_INACTIVE[channel%2];
	}
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital1,
						   MakePoint (BLOCK_TABLE_COL_directIO, channel+1),
						   ATTR_CMD_BUTTON_COLOR, color);
}




void DIGITALBLOCKS_displayAllDirectIOButtons (t_sequence *seq)
{
    int i;
	//int bit;
    
//    if (channels == NULL) {
//    	nChannels = seq->maxDigitalChannel;
//    	channels = lastDigitalValues();
//    }
    for (i = 0; i < seq->maxDigitalChannel; i++) {
//        if (i < nChannels) bit = channels[i];
//        else bit = 0;
//        SetCtrlAttribute (panelDigitalBlocks, CTRL_FastOnOff[i], ATTR_CTRL_VAL, bit);		
		DIGITALBLOCKS_displayDirectIOButton (seq, i);
    }
	
}




/************************************************************************/
/************************************************************************/
/*
/*    Tools, useful routines etc 
/*
/************************************************************************/
/************************************************************************/




//=======================================================================
//
//    conversion of block durations (double  <--> unsigned long)
//
//=======================================================================

//  duration in units of 50ns
unsigned long timeToInt(double t)
{
	t = fabs(t);
	t = min(t, 200000.0);
	return t * (1.0*VAL_ms) + 0.5;
}


// returns durtation (in units of 50ns) as a double
double ulToTime (unsigned __int64 time)
{
//    return -1.0
	unsigned low, high;
	unsigned __int64 low64, high64;
//	unsigned __int64 max32;
	
//	max32 = 0x100000000;
	low64 = time % POW2_32;
	high64  = time / POW2_32;
	high = (unsigned)high64;
	low = (unsigned)low64;
	
	return VAL_FLOAT_MIN_s * 1000.0 * (low + high*DPOW2_32);
}






//=======================================================================
//
//    display a time in ms 
//    and automatically adjust number of digits 
//
//=======================================================================

void setTime (int panel, int ctrl, unsigned __int64 uLtime, int sign)
{              
    SetCtrlAttribute (panel, ctrl, ATTR_CTRL_VAL, ulToTime(uLtime)*sign);
    
    if ((uLtime % VAL_ms) == 0)
        SetCtrlAttribute (panel, ctrl, ATTR_PRECISION, 1);
    else
    if ((uLtime % VAL_us) == 0)
        SetCtrlAttribute (panel, ctrl, ATTR_PRECISION, 3);
    else
    	SetCtrlAttribute (panel, ctrl, ATTR_PRECISION, 5); 
}


void setTime2 (int panel, int ctrl, unsigned __int64 uLtime, int sign)
{              
    int prec;
	
	SetCtrlAttribute (panel, ctrl, ATTR_CTRL_VAL, ulToTime(uLtime)*sign);
    
	prec = 0;
    if ((uLtime % VAL_ms) == 0) prec = 1;
    else
    if ((uLtime % VAL_us) == 0) prec = 3;
    else prec = 5;
	if (uLtime / VAL_ms > 1000) prec = 0;
	
    SetCtrlAttribute (panel, ctrl, ATTR_PRECISION, prec); 
}




void setTimeTableCell (int panel, int ctrl, Point cell, unsigned __int64 uLtime, int sign)
{              
    SetTableCellAttribute (panel, ctrl, cell, ATTR_CTRL_VAL, ulToTime(uLtime)*sign);
    
    if ((uLtime % VAL_ms) == 0)
        SetTableCellAttribute (panel, ctrl, cell, ATTR_PRECISION, 1);
    else
    if ((uLtime % VAL_us) == 0)
        SetTableCellAttribute (panel, ctrl, cell, ATTR_PRECISION, 3);
    else
    	SetTableCellAttribute (panel, ctrl, cell, ATTR_PRECISION, 5); 
	
}


//=======================================================================
//
//    display a "double" as a time (ms)
//
//=======================================================================
void setDoubleTime (int panel, int ctrl, double dTime)
{              
    setTime (panel, ctrl, timeToInt(dTime), sign(dTime));
}







void DIGITALBLOCKS_displayGlobalAnalogVoltages (void)
{
	Point cell;
	int i;
	
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		cell = MakePoint (BLOCK_TABLE_COL_directIO, i+1);
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_dac1, cell, ATTR_CTRL_VAL,
						       globalAnalogVoltages[i]);
	}
}



// display single analog channel cell
void DIGITALBLOCKS_displayDACChannelInTable (int panel, int ctrlDac, t_sequence *seq, t_digitalBlock *b, int i)
{
	Point cell;
	t_waveform *wfm;
	int offColor;
	int bgcolor;
	int dimmed;
	int textBgColor;
	int cellMode;
	int cellJustify;
	

	
	cell = MakePoint (b->blockNr, i+1);
	if (DIGITALBLOCKS_isFirstOfCompressedGroup (b)) {
		SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_TYPE,
						      VAL_CELL_PICTURE);
		if (b->bitmapPtrArr == NULL) DIGITALBLOCKS_createBitmaps (seq, b, i, -1);
		
		if (b->bitmapPtrArr != NULL)  {
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CTRL_VAL,
								   b->bitmapPtrArr[i+N_TOTAL_DIO_CHANNELS]);
		}
		return;
		
	}

	dimmed =  b->disable; //(b->waveforms[i] == WFM_ID_UNCHANGED) || 
	SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_DIMMED, dimmed);
	wfm = WFM_ptr (seq, b->waveforms[i]);
	bgcolor = WFM_hasVariableParameters(wfm) ? VAL_ORANGE : VAL_COLOR_DAC_WAVEFORMS;
//	bgcolor = WFM_hasVariableParameters(wfm) ? VAL_ORANGE : colors[i % NColors];
	cellMode = VAL_INDICATOR;

	switch (b->waveforms[i]) {
		case WFM_ID_HOLD:
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_TYPE,
									   VAL_CELL_STRING);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CTRL_VAL,
								   strHold);
			cellJustify = VAL_CENTER_CENTER_JUSTIFIED;
			textBgColor = VAL_COLOR_DAC_HOLD;
			break;
		case WFM_ID_CONSTVOLTAGE:
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_TYPE,
							   VAL_CELL_NUMERIC);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_DATA_TYPE,
								   VAL_DOUBLE);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_FORMAT,
								   VAL_FLOATING_PT_FORMAT);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_PRECISION, 4);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_MIN_VALUE, -10.0);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_MAX_VALUE, 10.0);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CHECK_RANGE,
								   VAL_COERCE);
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_INCR_VALUE, 0.1);
			cellJustify = VAL_CENTER_RIGHT_JUSTIFIED;
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CTRL_VAL,
								   b->constVoltages[i]);
			textBgColor =  VAL_COLOR_DAC_CONSTVOLATGE;
			cellMode = VAL_HOT;
			break;
		default:
			//offColor = TABLE_COLOR_BUTTON_INACTIVE_DAC[(i+1)%2]; colors[i % NColors]; // //(b->groupNr > 0) ? VAL_OFFWHITE : 
			if (COLOR_DAC_CHANNELS) {
				offColor = lightenColor(colors[i % NColors],0.7f);
			} else {
				offColor = TABLE_COLOR_BUTTON_INACTIVE_DAC[(i+1)%2];         
			}
			
			textBgColor = (b->waveforms[i] == WFM_ID_UNCHANGED) ? offColor : bgcolor;
			
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_TYPE,
									   VAL_CELL_STRING);
			cellJustify = VAL_CENTER_LEFT_JUSTIFIED;
			SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CTRL_VAL,
								   wfm == NULL ? "" : wfm->name );
			break;
	}
	
	SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_JUSTIFY, cellJustify);


	SetTableCellAttribute (panel, ctrlDac, cell, ATTR_TEXT_BGCOLOR, textBgColor);
	
//		SetTableCellAttribute (panel, ctrlDac, cell, ATTR_TEXT_COLOR, VAL_WHITE);

	SetTableCellAttribute (panel, ctrlDac, cell, ATTR_CELL_MODE, cellMode);

}

 


/////////////



// display single digital channel cell
void DIGITALBLOCKS_displayDigitalChannelSingle (t_sequence *seq, t_digitalBlock *block, int ch) 
{
	int color;
	int cellMode;
	//int dimmed;
	int state;
	//char *txt;
	//int first;
	//int startCh, stopCh;
	char help[2000];
	//char s[2000]; 
	//int cellType;
	int panel = panelSequence;
	int ctrlDigital = SEQUENCE_TABLE_digital2;
	
	char always[] = "    always";
	char on[] = "on     ";
	char off[] = "off     ";
	char empty[] = "";
	char *txtBufAlwaysOn;
	char *txtBufAlwaysOff;
	Point cell = MakePoint(block->blockNr, ch+1); 
	
	if (DIGITALBLOCKS_isFirstOfCompressedGroup (block)) {
		// group
		SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_BGCOLOR, VAL_OFFWHITE);   
		//SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_DIMMED, 1);
		SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_FRAME_COLOR, VAL_OFFWHITE);
		SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_TYPE, VAL_CELL_PICTURE);
		
		if (block->bitmapPtrArr == NULL) DIGITALBLOCKS_createBitmaps (seq, block, -1, ch);
		if (block->bitmapPtrArr != NULL) {
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL,
								   block->bitmapPtrArr[ch]);
		}
		return;
	}
	
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_MODE, VAL_HOT);
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_FRAME_COLOR, VAL_PANEL_GRAY);
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_NO_EDIT_TEXT, TRUE);
	//SetTableCellRangeAttribute (panel, ctrlDigital, allCells, ATTR_CTRL_VAL, "");
	
	// just set ATTR_CELL_JUSTIFY for all cells - a bit faster a does not harm cells with empty text in there.
	txtBufAlwaysOn = empty;
	txtBufAlwaysOff = empty;
	switch (block->blockNr  % 3) {
		case 0: 
			txtBufAlwaysOn = always;
			txtBufAlwaysOff = always;
			SetTableCellAttribute (panel, ctrlDigital, cell,
									ATTR_CELL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);
		    break;
		case 1: 
			txtBufAlwaysOn = on;
			txtBufAlwaysOff = off;
			SetTableCellAttribute (panel, ctrlDigital, cell,
									ATTR_CELL_JUSTIFY, VAL_CENTER_LEFT_JUSTIFIED);
		    break;
		default:
			txtBufAlwaysOn = empty;
			txtBufAlwaysOff = empty;
	}
	
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL, ""); 
		
	
	color = TABLE_COLOR_BUTTON_INACTIVE[ch%2]; // default
	switch (seq->DIO_channelStatus[ch]) {
		case DIO_CHANNELSTATUS_STANDARD:
			color = TABLE_COLOR_BUTTON_INACTIVE[ch%2];				
			cellMode = VAL_HOT;
			
			/*if (block->varyPulse[ch] && (block->blockMode == 0)) {
				strcpy (help, arrow(block->varyPulseStartIncrement[ch]));
				strcat (help, " vary ");
				strcat (help, arrow(block->varyPulseStopIncrement[ch]));
				txt = help;
			}
			else if (block->alternate[ch]) {
				txt = "alternate";
				
			}
			else txt[0] = 0;*/

			break;
		case DIO_CHANNELSTATUS_ALWAYSON:
			color = colors[ch % NColors];
			SetTableCellAttribute (panel, ctrlDigital,
										cell,
										ATTR_CELL_MODE, VAL_INDICATOR);
			SetTableCellAttribute (panel, ctrlDigital,
										cell,
										ATTR_TEXT_COLOR, VAL_WHITE);
			SetTableCellVal(panel, ctrlDigital, cell, txtBufAlwaysOn);
			
			
			break;
		case DIO_CHANNELSTATUS_ALWAYSOFF:
			state = 0;
			color = TABLE_COLOR_channel_alwaysOff;
			SetTableCellAttribute (panel, ctrlDigital,cell, ATTR_CELL_MODE, VAL_INDICATOR);
			SetTableCellAttribute (panel, ctrlDigital,cell, ATTR_TEXT_COLOR, VAL_RED);
			SetTableCellVal(panel, ctrlDigital, cell, txtBufAlwaysOff); 
			
			break;
		default:
			seq->DIO_channelStatus[ch] = DIO_CHANNELSTATUS_STANDARD;
			return;
	}
	SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_BGCOLOR, color);
	
	//
	//    display buttons
	//  
	if (seq->DIO_channelStatus[ch] == DIO_CHANNELSTATUS_STANDARD) {
		state =  block->channels[ch] ^ seq->DIO_invertDisplay[ch];
		if (state) {
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_BGCOLOR, colors[ch % NColors]);
		}
	 	if (block->varyPulse[ch] && (block->blockMode == 0)) {
			strcpy (help, arrow(block->varyPulseStartIncrement[ch]));
			strcat (help, " vary ");
			strcat (help, arrow(block->varyPulseStopIncrement[ch]));
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_COLOR, state ? VAL_WHITE: VAL_RED);
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL, help);
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
		}
		else if (block->alternate[ch]) {
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_COLOR, state ? VAL_WHITE: VAL_RED);
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL, "alternate");
			SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
		}
	}
}


////////

  /*
void DIGITALBLOCKS_displayDigitalChannels_old (t_sequence *seq, t_digitalBlock *block, int ch)
{
	static char help[20];
	char *txt;
	int state;
	int color;
	Point cell;
	int dimmed;
	int cellMode;
	int justified;
	int cellFrameColor;
	int type;
	//int textColor;

	cell = MakePoint (block->blockNr, ch+1);
	
	txt = "";
	dimmed = 0;
	state = 0;
    justified = VAL_CENTER_CENTER_JUSTIFIED;
	cellFrameColor = VAL_PANEL_GRAY;
	type = VAL_CELL_STRING;
	if (ch > N_TOTAL_DIO_CHANNELS) return;
	if (ch < 0) return;
	
	if (DIGITALBLOCKS_isFirstOfCompressedGroup (block)) {
		// group
		color = VAL_OFFWHITE;
		dimmed = 1;
		cellFrameColor = VAL_OFFWHITE;
		type = VAL_CELL_PICTURE;
		if (block->bitmapPtrArr == NULL) DIGITALBLOCKS_createBitmaps (seq, block, -1, ch);
		SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_CELL_TYPE,
								   type);
		if (block->bitmapPtrArr != NULL) {
			SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_CTRL_VAL,
								   block->bitmapPtrArr[ch]);
		}
//		ProcessDrawEvents();
	}
	
	else {
		switch (seq->DIO_channelStatus[ch]) {
			case DIO_CHANNELSTATUS_STANDARD:
				state = block->channels[ch] ^ seq->DIO_invertDisplay[ch];
				if (state == 0) {
					if (block->groupNr != 0) color = VAL_OFFWHITE;
//					else if (seq->DIO_lockChannel[ch]) color = TABLE_COLOR_BUTTON_INACTIVE_LOCKED[ch%2];
					else color = TABLE_COLOR_BUTTON_INACTIVE[(ch+1)%2];
//					color = (block->groupNr == 0) ? TABLE_COLOR_BUTTON_INACTIVE[ch%2] : VAL_OFFWHITE;
//				if (state == 0) color = (block->groupNr == 0) ? TABLE_COLOR_directIO_inactive : VAL_OFFWHITE;
				}
				else color = colors[ch % NColors];
			 	if (block->varyPulse[ch] && (block->blockMode == 0)) {
					strcpy (help, arrow(block->varyPulseStartIncrement[ch]));
					strcat (help, " vary ");
					strcat (help, arrow(block->varyPulseStopIncrement[ch]));
					txt = help;
				}
				else if (block->alternate[ch]) {
					txt = "alternate";
					
				}
				else txt[0] = 0;

				cellMode = VAL_HOT;
				break;
			case DIO_CHANNELSTATUS_ALWAYSON:
				color = colors[ch % NColors];
				cellMode = VAL_INDICATOR;
				dimmed = 0;
				state = 1;
				switch ((block->blockNr) % 3) {
					 case 0: txt = "always"; 
					 	 justified = VAL_CENTER_RIGHT_JUSTIFIED;
					     break;
					 case 1: txt = "on";    
	 				 	 justified = VAL_CENTER_LEFT_JUSTIFIED;
					     break;
					 case 2: txt = "";	    
					 	  break;
				}
				break;
			case DIO_CHANNELSTATUS_ALWAYSOFF:
				state = 0;
				color = TABLE_COLOR_channel_alwaysOff;
				cellMode = VAL_INDICATOR;
				dimmed = 0;
				switch ((block->blockNr+1) % 3) {
					 case 0: txt = "always"; 
					 	 justified = VAL_CENTER_RIGHT_JUSTIFIED;
					     break;
					 case 1: txt = "off";    
	 				 	 justified = VAL_CENTER_LEFT_JUSTIFIED;
					     break;
					 case 2: txt = "";	    
					 	  break;
				}
				break;
			default:
				seq->DIO_channelStatus[ch] = DIO_CHANNELSTATUS_STANDARD;
				DIGITALBLOCKS_displayDigitalChannelSingle (seq, block, ch);
				return;
		}
	}
	
	
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell,
						   ATTR_CELL_FRAME_COLOR, cellFrameColor);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell,
							   ATTR_CELL_JUSTIFY, justified);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_CELL_TYPE,
						   type);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_TEXT_BGCOLOR, color);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_CELL_DIMMED,
					   	   block->disable);
	//SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_TEXT_COLOR, textColor);
		
	if(type == VAL_CELL_STRING) SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_digital2, cell, ATTR_CTRL_VAL, txt);

}  */





/*

void DIGITALBLOCKS_resetAllTableAttributes (t_sequence *seq)
{
	t_digitalBlock *b;
	int i;
	
	for (i = ListNumItems (seq->lDigitalBlocks); i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		DIGITALBLOCK_resetTableAttributes (b);
    }
		
}

*/
/*
void setButtonState (int panel, int ctrl, int state)
{
	if (state) SetCtrlAttribute (panel, ctrl, ATTR_ON_TEXT, "");
	else SetCtrlAttribute (panel, ctrl, ATTR_OFF_COLOR, VAL_LT_GRAY);
	SetCtrlAttribute (panel, ctrl, ATTR_CTRL_VAL, state);
}
*/



void DIGITALBLOCKS_setNTableColumns (int panel, int ctrlHeader, int ctrlDac, int ctrlDigital, int nBlocks)
{
	int nCols;

	GetNumTableColumns (panel, ctrlDigital, &nCols);
	if (nCols < nBlocks) {
		DIGITALBLOCKS_insertTableColumns (panel, ctrlHeader, ctrlDac, ctrlDigital, 
										 nCols+1, nBlocks - nCols);
	}
	else if (nCols > nBlocks) {
		DIGITALBLOCKS_deleteTableColumns (panel, ctrlHeader, ctrlDac, ctrlDigital, 
										 nBlocks+1, nCols-nBlocks);
	}
}




void DIGITALBLOCKS_displayInTable (t_sequence *seq, int blockNr, int panel, int ctrlHeader, int ctrlDac, int ctrlDigital)
{
	int test;
	//Rect cells;
	//Point cell;
	int nRows;
	int i;
	//int color;
 	t_digitalBlock *b;
	//t_waveform *wfm;
	//int cellMode;
	//char *txt;
	//int dimmed;
	int nBlocks;
	int width;
	#define GRID_COLOR_DISABLED VAL_BLACK
	#define GRID_COLOR_NORMAL VAL_DK_GRAY
	
	
	clock_t startTime;
	//double duration;
	
	
	startTime = clock ();
	ListGetItem (seq->lDigitalBlocks, &b, blockNr);
//	DIGITALBLOCKS_createBitmaps	(seq, b, -1, -1);

	GetNumTableColumns (panel, ctrlHeader, &test);
//	if (test != nCols) MessagePopup ("Warning", "N Table cols inconsistent... function: TABLE_displayValues(...) ");

	GetNumTableRows (panel, ctrlDigital, &nRows);

	nBlocks = ListNumItems (seq->lDigitalBlocks);
	DIGITALBLOCKS_setNTableColumns (panel, ctrlHeader, ctrlDac, ctrlDigital, nBlocks);

//	DebugPrintf ("block=%d, panel=%d: (%d,%d,%d): %1.3f", blockNr, panel, ctrlHeader, ctrlDac, ctrlDigital, timeStop_s (startTime));
//	startTime = clock ();  
//	DebugPrintf ("   2: %1.3f", timeStop_s (startTime));

//	startTime = clock ();   
	width = b->tableColumnWidth;
	if (b->disable) width /= 2;
	SetTableColumnAttribute (panel, ctrlHeader, blockNr,
							 ATTR_COLUMN_WIDTH, width);
	SetTableColumnAttribute (panel, ctrlDigital, blockNr,
							 ATTR_COLUMN_WIDTH, width);
	SetTableColumnAttribute (panel, ctrlDigital, blockNr, ATTR_CELL_DIMMED, b->disable);
	SetTableColumnAttribute (panel, ctrlDac, blockNr,
							 ATTR_COLUMN_WIDTH, width );
	SetTableColumnAttribute (panel, ctrlDac, blockNr, ATTR_CELL_DIMMED, b->disable);
 	
//	DebugPrintf ("   2b: %1.3f", timeStop_s (startTime));
	
	if (b->tableColumnWidth != BLOCK_TABLE_COL_WIDTH_inGroup) {
		// display header data
//		startTime = clock ();
		DIGITALBLOCKS_displayHeaderInTable (seq, b, panel, ctrlHeader);
//		duration = timeStop_s (startTime);
	
//		DebugPrintf ("   3: %1.3f", duration);
	
		// display channels
//		startTime = clock ();
		
			if (nRows > N_TOTAL_DIO_CHANNELS) nRows = N_TOTAL_DIO_CHANNELS;
			for (i = 0; i < seq->maxDigitalChannel; i++) {
				DIGITALBLOCKS_displayDigitalChannelSingle (seq, b, i);
			}	
	//		duration = timeStop_s (startTime);
	//		DebugPrintf ("   4: %1.3f", duration);
	
			// display DAC-Channels
	//		startTime = clock ();
	
			for (i = 0; i < N_DAC_CHANNELS; i++) {
				 DIGITALBLOCKS_displayDACChannelInTable (panel, ctrlDac, seq, b, i);

			}
	//		duration = timeStop_s (startTime);

	//		DebugPrintf ("   5: %1.3f", duration);
			
	}
//	DebugPrintf ("   6: %1.3f \n", timeStop_s (startTime));
	
}



void DIGITALBLOCKS_displayDIOChannel (t_sequence *seq, int DIOchannel, int createBitmaps, int panel, int ctrlDigital)
{
	t_digitalBlock **bl;
	int i;
	int nBlocks;
	
	if (seq == NULL) return;
	
	nBlocks = ListNumItems (seq->lDigitalBlocks);
	bl = (t_digitalBlock **) malloc (sizeof (t_digitalBlock*) * nBlocks);
	ListGetItems (seq->lDigitalBlocks, bl, 1, nBlocks);
  	for (i = 0; i < nBlocks; i++) {
		if (createBitmaps) DIGITALBLOCKS_createBitmaps (seq, bl[i], -1, DIOchannel);
	}	
	DIGITALBLOCKS_displayDigitalChannel_fast (seq, bl, nBlocks, panel, ctrlDigital, DIOchannel);
		
	free (bl);
}





//=======================================================================
//
//    displays all digital blocks of a sequence
//  
//=======================================================================

void DIGITALBLOCKS_setHeightForAllRows (t_sequence *seq)
{
	int i;
	int height;
	
	for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
		height =  i < seq->maxDigitalChannel ? config->buttonHeight : 1; 
		if (seq->tableConfig.DIOvisible[i] == 0) height = 1;
		SetTableRowAttribute (panelSequence, SEQUENCE_TABLE_digital1, i+1, ATTR_ROW_HEIGHT, height);
		SetTableRowAttribute (panelSequence, SEQUENCE_TABLE_digital2, i+1, ATTR_ROW_HEIGHT, height);
	}

	for (i = 0; i < N_DAC_CHANNELS; i++) {
//		height =  i < seq->maxDigitalChannel ? config->buttonHeight : 1; 
		height = config->buttonHeight;
		if (seq->tableConfig.AOvisible[i] == 0) height = 1;
		SetTableRowAttribute (panelSequence, SEQUENCE_TABLE_dac1, i+1, ATTR_ROW_HEIGHT, height + 5);
		SetTableRowAttribute (panelSequence, SEQUENCE_TABLE_dac2, i+1, ATTR_ROW_HEIGHT, height + 5);
	}
	
}


void DIGITALBLOCKS_displayAllAnalogChannels_fast (t_sequence *seq, t_digitalBlock **bl, int nBlocks, int panel, int ctrlAnalog)
{
	int i, k;
	Rect column;
	int width;
	
	
	//Rect allCells;
	//						VAL_TABLE_ROW_RANGE
	//allCells = MakeRect (1, 1, N_DAC_CHANNELS, nBlocks);
	SetTableCellRangeAttribute (panel, ctrlAnalog, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_DIMMED, 0);
	SetTableCellRangeAttribute (panel, ctrlAnalog, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_MODE, VAL_HOT);
	SetTableCellRangeAttribute (panel, ctrlAnalog, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableCellRangeAttribute (panel, ctrlAnalog, VAL_TABLE_ENTIRE_RANGE, ATTR_CTRL_VAL, "");

	for (i = 0; i < N_DAC_CHANNELS; i++) {
		if (COLOR_DAC_CHANNELS) {
			SetTableCellRangeAttribute (panel, ctrlAnalog,VAL_TABLE_ROW_RANGE(i+1),ATTR_TEXT_BGCOLOR, lightenColor(colors[i % NColors],0.7f));
		} else {
			SetTableCellRangeAttribute (panel, ctrlAnalog,VAL_TABLE_ROW_RANGE(i+1),ATTR_TEXT_BGCOLOR, TABLE_COLOR_BUTTON_INACTIVE_DAC[(i+1)%2]);             
		}
		for (k = 0; k < nBlocks; k++) {
			if ((bl[k]->waveforms[i] != WFM_ID_UNCHANGED) || DIGITALBLOCKS_isFirstOfCompressedGroup (bl[k])) {
				DIGITALBLOCKS_displayDACChannelInTable (panel, ctrlAnalog, seq, bl[k], i);
			}
		}
	}
	for (k = 0; k < nBlocks; k++) {
		column = VAL_TABLE_COLUMN_RANGE(k+1);
		width = bl[k]->tableColumnWidth;
		if (bl[k]->disable) width /= 2;
		if (width == 0) width = 1; // SetTableColumnAttribute does not like width==0
		SetTableColumnAttribute (panel, ctrlAnalog, k+1, ATTR_COLUMN_WIDTH, width);
		SetTableColumnAttribute (panel, ctrlAnalog, k+1, ATTR_CELL_DIMMED, bl[k]->disable);
		SetTableCellRangeAttribute (panel, ctrlAnalog, column, ATTR_CELL_DIMMED, bl[k]->disable);
	}
	
}




void DIGITALBLOCKS_displayDigitalChannel_fast (t_sequence *seq, t_digitalBlock **bl, int nBlocks, int panel, int ctrlDigital, int channel)
{
	int i, k;
	//int ch;
	int color, cellMode;
	//int dimmed;
	int state;
	int width;
	Point cell;
	//char *txt;
	int first;
	char help[20];
	Rect cells, channelCells;   //allCells, 
	int startCh, stopCh;
	//char s[2000]; 
	//int cellType;
	
	char always[] = "    always";
	char on[] = "on     ";
	char off[] = "off     ";
	char empty[] = "";
	char **txtBufAlwaysOn;
	char **txtBufAlwaysOff;

	txtBufAlwaysOn = calloc(nBlocks, sizeof(char*));
	txtBufAlwaysOff = calloc(nBlocks, sizeof(char*));
	

	//allCells = MakeRect (1, 1, seq->maxDigitalChannel, nBlocks);
	SetTableCellRangeAttribute (panel, ctrlDigital, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_MODE, VAL_HOT);
	SetTableCellRangeAttribute (panel, ctrlDigital, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableCellRangeAttribute (panel, ctrlDigital, VAL_TABLE_ENTIRE_RANGE, ATTR_CELL_FRAME_COLOR, VAL_PANEL_GRAY);
	SetTableCellRangeAttribute (panel, ctrlDigital, VAL_TABLE_ENTIRE_RANGE, ATTR_NO_EDIT_TEXT, TRUE);
	//SetTableCellRangeAttribute (panel, ctrlDigital, allCells, ATTR_CTRL_VAL, "");
	
	
	// just set ATTR_CELL_JUSTIFY for all cells - a bit faster a does not harm cells with empty text in there.
	for (k = 0; k < nBlocks; k++) {
		cells = VAL_TABLE_COLUMN_RANGE(k+1);     
		
		width = bl[k]->tableColumnWidth;
		if (bl[k]->disable) width /= 2;
		if (width == 0) width = 1; // SetTableColumnAttribute does not like width==0
		SetTableColumnAttribute (panel, ctrlDigital, k+1, ATTR_COLUMN_WIDTH, width);
		SetTableColumnAttribute (panel, ctrlDigital, k+1, ATTR_CELL_DIMMED, bl[k]->disable);
		SetTableCellRangeAttribute (panel, ctrlDigital, cells, ATTR_CELL_DIMMED, bl[k]->disable);
		
		
		switch (k  % 3) {
			case 0: 
				txtBufAlwaysOn[k] = always;
				txtBufAlwaysOff[k] = always;
				SetTableCellRangeAttribute (panel, ctrlDigital, cells,
										ATTR_CELL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);
			    break;
			case 1: 
				txtBufAlwaysOn[k] = on;
				txtBufAlwaysOff[k] = off;
				SetTableCellRangeAttribute (panel, ctrlDigital, cells,
										ATTR_CELL_JUSTIFY, VAL_CENTER_LEFT_JUSTIFIED);
			    break;
			case 2: 
				txtBufAlwaysOn[k] = empty;
				txtBufAlwaysOff[k] = empty;
		}
	}

	//
	//   display all rows
	//
	if (channel == -1) {
		startCh = 0;
		stopCh = seq->maxDigitalChannel;
	}
	else {
		startCh = channel;
		stopCh = channel + 1;
	}
	
	for (i = startCh; i < stopCh; i++) {
		channelCells = VAL_TABLE_ROW_RANGE(i+1);//MakeRect(i+1,1,1,nBlocks); 
		SetTableCellRangeAttribute (panel, ctrlDigital, channelCells, ATTR_CTRL_VAL, ""); 
		
		color = TABLE_COLOR_BUTTON_INACTIVE[i%2]; // default
		switch (seq->DIO_channelStatus[i]) {
			case DIO_CHANNELSTATUS_STANDARD:
				color = TABLE_COLOR_BUTTON_INACTIVE[i%2];				
				cellMode = VAL_HOT;
				      
				//SetTableCellRangeAttribute (panel, ctrlDigital, channelCells, ATTR_CTRL_VAL, "");
				
				/*if (block->varyPulse[ch] && (block->blockMode == 0)) {
					strcpy (help, arrow(block->varyPulseStartIncrement[ch]));
					strcat (help, " vary ");
					strcat (help, arrow(block->varyPulseStopIncrement[ch]));
					txt = help;
				}
				else if (block->alternate[ch]) {
					txt = "alternate";
					
				}
				else txt[0] = 0;*/

				break;
			case DIO_CHANNELSTATUS_ALWAYSON:
				color = colors[i % NColors];
				SetTableCellRangeAttribute (panel, ctrlDigital,
											channelCells,
											ATTR_CELL_MODE, VAL_INDICATOR);
				SetTableCellRangeAttribute (panel, ctrlDigital,
											channelCells,
											ATTR_TEXT_COLOR, VAL_WHITE);
				SetTableCellRangeVals(panel, ctrlDigital, channelCells, txtBufAlwaysOn, VAL_ROW_MAJOR);
				
				
				break;
			case DIO_CHANNELSTATUS_ALWAYSOFF:
				state = 0;
				color = TABLE_COLOR_channel_alwaysOff;
				SetTableCellRangeAttribute (panel, ctrlDigital,channelCells, ATTR_CELL_MODE, VAL_INDICATOR);
				SetTableCellRangeAttribute (panel, ctrlDigital,channelCells, ATTR_TEXT_COLOR, VAL_RED);
				SetTableCellRangeVals(panel, ctrlDigital, channelCells, txtBufAlwaysOff, VAL_ROW_MAJOR); 
				
				break;
			default:
				seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_STANDARD;
				return;
		}
		SetTableCellRangeAttribute (panel, ctrlDigital, channelCells, ATTR_TEXT_BGCOLOR, color);
	}


	
	
	//
	//    display buttons
	//  
	for (i = startCh; i < stopCh; i++) {//(i = 0; i < seq->maxDigitalChannel; i++) {
		if (seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_STANDARD) {
			first = -1;
			for (k = 0; k < nBlocks; k++) {
				state =  bl[k]->channels[i] ^ seq->DIO_invertDisplay[i];
				if ((state == 1) && (first == -1)) first = k;
				else if ((state == 0) && (first != -1)) {
					SetTableCellRangeAttribute (panel, ctrlDigital,
												MakeRect (i+1, first+1, 1, k-first),
												ATTR_TEXT_BGCOLOR, colors[i % NColors]);
					first = -1;
				}
				cell = MakePoint (k+1,i+1);
			 	if (bl[k]->varyPulse[i] && (bl[k]->blockMode == 0)) {
					strcpy (help, arrow(bl[k]->varyPulseStartIncrement[i]));
					strcat (help, " vary ");
					strcat (help, arrow(bl[k]->varyPulseStopIncrement[i]));
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_COLOR, state ? VAL_WHITE: VAL_RED);
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL, help);
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
				}
				else if (bl[k]->alternate[i]) {
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_TEXT_COLOR, state ? VAL_WHITE: VAL_RED);
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CTRL_VAL, "alternate");
					SetTableCellAttribute (panel, ctrlDigital, cell, ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
				}
			}
			if (first != -1) {
				SetTableCellRangeAttribute (panel, ctrlDigital,
											MakeRect (i+1, first+1, 1, k-first),
											ATTR_TEXT_BGCOLOR, colors[i % NColors]);
			}
		}
	}
	
	// display all groups
	for (i = 0; i < nBlocks; i++) {
		if (DIGITALBLOCKS_isFirstOfCompressedGroup (bl[i])) {
			cells = VAL_TABLE_COLUMN_RANGE(i+1); //MakeRect (1, i+1, seq->maxDigitalChannel, 1);
   
			
			SetTableCellRangeAttribute (panel, ctrlDigital, cells, ATTR_CELL_TYPE, VAL_CELL_PICTURE);
			SetTableCellRangeAttribute (panel, ctrlDigital, cells, ATTR_TEXT_BGCOLOR, VAL_OFFWHITE);
			
			if (bl[i]->bitmapPtrArr == NULL) {
				DIGITALBLOCKS_createBitmaps (seq, bl[i], -2, -2);
			}
			if (bl[i]->bitmapPtrArr != NULL) {
					SetTableCellRangeVals(panel, ctrlDigital, cells, bl[i]->bitmapPtrArr, VAL_COLUMN_MAJOR);  
					/*for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
			
						//tprintf("bl[i=%d]->bitmapPtrArr[ch=%d] = %d\n",  i,ch,bl[i]->bitmapPtrArr[ch]);
						//PostMessagePopup ("Error", s);
						//ProcessDrawEvents();
			
						//GetTableCellAttribute (panel, ctrlDigital, MakePoint (i+1,ch+1), ATTR_CELL_TYPE, &cellType); // MakePoint (i,ch+1)
						//if (cellType != VAL_CELL_PICTURE) {
						//	 tprintf("cell type collision at i=%d,ch=%d, type was: %d\n",i,ch,cellType);  
						//}
			
						SetTableCellAttribute (panel, ctrlDigital, MakePoint (i+1,ch+1), ATTR_CTRL_VAL, bl[i]->bitmapPtrArr[ch]);
		
					} */
			}
		}
		if ((bl[i]->groupNr > 0) && (bl[i]->isGroupExpanded) && (bl[i]->nElementsInGroup > 0)) {
			cells = MakeRect (1, i+1, seq->maxDigitalChannel, bl[i]->nElementsInGroup );
			SetTableCellRangeAttribute (panel, ctrlDigital, cells,
										ATTR_CELL_FRAME_COLOR, VAL_BLACK);
		}
			

	}
	
	free(txtBufAlwaysOn);
	free(txtBufAlwaysOff);		
}		
			


void DIGITALBLOCKS_updateCorrespondingChannels(t_sequence *seq, int wfmNr)
{
	//t_waveform *w;
	int i, n;
	t_digitalBlock *b;
	int ch;
	
	n = ListNumItems (seq->lDigitalBlocks); 
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);	
		for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
			if (b->waveforms[ch] == wfmNr) {
				// used in this channel, so update channel
				DIGITALBLOCKS_displayDACChannelInTable (panelSequence, SEQUENCE_TABLE_dac2, seq, b, ch);  
			}
		}
	}
	
}
			

	
	
int DIGITALBLOCKS_isFirstOfCompressedGroup (t_digitalBlock *b)
{
	return ((b->groupNr > 0) && (!b->isGroupExpanded) && (b->nElementsInGroup > 0));
}



void DIGITALBLOCKS_displayAllInTable_fast (t_sequence *seq, int panel, int ctrlHeader, int ctrlDac, int ctrlDigital)
{
	int nBlocks;
	int i;
	//int ch;
	int width;
	//int color, cellMode, dimmed;
	//int state;
	t_digitalBlock **bl;
	//Rect cells;
	
	clock_t startTime;      	
	

	nBlocks = ListNumItems (seq->lDigitalBlocks);
	
//	DeleteTableColumns (panel, ctrlDigital, 1, -1);
//	DeleteTableColumns (panel, ctrlDac, 1, -1);
	
	DIGITALBLOCKS_setNTableColumns (panel, ctrlHeader, ctrlDac, ctrlDigital, nBlocks);
	
	bl = (t_digitalBlock **) malloc (sizeof (void*) * nBlocks);
	for (i = 0; i < nBlocks; i++) {
		ListGetItem (seq->lDigitalBlocks, &bl[i], i+1);
		// group
	}

	
	SetCtrlAttribute(panel, ctrlDac, ATTR_VISIBLE,0);   
	SetCtrlAttribute(panel, ctrlDigital,ATTR_VISIBLE,0);  
	
	startTime = clock(); 
	DIGITALBLOCKS_displayDigitalChannel_fast (seq, bl, nBlocks, panel, ctrlDigital, -1);
		#ifdef _CVI_DEBUG_        	
	tprintf ("total DIGITALBLOCKS_displayDigitalChannel_fast: %1.3f\n", timeStop_s (startTime));
#endif	
	
	startTime = clock();
	DIGITALBLOCKS_displayAllAnalogChannels_fast (seq, bl, nBlocks, panel, ctrlDac);
		#ifdef _CVI_DEBUG_        	
	tprintf ("total DIGITALBLOCKS_displayAllAnalogChannels_fast: %1.3f\n", timeStop_s (startTime));
#endif
	
	SetCtrlAttribute(panel, ctrlDac, ATTR_VISIBLE,1);   
	SetCtrlAttribute(panel, ctrlDigital,ATTR_VISIBLE,1);  
	
	for (i = 0; i < nBlocks; i++) {
		if (bl[i]->gpibCommandNr != 0)
		{
			 t_gpibCommand *g;
			ListGetItem (seq->lGpibCommands, &g, bl[i]->gpibCommandNr);
			if (g->commandType == GPIB_LOOP_TRIGGER) {
				bl[i]->doTriggerLoop = 1;	
			}
			else if (g->commandType == GPIB_RECV_LOOP_TRIGGER) {
				bl[i]->doRecvTriggerLoop = 1;
			}
		}
		width = bl[i]->tableColumnWidth;
		if (bl[i]->disable) width /= 2;
		if (width == 0) width = 1; // SetTableColumnAttribute does not like width==0
		SetTableColumnAttribute (panel, ctrlHeader, i+1, ATTR_COLUMN_WIDTH, width);
		//SetTableColumnAttribute (panel, ctrlDigital, i+1, ATTR_COLUMN_WIDTH, width);
		//SetTableColumnAttribute (panel, ctrlDigital, i+1, ATTR_CELL_DIMMED, bl[i]->disable);
		//SetTableCellRangeAttribute (panel, ctrlDigital, MakeRect (1, i+1, seq->maxDigitalChannel, 1), ATTR_CELL_DIMMED, bl[i]->disable);
		//SetTableColumnAttribute (panel, ctrlDac, i+1, ATTR_COLUMN_WIDTH, width);
		//SetTableColumnAttribute (panel, ctrlDac, i+1, ATTR_CELL_DIMMED, bl[i]->disable);
		DIGITALBLOCKS_displayHeaderInTable (seq, bl[i], panel, ctrlHeader);
		if (DIGITALBLOCKS_isFirstOfCompressedGroup (bl[i])) {
//			SetTableCellRangeAttribute (panel, ctrlDigital, MakeRect (1, i+1, seq->maxDigitalChannel, 1), ATTR_CELL_DIMMED, bl[i]->disable);
		}
		
		

	}

	free (bl);
}


void DIGITALBLOCKS_displayAllBlocks (t_sequence *seq)

{
	//int i;
	int nBlocks;
	int activeCtrl;
	//int oldBreakOnLibraryErrorsValue;

 	clock_t startTime;
	
	if (seq == NULL) return;
	
	SetWaitCursor (1);
	MAIN_setFilenameToTitle (seq->filename, seq->changes);
	MAIN_displayTimebases (seq);    

	DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (seq);
    seq->referenceTime = DIGITALBLOCK_calculateAllAbsoluteTimes (seq, 0);
	DIGITALBLOCKS_displaySequenceDuration (seq);

	DIGITALBLOCKS_setHeightForAllRows (seq);
	
 	nBlocks = ListNumItems (seq->lDigitalBlocks);

	startTime = clock ();   
	activeCtrl =  GetActiveCtrl(panelSequence);
	
//	DebugPrintf ("active ctrl = (%d, %d ,%d)\n",GetActivePanel (), panelSequence, GetActiveCtrl(GetActivePanel ()));
//	SetActiveCtrl (panelSequence, SEQUENCE_BTN_startSequence);  // TODO check if start button is active
	DIGITALBLOCKS_displayAllInTable_fast (seq,  panelSequence, SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
//	ProcessDrawEvents ();
//	for (i = nBlocks; i > 0; i--) {
//		DIGITALBLOCKS_displayInTable (seq, i, panelSequence, SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
//    }
#ifdef _CVI_DEBUG_  	
	//tprintf ("total DIGITALBLOCKS_displayAllInTable_fast: %1.3f\n", timeStop_s (startTime));
#endif	
	
	/*if (activeCtrl) { // this caused a crash and is not essential -> kick it.
			
		oldValue = SetBreakOnLibraryErrors (0);
		SetActiveCtrl (panelSequence, activeCtrl);
		SetBreakOnLibraryErrors (oldBreakOnLibraryErrorsValue);
	}*/

	DIGITALBLOCKS_displayChannelNames (seq);
	DIGITALBLOCKS_displayAllDirectIOButtons  (seq);
	DIGITALBLOCKS_displayGlobalAnalogVoltages ();
	PARAMETERSWEEPS_updateDisplay (seq);
	ProcessDrawEvents ();
	SetWaitCursor (0);
	
}





void DIGITALBLOCKS_displayBlockNrInTable (t_digitalBlock *b)
{
	Point cell;
	int bgcolor;
	char txt[50];
	char txt2[50];  
	int textSize;
	int dimmed;
	
	if (b == NULL) return;
	cell = MakePoint (b->blockNr, BLOCK_TABLE_HEADER_ROW_blockNo);
	switch (b->blockMode) {
		case BMODE_Standard:
			if ((b->nElementsInGroup == 0) || (b->isGroupExpanded)) strcpy (txt, intToStr (b->blockNr)); 
			else sprintf (txt, "%d - %d", b->blockNr, b->blockNr+b->nElementsInGroup-1);
			if (b == variableTimeBlock) bgcolor =  VAL_COLOR_SLATEBLUE1;
			else {
				if (b->groupNr == 0) bgcolor = TABLE_COLOR_header_bkgrnd;
				else bgcolor = TABLE_COLOR_header_bkgrnd_groups[b->groupNr % 2];
			}
			textSize = 16;
			dimmed = 1;
			txt2[0] = 0;
			break;
		case BMODE_LoopEnd:
			strcpy (txt, "LoopEnd");
			bgcolor = VAL_YELLOW;
			textSize = 8;
			dimmed = 1;
			txt2[0] = 0;
			break;
		case BMODE_InLoop:
			strcpy (txt, intToStr (b->blockNr));
			textSize = 16;
			bgcolor = VAL_YELLOW;
			dimmed = 1;
			txt2[0] = 0;
			break;  
		default: // loop Start
			sprintf (txt, "LoopStart\n%d x", b->blockMode);
			bgcolor = VAL_YELLOW;
			textSize = 8;
			dimmed = 0;
			sprintf (txt2, "%d rep.", b->blockMode);
	}
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell, ATTR_CTRL_VAL, txt);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell,
						   ATTR_TEXT_BGCOLOR, bgcolor);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell,
						   ATTR_TEXT_POINT_SIZE, textSize);
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell, ATTR_CELL_DIMMED, b->disable);
	
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell,  ATTR_CELL_MODE, VAL_INDICATOR ); 
	SetTableCellAttribute (panelSequence, SEQUENCE_TABLE_header2, cell,  ATTR_NO_EDIT_TEXT, TRUE );
}	





int DIGITALBLOCKS_EDIT_insertNew (t_sequence *seq, int nr, int behind)
{
     t_digitalBlock *p, *source, *k;
     int mode = 0;
     int i;
	 //Point cell;

	// ---------------------------------------------
	//   get pointer to block
	// ---------------------------------------------
	source = DIGITALBLOCK_ptr (seq, nr);
	if (behind) { 
		k = DIGITALBLOCK_ptr (seq, nr+1); 
		nr++;
	}
	else 
	    k = source;
	// ---------------------------------------------
	//   insert new block
	// ---------------------------------------------
	if (k == NULL) {         
		// append block at the end of the list
		p = DIGITALBLOCK_new (seq);
	}
	else {
		p = DIGITALBLOCK_insert (seq, nr);
	 	// insert blocks into a loop?
	 	if ( (k->blockMode == BMODE_LoopEnd)
	    	||(k->blockMode == BMODE_InLoop) )
	      	mode = BMODE_InLoop;
	}
	// --------------------------------------------
	//   copy data from old block
	// --------------------------------------------
	DIGITALBLOCK_init (p);
	p->duration       = source->duration;
	p->blockMode 	   = mode;
	memcpy (p->channels, source->channels,
	     seq->maxDigitalChannel*sizeof(int));

	DIGITALBLOCKS_insertTableColumns (panelSequence, SEQUENCE_TABLE_header2, 
									 SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2,
									 nr, 1);
	// --------------------------------------------
	//   update block numbers and positions
	// --------------------------------------------
	DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (seq);
	DIGITALBLOCKS_displayAllAbsoluteTimes (seq);     	


	DIGITALBLOCKS_displayInTable (seq, nr, panelSequence, SEQUENCE_TABLE_header2, 
								 SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);

	for (i = nr; i <= ListNumItems (seq->lDigitalBlocks); i++) {
		ListGetItem (seq->lDigitalBlocks, &k, i);
		DIGITALBLOCKS_displayBlockNrInTable (k);
	}

	return 0;;
}
        
 


//=======================================================================
//
//    delete block 
//
//=======================================================================
int DIGITALBLOCKS_EDIT_deleteBlocks (t_sequence *seq, Rect selection)
{
     t_digitalBlock *k, *weg;
     int i;
     
	 for (i = 0; i < selection.width; i++) {
		// ---------------------------------------------
		//   get pointer to block
		// ---------------------------------------------
	    k = DIGITALBLOCK_ptr (seq, selection.left);
	    weg = k;
		// ---------------------------------------------------
		//   if "loop start" or "loop end", then delete loop
		// ---------------------------------------------------
	    if ((k->blockMode > 0) || (k->blockMode == BMODE_LoopEnd))
	    	 DIGITALBLOCKS_EDIT_deleteLoop (seq, selection.left);
	 	// --------------------------------------------
		//   remove panel & block
		// --------------------------------------------
	     DIGITALBLOCK_delete (seq, selection.left);
		// --------------------------------------------
		//   update block numbers and positions
		// --------------------------------------------
	
	 }
 	 DIGITALBLOCKS_deleteTableColumns (panelSequence, SEQUENCE_TABLE_header2, 
									 SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2,
									 selection.left, selection.width);
	 
	 DIGITALBLOCKS_displayAllBlocks (seq);

/*		 
		 //	 ProcessDrawEvents();
	 DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (seq);
     seq->referenceTime = DIGITALBLOCK_calculateAllAbsoluteTimes (seq);
	 DIGITALBLOCKS_displaySequenceDuration (seq);

//	 DIGITALBLOCKS_displayAll
	 for (i = selection.left; i <= ListNumItems (seq->lDigitalBlocks); i++) {
//		 DIGITALBLOCKS_displayInTable (seq, i, panelSequence,
//			 						   SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
		 ListGetItem (seq->lDigitalBlocks, &k, i);		 
		 DIGITALBLOCKS_displayHeaderInTable (seq, k, panelSequence, SEQUENCE_TABLE_header2);
	 }
*/
	 return 0;
	 
	 
}



//=======================================================================
//
//    sets block mode: "LoopStart", "LoopEnd", "normal"
//
//=======================================================================
void DIGITALBLOCKS_setBlockMode (t_sequence *seq, t_digitalBlock *b, int mode)
{
	//int txtSize;
	int ch;
	int vary;
	
	// --------------------------------------------
	//   alter visibility of buttons, depending
	//   wether loop or not
	// --------------------------------------------
	b->blockMode = mode;
	
	vary = 0;
	for (ch = 0; ch < N_TOTAL_DIO_CHANNELS; ch++) {
		if (b->varyPulse[ch]) vary = 1;
	}
	if (vary) DIGITALBLOCKS_displayAllBlocks (seq);     
	
	
 	DIGITALBLOCKS_displayBlockNrInTable (b);
	
}





void DIGITALBLOCKS_EDIT_setLoopReps (t_sequence *seq, Rect selection, int top, int left)
{			
	int i;
	t_digitalBlock *p;
	
	if (panelLoop <= 0) {
		panelLoop = LoadPanel (0, UIR_File, LOOP);	
	}
	for (i = selection.left; i >= 1; i--) {
		p =  DIGITALBLOCK_ptr(seq,i);
		if (p == NULL) return;
		if (p->blockMode > 0) {
			// loopStart
			SetBreakOnLibraryErrors (0);
			RemovePopup (1);
			SetBreakOnLibraryErrors (1);
			loopBlock = p;
//			SetPanelPos (panelLoop, top, left);
			SetCtrlVal (panelLoop, LOOP_NUMERIC_nReps, p->blockMode);
			InstallPopup (panelLoop);
			return;
		}
	}
}


//=======================================================================
//
//    sets block as "LoopStart"
//
//=======================================================================
void DIGITALBLOCKS_EDIT_setLoop (t_sequence *seq, Rect selection)
{
    t_digitalBlock *k, *l;

    int i;

	
	DIGITALBLOCKS_setBlockMode (seq, DIGITALBLOCK_ptr(seq,selection.left), 1);
	if (selection.width == 1) return;
     
	// --------------------------------------------
	//  set new block mode
	// --------------------------------------------
    k = DIGITALBLOCK_ptr (seq, selection.left + selection.width-1);
    DIGITALBLOCKS_setBlockMode (seq, k, BMODE_LoopEnd);
	// --------------------------------------------
	//   fill everything until previous "LoopStart"
	//   with "InLoop"
	// --------------------------------------------
	for (i = selection.left+1; i < selection.left + selection.width-1; i++) {
    	l = DIGITALBLOCK_ptr (seq, i);
		if (l != NULL) DIGITALBLOCKS_setBlockMode (seq, l, BMODE_InLoop);
	}
	// --------------------------------------------
	//   fill everything until old "LoopEnd" 
	//   with "normal"
	// --------------------------------------------
	for (i = selection.left + selection.width+1; i <= ListNumItems(seq->lDigitalBlocks); i++) {
    	l = DIGITALBLOCK_ptr (seq, i);
		if ((l != NULL) && 
		   ((l->blockMode == BMODE_InLoop)
		   || (l->blockMode == BMODE_LoopEnd))) 
		   DIGITALBLOCKS_setBlockMode (seq, l, BMODE_Standard);
		else break;
	}    	
}

//=======================================================================
//
//    delete loop
//
//=======================================================================
void DIGITALBLOCKS_EDIT_deleteLoop (t_sequence *seq, int nr)
{
    t_digitalBlock *k, *eK;
    int i;
     
    k = DIGITALBLOCK_ptr (seq, nr);
	// --------------------------------------------
	//   find start of loop --> sk
	// --------------------------------------------
	if (k->blockMode < 0) // if in loop
		for (i = nr-1; i > 0; i--) {
			eK = DIGITALBLOCK_ptr (seq, i); 
			if (eK == NULL) break;
			if (eK ->blockMode > 0) {
			   DIGITALBLOCKS_setBlockMode (seq, eK, BMODE_Standard);
			   break;
			}
			if ((eK->blockMode == BMODE_LoopEnd) || (eK->blockMode == BMODE_InLoop))
			   DIGITALBLOCKS_setBlockMode (seq, eK, BMODE_Standard);
			else break;
		}	
	// --------------------------------------------
	//   find end of loop 
	// --------------------------------------------
	for (i = nr+1; i <= ListNumItems (seq->lDigitalBlocks); i++) {
		eK = DIGITALBLOCK_ptr (seq, i); 
		if (eK == NULL) break;
		if ((eK->blockMode == BMODE_LoopEnd) || (eK->blockMode == BMODE_InLoop))
		   DIGITALBLOCKS_setBlockMode (seq, eK, BMODE_Standard);
		else break; 
	}		
	DIGITALBLOCKS_setBlockMode (seq, k, BMODE_Standard);
	

}



//=======================================================================
//
//    reset block contents
//
//=======================================================================
void DIGITALBLOCKS_EDIT_resetBlockContents (t_sequence *seq, int nr)
{
     t_digitalBlock *k;
     int oldMode;
     
     k = DIGITALBLOCK_ptr(seq, nr);
     // wenn Block = LoopStart oder LoopEnde, dann Loop löschen
     oldMode = k->blockMode;
     DIGITALBLOCK_reset (k);
//     displayDigitalBlock (seq, k);
     DIGITALBLOCKS_setBlockMode (seq, k, oldMode);
}


/*
//=======================================================================
//
//    switches "variable time" on / off
//
//=======================================================================
void DIGITALBLOCKS_EDIT_switchVariableTime (t_sequence *seq, int nr)
{
     t_digitalBlock *k;

     k = DIGITALBLOCK_ptr (seq, nr);
     k->variableTime = !k->variableTime;
//     displayDigitalBlock(seq, k);
     if (k->variableTime) DIGITALBLOCKS_editVariableTime (k);
}
*/

//=======================================================================
//
//    set current block as time reference for display of absolute times
//
//=======================================================================
void DIGITALBLOCKS_EDIT_setAsTimereference (t_sequence *seq, int blockNr)
{
	int i;
	t_digitalBlock *b;
	
	for (i = 1; i <= ListNumItems (seq->lDigitalBlocks); i++) {
		b = DIGITALBLOCK_ptr (seq, i);
		b->isAbsoluteTimeReference = (i == blockNr);
	}																	   
}






//=======================================================================
//
//    configure popup menu (appears when right clicked on digital block)
//
//=======================================================================
void DIGITALBLOCKS_configureBlockPopUpMenu (t_sequence *seq, Rect selection)
{
    //t_digitalBlock *l;
	t_digitalBlock *k;
    //int i;
    

	//   get pointer to block 
	k = DIGITALBLOCK_ptr (seq, selection.left);
	if (k == NULL) return;
	// --------------------------------------------
	//  delete Block
	// --------------------------------------------
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_DELETE,
                         ATTR_DIMMED,  (ListNumItems (seq->lDigitalBlocks) <= 1) ||
						 			    (selection.width >= ListNumItems (seq->lDigitalBlocks)));
            	
	
	// --------------------------------------------
	// copy+Paste Blocks 
	// --------------------------------------------
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_PASTE_BEFORE,
                         ATTR_DIMMED,  (blockClipboard == NULL) );
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_PASTE_BEHIND,
                         ATTR_DIMMED, (blockClipboard == NULL) );
	// ungroup + group

	SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_GROUP,
                         ATTR_DIMMED, (selection.width <= 1));
  	SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_UNGROUP,
                         ATTR_DIMMED, k->groupNr == 0);

	SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_EXPAND,
                        ATTR_DIMMED, (k->groupNr == 0) || k->isGroupExpanded);
	SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_COMPRESS,
                        ATTR_DIMMED, (k->groupNr == 0) && !k->isGroupExpanded);

	// --------------------------------------------
	//  "DeleteLoop" only if within a loop 
	// --------------------------------------------
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_DELETE_LOOP,
                         ATTR_DIMMED, k->blockMode == BMODE_Standard);

	// ----------------------------------------------
	//   "Set Loop Start" 
	// ----------------------------------------------
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_SET_LOOP,
    					 ATTR_DIMMED, 
    					 ((k->blockMode == BMODE_InLoop)
    					 || (k->blockMode == BMODE_LoopEnd)));
    SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_SET_LOOPREPS,
    					 ATTR_DIMMED, 
    					 (k->blockMode == BMODE_Standard));
	
	
	// ----------------------------------------------
	//   "variable Time"
	// ----------------------------------------------
    
	SetMenuBarAttribute  (menuBlock, MENU3_BLKEDIT_VARTIME,
                         ATTR_CHECKED, k->variableTime && (selection.width <= 1));
	SetMenuBarAttribute  (menuBlock, MENU3_BLKEDIT_VARTIME,
                         ATTR_DIMMED, selection.width > 1);
	// ----------------------------------------------
	//   set as time reference
	// ----------------------------------------------
	SetMenuBarAttribute (menuBlock,
						 MENU3_BLKEDIT_TIMEREF,
						 ATTR_DIMMED, 
						 (k->blockMode < 0) || !config->displayAbsoluteTime || (selection.width > 1));
	// ----------------------------------------------
	//   GPIB
	// ----------------------------------------------
	
	SetMenuBarAttribute (menuBlock,
						 MENU3_BLKEDIT_GPIB,
						 ATTR_DIMMED, 
						 (k->blockMode < 0) || (selection.width > 1));

	DIGITALBLOCKS_insertGpibCommandsToMenu (seq, selection.left);
	
}



void DIGITALBLOCKS_EDIT_initClipboard (void)
{
	t_digitalBlock *b;
	t_waveform *w;
	int i;
	
	if (blockClipboard == NULL) {
		// create new list
		blockClipboard  = ListCreate (sizeof(t_digitalBlock *));
		blockClipboardWaveforms	= ListCreate (sizeof(t_waveform *));
	}
    else {
		// free blocks
		for (i = ListNumItems (blockClipboard); i > 0; i--) {
			ListGetItem (blockClipboard, &b, i);	
			DIGITALBLOCK_free (b);
			free (b);
		}
		ListClear (blockClipboard);

		//   free waveforms
	 	for (i = ListNumItems (blockClipboardWaveforms); i > 0; i--) {
			ListGetItem (blockClipboardWaveforms, &w, i);	
			WFM_free (w);
			free (w);
		}
		ListClear (blockClipboardWaveforms);
	}
}


void DIGITALBLOCKS_EDIT_copyToClipboard (t_sequence *seq, Rect selection)
{
	int i, n;
	t_digitalBlock *b, *newBlock;
	t_waveform *w, *newWfm;
	
	DIGITALBLOCKS_EDIT_initClipboard ();
	
	for (i = selection.left; i < selection.left+selection.width; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
	    newBlock = (t_digitalBlock *) malloc (sizeof (t_digitalBlock));
    	DIGITALBLOCK_duplicate (newBlock, b);
		if (newBlock->groupNr > 0) newBlock->groupNr += 10000000;
//		DIGITALBLOCK_resetTableAttributes (newBlock);            
		ListInsertItem (blockClipboard, &newBlock, END_OF_LIST);
	}	
	// duplicate all waveforms

	n = ListNumItems (seq->lWaveforms);
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lWaveforms, &w, i);
  		newWfm = (t_waveform *) malloc (sizeof (t_waveform));
		WFM_duplicate (newWfm, w);
		ListInsertItem (blockClipboardWaveforms, &newWfm, END_OF_LIST);
	}
}


// paste Clipboard behind "position"
void DIGITALBLOCKS_EDIT_paste (t_sequence *seq, int position, int behind)
{
	int i, n;
	t_digitalBlock *b, *pastedBlock ;
	t_digitalBlock *insertedAt;
	t_waveform *wfm, *pastedWfm, *newWfm;
	int ch;
	int insertPos;
	//int it;
	//int l;

	if (blockClipboard == NULL) return;
	n = ListNumItems (blockClipboard);
	if (n == 0) return;
	
	ListGetItem (seq->lDigitalBlocks, &insertedAt, position);
	for (i = 1; i <= n; i++) {
		insertPos = position + behind + (i-1);
		if (insertPos > (int)ListNumItems(seq->lDigitalBlocks)) insertPos = END_OF_LIST;
		pastedBlock = DIGITALBLOCK_insert (seq, insertPos);
		ListGetItem (blockClipboard, &b, i);
    	DIGITALBLOCK_duplicate (pastedBlock , b);
		
		// check waveforms
		for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
			//tprintf("Info pasting digital block wfm_id=%d (waveform list len: %d)\n",pastedBlock->waveforms[ch],ListNumItems (blockClipboardWaveforms));
			//ProcessDrawEvents();
										 
			pastedBlock->wfmP[ch] = NULL;
			if (pastedBlock->waveforms[ch] > (int)ListNumItems (blockClipboardWaveforms)) { 
				pastedBlock->waveforms[ch] = WFM_ID_UNCHANGED; 
				tprintf("Error: waveform for channel %d lost during copy and paste\n",ch);
				ProcessDrawEvents();
			}
			if (pastedBlock->waveforms[ch] > 0) {
				ListGetItem (blockClipboardWaveforms, &pastedWfm, pastedBlock->waveforms[ch]);
				
				pastedWfm->add = WFM_ID_UNCHANGED;; // DEBUG --> correctly paste added waveform
					
				wfm = WFM_ptr (seq, pastedBlock->waveforms[ch]);
				if (!WFM_isEqual (wfm, pastedWfm)) {
					tprintf("Error: duplicated wfm not equal to source\n");
					ProcessDrawEvents();
					// waveforms not equal:
					pastedBlock->waveforms[ch] = WFM_findIdentical (seq, pastedWfm);
					if (pastedBlock->waveforms[ch] == WFM_ID_UNCHANGED) {
						// none found --> add to list
						newWfm = WFM_new (seq);
						WFM_duplicate (newWfm, pastedWfm);
						pastedBlock->waveforms[ch] = ListNumItems (seq->lWaveforms);
					}					
				}
				pastedBlock->wfmP[ch] = WFM_ptr (seq, b->waveforms[ch]);
			}
		}			
		pastedBlock->seq = seq;
	}	
//	DIGITALBLOCKS_resetAllTableAttributes (seq);
	DIGITALBLOCKS_displayAllBlocks (seq);
}


void DIGITALBLOCKS_displaySequenceDuration (t_sequence *seq)
{
    t_digitalBlock *b;

    b = DIGITALBLOCK_ptr (seq, ListNumItems (seq->lDigitalBlocks));
	if (b != NULL) {
		setTime2 (panelMain, MAIN_NUMERIC_seqDuration, b->absoluteTime + b->duration, 1);
	}
}

//=======================================================================
//
//    calculate and display absolute times
//
//=======================================================================
void DIGITALBLOCKS_displayAbsoluteTime (t_sequence *seq, t_digitalBlock *b, int panel, int ctrlHeader)
{
    __int64 displayTime;
//    int loopRepetitions;
    int sign;
	Point cell;

	displayTime = b->absoluteTime - seq->referenceTime;
	sign = (displayTime >= 0) ? 1 : -1;

	cell = MakePoint (b->blockNr, BLOCK_TABLE_HEADER_ROW_absTime);
	if ((b->blockMode >=0) && !b->disable) {
		SetTableCellRangeAttribute (panel, ctrlHeader, MakeRect (cell.y, cell.x, 1, 1), ATTR_CELL_TYPE,
	 						        VAL_CELL_NUMERIC);
		setTimeTableCell (panel, ctrlHeader, cell, sign * displayTime, sign);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_COLOR,
							   b->isAbsoluteTimeReference ? VAL_BLUE : VAL_MED_GRAY);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_PRECISION, 1);

	}
	else {
		SetTableCellRangeAttribute (panel, ctrlHeader, MakeRect (cell.y, cell.x, 1, 1), ATTR_CELL_TYPE,
	 						        VAL_CELL_STRING);
	    SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CTRL_VAL, "");
	}

	SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_DIMMED,
						   b->disable);
	
}


void DIGITALBLOCKS_displayAllAbsoluteTimes(t_sequence *seq)
{
    t_digitalBlock *b;
    int i;
    //__int64 referenceTime;
	
	if (config->displayAbsoluteTime == 0) return;
	if (ListNumItems (seq->lDigitalBlocks) == 0) return;

    seq->referenceTime = DIGITALBLOCK_calculateAllAbsoluteTimes (seq, 0);
    // display absoluteTimes
	for (i = 1; i <= ListNumItems (seq->lDigitalBlocks); i++) {
		b = DIGITALBLOCK_ptr (seq, i);
		DIGITALBLOCKS_displayAbsoluteTime (seq, b, panelSequence, SEQUENCE_TABLE_header2);
	}	
	DIGITALBLOCKS_displaySequenceDuration (seq);
}


		 



//=======================================================================
//
//    variable time: opens new window to variable duration
//
//=======================================================================
void DIGITALBLOCKS_editVariableTime (t_digitalBlock *block, int top, int left)
{
 	// ----------------------------------------------
	//   init panel "variable time"
	// ----------------------------------------------
    variableTimeBlock = block;
 	// ----------------------------------------------
	//   everything painted red 
	// ----------------------------------------------
	SetPanelAttribute (panelVariableTime, ATTR_BACKCOLOR, VAL_COLOR_VARIABLE_TIME);
	SetAttributeForCtrls (panelVariableTime, ATTR_LABEL_BGCOLOR, 
						  VAL_COLOR_VARIABLE_TIME, 0,
						  VARTIME_CHECKBOX_varDuration,
						  VARTIME_NUMERIC_StartTime,
						  VARTIME_NUMERIC_IncrementTime,
						  VARTIME_NUMERIC_varTimeExec,
						  VARTIME_NUMERIC_varTimeStep,
						  VARTIME_NUMERIC_varTimeModulo,
						  0);
	SetAttributeForCtrls (panelVariableTime, ATTR_TEXT_BGCOLOR, 
						  VAL_COLOR_VARIABLE_TIME, 0,
						  VARTIME_TEXTMSG_ms,
						  VARTIME_TEXTMSG_ms_2,
						  VARTIME_TEXTMSG_rep,
						  VARTIME_TEXTMSG_rep2,
						  VARTIME_TEXTMSG_rep3,
						  0);
 	// ----------------------------------------------
	//   display times
	// ----------------------------------------------
	setTime (panelVariableTime, VARTIME_NUMERIC_StartTime, block->duration,1);
	setTime (panelVariableTime, VARTIME_NUMERIC_IncrementTime, abs(block->incrementTime),sign(block->incrementTime));
	SetCtrlVal (panelVariableTime, VARTIME_CHECKBOX_varDuration, block->variableTime);
	SetCtrlVal (panelVariableTime, VARTIME_NUMERIC_varTimeExec, block->variableTimeExecuteRep);
	SetCtrlVal (panelVariableTime, VARTIME_NUMERIC_varTimeExecOf, block->variableTimeExecuteRepOffset);
	SetCtrlVal (panelVariableTime, VARTIME_NUMERIC_varTimeModulo, block->variableTimeModulo);
	SetCtrlVal (panelVariableTime, VARTIME_NUMERIC_varTimeStep, block->variableTimeStepRep);
	SetActiveCtrl (panelVariableTime, VARTIME_NUMERIC_StartTime);
 	// ----------------------------------------------
	//   determine position of window + open it
	// ----------------------------------------------
	SetPanelPos (panelVariableTime, top, left);
	InstallPopup (panelVariableTime);
}
  
  
//=======================================================================
//
//    variable time: done
//
//=======================================================================
int CVICALLBACK VARTIME_BTNDone (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double dTime;
	unsigned long startTime, incrementTime;
	int old;
	int newVal;
	t_sequence *seq;
	t_digitalBlock *b;
	
	switch (event)
		{
		case EVENT_COMMIT:
		 	if (variableTimeBlock == NULL) return 0;
			seq = activeSeq();
		 	// ----------------------------------------------
			//   varaible time
			// ----------------------------------------------
			GetCtrlVal (panelVariableTime, VARTIME_CHECKBOX_varDuration, &newVal);
			if (newVal !=  variableTimeBlock->variableTime) {
			    setChanges (seq, 1);  
			    variableTimeBlock->variableTime = newVal;
			}
		 	// ----------------------------------------------
			//   get start time
			// ----------------------------------------------
   			GetCtrlVal (panel, VARTIME_NUMERIC_StartTime, &dTime);
			startTime =  timeToInt (dTime);
			if (startTime !=  variableTimeBlock->duration) {
			    setChanges (seq, 1);  
			    variableTimeBlock->duration = startTime;
			}
		 	// ----------------------------------------------
			//   get increment time
			// ----------------------------------------------
   			GetCtrlVal (panel, VARTIME_NUMERIC_IncrementTime, &dTime);
			incrementTime =  timeToInt (dTime);
			if ((incrementTime*sign(dTime)) !=  variableTimeBlock->incrementTime) {
			    setChanges (seq, 1);  
			    variableTimeBlock->incrementTime = incrementTime * sign(dTime);
			}
		 	// ----------------------------------------------
			//   get execute every n-th repetition
			// ----------------------------------------------
			old = variableTimeBlock->variableTimeExecuteRep;
			GetCtrlVal (panel, VARTIME_NUMERIC_varTimeExec, &variableTimeBlock->variableTimeExecuteRep);
			if (old != variableTimeBlock->variableTimeExecuteRep) setChanges (seq,1);

			old = variableTimeBlock->variableTimeExecuteRepOffset;
			GetCtrlVal (panel, VARTIME_NUMERIC_varTimeExecOf, &variableTimeBlock->variableTimeExecuteRepOffset);
			if (old != variableTimeBlock->variableTimeExecuteRepOffset) setChanges (seq,1);
		 	// ----------------------------------------------
			//   get step every n-th repetition
			// ----------------------------------------------
			old = variableTimeBlock->variableTimeStepRep;
			GetCtrlVal (panel, VARTIME_NUMERIC_varTimeStep, &variableTimeBlock->variableTimeStepRep);
			if (old != variableTimeBlock->variableTimeStepRep) setChanges (seq,1);

			old = variableTimeBlock->variableTimeModulo;
			GetCtrlVal (panel, VARTIME_NUMERIC_varTimeModulo, &variableTimeBlock->variableTimeModulo);
			if (old != variableTimeBlock->variableTimeModulo) setChanges (seq,1);
		 	// ----------------------------------------------
			//   uptdate values in digital block
			// ----------------------------------------------
//			displayDigitalBlock(seq, variableTimeBlock);
			b = variableTimeBlock;
			variableTimeBlock = NULL;
           	DIGITALBLOCKS_displayAllAbsoluteTimes(seq);
			DIGITALBLOCKS_displayInTable (seq, b->blockNr, panelSequence, 
										 SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
			PARAMETERSWEEPS_updateDisplay (seq);			
			RemovePopup (0);
			break;
		}
	return 0;
}

//=======================================================================
//
//    variable time: abort
//
//=======================================================================
int CVICALLBACK VARTIME_BTNAbort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			variableTimeBlock = NULL;
			RemovePopup (0);
			break;
		}
	return 0;
}

//=======================================================================
//
//    variable time: values changed
//
//=======================================================================
int CVICALLBACK VARTIME_edited (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double dTime;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, VARTIME_NUMERIC_StartTime, &dTime);
			setDoubleTime (panel, VARTIME_NUMERIC_StartTime, dTime);
			GetCtrlVal (panel, VARTIME_NUMERIC_IncrementTime,  &dTime);
			setDoubleTime (panel, VARTIME_NUMERIC_IncrementTime, dTime);
			break;
		case EVENT_KEYPRESS:
			switch (eventData1) {
				 case ',': FakeKeystroke ('.'); break;
				 case VAL_ENTER_VKEY:
					 switch (control) {
						 case VARTIME_NUMERIC_StartTime: 
							 SetActiveCtrl (panel, VARTIME_NUMERIC_IncrementTime); 
							 break;
						 case VARTIME_NUMERIC_IncrementTime: 
							 SetActiveCtrl (panel, VARTIME_BTN_DONE); 
							 break;
					 }
					 break;
			}
			
			break;

		}
	return 0;
}





    

/*
//=======================================================================
//
//   direct IO:  buttons on the left row pressed  
//   (to directly switch on and off channels)
//
//=======================================================================
int CVICALLBACK BTN_DirectOnOff (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    int i;
    int bit;
    t_sequence *seq;
   
    switch (event)
        {
        case EVENT_COMMIT:
            seq = activeSeq();
            // DEBUG nur einen schreiben
            for (i = 0; i < N_DIO_CHANNELS; i++) 
            	if (CTRL_FastOnOff[i] == control) {
            		GetCtrlVal (panel, CTRL_FastOnOff[i], &bit); 
            		bit ^= seq->DIO_invertDisplay[i];
           			sendDirectDigitalValue (i, bit);
           		}
            break;
        }
    return 0;
}

*/






void BLOCK_editVariablePulse (t_digitalBlock *d, int channel)
{
	if (panelVariablePulse == -1) 
		panelVariablePulse = LoadPanel (0, UIR_File, VAR_PULSE);

	setTime (panelVariablePulse, VAR_PULSE_NUMERIC_changeStart, 
			 abs(d->varyPulseStartIncrement[channel]),
			 sign(d->varyPulseStartIncrement[channel]));
	setTime (panelVariablePulse, VAR_PULSE_NUMERIC_changeStop, 
			 abs(d->varyPulseStopIncrement[channel]),
			 sign(d->varyPulseStopIncrement[channel]));
	SetCtrlVal (panelVariablePulse, VAR_PULSE_NUMERIC_stepRep, d->varyPulseStepRep[variableTimeChannel]);
	
	variableTimeChannel = channel;
	variableTimeBlock   = d;
	InstallPopup (panelVariablePulse);	
}


int CVICALLBACK BLOCK_variablePulseAbort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_COMMIT:
			RemovePopup (0);
			break;
	}
	return 0;
}

int CVICALLBACK BLOCK_variablePulseDone (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double dTime;
	int incr;
	t_sequence *seq;
	
	switch (event) {
		case EVENT_COMMIT:
   			// get start time
			seq = activeSeq();
   			GetCtrlVal (panel, VAR_PULSE_NUMERIC_changeStart, &dTime);
			incr =  timeToInt (dTime);
		    variableTimeBlock->varyPulseStartIncrement[variableTimeChannel] = incr * sign(dTime);
   			// get stop time
   			GetCtrlVal (panel, VAR_PULSE_NUMERIC_changeStop, &dTime);
			incr =  timeToInt (dTime);
		    variableTimeBlock->varyPulseStopIncrement[variableTimeChannel] = incr * sign(dTime);
			GetCtrlVal (panel, VAR_PULSE_NUMERIC_stepRep, &variableTimeBlock->varyPulseStepRep[variableTimeChannel]);
		    setChanges (seq, 1);  
			DIGITALBLOCKS_displayInTable (seq, variableTimeBlock->blockNr, panelSequence,
										 SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
			PARAMETERSWEEPS_updateDisplay (seq);			
			RemovePopup (0);
			break;
	}
	return 0;
}



void TABLE_initCommonProperties (int panel, int control, int isLeft)
{
	int col;
	
	DeleteTableColumns (panel, control, 1, -1);

	SetCtrlAttribute (panel, control, ATTR_AUTO_EDIT, 1);
	SetCtrlAttribute (panel, control, ATTR_COLUMN_LABELS_VISIBLE, 0);
	SetCtrlAttribute (panel, control, ATTR_ROW_LABELS_VISIBLE, 0);
	SetCtrlAttribute (panel, control, ATTR_TABLE_MODE, VAL_GRID);
	SetCtrlAttribute (panel, control, ATTR_ENABLE_ROW_SIZING, 0);
	SetCtrlAttribute (panel, control, ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute (panel, control, ATTR_TABLE_BGCOLOR, VAL_PANEL_GRAY);
//	SetCtrlAttribute (panel, control, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
//	SetCtrlAttribute (panel, control, ATTR_VERTICAL_GRID_VISIBLE, 1);

	SetCtrlAttribute (panel, control, ATTR_ENABLE_POPUP_MENU, 0);
	
	if (isLeft) {
		col = BLOCK_TABLE_COL_directIO;
		
		InsertTableColumns (panel, control, -1, 1,
							VAL_CELL_BUTTON);
		SetTableColumnAttribute (panel, control, col, ATTR_COLUMN_WIDTH, BLOCK_TABLE_COL_widths[col]);

	
		col = BLOCK_TABLE_COL_DIOName;
		InsertTableColumns (panel, control, -1, 1,
							VAL_CELL_STRING);
		SetTableColumnAttribute (panel, control, col, ATTR_COLUMN_WIDTH, BLOCK_TABLE_COL_widths[col]);

	
		col = BLOCK_TABLE_COL_channelName;

		InsertTableColumns (panel, control, -1, 1,
							VAL_CELL_STRING);
		SetTableColumnAttribute (panel, control, col, ATTR_COLUMN_WIDTH, BLOCK_TABLE_COL_widths[col]);
	}
}




void TABLE_HEADER_init (int panel, int control, int isLeft)
{
	//int nCols;
	//int nRows;
 	Rect cells;      
		
	TABLE_initCommonProperties (panel, control, isLeft);  

	setNumTableRows (panel, control, BLOCK_TABLE_HEADER_NRows-1, 20);
	SetTableRowAttribute (panel, control, BLOCK_TABLE_HEADER_ROW_absTime, ATTR_ROW_HEIGHT, TABLE_ROW_HEIGHT_HEADER_absTime );
	SetTableRowAttribute (panel, control, BLOCK_TABLE_HEADER_ROW_gpib, ATTR_ROW_HEIGHT, TABLE_ROW_HEIGHT_HEADER_gpib );
	SetTableRowAttribute (panel, control, BLOCK_TABLE_HEADER_ROW_name, ATTR_ROW_HEIGHT, TABLE_ROW_HEIGHT_HEADER_name);
	SetTableRowAttribute (panel, control,
						  BLOCK_TABLE_HEADER_ROW_name,
						  ATTR_WRAP_MODE, VAL_WORD_WRAP);

	if (isLeft) {

//		SetTableCellRangeAttribute (panel, control, cells,
//	    								ATTR_CELL_DIMMED, 1);
 		
		cells = MakeRect (1, BLOCK_TABLE_COL_DIOName, BLOCK_TABLE_HEADER_NRows-1, 1);
		
		SetTableCellRangeAttribute (panel, control, cells,
								ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_CELL_DIMMED, 1);

	
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_blockNo), ATTR_CTRL_VAL, 
							   "Block No.");		
//		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_properties), ATTR_CTRL_VAL, 
//							   "");		
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_absTime), ATTR_CTRL_VAL, 
							   "abs. time (ms)");		
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_name), ATTR_CTRL_VAL, 
							   "name");		
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_duration), ATTR_CTRL_VAL, 
							   "duration  (ms)");		
	
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_gpib), ATTR_CTRL_VAL, 
							   "GPIB command");		

		cells = MakeRect (1,BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_NRows-1, 1),
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_RIGHT_JUSTIFIED);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_TEXT_BOLD, 1);
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_absTime), ATTR_TEXT_BOLD, 0);
		SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_channelName, BLOCK_TABLE_HEADER_ROW_gpib), ATTR_TEXT_BOLD, 0);
							   
		SetTableCellRangeAttribute (panel, control, cells, ATTR_TEXT_BGCOLOR,
									VAL_LT_GRAY);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);

	}
}
	



void TABLE_DAC_init (int panel, int control, int isLeft)
{
	
	int i;
	int nRows = N_DAC_CHANNELS;
	Rect cells;
	//Point cell;
	

	TABLE_initCommonProperties (panel, control, isLeft);
	setNumTableRows (panel, control, nRows, 20);
	if (isLeft) {

		for (i = 0; i < nRows; i++) {
			SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_CTRL_VAL, 
								   str_AnalogChannelNames (i));		
			SetTableCellAttribute (panel, control,
								   MakePoint (BLOCK_TABLE_COL_channelName, i+1),
								   ATTR_TEXT_COLOR, colors[i % NColors]);
			SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_TEXT_BGCOLOR,
							   	   TABLE_COLOR_BUTTON_INACTIVE[i%2]);	
			
		}

		cells = MakeRect (1, BLOCK_TABLE_COL_directIO, N_DAC_CHANNELS, 1);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_PRECISION, 3);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_MAX_VALUE, 10.0);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_MIN_VALUE, -10.0);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_CELL_DIMMED, 0);
 		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_MODE,
									VAL_HOT);
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_RIGHT_JUSTIFIED);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_TEXT_BGCOLOR, VAL_CYAN);
		
		
		
		cells = MakeRect (1, BLOCK_TABLE_COL_DIOName, nRows, 1);
//		SetTableCellRangeAttribute (panel, control, cells, ATTR_TEXT_BGCOLOR,
//									VAL_PANEL_GRAY);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_TYPE,
									VAL_CELL_STRING);
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_TEXT_BOLD, 1);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_CENTER_JUSTIFIED);
 		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);



		cells = MakeRect (1, BLOCK_TABLE_COL_channelName, nRows, 1);
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_TEXT_BOLD, 1);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_JUSTIFY,
									VAL_CENTER_RIGHT_JUSTIFIED);
	}
}




void TABLE_DIGITAL_init (int panel, int control, int isLeft)
{

	int i;
	Rect cells;
	//Point cell;
	int nRows = N_TOTAL_DIO_CHANNELS;
	int rowHeight = 20;
	
	TABLE_initCommonProperties (panel, control, isLeft);

	
	setNumTableRows (panel, control, nRows, rowHeight);

	if (isLeft) {
		cells = MakeRect (1, BLOCK_TABLE_COL_directIO, nRows, 1);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_TYPE,
									VAL_CELL_BUTTON);

		for (i = 0; i < nRows; i++) {
			SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_CTRL_VAL, 
								   str_ChannelNames (i));	
			SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_TEXT_BGCOLOR,
							   TABLE_COLOR_BUTTON_INACTIVE[i%2]);	
			//SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_CELL_MODE, VAL_INDICATOR); 
			//SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_NO_EDIT_TEXT, 1);	
			//SetTableCellAttribute (panel, control, MakePoint (BLOCK_TABLE_COL_DIOName, i+1), ATTR_CELL_DIMMED, 1);	
			    

			SetTableCellAttribute (panel, control,
								   MakePoint (BLOCK_TABLE_COL_channelName, i+1),
								   ATTR_TEXT_COLOR, colors[i % NColors]);
			
//			SetTableCellAttribute (panel, control,
//								   MakePoint (BLOCK_TABLE_COL_directIO, i+1),
//								   ATTR_CMD_BUTTON_COLOR, colors[i % NColors]);
		}


		cells = MakeRect (1, BLOCK_TABLE_COL_DIOName, nRows, 1);
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_TEXT_BOLD, 1);
		SetTableCellRangeAttribute (panel, control, cells,
									ATTR_CELL_JUSTIFY,
									VAL_CENTER_CENTER_JUSTIFIED);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);



		cells = MakeRect (1, BLOCK_TABLE_COL_channelName, nRows, 1);
		SetTableCellRangeAttribute (panel, control, cells, 
									ATTR_TEXT_BOLD, 1);
		SetTableCellRangeAttribute (panel, control, cells, ATTR_CELL_JUSTIFY,
									VAL_CENTER_RIGHT_JUSTIFIED);
	}
}






void TABLE_initAll (void)
{	
	int width1;
	//int top;
	static int createFont = 1;
	
	if (createFont) {
		CreateMetaFont (VAL_DIALOG_META_FONT"BOLD", VAL_DIALOG_META_FONT, 13, 1, 0, 0, 0);
		createFont = 0;
		GetTextDisplaySize (str_AnalogChannelNames (15), VAL_DIALOG_META_FONT"BOLD", 0, &width1);
		BLOCK_TABLE_COL_widths[BLOCK_TABLE_COL_DIOName] = width1 + 4;
	}

	TABLE_HEADER_init (panelSequence, SEQUENCE_TABLE_header1, 1);
	TABLE_DIGITAL_init (panelSequence, SEQUENCE_TABLE_digital1, 1 );
	TABLE_DAC_init (panelSequence, SEQUENCE_TABLE_dac1, 1);
	
	TABLE_HEADER_init (panelSequence, SEQUENCE_TABLE_header2, 0);
	TABLE_DIGITAL_init (panelSequence, SEQUENCE_TABLE_digital2, 0);
	TABLE_DAC_init (panelSequence, SEQUENCE_TABLE_dac2, 0);
//	DIGITALBLOCKS_TABLE_init (panelSequence, SEQUENCE_TABLE, BLOCK_TABLE_COL_directIO, BLOCK_TABLE_HEADER_ROW_blockNo, NULL);
//	DIGITALBLOCKS_TABLE_init (panelSequence, SEQUENCE_TABLE_DIGITAL, BLOCK_TABLE_COL_directIO, BLOCK_TABLE_HEADER_ROW_blockNo, NULL);
	
	
	DIGITALBLOCKS_resizePanel();
}
	



void DIGITALBLOCKS_insertTableColumns (int panel, int ctrlHeader, int ctrlDac, int ctrlDigital, int startCol, int nColumns)
{
	//int nRows;
	//int test;
	Rect cells;
	

	cells = MakeRect (1, startCol, VAL_TO_EDGE, nColumns);
	
	//	
	//		Header
	//
	if (ctrlHeader > 0) {
		InsertTableColumns (panel, ctrlHeader, startCol, nColumns,
							VAL_USE_MASTER_CELL_TYPE);
	
		// blockNr
		cells = MakeRect (BLOCK_TABLE_HEADER_ROW_blockNo, startCol, 1, nColumns);
		SetTableCellRangeAttribute (panel,ctrlHeader, cells,
							   ATTR_CELL_JUSTIFY, VAL_CENTER_CENTER_JUSTIFIED);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells,
							   ATTR_TEXT_POINT_SIZE, 16);
		SetTableCellRangeAttribute (panel,ctrlHeader, cells,
							   ATTR_TEXT_BOLD, 1);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);
	
		// absolute time
		cells = MakeRect (BLOCK_TABLE_HEADER_ROW_absTime, startCol, 1, nColumns);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_CELL_TYPE,
									VAL_CELL_NUMERIC);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_DATA_TYPE,
									VAL_DOUBLE);
		SetTableCellRangeAttribute (panel,ctrlHeader, cells,
							   ATTR_CELL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_CELL_MODE,
									VAL_INDICATOR);

		// name
		cells = MakeRect (BLOCK_TABLE_HEADER_ROW_name, startCol, 1, nColumns);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells,
									ATTR_CELL_JUSTIFY,
									VAL_TOP_LEFT_JUSTIFIED);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells,
									ATTR_MAX_ENTRY_CHARS,
									MAX_DIGITALBLOCK_NAME_LEN-1);
	
	

		// duration
		cells = MakeRect (BLOCK_TABLE_HEADER_ROW_duration, startCol, 1, nColumns);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_CELL_TYPE,
									VAL_CELL_NUMERIC);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_DATA_TYPE,
									VAL_DOUBLE);
		SetTableCellRangeAttribute (panel,ctrlHeader, cells,
							   ATTR_CELL_JUSTIFY, VAL_CENTER_RIGHT_JUSTIFIED);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells,
									ATTR_SHOW_INCDEC_ARROWS, 1);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells,
									ATTR_CHECK_RANGE, VAL_COERCE);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_MIN_VALUE,
									0.0);
		SetTableCellRangeAttribute (panel, ctrlHeader, cells, ATTR_MAX_VALUE,
									999999.0);
	}
	
	if (ctrlDac > 0) {
		InsertTableColumns (panel, ctrlDac, startCol, nColumns,
							VAL_USE_MASTER_CELL_TYPE);
	};
	if (ctrlDigital > 0) {
		InsertTableColumns (panel, ctrlDigital, startCol, nColumns,
							VAL_USE_MASTER_CELL_TYPE);
		//GetNumTableColumns (panel, ctrlDigital, &test);
		//GetNumTableRows (panel, ctrlDigital, &nRows);
		SetTableCellRangeAttribute (panel, ctrlDigital,
									cells, ATTR_CELL_MODE, VAL_INDICATOR);
	}

}


void DIGITALBLOCKS_deleteTableColumns (int panel, int ctrlHeader, int ctrlDac, int ctrlDigital, int startCol, int nColumns)
{
	int nCols;
	
	DeleteTableColumns (panel, ctrlDigital,
						startCol, nColumns);
	DeleteTableColumns (panel, ctrlHeader,
						startCol, nColumns);
	
	GetNumTableColumns (panel, ctrlDac, &nCols);
	if (nCols > startCol) DeleteTableColumns (panel, ctrlDac, startCol, nColumns);
}




void DIGITALBLOCKS_displayHeaderInTable (t_sequence *seq, t_digitalBlock *b, int panel, int ctrlHeader)
{	
    //int sign;
	Point cell, cellName;
  // __int64 displayTime;
	
	DIGITALBLOCKS_displayBlockNrInTable (b);

	DIGITALBLOCKS_displayAbsoluteTime (seq, b, panel, ctrlHeader);

	
	cellName = MakePoint (b->blockNr, BLOCK_TABLE_HEADER_ROW_name);
	SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_CELL_DIMMED,
						   b->disable);

	
	cell = MakePoint (b->blockNr, BLOCK_TABLE_HEADER_ROW_duration);
	SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_TYPE,
						   VAL_CELL_NUMERIC);
	SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_DIMMED,
						   b->disable);
	
	
	if ((b->groupNr > 0) && (b->nElementsInGroup > 0) && (!b->isGroupExpanded)) {
		// in group!
		setTimeTableCell (panel, ctrlHeader, cell, b->groupDuration, 1);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_BOLD, 1);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_MODE,
							   VAL_INDICATOR);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_BGCOLOR,
							   VAL_MED_GRAY);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_DIMMED, 1);
		
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_CTRL_VAL, b->groupName);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_TEXT_BOLD, 1);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_CELL_JUSTIFY,
							   VAL_CENTER_LEFT_JUSTIFIED);
		SetTableCellAttribute (panel, ctrlHeader, cellName, 
							  ATTR_WRAP_MODE, VAL_WORD_WRAP);
	//	SetTableCellAttribute (panel, ctrlHeader, cellName,
	//						   ATTR_ENTER_IS_NEWLINE, 1);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_TEXT_BGCOLOR, VAL_WHITE);
		
	}
	else {
		setTimeTableCell (panel, ctrlHeader, cell, b->duration, 1);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_MODE,
							   VAL_HOT);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_BOLD, 0);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_BGCOLOR,
							   b->variableTime ? VAL_COLOR_VARIABLE_TIME : VAL_MED_GRAY);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_TEXT_COLOR,
							   b->variableTime ? VAL_WHITE : VAL_BLACK);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_DIMMED, 0);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CELL_DIMMED, 0);

		
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_CTRL_VAL, b->blockName);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_TEXT_BOLD, 0);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_CELL_JUSTIFY,
							   VAL_CENTER_LEFT_JUSTIFIED);
		SetTableCellAttribute (panel, ctrlHeader, cellName, 
							  ATTR_WRAP_MODE, VAL_WORD_WRAP);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_TEXT_BGCOLOR, b->waitForExtTrigger ? VAL_CYAN : VAL_WHITE);
		SetTableCellAttribute (panel, ctrlHeader, cellName, ATTR_TEXT_BGCOLOR, b->doTriggerLoop ? VAL_RED : (b->doRecvTriggerLoop ? VAL_DK_RED : VAL_WHITE));
		

		cell = MakePoint (b->blockNr, BLOCK_TABLE_HEADER_ROW_gpib);
		SetTableCellAttribute (panel, ctrlHeader, cell, ATTR_CTRL_VAL, GPIBCOMMAND_name (seq, b->gpibCommandNr));
		
		//SetTableCellAttribute (panel, ctrlHeader, cellName,
		//					   ATTR_ENTER_IS_NEWLINE, 1);
	}
	
}	






 int CVICALLBACK TABLE_digital1_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int vScrollOffset;
    t_sequence *seq;
 	Point cell, cellChName;
	int channel;
	char help[2000];
	int i;
	int color;
	Rect selection;
	
	//if (activeSeq()->isRunning && (event == EVENT_LEFT_CLICK_UP || event == EVENT_RIGHT_CLICK || event == EVENT_COMMIT)) {
	//	tprintf("Changes in running sequence are not allowed.\n");
	//	return 0;	
	//}
	
	
//	if (event != EVENT_MOUSE_POINTER_MOVE) {
//		tprintf ("panel= %d, ctrl=%d, events=%s\n", panel, control, eventStr(event,eventData1,eventData2));
//	}

	GetTableSelection (panel, control, &selection);
	switch (event)
	{

		case EVENT_COMMIT:
       		seq = activeSeq();
			if (seq == NULL) return 0;
			GetTableSelection (panel, control, &selection);
			if (RectEmpty (selection)) {
				selection.left = eventData2;
				selection.width = 1;
				selection.top = eventData1;
				selection.height = 1;
			}
			if (selection.left  == BLOCK_TABLE_COL_channelName) {
		        for (i = selection.top; i < selection.top+selection.height; i++) {
					//help = getTmpStr ();
					GetTableCellAttribute (panel, control, MakePoint (selection.left, i), ATTR_CTRL_VAL, help); 
		            seq->DIO_channelNames[i-1] = strnewcopy (seq->DIO_channelNames[i-1], help);
//		            seq->DIO_channelNames[cell.y-1] = strnewcopy (seq->DIO_channelNames[cell.y-1], help);
		            setChanges (seq, 1);
				}
			}
			break;
         case EVENT_RIGHT_CLICK:
            seq = activeSeq();
			
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			cellChName.x = BLOCK_TABLE_COL_channelName;
			cellChName.y = cell.y;
			GetTableCellAttribute (panel, control, cellChName, ATTR_TEXT_BGCOLOR, &color);
			SetTableCellAttribute (panel, control, cellChName, ATTR_TEXT_BGCOLOR, VAL_COLOR_SLATEBLUE1);
			ProcessDrawEvents();
			i = cell.y -1;
			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_ALWAYSON ,
								 ATTR_CHECKED, seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSON);
			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_ALWAYSOFF ,
								 ATTR_CHECKED, seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSOFF);
			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_STANDARD ,
								 ATTR_CHECKED, seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_STANDARD);
			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_INVERT_LOGIC,
								 ATTR_CHECKED, seq->DIO_invertDisplay[i]);
			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_LOCK,
								 ATTR_CHECKED, seq->DIO_lockChannel[i]);

			//			SetMenuBarAttribute (menuChannelProperties, MENU1_CHANNELS_INVERT_CHANNEL,
//								 ATTR_CHECKED, 0);
			switch (RunPopupMenu (menuChannelProperties, MENU1_CHANNELS,
								   panel, eventData1, eventData2, 0, 0, 0, 0)) {
				case MENU1_CHANNELS_INVERT_LOGIC: 
					seq->DIO_invertDisplay[i] ^= 1;
					if (seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSOFF)
						seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_ALWAYSON;
					else if (seq->DIO_channelStatus[i] == DIO_CHANNELSTATUS_ALWAYSON)
						seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_ALWAYSOFF;
							
					DIGITALBLOCKS_displayDirectIOButton (seq, i);					
//					DIGITALBLOCKS_setAllDirectIOValues (seq, 0, NULL);
					DIGITALBLOCKS_displayChannelNames (seq);
					DIGITALBLOCKS_displayDIOChannel (seq, i, 1, panel, SEQUENCE_TABLE_digital2);
					//DIGITALBLOCKS_displayAllBlocks (seq);
					setChanges (seq, 1);
					break;
				case MENU1_CHANNELS_FLIP_SELECTION: 
					DIGITALBLOCKS_flipChannel (seq, i);
					DIGITALBLOCKS_displayChannelNames (seq);
					DIGITALBLOCKS_displayDIOChannel (seq, i, 1, panel, SEQUENCE_TABLE_digital2);
//					DIGITALBLOCKS_displayAllBlocks (seq);
					setChanges (seq, 1);
					break;
				case MENU1_CHANNELS_LOCK: 
					seq->DIO_lockChannel[i] ^= 1;
					DIGITALBLOCKS_displayChannelNames (seq);
					DIGITALBLOCKS_displayDIOChannel (seq, i, 0, panel, SEQUENCE_TABLE_digital2);
					GetTableCellAttribute (panel, control, cellChName, ATTR_TEXT_BGCOLOR, &color);
					//DIGITALBLOCKS_displayAllBlocks (seq);
					setChanges (seq, 1);
					break;
				case MENU1_CHANNELS_ALWAYSOFF: 
					if (seq->DIO_channelStatus[i] != DIO_CHANNELSTATUS_ALWAYSOFF) setChanges (seq, 1); 
					seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_ALWAYSOFF;
					DIGITALBLOCKS_displayDIOChannel (seq, i, 1, panel, SEQUENCE_TABLE_digital2);
					//DIGITALBLOCKS_displayAllBlocks (seq);
					break;
				case MENU1_CHANNELS_ALWAYSON:
					if (seq->DIO_channelStatus[i] != DIO_CHANNELSTATUS_ALWAYSON) setChanges (seq, 1);
					seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_ALWAYSON;	
					DIGITALBLOCKS_displayDIOChannel (seq, i, 1, panel, SEQUENCE_TABLE_digital2);
					//DIGITALBLOCKS_displayAllBlocks(seq);
					break;
				case MENU1_CHANNELS_STANDARD:
					if (seq->DIO_channelStatus[i] != DIO_CHANNELSTATUS_STANDARD) setChanges (seq, 1);
					seq->DIO_channelStatus[i] = DIO_CHANNELSTATUS_STANDARD;	
					DIGITALBLOCKS_displayDIOChannel (seq, i, 1, panel, SEQUENCE_TABLE_digital2);
					//DIGITALBLOCKS_displayAllBlocks (seq);
					break;
				case MENU1_CHANNELS_HIDE_CHANNELS:
					if (selection.top == 0) {
						GetTableCellFromPoint (panel, control,
											  MakePoint (eventData2,eventData1), &cell);
						selection.top = cell.y;
						selection.height = 1;
					}
					for (i = selection.top; i < selection.top+selection.height; i++) {
						seq->tableConfig.DIOvisible[i-1] = 0;	
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_HIDE_UNUSED_CHANNELS:
					for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) {
						if (seq->DIO_channelNames[i] == NULL || strlen(seq->DIO_channelNames[i]) == 0) {
							seq->tableConfig.DIOvisible[i] = 0;
						}
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_SHOW_CHANNELS:
					if (selection.top == 0) {
						GetTableCellFromPoint (panel, control,
											  MakePoint (eventData2,eventData1), &cell);
						selection.top = cell.y;
						selection.height = 1;
					}
					for (i = selection.top; i < selection.top+selection.height; i++) {
						seq->tableConfig.DIOvisible[i-1] = 1;	
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_SHOW_ALL:
					for (i = 0; i < N_TOTAL_DIO_CHANNELS; i++) seq->tableConfig.DIOvisible[i] = 1;	
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_SAVE_CONFIG_CONF1:
					config->tableConfig[0] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU1_CHANNELS_SAVE_CONFIG_CONF2:
					config->tableConfig[1] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU1_CHANNELS_SAVE_CONFIG_CONF3:
					config->tableConfig[2] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU1_CHANNELS_GET_CONFIG_CONF1:
					seq->tableConfig = config->tableConfig[0];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_GET_CONFIG_CONF2:
					seq->tableConfig = config->tableConfig[1];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU1_CHANNELS_GET_CONFIG_CONF3:
					seq->tableConfig = config->tableConfig[2];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
			}
			SetTableCellAttribute (panel, control, cellChName, ATTR_TEXT_BGCOLOR, color);
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			if (cell.x == BLOCK_TABLE_COL_DIOName) {
				return 1;	
			}
			break;
        case EVENT_LEFT_CLICK_UP:
        	seq = activeSeq();
          	if (SEQUENCE_getStatus() != SEQ_STATUS_FINISHED) return 0;
			GetActiveTableCell (panel, control, &cell);
			if (cell.x == BLOCK_TABLE_COL_directIO) {
				channel = cell.y - 1;
				globalDigitalValues[channel] = !globalDigitalValues[channel];
	           	HARDWARE_sendDirectDigitalValue (channel, (short)globalDigitalValues[channel]);
				DIGITALBLOCKS_displayDirectIOButton (seq, channel);
			}
            break;
		case EVENT_VSCROLL:
			GetCtrlAttribute (panel, control, ATTR_VSCROLL_OFFSET, &vScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_VSCROLL_OFFSET, vScrollOffset);
			break;
	}
	return 0;
}




int CVICALLBACK TABLE_digital2_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	int hScrollOffset;
	int vScrollOffset;
	int ch = 0;
	Point cell = MakePoint(0,0);
	t_sequence *seq = 0;
	t_digitalBlock *p = 0;
	t_digitalBlock *firstOfGroup;
	
	
	//if (activeSeq()->isRunning && (event == EVENT_LEFT_CLICK || event == EVENT_RIGHT_CLICK || event == EVENT_LEFT_DOUBLE_CLICK)) {
	//	tprintf("Changes in running sequence are not allowed.\n");
	//	return 0;	
	//}
	
//	if (event != EVENT_MOUSE_POINTER_MOVE) {
//		DebugPrintf ("panel= %d, ctrl=%d, events=%s\n", panel, control, eventStr(event,eventData1,eventData2));
//	}
	
	
	switch (event) {
		case EVENT_LEFT_CLICK:
 		case EVENT_RIGHT_CLICK:	
		case EVENT_LEFT_DOUBLE_CLICK:
			seq = activeSeq();   
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			//:GetActiveTableCell (panel, control, &cell);
            
			p = DIGITALBLOCK_ptr (seq, cell.x);
			if (p == NULL) return 0;
//            if (p->blockMode != 0) return 0;
			ch = cell.y-1;
			break;
	}
	
/*	if (event != EVENT_MOUSE_POINTER_MOVE) {
		DebugPrintf ("panel= %d, ctrl=%d, events=%s\n", panel, control, eventStr(event,eventData1,eventData2));
	}
*/
	switch (event)
	{
		case EVENT_COMMIT:
			break;
			
		case EVENT_LEFT_DOUBLE_CLICK:
		case EVENT_LEFT_CLICK:
		 	// ----------------------------------------------
			//   copy value to sequence data
			// ----------------------------------------------
			if (seq->DIO_channelStatus[ch] != DIO_CHANNELSTATUS_STANDARD) return 1;
			if (seq->DIO_lockChannel[ch]) return 0;
			p->channels[ch] = !p->channels[ch];
			DIGITALBLOCKS_displayDigitalChannelSingle (seq, p, ch); 
			//DIGITALBLOCKS_displayDigitalChannel_fast (seq, &p, 1, panelSequence, SEQUENCE_TABLE_digital2, ch);
            setChanges (seq, 1);
			ProcessDrawEvents();
			if (event == EVENT_LEFT_DOUBLE_CLICK) return 1;
            break;
		case EVENT_RIGHT_CLICK:	
			if (seq->DIO_channelStatus[ch] != DIO_CHANNELSTATUS_STANDARD) return 0;
			if (p->groupNr > 0) {
				firstOfGroup = DIGITALBLOCKS_getFirstOfGroup (seq, cell.x);
				if (firstOfGroup == NULL) return 0;
				if (!firstOfGroup ->isGroupExpanded) return 0;
			}
			SetMenuBarAttribute (menuPulseProperties, MENU2_PULSEPROP_ALTERNATE, 
							     ATTR_CHECKED, p->alternate[ch]);
			SetMenuBarAttribute (menuPulseProperties, MENU2_PULSEPROP_VARPULSE_ON, 
							     ATTR_CHECKED, p->varyPulse[ch]);
			SetMenuBarAttribute (menuPulseProperties, MENU2_PULSEPROP_VARPULSE_OFF, 
							     ATTR_CHECKED, !p->varyPulse[ch]);
			switch (RunPopupMenu (menuPulseProperties, MENU2_PULSEPROP,
								   panel, eventData1, eventData2, 0, 0, 0, 0)) {
				case MENU2_PULSEPROP_ALTERNATE:
					p->alternate[ch] = !p->alternate[ch];
					setChanges (seq, 1);
					break;
				case MENU2_PULSEPROP_EDIT:
					BLOCK_editVariablePulse (p, ch);
					break;
					
				case MENU2_PULSEPROP_VARPULSE_ON:
					p->varyPulse[ch] = 1; 
            		setChanges (seq, 1);
					break;
				case MENU2_PULSEPROP_VARPULSE_OFF:
					p->varyPulse[ch] = 0; 
            		setChanges (seq, 1);
					break;
			}
			DIGITALBLOCKS_displayDigitalChannelSingle (seq, p, ch);
			break;
			
 		case EVENT_ACTIVE_CELL_CHANGE:
			return 1;
			
		case EVENT_HSCROLL:
			GetCtrlAttribute (panel, control, ATTR_HSCROLL_OFFSET, &hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			break;
		case EVENT_VSCROLL:
			GetCtrlAttribute (panel, control, ATTR_VSCROLL_OFFSET, &vScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital1, ATTR_VSCROLL_OFFSET, vScrollOffset);
			break;
	}
	return 0;
}



int CVICALLBACK TABLE_header1_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}



void DIGITALBLOCKS_EDIT_groupBlocks (t_sequence *seq, Rect selection)
{
	int groupNr;
	int i;
	t_digitalBlock *b;
	
	groupNr = ListNumItems (seq->lDigitalBlocks) + 1;
	
	for (i = selection.left; i < selection.left+selection.width; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		b->groupNr = groupNr;
	}
	
	ListGetItem (seq->lDigitalBlocks, &b, selection.left);
	b->isGroupExpanded = 0;
	DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (seq);
	DIGITALBLOCKS_createBitmaps (seq, b, -2, -2);

	DIGITALBLOCKS_displayAllBlocks (seq);
	
}





void DIGITALBLOCKS_EDIT_ungroupBlocks (t_sequence *seq, Rect selection)
{
	int i, n;
	t_digitalBlock *b;
	int lastGroupNr = 0; // TODO check if 0 is ok
	
	for (i = selection.left; i < selection.left+selection.width; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		lastGroupNr = b->groupNr;
		b->groupNr = 0;
	}
	
	n = ListNumItems(seq->lDigitalBlocks);
	for (i = selection.left+selection.width; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->groupNr == lastGroupNr) b->groupNr = 0;
		else break;
	}
	DIGITALBLOCKS_displayAllBlocks (seq);
}







void DIGITALBLOCKS_EDIT_disableBlocks (t_sequence *seq, Rect selection, int disable)
{
	//int groupNr;
	int i;
	t_digitalBlock *b;
	int nItems;
	
	nItems = ListNumItems (seq->lDigitalBlocks); 
	
	// select entire loop
	do {
		ListGetItem (seq->lDigitalBlocks, &b, selection.left);
		if (b->blockMode < 0) selection.left--;
	} while ((selection.left > 0) && (b->blockMode < 0));
	if (selection.left < 1) selection.left = 1;

	do {
		ListGetItem (seq->lDigitalBlocks, &b, selection.width+selection.left);
		if (b->blockMode < 0) selection.width++;
	} while ((selection.width+selection.left <= nItems) && (b->blockMode < 0));
	
	for (i = selection.left; i < selection.left+selection.width; i++) {
		if (i <= nItems) ListGetItem (seq->lDigitalBlocks, &b, i);
		b->disable = disable;
	}
	
	DIGITALBLOCKS_updateColumnWidthsAndBlockNrs (seq);
    seq->referenceTime = DIGITALBLOCK_calculateAllAbsoluteTimes (seq, 0);
	DIGITALBLOCKS_displaySequenceDuration (seq);
	
	DIGITALBLOCKS_createBitmaps (seq, b, -2, -2);
	DIGITALBLOCKS_displayAllBlocks (seq);
}




/*
void DIGITALBLOCKS_displayDIOchannelsAlwaysOnOff (t_sequence *seq)
{
	t_digitalBlock *b;
	int i;
	int ch;
	
	if (seq == NULL) return;
	
  	for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
		if ((seq->DIO_channelStatus[ch] == DIO_CHANNELSTATUS_ALWAYSON) || (seq->DIO_channelStatus[ch] == DIO_CHANNELSTATUS_ALWAYSOFF)) {
			for (i = ListNumItems (seq->lDigitalBlocks); i > 0; i--) {
				ListGetItem (seq->lDigitalBlocks, &b, i);
				DIGITALBLOCKS_displayDigitalChannels (seq, b, ch);

			}	
		}
	}
}
*/


int CVICALLBACK TABLE_header2_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int hScrollOffset;
	int ch;
	Point cell = MakePoint(0,0);
	Point newCell, cellBlockNr;
	t_sequence *seq = 0;
	t_digitalBlock *p = 0;
	t_digitalBlock *p2;
	double dTime;
	//char name[MAX_DIGITALBLOCK_NAME_LEN];
	int color;
	int width, pos;
	Rect selection = VAL_EMPTY_RECT;
	//int x;
	int ctrlMode;
	int gpibCommandNr;
	int choice;
	
	//if (activeSeq()->isRunning && (event == EVENT_LEFT_CLICK || event == EVENT_RIGHT_CLICK || event == EVENT_KEYPRESS || event == EVENT_COMMIT)) {
	//	tprintf("Changes in running sequence are not allowed.\n");
	//	return 0;	
	//}

//	if (event != EVENT_MOUSE_POINTER_MOVE) {
//		pprintf (panelMain, MAIN_TEXTBOX_Output1, "panel= %d, ctrl=%d, events=%s\n", panel, control, eventStr(event,eventData1,eventData2));
//	}

	GetCtrlAttribute (panel, control, ATTR_CTRL_MODE, &ctrlMode);

	 
	switch (event) {
		case EVENT_LEFT_CLICK:
 		case EVENT_RIGHT_CLICK:
			seq = activeSeq();
			GetTableSelection (panel, control, &selection);
			GetActiveTableCell (panel, control, &cell);

			GetTableCellFromPoint (panel, control,
								   MakePoint (eventData2, eventData1), &newCell);
//			if (RectEmpty(selection)) {
//				if ((newCell.x != cell.x) || (newCell.y != cell.y)) 
//					SetActiveTableCell (panel, control, newCell);
//			}

			cell = newCell;
            p = DIGITALBLOCK_ptr (seq, cell.x);     
			if (p == NULL) return 0;
			ch = cell.y-1;
			break;
	}
	switch (event)
	{
		case EVENT_KEYPRESS:
           	seq = activeSeq();
			GetTableSelection (panel, control, &selection);
			GetActiveTableCell (panel, control, &cell);
			if (RectEmpty(selection)) {
				selection.left = cell.x;
				selection.width = 1;
			}
			else if ((cell.x < selection.left) || (cell.x >= selection.left + selection.width)) {
				selection.left = cell.x;
				selection.width = 1;
				selection.top = cell.y;
				selection.height = 1;
				SetTableSelection (panel, control, selection);
			}
			p = DIGITALBLOCK_ptr (seq, selection.left);
			// select at least entire group 
			if ((p->groupNr > 0) && (!p->isGroupExpanded)) {
				selection.width	= max (p->nElementsInGroup, selection.width);
			}
			p2 = DIGITALBLOCK_ptr (seq, selection.left+selection.width-1);
			if ((p2->groupNr > 0) && (!p2->isGroupExpanded) && (p2->nElementsInGroup > 1)) {
				selection.width	+= p->nElementsInGroup-1;
			}
			if (selection.left+selection.width-1 > (int)ListNumItems (seq->lDigitalBlocks)) {
				selection.width = ListNumItems (seq->lDigitalBlocks)-selection.left+1;
			}
			
			switch (eventData1) {
				case VAL_CTRL_C_KEY:
					DIGITALBLOCKS_EDIT_copyToClipboard (seq, selection);
					DIGITALBLOCKS_displayBlockNrInTable (p);			
					break;
				case VAL_CTRL_X_KEY:
					DIGITALBLOCKS_EDIT_copyToClipboard (seq, selection);
					DIGITALBLOCKS_EDIT_deleteBlocks (seq, selection);
					setChanges (seq, 1);
					p = NULL;
					break;
				case VAL_CTRL_V_KEY:
					if (blockClipboard == NULL) return 0;
					DIGITALBLOCKS_EDIT_paste (seq, selection.left, 0);
					setChanges (seq, 1);
					break;
				case ',':
					if (selection.top != BLOCK_TABLE_HEADER_ROW_name) FakeKeystroke ('.');
					break;
			}
			break;
		case EVENT_LEFT_CLICK:
			cellBlockNr = MakePoint (cell.x, 1);
			if ((cell.y == BLOCK_TABLE_HEADER_ROW_duration) && p->variableTime) {
				GetTableCellAttribute (panel, control, cellBlockNr, ATTR_TEXT_BGCOLOR, &color);
				SetTableCellAttribute (panel, control, cellBlockNr, ATTR_TEXT_BGCOLOR, VAL_COLOR_SLATEBLUE1);
				variableTimeBlock = p; 				
				GetTableColumnAttribute (panel, control, cell.x,
										 ATTR_COLUMN_ACTUAL_WIDTH, &width);
				pos = width + eventData2;
				if (pos + panelWidth(panelVariableTime) > panelWidth (panelSequence)) 
					pos = eventData2 - panelWidth(panelVariableTime) - width;
				ProcessDrawEvents();
				//SetTableCellAttribute (panel, control, cellBlockNr, ATTR_TEXT_BGCOLOR, color);
				DIGITALBLOCKS_editVariableTime (p, eventData1, pos) ;
				//DIGITALBLOCKS_displayHeaderInTable (seq, p, panel, SEQUENCE_TABLE_header2);
			}
			break;
			
		case EVENT_RIGHT_CLICK:	
			if (ctrlMode == VAL_INDICATOR) return 0;
			cellBlockNr = MakePoint (cell.x, 1);
			GetTableCellAttribute (panel, control, cellBlockNr , ATTR_TEXT_BGCOLOR, &color);
			SetTableCellAttribute (panel, control, cellBlockNr , ATTR_TEXT_BGCOLOR, VAL_COLOR_SLATEBLUE1);
			
			SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_DELETE,
			                     ATTR_DIMMED, 
			                     (ListNumItems (seq->lDigitalBlocks) <= 1));
			SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_VARTIME, ATTR_CHECKED,
								 p->variableTime);
			SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_TRIGGER, ATTR_CHECKED,
								 p->waitForExtTrigger);
			SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_LOOP_TRIG, ATTR_CHECKED,
								 p->doTriggerLoop);
			SetMenuBarAttribute (menuBlock, MENU3_BLKEDIT_RECV_LOOP_TRIG, ATTR_CHECKED,
								 p->doRecvTriggerLoop);
			if (RectEmpty(selection)) {
				selection.left = cell.x;
				selection.width = 1;
			}
			else if ((cell.x < selection.left) || (cell.x >= selection.left + selection.width)) {
				selection.left = cell.x;
				selection.width = 1;
				selection.top = cell.y;
				selection.height = 1;
				SetTableSelection (panel, control, selection);
			}
			p = DIGITALBLOCK_ptr (seq, selection.left);
			// select at least entire group 
			if ((p->groupNr > 0) && (!p->isGroupExpanded)) {
				selection.width	= max (p->nElementsInGroup, selection.width);
			}
			p2 = DIGITALBLOCK_ptr (seq, selection.left+selection.width-1);
			if ((p2->groupNr > 0) && (!p2->isGroupExpanded) && (p2->nElementsInGroup > 1)) {
				selection.width	+= p->nElementsInGroup-1;
			}
			if (selection.left+selection.width-1 > (int)ListNumItems (seq->lDigitalBlocks)) {
				selection.width = ListNumItems (seq->lDigitalBlocks)-selection.left+1;
			}
			DIGITALBLOCKS_configureBlockPopUpMenu (seq, selection);
			switch (choice = RunPopupMenu (menuBlock, MENU3_BLKEDIT, panel, 
					eventData1, eventData2, 0, 0, 0, 0)) {
				case MENU3_BLKEDIT_NEW_BEFORE:
					DIGITALBLOCKS_EDIT_insertNew  (seq, selection.left, 0);
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_NEW_BEHIND:
					DIGITALBLOCKS_EDIT_insertNew (seq, selection.left+selection.width-1, 1);
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_DISABLE:
					DIGITALBLOCKS_EDIT_disableBlocks (seq, selection, 1);
					break;
				case MENU3_BLKEDIT_ENABLE:
					DIGITALBLOCKS_EDIT_disableBlocks (seq, selection, 0);
					break;
				case MENU3_BLKEDIT_DELETE:
					if (ConfirmPopupf ("Delete block(s)", "Do you really want to delete block no. %d%s%s?", 
						selection.left,
						selection.width > 1 ? " to " : "",
						selection.width > 1 ? intToStr(selection.left+selection.width-1) : "")) {
						DIGITALBLOCKS_EDIT_deleteBlocks (seq, selection);
						PARAMETERSWEEPS_updateDisplay (seq);			
						setChanges (seq, 1);
					}
					p = NULL;
					break;
				case MENU3_BLKEDIT_SET_LOOP:
					DIGITALBLOCKS_EDIT_setLoop (seq, selection);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_SET_LOOPREPS:
					DIGITALBLOCKS_EDIT_setLoopReps (seq, selection, 
						eventData1+ctrlTop(panel, control),
						eventData2+ctrlLeft(panel, control));
				
					break;
				case MENU3_BLKEDIT_DELETE_LOOP:
					DIGITALBLOCKS_EDIT_deleteLoop (seq, p->blockNr);
					setChanges (seq, 1);
					break;
					
				case MENU3_BLKEDIT_COPY:
					DIGITALBLOCKS_EDIT_copyToClipboard (seq, selection);
					DIGITALBLOCKS_displayBlockNrInTable (p);			
					break;
				case MENU3_BLKEDIT_CUT:
					DIGITALBLOCKS_EDIT_copyToClipboard (seq, selection);
					DIGITALBLOCKS_EDIT_deleteBlocks (seq, selection);
					setChanges (seq, 1);
					p = NULL;
					break;
				case MENU3_BLKEDIT_PASTE_BEFORE:
					DIGITALBLOCKS_EDIT_paste (seq, selection.left, 0);
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_PASTE_BEHIND:
					DIGITALBLOCKS_EDIT_paste (seq, selection.left+selection.width-1, 1);
					PARAMETERSWEEPS_updateDisplay (seq);			
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_GROUP:
					DIGITALBLOCKS_EDIT_groupBlocks (seq, selection);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_UNGROUP:
					if (p->groupNr > 0) {
						// get first element of group
						do {
							selection.left--;
							p2 = DIGITALBLOCK_ptr (seq, selection.left);
						} while ((p2 != NULL) && (p2->groupNr == p->groupNr));
						selection.left++;
					}
					DIGITALBLOCKS_EDIT_ungroupBlocks (seq, selection);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_EXPAND:
					p->isGroupExpanded = 1;
					DIGITALBLOCKS_displayAllBlocks (seq);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_COMPRESS:
					p = DIGITALBLOCKS_getFirstOfGroup (seq, selection.left);
					if (p == NULL) return 0;
					p->isGroupExpanded = 0;
					DIGITALBLOCKS_createBitmaps (seq, p, -2, -2);
					DIGITALBLOCKS_displayAllBlocks (seq);
					setChanges (seq, 1);
					break;					
				case MENU3_BLKEDIT_VARTIME:
					p->variableTime = !p->variableTime;
     				if (p->variableTime) {
						GetTableColumnAttribute (panel, control, cell.x,
												 ATTR_COLUMN_ACTUAL_WIDTH, &width);
						pos = width + eventData2;
						if (pos + panelWidth(panelVariableTime) > panelWidth (panelSequence)) 
							pos = eventData2 - panelWidth(panelVariableTime) - width;
						ProcessDrawEvents();
						variableTimeBlock = p;
						DIGITALBLOCKS_editVariableTime (p, eventData1, pos);
						color = VAL_COLOR_SLATEBLUE1;
					}
					else {
						DIGITALBLOCKS_displayInTable (seq, p->blockNr, panelSequence, 
						 			 				  SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);
					}
					PARAMETERSWEEPS_updateDisplay (seq); 					
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_TRIGGER:
					p->waitForExtTrigger = !p->waitForExtTrigger;
					DIGITALBLOCKS_displayHeaderInTable (seq, p, panel, SEQUENCE_TABLE_header2);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_TIMEREF:
					DIGITALBLOCKS_EDIT_setAsTimereference (seq, p->blockNr);
					DIGITALBLOCKS_displayAllAbsoluteTimes(seq);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_SHOW_ABSOLUTE:
					//setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_LOOP_TRIG:
					// if selecting p->triggerLoop means both loops are selected, don't allow it to happen
					if (p->doRecvTriggerLoop && !p->doTriggerLoop)
					{
						break;	
					}
					p->doTriggerLoop = !p->doTriggerLoop;
					sendingTrigger = !sendingTrigger;
					if (p->doTriggerLoop)
					{
						// inserts right after "NONE"
						t_gpibCommand *g = GPIBCOMMAND_new_at_front(seq);
						p->gpibCommandNr = 1;
						GPIBCOMMAND_init(g, GPIB_LOOP_TRIGGER);
						g->gpibAddress = 0; 
						g->noGPIBdevice = 1;
						g->transmitAfterStartOfSequence = 1;
					}
					else
					{
						GPIBCOMMAND_delete(seq, 1);
					}
					DIGITALBLOCKS_displayHeaderInTable (seq, p, panel, SEQUENCE_TABLE_header2);
					setChanges (seq, 1);
					break;
				case MENU3_BLKEDIT_RECV_LOOP_TRIG:
					// if selecting p->recvTriggerLoop means both loops are selected, don't allow it to happen
					if (p->doTriggerLoop && !p->doRecvTriggerLoop)
					{
						break;	
					}
					p->doRecvTriggerLoop = !p->doRecvTriggerLoop;
					if (p->doRecvTriggerLoop)
					{
						t_gpibCommand *g;
						if (!sendingTrigger)
						{
							g = GPIBCOMMAND_new_at_front(seq);
							p->gpibCommandNr = 1;
						}
						else
						{
							g = GPIBCOMMAND_new_at_second(seq);
							p->gpibCommandNr = 2;	
						} 
						GPIBCOMMAND_init(g, GPIB_RECV_LOOP_TRIGGER);
						g->gpibAddress = 0; 
						g->noGPIBdevice = 1;
						g->transmitAfterStartOfSequence = 1;
					}
					else
					{
						if (!sendingTrigger)
						{ 	 
							GPIBCOMMAND_delete(seq, 1);
						}
						else
						{
							GPIBCOMMAND_delete(seq, 2);	
						}
					}
					DIGITALBLOCKS_displayHeaderInTable (seq, p, panel, SEQUENCE_TABLE_header2);
					setChanges (seq, 1);
					break;
				default:
					int actualChoice = choice;
					if (p->doTriggerLoop)
					{
						actualChoice++;	
					}
					if (p->doRecvTriggerLoop)
					{
						actualChoice++;	
					}
					gpibCommandNr = GPIBCOMMAND_nrFromMenuID (seq, menuBlock, choice);
					if (gpibCommandNr >= 0) {
						p->gpibCommandNr = gpibCommandNr;
						setChanges (seq, 1);
					};
					DIGITALBLOCKS_displayHeaderInTable (seq, p, panel, SEQUENCE_TABLE_header2);
			}
			DIGITALBLOCKS_displayBlockNrInTable (p);			
			break;
		case EVENT_COMMIT:
//		case EVENT_VAL_CHANGED:
			cell = MakePoint (eventData2, eventData1);
           	seq = activeSeq();
            p = DIGITALBLOCK_ptr (seq, cell.x);     
			if (p == NULL) return 0;
			switch (eventData1) {
				case BLOCK_TABLE_HEADER_ROW_name:
					if ((p->groupNr > 0) && (!p->isGroupExpanded) && (p->nElementsInGroup > 0)) {
						GetTableCellVal (panel, control, cell, p->groupName);
					}
					else {
						GetTableCellVal (panel, control, cell, p->blockName);
					}

					
					setChanges (seq, 1);
					break;
				case BLOCK_TABLE_HEADER_ROW_duration:
					GetTableCellVal (panel, control, cell, &dTime);
					
					p->duration = timeToInt (dTime);
					setTimeTableCell (panel, control, cell, p->duration, 1);
		            setChanges (seq, 1);
		    		DIGITALBLOCKS_displayAllAbsoluteTimes(seq);
					DIGITALBLOCKS_displaySequenceDuration (seq);
					if (p->variableTime) PARAMETERSWEEPS_updateDisplay (seq); 					
					setChanges (seq, 1);
//					pprintf (panelMain, MAIN_TEXTBOX_Output2, "Time: %3.1f\n", p->duration / (1.0 * VAL_ms));
					break;
				case 0:
					// multiple cells affected
					break;
			}
			
			break;
		case EVENT_HSCROLL:
			GetCtrlAttribute (panel, control, ATTR_HSCROLL_OFFSET, &hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			break;
	}
	return 0;
}


void TABLE_changeCellIncDecValue (int panel, int control, double changeMultiply)
{
	Point cell;
	int cellType;
	double incValue;
	int precision;
	
	GetActiveTableCell (panel, control, &cell);
	GetTableCellAttribute (panel, control, cell, ATTR_CELL_TYPE,
						   &cellType);
	if (cellType != VAL_CELL_NUMERIC) return;
	GetTableCellAttribute (panel, control, cell, ATTR_INCR_VALUE,
						   &incValue);
	GetTableCellAttribute (panel, control, cell, ATTR_PRECISION, &precision);
	incValue *= changeMultiply;
	if (incValue > 1.0) incValue = 1.0;
	if (incValue < pow (10, -precision)) incValue = pow (10, -precision);
	SetTableCellAttribute (panel, control, cell, ATTR_INCR_VALUE,
						   incValue);
}


 
int CVICALLBACK TABLE_dac1_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int vScrollOffset;
	t_sequence *seq;
	int ch;
	Point cell;
	Rect selection;
	char help[2000];
	//double voltage;
	int i;

	switch (event)
	{
		case EVENT_KEYPRESS:
			GetActiveTableCell (panel, control, &cell);
			switch (eventData1) {
				case VAL_LEFT_ARROW_VKEY:
					if (cell.x == BLOCK_TABLE_COL_directIO) {			
						TABLE_changeCellIncDecValue (panel, control, 10);
						return 1;
					}
					break;
				case VAL_RIGHT_ARROW_VKEY:
					if (cell.x == BLOCK_TABLE_COL_directIO) {			
						TABLE_changeCellIncDecValue (panel, control, 0.1);
						return 1;
					}
					break;
				case ',':
					FakeKeystroke ('.');
					break;
				case VAL_ENTER_VKEY:
					if (cell.x == BLOCK_TABLE_COL_directIO) FakeKeystroke (VAL_SHIFT_MODIFIER | VAL_ENTER_VKEY);
					break;
			}
			break;
		
		case EVENT_COMMIT:
			seq = activeSeq ();
			cell = MakePoint (eventData2, eventData1);
			
			switch (eventData2) {
				case BLOCK_TABLE_COL_directIO:
					ch = eventData1-1;
					if ((ch < 0) || (ch >= N_DAC_CHANNELS)) break;
					GetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, &globalAnalogVoltages[ch]); 
			    	HARDWARE_sendDirectAnalogValue (ch, globalAnalogVoltages[ch]);
					//tprintf("changed direct value\n"); // TODO remove
					break;
				case BLOCK_TABLE_COL_channelName:
					ch = eventData1-1;
					if ((ch < 0) || (ch >= N_DAC_CHANNELS)) break;
			        //help = getTmpStr ();
					GetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, help); 
		            seq->AO_channelNames[ch] = strnewcopy (seq->AO_channelNames[ch], help);
					setChanges (seq, 1);
					break;
			}
			break;		 
		case EVENT_VSCROLL:
			GetCtrlAttribute (panel, control, ATTR_VSCROLL_OFFSET, &vScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_VSCROLL_OFFSET, vScrollOffset);
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			if (cell.x == BLOCK_TABLE_COL_DIOName) {
				return 1;	
			}
			break;
         case EVENT_RIGHT_CLICK:
            seq = activeSeq();
			GetTableSelection (panel, control, &selection);
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			switch (RunPopupMenu (menuDACChannelProperties, MENU7_CHANNELS,
								   panel, eventData1, eventData2, 0, 0, 0, 0)) {
				case MENU7_CHANNELS_HIDE_CHANNELS:
					if (selection.top == 0) {
						GetTableCellFromPoint (panel, control,
											  MakePoint (eventData2,eventData1), &cell);
						selection.top = cell.y;
						selection.height = 1;
					}
					for (i = selection.top; i < selection.top+selection.height; i++) {
						seq->tableConfig.AOvisible[i-1] = 0;	
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_HIDE_UNUSED_CHANNELS:
					for (i = 0; i < N_DAC_CHANNELS; i++) {
						if (seq->AO_channelNames[i] == NULL || strlen(seq->AO_channelNames[i]) == 0) {
							seq->tableConfig.AOvisible[i] = 0;
						}
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_SHOW_CHANNELS:
					if (selection.top == 0) {
						GetTableCellFromPoint (panel, control,
											  MakePoint (eventData2,eventData1), &cell);
						selection.top = cell.y;
						selection.height = 1;
					}
					for (i = selection.top; i < selection.top+selection.height; i++) {
						seq->tableConfig.AOvisible[i-1] = 1;	
					}
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_SHOW_ALL:
					for (i = 0; i < N_DAC_CHANNELS; i++) seq->tableConfig.AOvisible[i] = 1;	
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_SET_ALL_ZERO:
					for (i = 0; i < N_DAC_CHANNELS; i++) { 
						globalAnalogVoltages[i] = 0;
			    		HARDWARE_sendDirectAnalogValue (i, globalAnalogVoltages[i]);
					}
					DIGITALBLOCKS_displayGlobalAnalogVoltages();
					break;
				case MENU7_CHANNELS_SAVE_CONFIG_CONF1:
					config->tableConfig[0] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU7_CHANNELS_SAVE_CONFIG_CONF2:
					config->tableConfig[1] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU7_CHANNELS_SAVE_CONFIG_CONF3:
					config->tableConfig[2] = seq->tableConfig;
					CONFIG_write (config);
					break;
				case MENU7_CHANNELS_GET_CONFIG_CONF1:
					seq->tableConfig = config->tableConfig[0];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_GET_CONFIG_CONF2:
					seq->tableConfig = config->tableConfig[1];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
				case MENU7_CHANNELS_GET_CONFIG_CONF3:
					seq->tableConfig = config->tableConfig[2];
					setChanges (seq, 1);
					DIGITALBLOCKS_setHeightForAllRows (seq);
					break;
			}
	}
	return 0;
}
	




int CVICALLBACK TABLE_dac2_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int vScrollOffset;
	int hScrollOffset;
 	Point cell = MakePoint(0,0);  
	t_sequence *seq = 0;
	t_digitalBlock *p = 0;
	int ch = 0;
	t_waveform *wfm;
	int color;
	int choice;
	int wfmNr;
	int width, pos;
	
	//if (activeSeq()->isRunning && (event == EVENT_LEFT_CLICK || event == EVENT_RIGHT_CLICK || event == EVENT_KEYPRESS || event == EVENT_COMMIT)) {
	//	tprintf("Changes in running sequence are not allowed.\n");
	//	return 0;	
	//}

//	if (event != EVENT_MOUSE_POINTER_MOVE) {
//		tprintf ("panel= %d, ctrl=%d, events=%s\n", panel, control, eventStr(event,eventData1,eventData2));
//	}
	switch (event) {
		case EVENT_LEFT_DOUBLE_CLICK:
		case EVENT_LEFT_CLICK:
 		case EVENT_RIGHT_CLICK:	
           	seq = activeSeq();
			GetTableCellFromPoint (panel, control,
										  MakePoint (eventData2,eventData1), &cell);
			//:GetActiveTableCell (panel, control, &cell);
            p = DIGITALBLOCK_ptr (seq, cell.x);     
			if (p == NULL) return 0;
//            if (p->blockMode != 0) return 0;
			ch = cell.y-1;
			break;
	}

	switch (event)
	{
		case EVENT_KEYPRESS:
			switch (eventData1) {
				case ',':
					FakeKeystroke ('.');
					break;
			}
			break;
		case EVENT_COMMIT:
           	seq = activeSeq();
			cell.x = eventData2;
			cell.y = eventData1;
            p = DIGITALBLOCK_ptr (seq, cell.x);     
			ch = cell.y-1;
			if (p == NULL) return 0;
			switch (p->waveforms[ch]) {
				case WFM_ID_HOLD:
					break;
				case WFM_ID_CONSTVOLTAGE:
					GetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL,
									       &p->constVoltages[ch]);
					setChanges (seq, 1);
					break;
				case WFM_ID_UNCHANGED:
					break;
				default:
					wfm = WFM_ptr (seq, p->waveforms[ch]);
					break;
			}
			break;
											
		case EVENT_RIGHT_CLICK:
			if ((p->groupNr > 0) && (!p->isGroupExpanded) && (p->nElementsInGroup > 0)) return 0;
			GetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR, &color);
			SetTableCellAttribute (panel, control, cell, ATTR_TEXT_BGCOLOR, VAL_BLUE);
			ProcessDrawEvents();
			DIGITALBLOCKS_insertWaveformsToMenu (seq, ch);
			SetMenuBarAttribute (menuDacProperties, MENU4_DAC_WAVEFORM, ATTR_BOLD,
								 p->waveforms[ch] <= 0);
			SetMenuBarAttribute (menuDacProperties, MENU4_DAC_EDIT, ATTR_DIMMED, 
								 p->waveforms[ch] <= 0);
			SetMenuBarAttribute (menuDacProperties, MENU4_DAC_WAVEFORM, ATTR_DIMMED, 
								 ListNumItems (seq->lWaveforms) == 0);
			ProcessDrawEvents();
			switch (choice = RunPopupMenu (menuDacProperties, MENU4_DAC,
								   panel, eventData1, eventData2, 0, 0, 0, 0)) {
				case 0:
					break;
				case MENU4_DAC_EDIT:
					SetActiveTabPage (panelMain2, MAIN2_TAB0, TAB0_INDEX_WFM);
					WFM_edit (activeSeq(), p->waveforms[ch], 1);
					break;
				case MENU4_DAC_UNCHANGED:
					p->waveforms[ch] = WFM_ID_UNCHANGED;
					setChanges (seq, 1);
					break;
				case MENU4_DAC_CONST:
					p->waveforms[ch] = WFM_ID_CONSTVOLTAGE;
					setChanges (seq, 1);
									
					SetCtrlAttribute (panel, control, ATTR_TABLE_RUN_STATE,
									  VAL_EDIT_STATE);
					
					//return 1;
					break;
				case MENU4_DAC_WAVEFORM:
					setChanges (seq, 1);
					break;
				case MENU4_DAC_HOLD:
					p->waveforms[ch] = WFM_ID_HOLD;
					setChanges (seq, 1);
					break;
				default:
					wfmNr = WFM_nrFromMenuID (seq, choice);
					if (wfmNr != WFM_ID_UNCHANGED) p->waveforms[ch] = wfmNr;
					PARAMETERSWEEPS_updateDisplay (seq);
					setChanges (seq, 1);
			}
			DIGITALBLOCKS_displayDACChannelInTable (panel, control, seq, p, ch);
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
		case EVENT_LEFT_CLICK:
			if (p->waveforms[ch] > 0) {
				GetTableColumnAttribute (panel, control, cell.x,
										 ATTR_COLUMN_ACTUAL_WIDTH, &width);
				pos = width + eventData2;
				if (pos + panelWidth(panelWfmSmall) > panelWidth (panelSequence)) 
					pos = eventData2 - panelWidth(panelWfmSmall) - width;
				DIGITALBLOCKS_smallWfmShowPanel (eventData1, pos, seq, p->waveforms[ch]); 
			}
			else HidePanel (panelWfmSmall);
			if (p->waveforms[ch] != WFM_ID_CONSTVOLTAGE) {
				return 1; // avoid text edit	
			}
			break;
		case EVENT_HSCROLL:
			GetCtrlAttribute (panel, control, ATTR_HSCROLL_OFFSET, &hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_digital2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header2, ATTR_HSCROLL_OFFSET, hScrollOffset);
			break;
			
		case EVENT_VSCROLL:
			GetCtrlAttribute (panel, control, ATTR_VSCROLL_OFFSET, &vScrollOffset);
			SetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac1, ATTR_VSCROLL_OFFSET, vScrollOffset);
			break;
		case EVENT_LOST_FOCUS:
			
			break;
			
	}
	return 0;
}



int CVICALLBACK DIGITALBLOCKS_smallWfmPanelClicked (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
//	tprintf ("panel: %s\n", eventStr(event,eventData1,eventData2));
	
	t_waveform *wfm;

	switch (event)
	{
		case EVENT_KEYPRESS:
			if (eventData1 == VAL_ESC_VKEY) 
				HidePanel (panelWfmSmall); 
			break;
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:
			HidePanel (panelWfmSmall); 
			break;
		case EVENT_CLOSE:
		case EVENT_LEFT_CLICK:
			GetPanelAttribute (panelWfmSmall, ATTR_CALLBACK_DATA, &wfm);
			if (wfm->type != WFM_TYPE_STEP) HidePanel (panelWfmSmall);
			break;
	}
	return 0;
}


int CVICALLBACK DIGITALBLOCKS_smallWfmPanelDoneBtnClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	tprintf ("ctrl: %s\n", eventStr(event,eventData1,eventData2));
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (panelWfmSmall);
			break;
	}
	return 0;
}


int CVICALLBACK DIGITALBLOCKS_smallWfmPanelStepCtrlClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_waveform *wfm;
	t_sequence *seq;

	switch (event)
	{
		case EVENT_COMMIT:
			GetPanelAttribute (panelWfmSmall, ATTR_CALLBACK_DATA, &wfm);
			if (wfm->type != WFM_TYPE_STEP) HidePanel (panelWfmSmall);
			GetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepRep, &wfm->stepRep);
			GetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepFrom, &wfm->stepFrom);
			GetCtrlVal (panelWfmSmall, WFMSMALL_NUMERIC_stepTo, &wfm->stepTo);
			seq = activeSeq();
			setChanges (seq, 1);
			PARAMETERSWEEPS_updateDisplay (seq);			
			break;
		case EVENT_LEFT_CLICK:
			break;
	}
	return 0;
}


int CVICALLBACK DIGITALBLOCKS_smallWfmPanelCtrlClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	tprintf ("ctrl: %s\n", eventStr(event,eventData1,eventData2));
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_LEFT_CLICK:
			HidePanel (panelWfmSmall);
			break;
	}
	return 0;
}



int CVICALLBACK DIGITALBLOCKS_splitterMoved (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	tprintf ("ctrl: %s\n", eventStr(event,eventData1,eventData2));

	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_DROP:
			GetCtrlAttribute (panelSequence, SEQUENCE_TABLE_dac2, ATTR_HEIGHT, &config->splitterHPos);
			break;
	}
	return 0;
}


int CVICALLBACK DIGITALBLOCKS_VsplitterMoved (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

//	if (event != EVENT_MOUSE_POINTER_MOVE) {
//		tprintf ("%s\n", eventStr (event, eventData1, eventData2));
//	}
	
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_DROP:
		case EVENT_DROPPED:
			GetCtrlAttribute (panelSequence, SEQUENCE_TABLE_header1, ATTR_WIDTH, &config->splitterVPos);
			DIGITALBLOCKS_setTableComlumWidth_channelNames ();
			ProcessDrawEvents();
			break;
	}
	return 0;
}



int CVICALLBACK VARTIME_panelCallback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			variableTimeBlock = NULL;
			RemovePopup (0);
			break;
	}
	return 0;
}





int CVICALLBACK BLOCK_loopAbort (int panel, int control, int event,
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



int CVICALLBACK BLOCK_loopDone (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	int nReps;
	
	switch (event)
	{
		case EVENT_COMMIT:
   			// get start time
			seq = activeSeq();
   			GetCtrlVal (panel, LOOP_NUMERIC_nReps, &nReps);
			if (nReps != loopBlock->blockMode) setChanges (seq, 1);  
		    loopBlock->blockMode = nReps;
			DIGITALBLOCKS_displayInTable (seq, loopBlock->blockNr, panelSequence,
										 SEQUENCE_TABLE_header2, SEQUENCE_TABLE_dac2, SEQUENCE_TABLE_digital2);

			RemovePopup (1);
			break;
	}
	return 0;
}



int CVICALLBACK ADDRSIM_panel_CB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			HidePanel (panel);
			break;
	}
	return 0;
}
