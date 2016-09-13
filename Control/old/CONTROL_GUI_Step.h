
#ifndef CTRLstepper
#define CTRLstepper

//
//
// DMD shapes
//
//



//typedef void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, void *dmdWaveform);   // full declaration due to circular reference in code file
//typedef struct t_DMDshape t_DMDshape;

//void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, t_waveform *dmdWaveform);



// what i want to define but does not work:
//void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, t_waveform *dmdWaveform);


typedef void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, void *dmdWaveform);  // should be "t_waveform *dmdWaveform"

typedef struct {
	int type;
	char **parameterNames;
	char **parameterUnits;
	double *defaultValues;
	double *minValue;
	double *maxValue;
	DMDshape_plotFunctPtr plotFunc;
} t_DMDshape;



// ========================================
//   different types of steppers
// ========================================
enum { STEPPER_TYPE_NONE  = -1,
	   STEPPER_TYPE_LINEAR, 
       STEPPER_TYPE_FILE,
 
      N_STEPPERTypes
};


void STEPPER_init (t_DMDshape *f);









#endif // CTRLstepper


