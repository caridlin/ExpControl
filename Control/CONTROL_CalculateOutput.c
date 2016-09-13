
#include "CONTROL_DataStructure.h"
#include "CONTROL_CalculateOutput.h"
#include "CONTROL_HARDWARE.h"	
#include "CONTROL_Definitions.h"
#include "CONTROL_GUI_EditPoints.h"					
#include "CONTROL_GUI_Main.h"		
#include "CONTROL_GUI_Waveform.h"
#include "CONTROL_HARDWARE_GPIB.h"	
#include "GPIB_DRIVER_LeCroy.h"
#include "imageProcessing.h"
#include <utility.h>          

#define DIO_Protocol 0

#define DO_TEST_OF_NEW_ROUTINES 1	 




int OUTPUTDATA_insertVariablePulsesIntoEventList (t_sequence *seq);
int OUTPUTDATA_convertEventListIntoDIOValues (t_outputData *o);
unsigned long OUTPUTDATA_generateAnalogSamplesFromValues (t_outputData *o, int device);

int OUTPUTDATA_ADWIN_createData (t_sequence *seq, t_outputData *o);





	

void DEBUG_displaySample (t_analogSample *a)
{
	int i;
	
	for (i = 0; i < 8; i++) {
		DEBUGOUT_printf ("%6d", a->value[i]);	
	}
}

void DEBUG_displaySamples (t_outputData *o, int device)
{
	int i;
	unsigned int pos;
	
	DEBUGOUT_printf ("\nAnalog output for device %d (%d different samples):\n", device, o->AO_nSamples[device]);
	pos = 0;
	for (i = 0; i < o->AO_nSamples[device]; i++) {
		DEBUGOUT_printf ("%3d (x%4d @%5d): ", i, o->AO_SampleRepeat[device][i], pos);
		pos += o->AO_SampleRepeat[device][i];
		DEBUG_displaySample (&o->AO_Samples[device][i]);
		DEBUGOUT_printf ("\n");
	}
}


void DEBUG_displayOutBuf (t_analogSample *buf, int nSamples)
{
	int i;
	
	DEBUGOUT_printf ("\nAnalog output for (%d different samples):\n", nSamples);
	for (i = 0; i < nSamples; i++) {
		DEBUGOUT_printf ("%3d (x%5d-%5d): ", i, 1, 1);
		DEBUG_displaySample (&buf[i]);
		DEBUGOUT_printf ("\n");
	}
}










//=======================================================================
//
// 	  interpolate function values
//
//=======================================================================
#define linearInterpolate(u1, u2, dT, t) ((u2-u1) * (double) (t) /(double) (dT) + u1)







//=======================================================================
//
// 	  calculate the length of a waveform 
//    (may vary as a function of 'repetition')
//
//=======================================================================
void WFM_calculateOffsetAndDuration (t_waveform *wfm, int repetition, int nRepetitions, int allowNegativeTimes)
{
    t_point *last, *first;
	double from, to;
	int i;
	int nParams;
	__int64 tmp;
    
    if (wfm == NULL) return;
    
    switch (wfm->type) {
    	case WFM_TYPE_POINTS: 
	    	if (wfm->nPoints == 0) {
	    		wfm->durationThisRepetition_ns = 0;
	    		wfm->durationFrom_50ns = 0;	    		
				wfm->durationTo_50ns = 0;
	    	} else {
		    	// calculate current values
		    	POINTS_calculateValues (wfm->points, wfm->nPoints, repetition, allowNegativeTimes);
		    	first = &wfm->points[0];
		    	last = &wfm->points[wfm->nPoints-1];
		    	// duration is the time of the last point
		    	if (allowNegativeTimes) {
		    		if (wfm->nPoints == 1) wfm->durationThisRepetition_ns = last->thisTime_ns;
		    		else wfm->durationThisRepetition_ns = last->thisTime_ns - first->thisTime_ns;
		    	}
		    	else wfm->durationThisRepetition_ns = last->thisTime_ns;
		    	// calculate duration in units of 50ns
				wfm->durationVary = 0;
		    }
		   	break;
		case WFM_TYPE_FILE:
			from = ui64ToDouble (wfm->durationFrom_50ns * VAL_MIN_ns);
			if (wfm->durationVary) to = ui64ToDouble (wfm->durationTo_50ns * VAL_MIN_ns);
			else to = from;
			if (nRepetitions <= 1) wfm->durationThisRepetition_ns = from;
			else wfm->durationThisRepetition_ns = from + (((double)repetition) * (to - from)) / (double) (nRepetitions-1);
			if (wfm->durationThisRepetition_ns < 0) wfm->durationThisRepetition_ns = 0;
			
			wfm->offsetThisRepetition = wfm->offsetFrom;
			if ((wfm->offsetVary) && (nRepetitions > 1))
				wfm->offsetThisRepetition = wfm->offsetFrom + (((double)repetition) * (wfm->offsetTo - wfm->offsetFrom)) / (double) (nRepetitions-1);

			wfm->uScaleThisRepetition = wfm->uScaleFrom;
			if ((wfm->uScaleVary) && (nRepetitions > 1))
				wfm->uScaleThisRepetition = wfm->uScaleFrom + (((double)repetition) * (wfm->uScaleTo - wfm->uScaleFrom)) / (double) (nRepetitions-1);

			break;
		case WFM_TYPE_FUNCTION:
			tmp = wfm->durationFrom_50ns;
			tmp *= VAL_MIN_ns;
			from = ui64ToDouble (tmp);
			if (wfm->durationVary) to = ui64ToDouble (wfm->durationTo_50ns * VAL_MIN_ns);
			else to = from;
			if (nRepetitions <= 1) wfm->durationThisRepetition_ns = from;
			else wfm->durationThisRepetition_ns = from + (((double)repetition) * (to - from)) / (double) (nRepetitions-1);
			if (wfm->durationThisRepetition_ns < 0) wfm->durationThisRepetition_ns = 0;
			
			if (wfm->functionParametersThisRepetition == NULL) { 
				wfm->functionParametersThisRepetition = (double *) calloc (FUNCTION_maxParameters (), sizeof (double));
			}
			nParams = FUNCTION_nParameters (wfm->functionID);
			for (i = 0; i < nParams; i++) {
				from = wfm->functionParameters[wfm->functionID-1][i].from;
				to   = wfm->functionParameters[wfm->functionID-1][i].to; 
				
				switch (wfm->functionParameters[wfm->functionID-1][i].stepType) {
					case WFM_STEP_TYPE_TO_LINEAR:
						if (nRepetitions <= 1) {
							wfm->functionParametersThisRepetition[i] = from;
						} else {
							wfm->functionParametersThisRepetition[i] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);
						}
						break;
					case WFM_STEP_TYPE_LIST:
						if (wfm->functionParameters[wfm->functionID-1][i].list == NULL) {
							wfm->functionParametersThisRepetition[i] = from; // the default value for no sweep	
						} else if (repetition < wfm->functionParameters[wfm->functionID-1][i].listSize) {
							wfm->functionParametersThisRepetition[i] = wfm->functionParameters[wfm->functionID-1][i].list[repetition];	
						} else {
							wfm->functionParametersThisRepetition[i] = wfm->functionParameters[wfm->functionID-1][i].list[wfm->functionParameters[wfm->functionID-1][i].listSize-1];
						}
						break;
					default:
						wfm->functionParametersThisRepetition[i] = from;
						break;
					
				}
				//if (wfm->functionParametersVary[wfm->functionID-1][i] && (nRepetitions > 1)) {
				//	wfm->functionParametersThisRepetition[i] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);   
				//}
			}
			FUNCTION_setSpecialParameters(wfm->functionParametersThisRepetition, wfm->functionID, repetition);

			break;
		case WFM_TYPE_ADDRESSING:
			
			break;
			
		default:
//			wfm->durationThisRepetition_ns = 50; 
			wfm->offsetThisRepetition = 0;
			wfm->uScaleThisRepetition = 0;
    }
    wfm->durationThisRepetition_50ns = RoundRealToNearestInteger (wfm->durationThisRepetition_ns / VAL_MIN_ns);
}






//=======================================================================
//
// 	  return the current voltage for "step output" 
//
//=======================================================================
double WFM_getStepOutputVoltage (t_sequence *seq, t_waveform *wfm) 
{
	if (wfm->stepAlternate) {
		if ( (seq->repetition / wfm->stepRep) % 2 == 0) return wfm->stepFrom;
		else return wfm->stepTo;
	}
	
	return getStepOutputVoltage (wfm->stepFrom, wfm->stepTo, wfm->stepRep,
								 seq->nRepetitions, seq->repetition);
		
}
   


int WFM_estimateDigitizeStepsize (t_waveform *wfm, int nDigitalPoints, t_digitizeParameters *p)
{
	double min, max;
	int i;
	int digDiff;
	int stepsize;
	
	max = -1E20;
	min = +1E20;
	for (i = 0; i < wfm->nPoints; i++) {
		if (wfm->points[i].thisValue > max) max = wfm->points[i].thisValue;
		if (wfm->points[i].thisValue < min) min = wfm->points[i].thisValue;
	}
	digDiff = AnalogToDigital (max, p) - AnalogToDigital (min, p);
	
	stepsize = nDigitalPoints / digDiff / wfm->nPoints / 3;
	if (stepsize <= 0) stepsize = 1;
	return stepsize;
}




int WFM_calculateNPoints (t_waveform *wfm, unsigned long nDigitalPoints, 
					   unsigned long maxNValues, t_transferFunction *f,
					   double addVoltage, t_digitizeParameters *p, int fastMode) 
{
    unsigned long count;
    long digitalVal, oldVal;
    double analogVal;
    unsigned long n;
	double tLast, tNext;
	double dT_ns;
	int i;
	int stepsize, nextStep;
	
    count = 0;
	oldVal = SHRT_MAX;
	
	// 
	if (POINTS_hasNegativeTime (wfm->points, wfm->nPoints)) 
	 	return 0;
	if (fastMode) stepsize = WFM_estimateDigitizeStepsize (wfm, nDigitalPoints, p);
	else stepsize = 1*wfm->timebaseStretch; 
//	DebugPrintf ("Digitizing wfm: %s. %d points (stepsize=%d). ", wfm->name, nDigitalPoints, stepsize );

	if (wfm->nPoints > 1) {
	    i  = 1;
	    tLast = wfm->points[i-1].thisTime_ns;
	    tNext = wfm->points[i].thisTime_ns;
		n = 0;
		while (n < nDigitalPoints) {
			if (n + stepsize >= nDigitalPoints) nextStep = max (nDigitalPoints - n - 1, 1);
				else nextStep = stepsize;
			
        	dT_ns = min((double) (n+0.5*wfm->timebaseStretch) / (double) nDigitalPoints * wfm->durationThisRepetition_ns,wfm->points[wfm->nPoints-1].thisTime_ns);
//			t = (unsigned long) dT;
			// -----------------------------------------------------------
		    // 	    get value "i" from point list
			// -----------------------------------------------------------
			while ((dT_ns > tNext) && (i < wfm->nPoints)) {
			     i++;
				 tLast = tNext;
			     if (i < wfm->nPoints){
					 tNext = wfm->points[i].thisTime_ns;
				 }
			}
			// -----------------------------------------------------------
			// 	  calculate analog value 
			// -----------------------------------------------------------
//            interpolateT = dT - (double) tLast;
            analogVal  = linearInterpolate ( wfm->points[i-1].thisValue, wfm->points[i].thisValue, 
            								tNext - tLast, dT_ns - tLast) 
           			   + addVoltage;
			// -----------------------------------------------------------
		    // 	  apply transfer function
			// -----------------------------------------------------------
           	if (f != NULL) analogVal = TRANSFERFUNCT_apply (f, analogVal);
			// -----------------------------------------------------------
		    // 	  calculate digital value
			// -----------------------------------------------------------
			digitalVal = AnalogToDigital (analogVal, p);
			if (digitalVal == oldVal) {
				// -----------------------------------------------------------
			    // 	  same as previous value --> increase repetitions
				// -----------------------------------------------------------
			   	wfm->DIG_repeat[count-1] += nextStep;
			}
			else { 
				// -----------------------------------------------------------
			    // 	  different from previous value
				// -----------------------------------------------------------
				wfm->DIG_values[count] = digitalVal;
				oldVal = digitalVal;
				wfm->DIG_repeat[count] = nextStep;
				count ++;
				// -----------------------------------------------------------
			    // 	  reallocate new memory if necessary
				// -----------------------------------------------------------
				if (count >= maxNValues) {
					maxNValues *= 2;
					wfm->DIG_values = (long  *) realloc (wfm->DIG_values, sizeof(long)*maxNValues);
					wfm->DIG_repeat = (unsigned long *) realloc (wfm->DIG_repeat, sizeof(unsigned long)*maxNValues);
				}
			}
			n += nextStep;
	    }
	}
	// -----------------------------------------------------------
    // 	  add last point with 0 repetitions
	// -----------------------------------------------------------
	if (wfm->nPoints > 0) {
		analogVal = wfm->points[wfm->nPoints-1].thisValue + addVoltage;
		if (f != NULL) analogVal = TRANSFERFUNCT_apply (f, analogVal);
	    wfm->DIG_values[count] = AnalogToDigital (analogVal, p);

	    wfm->DIG_repeat[count] = 0;
	}
	// -----------------------------------------------------------
    // 	  return number of digitized values
	// -----------------------------------------------------------
    wfm->DIG_NValues = count;
//	DebugPrintf ("  %d values.\n", count);

    return count+1;
	
}


void WFM_displayDigitizedValues (t_waveform *wfm)
{
	int i;
	
	for (i = 0; i < wfm->DIG_NValues; i++) {
		DebugPrintf ("%6d:   val:%6d,     repeat: %6d\n", i, wfm->DIG_values[i], wfm->DIG_repeat[i]);
	}
}



unsigned long WFM_digitizePoints (t_sequence *seq, t_waveform *wfm, 
								 unsigned long maxNValues,
								 t_digitizeParameters *p)
{
    double addVoltage;
    unsigned long nDigitalPoints;
	t_transferFunction *f;
	int count;
	
	if (wfm == NULL) return 0;
	
	if (wfm->positionInList == 77) {
//		Breakpoint();	
	}
	
	// -----------------------------------------------------------
    // 	    init counters
	// -----------------------------------------------------------
	addVoltage = 0;
	// -----------------------------------------------------------
    // 	    add waveform: determine voltage to add
	// -----------------------------------------------------------
   	if (wfm->add != WFM_ID_UNCHANGED) {
    	addVoltage = WFM_getStepOutputVoltage(seq, WFM_ptr(seq, wfm->add));
    }
	// -----------------------------------------------------------
    // 	    calculate number of digital points
	// -----------------------------------------------------------
	if (p->nPoints == 0) {
		// calculate number of points from waveform duration
		nDigitalPoints  = (unsigned long) wfm->durationThisRepetition_50ns / p->timebase_50ns;
	}
	else {
		// number of points is given
		nDigitalPoints  = p->nPoints;
	}
	if (nDigitalPoints == 0) nDigitalPoints = 1;

	f = TRANSFERFUNCT_ptr (seq, wfm->transferFunction);
    POINTS_calculateValues (wfm->points, wfm->nPoints, seq->repetition, 0);
	
//	count = WFM_calculateNPoints (wfm, nDigitalPoints, maxNValues, f, addVoltage, p, 1);
//	WFM_displayDigitizedValues (wfm);
//	DebugPrintf ("-------------------------\n");
	count = WFM_calculateNPoints (wfm, nDigitalPoints, maxNValues, f, addVoltage, p, 0);
//	WFM_displayDigitizedValues (wfm);
	
	
    
	return count;
}





