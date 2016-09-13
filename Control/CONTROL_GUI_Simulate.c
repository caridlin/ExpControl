#include <userint.h>
#include <utility.h>
/*************************************************************************

   GUI_Simulate.c
   
   
   Simulation der Ausgabe
   
   
   (c) Stefan Kuhr 2000
   
   Institut für Angewandte Physik
   Universität Bonn
   

   
*************************************************************************/

#include "INCLUDES_CONTROL.h"    
#include "UIR_ExperimentControl.h"





// Größen der Digital/Analog-Kanäle im Ausgabefenster
#define DIO_ROW_HEIGHT     1.0



/************************************************************************
 *
 *  BEGIN: DEBUG_ROUTINES
 *
 *  Routines used for debugging only
 *
 ************************************************************************/

// display waveform
void DEBUG_DisplayWaveform (t_waveform *wfm)
{
	int i;

	if (wfm != NULL) {
		DebugPrintf ("Waveform: %s, DIG_NValues=%d", wfm->name, wfm->DIG_NValues);
		if (wfm->DIG_NValues < 100) 
		    for (i = 0; i < wfm->DIG_NValues; i++) {
       	        DebugPrintf ("DIG_Values[%3d]=%5d, DIG_Repeat[%3d]=%5d", 
    				i, wfm->DIG_values[i], i, wfm->DIG_repeat[i]);
    	}
 	}
 	else {
 		DebugPrintf ("Waveform: NULL");
	}
}


// displays the calculated data 
void DEBUG_DisplayOutData (t_outputData *out, int ch)
{
	int i;
	
	DebugPrintf ("OutSequence ANALOG OUT:");
	DebugPrintf ("numAOValues[%d]=%d", ch, out->numAOValues[ch]);
	for (i=0; i<out->numAOValues[ch]; i++) {
	   	DebugPrintf ("AO_Values[%d][%3d]=%5d, AO_ValueDuration[%d][%3d]=%5d", 
	   				ch, i, out->AO_Values[ch][i], ch, i, out->AO_ValueDuration[ch][i]);
 	}
	
}


// displays the contents of the analog blochs
void DEBUG_DisplayAOBufferContents (short *outBuf, unsigned long NValues, int NChannels)
{
    int ch;
    unsigned long i;
    char h[10], line[200];
    short val;
    
    DebugPrintf ("AnalogOutputBuffer NValues=%d, NChannels=%d", NValues, NChannels);
    if (NValues > 1000) return;

    for (i = 0; i < NValues; i++) {
        sprintf (line, "%4d: ", i);
        for (ch = 0; ch < NChannels; ch++) {
            val =  outBuf[i*NChannels+ch];
            sprintf (h, "%5d  ", val);
            strcat (line, h);
        }
        DebugPrintf (line);
    }
}

/************************************************************************
 *
 *  END: DEBUG_ROUTINES
 *
 ************************************************************************/

            


/************************************************************************
 *
 *  BEGIN: Routines for graphical user interface "Simulate output"
 *
 ************************************************************************/



void SIMULATE_resizePanel (int panel)
{
     //int spaceUpDown = 10;
    
    int percentPanelDAC1 = 30;
    int percentPanelDIO  = 70;
    int height;

	if (panel == -1) return;
	height = panelHeight (panel);
    setCtrlBoundingRect (panel, SIMULATE_GRAPH_DAC1,
    					 0, 
    					 ctrlRight (panel, SIMULATE_DECORATION),
    					 height * percentPanelDAC1 / 100,
    					 panelWidth (panel)-ctrlRight (panel, SIMULATE_RING_DAC));
    setCtrlBoundingRect (panel, SIMULATE_GRAPH_DIO,
    					 ctrlBottom(panel, SIMULATE_GRAPH_DAC1), 
    					 ctrlRight (panel, SIMULATE_DECORATION),
    					 height * percentPanelDIO / 100,
    					 panelWidth (panel)-ctrlRight (panel, SIMULATE_RING_DAC));
    setCtrlBoundingRect (panel, SIMULATE_TEXTBOX_info,
    					 ctrlBottom(panel, SIMULATE_DECORATION), 
    					 0,
    					 panelHeight(panel)-ctrlBottom (panel, SIMULATE_DECORATION),
    					 ctrlLeft(panel, SIMULATE_GRAPH_DAC1));
}




