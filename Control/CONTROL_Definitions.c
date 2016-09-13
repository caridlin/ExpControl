#include <ansi_c.h>
#include "control_definitions.h"



char *TCP_cmdStr[N_TCP_CMD_STRINGS] = {
	TCP_CMD_LOAD,
	TCP_CMD_RUN,
	TCP_CMD_SEQUENCE,
	TCP_CMD_GPIB,
	TCP_CMD_STOP,	
	TCP_CMD_DIGITAL_LINES,
	
	TCP_CMD_F5,
	TCP_CMD_F6,		
	TCP_CMD_F7,			
	TCP_CMD_F8,			
	TCP_CMD_F9,	
	
	TCP_CMD_CLOSE,
	TCP_CMD_STOPPED,
	TCP_CMD_DATA,	
	
	TCP_CMD_FILE,
	TCP_CMD_READY,
	TCP_CMD_QUIT,
	
};



int TCP_cmdStrID (const char *cmdStr)
{
	int i;
	
	for (i = 0; i < N_TCP_CMD_STRINGS; i++) {
		if (stricmp (cmdStr, TCP_cmdStr[i]) == 0) return i;
	}
	return -1;
}