//=======================================================================
//
// 	  digitize waveform type "from file"
//
//=======================================================================
unsigned long WFM_digitizeFile (t_sequence *seq, t_waveform *wfm, 
								 unsigned long maxNValues,
								 t_digitizeParameters *p)
{
    unsigned long i;

    unsigned long count;
    long digitalVal;
	int oldVal;
    double analogVal;
    double dI;
    unsigned long nDigitalPoints, n;
    
	// -----------------------------------------------------------
    // 	    init counters
	// -----------------------------------------------------------
//    t     = 0;
    count = 0;
    oldVal     = INT_MAX;
	// -----------------------------------------------------------
    // 	    EXIT, if no points
	// -----------------------------------------------------------
    if (wfm->nPoints == 0) {
    	wfm->DIG_values[count] = AnalogToDigital (0.0, p);
    	wfm->DIG_repeat[count] = 0;
    	return 0;
    }
	// -----------------------------------------------------------
    // 	    calculate number of digital points
	// -----------------------------------------------------------
	if (p->nPoints == 0) {
		nDigitalPoints  = (unsigned long)wfm->durationThisRepetition_50ns / p->timebase_50ns;
		
	}
	else {
		nDigitalPoints  = p->nPoints;
	}
	if (nDigitalPoints == 0) nDigitalPoints = 1;

    
    for (n = 0; n < nDigitalPoints; n++) {
		// -----------------------------------------------------------
	    // 	    get value "i" from point list
		// -----------------------------------------------------------
        dI = (double) (n+0.5) / (double) nDigitalPoints * (double) (wfm->nPoints-1);
        i = (unsigned long) dI;
        if (i > wfm->nPoints-1) 
            analogVal = wfm->uList[wfm->nPoints-1];
        else
        	analogVal  = linearInterpolate (wfm->uList[i], wfm->uList[i+1], 1, dI-i);
		// -----------------------------------------------------------
	    // 	  calculate digital value
		// -----------------------------------------------------------
        analogVal = analogVal * wfm->uScaleThisRepetition + wfm->offsetThisRepetition;
        digitalVal = AnalogToDigital (analogVal, p);
		// -----------------------------------------------------------
	    // 	  add new value 
		// -----------------------------------------------------------
        if (digitalVal == oldVal) {
            wfm->DIG_repeat[count-1]++;
        }
        else { 
            wfm->DIG_values[count] = digitalVal;
            oldVal = digitalVal;
            wfm->DIG_repeat[count] = 1;
            count ++;
			if (count >= maxNValues) {
				// -----------------------------------------------------------
				// 	  reallocate new memory if necessary
				// -----------------------------------------------------------
				maxNValues *= 2;
				wfm->DIG_values = (long  *) realloc (wfm->DIG_values, sizeof(long) * maxNValues );
				wfm->DIG_repeat = (unsigned long *) realloc (wfm->DIG_repeat, sizeof(unsigned long)*maxNValues);
			}
        }
    }
	// -----------------------------------------------------------
    // 	  add last point with 0 repetitions
	// -----------------------------------------------------------
    analogVal = wfm->uList[wfm->nPoints-1] * wfm->uScaleThisRepetition + wfm->offsetThisRepetition;
    wfm->DIG_values[count] = AnalogToDigital (analogVal, p);
    wfm->DIG_repeat[count] = 0;
	// -----------------------------------------------------------
    // 	  return number of digitized values
	// -----------------------------------------------------------
    return count+1;
}




//=======================================================================
//
// 	  digitize waveform type "function"
//
//=======================================================================
unsigned long WFM_digitizeFunction (t_sequence *seq, t_waveform *wfm, 
								   unsigned long maxNValues,
								   t_digitizeParameters *p)
{
    //unsigned long i;
    unsigned long count;
    long digitalVal;
	int oldVal;
    double analogVal;
    double x;
    unsigned long nDigitalPoints, n;
	t_function *f;
	double stepsize;
    
	// -----------------------------------------------------------
    // 	    init counters
	// -----------------------------------------------------------
    count = 0;
   	oldVal     = INT_MAX;
	// -----------------------------------------------------------
    // 	    calculate number of digital points
	// -----------------------------------------------------------
    if (wfm->durationThisRepetition_50ns == 0) {
    	wfm->DIG_values[count] = AnalogToDigital (0.0, p);
    	wfm->DIG_repeat[count] = 0;
    	return 0;
    }
	
	if (p->nPoints == 0) {
		nDigitalPoints  = (unsigned long)wfm->durationThisRepetition_50ns / p->timebase_50ns;
//		if (wfm->maxDigitalPoints != 0) nDigitalPoints = min (wfm->maxDigitalPoints, nDigitalPoints);
	}
	else {
		nDigitalPoints  = p->nPoints;
	}
	if (nDigitalPoints == 0) nDigitalPoints = 2;

	f = FUNCTION_ptr (wfm->functionID);
	if (f == NULL) return 0;
    
	if (wfm->timebaseStretch == 0) wfm->timebaseStretch = 1;
	stepsize = ui64ToDouble(wfm->durationThisRepetition_50ns)/(1.0*VAL_ms) / (1.0*(nDigitalPoints-1));
    for (n = 0; n < nDigitalPoints; n+= wfm->timebaseStretch) {
		// -----------------------------------------------------------
	    // 	    get value "i" from point list
		// -----------------------------------------------------------
        x = (double) (n) * stepsize;
		analogVal =  f->eval (x, wfm->functionParametersThisRepetition, wfm->durationThisRepetition_ns / 1E6, wfm->functionReverseTime);    
		// -----------------------------------------------------------
	    // 	  calculate digital value
		// -----------------------------------------------------------
        digitalVal = AnalogToDigital (analogVal, p);
		// -----------------------------------------------------------
	    // 	  add new value 
		// -----------------------------------------------------------
        if (digitalVal == oldVal) {
            wfm->DIG_repeat[count-1]+= wfm->timebaseStretch;
        }
        else { 
            wfm->DIG_values[count] = digitalVal;
            oldVal = digitalVal;
            wfm->DIG_repeat[count] = wfm->timebaseStretch;
            count ++;
			if (count >= maxNValues) {
				// -----------------------------------------------------------
				// 	  reallocate new memory if necessary
				// -----------------------------------------------------------
				maxNValues *= 2;
				wfm->DIG_values = (long *) realloc (wfm->DIG_values, sizeof(long) * maxNValues );
				wfm->DIG_repeat = (unsigned long *) realloc (wfm->DIG_repeat, sizeof(unsigned long)*maxNValues);
			}
        }
    }
	// -----------------------------------------------------------
    // 	  add last point with 0 repetitions
	// -----------------------------------------------------------
	analogVal = f->eval (wfm->durationThisRepetition_ns / 1E6, wfm->functionParametersThisRepetition, wfm->durationThisRepetition_ns / 1E6, wfm->functionReverseTime);
    wfm->DIG_values[count] = AnalogToDigital (analogVal, p);
    wfm->DIG_repeat[count] = 0;
	// -----------------------------------------------------------
    // 	  return number of digitized values
	// -----------------------------------------------------------
    return count+1;
}



ListType WFM_ADDR_convertVoltagesToImageCoords (t_waveform *wfm1, t_waveform *wfm2, dPoint offset)
{
	ListType pointList= NULL;
	dPoint U, X;
	int i;
	
	pointList = ListCreate (sizeof (dPoint));
	if (wfm1 == NULL) return pointList;
	if (wfm2 == NULL) return pointList;
	if (wfm1->nPoints != wfm2->nPoints) return pointList;
	
	for (i = 0; i < wfm1->nPoints; i+=2) {
		U.x = wfm1->points[i].valueStart - offset.x;
		U.y = wfm2->points[i].valueStart - offset.y;
		X = WFM_ADDR_ctrlVoltageToImage (wfm1, U.x, U.y);
		ListInsertItem (pointList, &X, END_OF_LIST);
	}
	return pointList;
}




ListType WFM_ADDR_lastVoltagesList (t_waveform *wfm1, t_waveform *wfm2)
{
	ListType pointList= NULL;
	dPoint U;
	int i;
	
	pointList = ListCreate (sizeof (dPoint));
	if (wfm1 == NULL) return pointList;
	if (wfm2 == NULL) return pointList;
	if (wfm1->nPoints != wfm2->nPoints) return pointList;
	
	for (i = 0; i < wfm1->nPoints; i+=2) {
		U.x = wfm1->points[i].valueStart;
		U.y = wfm2->points[i].valueStart;
		ListInsertItem (pointList, &U, END_OF_LIST);
	}
	return pointList;
}



/* TODO TODO4
ListType WFM_ADDR_DMD_addressedSitesToImageCoords (t_waveform *wfm)
{
	ListType pointList= NULL;
	dPoint X;
	//int i;

	pointList = ListCreate (sizeof (dPoint));
	
	
	int i, j;
	int value;
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		for (j = 0; j < WFM_ADDR_MATRIXDIM; j++) {
			value = WFM_ADDR_getMatrixValue (wfm, i, j);
			if (value) {
				X = WFM_ADDR_DMD_addressedSiteToImage (wfm, i, j);
				ListInsertItem (pointList, &X, END_OF_LIST);	
			}
			//TABLE_setCellValueYesNo2 (panel, ctrl, MakePoint (i+1, j+1), value);
		}
	}
	
	
	return pointList;	
	
}	 */

ListType WFM_ADDR_voltageListToImageCoords (t_waveform *wfm, ListType lastVoltagesList, dPoint Uoffset)
{
	ListType pointList= NULL;
	dPoint U, X;
	unsigned int i;

	pointList = ListCreate (sizeof (dPoint));
	
	for (i = 1; i <= ListNumItems (lastVoltagesList); i++) {
		ListGetItem (lastVoltagesList, &U, i);
		U.x = U.x + Uoffset.x;
		U.y = U.y + Uoffset.y;
		X = WFM_ADDR_ctrlVoltageToImage (wfm, U.x, U.y);
		ListInsertItem (pointList, &X, END_OF_LIST);
	}
	
	return pointList;
}





dPoint WFM_ADDR_trajSpiral (int i, double *para)
{
//	R(i) = (R1 * i + R2 * (N-i)) / N
//	theta (i) = 2Pi*Rev*i/N
	
	dPoint X;
	double N, r, t;
	
	N = para[WFM_ADDR_TRAJ_NPOINTS];
	if (N == 0) N = 1;
		
	r = (para[WFM_ADDR_TRAJ_RADIUS1] * i + para[WFM_ADDR_TRAJ_RADIUS2] * (N-i)) / N;
	t = (TWO_PI*para[WFM_ADDR_TRAJ_REVOLUTIONS]*i)/N;
	
	X.x = r*cos(t);
	X.y = r*sin(t);
	
	return X;
	
}



ListType WFM_ADDR_trajCalculateImageCoords (t_waveform *wfm, t_image *img)
{
	ListType pointList= NULL;
	dPoint X;
	int i;
	//int j;

	pointList = ListCreate (sizeof (dPoint));
	
	for (i = 0; i < wfm->addrTrajectParams[WFM_ADDR_TRAJ_NPOINTS]; i++) {
		X = WFM_ADDR_trajSpiral (i, wfm->addrTrajectParams);
		X.x += wfm->addrCenter.x;
		X.y += wfm->addrCenter.y;
		ListInsertItem (pointList, &X, -1);
	}
	
	return pointList;
}



ListType WFM_ADDR_convertMatrixToImageCoordsSingleSite (t_waveform *wfm, t_image *img, int matrixNum)
{
	ListType pointList= NULL;
	Point centerLatt, u;
	dPoint X;
	int i, j;
	int matrixCenter;
//	double oldGridPhi[2];

	
	matrixCenter = (WFM_ADDR_MATRIXDIM-1) / 2;
	
	
	pointList = ListCreate (sizeof (dPoint));
	
//	oldGridPhi[0] = img->gridPhi[0];	
//	oldGridPhi[1] = img->gridPhi[1];	
	centerLatt = IMAGE_coordinatesImageToLattice (img, wfm->addrCenter.x, wfm->addrCenter.y, img->gridIndexOffset);
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		for (j = 0; j < WFM_ADDR_MATRIXDIM; j++) {
			if (WFM_ADDR_getMatrixValue (wfm, matrixNum, i, j)) {
				u = MakePoint (j - matrixCenter  + centerLatt.x,  i - matrixCenter + centerLatt.y);
				X = IMAGE_coordinatesLatticeToImage (img, u, img->gridIndexOffset);
				ListInsertItem (pointList, &X, -1);
			}
		}
	}
//	img->gridPhi[0] = oldGridPhi[0];
//	img->gridPhi[1] = oldGridPhi[1];

	
	return pointList;
}

// converts addressed matrix coordinates for DMD to image coordinates.
ListType WFM_ADDR_convertMatrixToImageCoordsDMD (t_waveform *wfm, t_image *img, int matrixNum)
{
	ListType pointList= NULL;
	Point centerLatt;
	//Point u;
	dPoint X;
	int i, j;
	int matrixCenter;
	
	// center of the matrix projected to the atoms (has to be determined somehow)
	double imgCenterX = wfm->addrCalibration[WFM_ADDR_CAL_DMDimgCenterX];//281; // 258/211 from andor software coarse centering
	double imgCenterY = wfm->addrCalibration[WFM_ADDR_CAL_DMDimgCenterY];//234;
//	double oldGridPhi[2];
	
	double lambda = wfm->addrCalibration[WFM_ADDR_CAL_lambda];
	double theta1 = wfm->addrCalibration[WFM_ADDR_CAL_theta1];     
	double theta2 = wfm->addrCalibration[WFM_ADDR_CAL_theta2]; 
	
	double cos_theta1 = cos(theta1*PI/180);
	double sin_theta1 = sin(theta1*PI/180); 
	double cos_theta2 = cos(theta2*PI/180); 
	double sin_theta2 = sin(theta2*PI/180); 
	
	double phi1_offset = img->gridPhi[0]; // wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1] already included!
	double phi2_offset = img->gridPhi[1]; // wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2] already included!   

	
	//tprintf("WFM_ADDR_convertMatrixToImageCoords\n");
	
	matrixCenter = (WFM_ADDR_MATRIXDIM-1) / 2;

	//tprintf("matrix center: %f,%f\n",matrixCenter,matrixCenter);
	//tprintf("theta1/theta2/lambda = %f,%f,%f\n",theta1,theta2,lambda);
	
	
	pointList = ListCreate (sizeof (dPoint));
	
//	oldGridPhi[0] = img->gridPhi[0];	
//	oldGridPhi[1] = img->gridPhi[1];	
	
	// TODO phases for dmd chnage the transformation??
	// wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1]
	
	centerLatt = IMAGE_coordinatesImageToLattice (img, wfm->addrCenter.x, wfm->addrCenter.y, img->gridIndexOffset);
	
	for (i = 0; i < WFM_ADDR_MATRIXDIM; i++) {
		for (j = 0; j < WFM_ADDR_MATRIXDIM; j++) {
			if (WFM_ADDR_getMatrixValue (wfm, matrixNum, i, j)) {
				//u = MakePoint (j - matrixCenter  + centerLatt.x,  i - matrixCenter + centerLatt.y);
				//X = IMAGE_coordinatesLatticeToImage (img, u, img->gridIndexOffset);
				X.x = cos_theta1*lambda*(i-matrixCenter-phi1_offset) + cos_theta2*lambda*(j-matrixCenter-phi2_offset)+imgCenterX;
				X.y = sin_theta1*lambda*(i-matrixCenter-phi1_offset) + sin_theta2*lambda*(j-matrixCenter-phi2_offset)+imgCenterY;       
				ListInsertItem (pointList, &X, END_OF_LIST);
			}
		}
	}
//	img->gridPhi[0] = oldGridPhi[0];
//	img->gridPhi[1] = oldGridPhi[1];

	
	return pointList;
}

//
//
//

ListType WFM_ADDR_convertMatrixToImageCoords (t_waveform *wfm, t_image *img, int matrixNum)   
{
	switch (wfm->addrType) {
	case WFM_ADDR_TYPE_SINGLESITE:
		return WFM_ADDR_convertMatrixToImageCoordsSingleSite(wfm,img, matrixNum); 
	case WFM_ADDR_TYPE_DMD:
		return WFM_ADDR_convertMatrixToImageCoordsDMD(wfm,img, matrixNum);
	}
	return 0;
}