void analogChannelNamesToRing (int panel, int ctrl)
{
	int i;
	
	ClearListCtrl (panel, ctrl);
	for (i = 0; i < N_DAC_CHANNELS;	i++) {
		InsertListItem (panel, ctrl, -1, str_AnalogChannelNames(i), i);
	}
}


void SIMULATE_loadPanel (t_sequence *seq)
{
    char title[200];
	
	if (seq->panelSimulate <= 0) {
        seq->panelSimulate = LoadPanel (0, UIR_File, SIMULATE);
		SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DIO,  
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
		SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DAC1,  
					  ATTR_PLOT_BGCOLOR, VAL_CYAN);
		analogChannelNamesToRing (seq->panelSimulate, SIMULATE_RING_DAC);
		sprintf (title, "Simulate output - %s", extractFilename (seq->filename));
		SetPanelAttribute (seq->panelSimulate, ATTR_TITLE, title);
	}
}

/************************************************************************
 * SUBROUTINE:  initSimulateOutputWindows 
 ************************************************************************/


void SIMULATE_setGraphAxes (t_sequence *seq, double min, double max)
{
	double swap;
	
	if (max < min) {
	    swap = max;
		max= min;
		min = swap;
	}
	
	SetAxisScalingMode (seq->panelSimulate, SIMULATE_GRAPH_DIO, VAL_XAXIS, VAL_MANUAL, min, max);
	SetAxisScalingMode (seq->panelSimulate, SIMULATE_GRAPH_DAC1, VAL_XAXIS, VAL_MANUAL, min, max);
	SetGraphCursor (seq->panelSimulate, SIMULATE_GRAPH_DAC1, 1, min, 0);
	SetGraphCursor (seq->panelSimulate, SIMULATE_GRAPH_DAC1, 2, max, 0);
	SetGraphCursor (seq->panelSimulate, SIMULATE_GRAPH_DIO, 1, min, 0);
	SetGraphCursor (seq->panelSimulate, SIMULATE_GRAPH_DIO, 2, max, 0);
}



void SIMULATE_initPanel (t_sequence *seq)
{
	
	int ch;
	double YPos;
	double YMin, YMax;
	
	
	SIMULATE_loadPanel (seq);
	
//	SIMULATE_initGraphs (seq, 0);
	
//	SetAxisScalingMode (panelSimulate, SIMULATE_GRAPH_DAC2, VAL_XAXIS, VAL_MANUAL, 0, maxtime);

	YMin = 0;
	YMax = DIO_ROW_HEIGHT * (seq->maxDigitalChannel);

	SetAxisScalingMode (seq->panelSimulate, SIMULATE_GRAPH_DIO,  VAL_LEFT_YAXIS, VAL_MANUAL, 
	                    YMin, YMax);
	
	ClearAxisItems (seq->panelSimulate, SIMULATE_GRAPH_DIO, VAL_LEFT_YAXIS);
	PlotLine (seq->panelSimulate, SIMULATE_GRAPH_DIO,  0, YMin, 0, YMax, VAL_GREEN);
//	PlotLine (panelSimulate, SIMULATE_GRAPH_DIO,  0, 0, maxtime, 0, VAL_BLACK);
//	PlotLine (panelSimulate, SIMULATE_GRAPH_DAC1, 0, 0, maxtime, 0, VAL_BLACK);
//	PlotLine (panelSimulate, SIMULATE_GRAPH_DAC2, 0, 0, maxtime, 0, VAL_BLACK);

// Namen für Digitalkanäle eintragen
    for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
        YPos = ((seq->maxDigitalChannel - ch) + 0.5) * DIO_ROW_HEIGHT - 1;
		InsertAxisItem (seq->panelSimulate, SIMULATE_GRAPH_DIO,
						VAL_LEFT_YAXIS, -1, str_ChannelNames(ch), YPos);
	}

	SetPanelAttribute (seq->panelSimulate, ATTR_CALLBACK_DATA, (void *) seq);
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
	
	enableCommaInNumeric (seq->panelSimulate, SIMULATE_NUMERIC_tFrom);    	
	enableCommaInNumeric (seq->panelSimulate, SIMULATE_NUMERIC_tTo);
	
    SIMULATE_resizePanel (seq->panelSimulate);			  
}


						
unsigned long addTimeOffset (unsigned long time)
{
    static unsigned long sumT = ULONG_MAX;
    unsigned long oldT;
    
    oldT = sumT;
    if (time == 0) sumT = 0;
    sumT += time;
    return oldT;
}





