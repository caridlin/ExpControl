#include "INTERFACE_TCP_IP.h"    

#include "UIR_TCP_IP.h"


#include <formatio.h>
#include <utility.h>
#include <ansi_c.h>
#include <userint.h>
#include <tcpsupp.h>
#include "tools.h"


#define portNum 981
#define MAX_TCP_BUFFER_SIZE 5000000
#define TCP_TIMEOUT_MS 10000

/*---------------------------------------------------------------------------*/
/* Module-globals                                                            */
/*---------------------------------------------------------------------------*/

#define TCP_NO_CONNECTION ULONG_MAX


unsigned int SERVER_TCPhandle = TCP_NO_CONNECTION;
unsigned int CLIENT_TCPhandle = TCP_NO_CONNECTION;

int panelServer = -1;
int panelClient = -1;

int	panelClientLED   = -1;
int	ctrlClientLEDsend    = -1;
int	ctrlClientLEDreceive = -1;

int	panelServerLED  = -1;
int	ctrlServerLEDsend    = -1;
int	ctrlServerLEDreceive = -1;

//void *SERVER_parseFunction = NULL;

void (*SERVER_parseFunction )(char *, unsigned long) = NULL;
void (*CLIENT_parseFunction )(char *, unsigned long) = NULL;

int displayProtocol = 1;


const char TCP_okayStr[] = "okay";
const char TCP_strError[] = "error";
//char SERVER_name[MAX_PATHNAME_LEN] = "";

const char strLocalhost[] = "localhost";

int displayErrors = 1;

void TCP_clientSetLed (int panel, int sendLED, int receiveLED)
{
	panelClientLED   = panel;
	ctrlClientLEDsend    = sendLED;
	ctrlClientLEDreceive = receiveLED;
}


void TCP_serverSetLed (int panel, int sendLED, int receiveLED)
{
	panelServerLED        = panel;
	ctrlServerLEDsend    = sendLED;
	ctrlServerLEDreceive = receiveLED;
}



void TCP_setDisplayProtocol (int display)
{
	displayProtocol = display;
}


void clientLEDsend (int state)
{
	if (ctrlClientLEDsend != -1) 
		SetCtrlVal (panelClientLED, ctrlClientLEDsend, state);
}

void clientLEDreceive (int state)
{
	if (ctrlClientLEDreceive != -1) SetCtrlVal (panelClientLED, ctrlClientLEDreceive, state);
}

void serverLEDsend (int state)
{
	if (ctrlServerLEDsend != -1) SetCtrlVal (panelServerLED, ctrlServerLEDsend, state);
}

void serverLEDreceive (int state)
{
	if (ctrlServerLEDreceive != -1) SetCtrlVal (panelServerLED, ctrlServerLEDreceive, state);
}


const char *TCP_errorString (int errorNo)
{
	switch (abs(errorNo)) {
		case kTCP_NoError                     :  return "No error";                        
		case kTCP_UnableToRegisterService     :  return "Unable to register service";        
		case kTCP_UnableToEstablishConnection :  return "Unable to establish connection";    
		case kTCP_ExistingServer              :  return "Existing server";                 
		case kTCP_FailedToConnect             :  return "Failed to connect";               
		case kTCP_ServerNotRegistered         :  return "Server not registered";            
		case kTCP_TooManyConnections          :  return "Too many conversations";          
		case kTCP_ReadFailed                  :  return "Read failed";                     
		case kTCP_WriteFailed                 :  return "Write failed";                    
		case kTCP_InvalidParameter            :  return "Invalid parameter";               
		case kTCP_OutOfMemory                 :  return "Out of memory";                    
		case kTCP_TimeOutErr                  :  return "Time out error";                     
		case kTCP_NoConnectionEstablished     :  return "No connecton established";        
		case kTCP_GeneralIOErr                :  return "General IO-Error";                   
		case kTCP_ConnectionClosed            :  return "Connection closed";               
		case kTCP_UnableToLoadWinsockDLL      :  return "Unable to load WinsockDLL";         
		case kTCP_IncorrectWinsockDLLVersion  :  return "Incorrect WinsockDLL Version";     
		case kTCP_NetworkSubsystemNotReady    :  return "Network subsystem not ready";       
		case kTCP_ConnectionsStillOpen        :  return "Connections still open";           
		case kTCP_DisconnectPending           :  return "Disconnect pending";              
		case kTCP_InfoNotAvailable            :  return "Info not available";               
		case kTCP_HostAddressNotFound         :  return "Host address not found";            
		default: return "";
	}
}
		