dPoint WFM_ADDR_ctrlVoltageToImage (t_waveform *wfm, double U1, double U2)
{
	double x1, x2, y1, y2;
	dPoint p;
	
	x1 =  wfm->addrCalibration[WFM_ADDR_CAL_a1x] * U1 
		 + wfm->addrCalibration[WFM_ADDR_CAL_b1x] 
		 + wfm->addrCalibration[WFM_ADDR_CAL_a2x] * (U2 - wfm->addrCalibration[WFM_ADDR_CAL_U20]);

	 y1 =  wfm->addrCalibration[WFM_ADDR_CAL_a1y] * U1 
		 + wfm->addrCalibration[WFM_ADDR_CAL_b1y] 
		 + wfm->addrCalibration[WFM_ADDR_CAL_a2y] * (U2 - wfm->addrCalibration[WFM_ADDR_CAL_U20]);
	 

	 x2 =  wfm->addrCalibration[WFM_ADDR_CAL_a1x] * (U1 - wfm->addrCalibration[WFM_ADDR_CAL_U10])
		 + wfm->addrCalibration[WFM_ADDR_CAL_b2x] 
		 + wfm->addrCalibration[WFM_ADDR_CAL_a2x] * U2;

	 y2 =  wfm->addrCalibration[WFM_ADDR_CAL_a1y] * (U1 - wfm->addrCalibration[WFM_ADDR_CAL_U10])
		 + wfm->addrCalibration[WFM_ADDR_CAL_b2y] 
		 + wfm->addrCalibration[WFM_ADDR_CAL_a2y] * U2;
	 
	 
	 p.x = (x1 + x2) / 2;
	 p.y = (y1 + y2) / 2;
	 
	 return p;
}


dPoint WFM_ADDR_imageToCtrlVoltage (t_waveform *wfm, dPoint p, dPoint *err)
{
	double N, beta;
	dPoint U, V, W, Uerr;
	int e1, e2;
	
	U.x = 0;
	U.y = 0;
	Uerr.x = 0;
	Uerr.y = 0;
	
	if (wfm == 0) return U;
	
	N =    wfm->addrCalibration[WFM_ADDR_CAL_a1x] * wfm->addrCalibration[WFM_ADDR_CAL_a2y]
		-  wfm->addrCalibration[WFM_ADDR_CAL_a1y] * wfm->addrCalibration[WFM_ADDR_CAL_a2x];
	
	if (N != 0) {
		beta = 1/N;
		V.x =  beta * ((p.x - wfm->addrCalibration[WFM_ADDR_CAL_b1x]) * wfm->addrCalibration[WFM_ADDR_CAL_a2y]
					 - (p.y - wfm->addrCalibration[WFM_ADDR_CAL_b1y]) * wfm->addrCalibration[WFM_ADDR_CAL_a2x]);
		
		V.y = - beta * ((p.x - wfm->addrCalibration[WFM_ADDR_CAL_b1x]) * wfm->addrCalibration[WFM_ADDR_CAL_a1y]
					  - (p.y - wfm->addrCalibration[WFM_ADDR_CAL_b1y]) * wfm->addrCalibration[WFM_ADDR_CAL_a1x])
			 + wfm->addrCalibration[WFM_ADDR_CAL_U20];
		
		W.x =  beta * ((p.x - wfm->addrCalibration[WFM_ADDR_CAL_b2x]) * wfm->addrCalibration[WFM_ADDR_CAL_a2y]
					 - (p.y - wfm->addrCalibration[WFM_ADDR_CAL_b2y]) * wfm->addrCalibration[WFM_ADDR_CAL_a2x])
			  + wfm->addrCalibration[WFM_ADDR_CAL_U10]; 
		
		W.y = - beta * ((p.x - wfm->addrCalibration[WFM_ADDR_CAL_b2x]) * wfm->addrCalibration[WFM_ADDR_CAL_a1y]
					  - (p.y - wfm->addrCalibration[WFM_ADDR_CAL_b2y]) * wfm->addrCalibration[WFM_ADDR_CAL_a1x]);
		
		e1 = wfm->addrCalibration[WFM_ADDR_CAL_enableCal1] != 0;
		e2 = wfm->addrCalibration[WFM_ADDR_CAL_enableCal2] != 0;
		if ((e1 != 0) || (e2 != 0)) {
			
			U.x = (V.x * e1 + W.x * e2) / (1.0*(e1+e2))  + wfm->addrCalibration[WFM_ADDR_CAL_offsetU1];   	
			if (e1 && e2) Uerr.x = (V.x - W.x) / 2;
		
			U.y = (V.y * e1 + W.y * e2) / (1.0*(e1+e2))  + wfm->addrCalibration[WFM_ADDR_CAL_offsetU2];
			if (e1 && e2) Uerr.y = (V.y - W.y) / 2;
		}
	}

	if (err != NULL) *err = Uerr;
	return U;
}





dPoint WFM_ADDR_voltageOffsetFromPhaseDifference (t_waveform *wfm, t_image *img, Point testPoint)
{
	Point centerLatt;
	//Point u;
	dPoint X;
	double phi1, phi2;
	dPoint Uphi0, Uphi0err, U, Uerr;
	dPoint diff;
	Point indexOffset;

	if ((wfm == 0) || (img == NULL)) {
		diff.x = 0;
		diff.y = 0;
	}
	else {
		phi1 = img->gridPhi[0];
		phi2 = img->gridPhi[1];
		IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
								 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
								 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 0, 0);
		indexOffset = img->gridIndexOffset;
		centerLatt = IMAGE_coordinatesImageToLattice (img, testPoint.x, testPoint.y, img->gridIndexOffset);
		X = IMAGE_coordinatesLatticeToImage (img, MakePoint (centerLatt.x,  centerLatt.y), img->gridIndexOffset);
		Uphi0 = WFM_ADDR_imageToCtrlVoltage (wfm, X, &Uphi0err);
	
		IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
								 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
								 wfm->addrCalibration[WFM_ADDR_CAL_theta2], phi1 , phi2 );
		X = IMAGE_coordinatesLatticeToImage (img, MakePoint (centerLatt.x,  centerLatt.y), indexOffset);
		U = WFM_ADDR_imageToCtrlVoltage (wfm, X, &Uerr);
	
		diff.x =  U.x - Uphi0.x;
		diff.y =  U.y - Uphi0.y;
	}
	
	return diff;
}



	
	
void WFM_ADDR_offsetThisRepetition (t_waveform *wfm, int repetition, int nRepetitions)
{
	double from, to;
	int i;
	
	tprintf("Calculate addr offset for repetition for waveform '%s'\n",wfm->name);
	
	/*
	from = wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi1];
	to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_offsetPhi1];
	if ((wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi1]) && (nRepetitions > 1)) {
		// step offset
		wfm->addrOffsetThisRepetition[0] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);   
	}
	else {
		wfm->addrOffsetThisRepetition[0] = from;
	}
		
	from = wfm->addrCalibration[WFM_ADDR_CAL_offsetPhi2];
	to = wfm->addrCalibrationStepTo[WFM_ADDR_CAL_offsetPhi2];
	if ((wfm->addrCalibrationStep[WFM_ADDR_CAL_offsetPhi2]) && (nRepetitions > 1)) {
		// step offset
		wfm->addrOffsetThisRepetition[1] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);   
	}
	else {
		wfm->addrOffsetThisRepetition[1] = from;
	}	  */
		  
	for (i=0; i < WFM_ADDR_N_CALIBRATION_PARAMETERS; i++) {
		from = wfm->addrCalibration[i];
		to = wfm->addrCalibrationStepTo[i];
		if ((wfm->addrCalibrationStep[i]) && (nRepetitions > 1) && WFM_ADDR_allowStepParameter(i)) {
			// step calibration
			wfm->addrCalibrationThisRepetition[i] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);   
		} else {
			wfm->addrCalibrationThisRepetition[i] = from;
		}
	}
	

}


// returns first index of the matrix of index picNum for this repetition!
// picNum starts at zero!
// rep starts at zero
int WFM_ADDR_matrixNumThisRepetition(t_waveform* wfm, int rep, int nRep, int picNum)
{
	int matrixIndex = 0;
	if (picNum >= wfm->addrMatricesPerRepetition) {   // handle bad picNum
		picNum =  wfm->addrMatricesPerRepetition-1;
	}
	if (wfm->stepAddrMatrix) {
		matrixIndex = (rep*wfm->addrMatricesPerRepetition)+picNum;
	} else {
		matrixIndex = picNum;
	}
	if (matrixIndex < 0) {
		tprintf("Error: negative matrix index!!!\n");
		return 0;
	}
	if (matrixIndex >= WFM_ADDR_MATRIX_NUM) {
		return WFM_ADDR_MATRIX_NUM-1; // just use last one if too many repetitions!
	} else {
		return matrixIndex;
	}
}


t_waveform *WFM_ADDR_createWaveform (t_sequence *seq, t_waveform *wfm, double phi1, double phi2)
// returns linked waveform
{
	t_image *img;
	dPoint p;
	dPoint U;
	t_waveform *wfm2 = NULL;
	t_waveform *h;
	int i;
	ListType pointList;
	double timeStep_ms;
	int matrixNum;
	

	WFM_ADDR_offsetThisRepetition (wfm, seq->repetition, seq->nRepetitions);  
	
	if (wfm->addrLinkWfm != 0) return 0; // do not create waveform if this waveform is linked to another one
	
	//tprintf("WFM_ADDR_createWaveform ...\n");
	//tprintf("find link to waveform %s ...\n",wfm->name);
	
	
	
	
	// find linking WFM, i.e. wfm2 links to wfm1
	for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
		ListGetItem (seq->lWaveforms, &wfm2, i);
		if (wfm2->addrLinkWfm != 0) {
			h = WFM_ptr (seq, wfm2->addrLinkWfm);
			if (h == wfm) {
				break;
			}
		}
		wfm2 = NULL;
	}
	if (wfm2 == NULL) return 0; // no linking wfm found
	
	img = IMAGE_new ();
	
	
//	tprintf ("\nphase offset %1.3f %1.3f\n", wfm->addrOffsetThisRepetition[0], wfm->addrOffsetThisRepetition[1]);

	IMAGE_setGridParameters (img, wfm->addrCalibration[WFM_ADDR_CAL_lambda], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta1], 
							 wfm->addrCalibration[WFM_ADDR_CAL_theta2], 
							 phi1 + wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1], phi2 + wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2]);  
							 //phi1 + wfm->addrOffsetThisRepetition[0], phi2 + wfm->addrOffsetThisRepetition[1]);

	switch (wfm->addrType) {
		case WFM_ADDR_TYPE_SINGLESITE:
		case WFM_ADDR_TYPE_DMD:     
			tprintf("create waveform for matrix %d\n",seq->repetition);
			matrixNum = WFM_ADDR_matrixNumThisRepetition(wfm, seq->repetition, seq->nRepetitions, 0);  // only needed to show the pattern and write addr points at beginning of next repetition.  
			pointList = WFM_ADDR_convertMatrixToImageCoords (wfm, img, matrixNum); 
			wfm->nPoints = 2 * ListNumItems (pointList);
			wfm->points = realloc (wfm->points, sizeof (t_point) * wfm->nPoints);
			if (wfm2 != 0) {
				wfm2->nPoints = wfm->nPoints;
				wfm2->points = realloc (wfm2->points, sizeof (t_point) * wfm2->nPoints);
			}
	
			timeStep_ms = ui64ToDouble (wfm->durationFrom_50ns / VAL_ms);
			for (i = 0; i < ListNumItems (pointList); i++) {
				ListGetItem (pointList, &p, i+1);
				U = WFM_ADDR_imageToCtrlVoltage (wfm, p, 0);

				POINT_init (&wfm->points[2*i]);
				POINT_init (&wfm->points[2*i+1]);
				wfm->points[2*i].valueStart = U.x;
				wfm->points[2*i+1].valueStart = U.x;
				wfm->points[2*i].timeStart_ns = i * timeStep_ms * 1.0E6;
				wfm->points[2*i+1].timeStart_ns = (i+1) * timeStep_ms *  1.0E6;
				if (wfm2 != 0) {
					POINT_init (&wfm2->points[2*i]);
					POINT_init (&wfm2->points[2*i+1]);
					wfm2->points[2*i].valueStart = U.y;
					wfm2->points[2*i+1].valueStart = U.y;
					wfm2->points[2*i].timeStart_ns = i * timeStep_ms * 1.0E6;
					wfm2->points[2*i+1].timeStart_ns = (i+1) * timeStep_ms *  1.0E6;
				}
			}
			break;
		case WFM_ADDR_TYPE_TRAJECTORY:
			pointList = WFM_ADDR_trajCalculateImageCoords (wfm, img); 
			wfm->nPoints = ListNumItems (pointList);
			wfm->points = realloc (wfm->points, sizeof (t_point) * wfm->nPoints);
			if (wfm2 != 0) {
				wfm2->nPoints = wfm->nPoints;
				wfm2->points = realloc (wfm2->points, sizeof (t_point) * wfm2->nPoints);
			}
	
			if (wfm->addrTrajectParams[WFM_ADDR_TRAJ_NPOINTS] == 0) 
				wfm->addrTrajectParams[WFM_ADDR_TRAJ_NPOINTS] = 1;
			timeStep_ms = ui64ToDouble (wfm->addrTrajectParams[WFM_ADDR_TRAJ_DURATION_MS])
						 / wfm->addrTrajectParams[WFM_ADDR_TRAJ_NPOINTS];
			for (i = 0; i < ListNumItems (pointList); i++) {
				ListGetItem (pointList, &p, i+1);
				U = WFM_ADDR_imageToCtrlVoltage (wfm, p, 0);
				POINT_init (&wfm->points[i]);
				wfm->points[i].valueStart = U.x;
				wfm->points[i].timeStart_ns = i * timeStep_ms * 1.0E6;
				if (wfm2 != 0) {
					POINT_init (&wfm2->points[i]);
					wfm2->points[i].valueStart = U.y;
					wfm2->points[i].timeStart_ns = i * timeStep_ms * 1.0E6;
				}
			}
			
			break;
		default:
			pointList = ListCreate (sizeof (dPoint));
			break;
	}
	
	ListDispose (pointList);
	IMAGE_free (img);
	free (img);
	
	return wfm2;
	
}