void SIMULATE_ADWIN_plotDACbufferContents (t_sequence *seq, int channel)
{
    int i;
    double deltaT;
   double fval, XMin, XMax;
    float *pointListX, *pointListY;
    unsigned long pointCount;
    t_digitizeParameters *p;
	int ch, device;
	t_outputData *out;
	t_adwinData *a;
	unsigned long nSamples;
	int nPoints;
	int firstPoint;

	device = config->analogBoardID[channel / 8];
	ch = channel % 8 + 1;
	p = digitizeParameters_ADWIN();
	
	out = seq->outData;
	if (out == NULL) return;
	a = out->adwinData;
	
    deltaT = 1.0 * out->DIOdeltaT / VAL_ms;
 	nPoints = a->nAOelements;
    
	pointListX = (float *) malloc (nPoints * sizeof (float));
    pointListY = (float *) malloc (nPoints * sizeof (float));
	if (pointListX == NULL) {
		PostMessagePopupf ("Error", "Function 'SIMULATE_ADWIN_plotDACbufferContents': %d kBytes could not be allocated!", sizeof (float) * nPoints / 1024);
		return;	
	}
	if (pointListY == NULL) {	
		PostMessagePopupf ("Error", "Function 'SIMULATE_ADWIN_plotDACbufferContents': %d kBytes could not be allocated!", sizeof (float) * nPoints / 1024);
		return;	
	}

    XMin  = 0;
	fval = 0;
	pointCount = 0;
	nSamples = (unsigned long) out->AO_totalSamples / out->nCopies;
	firstPoint = 1;
    for (i = 0; i < a->nAOelements; i+= 2) {
		if ((ch == a->AObuffer[i+1] % 16) && (device == ((a->AObuffer[i+1] & 0xFF) >> 4))) {
			XMax = (a->AObuffer[i])*deltaT;
	        if (!firstPoint) {
	            pointListX[pointCount]   = XMin;
	            pointListX[pointCount+1] = XMax;
	            pointListY[pointCount]   = fval;
	            pointListY[pointCount+1] = fval;
	            pointCount = pointCount + 2;
	            XMin = XMax;
			}
			fval = DigitalToAnalog (a->AObuffer[i+1] >> 8, p);
			firstPoint = 0;
        }
    }
	XMax = a->maxCycleNo*deltaT;
    pointListX[pointCount]   = XMin;
    pointListX[pointCount+1] = XMax;
    pointListY[pointCount]   = fval;
    pointListY[pointCount+1] = fval;
    pointCount += 2;

	PlotXY (seq->panelSimulate, SIMULATE_GRAPH_DAC1, pointListX,
			pointListY, pointCount, VAL_FLOAT, VAL_FLOAT, VAL_FAT_LINE,
			VAL_EMPTY_SQUARE, VAL_SOLID, 1, colors[ch % NColors]);
    free (pointListX);
    free (pointListY);
}