char *TCP_errorStr (const char *functionName, const char *subroutineName, int errorNo)
{
	char *tmp;
	
	tmp = getTmpString();
	sprintf (tmp, "TCP/IP error %d: %s (subroutine \"%s\", function \"%s\")\n",
				  errorNo, TCP_errorString(errorNo), subroutineName, functionName);
	return tmp;
}
		
		
int TCP_displayError (const char *functionName, const char *subroutineName, int errorNo)
{
	if (errorNo >= 0) return 0;
	if (!displayErrors) return errorNo;
	SetWaitCursor (0);
	PostMessagePopupf ("TCP/IP Error",  
				   TCP_errorStr (functionName, subroutineName, errorNo));
	return errorNo;
}


		

/*---------------------------------------------------------------------------*/
/* Internal function prototypes                                              */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ServerTCPCB (unsigned handle, int event, int error,
                             void *callbackData);

int CVICALLBACK ClientTCPCB (unsigned handle, int event, int error,
                             void *callbackData);



int TCP_panelServer(void)
{
	return panelServer;
}



void TCP_setPanelServer(int newPanel)
{
	if (newPanel > 0) panelServer = newPanel;
}

int TCP_panelClient(void)
{
	return panelClient;
}



void TCP_initPanelServer (int parentPanel)
{
	panelServer = LoadPanel (parentPanel, INTERFACE_TCP_UIR_File, TCP_S);
}



void TCP_initPanelClient (int parentPanel)
{
	panelClient = LoadPanel (parentPanel, INTERFACE_TCP_UIR_File, TCP_C);
}


void TCP_setClientCtrlsVisible (int active)
{
	SetCtrlAttribute (panelClient, TCP_C_BTN_connectToServer,
					  ATTR_DIMMED, active);
   	SetCtrlVal (panelClient, TCP_C_CONNECTED, active);
	SetCtrlAttribute (panelClient, TCP_C_SERVER_NAME,
					  ATTR_CTRL_MODE,
					  active ? VAL_INDICATOR : VAL_HOT);
}


int TCP_initClient (const char *serverName, void (*parseFunction )(char *, unsigned long) , int showErrorMsg)
{
    char tempBuf[256] = {0};
    int error;

    /* Attempt to connect to TCP server... */
    CLIENT_parseFunction = parseFunction;
    if (serverName == NULL) return 0;
    SetWaitCursor (1);
    error = ConnectToTCPServer (&CLIENT_TCPhandle, portNum,
									serverName, ClientTCPCB, NULL,
									1000);
    if (error < 0) {
		if (showErrorMsg) TCP_displayError ("TCP_initClient()", "ConnectToTCPServer", error);
        CLIENT_TCPhandle = 0;
		TCP_setClientCtrlsVisible (0);
	
        goto ERRORID;
      	}
    else
        {
//        strcpy (SERVER_name, serverName);
        SetWaitCursor (0);
        
        /* We are successfully connected -- gather info */
        SetCtrlVal (panelClient, TCP_C_CONNECTED, 1);
        GetTCPHostAddr (tempBuf, 256);
        SetCtrlVal (panelClient, TCP_C_CLIENT_IP, tempBuf);
        GetTCPHostName (tempBuf, 256);
        SetCtrlVal (panelClient, TCP_C_CLIENT_NAME, tempBuf);
        GetTCPPeerAddr (CLIENT_TCPhandle, tempBuf, 256);
        SetCtrlVal (panelClient, TCP_C_SERVER_IP, tempBuf);
        GetTCPPeerName (CLIENT_TCPhandle, tempBuf, 256);
        SetCtrlVal (panelClient, TCP_C_SERVER_NAME, tempBuf);
		TCP_setClientCtrlsVisible (1);
        
    	pprintf (panelClient, TCP_C_RECEIVE, 
    			"%s --- client connected, handle = %d ---\n", TimeStr(), CLIENT_TCPhandle);
		return 0;
    }    
ERRORID:
    SetWaitCursor (0);
    return -1;
	
}