//=======================================================================
//
// 	  digitize waveform 
//
//=======================================================================
unsigned long WFM_digitize (t_sequence *seq, t_waveform *wfm, 
   							    t_digitizeParameters *p, int allowAddrLinkWfm)
{
	unsigned long nPoints = 10000; // default number of points
	unsigned long count;
	int stretch = 0;
	t_waveform *wfm2;
	
	// -----------------------------------------------------------
    // 	  "HOLD" 
	// -----------------------------------------------------------
	if (wfm->type == WFM_TYPE_HOLD) return 0;
	if ((wfm->addrLinkWfm != 0) && (!allowAddrLinkWfm)) return 0;
	// -----------------------------------------------------------
    // 	  free memory for digital values (from prev. digitalization)
	// -----------------------------------------------------------
	free (wfm->DIG_values);
	wfm->DIG_values = NULL;
	free (wfm->DIG_repeat);
	wfm->DIG_repeat = NULL;
	wfm->DIG_NValues = 0;
//	wfm->DIG_Timebase = 0;
	// -----------------------------------------------------------
    // 	  estimate duration of waveform
	//    (to allocate appropriate amount of memory)
	//    other memory is freed if the exact number of points is known
	// -----------------------------------------------------------
	WFM_calculateOffsetAndDuration (wfm, seq->repetition, seq->nRepetitions, 0);
	nPoints = (unsigned long)wfm->durationThisRepetition_50ns / p->timebase_50ns;
	if (nPoints > 10000) nPoints = 10000;
	if (nPoints == 0) nPoints = 1;	
	switch (wfm->type) {
	    case WFM_TYPE_POINTS:
	    	nPoints *= wfm->nPoints;
	    	break;
	    case WFM_TYPE_FILE:
	    	nPoints  = (nPoints * 5) / 4;
	    	break;
	    case WFM_TYPE_STEP:
	    	nPoints  = 1;
	    	break;
		case WFM_TYPE_FUNCTION:
	    	nPoints  = (nPoints * 5) / 4;
			break;		
		case WFM_TYPE_ADDRESSING:
			nPoints = WFM_ADDR_MATRIXDIM * WFM_ADDR_MATRIXDIM / 2;
			break;
	}	
	// -----------------------------------------------------------
    // 	  allocate memory for digital values
	// -----------------------------------------------------------
	wfm->DIG_values = (long *) 			malloc (sizeof(long)         * nPoints);
    wfm->DIG_repeat = (unsigned long *) malloc (sizeof(unsigned long) * nPoints);
	// -----------------------------------------------------------
    // 	  different digitalization procedures 
    //    depending on waveform type
	// -----------------------------------------------------------
    switch (wfm->type) {
        case WFM_TYPE_POINTS: 
//			insertSteppedOutputVoltages (seq, wfm);
            count = WFM_digitizePoints (seq, wfm, nPoints, p); 
            break;
        case WFM_TYPE_FILE:
	        count = WFM_digitizeFile (seq, wfm, nPoints, p);
	        break;
	    case WFM_TYPE_STEP:
			wfm->DIG_values[0] = AnalogToDigital (WFM_getStepOutputVoltage(seq, wfm), p);
			wfm->DIG_repeat[0] = 1;
	    	count = 1;
			break;
		case WFM_TYPE_FUNCTION:
	        count = WFM_digitizeFunction (seq, wfm, nPoints, p);
			if ((count > 100000) && (!wfm->asked)) {
				if (count > 100000) stretch = 10;
				if (count > 1000000) stretch = 100;
				if (ConfirmPopupf ("Warning!", "Digitized waveform '%s' contains many points (%d)!\nThis will drastically decrease performance.\nDo you want to stretch the timebase by a factor of %d?",
							   wfm->name, count, stretch)) {
					wfm->timebaseStretch = stretch;
			        count = WFM_digitizeFunction (seq, wfm, nPoints, p);
					wfm->asked = 1;
				}
			}
			break;
		case WFM_TYPE_ADDRESSING:
			if (wfm->addrLinkWfm == 0) { // only digitize addressing if it is not linked to another waveform
				wfm2 = WFM_ADDR_createWaveform (seq, wfm, 0, 0);
				if (wfm2 == 0) {
					//tprintf("Phase feedback waveform2 not found.\n");
				} else {
					WFM_digitize (seq, wfm2, p, 1);
					if (seq->outData != NULL) {
						seq->phaseFeedbackWfm[1] = wfm2;
						tprintf("Phase feedback waveform2 : %s\n",wfm2->name);   
					}
				}
				
			} else {
				//tprintf("Skip linked waveform %s\n",wfm->name);      
			}
			if (seq->outData != NULL) {
				seq->phaseFeedbackWfm[0] = wfm;
				//tprintf("Phase feedback waveform1 : %s\n",wfm->name);
			}
			wfm->type = WFM_TYPE_POINTS;
			WFM_calculateOffsetAndDuration (wfm, seq->repetition, seq->nRepetitions, 0);
			wfm->type = WFM_TYPE_ADDRESSING;
			count = WFM_digitizePoints (seq, wfm, nPoints, p);
			break;
			
		default:
			count = 0;
	}
	// -----------------------------------------------------------
    // 	  reallocate memory (to miminize memory usage)
	// -----------------------------------------------------------
	wfm->DIG_values = (long *) 	    realloc (wfm->DIG_values, sizeof(long)*count);
    wfm->DIG_repeat = (unsigned long *) realloc (wfm->DIG_repeat, sizeof(unsigned long)*count);
	// -----------------------------------------------------------
    // 	  set number of digitized values
	// -----------------------------------------------------------
    wfm->DIG_NValues = count;
//    wfm->DIG_Timebase = p->timebase_50ns;
    wfm->digParameters = *p;
    return count;
}

												   
												   
												   
//=======================================================================
//
// 	  digitize all waveforms 
//
//=======================================================================
void SEQUENCE_digitizeAllWaveforms (t_sequence *seq, int startCh, int endCh, int simulate)
{
	t_waveform *wfm;
	t_digitalBlock *d;
//	t_analogBlock *a;
	int i;
	int ch;
	
	if (seq->outData == NULL) return;

	
	// -----------------------------------------------------------
    // 	  reset digitalization flag for all waveforms
	// -----------------------------------------------------------
	for (i = ListNumItems (seq->lWaveforms); i > 0; i--) {
	    ListGetItem (seq->lWaveforms, &wfm, i);
		wfm->digitize = 0;
	}
	
	// -----------------------------------------------------------
    // 	  mark all used waveforms
	// -----------------------------------------------------------
	for (i = ListNumItems (seq->lDigitalBlocks); i > 0; i--) {
		ListGetItem (seq->lDigitalBlocks, &d, i);
		if (!d->disable) {
			for (ch = startCh; ch < endCh; ch++) {
				if (ch < N_DAC_CHANNELS) {
			    	wfm = WFM_ptr(seq, d->waveforms[ch]);
		    		if (wfm != NULL) {
			    		wfm->digitize = 1;
		    			// 	if "add stepped waveform", always digitize
		    			if (wfm->add != WFM_ID_UNCHANGED) {
							WFM_ptr(seq, wfm->add)->digitize = 1;
						}
						if (wfm->addrLinkWfm != WFM_ID_UNCHANGED) {
							WFM_ptr(seq, wfm->addrLinkWfm)->digitize = 1;
						}
					}
				}
			}
		}
	}
	
	// -----------------------------------------------------------
    // 	  get digitalization parameters
	// -----------------------------------------------------------

	// -----------------------------------------------------------
    // 	  digitize marked waveforms
	// -----------------------------------------------------------
	for (i = 1; i <= ListNumItems (seq->lWaveforms); i++) {
	    wfm = WFM_ptr(seq, i);
	    if (wfm->type == WFM_TYPE_ADDRESSING) {
			WFM_digitize (seq, wfm, seq->outData->digParameters, 0);
//		    DEBUGOUT_printf ("Digitizing %s\n", wfm->name);
		} else if (wfm->digitize) {
	    	// speedup:
	    	// digitize only if parameters have changed
	    	// or if waveform contains variable voltages
	    	// (otherwise keep old digitalization results)
	    	if ((wfm->type == WFM_TYPE_STEP) || WFM_hasVariableParameters (wfm) || DIGITIZEPARAMETERS_compare(&wfm->digParameters, seq->outData->digParameters) != 0) {
		        WFM_digitize (seq, wfm, seq->outData->digParameters, 0);
//		        DEBUGOUT_printf ("Digitizing %s\n", wfm->name);
	    	 }
	    }
	}
}

	    





/************************************************************************
 *
 *    handling of digital blocks
 *
 ************************************************************************/

//=======================================================================
//
// 	  init counter for blocks (loops)
//
//=======================================================================
void SEQUENCE_initBlockCount (t_sequence *seq)
{
    seq->inLoop      = 0;
    seq->loopsToDo   = 0;
    seq->loopStartNr = 0;
}



//=======================================================================
//
// 	  return pointer to next block (resolve loops)
//
//=======================================================================
t_digitalBlock *DIGITALBLOCK_getNext (t_digitalBlock *p)
{
	t_sequence *seq;
	
	seq = p->seq;
	if (!seq->inLoop && (p->blockMode > 1)) {
		// -----------------------------------------------------------
	    // 	  found loop start
		// -----------------------------------------------------------
	    seq->inLoop     = 1;	
	    // remember number of repetitions
	    seq->loopsToDo  = p->blockMode; 
	    // memorize loop start
	    seq->loopStartNr = p->blockNr;	
	}

	// -----------------------------------------------------------
    // 	  get pointer to next block
	// -----------------------------------------------------------
	p = DIGITALBLOCK_ptr (seq, p->blockNr+1);
	if (!seq->inLoop) return p; // no  loop!
	
	// -----------------------------------------------------------
    // 	  repeat loop if
    // -----------------------------------------------------------
	if ((p == NULL)							  		// ...sequence ends
		|| (p->blockMode == BMODE_Standard) // ...or next block normal
	  	|| (p->blockMode > 0)) {			// ...or new start of loop
	  		(seq->loopsToDo) --;
	  		if (seq->loopsToDo > 0) {
	  			p = DIGITALBLOCK_ptr (seq, seq->loopStartNr);
	  		}
	  		else seq->inLoop = 0;
	  			
	}

	return p;
}








//=======================================================================
//
//   calculate the number of blocks INCLUDING LOOPS
//   and the total duration of the sequence
//
//=======================================================================
unsigned long SEQUENCE_effectiveNumberOfBlocks (t_sequence *seq)
{
    unsigned long n;
    t_digitalBlock *k;

    k = DIGITALBLOCK_ptr (seq, 1);                
    n = 0;	
    SEQUENCE_initBlockCount (seq);
    while (k != NULL) {
        k = DIGITALBLOCK_getNext (k);
        n++; 
    }
    return n;

/*  alte Version: Schneller, aber weniger elegant

    unsigned long loopLength;
    unsigned long nLoop;
    
    seq->numDIOValues = 0;
    while (k != NULL) {
        loopLength = 1;
        if ((BlockP(k)->blockMode) > 0) {
           // Länge des Loops bestimmen
		    nLoop = BlockP(k)->blockMode;
           	while ((k != NULL) && (BlockP(k)->blockMode != BMODE_LoopEnd)
           					 && (BlockP(k)->blockMode != BMODE_Standard)) {
			    k = k->next;
			    loopLength++;
            } 	
         }
         else {
             nLoop = 1;
         }  
         seq->numDIOValues += nLoop * loopLength;
         k = k->next;
    }
*/
}




//=======================================================================
//
//   calculate the maximum allowed length of a waveform 
//   (it can be interrupted by an other block or just end..)
//
//=======================================================================
unsigned long DIGITALBLOCK_getMaxLengthOfWaveform (
                t_digitalBlock *p, int channel, int repetition, int nRepetitions)
{
    unsigned long maxLength;
    t_waveform *wfm, *startWfm;
    
    maxLength = 0;
	// -----------------------------------------------------------
    //   get started waveform
    // -----------------------------------------------------------
    wfm = p->wfmP[channel];
    startWfm = wfm;
    WFM_calculateOffsetAndDuration (wfm, repetition, nRepetitions, 0);
	// -----------------------------------------------------------
    //   search for the next change of that channel
    //   and calculate the time "maxLength" when this happens
    // -----------------------------------------------------------
    do {
        maxLength += p->calcDuration;
        p = DIGITALBLOCK_getNext(p);
        if ((p != NULL) && (!p->disable)) {
            wfm = p->wfmP[channel];
        }
    } while ((p != NULL) && (wfm == NULL));
	// -----------------------------------------------------------
    //    no further waveform is started (no overlap)
    //    entire waveform is started
    // -----------------------------------------------------------
    if (wfm == NULL) {
    	return (unsigned long)startWfm->durationThisRepetition_50ns;
    }
	// -----------------------------------------------------------
    //    no further waveform is started (no overlap)
    //    entire waveform is started
    // -----------------------------------------------------------

    if (maxLength < startWfm->durationThisRepetition_50ns)
		// -----------------------------------------------------------
	    //    overlap with another waveform
	    //    return the maximium length
	    // -----------------------------------------------------------
        return maxLength;
    else 
		// -----------------------------------------------------------
	    //    no overlap with another waveform
	    // -----------------------------------------------------------
       return (unsigned long)startWfm->durationThisRepetition_50ns;
    
}



//=======================================================================
//
//   increase memory for AO values if necessary
//
//=======================================================================
void OUTPUTDATA_checkMemoryForAOValues (t_outputData *out, int ch, unsigned long bytesToAdd)
{
	int changed = 0;
	
	while (out->numAOValues[ch] + bytesToAdd > out->maxAOValues[ch]) {
		// -----------------------------------------------------------
	    //    check if there is insufficient memory in one of 
	    //    the channels
	    // -----------------------------------------------------------
	    out->maxAOValues[ch] = out->maxAOValues[ch] * 2;
	    changed = 1;
	}
	if (changed) {
		// -----------------------------------------------------------
	    //    re-allocate memory
	    // -----------------------------------------------------------
	    out->AO_Values[ch] = (long *) realloc (out->AO_Values[ch], out->maxAOValues[ch] * sizeof (long));
	    out->AO_ValueDuration[ch] = (unsigned long *) realloc (out->AO_ValueDuration[ch], out->maxAOValues[ch] * sizeof (unsigned long));
	}

}





//=======================================================================
//
//     calculate waveforms to be started for each digital block 
//     return value: 1: variable block duration
// 				     0: fixed    block duration
//
//=======================================================================
int DIGITALBLOCK_getWaveformsAndDuration (t_sequence *seq, t_digitalBlock *b)             
{
    int ch;
    int variableBlock = 0;
    long uout;
	t_digitizeParameters *digParameters = NULL;
	
	if (seq->outData != NULL) digParameters = seq->outData->digParameters;
    
    for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
        // -----------------------------------------------------------
    	//    get analog sequence + waveform
    	// -----------------------------------------------------------
		if (b->disable) b->wfmP[ch] = NULL;
		else b->wfmP[ch]  = WFM_ptr(seq, b->waveforms[ch]);
        if (b->wfmP[ch] != NULL) {
            // -----------------------------------------------------------
	    	//    check if sequence contains VARIABLE TIMES
	    	// -----------------------------------------------------------
            if (b->wfmP[ch]->type == WFM_TYPE_STEP) {
	            // -----------------------------------------------------------
	            // 	step --> variable time
		    	// -----------------------------------------------------------
	    		variableBlock = 1;
        		uout = AnalogToDigital (WFM_getStepOutputVoltage(seq, b->wfmP[ch]), digParameters );
                variableBlock = 1;
            }
            if (b->wfmP[ch]->add != WFM_ID_UNCHANGED) {
	            // -----------------------------------------------------------
            	//  stepped waveform added
		    	// -----------------------------------------------------------
            	variableBlock = 1;
        		uout = AnalogToDigital (WFM_getStepOutputVoltage(seq, WFM_ptr(seq, b->wfmP[ch]->add)), digParameters );
            }
            if (WFM_hasVariableParameters (b->wfmP[ch])) {
            	variableBlock = 1;
            };
			

            // -----------------------------------------------------------
	    	//    check if sequence contains VARIABLE TIMES
	    	// -----------------------------------------------------------
        }
    }
	
    
    // -----------------------------------------------------------
	//    get the duration of the current digital block
	// -----------------------------------------------------------
    b->calcDuration = DIGITALBLOCK_getDuration(b, seq->repetition);
    if (b->variableTime && !b->disable) variableBlock = 1;
    return variableBlock;				   
}

					 

//=======================================================================
//
//     insert all block numbers and references
//
//     return value: 1: variable block duration
// 				     0: fixed    block duration
//
//=======================================================================
int SEQUENCE_insertBlockNumbersAndReferences (t_sequence *seq, int *nTriggerDuringSeq)
{
    int i, n;
    int variableBlock = 0;
    t_digitalBlock *b;
	int nTriggers = 0;
	
    
	n = ListNumItems (seq->lDigitalBlocks);
    for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
	    // -----------------------------------------------------------
		//    insert block no
		// -----------------------------------------------------------
	    b->blockNr = i;
	    // -----------------------------------------------------------
		//    self-reference sequence
		// -----------------------------------------------------------
	    b->seq   = seq;
	    // -----------------------------------------------------------
		//    check for variable block durations
		// -----------------------------------------------------------
 	    variableBlock |= DIGITALBLOCK_getWaveformsAndDuration (seq, b);
 	    variableBlock |= DIGITALBLOCK_hasVariablePulse (b);
		
		if (b->waitForExtTrigger && !b->disable) {
			if (b->blockMode == 0) nTriggers ++;
			else nTriggers += DIGITALBLOCK_getLoopRepetitions (seq, i);
		}		
	}
	if (nTriggerDuringSeq != NULL) *nTriggerDuringSeq = nTriggers;
	
	return variableBlock;
	

}