void SIMULATE_plotDACbufferContents (t_sequence *seq, int channel)
{
    int i;
    double deltaT;
    int val = 0;
	int oldVal;
    double fval, XMin, XMax, timeOffset;
    float *pointListX, *pointListY;
    unsigned long pointCount;
    t_digitizeParameters *p;
	int ch, device;
	t_outputData *out;
	unsigned long nSamples;
	int maxPoints = 2000000;
	unsigned long nPoints;
  
	
	DeleteGraphPlot (seq->panelSimulate, SIMULATE_GRAPH_DAC1, -1,
					 VAL_IMMEDIATE_DRAW);

	if (config->hardwareType == HARDWARE_TYPE_ADWIN) {
		SIMULATE_ADWIN_plotDACbufferContents (seq, channel);
		return;
	}

	
	device = channel / 8;
	ch = channel % 8;
	if (device > MAX_AO_DEVICES) return;

	p = digitizeParameters_PCI67XX(1);
	
	out = seq->outData;
	

    deltaT = 1.0 * out->AOdeltaT / VAL_ms;
    timeOffset = 0;
//    timeOffset = addTimeOffset (NValues * deltaT);
    
	
    if (out->AO_totalSamples == 0) return;
	if (seq->simulate_AO_buffer[device] == NULL) return;
	
	nPoints = min (maxPoints, out->AO_totalSamples * 2);
	
    pointListX = (float *) malloc (nPoints * sizeof (float));
    pointListY = (float *) malloc (nPoints * sizeof (float));
	if (pointListX == NULL) {
		PostMessagePopupf ("Error", "Function 'SIMULATE_plotDACbufferContents': %d kBytes could not be allocated!", sizeof (float) * nPoints / 1024);
		return;	
	}
	if (pointListY == NULL) {	
		PostMessagePopupf ("Error", "Function 'SIMULATE_plotDACbufferContents': %d kBytes could not be allocated!", sizeof (float) * nPoints / 1024);
		return;	
	}

    oldVal  = seq->simulate_AO_buffer[device][ch];
    XMin    = 0;
	pointCount = 0;
	nSamples = (unsigned long) out->AO_totalSamples / out->nCopies;
    for (i = 0; ((i < nSamples) && (pointCount < maxPoints)) ; i++) {
        val = seq->simulate_AO_buffer[device][i*8+ch];//
        if ((val != oldVal) && (i > 0)) {
            XMax = (i*deltaT);
            fval = (double) p->maxVoltage * oldVal / (double) p->maxDigital;
//                DebugPrintf ("i=%4d, oldVal=%5d, fval=%f", i, oldVal, fval);
            pointListX[pointCount]   = XMin + timeOffset;
            pointListX[pointCount+1] = XMax + timeOffset;
            pointListY[pointCount]   = fval;
            pointListY[pointCount+1] = fval;
            pointCount = pointCount + 2;
            XMin = i * deltaT;
            oldVal = val;
        }
    }
    XMax = (i*deltaT);
    fval = (double) p->maxVoltage * val / (double) p->maxDigital;
    pointListX[pointCount]   = XMin + timeOffset;
    pointListX[pointCount+1] = XMax + timeOffset;
    pointListY[pointCount]   = fval;
    pointListY[pointCount+1] = fval;
    pointCount = pointCount + 2;

//	DeleteGraphPlot (seq->panelSimulate, SIMULATE_GRAPH_DAC1, -1, VAL_DELAYED_DRAW);
	PlotXY (seq->panelSimulate, SIMULATE_GRAPH_DAC1, pointListX,
			pointListY, pointCount, VAL_FLOAT, VAL_FLOAT, VAL_FAT_LINE,
			VAL_EMPTY_SQUARE, VAL_SOLID, 1, colors[ch % NColors]);
    free (pointListX);
    free (pointListY);
}






