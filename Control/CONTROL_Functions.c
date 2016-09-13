

#include <userint.h>
#include "UIR_ExperimentControl.h"
#include <utility.h>    
#include "tools.h"

#include "CONTROL_DataStructure.h"					

ListType functionList = NULL;
int functionMaxNParameters = 0;






//=======================================================================
//
// 	  FUNCTIONS
//
//=======================================================================


void FUNCTION_init (t_function *f)
{
	f->name = 0;
	f->formula = 0;
	f->nParameters = 0;
	f->parameterNames = NULL;
	f->parameterUnits = NULL;
	f->parameterType = NULL;
	f->defaultValues = NULL;
	f->minValue = NULL;
	f->maxValue = NULL;
	f->eval = 0;
	f->bitmap = 0;
	f->identifier = 0;
	f->defaultDuration = 10.0;
	f->independentDuration = 1;
}


void FUNCTION_free (t_function *f)
{
	free (f->parameterNames);
	free(f->parameterUnits);
	free(f->parameterType);
	free(f->defaultValues);
	free(f->minValue);
	free(f->maxValue);
	DiscardBitmap (f->bitmap);
	FUNCTION_init (f);
}


t_function *FUNCTION_new (int nParameters, const char *identifier)
{
	t_function *f;
	int id;
	int i;

	id = FUNCTION_idFromIdentifier (identifier);
	if (id != 0) {
		PostMessagePopup ("Error!", "Duplicate function defined.");
		f = FUNCTION_ptr (id);
		FUNCTION_free (f); // this will overwrite the old function
	}
	else {
		f = (t_function *) malloc (sizeof (t_function));
		ListInsertItem (functionList, &f, END_OF_LIST);
		FUNCTION_init (f);
	}
	f->nParameters = nParameters;
	f->parameterNames = (char **) calloc (nParameters, sizeof (char*));
	f->parameterUnits = (char **) calloc (nParameters, sizeof (char*));
	f->parameterType = (int *) calloc (nParameters, sizeof (int*));       
	f->defaultValues = (double *) calloc (nParameters, sizeof (double));
	f->minValue = (double *) calloc (nParameters, sizeof (double));
	f->maxValue = (double *) calloc (nParameters, sizeof (double));
	f->identifier = identifier;
	
	for (i = 0; i < nParameters; i++) {
		f->maxValue[i] = 100000.0;
		f->parameterType[i] = FUNCTION_PARAM_TYPE_STANDARD;
	}
	
	if (nParameters > functionMaxNParameters) functionMaxNParameters = nParameters;
	
	return f;
}




double FUNCTION_exponential (double x, double *p, double duration, int reverseTime)
{
	// f(x) = A * exp(-x/B) + 	C;
	
	double val;
	
	if (reverseTime) return FUNCTION_exponential (duration-x, p, duration, 0);
	
	if (p[1] == 0) return 0;
	val = p[0] * exp(- x / p[1]) + p[2];
	if (fabs(val) < 1E-6) val = 0;
	return val;
}



double FUNCTION_exponential2 (double x, double *p, double duration, int reverseTime)
{
	
	double val;
	double D, tau;
	
	if (reverseTime) return FUNCTION_exponential2 (duration-x, p, duration, 0);
	
		
	if (p[1] == 0) return 0;
	D = p[0] / p[1];
	if (D == 0) return 0;
	tau = duration / log (D);

	val = p[0] * exp(-x/tau);
	if (fabs(val) < 1E-6) val = 0;
	return val;
}




double FUNCTION_exponential3 (double x, double *p, double duration, int reverseTime)
{
	// f(x) = A * ( exp(t/tau)-1) + U1
	// A = (U2 - U1) / (exp(duration/tau)-1);
	
	double val;
	double D;
	
	if (reverseTime) return FUNCTION_exponential3 (duration-x, p, duration, 0);
	
		
	if (p[2] == 0) return 0;
	D = exp(-duration/p[2]) - 1;
	if (D == 0) return 0;
	
	val = ((p[1] - p[0]) / D) * (exp(-x/p[2])-1) + p[0];

	if (fabs(val) < 1E-6) val = 0;
	return val;
}








double FUNCTION_sin1 (double x, double *p, double duration, int reverseTime)
{
	double arg; 
	
	if (reverseTime) return FUNCTION_sin1 (duration-x, p, duration, 0);

	if (p[1] == 0) return 0;
	arg = fmod (2*PI*x/p[1], 2*PI);
	return p[2] + p[0]*sin(arg);
}


double FUNCTION_sin2 (double x, double *p, double duration, int reverseTime)
{
	double arg; 
	
	if (reverseTime) return FUNCTION_sin2 (duration-x, p, duration, 0);
	arg = fmod (2*PI*x*p[1]*0.001, 2*PI);
	return p[2] + p[0]*sin(arg);
}


