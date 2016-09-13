
#ifndef CTRLDMDIMAGE
#define CTRLDMDIMAGE

#include <userint.h> 
#include "tools.h" 

//
//
//   DMD image
//
//

typedef struct {
	int shapeID;
	double *shapeParametersFrom;
	double *shapeParametersTo;
	int *shapeParametersVary;

	double rotationAngle;
	Point centerOffset;
	Point bitmapSize;
	int smoothEdges;
	
	int enableDMD;
	int enableDMDPhaseFeedback;
	int enableDMDErrorDiffusion;
	int enableDMDSmoothing;
	
	dPoint phaseFeedbackPhase;
	
// during runtime only
	//int bitmapID;
	double *shapeParametersThisRepetition;
} t_DMDimage;


void DMDIMAGE_init (t_DMDimage *d);

void DMDIMAGE_free (t_DMDimage *d);

void DMDIMAGE_allocateMemory (t_DMDimage *d);

void DMDIMAGE_calculateParametersThisRepetition (t_DMDimage *d, int nRepetitions, int repetition);

void DMDIMAGE_prepareBitmapForDMD(int bitmapID, FILE* output_pattern_file, int apply_error_diffuse, int apply_smoothing);

  // int DMDIMAGE_writeCommandFile (t_sequence *seq, t_DMDimage *d, int repetition);  



#endif