int SIMULATE_ADWIN_plotDIOBufferContents (t_sequence *seq, int progressDialog, int percStart)
{
//    static int oldPlotLine = -255;
    
    int i;
    int ch = 0;
    unsigned long val=0, oldVal;
    double YMin, YMax, XMin, XMax;
    double deltaT;
	int board;
	unsigned long andVal;
	int step;
	t_outputData *o;
	t_adwinData *a;
	
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DIO, ATTR_REFRESH_GRAPH, 0);
	DeleteGraphPlot (seq->panelSimulate, SIMULATE_GRAPH_DIO, -1,
					 VAL_DELAYED_DRAW);
	
	o = seq->outData;
	a = o->adwinData;
	
    deltaT = 1.0 * o->DIOdeltaT / VAL_ms;
	
	
    for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
        XMin = 0;
        YMin = (seq->maxDigitalChannel - ch - 1) * DIO_ROW_HEIGHT;
        YMax = (seq->maxDigitalChannel - ch    ) * DIO_ROW_HEIGHT;
        oldVal = 0;
		board = ch / N_BOARD_DIO_CHANNELS;
		andVal = 1 << (ch%N_BOARD_DIO_CHANNELS);    
		step = config->nDigitalBoards+1;
		for (i = 0; i < a->nDIOelements; i+=step) {
			val = a->DIObuffer[i+board+1] & andVal;
			if (i+step >= a->nDIOelements) XMax = a->maxCycleNo*deltaT;
			else XMax = (a->DIObuffer[i+step]-1) *deltaT;
			if (val > 0) {
//				DEBUGOUT_printf ("ch %s: Rect: [%8.3f, %8.3f]\n", str_ChannelNames (ch), XMin, XMax);
				PlotRectangle (seq->panelSimulate, SIMULATE_GRAPH_DIO,
							   XMin, YMin,
							   XMax, YMax,
							   colors[ch % NColors], colors[ch % NColors]);
            }
            XMin = XMax;
		}
		if (UpdateProgressDialog (progressDialog, percStart + (100 - percStart) * ch/seq->maxDigitalChannel, 1)) {
			return -1;	
		}
		if (ch % 4 == 0) {
			RefreshGraph (seq->panelSimulate,  SIMULATE_GRAPH_DIO);
			if (panelHidden(seq->panelSimulate)) DisplayPanel (seq->panelSimulate);
		}
    }
	RefreshGraph (seq->panelSimulate,  SIMULATE_GRAPH_DIO);
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DIO, ATTR_REFRESH_GRAPH, 1);
	return 0;
}







int SIMULATE_plotDIOBufferContents (t_sequence *seq, int progressDialog, int percStart)
{
//    static int oldPlotLine = -255;
    
    int i;
    int ch = 0;
    unsigned long val=0, oldVal;
    double YMin, YMax, XMin, XMax;
    double deltaT;
	int board;
	unsigned long andVal;
	unsigned long *array;
	t_outputData *o;
	unsigned long nSamples;
    
	
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DIO, ATTR_REFRESH_GRAPH, 0);
	DeleteGraphPlot (seq->panelSimulate, SIMULATE_GRAPH_DIO, -1,
					 VAL_DELAYED_DRAW);

	if (config->hardwareType == HARDWARE_TYPE_ADWIN) {
		return SIMULATE_ADWIN_plotDIOBufferContents (seq, progressDialog, percStart);
	}
	
	
	o = seq->outData;
    deltaT = 1.0 * o->DIOdeltaT / VAL_ms;
	nSamples = (unsigned long)o->DIO_totalSamples / o->nCopies;
    for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
        XMin = 0;
        YMin = (seq->maxDigitalChannel - ch - 1) * DIO_ROW_HEIGHT;
        YMax = (seq->maxDigitalChannel - ch    ) * DIO_ROW_HEIGHT;
        oldVal = 0;
		board = ch / N_BOARD_DIO_CHANNELS;
		array = seq->simulate_DIO_buffer[board];
		andVal = 1 << (ch%N_BOARD_DIO_CHANNELS);    
		if (array != NULL) {
			for (i = 0; i < nSamples; i++) {
				val = array[i] & andVal;
	            if ((oldVal == 0) && (val > 0)) XMin = deltaT * i;
	            else 
	            if ((val == 0) && (oldVal > 0)) {
	                XMax = (i*deltaT);
					PlotRectangle (seq->panelSimulate, SIMULATE_GRAPH_DIO,
								   XMin, YMin,
								   XMax, YMax,
								   colors[ch % NColors], colors[ch % NColors]);
	            }
	            oldVal = val;
	        }
	        if (oldVal > 0) {
	            XMax = (i*deltaT);
				PlotRectangle (seq->panelSimulate, SIMULATE_GRAPH_DIO,
							   XMin, YMin,
							   XMax, YMax,
							   colors[ch % NColors], colors[ch % NColors]);
	        }
		}
		if (UpdateProgressDialog (progressDialog, percStart + (100 - percStart) * ch/seq->maxDigitalChannel, 1)) {
			return -1;	
		}
		if (ch % 4 == 0) {
			RefreshGraph (seq->panelSimulate,  SIMULATE_GRAPH_DIO);
			if (panelHidden(seq->panelSimulate)) DisplayPanel (seq->panelSimulate);
		}
    }
	RefreshGraph (seq->panelSimulate,  SIMULATE_GRAPH_DIO);
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_GRAPH_DIO, ATTR_REFRESH_GRAPH, 1);
	return 0;
}