void OUTPUTDATA_getTimebases (t_sequence *seq)
{
	t_outputData *o;
	
	o = seq->outData;
	
	
	if (seq->DIO_timebase_50ns < 2) seq->DIO_timebase_50ns = 2;
	if (seq->DIO_timebase_50ns > 20*VAL_us) seq->DIO_timebase_50ns = 20*VAL_us;
	if (seq->AO_timebase_50ns < 2*VAL_us) seq->AO_timebase_50ns = 2*VAL_us;
	if (seq->AO_timebase_50ns > 20*VAL_us) seq->AO_timebase_50ns = 20 *VAL_us;
	if (seq->DIO_timebase_50ns > seq->AO_timebase_50ns) seq->AO_timebase_50ns = seq->DIO_timebase_50ns;
	if (o == NULL) return;
	
	switch (config->hardwareType) {
		case HARDWARE_TYPE_ADWIN:
			o->DIOdeltaT = ADWIN_TIMEBASE_DIGITAL;
			o->AOdeltaT = ADWIN_TIMEBASE_ANALOG;
			o->digParameters = digitizeParameters_ADWIN (); 
			break;
		default:
			o->DIOdeltaT = seq->DIO_timebase_50ns;
			o->AOdeltaT = seq->AO_timebase_50ns;
			o->digParameters = digitizeParameters_PCI67XX (o->AOdeltaT); 
	}	
	
	
}




//=======================================================================
//
//   append digitized values to output data
//
//=======================================================================
unsigned long OUTPUTDATA_appendBlockAOValues (t_outputData *out, t_digitalBlock *bl, 
                       	       int ch, 
                       	       int repetition, int nRepetitions)
{
    t_waveform		*wfm;
    unsigned long maxLength;
    unsigned long i, start, valuesToCopy;
    double voltage;
    unsigned long deltaT;
	//long tmp;

	deltaT = out->AOdeltaT;
	
	// -----------------------------------------------------------
    //    first call: --> set first value to zero 
    // -----------------------------------------------------------
	if (out->numAOValues[ch] == 0) {
        out->AO_Values[ch][0]        = AnalogToDigital (0.0, out->digParameters);
        out->AO_ValueDuration[ch][0] = 0;
        out->numAOValues[ch] = 1;
	}
	
	wfm = bl->wfmP[ch];
	// -----------------------------------------------------------
    //    repeat last value, if empty waveform 
    // -----------------------------------------------------------
	if (wfm == NULL) {
	    out->AO_ValueDuration[ch][out->numAOValues[ch]-1] 
	    	+= bl->calcDuration;
	    return bl->calcDuration;
	}    
	// -----------------------------------------------------------
    //    get maximum length of waveform
    //	  (check if it is interrupted by another waveform)
    // -----------------------------------------------------------
	maxLength = DIGITALBLOCK_getMaxLengthOfWaveform (bl, ch, repetition, nRepetitions);
	start = out->numAOValues[ch];
 	// -----------------------------------------------------------
    //    determinie number of values to copy
    // -----------------------------------------------------------
	valuesToCopy = maxLength / deltaT;

/*	
	if ((wfm->positionInList == 77) && (bl->blockNr == 25)) {
		Breakpoint();	
	}
*/
// -----------------------------------------------------------
//   Special case: CONSTANT VOLTAGE
// -----------------------------------------------------------
	if (wfm->type == WFM_TYPE_CONSTVOLTAGE) {
		// -----------------------------------------------------------
		//    check if sufficient memory
		// -----------------------------------------------------------
		OUTPUTDATA_checkMemoryForAOValues(out, ch, 1);
		// -----------------------------------------------------------
	    //    add new value to list
	    // -----------------------------------------------------------
        voltage =  bl->constVoltages[ch];
        out->AO_Values[ch][start] = AnalogToDigital (voltage, out->digParameters);
        out->AO_ValueDuration[ch][start] = bl->calcDuration;
		out->numAOValues[ch] += 1;
        return bl->calcDuration;;
	}

// -----------------------------------------------------------
//    special case: 1 point (= constant value), but not HOLD
// -----------------------------------------------------------
	if ((wfm->nPoints == 1) && (wfm->type != WFM_TYPE_HOLD)) {
		// -----------------------------------------------------------
		//    check if sufficient memory
		// -----------------------------------------------------------
		OUTPUTDATA_checkMemoryForAOValues(out, ch, wfm->DIG_NValues);
		// -----------------------------------------------------------
	    //    add new value to list
	    // -----------------------------------------------------------
        out->AO_Values[ch][start] = wfm->DIG_values[0];
        out->AO_ValueDuration[ch][start] = bl->calcDuration;
		out->numAOValues[ch] += 1;
        return bl->calcDuration;;
	}
	

	// -----------------------------------------------------------
    //    include the last value, Repeat[N]=0)
    // -----------------------------------------------------------
	if (wfm->durationThisRepetition_50ns == maxLength) valuesToCopy ++;

// -----------------------------------------------------------
//   special case "HOLD": repeat last value from analog sequence
// -----------------------------------------------------------
	if (wfm->type == WFM_TYPE_HOLD) {
	    out->AO_ValueDuration[ch][out->numAOValues[ch]-1] += valuesToCopy * deltaT;
	    return maxLength;
	}

	// -----------------------------------------------------------
    //    copy digitized values
    // -----------------------------------------------------------

	
	i = 0;
	OUTPUTDATA_checkMemoryForAOValues(out, ch, valuesToCopy);

	if (wfm->DIG_values == NULL) {
	    Breakpoint();
		out->AO_ValueDuration[ch][out->numAOValues[ch]-1] += bl->calcDuration;
	    return bl->calcDuration;
	};
	
    
	while (valuesToCopy > 0) {
//		tmp = wfm->DIG_values[i];
        if (i < wfm->DIG_NValues) out->AO_Values[ch][start + i] = wfm->DIG_values[i];
			else out->AO_Values[ch][start + i] = wfm->DIG_values[wfm->DIG_NValues-1];
/*		if ((valuesToCopy == 2) && (wfm->positionInList == 77) && (bl->blockNr == 25)) {
			Breakpoint();	
		}
*/        if ((wfm->DIG_repeat[i] > valuesToCopy) || (i >= wfm->DIG_NValues)) {
			// -----------------------------------------------------------
		    //    interrupt waveform
		    // -----------------------------------------------------------
           out->AO_ValueDuration[ch][start + i] = valuesToCopy * deltaT;
           valuesToCopy = 0;
        }
        else {
			// -----------------------------------------------------------
		    //    copy all values
		    // -----------------------------------------------------------
			out->AO_ValueDuration[ch][start + i] = wfm->DIG_repeat[i] * deltaT;
			valuesToCopy -= Max(1, wfm->DIG_repeat[i]);
        };
        i ++;
	}
	// -----------------------------------------------------------
    //    record number of appended values
    // -----------------------------------------------------------
	out->numAOValues[ch] += i;
	// -----------------------------------------------------------
    //    return length of appended waveform
    // -----------------------------------------------------------
	return maxLength;

}





void OUTPUTDATA_displayAllAnalogData (t_outputData *o)
{
	//t_digitalEvent *e;
	int i;
	//unsigned long etime;
	int ch;
	
	#ifndef _CVI_DEBUG_
		return;
	#endif
	DEBUGOUT_printf("\n----- AnalogData:\n");
	
	for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
		
		DEBUGOUT_printf ("Channel %d (%d values):\n", ch, o->numAOValues[ch]);
		for (i = 0; i < o->numAOValues[ch]; i++) {
			if (o->numAOValues[ch] > 20) {
				if (i == 8) i = o->numAOValues[ch]-8;
			}
			DEBUGOUT_printf ("%3d: %05d (%5.3f) REPEAT: %10d (%8.3f ms)\n", 
						 i, o->AO_Values[ch][i], DigitalToAnalog (o->AO_Values[ch][i],o->digParameters), 
						 o->AO_ValueDuration[ch][i], (double) (o->AO_ValueDuration[ch][i]) / VAL_ms);
		}
	}
	
	DEBUGOUT_printf ("Timebase: %3.1f us; Duration: %3.2f, TotalSamples: %I64u", (double) o->AOdeltaT /  VAL_us, 
				 ui64ToDouble (o->duration/VAL_ms), o->AO_totalSamples);
}


char *sampleStr (t_analogSample *s) 
{
	static char help[200];
	char h[20];	
	int i;
	
	help[0] = 0;
	for (i = 0; i < MAX_CHANNEL_VECT; i++) {
		sprintf (h, "%05d ", s->value[i]);
		strcat (help, h);
	}
	return help;
}



void OUTPUTDATA_displayAllAnalogSamples (t_outputData *o, int device)
{
	//t_digitalEvent *e;
	int i;
	
	#ifndef _CVI_DEBUG_
		return;
	#endif
	DEBUGOUT_printf ("\n----- Analog Samples (device=%d):\n", device);
	
	for (i = 0; i < o->AO_nSamples[device]; i++) {
		if (o->AO_nSamples[device] > 20) {
			if (i == 8) i = o->AO_nSamples[device]-8;
		}
		
		DEBUGOUT_printf ("%3d: %s REPEAT: %10d (%8.3f ms)\n", 
						 i, sampleStr (&o->AO_Samples[device][i]),
//					 		o->AO_Values[ch][i], DigitalToAnalog (o->AO_Values[ch][i],p), 
						 o->AO_SampleRepeat[device][i], (double) (o->AO_SampleRepeat[device][i])*(double)o->AOdeltaT / VAL_ms);
		}
	DEBUGOUT_printf ("Timebase: %3.1f us; Duration: %3.2f, TotalSamples: %I64u", (double) o->AOdeltaT /  VAL_us, 
				 ui64ToDouble (o->duration/VAL_ms), o->AO_totalSamples);
}



void OUTPUTDATA_displayAllDigitalData (t_outputData *o)
{
	//t_digitalEvent *e;
	int i;
	//unsigned long etime;
	
	#ifndef _CVI_DEBUG_
		return;
	#endif
	DEBUGOUT_printf ("\n----- DigitalData:\n");
	for (i = 0; i < o->numDIOValues; i++) {
		DEBUGOUT_printf ("%3d: %08x %08x, REPEAT: %6d (%3.1f ms)\n", i, o->DIO_Values[0][i], o->DIO_Values[1] == NULL ? 0 : o->DIO_Values[1][i], 
					 o->DIO_Repeat[i], (double) (o->DIO_Repeat[i]*o->DIOdeltaT) / VAL_ms);
	}
	
	DEBUGOUT_printf ("Timebase: %3.1f us; Duration: %3.2f, TotalSamples: %I64u", (double) o->DIOdeltaT /  VAL_us, 
				 ui64ToDouble (o->duration/VAL_ms), o->DIO_totalSamples);
}





void OUTPUTDATA_displayBuffer (unsigned long *buffer, unsigned long bufSize)
{
	unsigned long i;
	unsigned long oldStart;
	unsigned long oldVal;
	
	DEBUGOUT_printf ("\n----- Buffer %08x (size: %d)\n", buffer, bufSize);
	if (bufSize == 0) return;
	oldVal = buffer[0];
	oldStart = 0;
	for (i = 0; i < bufSize; i++) {
		if (buffer[i] != oldVal) {
			DEBUGOUT_printf ("%08x, %6d times\n", oldVal, i - oldStart);
			oldVal = buffer[i];
			oldStart = i;
		}
	}
	if (i != oldStart) DEBUGOUT_printf ("%08x, %6d times\n", oldVal, i - oldStart);

}




void OUTPUTDATA_displayAllDigitalEvents (t_outputData *o)
{
	t_digitalEvent *e;
	int i;
	unsigned long eTime, diff, oldTime;
	
	#ifndef _CVI_DEBUG_
		return;
	#endif
	DEBUGOUT_printf ("\n----- DigitalEvents:\n");
	oldTime = 0;
	for (i = 1; i <= ListNumItems (o->lDigitalEvents); i++) {
		ListGetItem (o->lDigitalEvents, &e, i);
		eTime =(unsigned long) e->time;
		diff = (eTime - oldTime) / o->DIOdeltaT;
		DEBUGOUT_printf ("%3d: %08x %08x, REPEAT:%6d (time=%I64d, %3.2f ms)\n", i, e->digValue[0], e->digValue[1], e->duration / o->DIOdeltaT, e->time, ui64ToDouble (e->time)/20000.0);
		oldTime = eTime;
	}
}



//=======================================================================
//
//   calculate all digital values
//
//   return value:  0  = all time intervals ok
//				   > 0 number of the erroneous block	
//
//=======================================================================
int OUTPUTDATA_createListOfDigitalEvents (t_sequence *seq)
{
    unsigned long index;
    unsigned i;
    t_outputData *o;
    t_digitalBlock *k;
    int invert;
    unsigned bit = 0; // TODO check if zero is ok
    //unsigned andVal;
	int maxDevices = 1;
	int device;
	unsigned int DIG_values[MAX_DIO_DEVICES];
	int nBlocks;
	t_digitalEvent *e;
   
    o = seq->outData;
	
	OUTPUTDATA_freeDigitalEvents (o);
	
	// -----------------------------------------------------------
    //    allocate memory for digital values
    // -----------------------------------------------------------
    nBlocks = SEQUENCE_effectiveNumberOfBlocks (seq);
	if (nDIOBoards() == 0) {
		if (seq->maxDigitalChannel > N_BOARD_DIO_CHANNELS) maxDevices = 2;
		else maxDevices = 1;
	}
	else maxDevices = nDIOBoards();
	
	ListPreAllocate (o->lDigitalEvents, nBlocks + 50);
/*	for (device = 0; device < maxDevices; device++) {
		o->DIO_Values[device] = (unsigned long *) malloc (o->numDIOValues * sizeof(unsigned long));
	}
	o->DIO_Repeat = (unsigned long *) malloc (o->numDIOValues* sizeof(unsigned long));
*/
	// -----------------------------------------------------------
    // 	  init counters
    // -----------------------------------------------------------
	index  = 0;
	SEQUENCE_initBlockCount (seq);
	k = DIGITALBLOCK_ptr (seq, 1);
	invert = config->invertDigitalOutput;
	
   	o->duration = 0;
    while ((index < nBlocks) && (k != NULL)) {
		// -----------------------------------------------------------
		//      if block duration does NOT match timebase,
	    // 		return number of block 
		// -----------------------------------------------------------
	    if (((k->calcDuration % o->DIOdeltaT) != 0)) {
	        return index+1;
	    }
			// -----------------------------------------------------------
		   	//      if block duration == 0
		   	//  	don't include it into the list of values     
			// -----------------------------------------------------------
	    if (k->calcDuration != 0) {
			// -----------------------------------------------------------
		    // 		calculate DIO 32 bit value
			// -----------------------------------------------------------
			for (device = 0; device < MAX_DIO_DEVICES; device++) {
				DIG_values[device] = 0;
			}
			for (i = 0; i < seq->maxDigitalChannel; i++) {
	    		device = i / N_BOARD_DIO_CHANNELS;
	    		switch (seq->DIO_channelStatus[i]) {
	    			case DIO_CHANNELSTATUS_STANDARD: 
	    				if (k->varyPulse[i]) {
		    				if (index == 0) bit = 0;
	    				} else {
							bit = k->channels[i];
						}
						bit ^= (k->alternate[i] && (seq->repetition % 2 == 1));
						bit ^= seq->DIO_invertDisplay[i];
	    				break;
	    			case DIO_CHANNELSTATUS_ALWAYSON:
	    				bit = 1 ^ seq->DIO_invertDisplay[i];
	    				break;
	    			case DIO_CHANNELSTATUS_ALWAYSOFF:
	    				bit = 0 ^ seq->DIO_invertDisplay[i];
	    				break;
	    			default:
						bit = 0;
	    		}
				// TODO check here that device < MAX_DIO_DEVICES ?!
				DIG_values[device] += ((1 & bit) << i);
		    }
			for (device = 0; device < min(MAX_DIO_DEVICES,maxDevices); device++) {
		    	if (invert) DIG_values[device] ^= 0xFFFFFFFF;
			}

			e = DIGITALEVENT_new (o->lDigitalEvents, o->duration, k->calcDuration, DIG_values);
			// -----------------------------------------------------------
			// 	   calculate duration of sequence
			// -----------------------------------------------------------
			o->duration += (unsigned __int64) k->calcDuration;
			// -----------------------------------------------------------
			// 	   add to total values
			// -----------------------------------------------------------
	    	index ++;
	    }
		// -----------------------------------------------------------
	    //     to next block
		// -----------------------------------------------------------
	    k = DIGITALBLOCK_getNext (k);
	}
	
	// insert variable pulses
	OUTPUTDATA_insertVariablePulsesIntoEventList (seq);
	return 0;
	
}