void TCP_initServer (void *parseFunction)
{
    char tempBuf[256] = {0};
 
    /* Prompt for the port number on which to receive connections */
    /* Attempt to register as a TCP server... */
    SetWaitCursor (1);
    if (RegisterTCPServer (portNum, ServerTCPCB, 0) < 0) {
        SERVER_parseFunction = NULL;
        MessagePopup("TCP Server", "Server registration failed!");
        return;
    }
    SetWaitCursor (0);
    /* We are successfully registered -- gather info */
    SetCtrlVal (panelServer, TCP_S_ONLINE, 1);
    GetTCPHostAddr (tempBuf, 256);
    SetCtrlVal (panelServer, TCP_S_SERVER_IP, tempBuf);
    GetTCPHostName (tempBuf, 256);
    SetCtrlVal (panelServer, TCP_S_SERVER_NAME, tempBuf);
//        SetCtrlAttribute (g_hmainPanel, MAINPNL_STRING, ATTR_DIMMED, 1);
    SetCtrlVal (panelServer, TCP_S_CONNECTED, 0); 
    pprintf (panelServer, TCP_S_RECEIVE, "%s --- TCP/IP server started.\n", TimeStr());
    SERVER_parseFunction = parseFunction;
        
}



int serverTCPReadAll (char *dataBuffer, unsigned nBytes, unsigned timeOut)
{
    unsigned totalBytesRead;
	int bytesRead;

	totalBytesRead = 0;
	while (totalBytesRead < nBytes) {
		bytesRead = ServerTCPRead (SERVER_TCPhandle, dataBuffer+totalBytesRead, nBytes-totalBytesRead, timeOut);
		if (bytesRead < 0) {
//			tprintf (TCP_errorStr		
//			TCP_displayError ("serverTCPReadAll", "serverTCPRead", bytesRead);				
			tprintf (TCP_errorStr ("serverTCPReadAll", "serverTCPRead", bytesRead));
			return bytesRead;
		}
		totalBytesRead += bytesRead;
//		*dataBuffer += bytesRead;
	}
	return totalBytesRead;
}


int serverTCPReadBuffer (char **dataBuffer, unsigned *nBytes, unsigned timeOut)
{
    char funcName[] = "serverTCPReadBuffer";
    
	if (displayProtocol) serverLEDreceive (1);
    *dataBuffer = NULL;
	if (SERVER_TCPhandle == TCP_NO_CONNECTION) goto ERRORID;
    if (serverTCPReadAll ((char *) nBytes, 4, timeOut) != 4) goto ERRORID;
	*nBytes -= 4;

	if (*nBytes > MAX_TCP_BUFFER_SIZE) {
		PostMessagePopupf ("TCP/IP error", "Function: '%s':\nReceived datastructure"
					   "is larger than %d bytes!", funcName, MAX_TCP_BUFFER_SIZE);
		goto ERRORID;
	}
	
	*dataBuffer = malloc (*nBytes);
	if (serverTCPReadAll ((char *) *dataBuffer, *nBytes, timeOut) != *nBytes) {
		PostMessagePopupf ("TCP/IP error", "Function: '%s':\nIncomplete data transfer!\n");
		goto ERRORID;
	}
	if (displayProtocol) {
		pprintf (panelServer, TCP_S_RECEIVE, "%s %s (%d bytes)\n", TimeStr(), (char *) *dataBuffer, *nBytes+4);
		serverLEDreceive (0);
		textboxCheckMaxLines (panelServer, TCP_S_RECEIVE, 500);
	}
	return 0;
	
	
	
ERRORID:
	free (*dataBuffer);
	*nBytes = 0;
	serverLEDreceive (0);
	return -10000;
}