double FUNCTION_cos (double x, double *p, double duration, int reverseTime)
{
	
	double arg;
	
	if (reverseTime) return FUNCTION_cos (duration-x, p, duration, 0);

	
	if (p[1] == 0) return 0;
	arg = fmod (2*PI*x/p[1], 2*PI);
	return p[2] + p[0]*cos(arg);
}



double FUNCTION_sshape1 (double x, double *p, double duration, int reverseTime)
{
	double v;
	
	if (reverseTime) return FUNCTION_sshape1 (duration-x, p, duration, 0);
	
	v = p[2] + fabs (x-p[1]);
	if (v == 0) return 0;
	return p[3] + p[0] * (x-p[1]) / v;
	
}




void FUNCTION_addSShape1 (void)
{
	t_function *f; 
	
	f = FUNCTION_new (4, "S-SHAPE1");
	f->name	   = "S-Shape1";
	f->formula = "f(x) = A * (x-x0) / (C+|x-x0|) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "x0";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 10.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "C";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 5.0;
	f->minValue[2] = 0;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->parameterNames[3] = "Offs";
	f->parameterUnits[3] = "V";
	f->minValue[3] = -10.0;
	f->maxValue[3] = 10.0;
	f->eval = FUNCTION_sshape1;
	
	f->defaultDuration = 20.0;
}




double FUNCTION_ramp1_V0 (double x, double *p, double duration) 
{
	if (p[1] == 0) return 0;			
	return p[0] / (1 + exp (-x/p[1]));
}


double FUNCTION_ramp1 (double x, double *p, double duration, int reverseTime)
{
	double vM, vP, vX;
	
	if (reverseTime) return FUNCTION_ramp1 (duration-x, p, duration, 0);

	vM = FUNCTION_ramp1_V0 (-duration/2, p, 0);
	vP = FUNCTION_ramp1_V0 (+duration/2, p, 0);
	vX = FUNCTION_ramp1_V0 (x-duration/2, p, 0);
	if (vP-vM == 0) return 0;
	return (vX - vM) * p[0] / (vP - vM) + p[2];
	
}


double FUNCTION_ramp2_V0 (double x, double *p, double duration) 
{
	if (p[1] == 0) return 0;			
	if (p[2] == 0) return 0;			
	if (x < 0) return p[0] / (1 + exp (-x/p[1]));
	else return p[0] / (1 + exp (-x/p[2]));

}



double FUNCTION_ramp2 (double x, double *p, double duration)
{
	double vM, vP, vX;

	vM = FUNCTION_ramp2_V0 (-duration/2, p, 0);
	vP = FUNCTION_ramp2_V0 (+duration/2, p, 0);
	vX = FUNCTION_ramp2_V0 (x-duration/2, p, 0);
	if (vP-vM == 0) return 0;
	return (vX - vM) * p[0] / (vP - vM) + p[3];
	
}




/*double FUNCTION_ramp1neg (double x, double *p, double duration)
{
	return FUNCTION_ramp1 (duration-x, p, duration);
	
}
*/



void FUNCTION_addRamp1 (void)
{
	t_function *f; 

	f = FUNCTION_new (3, "RAMP1");
	f->name	   = "Ramp 1";
	f->formula = "f(x) = A' / (1 + exp(-x/t) + Offs";
	
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 2.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "t";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = 0.001;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval =  FUNCTION_ramp1;
	
	f->defaultDuration = 100.0;
}

/*
void FUNCTION_addRamp2 (void)
{
	t_function *f; 

	f = FUNCTION_new (4, "RAMP2");
	f->name	   = "Ramp 1";
	f->formula = "f(x) = A' / (1 + exp(-x/t) + Offs";
	
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 2.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "t1";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = 0.001;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "t2";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 1.0;
	f->minValue[2] = 0.001;
	f->maxValue[2] = FUNCTION_MAX_time;

	
	f->parameterNames[3] = "Offs";
	f->parameterUnits[3] = "V";
	f->minValue[3] = -10.0;
	f->maxValue[3] = 10.0;

	f->eval =  FUNCTION_ramp2;
	
	f->defaultDuration = 100.0;
}
*/

/*

void FUNCTION_addRamp1neg (void)
{
	t_function *f; 

	f = FUNCTION_new (3, "RAMP1NEG");
	f->name	   = "Ramp 1 (neg)";
	f->formula = "f(x) = A' / (1 - exp(-x/t) + Offs";
	
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 2.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "t";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1;
	f->minValue[1] = 0.001;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval =  FUNCTION_ramp1neg;
	
	f->defaultDuration = 100.0;
}

*/





void FUNCTION_addSin1 (void)
{
	t_function *f; 
	
	f = FUNCTION_new (3, "SIN");
	f->name	   = "Sin (tau)";
	f->formula = "f(x) = A * sin (2*Pi*x / tau) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "tau";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;
	
	f->eval = FUNCTION_sin1;
	f->defaultDuration = 2.0;
}