int OUTPUTDATA_calculateDigitalValues (t_sequence *seq) 
{
	//int i;
	int errBlk;
	t_digitalBlock *b;
	double duration = 0;
	double timebase;

	//for (i = 0; i < 10; i++) DEBUGOUT_printf ("\n");
//	DEBUGOUT_printf ("CALCULATE DIGITAL VALUES:\n");
	
    errBlk = OUTPUTDATA_createListOfDigitalEvents (seq);
	if (errBlk != 0) {
		b = DIGITALBLOCK_ptr (seq, errBlk);
		if ((b != NULL) && (!b->disable)) duration = b->duration / (1.0 * VAL_ms);
		timebase = seq->DIO_timebase_50ns / (1.0 * VAL_us);
		PostMessagePopupf ("Error!", "Error in DIGITAL BLOCK #%d:\n\nDuration %3.4f ms is no integer multiple of hardware timebase (%3.1f µs).",
					   errBlk, duration, timebase);
		return -1;
	}
//	OUTPUTDATA_displayAllDigitalEvents (seq->outData);
	
	if (OUTPUTDATA_convertEventListIntoDIOValues (seq->outData) != 0) return -1;
	
//	OUTPUTDATA_displayAllDigitalData (seq->outData);
	
	return 0;
}




int	OUTPUTDATA_adjustLengthOfSequence (t_sequence *seq)
{
	//unsigned long nTicks;
	int ch;
	//unsigned long deltaTmax;
	t_outputData *o;
	int redo;
	int nIter = 0;
	
	o = seq->outData;
	

	if (o->AO_totalSamples < ANALOG_MIN_SAMPLES) o->AO_totalSamples = ANALOG_MIN_SAMPLES;
	
	o->AO_totalSamples += xMod64 (o->AO_totalSamples, 16);
	o->DIO_totalSamples  += xMod64 (o->AO_totalSamples, 16);
	
	
	
	
	if (o->DIO_dblBuf || o->AO_dblBuf) return 0;
	
	return 0;
	
	for (ch = 0; ch < N_DAC_CHANNELS; ch++) {
//		    addDuration[ch] = o->DIOdeltaT;
		if (o->numAOValues[ch] != 0)
		   o->AO_ValueDuration[ch][o->numAOValues[ch]-1] += o->DIOdeltaT;
	}
	
	// make length of both equal
	o->duration += xMod (o->duration, o->AOdeltaT);
	o->duration += xMod (o->duration, o->DIOdeltaT);


	nIter = 0;
	do {
		redo = 0;
		o->DIO_totalSamples = o->duration / o->DIOdeltaT;
		////////////////////////////////////////previously xmod (,4)/////
	//	o->DIO_totalSamples += MI6021_NSAMPLESBEFORETRIGG;
		o->DIO_totalSamples += xMod (o->DIO_totalSamples, 16);
		/////////////////////////////////////////////////////////////////
		o->duration = o->DIO_totalSamples * o->DIOdeltaT;
		// make sure that the number of samples for the analog board
		// is even
		if ((o->duration / o->AOdeltaT) % 2 != 0) {
			o->duration+= o->AOdeltaT;
			redo = 1;
		}
		o->AO_totalSamples = o->duration / o->AOdeltaT;
		nIter++;
		if (nIter > 20) {
			PostMessagePopupf ("Error.", "Illegal length of sequence.");
			return -1;
		}
	} while (redo);

//	if (o->DIO_totalSamples % 4 != 0) Breakpoint();


	return 0;
}



//=======================================================================
//    initialize channel vectors
//=======================================================================
void OUTPUTDATA_initializeChannelVectors (t_sequence *seq) 
{
	int i;
	short ch;
	t_outputData *o;
	
	o = seq->outData;
	
	for (i = 0; i < MAX_AO_DEVICES; i++) {
		o->AOnChannels[i] = 8;
		for (ch = 0; ch < 8; ch++) o->AOchannelVect[i][ch] = ch;
	}
}





int OUTPUTDATA_calculate_DACchannels (t_sequence *seq, int simulate)
{
    int ch; 
    unsigned long duration[N_DAC_CHANNELS] = {0};
    t_digitalBlock *p;  
    t_outputData *o;
	int inLoop;
	int loopsToDo;
	int loopStartNr;
	unsigned __int64 absoluteT = 0;
	unsigned __int64 triggerCycle;
	int device;
	int nChannels, nDevices;
	int triggerTimeIndex;
	int i;


	o = seq->outData;
	
    for (ch = 0; ch < N_DAC_CHANNELS; ch++) duration[ch] = 0;
	nChannels = N_DAC_CHANNELS;
	//else nChannels = seq->maxAnalogChannels;
	
	

	o->triggerTimesDuringSeq = (long *) malloc (sizeof(long) * (o->nTriggerTimesDuringSeq + 1));
	for (i = 0; i < o->nTriggerTimesDuringSeq+1; i++) o->triggerTimesDuringSeq[i] = -1;
	triggerTimeIndex = 0;
	
	

	
    p = DIGITALBLOCK_ptr (seq, 1);
    while (p != NULL) {
       // check if analog block  does something weird
       for (ch = 0; ch < nChannels; ch++) {
	       if (duration[ch] == 0) {
				// -----------------------------------------------------------
				// 	append analog values to output data
			    // -----------------------------------------------------------
			    if (p->wfmP[ch] != NULL) {
					// -----------------------------------------------------------
					//  check for error
				    // -----------------------------------------------------------
			    	if (absoluteT % o->AOdeltaT != 0) {
            			PostMessagePopupf ("Error", "Error in DIGITAL BLOCK #%d: Invalid time interval.\n\n"
            									"Analog channels can only be changed at times which are\n"
            									"integer multiples of the analog hardware timebase (%s).\n",
            									p->blockNr, strTime(o->AOdeltaT));
            			return -1;
			    	}
		    	
			    }

			    inLoop      = seq->inLoop;
				loopsToDo   = seq->loopsToDo;
				loopStartNr = seq->loopStartNr;
	            duration[ch] = OUTPUTDATA_appendBlockAOValues (o, p, ch, seq->repetition, seq->nRepetitions);
			    seq->inLoop       = inLoop;
				seq->loopsToDo   = loopsToDo;
				seq->loopStartNr = loopStartNr;
	       }
	       if (p->calcDuration <= duration[ch]) {
				// -----------------------------------------------------------
				// 	analog block exceeds length of digital block
				//  --> go to next digital block
			    // -----------------------------------------------------------
	           duration[ch] = duration[ch] - p->calcDuration;
	       }
	       else {
				// -----------------------------------------------------------
				// 	just increase #repetitions of last value
				// -----------------------------------------------------------
	           	o->AO_ValueDuration[ch][seq->outData->numAOValues[ch]-1] += 
	           	    (p->calcDuration - duration[ch]);
	           	duration[ch] = 0;
		   }
	   }
	   if (p->waitForExtTrigger) {
			if (triggerTimeIndex < o->nTriggerTimesDuringSeq) {
				triggerCycle = absoluteT / o->DIOdeltaT;
				o->triggerTimesDuringSeq[triggerTimeIndex] = triggerCycle;
				triggerTimeIndex++;
			}				
	   }
   	   absoluteT += p->calcDuration;
	   p = DIGITALBLOCK_getNext(p);
	}
	
	
	// -----------------------------------------------------------
    // 	 calculate number of analog values 
    // -----------------------------------------------------------
    o->AO_totalSamples     = o->duration / o->AOdeltaT;
	o->AO_totalSamples    *= seq->nCopies;
	
//	DEBUGOUT_initPanel ();
//	OUTPUTDATA_displayAllAnalogData (o);


	if (config->hardwareType == HARDWARE_TYPE_NATINST) {
		// -----------------------------------------------------------
	    // 	 convert values into 2*8 byte samples
	    // -----------------------------------------------------------
		if (simulate) nDevices = MAX_AO_DEVICES;
		else nDevices = nAOBoards ();
		for (device = 0; device < nDevices; device++) {
			OUTPUTDATA_generateAnalogSamplesFromValues (o, device);
//			OUTPUTDATA_displayAllAnalogSamples (o, device);		
		}
	}

	
	return 0;
}	



//=======================================================================
//
//   calculate output data
//
//   (generate output data, store in seq-outData)
//
//=======================================================================

void OUTPUTDATA_setConfigCounter (t_sequence *seq)
{

}


void OUTPUTDATA_ADWIN_initTriggerSources (t_sequence *seq)
{
	if ((seq->trigger == ADWIN_TRIGGER_DIGIO0) || (seq->trigger == ADWIN_TRIGGER_DIGIO1)) 
		seq->outData->adwinData->trigger = seq->trigger;
	else
		seq->outData->adwinData->trigger = VAL_NO_TRIGGER;
}




void OUTPUTDATA_ADWIN_calculateGPIBCommandsDuringSequence (t_sequence *seq)
{
	t_adwinData *a;
	t_digitalBlock *b;
	int n, i, j;
	__int64 cycleNo;
	
	
	a = seq->outData->adwinData;
	a->nGpibCommands = SEQUENCE_getNumberOfGpibCommandsDuringSequence (seq);
	if (a->nGpibCommands == 0) return;
	
	a->gpibCommandPtr = (void **) malloc (sizeof (void*) * a->nGpibCommands);
	a->gpibCommandExecutionCycle = (int *) malloc (sizeof (int) * a->nGpibCommands);

	n = ListNumItems (seq->lDigitalBlocks);
	j = 0;
	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (b->gpibCommandNr > 0 && !b->disable && seq->outData != NULL && seq->outData->DIOdeltaT != 0) { // only execute gbip commands in enabled fields
			if (j >= a->nGpibCommands) {
				tprintf("Number of gpib commands unclear\n");
				break;
			}
			a->gpibCommandPtr[j] = GPIBCOMMAND_ptr (seq, b->gpibCommandNr);
			cycleNo = b->absoluteTime / seq->outData->DIOdeltaT; 	
			a->gpibCommandExecutionCycle[j] = (int)cycleNo;			 //this is save unitl sequence times of at least 3000s with our current smallest times steps of 2us
			j++;
		}
	}
	a->nGpibCommands = j;
	
}


void OUTPUTDATA_ADWIN_displayGPIBCommandsDuringSequence (t_adwinData *a)
{
	
	int i;
	t_gpibCommand *g;
	
	DebugPrintf ("GPIB commands during sequence:\n");
	for (i = 0; i < a->nGpibCommands; i++) {
		g = (t_gpibCommand *) a->gpibCommandPtr[i];
		DebugPrintf ("command '%s' at cycleNo %d\n", g->name, a->gpibCommandExecutionCycle[i]);
	}

}



//=======================================================================
//
//   calculate output data
//
//   (generate output data, store in seq-outData)
//
//=======================================================================
int OUTPUTDATA_calculate (t_sequence *seq, int firstRepetition, int simulate)//, int calculateLeCroyWaveformsOnly)
{
    
    int errorBlock;
    int sleepPolicy;
    t_outputData *o;
    //unsigned nTicks;
	clock_t startTime;
	
	startTime = clock ();
	
	if (!config->suppressProtocol){ 
		tprintf ("\nRepetition %d: calculating output... (memory=%1.2f MB)  ", (seq->repetition)+1, freePhysicalMemory());
		ProcessDrawEvents();
	}
    
	sleepPolicy = GetSleepPolicy ();
	SetSleepPolicy (VAL_SLEEP_NONE);
//	pprintf (panelMain(), MAIN_TEXTBOX_Output2, "Repetition %5d: calculating output values...", (seq->repetition)+1);
	// -----------------------------------------------------------
    //    delete old data & init 
    // -----------------------------------------------------------
	
    if (seq->outData == NULL) {
    	seq->outData =  (t_outputData *) malloc (sizeof (t_outputData));
    	OUTPUTDATA_init (seq->outData, 1);
    }
    OUTPUTDATA_free (seq->outData, firstRepetition);

    o = seq->outData;
	o->simulate = simulate;
    OUTPUTDATA_init (o, firstRepetition);
	OUTPUTDATA_getTimebases (seq);
	o->nActiveDACChannels = N_DAC_CHANNELS;
	// -----------------------------------------------------------
    //   insert a number into each digital block 
   	//   recalculate all links to waveforms etc
    // -----------------------------------------------------------
    o->variableOutput = SEQUENCE_insertBlockNumbersAndReferences (seq, &o->nTriggerTimesDuringSeq);
	// -----------------------------------------------------------
    //    calculate timebase 
    //    (check if analog timebase is set correctly, i.e.
    //     digital timebase is integer multiple of analog timebase)
    // -----------------------------------------------------------
    o->nCopies      = seq->nCopies;
	// -----------------------------------------------------------
    //    calculate digital values
    //    (check for invalid time intervals)
    // -----------------------------------------------------------
    SEQUENCE_initBlockCount (seq);
    if ((errorBlock = OUTPUTDATA_calculateDigitalValues (seq)) != 0) {
 		goto ERROR123;
    }
	
	OUTPUTDATA_initializeChannelVectors (seq);

//=======================================================================
//    calculate data for DAC channels
//=======================================================================
   	SEQUENCE_digitizeAllWaveforms (seq, 0, N_DAC_CHANNELS, simulate);
    SEQUENCE_initBlockCount (seq);
    
	if (OUTPUTDATA_calculate_DACchannels (seq, simulate) != 0) goto ERROR123;

//=======================================================================
// 	add repetitions of last value (if timebases don't match)
//=======================================================================
	if (OUTPUTDATA_adjustLengthOfSequence (seq) != 0) goto ERROR123;
	
	if (config->hardwareType == HARDWARE_TYPE_ADWIN) {
		if (OUTPUTDATA_ADWIN_createData (seq, o) != 0) goto ERROR123;
		OUTPUTDATA_ADWIN_initTriggerSources (seq);
		OUTPUTDATA_ADWIN_calculateGPIBCommandsDuringSequence (seq);
		OUTPUTDATA_ADWIN_displayGPIBCommandsDuringSequence (o->adwinData);
	}
	
	OUTPUTDATA_setConfigCounter (seq);
	
	if (!config->suppressProtocol) ProcessDrawEvents ();

	if (o->DIO_Values == NULL) goto ERROR123;
	SetSleepPolicy (sleepPolicy);
	
	
	if (!config->suppressProtocol) {
		tprintf ("[%2.3f s]\n", timeStop_s (startTime));
	}
	
	
	return 0;
	
ERROR123:
    OUTPUTDATA_free (seq->outData, 1);
	SetSleepPolicy (sleepPolicy);
	return -1;
}





void OUTPUTDATA_ADWIN_displayAnalogValue (int nr, long timestamp, long v)
{
	long value;
	unsigned int channel;
	unsigned int module;
	
	value = v >> 8;
	module = (v & 0xFF) >> 4;
	channel = (v & 0xF);
	
	DEBUGOUT_printf ("%8d: %8d µs: %6ld (mod=%d,ch=%d)\n", nr, timestamp, value, module, channel);
}




void OUTPUTDATA_ADWIN_displayAnalogValues (t_adwinData *a)
{
	int n, i, start;
	
#ifndef _CVI_DEBUG_
		return;
#endif
	
	n = min (a->nAOelements/2, 100);
	
	DEBUGOUT_printf ("%d analog values\n", a->nAOelements/2);
	
	for (i = 0; i < n; i++) {
		OUTPUTDATA_ADWIN_displayAnalogValue (i, a->AObuffer[i*2], a->AObuffer[i*2+1]);
	}
	

	start = a->nAOelements/2 - 50;
	if (start < n) start = n;

	for (i = start; i < a->nAOelements/2; i++) {
		OUTPUTDATA_ADWIN_displayAnalogValue (i, a->AObuffer[i*2], a->AObuffer[i*2+1]);
	}
}




void OUTPUTDATA_ADWIN_displayDigitalValue (int nr, long timestamp, long *values)
{
	unsigned long i;

#ifndef _CVI_DEBUG_
		return;
#endif
	
	DEBUGOUT_printf ("%8d: %8d µs:", nr, timestamp);
	for (i = 0; i < config->nDigitalBoards; i++) DEBUGOUT_printf ("%8x ", values[i]);
	DEBUGOUT_printf ("\n");
	
}




