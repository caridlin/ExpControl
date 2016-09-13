
#ifndef CTRLDMDshapes
#define CTRLDMDshapes


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


typedef void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, void *dmdWaveform, int repetition, int picNum);  // should be "t_waveform *dmdWaveform"

typedef struct {
	const char *identifier;
	const char *name;
	const char *description;
	int nParameters;
	char **parameterNames;
	char **parameterUnits;
	double *defaultValues;
	double *minValue;
	double *maxValue;
	DMDshape_plotFunctPtr plotFunc;
} t_DMDshape;




void DMDSHAPE_init (t_DMDshape *f);

void DMDSHAPE_initializeAll  (void);

t_DMDshape *DMDSHAPE_ptr (int shapeID);

const char *DMDSHAPE_identifier (int shapeID);

int DMDSHAPE_idFromIdentifier (const char *identifier);

void DMDSHAPE_addAllNamesToRing (int panel, int ctrl);

const char *DMDSHAPE_getDescription (int id);

int DMDSHAPE_nParameters (int shapeID);

int DMDSHAPE_nFunctions (void);

int DMDSHAPE_maxParameters (void);

//void DMDSHAPE_displayBitmapInCanvas (int panel, int ctrl, int id);









#endif // CTRLDMDshapes