int clientTCPReadAll (char *dataBuffer, unsigned nBytes, unsigned timeOut)
{
    unsigned totalBytesRead;
	int bytesRead;

	totalBytesRead = 0;
	while (totalBytesRead < nBytes) {
		bytesRead = ClientTCPRead (CLIENT_TCPhandle, dataBuffer+totalBytesRead, nBytes-totalBytesRead, timeOut);
		if (bytesRead < 0) {
			TCP_displayError ("clientTCPReadAll", "ClientTCPRead", bytesRead);				
			return bytesRead;
		}
		totalBytesRead += bytesRead;
	}
	return totalBytesRead;
}





int clientTCPReadBuffer (char **dataBuffer, unsigned *nBytes, unsigned timeOut)
{
    char funcName[] = "clientTCPReadBuffer";
    
	if (displayProtocol) clientLEDreceive (1);
    *dataBuffer = NULL;
    *nBytes = 0;    
    if (CLIENT_TCPhandle == TCP_NO_CONNECTION)  goto ERRORID;
    if (clientTCPReadAll ((char *) nBytes, 4, timeOut) != 4) goto ERRORID;
	*nBytes -= 4;

	if (*nBytes > MAX_TCP_BUFFER_SIZE) {
		PostMessagePopupf ("TCP/IP error", "Function: '%s':\nReceived datastructure"
					   "is larger than %d bytes!", funcName, MAX_TCP_BUFFER_SIZE);
		goto ERRORID;
	}
	
	*dataBuffer = (char *) malloc (*nBytes);
	if (clientTCPReadAll ((char *) *dataBuffer, *nBytes, timeOut) != *nBytes) {
		PostMessagePopupf ("TCP/IP error", "Function: '%s':\nIncomplete data transfer!\n");
		goto ERRORID;
	}
	if (displayProtocol) {
		pprintf (panelClient, TCP_C_RECEIVE, "%s %s (%d bytes)\n", TimeStr(), (char *) *dataBuffer, *nBytes+4);
		clientLEDreceive (0);
		textboxCheckMaxLines (panelClient, TCP_C_RECEIVE, 500);
	}
	return 0;
	
ERRORID:
	clientLEDreceive (0);
	free (*dataBuffer);
	*dataBuffer = NULL;
	*nBytes = 0;
	return -10000;
}





/*---------------------------------------------------------------------------*/
/* Respond to the UI and clear the receive screen for the user.              */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ClearScreenClientCB (int panel, int control, int event,
                               void *callbackData, int eventData1,
                               int eventData2)
{
    if (event == EVENT_COMMIT)
		ResetTextBox (panel, TCP_C_RECEIVE, "");
    return 0;
}

int CVICALLBACK ClearScreenServerCB (int panel, int control, int event,
                               void *callbackData, int eventData1,
                               int eventData2)
{
    if (event == EVENT_COMMIT)
		ResetTextBox (panel, TCP_S_RECEIVE, "");
    return 0;
}



void TCP_closeServer (void)
{
    /* Unregister the TCP server */
    UnregisterTCPServer (portNum);
    pprintf (panelServer, TCP_S_RECEIVE, "TCP/IP server closed.\n");
    SERVER_TCPhandle = TCP_NO_CONNECTION;
}
    


void TCP_closeClient (void)
{
	/* Disconnect from the TCP server */
    DisconnectFromTCPServer (CLIENT_TCPhandle);
	SetCtrlAttribute (panelClient, TCP_C_BTN_connectToServer,
					  ATTR_DIMMED, 1);
   	SetCtrlVal (panelClient, TCP_C_CONNECTED, 0);
    CLIENT_TCPhandle = TCP_NO_CONNECTION;
    pprintf (panelClient, TCP_C_RECEIVE, "Disconnected from server.\n");
}



const char *TCP_getComputerName (void)
{
	static char computerName[200];
	int error;

	error = GetTCPHostAddr (computerName, 200);
	if (error != 0) {
		TCP_displayError ("TCP_getComputerName()", "GetTCPHostAddr", error);
		return NULL;
	}
	
	return computerName;
}


const char *TCP_getServerName (void)
{
	static char serverName[200];

	if (panelClient == -1) return NULL;
	GetCtrlVal (panelClient, TCP_C_SERVER_NAME, serverName);
	if (strcmp(serverName, "") == 0) 
		strcpy (serverName, TCP_getComputerName ());
	return serverName;
}


