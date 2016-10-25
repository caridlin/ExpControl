#ifndef WATCHDOGVARIABLES
#define WATCHDOGVARIABLES


enum {
	STATUS_OK = 0, 
	STATUS_PREALERT, 
	STATUS_UNLOCKED
};


//#define DS_VARIABLE_PREALERT "%s/watchdog/prealert"

#define DS_WATCHDOG_PATH "%s/watchdog/%s"

#define DS_VAR_WD_STATUS   "status"
#define DS_VAR_WD_RECEIVED "received"





#endif