void SIMULATE_displayMemoryInfo (t_sequence *seq, int panel, int ctrl)
{
	t_outputData *o;
	DeleteTextBoxLines (panel, ctrl, 0, -1);
	
	o = seq->outData;
	if (o == NULL) return;
	
	//digital IO:
	pprintf(panel, ctrl, 
			"%d Digital boards(32bit):\n"
			"%5.2f kSamples\n"
			"%5.2f MB buffer/board\n\n",
			nDIOBoards(),			
			ui64ToDouble (o->DIO_totalSamples) / 1024.0,
			ui64ToDouble (o->DIO_totalSamples) / (1024.0*1024.0) * 4);
			
	pprintf(panel, ctrl, 
			"%d Analog boards (12/16bit):\n"
			"%5.2f kSamples\n"
			"%5.2f MB buffer/board\n\n",
			nAOBoards(),			
			ui64ToDouble (o->AO_totalSamples) / 1024.0,
			ui64ToDouble (o->AO_totalSamples) / (1024.0*1024.0) *2*8);
}








int SIMULATE_calculateOutput (t_sequence *seq, int firstRepetition, int repetitionNr, int *panelProgr)
{
    t_outputData *o;
	
	
    //int DACNr;
    //int nBoards;
	int device;
	int errorCode = 0;
    
// Ausgabewerte berechnen 
	if (SEQUENCE_getStatus () != SEQ_STATUS_FINISHED) return -1;
	SetWaitCursor (1);
	
//	if (config->nAnalogBoards == 0) config->nAnalogBoards = 2;
	*panelProgr = CreateProgressDialog ("Simulate Output",
										  "", 1,
										  VAL_NO_INNER_MARKERS, "");
	
	seq->repetition = repetitionNr-1;
	if (OUTPUTDATA_calculate (seq, firstRepetition, 1) < 0) {
		errorCode = -1;
		goto ENDE;
	}
   	o = seq->outData;
	
// Ausgabebuffer initialisieren
	switch (config->hardwareType) {
		case HARDWARE_TYPE_NATINST:
			SEQUENCE_freeSimulateBuffers (seq);
			seq->simulate_DIO_bufferSizeSamples = o->DIO_totalSamples / o->nCopies;
			seq->simulate_AO_bufferSizeSamples = o->AO_totalSamples / o->nCopies;
			for (device = 0; device < MAX_DIO_DEVICES; device++) {
				seq->simulate_DIO_buffer[device] = (unsigned int *) malloc (N_OUTPUT_PORTS * seq->simulate_DIO_bufferSizeSamples);
			}
			for (device = 0; device < MAX_AO_DEVICES; device++) {
				seq->simulate_AO_buffer[device] = (short *) malloc (2*8* seq->simulate_AO_bufferSizeSamples);
			}
	
		// Zeit initialisieren  
		    addTimeOffset (0);

			if (o->duration == 0) {
		    	if (seq->panelSimulate >= 0) {
					DiscardPanel (seq->panelSimulate);
					seq->panelSimulate = 0;
				}
			    MessagePopup ("Error", "Total time = 0. No output could be generated.");

				//    	OUTPUTDATA_freeBuffers (o);
				goto ABORT;
		    }
 
			// DEBUG TimeHiLo
		//DEBUG

		/**************************************
		    digital out
		 ***************************************/
    
			OUTPUTDATA_resetCounters (o);
	
			o->simulate = 1;
	
			// prepare first full buffer
			OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (o, seq->simulate_DIO_buffer, (unsigned long)seq->simulate_DIO_bufferSizeSamples);
		//    OUTPUTDATA_writeDIOBytesToOutputBuffer_OLD (seq);
			if (UpdateProgressDialog (*panelProgr, 20, 1)) goto ABORT;

			OUTPUTDATA_writeAOBytesToOutputBuffer_DBLBUF (o, seq->simulate_AO_buffer,(unsigned long) seq->simulate_AO_bufferSizeSamples);

			SIMULATE_displayMemoryInfo (seq, seq->panelSimulate, SIMULATE_TEXTBOX_info);
			break;
		case HARDWARE_TYPE_ADWIN:
			break;
	}			
									 
	



ENDE:

//   SEQUENCE_freeSimulateBuffers (seq);
	
   DiscardProgressDialog (*panelProgr);
   *panelProgr = 0;
   SetWaitCursor (0);
   
   return errorCode;	 
   
	
ABORT:
	errorCode = -1;
	goto ENDE;	
	
   
}