void OUTPUTDATA_ADWIN_displayDigitalValues (t_adwinData *a)
{
	int n, i, start;
	int nTotalElements;
	int mult;
	
	mult = config->nDigitalBoards+1;
	nTotalElements = a->nDIOelements / mult;

	n = min (nTotalElements, 50);
	
	DEBUGOUT_printf ("\n\n%d digital timestamps \n", nTotalElements);
	
	for (i = 0; i < n; i++) {
		OUTPUTDATA_ADWIN_displayDigitalValue (i, a->DIObuffer[i*mult], &a->DIObuffer[i*mult+1]);
	}
	

	start = nTotalElements  - 80;
	if (start < n) start = n;

	for (i = start; i < nTotalElements; i++) {
		OUTPUTDATA_ADWIN_displayDigitalValue (i, a->DIObuffer[i*mult], &a->DIObuffer[i*mult+1]);
	}
}






int OUTPUTDATA_ADWIN_generateDigitalDataFromValues (t_outputData *o)
{
	t_adwinData *a;
	//unsigned int nElements;
	unsigned long i, j, index;
	long timestamp;

	
	a = o->adwinData;
	if (a == NULL) return -1;
	
	
	
	timestamp = 0;
	a->nDIOelements = o->numDIOValues * (config->nDigitalBoards + 1) ;
	a->DIObuffer = (long *) malloc (sizeof(long) * a->nDIOelements);
	
	for (i = 0; i < o->numDIOValues; i++) {
		if (o->DIO_Repeat[i] == 0) {
			PostMessagePopupf ("Error!", "Error in function 'OUTPUTDATA_ADWIN_generateDigitalDataFromValues'\nDuration for digital value is zero!");
			return -1;
		}
		
		// set timestamp
		index = i * (config->nDigitalBoards + 1);
		a->DIObuffer[index] = timestamp;
		timestamp += o->DIO_Repeat[i];

		// copy values
		for (j = 0; j < config->nDigitalBoards; j++) {
			a->DIObuffer[index+j+1] = o->DIO_Values[j][i];
		}
	}
	
	for (i = 0; i < MAX_DIO_DEVICES; i++) {
		free (o->DIO_Values[i]);
		o->DIO_Values[i] = NULL;
	}
	

	return 0;
}







int OUTPUTDATA_ADWIN_generateAnalogDataFromValues (t_outputData *o)
{
	int i,j,k,n;
	int valueIndex[N_DAC_CHANNELS];
	unsigned int nElements;
	unsigned int sampleIndex;
	t_adwinData *a;
	long timestamp, nextTimestamp;
	int repeat;
	int startIndex, endIndex;
	long timestampNext[N_DAC_CHANNELS];
	long timestampChannel[N_DAC_CHANNELS];
	__int64 h64;
	
	a = o->adwinData;
	if (a == NULL) return -1;
	
	nElements = 0;
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		nElements += o->numAOValues[i];
		valueIndex[i] = 0;
		timestampNext[i] = -1;
		timestampChannel[i] = -1;
	}
	if (nElements == 0) return 0;
	nElements = (nElements + 256) * 2;
	nElements = nElements + (nElements % (ADWIN_NCOPY_ELEMENTS*2));  
	
	a->nAOelements = 0;
//	if (nElements*4 > ADWIN_MAX_AO_BUFFERSIZE) {
//		PostMessagePopup ("Error", "Analog data exceeds buffer size!");
//		return -1;
//	}

	a->AObuffer = (long *) malloc (sizeof(long)*nElements);
	sampleIndex = 0;
	timestamp = 0;

	
//	OUTPUTDATA_displayAllAnalogData (o);
	
	// put first values
	for (i = 0; i < o->nActiveDACChannels; i++) {
		do {
			nextTimestamp = o->AO_ValueDuration[i][valueIndex[i]] / ADWIN_TIMEBASE_DIGITAL;
			repeat = (nextTimestamp == 0);
			if (repeat) {
				repeat = (valueIndex[i] < o->numAOValues[i]-1) ;
				if (repeat) valueIndex[i]++;
			}
		} while (repeat);
		a->AObuffer[sampleIndex] = 0;
		a->AObuffer[sampleIndex+1] = ((long) o->AO_Values[i][valueIndex[i]] << 8) + (config->analogBoardID[i/8] << 4) + ((i % 8)+1);
//		DebugPrintf ("%8d: %8d: value = %d\n", sampleIndex, a->AObuffer[sampleIndex], o->AO_Values[i][valueIndex[i]] );
		sampleIndex += 2;
	
		j = 0;
		while ((j < i) && (timestampNext[j] <= nextTimestamp)) j++;
		// insert after element j			
		for (k = i; k > j; k--) {
			timestampNext[k] = timestampNext[k-1];
			timestampChannel[k] = timestampChannel[k-1];
		}
		// insert at element j			
		timestampNext[j] = nextTimestamp;
		timestampChannel[j] = i;
		if (valueIndex[i] < o->numAOValues[i]-1) valueIndex[i]++;
		
	}
		
	timestamp = timestampNext[0];
	startIndex = 0;
	while (startIndex < o->nActiveDACChannels) {
		i = timestampChannel[startIndex];
		do {
			// jump over values with 0 duration
			repeat = (o->AO_ValueDuration[i][valueIndex[i]] == 0);
			if (repeat) {
				repeat = (valueIndex[i] < o->numAOValues[i]-1) ;
				if (repeat) valueIndex[i]++;
			}
		} while (repeat);
		a->AObuffer[sampleIndex] = timestampNext[startIndex];
		a->AObuffer[sampleIndex+1] = ((long) o->AO_Values[i][valueIndex[i]] << 8) + (config->analogBoardID[i/8] << 4) + ((i % 8)+1);
//		DebugPrintf ("%8d: %8d: value = %d\n", sampleIndex, a->AObuffer[sampleIndex], o->AO_Values[i][valueIndex[i]] );
		
		if (sampleIndex < nElements) sampleIndex += 2;
		else return -1;

		nextTimestamp = timestampNext[startIndex] + o->AO_ValueDuration[i][valueIndex[i]] / ADWIN_TIMEBASE_DIGITAL;
		
		if (valueIndex[i] < o->numAOValues[i]-1) {
			// more values in list -->insert
			valueIndex[i]++;
			// insert new value
			j = startIndex+1;
			while ((j < o->nActiveDACChannels) && (timestampNext[j] < nextTimestamp)) j++;
			// insert after element j			
			for (k = startIndex+1; k < j; k++) {
				timestampNext[k-1] = timestampNext[k];
				timestampChannel[k-1] = timestampChannel[k];
			}
			// insert at element j			
			timestampNext[j-1] = nextTimestamp;
			timestampChannel[j-1] = i;
		}
		else {
			// all values inserted --> take next value in list
			startIndex++;
		}			  
	}		

	h64 = o->duration / ADWIN_TIMEBASE_DIGITAL;
	a->maxCycleNo =(long) h64;  //for current time steps of 2us long is by fare enough
	endIndex = min (sampleIndex + (sampleIndex % (ADWIN_NCOPY_ELEMENTS*2)), nElements);
	for (i = sampleIndex; i < endIndex; i++) {
		a->AObuffer[i] = 0;
	}
	a->nAOelements = endIndex;

	
	// insert last AO_values
	for (i = 0; i < N_DAC_CHANNELS; i++) {
		n = o->numAOValues[i]-1;
		if (n < 0) a->AOvaluesAtEndOfSequence[i] = AnalogToDigital (0.0, digitizeParameters_ADWIN ());
		else a->AOvaluesAtEndOfSequence[i] = o->AO_Values[i][n];
	}
		
	return 0;
}
	


int OUTPUTDATA_ADWIN_generateTriggerTimes (t_outputData *o)
{
	t_adwinData *a;
	
	
	
	a = o->adwinData;
	if (a == NULL) return -1;

	
	a->triggerTimesDuringSeq = o->triggerTimesDuringSeq;
	a->nTriggerTimesDuringSeq = o->nTriggerTimesDuringSeq;
	// set last value to -1
	a->triggerTimesDuringSeq[a->nTriggerTimesDuringSeq] = -1;

	
	o->triggerTimesDuringSeq = NULL;
	o->nTriggerTimesDuringSeq = 0;
	return 0;	
}



int OUTPUTDATA_ADWIN_initPhaseFeedback (t_sequence *seq)
{
	t_waveform *wfm1, *wfm2;
	t_adwinData *a;
	t_outputData *o;
	char suffix[30];
	
	wfm1 = (t_waveform *) seq->phaseFeedbackWfm[0];
	if (wfm1 == 0) return 0;
	wfm2 = (t_waveform *) seq->phaseFeedbackWfm[1];
	if (wfm2 == 0) {
		tprintf ("Error! NO second waveform selected for phase feedback!\n");
		return 0;
	}
	o = seq->outData;
	if (o == NULL) return -1;
	
	a = o->adwinData;
	if (a == NULL) return -1;
	a->phaseFeedbackWfm[0] = seq->phaseFeedbackWfm[0];
	a->phaseFeedbackWfm[1] = seq->phaseFeedbackWfm[1];
	
	a->phaseFeedbackActive = wfm1->addrEnablePhaseFeedback;
	a->repetition = seq->repetition+1;
	a->startNr = seq->startNr;

	if (seq->nRepetitions == 1) strcpy (suffix, "");
	else sprintf (suffix, "_%03d", seq->repetition);
	
	sprintf (a->phaseFeedbackImageFilename, "%s\\%03d_%d_subtr%s.sif", seq->currentPath,
		seq->startNr, wfm1->addrCamera, suffix);
	if (wfm1->addrCamera == 0) {
		tprintf("Error in initPhaseFeedback: camera number not set.\n"); 
	}
	
	sprintf (a->phaseFeedbackProtocolFilename, "%s\\%03d_phaseFeedback.txt", 
		seq->currentPath, seq->startNr);
	
	return 0;
	
	
}



int OUTPUTDATA_ADWIN_createData (t_sequence *seq, t_outputData *o)
{
	int err;
	__int64 nCycles;
	
//	DEBUGOUT_initPanel (); 
	if (o->adwinData == NULL) {
		o->adwinData = (t_adwinData *) malloc (sizeof (t_adwinData));	
		ADWINDATA_init (o->adwinData);
	}
	
	nCycles = o->duration/o->DIOdeltaT;
	if (nCycles > ADWIN_MAX_CYCLES) {
		PostMessagePopupf ("Error", "Sequence duration exceeds maximum duration allowed by ADWIN system (%2.2f s)", (1.0*ADWIN_MAX_CYCLES)/1.0E6*(1.0*ADWIN_TIMEBASE_DIGITAL)/(1.0*VAL_us));
		return -1;
	}
	
	ADWINDATA_free (o->adwinData);
	
	err = OUTPUTDATA_ADWIN_generateAnalogDataFromValues (o);
	if (err != 0) return err;
	
	err = OUTPUTDATA_ADWIN_generateDigitalDataFromValues (o);
	if (err != 0) return err;
	
	err = OUTPUTDATA_ADWIN_generateTriggerTimes (o);
	if (err != 0) return err;
	
	o->adwinData->callingSequence = seq;
	
//	err = OUTPUTDATA_ADWIN_initPhaseFeedback (seq);
//	if (err != 0) return err;
	
//	OUTDATA_ADWIN_displayAnalogValues (&o->adwinData);
//	OUTDATA_ADWIN_displayDigitalValues (&o->adwinData);
	
	
	return err;
}



unsigned long OUTPUTDATA_generateAnalogSamplesFromValues (t_outputData *o, int device)
{
	
	unsigned int nElements;
	int k, l;
	t_analogSample *newSample;
	int valueIndex[MAX_CHANNEL_VECT];
	unsigned int *duration;
	unsigned int sampleIndex, totalSamples;
	unsigned int minDuration, minIndex;
	int keepGoing;
	
	long value;
	
//	DEBUGOUT_initPanel ();
	
	// init
	o->AO_nSamples[device] = 0;
	free (o->AO_Samples[device]);
	o->AO_Samples[device] = NULL;
	free (o->AO_SampleRepeat[device]);
	o->AO_SampleRepeat[device] = NULL;
	nElements = 0;
	
	l = device*MAX_CHANNEL_VECT; 
	for (k = 0; k < MAX_CHANNEL_VECT; k++) {
		nElements += o->numAOValues[k + l];
		valueIndex[k] = 0;
	}
	if (nElements == 0) return 0;
	nElements += 64;
	o->AO_Samples[device] = (t_analogSample *) malloc (sizeof (t_analogSample) * nElements);
	o->AO_SampleRepeat[device] = (unsigned long *) malloc (sizeof (unsigned long) * nElements);

	sampleIndex = 0;
	totalSamples = 0;
	keepGoing = 1;
	while (keepGoing) {
		newSample = &o->AO_Samples[device][sampleIndex];
		minDuration = ULONG_MAX;
		minIndex = 0;
		for (k = 0; k < MAX_CHANNEL_VECT; k++) {
			newSample->value[k] = o->AO_Values[k+l][valueIndex[k]];
			value = newSample->value[k]; // DEBUG
			duration = &o->AO_ValueDuration[k+l][valueIndex[k]];
			if ((*duration < minDuration)) {
				minDuration = *duration;
				minIndex = k;
			}
		}
		o->AO_SampleRepeat[device][sampleIndex] = minDuration / o->AOdeltaT;
		totalSamples += o->AO_SampleRepeat[device][sampleIndex];// * o->nCopies;
		if (sampleIndex < nElements-1) sampleIndex ++;
		else {
//			tprintf ("OUT of Memory ERROR123 in 'OUTPUTDATA_generateSamplesFromValues' [Index:%d, nElements:%d]\n", sampleIndex, nElements);
			goto END;
		}
		keepGoing = (minDuration == 0);
		for (k = 0; k < MAX_CHANNEL_VECT; k++) {
			duration = &o->AO_ValueDuration[k+l][valueIndex[k]];
			if (*duration >= minDuration) { 
				*duration = *duration  - minDuration;
				if (*duration == 0) {
					if (valueIndex[k] < o->numAOValues[k+l]-1) {
						valueIndex[k]++;
						keepGoing = 1;
					}
				}
				else keepGoing = 1;
			}
		}
		
//		if (sampleIndex == 4) Breakpoint();
	}
	
	
	// memory cleanup
	o->AO_SampleRepeat[device] = realloc (o->AO_SampleRepeat[device], sizeof (unsigned int)*sampleIndex);
	o->AO_Samples[device] = realloc (o->AO_Samples[device], sizeof (t_analogSample)*sampleIndex);
END:	
	if (sampleIndex > 0) o->AO_nSamples[device] = sampleIndex;
	
//	DEBUG_displaySamples (o, 0);
	
	for (k = 0; k < MAX_CHANNEL_VECT; k++) {
		free (o->AO_ValueDuration[k+l]);
		o->AO_ValueDuration[k+l] = NULL;
		free (o->AO_Values[k+l]);
		o->AO_Values[k+l] = NULL;
		o->numAOValues[k+l] = 0;
	}
	
	return 0;
}




int DIGITALEVENT_getIndexFromTime (ListType list, __int64 start)
{
	t_digitalEvent dummy;	
	t_digitalEvent *dummyPtr;
	int pos;
	
	dummy.time = start;
	dummyPtr = &dummy;
	
	pos = ListBinSearch (list, &dummyPtr, DIGITALEVENT_compare);  	
	return pos;
}



void OUTPUTDATA_deleteDuplicateTimesInEventList (t_outputData *o)
{
	int n, i;
	t_digitalEvent *e, *lastE;
	
	n = ListNumItems (o->lDigitalEvents);
	if (n <= 1) return;
	
	ListGetItem (o->lDigitalEvents, &lastE, 1);
	for (i = 2; i <= n; i++) {
		ListGetItem (o->lDigitalEvents, &e, i);
		if (e->time == lastE->time) {
			ListRemoveItem (o->lDigitalEvents, 0, i-1);
			DIGITALEVENT_free (lastE);
			free (lastE);
			i--;
		}
		lastE = e;
	}
		
}