void TCP_setServerName (char *serverName)
{
	if (panelClient != -1) 
		SetCtrlVal (panelClient, TCP_C_SERVER_NAME, serverName);
}





int CVICALLBACK ClientConnectCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			TCP_initClient (TCP_getServerName(), CLIENT_parseFunction, 1);
			break;
		}
	return 0;
}



int clientTCPWriteAll (char *buffer, unsigned bufferLen, unsigned timeout, int showError)
{
    unsigned totalBytesWritten;
	int bytesWritten;
	int error = 0; // TODO error not initialized

	totalBytesWritten = 0;
	while (totalBytesWritten < bufferLen) {
		bytesWritten = ClientTCPWrite (CLIENT_TCPhandle,
									   buffer+totalBytesWritten,
									   bufferLen-totalBytesWritten,
									   timeout);
		if (bytesWritten < 0) {
			// try to establish connection
			if (abs(bytesWritten) == kTCP_NoConnectionEstablished) {
//				error = TCP_initClient (TCP_getServerName(), CLIENT_parseFunction, showError);
//				if (error < 0) {
					if (showError) return TCP_displayError ("clientTCPWriteAll", "ClientTCPWrite", error);				
					else return bytesWritten;
//				}
				//bytesWritten = 0;
			}
			else {
				if (showError) return TCP_displayError ("clientTCPWriteAll", "ClientTCPWrite", error);				
				else return error;
			}			
		}
		totalBytesWritten += bytesWritten;
//		*dataBuffer += bytesRead;
	}
	return totalBytesWritten;
}



int TCP_clientSendBuffer (char *buffer, unsigned long bufferLen, int showErrorMsg)
{
	int error, errorConnect;
	char displayStr[TCP_MAX_COMMANDLEN] = "--> ";

	if (displayProtocol) clientLEDsend (1);
	error = clientTCPWriteAll (buffer, bufferLen, 0, 0);
    if (displayProtocol) {	
		strncpy (displayStr+4, buffer+4, TCP_MAX_COMMANDLEN-4);
		displayStr[TCP_MAX_COMMANDLEN-1] = 0;
	   	pprintf (panelClient, TCP_C_RECEIVE, "%s %s\n", TimeStr(), displayStr);
		clientLEDsend (0);
		textboxCheckMaxLines (panelClient, TCP_C_RECEIVE, 500);
	}
	if (error < 0) {
		errorConnect = TCP_initClient (TCP_getServerName(), CLIENT_parseFunction, 1);
		if (errorConnect < 0) return error;
		error = clientTCPWriteAll (buffer, bufferLen, 0, showErrorMsg);
	}
   	if (error < 0) return error;
   	return 0;
}


int serverTCPWriteAll (char *buffer, unsigned bufferLen, unsigned timeout)
{
    unsigned totalBytesWritten;
	int bytesWritten;

	totalBytesWritten = 0;
	while (totalBytesWritten < bufferLen) {
		bytesWritten = ServerTCPWrite (SERVER_TCPhandle,
									   buffer+totalBytesWritten,
									   bufferLen-totalBytesWritten,
									   timeout);
		if (bytesWritten < 0) {
			tprintf (TCP_errorStr ("serverTCPWriteAll", "ServerTCPWrite", bytesWritten));				
//			TCP_displayError ("serverTCPWriteAll", "ServerTCPWrite", bytesWritten);				
			return bytesWritten;
		}
		totalBytesWritten += bytesWritten;
//		*dataBuffer += bytesRead;
	}
	return totalBytesWritten;
}






int TCP_serverSendBuffer (char *buffer, unsigned long bufferLen)
{
	int error;
	char displayStr[TCP_MAX_COMMANDLEN] = "--> ";

	if (displayProtocol) serverLEDsend (1);
	error = serverTCPWriteAll (buffer, bufferLen, TCP_TIMEOUT_MS);
	if (displayProtocol) {
		strncpy (displayStr+4, buffer+4, TCP_MAX_COMMANDLEN-4);
		displayStr[TCP_MAX_COMMANDLEN-1] = 0;
		serverLEDsend (0);
   		pprintf (panelServer, TCP_S_RECEIVE, "%s %s (%d bytes)\n", TimeStr(), displayStr, bufferLen);
		textboxCheckMaxLines (panelServer, TCP_S_RECEIVE, 500);
   	}
   	if (error < 0) return error;
   	return 0;
}