void FUNCTION_addSin2 (void)
{
	t_function *f; 
	
	f = FUNCTION_new (3, "SIN2");
	f->name	   = "Sin (freq)";
	f->formula = "f(x) = A * sin (2*Pi*freq*x) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "freq";
	f->parameterUnits[1] = "Hz";
	f->defaultValues[1] = 1000.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = 1000000.0;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;
	
	f->eval = FUNCTION_sin2;
	f->defaultDuration = 2.0;
}




void FUNCTION_addCos (void)
{
	t_function *f; 
	
	f = FUNCTION_new (3, "COS");
	f->name	   = "Cos";
	f->formula = "f(x) = A * cos (2*Pi*x / tau) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "tau";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;
	
	f->eval = FUNCTION_cos;
	f->defaultDuration = 2.0;
}


void FUNCTION_addExp (void) 
{
	t_function *f; 
	
	f = FUNCTION_new (3, "EXPDECAY");
	f->name	   = "Exponential decay (general)";
	f->formula = "f(x) = A * exp(- x / tau) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "tau";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 1.0;
 	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_exponential;
	f->defaultDuration = 5.0;

}				  



void FUNCTION_addExp2 (void) 
{
	t_function *f; 
	
	f = FUNCTION_new (2, "EXPDECAY2");
	f->name	   = "Exponential decay (fixed U1,U2)";
	f->formula = "f(x) = U1*exp(-x/tau); tau=DUR/ln(U1/U2)";
	f->parameterNames[0] = "U1";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 5.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "U2";
	f->parameterUnits[1] = "V";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;


	f->eval = FUNCTION_exponential2;
	f->defaultDuration = 5.0;

}				  


void FUNCTION_addExp3 (void) 
{
	t_function *f; 
	
	f = FUNCTION_new (3, "EXPDECAY3");
	f->name	   = "Exponential decay (fixed U1,U2,tau)";
	f->formula = "f(x) = A * (exp(-x/tau)-1) + U1; A = (U2-U1)/(exp(DUR/tau)-1)";
	f->parameterNames[0] = "U1";
	f->parameterUnits[0] = "V";
	f->defaultValues[0] = 5.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "U2";
	f->parameterUnits[1] = "V";
	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;

	f->parameterNames[2] = "tau";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 1.0;
 	f->minValue[2] = 0.0;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->eval = FUNCTION_exponential3;
	f->defaultDuration = 5.0;

}				  



double FUNCTION_sech (double x, double *p, double duration, int reverseTime)
{

	double vM, vX;
	
	if (reverseTime) return FUNCTION_sech (duration-x, p, duration, 0);
	

	if (p[1] == 0) return 0;
	DisableBreakOnLibraryErrors();
	vM = sech (-duration / p[1]);
	vX = sech (2*(x-0.5*duration) / p[1]);
	EnableBreakOnLibraryErrors();

	if (vM == 1) return 0;
	return (vX - vM) * p[0] / (1 - vM) + p[2];
}



double FUNCTION_sech_sqr (double x, double *p, double duration, int reverseTime)
{

	double vM, vX;
	
	if (reverseTime) return FUNCTION_sech_sqr (duration-x, p, duration, 0);
	

	if (p[1] == 0) return 0;
	DisableBreakOnLibraryErrors(); // exp can fail with ERANGE
	vM = sech (-duration / p[1]);
	vM = vM * vM;
	vX = sech (2*(x-0.5*duration) / p[1]);
	vX = vX * vX;
	EnableBreakOnLibraryErrors();

	if (vM == 1) return 0;
	return (vX - vM) * p[0] / (1 - vM) + p[2];
}


	
	
void FUNCTION_addSech (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "SECH");
	f->name	   = "sech";
	f->formula = "f(x) = A'*sech(2t/T) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "T";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_sech;
	f->defaultDuration = 40.0;
	
}				  


void FUNCTION_addSechSqr (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "SECH_SQR");
	f->name	   = "sech^2";
	f->formula = "f(x) = A'*[sech(2t/T)]^2 + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "T";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_sech_sqr;
	f->defaultDuration = 40.0;
	
}				  
	

double FUNCTION_tanh (double x, double *p, double duration, int reverseTime)
{

	double vM, vP, vX;
	
	if (reverseTime) return FUNCTION_tanh (duration-x, p, duration, 0);
	
	if (p[1] == 0) return 0;
	vM = tanh (-duration / p[1]);
	vP = tanh (duration / p[1]);
	vX = tanh (2*(x-0.5*duration) / p[1]);

	if (vP-vM == 0) return 0;
	return (vX - vM) * p[0] / (vP - vM) + p[2];
}





	
void FUNCTION_addTanh (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "TANH");
	f->name	   = "tanh (A, Offs)";
	f->formula = "f(x) = A'*tanh(2t/T) ";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "T";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.001;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_tanh;
	f->defaultDuration = 40.0;
}				  


