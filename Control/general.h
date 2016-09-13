
#ifndef CONTROL_GENERAL
#define CONTROL_GENERAL


#define MAX_DIO_DEVICES 3
#define MAX_AO_DEVICES 8


#define MByte (1024*1024)


// default values (for EDIT->New Sequence)
#define defaultNr_DIO_Channels 32
//#define defaultNr_AO_Channels 8

// Anzahl ns in kleinster Zeiteinheit(=50 ns)
#define VAL_MIN_ns   50                   

#define VAL_FLOAT_MIN_s (1.0E-9*VAL_MIN_ns)
#define VAL_FLOAT_MIN_ms (1.0E-6*VAL_MIN_ns)
#define VAL_FLOAT_MIN_us (1.0E-3*VAL_MIN_ns)
#define VAL_FLOAT_MIN_ns (1.0*VAL_MIN_ns)

// andere Zeiteinheiten in Einheiten von 50ns
#define VAL_50ns      1
#define VAL_us       20
#define VAL_ms    20000
#define VAL_s  20000000

#define VAL_MAX_s (ULONG_MAX / VAL_s)
#define VAL_MAX_T (VAL_MAX_s * VAL_s)

// Zeit in Flieﬂkomma umrechnen
#define fTime_s(T) (VAL_FLOAT_ns*(double)T)


typedef struct {
    double minVoltage;
	double maxVoltage;
	double Vpp;
	int minDigital;
	int maxDigital;
	int Dpp;
	int timebase_50ns;
	unsigned long nPoints;
	double duration_us;
	int offsetCorrection;
} t_digitizeParameters;


#define BIT(x) (1<<(x))


#endif