int  TCP_displayErrorMessages (int display)
{
	int lastState;
	
	lastState = displayErrors;
	displayErrors = display;
	return lastState;
}




int TCP_clientSendStrf (char *format, ...)
{
	char helpStr[TCP_MAX_COMMANDLEN];
	unsigned len;
	
	va_list arg;

	va_start( arg, format );
    vsprintf(helpStr+4, format, arg  );
    va_end( arg );
    
    len = strlen (helpStr+4) + 5;
    memcpy (helpStr, &len, 4);

	return TCP_clientSendBuffer (helpStr, len, 1);
}



int TCP_clientSendCommand (char *commandStr)
{
	return TCP_clientSendStrf (commandStr);
}



int TCP_serverSendStrf (char *format, ...)
{
 	char helpStr[TCP_MAX_COMMANDLEN];
	unsigned len;
	
	va_list arg;

	va_start( arg, format );
    vsprintf(helpStr+4, format, arg  );
    va_end( arg );
    
    len = strlen (helpStr+4) + 5;
    memcpy (helpStr, &len, 4);

	return TCP_serverSendBuffer (helpStr, len);
}


int TCP_clientIniTransmit (IniText ini, const char *commandStr, int showErrorMsg)
{
	char *buffer;
	unsigned long bufSize;
	
	if (Ini_WriteToBuffer (ini, &buffer, &bufSize, commandStr, 1) != 0) goto ERRORID;
	
	if (TCP_clientSendBuffer (buffer, bufSize, showErrorMsg) != 0) goto ERRORID;

	free (buffer);
	return 0;
ERRORID:
	free (buffer);
	return -1;
}


/*
int TCP_serverBufferTransmit (const char *commandStr, void *buffer, unsigned bufSize)
{
	char *buffer;
	int filesize;
	unsigned long bufSize;
	
	if (Ini_WriteToBuffer (ini, &buffer, &bufSize, commandStr, 1) != 0) goto ERRORID;

	if (TCP_serverSendBuffer (buffer, bufSize) != 0) goto ERRORID;

	free (buffer);
	return 0;
ERRORID:
	free (buffer);
	return -1;
	
}
*/


int TCP_serverIniTransmit (IniText ini, const char *commandStr)
{
	char *buffer;
	unsigned long bufSize;
	
	if (Ini_WriteToBuffer (ini, &buffer, &bufSize, commandStr, 1) != 0) goto ERRORID;
	
	if (TCP_serverSendBuffer (buffer, bufSize) != 0) goto ERRORID;

	free (buffer);
	return 0;
ERRORID:
	free (buffer);
	return -1;
}





const char *TCP_eventStr (int event)
{
	//static char help[50];
	
	switch (event) {
		case TCP_CONNECT:        		return "TCP_CONNECT ";
		case TCP_DISCONNECT:      		return "TCP_DISCONNECT ";
		case TCP_DATAREADY: 		return "TCP_DATAREADY ";
		default: return "";
	}
}