double FUNCTION_tanh2 (double x, double *p, double duration, int reverseTime)
{

	double vM, vP, vX, A;
	
	if (reverseTime) return FUNCTION_tanh2 (duration-x, p, duration, 0);
	
	if (p[2] == 0) return 0;
	vM = tanh (-duration / p[2]);
	vP = tanh (duration / p[2]);
	vX = tanh (2*(x-0.5*duration) / p[2]);
	A = p[0] - p[1];
	if (vP-vM == 0) return 0;
	
	return (vX - vM) * A / (vP - vM) + p[1];
}


void FUNCTION_addTanh2 (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "TANH2");
	f->name	   = "tanh (U1, U2)";
	f->formula = "f(x) = A'*tanh(2t/T) + Offs";

	f->parameterNames[0] = "U1";
	f->parameterUnits[0] = "V";
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "U2";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;

	f->parameterNames[2] = "T";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 5.0;
	f->minValue[2] = 0.001;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->eval = FUNCTION_tanh2;
	f->defaultDuration = 40.0;
	
}				  


double FUNCTION_tanhSqr (double x, double *p, double duration, int reverseTime)
{

	double vM, vP, vX, A;
	
	if (reverseTime) return FUNCTION_tanhSqr (duration-x, p, duration, 0);
	
	if (p[2] == 0) return 0;
	vM = tanh (-duration / p[2]);
	vP = tanh (duration / p[2]);
	vX = tanh (2*(x-0.5*duration) / p[2]);
	A = p[0] - p[1];
	if (vP-vM == 0) return 0;
	
	return (vX - vM) * A / (vP - vM) + p[1];
}




void FUNCTION_addTanhSqr (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "TANHSQR");
	f->name	   = "tanh^2 (U1, U2)";
	f->formula = "f(x) = A'*tanh(2t/T) + Offs";

	f->parameterNames[0] = "U1";
	f->parameterUnits[0] = "V";
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "U2";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;

	f->parameterNames[2] = "T";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 5.0;
	f->minValue[2] = 0.001;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->eval = FUNCTION_tanhSqr;
	f->defaultDuration = 40.0;
	
}				  



#define gauss(x,sigma) (exp(-(x)*(x)/(2*(sigma)*(sigma))))

double FUNCTION_gauss (double x, double *p, double duration, int reverseTime)
{
	double val;
	
	if (reverseTime) return FUNCTION_gauss (duration-x, p, duration, 0);
	
	if (p[2] == 0) return 0;
	val = p[0] * gauss(x-p[1],p[2]) + p[3];
	if (fabs(val) < 1E-6) val = 0;
	return val;
}





	
void FUNCTION_addGauss (void) 
{
	t_function *f; 

	f = FUNCTION_new (4, "GAUSS");
	f->name	   = "Gauss";
	f->formula = "f(x) = A * exp[-(x-x0)^2/(2sigma^2)] + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "x0";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 20.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;
	
	f->parameterNames[2] = "sigma";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 5.0;
	f->minValue[2] = 0.0;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->parameterNames[3] = "Offs";
	f->parameterUnits[3] = "V";
	f->minValue[3] = -10.0;
	f->maxValue[3] = 10.0;

	f->eval = FUNCTION_gauss;
	f->defaultDuration = 40.0;
	
}				  


double FUNCTION_gauss2 (double x, double *p, double duration, int reverseTime)    
{
	double vM, vX;

	if (reverseTime) return FUNCTION_gauss (duration-x, p, duration, 0);
	
	
	if (p[1] == 0) return 0;
	vM = gauss(-0.5*duration,p[1]);
	vX = gauss(x-0.5*duration,p[1]);
	
	if (vM == 1.0) return 0;
	return (vX - vM) * p[0] / (1-vM) + p[2];

}





	

void FUNCTION_addGauss2 (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "GAUSS2");
	f->name	   = "Gauss (norm)";
	f->formula = "f(x) = A' * exp[-x^2/(2sigma^2)] + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "sigma";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_gauss2;
	f->defaultDuration = 40.0;
	
}		



double FUNCTION_lorentz1 (double x, double *p, double duration, int reverseTime)    
{
	double n;

	if (reverseTime) return FUNCTION_lorentz1 (duration-x, p, duration, 0);
	
	if (p[2] == 0) return 0;
	
	n = (1 + (x - p[1])/p[2]);
	if (n == 0) return 0;
	
	return p[0] / (n*n) + p[3];
}