/************************************************************************
 *
 * SUBROUTINE:  simulateOutput
 *
 * BESCHR:      Simulation der Ausgabewerte
 *
 ************************************************************************/

void SIMULATE_zoomGraph (int panel, double timeStart, double timeStop);



void SIMULATE_setMinMax (t_sequence *seq, int update)
{
	double maxT;
	
	maxT = 1.0*ui64ToDouble (seq->outData->duration) / VAL_ms;
	
	SIMULATE_setGraphAxes (seq, 0, maxT);
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_tFrom, ATTR_MAX_VALUE, maxT);
	SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_tTo, ATTR_MAX_VALUE, maxT);
	if (!update) {
		SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_tFrom, ATTR_CTRL_VAL, 0.0);
		SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_tTo, ATTR_CTRL_VAL, maxT);
	}

	
}


void SIMULATE_start (t_sequence *seq, int update)
{
    int dacNr;

	int panelProgress;
	
	if (SEQUENCE_getStatus () != SEQ_STATUS_FINISHED) return;

	SIMULATE_initPanel (seq);
 
	if (SIMULATE_calculateOutput (seq, 1, 1, &panelProgress) < 0) {
		HidePanel (seq->panelSimulate);
		return;
	}
    DisplayPanel (seq->panelSimulate);
	
	if (SIMULATE_plotDIOBufferContents (seq, panelProgress, 75) != 0) goto ENDE;
	SIMULATE_setMinMax (seq, update);
	GetCtrlVal (seq->panelSimulate, SIMULATE_RING_DAC, &dacNr);
	SIMULATE_plotDACbufferContents (seq, dacNr);

   
ENDE:	
	DiscardProgressDialog (panelProgress);
   return;
	
	
}