/*---------------------------------------------------------------------------*/
/* This is the TCP server's TCP callback.  This function will receive event  */
/* notification, similar to a UI callback, whenever a TCP event occurs.      */
/* We'll respond to CONNECT and DISCONNECT messages and indicate to the user */
/* when a client connects to or disconnects from us.  when we have a client  */
/* connected, ee'll respond to the DATAREADY event and read in the avaiable  */
/* data from the client and display it.                                      */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ServerTCPCB (unsigned handle, int event, int error,
                             void *callbackData)
{
    char receiveBuf[256] = {0};
    char addrBuf[31];
    char *buffer;
    unsigned bufferSize;

	PROTOCOLFILE_printf ("    ServerTCPCB", TCP_eventStr (event));
    switch (event)
        {
        case TCP_CONNECT:
            if (SERVER_TCPhandle)
                {
                
                /* We already have one client, don't accept another... */
                DisconnectTCPClient (SERVER_TCPhandle);
/*                GetTCPPeerAddr (handle, addrBuf, 31);
                sprintf (receiveBuf, "-- Refusing conection request from "
                                     "%s --\n", addrBuf); 
                SetCtrlVal (g_hmainPanel, MAINPNL_RECEIVE, receiveBuf);
                DisconnectTCPClient (handle);*/
                }
//            else
//                {
                
                /* Handle this new client connection */
                SERVER_TCPhandle = handle;
                SetCtrlVal (panelServer, TCP_S_CONNECTED, 1);
                GetTCPPeerAddr (SERVER_TCPhandle, addrBuf, 31);
                SetCtrlVal (panelServer, TCP_S_CLIENT_IP, addrBuf);
                GetTCPPeerName (SERVER_TCPhandle, receiveBuf, 256);
                SetCtrlVal (panelServer, TCP_S_CLIENT_NAME, receiveBuf);
                pprintf (panelServer, TCP_S_RECEIVE, "%s --- New connection from %s ---\n",
                         TimeStr(), addrBuf); 
//                SetCtrlAttribute (panelServer, TCP_STRING, ATTR_DIMMED,
//                                  0);
                
                /* Set the disconect mode so we do not need to terminate */
                /* connections ourselves. */
				SetTCPDisconnectMode (SERVER_TCPhandle, TCP_DISCONNECT_MANUAL);
//                }
            break;
        case TCP_DATAREADY:
			if (serverTCPReadBuffer (&buffer, &bufferSize, 10000) < 0) return 0;
            
            SERVER_parseFunction (buffer, bufferSize);
            free (buffer);
			break;
        case TCP_DISCONNECT:
            if (handle == SERVER_TCPhandle) {
                /* The client we were talking to has disconnected... */
                SetCtrlVal (panelServer, TCP_S_CONNECTED, 0);
                SERVER_TCPhandle = TCP_NO_CONNECTION;
                SetCtrlVal (panelServer, TCP_S_CLIENT_IP, "");
                SetCtrlVal (panelServer, TCP_S_CLIENT_NAME, "");
                pprintf (panelServer, TCP_S_RECEIVE,
                            "%s -- Client disconnected --\n", TimeStr());
//                SetCtrlAttribute (panelServer, TCP_STRING, ATTR_DIMMED,
//                                  1);
                
                /* Note that we do not need to do any more because we set the*/
                /* disconnect mode to AUTO. */
                }
            break;
    }
    return 0;
}


/*---------------------------------------------------------------------------*/
/* This is the TCP client's TCP callback.  This function will receive event  */
/* notification, similar to a UI callback, whenever a TCP event occurs.      */
/* We'll respond to the DATAREADY event and read in the avaiable data from   */
/* the server and display it.  We'll also respond to DISCONNECT events, and  */
/* tell the user when the server disconnects us.                             */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ClientTCPCB (unsigned handle, int event, int error,
                             void *callbackData)
{
	char *buffer;
	unsigned long bufferSize;

    switch (event)
        {
        case TCP_DATAREADY:
			if (clientTCPReadBuffer (&buffer, &bufferSize, 10000) < 0) return 0;
            CLIENT_parseFunction (buffer, bufferSize);
            free (buffer);
			break;

/*        case TCP_DATAREADY:
            if ((dataSize = ClientTCPRead (CLIENT_TCPhandle, receiveBuf,
                                           dataSize, 1000))
                < 0)
                TCP_displayError ("ClientTCPCB", "ClientTCPRead", dataSize);
            else {
                pprintf (panelClient, TCP_C_RECEIVE, "%s %s\n", TimeStr(), receiveBuf);
			}
			break;
*/        case TCP_DISCONNECT:
//            PostMessagePopup ("TCP Client", "Server has closed connection!");
    		pprintf (panelClient, TCP_C_RECEIVE, "%s --- Server has closed connection! ---\n", TimeStr());
			TCP_setClientCtrlsVisible (0);
            CLIENT_TCPhandle = TCP_NO_CONNECTION;
            break;
    }
    return 0;
}




int TCP_serverOnSameComputer (void)
{
	char serverName[200] = "";
	GetCtrlVal (panelClient, TCP_C_SERVER_NAME, serverName);
	return (CompareStrings (serverName, 0, strLocalhost, 0, 0) == 0);
}