void FUNCTION_addLorentz1 (void) 
{
	t_function *f; 

	f = FUNCTION_new (4, "LORENTZ1");
	f->name	   = "Lorentian";
	f->formula = "f(t) = A / (1 + (t-t0)/tau)^2 + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "t0";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	
	f->parameterNames[2] = "tau";
	f->parameterUnits[2] = "ms";
	f->defaultValues[2] = 5.0;
	f->minValue[2] = 0.0;
	f->maxValue[2] = FUNCTION_MAX_time;

	f->parameterNames[3] = "Offs";
	f->parameterUnits[3] = "V";
	f->minValue[3] = -10.0;
	f->maxValue[3] = 10.0;

	f->eval = FUNCTION_lorentz1;
	f->defaultDuration = 40.0;
	
}				  



double FUNCTION_blackman (double x, double *p, double duration, int reverseTime)    
{
	double k;

	if (reverseTime) return FUNCTION_blackman (duration-x, p, duration, 0);
	
	if (duration == 0) return 0;
	
	k = PI * (x/duration);
	return (0.35875 - 0.48829 * cos (2*k) + 0.14128 * cos (4*k) - 0.01168 * cos (6*k)) * p[0] + p[1];
}




void FUNCTION_addBlackman (void) 
{
	t_function *f; 

	f = FUNCTION_new (2, "Blackman");
	f->name	   = "Blackman";
	f->formula = "f(t) = A * (...) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "Offs";
	f->parameterUnits[1] = "V";
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;

	f->eval = FUNCTION_blackman;
	f->defaultDuration = 5.0;
	
}	


double FUNCTION_sech2 (double x, double *p, double duration, int reverseTime)
{

	double vM, vX;
	
	if (reverseTime) return FUNCTION_sech (duration-x, p, duration, 0);
	

	if (p[1] == 0) return 0;
	vM = 0;//sech (-duration / p[1]);
	
	DisableBreakOnLibraryErrors(); // exp can fail with ERANGE
	vX = sech (2*(x-0.5*duration) / p[1]);
	EnableBreakOnLibraryErrors();

	if (vM == 1) return 0;
	return (vX - vM) * p[0] / (1 - vM) + p[2];
}

	
	
void FUNCTION_addSech2 (void) 
{
	t_function *f; 

	f = FUNCTION_new (3, "SECH2");
	f->name	   = "sech2";
	f->formula = "f(x) = A'*sech(2t/T) + Offs";
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;

	f->parameterNames[1] = "T";
	f->parameterUnits[1] = "ms";
	f->defaultValues[1] = 5.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = FUNCTION_MAX_time;

	f->parameterNames[2] = "Offs";
	f->parameterUnits[2] = "V";
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;

	f->eval = FUNCTION_sech2;
	f->defaultDuration = 40.0;
	
}				  


double FUNCTION_HS1(double x, double *p, double duration, int reverseTime)    
{
	double k;
	double sechParams[3];
	double tanh2Params[3]; 
	double freq;
	double A;
	double offset;
	//double arg;
	

	if (reverseTime) return FUNCTION_HS1 (duration-x, p, duration, 0);
	
	if (duration == 0) return 0;
	
	offset = p[0];
	
	sechParams[0] = p[1];
	sechParams[1] = p[5]; 
	sechParams[2] = p[2];
	
	tanh2Params[0] = p[3];
	tanh2Params[1] = p[4];
	tanh2Params[2] = p[5];
	
	
	k = PI *x*0.001;
	
	freq = FUNCTION_tanh2(x,tanh2Params,duration,reverseTime);
	A = FUNCTION_sech2(x,sechParams,duration,reverseTime);                   
	
	//arg = fmod (2*freq*k, 2*PI); 
	return A*sin(2*freq*k)+offset;
}

// abc
void FUNCTION_addHS1Sweep (void) 
{
	t_function *f; 

	f = FUNCTION_new (6, "HS1");
	f->name	   = "HS1";
	f->formula = "f(t) = A(t)*sin(w(t)*t)+offset";
	
	f->parameterNames[0] = "Offset";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "A1";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;
	
	f->parameterNames[2] = "A2";
	f->parameterUnits[2] = "V";
 	f->defaultValues[2] = 0.0;
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;
	
	f->parameterNames[3] = "w1";
	f->parameterUnits[3] = "Hz";
 	f->defaultValues[3] = 1000.0;
	f->minValue[3] = 0.0;
	f->maxValue[3] = 10000.0;
	
	f->parameterNames[4] = "w2";
	f->parameterUnits[4] = "Hz";
 	f->defaultValues[4] = 2000.0;
	f->minValue[4] = 0.0;
	f->maxValue[4] = 10000.0;
	
	f->parameterNames[5] = "T";
	f->parameterUnits[5] = "ms";
 	f->defaultValues[5] = 20.0;
	f->minValue[5] = 0.0;
	f->maxValue[5] = 10000.0;
	
	f->eval = FUNCTION_HS1;
	f->defaultDuration = 100.0;
	
}				  