int CVICALLBACK SIMULATE_panelCallback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	t_sequence *seq;
	
	switch (event)
		{
		case EVENT_GOT_FOCUS:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			SetCtrlAttribute (seq->panelSimulate, SIMULATE_NUMERIC_repetition, ATTR_MAX_VALUE, seq->nRepetitions);
//			SIMUALTE_changeRepetition_CB (panel, 0, EVENT_COMMIT, 0, 0, 0);
			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			SEQUENCE_freeSimulateBuffers (seq);
			DiscardPanel (seq->panelSimulate);
			seq->panelSimulate = 0;
			break;
		case EVENT_PANEL_SIZE:            
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
		    SIMULATE_resizePanel (seq->panelSimulate);			  
			break;
		}
	return 0;
}




int CVICALLBACK BTN_SimulateOutput_Update (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	t_sequence *seq;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			SIMULATE_start (seq, 1);
			break;
		}
	return 0;
}




int CVICALLBACK NUM_DAC_Changed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int dacNr; 
	t_sequence *seq;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			GetCtrlVal (seq->panelSimulate, SIMULATE_RING_DAC, &dacNr);
			SIMULATE_plotDACbufferContents (seq, dacNr);
			break;
		}
	return 0;
}

	
	

int CVICALLBACK SIMULATE_axisChanged_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double tFrom, tTo;
	t_sequence *seq;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, SIMULATE_NUMERIC_tFrom, &tFrom);
			GetCtrlVal (panel, SIMULATE_NUMERIC_tTo, &tTo);
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			SIMULATE_setGraphAxes (seq, tFrom, tTo);
//			RefreshGraph (seq->panelSimulate, SIMULATE_GR);
			break;
	}
	return 0;
}



int CVICALLBACK SIMULATE_showAll_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double maxT;
	t_sequence *seq;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			maxT = SEQUENCE_duration_ms (seq);
			SetCtrlAttribute (panel, SIMULATE_NUMERIC_tTo, ATTR_MAX_VALUE, maxT);
			SetCtrlAttribute (panel, SIMULATE_NUMERIC_tTo, ATTR_CTRL_VAL, maxT);
			SetCtrlAttribute (panel, SIMULATE_NUMERIC_tFrom, ATTR_CTRL_VAL, 0.0);
			SIMULATE_setGraphAxes (seq, 0, maxT);
			break;
	}
	return 0;
}




int CVICALLBACK SIMUALTE_changeRepetition_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int rep;
	int dacNr;
	t_sequence *seq;
	int panelProgress;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			GetCtrlVal (seq->panelSimulate, SIMULATE_NUMERIC_repetition, &rep);
			SIMULATE_setMinMax (seq, control != SIMULATE_NUMERIC_repetition);
			
			if (SIMULATE_calculateOutput (seq, 0, rep, &panelProgress) < 0) {
				DiscardProgressDialog (panelProgress);
				return 0;
			}
			GetCtrlVal (seq->panelSimulate, SIMULATE_RING_DAC, &dacNr);
			SIMULATE_plotDACbufferContents (seq, dacNr);
			if (SIMULATE_plotDIOBufferContents (seq, panelProgress, 75) != 0) return 0;
			
			break;
	}
	return 0;
}


int CVICALLBACK SIMUALTE_update_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int rep;
	int dacNr;
	t_sequence *seq;
	int panelProgress;

	switch (event)
	{
		case EVENT_COMMIT:
			GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &seq);
			GetCtrlVal (seq->panelSimulate, SIMULATE_NUMERIC_repetition, &rep);
			
			if (SIMULATE_calculateOutput (seq, 1, rep, &panelProgress) < 0) {
				DiscardProgressDialog (panelProgress);
				return 0;
			}
			SIMULATE_setMinMax (seq, control != SIMULATE_NUMERIC_repetition);
			if (SIMULATE_plotDIOBufferContents (seq, panelProgress, 75) != 0) return 0;
			
			GetCtrlVal (seq->panelSimulate, SIMULATE_RING_DAC, &dacNr);
			SIMULATE_plotDACbufferContents (seq, dacNr);
			break;
	}
	return 0;
}
