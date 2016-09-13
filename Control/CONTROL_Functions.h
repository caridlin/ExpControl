
#ifndef CTRLFunctions
#define CTRLFunctions




#define FUNCTION_MAX_time 100000.0

typedef double (*FUNCTION_evalPtr)(double arg, double *parameterArray, double duration, int reverseTime);


enum { FUNCTION_PARAM_TYPE_STANDARD = 0,
	   FUNCTION_PARAM_TYPE_REPETITION = 1,
 
      N_FUNCTION_PARAM_TYPES
};


typedef struct {
	const char *identifier;
	const char *name;
	const char *formula;
	int nParameters;
	char **parameterNames;
	char **parameterUnits;
	int *parameterType; // if >0 not editable and set by the system
	double *defaultValues;
	double *minValue;
	double *maxValue;
	FUNCTION_evalPtr eval;

	int bitmap;
	int independentDuration;
	double defaultDuration;
	
} t_function;


void FUNCTION_init (t_function *f);

void FUNCTION_initializeAll  (void);

t_function *FUNCTION_ptr (int functionID);

const char *FUNCTION_identifier (int functionID);

int FUNCTION_idFromIdentifier (const char *identifier);

void FUNCTION_addAllNamesToRing (int panel, int ctrl);

const char *FUNCTION_getFormula (int id);

int FUNCTION_nParameters (int functionID);

int FUNCTION_nFunctions (void);

int FUNCTION_maxParameters (void);

void FUNCTION_displayBitmapInCanvas (int panel, int ctrl, int id);

FUNCTION_setSpecialParameters(double* functionParameters, int functionID, int repetition);











#endif