double FUNCTION_HS1_fixed_cycle_num(double x, double *p, double duration, int reverseTime)    
{
	double k;
	double sechParams[3];
	double tanh2Params[3]; 
	double center_freq;
	double tau;
	double freq;
	double A;
	double offset;
	//double arg;
	

	if (reverseTime) return FUNCTION_HS1 (duration-x, p, duration, 0);
	
	if (duration == 0) return 0;
	
	offset = p[0];
	
	center_freq = (p[3]+p[4])/2;
	tau = 1000/1.3*p[5]/center_freq; // 1.3 is chosen such that num cycles in FWHM matches the setting 
	
	sechParams[0] = p[1];
	sechParams[1] = tau;    
	sechParams[2] = p[2];
	
	tanh2Params[0] = p[3];
	tanh2Params[1] = p[4];
	
	
	tanh2Params[2] = tau;
	
	
	k = PI *x*0.001;
	
	freq = FUNCTION_tanh2(x,tanh2Params,duration,reverseTime);
	A = FUNCTION_sech2(x,sechParams,duration,reverseTime);                   
	
	//arg = fmod (2*freq*k, 2*PI); 
	return A*sin(2*freq*k+0.7)+offset;
}


void FUNCTION_addHS1Sweep_fixed_cycle_num (void) 
{
	t_function *f; 

	f = FUNCTION_new (6, "HS1_fixed_cycle_num");
	f->name	   = "HS1 fixed cycle num";
	f->formula = "f(t) = A(t)*sin(w(t)*t)+offset";
	
	f->parameterNames[0] = "Offset";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 1.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "A1";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;
	
	f->parameterNames[2] = "A2";
	f->parameterUnits[2] = "V";
 	f->defaultValues[2] = 0.0;
	f->minValue[2] = -10.0;
	f->maxValue[2] = 10.0;
	
	f->parameterNames[3] = "w1";
	f->parameterUnits[3] = "Hz";
 	f->defaultValues[3] = 1000.0;
	f->minValue[3] = 0.0;
	f->maxValue[3] = 10000.0;
	
	f->parameterNames[4] = "w2";
	f->parameterUnits[4] = "Hz";
 	f->defaultValues[4] = 2000.0;
	f->minValue[4] = 0.0;
	f->maxValue[4] = 10000.0;
	
	f->parameterNames[5] = "#cycles FWHM";
	f->parameterUnits[5] = "";
 	f->defaultValues[5] = 20.0;
	f->minValue[5] = 0.0;
	f->maxValue[5] = 1000000.0;
	
	
	f->eval = FUNCTION_HS1_fixed_cycle_num;
	f->defaultDuration = 100.0;
	
}	



double FUNCTION_RectPulse(double x, double *p, double duration, int reverseTime)    
{
	double offset;
	double t0;
	double dt;
	double A;
	
	offset = p[0];
	A = p[1];
	t0 = p[2];
	dt = p[3];
	

	if (reverseTime) return FUNCTION_RectPulse (duration-x, p, duration, 0);
	
	if (duration == 0) return 0;
	
	if (x > t0 && x < t0+dt)
		return offset+A;
	
	return offset;
}


void FUNCTION_addRectPulse (void) 
{
	t_function *f; 

	f = FUNCTION_new (4, "RectPulse");
	f->name	   = "Rect Pulse";
	f->formula = "f(t) = A*theta(t-t0)*theta(t0+dt-t)+offset";
	
	f->parameterNames[0] = "Offset";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 0.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "A";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;
	
	f->parameterNames[2] = "t0";
	f->parameterUnits[2] = "ms";
 	f->defaultValues[2] = 20.0;
	f->minValue[2] = 0.0;
	f->maxValue[2] = 100000.0;
	
	f->parameterNames[3] = "dt";
	f->parameterUnits[3] = "ms";
 	f->defaultValues[3] = 60;
	f->minValue[3] = 0.0;
	f->maxValue[3] = 100000.0;
	
	
	f->eval = FUNCTION_RectPulse;
	f->defaultDuration = 100.0;
	
}

double FUNCTION_RectPulse2(double x, double *p, double duration, int reverseTime)    
{
	double offset;
	double t1, dt1, A1;
	double t2, dt2, A2;    
	double result;
	
	offset = p[0];
	A1 = p[1];
	t1 = p[2];
	dt1 = p[3];
	A2 = p[4];
	t2 = p[5];
	dt2 = p[6];
	

	if (reverseTime) return FUNCTION_RectPulse2 (duration-x, p, duration, 0);
	
	if (duration == 0) return 0;
	
	result = offset;
	if (x > t1 && x < t1+dt1) {
		result += A1;
	}
	if (x > t2 && x < t2+dt2) {
		result += A2;
	}
	
	return result;
}