void OUTPUTDATA_allocateMemoryForDIOValues (t_outputData *o, int nValues) 
{
	int d;
	
	if (nValues > o->numDIOValues) {
		// same number of values (change nothing)
		for (d = 0; d < MAX_DIO_DEVICES; d++) {
			free (o->DIO_Values[d]);
			o->DIO_Values[d] = (unsigned long *) malloc (sizeof (unsigned long) * nValues);
		}
		free (o->DIO_Repeat);
		o->DIO_Repeat = (unsigned long *) malloc (sizeof (unsigned long) * nValues);
	}
	o->numDIOValues = nValues;
	
			
	
}

int OUTPUTDATA_convertEventListIntoDIOValues (t_outputData *o)
{
	int n, i, d;
	t_digitalEvent *e, *lastE;
	__int64 duration;
	__int64 lastTime;
	
	n = ListNumItems (o->lDigitalEvents);
	if (n == 0) return -1;
	
	OUTPUTDATA_allocateMemoryForDIOValues (o, n);
	
	// first item:
	i = 0;
	ListGetItem (o->lDigitalEvents, &e, i+1);
	for (d = 0; d < MAX_DIO_DEVICES; d++) o->DIO_Values[d][i] = e->digValue[d];
	
	o->DIO_Repeat[i] = e->duration / o->DIOdeltaT;

	for (i = 1; i < n; i++) {
		lastE = e;
		ListGetItem (o->lDigitalEvents, &e, i+1);
		for (d = 0; d < MAX_DIO_DEVICES; d++) o->DIO_Values[d][i] = e->digValue[d];
		duration = e->time - lastE->time;
		duration /= o->DIOdeltaT;
		o->DIO_Repeat[i-1] = (unsigned long) duration;
	}
	lastTime = e->time;
	if (lastTime != o->duration) {
//		Breakpoint ();	
		duration = (o->duration - lastTime) / o->DIOdeltaT;
	}
	else {
		duration = 0;
	}
	o->DIO_Repeat[n-1] = (unsigned long) duration;
	
	o->DIO_totalSamples = o->duration/o->DIOdeltaT * o->nCopies;
	o->DIO_totalSamples += xMod64 (o->DIO_totalSamples, 16);
	
	OUTPUTDATA_freeDigitalEvents (o);
	return 0;
}



int OUTPUTDATA_insertVariablePulsesIntoEventList (t_sequence *seq)
{
	t_digitalBlock *b;
	__int64 start, stop;
	//__int64 indexStart, indexStop;
	int mult;
	int i, ch;
	//int k;
	unsigned valOR, valAND;
	t_outputData *out;
	t_digitalEvent *e1, *e2, *eCopy, *e;
	int e1Pos, e2Pos;
	int state;
	int board;
	int n, nEvents;
	int copyPos, index;
	//t_digitalEvent dummy;
	
	out = seq->outData;
//	return 0;
	
	n = ListNumItems(seq->lDigitalBlocks);
	DIGITALBLOCK_calculateAllAbsoluteTimes (seq, seq->repetition); 
	OUTPUTDATA_deleteDuplicateTimesInEventList (out);

	for (i = 1; i <= n; i++) {
		ListGetItem (seq->lDigitalBlocks, &b, i);
		if (!b->disable) {
			for (ch = 0; ch < seq->maxDigitalChannel; ch++) {
				if (b->varyPulse[ch] && (b->blockMode == 0)) {
					if (b-> varyPulseStepRep[ch] < 1) b-> varyPulseStepRep[ch] = 1;
					mult = seq->repetition /  b->varyPulseStepRep[ch];
					start = b->absoluteTime + (__int64) (b->varyPulseStartIncrement[ch] * mult); 
					stop  = b->absoluteTime + (__int64) (b->varyPulseStopIncrement[ch] * mult)
							+ b->duration;
					if (start < 0) start = 0;
					if (stop > out->duration) stop = out->duration;
					if (start < stop) {
						e1Pos = DIGITALEVENT_getIndexFromTime (out->lDigitalEvents, start);
						if (e1Pos == 0) {
						// insert Event with time "start"
							e1 = DIGITALEVENT_new (out->lDigitalEvents, start, 0, 0);
							e1Pos = ListBinSearch (out->lDigitalEvents, &e1, DIGITALEVENT_compare);
							copyPos = e1Pos-1;
							ListGetItem (out->lDigitalEvents, &eCopy, copyPos);
							DIGITALEVENT_copyValues (e1, eCopy);
	//						OUTPUTDATA_displayAllDigitalEvents (out);
						}
				
						// insert Event with time "stop"
						e2Pos = DIGITALEVENT_getIndexFromTime (out->lDigitalEvents, stop);
						if (e2Pos == 0) {
							e2 = DIGITALEVENT_new (out->lDigitalEvents, stop, 0, 0);
							e2Pos = ListBinSearch (out->lDigitalEvents, &e2, DIGITALEVENT_compare);
							nEvents = ListNumItems (out->lDigitalEvents);
							ListGetItem (out->lDigitalEvents, &eCopy, e2Pos-1);
							DIGITALEVENT_copyValues (e2, eCopy);
	//						OUTPUTDATA_displayAllDigitalEvents (out);	
	//						if (e2Pos == nEvents) e2Pos++;
						}
				
						// insert pulse values
						board = ch / N_BOARD_DIO_CHANNELS;
						valOR = (0x00000001 << (ch%N_BOARD_DIO_CHANNELS));
						valAND = 0xFFFFFFFF - valOR;
					    state  = b->channels[ch] ^ config->invertDigitalOutput;
						for (index = e1Pos; index < e2Pos; index++) {
							ListGetItem (out->lDigitalEvents, &e, index);
						    // change values in output buffer
						    if (state == 1) e->digValue[board] |= valOR;
							else e->digValue[board] &= valAND;
						}
					}
				}
			}
		}
	}

	OUTPUTDATA_deleteDuplicateTimesInEventList (out);
	
	return 0;
}








unsigned long OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (t_outputData *o,
			  unsigned long **buffers, unsigned long bufferSizeSamples)
{

   	unsigned long cpyVal32;
    unsigned long cpyVal32b;
    unsigned long i;
    unsigned long samplesToWrite, samplesToCopy;
//    t_outputData *o;
	int secondBoard = 0;
    unsigned __int64 samplesLeft64;
	
//    o = seq->outData;
	if (o == NULL) return 0;
	if (buffers == NULL) return 0;
	if (buffers[0] == NULL) return 0;
	if (o->DIO_Values[0] == NULL) return 0;
	
	if (bufferSizeSamples == 0) return 0;

	samplesLeft64 = o->DIO_totalSamples - o->DIO_totalSamplesWritten;
	samplesToWrite = (unsigned long) min (samplesLeft64, bufferSizeSamples);
//    samplesToWrite = o->DIO_totalSamples;
	// ---------------------------------------------
	//    init valiables
	// ---------------------------------------------
    i 	= 0;
	// ---------------------------------------------
	//    tried to optimize routines for speed
	//    (so they don't look nice)
	// ---------------------------------------------
	secondBoard = (o->DIO_Values[1] != NULL) && (buffers[1] != NULL);
   
    // get number of repetitions
//	repCnt   = o->DIO_Repeat[vNr];
    // get value to copy

	if (o->DIO_Values[0] == NULL) return 0;
	cpyVal32 = o->DIO_Values[0][o->DIO_valueNr];
    if (secondBoard) cpyVal32b = o->DIO_Values[1][o->DIO_valueNr];
	
	while (i < samplesToWrite)  {
		samplesToCopy = o->DIO_Repeat[o->DIO_valueNr] - o->DIO_repCnt;
		if (samplesToCopy + i > samplesToWrite) samplesToCopy = samplesToWrite - i;
		memfill (&buffers[0][i], &cpyVal32, 4, samplesToCopy);
        if (secondBoard) memfill (&buffers[1][i], &cpyVal32b, 4, samplesToCopy);
        i+= samplesToCopy;
		o->DIO_repCnt += samplesToCopy;
		// -------------------------------------
		//    get new value to copy
		// -------------------------------------
		if (o->DIO_repCnt >= o->DIO_Repeat[o->DIO_valueNr]) {
	        o->DIO_valueNr++;
			o->DIO_repCnt = 0;
			if (o->DIO_valueNr >= o->numDIOValues) {
				// NEW COPY
				o->DIO_copyNr++;
				o->DIO_valueNr = 0;
				if (o->DIO_copyNr > o->nCopies) {
					// fillRest
					samplesToCopy = samplesToWrite - i;
					memfill (&buffers[0][i], &cpyVal32, 4, samplesToCopy);
			        if (secondBoard) memfill (&buffers[1][i], &cpyVal32b, 4, samplesToCopy);
			        i+= samplesToCopy;
				}
	        }
	        else {
				// 
	            cpyVal32 = o->DIO_Values[0][o->DIO_valueNr];
	            if (secondBoard) cpyVal32b = o->DIO_Values[1][o->DIO_valueNr];
	        }
		}
    }
	DEBUGOUT_printf ("\nwriteDIOBytesToOutputBuffer_DBLBUF: %7d samples (valueNr=%03d, repCnt=%6d, copyNr=%03d)",
				 i, o->DIO_valueNr, o->DIO_repCnt, o->DIO_copyNr);

	return i;
}    




unsigned long OUTPUTDATA_writeAOBytesToOutputBuffer_DBLBUF (t_outputData *o,
			  short **buffers, unsigned long bufferSizeSamples)
{

   	t_analogSample *cpySample;
    unsigned long i = 0;
	int d, nDevices;
    unsigned long samplesToWrite, samplesToCopy;

    unsigned __int64 samplesLeft64;
	
	if (o == NULL) return 0;
	if (buffers == 0) return 0;
	if (buffers[0] == NULL) return 0;
	
	if (bufferSizeSamples == 0) return 0;

	samplesLeft64 = o->AO_totalSamples - o->AO_totalSamplesWritten;
	samplesToWrite = (unsigned long) min (samplesLeft64, bufferSizeSamples);
//    samplesToWrite = o->DIO_totalSamples;
	// ---------------------------------------------
	//    init valiables
	// ---------------------------------------------
	// ---------------------------------------------
	//    tried to optimize routines for speed
	//    (so they don't look nice)
	// ---------------------------------------------
	if (o->simulate) nDevices = MAX_AO_DEVICES;
		else nDevices = nAOBoards();
	if ((o->AO_Values[1] != NULL) && (buffers[1] != NULL)) nDevices = 2;
   
    // get number of repetitions
//	repCnt   = o->DIO_Repeat[vNr];
    // get value to copy
	if (o->AO_SampleRepeat[0] == NULL) return 0;

	for (d = 0; d < nDevices; d++) {
    	i 	= 0;
		cpySample = &o->AO_Samples[d][o->AO_valueNr[d]];
		while (i < samplesToWrite)  {
			samplesToCopy = o->AO_SampleRepeat[d][o->AO_valueNr[d]] - o->AO_repCnt[d];
			if (samplesToCopy + i > samplesToWrite) samplesToCopy = samplesToWrite - i;
			memfill (&buffers[d][i<<3], cpySample, 2*8, samplesToCopy);
        	i+= samplesToCopy;
			o->AO_repCnt[d] += samplesToCopy;
			// -------------------------------------
			//    get new value to copy
			// -------------------------------------
			if (o->AO_repCnt[d] >= o->AO_SampleRepeat[d][o->AO_valueNr[d]]) {
		        o->AO_valueNr[d]++;
				o->AO_repCnt[d] = 0;
				if (o->AO_valueNr[d] >= o->AO_nSamples[d]) {
					// NEW COPY
					o->AO_copyNr[d]++;
					o->AO_valueNr[d] = 0;
					if (o->AO_copyNr[d] > o->nCopies) {
						// fillRest
						samplesToCopy = samplesToWrite - i;
						memfill (&buffers[d][i<<3], cpySample, 4, samplesToCopy);
				        i+= samplesToCopy;
					}
		        }
		        else {
					// 
		            cpySample = &o->AO_Samples[d][o->AO_valueNr[d]];
		        }
			}
	    }
		DEBUGOUT_printf ("\nwriteAOBytesToOutputBuffer_DBLBUF: %7d samples (valueNr=%03d, repCnt=%6d, copyNr=%03d)",
					 i, o->AO_valueNr[d], o->AO_repCnt[d], o->AO_copyNr[d]);
	}
	return i;
}    




void OUTPUTDATA_writeDIOBytesTEST (t_sequence *seq)
{
	t_outputData *o;
	unsigned long *buffers[2];
	unsigned long *bufAdr[2];
	//unsigned long bufferSize;
	int i;
	int nSteps = 4; 
	__int64 stepSize;
	
	o = seq->outData;
	o->DIO_totalSamples;
	
	buffers[0] = (unsigned long *) malloc (o->DIO_totalSamples * 4);
	stepSize = o->DIO_totalSamples / nSteps;
	
	OUTPUTDATA_resetCounters (o);

	
	DEBUGOUT_printf ("\nOUTPUTDATA_writeDIOBytesTEST:");

	for (i = 0; i < nSteps; i++) {
		bufAdr[0] = (unsigned long *) buffers[0];
		bufAdr[0] += i * stepSize * 4;
		OUTPUTDATA_writeDIOBytesToOutputBuffer_DBLBUF (o, (unsigned long **) bufAdr, (unsigned long) stepSize);
		OUTPUTDATA_displayBuffer ((unsigned long *) bufAdr, (unsigned long) stepSize);
	}
	DEBUGOUT_printf ("TOTAL Buffer:");
	OUTPUTDATA_displayBuffer (buffers[0], (unsigned long) o->DIO_totalSamples);
}



int OUTPUTDATA_hasChanged (t_outputData *out, int ch)
{
    // DEBUG change this function and implement analog Samples
	
	if (ch >= N_DAC_CHANNELS) return 0;
    if (out->numAOValues[ch] == 0) return 0;
    if (out->numAOValues[ch] != out->last_numAOValues[ch]) return 1;
    if (memcmp (out->last_AO_Values[ch], out->AO_Values[ch], out->numAOValues[ch]*sizeof(long))) return 1;
	if (memcmp (out->last_AO_ValueDuration[ch], out->AO_ValueDuration[ch], out->numAOValues[ch]*sizeof(unsigned))) return 1;
	return 0;
}



long OUTPUTDATA_getFirstOutputValue (t_outputData *out, int ch, unsigned *position)
{
	unsigned n, nValues;
	long value;
	
    nValues = out->numAOValues[ch];
    n = 0;
    value = 0;
    if (nValues != 0) {
    	while ((out->AO_ValueDuration[ch][n] == 0) && (n < nValues)) n++;
    	value = out->AO_Values[ch][n];
    }
    if (position != NULL) *position = n;
	return value;
}



int OUTPUTDATA_isChannelConstant (t_outputData *out, int ch)
{
    long *values;
    unsigned i, n;
    unsigned nValues;
    long firstValue;
    
    
    if (ch >= N_DAC_CHANNELS) return 0;
    if (out == NULL) return 0;
    
    nValues = out->numAOValues[ch];
    if (nValues <= 1) return 1;

	values = out->AO_Values[ch]; 
    
    firstValue = OUTPUTDATA_getFirstOutputValue (out, ch, &n);
    for (i = n+1; i < nValues; i++) {
    	if (out->AO_ValueDuration[ch][i] != 0) {
    		if (values[i] != firstValue) return 0;
    	}
    }
    return 1;
}



void OUTPUTDATA_rememberLastValues (t_outputData *out, int ch)
{
    unsigned nValues;
    
    if (ch >= N_DAC_CHANNELS) return;
    nValues = out->numAOValues[ch];
    
	// check if buffersize has changed, if yes: reallocate new memory
	if (nValues != out->last_numAOValues[ch]) {
		free (out->last_AO_Values[ch]);
		free (out->last_AO_ValueDuration[ch]);
		out->last_AO_Values[ch] = (long *) malloc (nValues * sizeof(long));
		out->last_AO_ValueDuration[ch]= (unsigned *) malloc (nValues * sizeof(unsigned));   
	}
	memcpy (out->last_AO_Values[ch], out->AO_Values[ch], nValues * sizeof(long));
	memcpy (out->last_AO_ValueDuration[ch], out->AO_ValueDuration[ch], nValues * sizeof (unsigned));
	out->last_numAOValues[ch] = nValues;
}





// create list of sweeps

void SEQUENCE_SWEEPS_createList (t_sequence *s)
{
	 PARAMETERSWEEP_deleteAll (s);
	 
}