void FUNCTION_addRectPulse2 (void) 
{
	t_function *f; 

	f = FUNCTION_new (7, "RectPulse2");
	f->name	   = "Rect Pulse 2";
	f->formula = "f(t) = A1*theta(t-t1)*theta(t1+dt1-t)+A2*theta(t-t2)*theta(t2+dt2-t)+offset";
	
	f->parameterNames[0] = "Offset";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 0.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	// pulse 1
	f->parameterNames[1] = "A1";
	f->parameterUnits[1] = "V";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = -10.0;
	f->maxValue[1] = 10.0;
	
	f->parameterNames[2] = "t1";
	f->parameterUnits[2] = "ms";
 	f->defaultValues[2] = 20.0;
	f->minValue[2] = 0.0;
	f->maxValue[2] = 100000.0;
	
	f->parameterNames[3] = "dt1";
	f->parameterUnits[3] = "ms";
 	f->defaultValues[3] = 10;
	f->minValue[3] = 0.0;
	f->maxValue[3] = 100000.0;
	
	// pulse 2
	f->parameterNames[4] = "A2";
	f->parameterUnits[4] = "V";
 	f->defaultValues[4] = 1.0;
	f->minValue[4] = -10.0;
	f->maxValue[4] = 10.0;
	
	f->parameterNames[5] = "t2";
	f->parameterUnits[5] = "ms";
 	f->defaultValues[5] = 40.0;
	f->minValue[5] = 0.0;
	f->maxValue[5] = 100000.0;
	
	f->parameterNames[6] = "dt2";
	f->parameterUnits[6] = "ms";
 	f->defaultValues[6] = 10;
	f->minValue[6] = 0.0;
	f->maxValue[6] = 100000.0;
	
	
	f->eval = FUNCTION_RectPulse2;
	f->defaultDuration = 100.0;
	
}	



double FUNCTION_CrazyLinear(double x, double *p, double duration, int reverseTime)    
{
	double startVal;
	double endVal;
	double repetition;
	
	startVal = p[0];
	repetition = p[1];
	
	endVal = repetition*repetition/10;
	

	if (reverseTime) return FUNCTION_CrazyLinear (duration-x, p, duration, 0);
	
	if (duration == 0) return startVal;
	
	return startVal+(endVal-startVal)*x/duration;
}


void FUNCTION_addCrazyLinear (void) 
{
	t_function *f; 

	f = FUNCTION_new (2, "CrazyLinear");
	f->name	   = "CrazyLinear";
	f->formula = "f(t) = A+(5*rep^2-A)*t";
	
	f->parameterNames[0] = "A";
	f->parameterUnits[0] = "V";
 	f->defaultValues[0] = 0.0;
	f->minValue[0] = -10.0;
	f->maxValue[0] = 10.0;
	
	f->parameterNames[1] = "repetition";
	f->parameterUnits[1] = "";
 	f->defaultValues[1] = 1.0;
	f->minValue[1] = 0.0;
	f->maxValue[1] = 100000.0; 
	f->parameterType[1] = FUNCTION_PARAM_TYPE_REPETITION;
	
	
	f->eval = FUNCTION_CrazyLinear;
	f->defaultDuration = 100.0;
	
}	








void FUNCTION_createBitmap (t_function *f, int panel, int ctrl)
{
	int nPoints;
	double *x, *y;
	int width, height;
	double yMax = DBL_MIN;
	double yMin = DBL_MAX;
	int i;
	double xScale, yScale;
	Point p;
	int err;
	
	if (f->bitmap > 0) DiscardBitmap (f->bitmap);

	width = ctrlWidth (panel, ctrl);
	height = ctrlHeight (panel, ctrl);
		
	nPoints = width / 2 + 1;
	x = (double *) malloc (sizeof (double) * nPoints);
	y = (double *) malloc (sizeof (double) * nPoints);
	for (i = 0; i < nPoints; i++) {
		x[i]  = (double) i * f->defaultDuration  / ((double) nPoints);
		y[i]  = f->eval (x[i], f->defaultValues, f->defaultDuration, 0);
		if (y[i] > yMax) yMax = y[i];
		if (y[i] < yMin) yMin = y[i];
	}						  
	xScale = ((double) width) / f->defaultDuration;
	yScale = ((double)height) / (yMax - yMin);

	SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_RED);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 2);
	
	p.x = RoundRealToNearestInteger (x[0] * xScale);
	p.y = height - RoundRealToNearestInteger ((y[0]-yMin) * yScale);
	CanvasSetPenPosition (panel, ctrl, p);
	for (i = 1; i < nPoints; i++) {
		p.x = RoundRealToNearestInteger (x[i] * xScale);
		p.y = height - RoundRealToNearestInteger ((y[i]-yMin) * yScale);
		CanvasDrawLineTo (panel, ctrl, p);
	}
	err = GetCtrlDisplayBitmap (panel, ctrl, 0, &f->bitmap);

	free (x);
	free (y);
}


void FUNCTION_displayBitmapInCanvas (int panel, int ctrl, int id)
{
	t_function *f;
	
	f = FUNCTION_ptr (id);

	CanvasClear (panel, ctrl, VAL_ENTIRE_OBJECT);
	if (f == NULL) return;
	if (f->bitmap == 0) {
		FUNCTION_createBitmap (f, panel, ctrl);
	}
	else {
		CanvasDrawBitmap (panel, ctrl, f->bitmap,
						  VAL_ENTIRE_OBJECT,
						  MakeRect(0,0, VAL_KEEP_SAME_SIZE, VAL_KEEP_SAME_SIZE));
	}
}


void FUNCTION_initializeAll (void)
{
	if (functionList != NULL) return;
	functionList = ListCreate (sizeof (t_function *));

	FUNCTION_addGauss ();
	FUNCTION_addGauss2 ();
	FUNCTION_addSin1 ();
	FUNCTION_addSin2 ();
	FUNCTION_addCos ();
//	FUNCTION_addSShape1 ();
	FUNCTION_addSech ();
	FUNCTION_addSechSqr ();
	FUNCTION_addTanh ();
	FUNCTION_addTanh2 ();
	FUNCTION_addTanhSqr ();

	FUNCTION_addRamp1 ();
//	FUNCTION_addRamp2 ();

	FUNCTION_addExp ();
	FUNCTION_addExp2 ();
	FUNCTION_addExp3 ();
	FUNCTION_addLorentz1 ();
	FUNCTION_addBlackman ();
	FUNCTION_addHS1Sweep ();  
	FUNCTION_addHS1Sweep_fixed_cycle_num();
	FUNCTION_addSech2();
	FUNCTION_addRectPulse();
	FUNCTION_addRectPulse2();
	
	FUNCTION_addCrazyLinear(); // only demo - not used anywhere
}


t_function *FUNCTION_ptr (int nr)
{
	t_function *f;
	
	if ((nr <= 0) || (nr > (int)ListNumItems (functionList))) return NULL;
	ListGetItem (functionList, &f, nr);
	return f;
}


const char *FUNCTION_identifier (int functionID)
{
	t_function *f;
	
	f = FUNCTION_ptr (functionID);
	if (f == NULL) return "";
	return f->identifier;
}


int FUNCTION_idFromIdentifier (const char *identifier)
{
	int id;
	t_function *f;

	for (id = ListNumItems (functionList); id > 0; id--) {
		ListGetItem (functionList, &f, id);
		if (strcmp (identifier, f->identifier) == 0) return id;
	}
	return 0;
}



void FUNCTION_addAllNamesToRing (int panel, int ctrl)
{
	int i;
	t_function *f;
	
	if (functionList == NULL) FUNCTION_initializeAll ();
	ClearListCtrl (panel, ctrl);
	for (i = 1; i <= ListNumItems (functionList); i++) {
		ListGetItem (functionList, &f, i);
		InsertListItem (panel, ctrl, -1, f->name, i);
	}
}


int FUNCTION_nFunctions (void)
{
	if (functionList == NULL) FUNCTION_initializeAll ();
	return ListNumItems (functionList);	
}
	

const char *FUNCTION_getFormula (int id) 
{
	t_function *f;
	
	f = FUNCTION_ptr (id);
	if (f == NULL) return "";
	else return f->formula;
}


int FUNCTION_nParameters (int functionID)
{
	t_function *f;
	
	f = FUNCTION_ptr (functionID);
	if (f == NULL) return 0;
	else return f->nParameters;
}


int FUNCTION_maxParameters (void)
{
	if (functionList == NULL) FUNCTION_initializeAll ();
	return functionMaxNParameters;	
}



int FUNCTION_setSpecialParameters(double* functionParameters, int functionID, int repetition)
{
	int i;
	t_function* f;
	int changed = 0;
	
	f = FUNCTION_ptr (functionID);
	if (f == NULL) return 0;
	
	for (i=0; i < f->nParameters; i++) {
		//tprintf("check param %s : %d\n",f->parameterNames[i],f->parameterType[i]);    
		switch (f->parameterType[i]) {
			case FUNCTION_PARAM_TYPE_REPETITION:
				functionParameters[i] = repetition;
				changed++;
				break;
			case FUNCTION_PARAM_TYPE_STANDARD:
				break;
			default: break;
		}
	}
	return 0;
}
		/*
int FUNCTION_setSpecialParameters(double *functionParameters, int repetition)
{
	int i,j;
	int found = 0;
	t_function *f;
	
	for (i=0; i < FUNCTION_nFunctions(); i++) {
		f = FUNCTION_ptr (i);
		if (f == NULL) continue;
		for (j=0; j < f->nParameters; j++) {
			found += FUNCTION_setSpecialParameter(functionParameters, f,i,repetition);
		}
	}
	return found;
}		*/



					  
					  
					  
