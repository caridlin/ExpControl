
/****************************************************************************/
/* CLIENT.C: All the data and code pertaining to the client are contained   */
/* in this module. It consists mostly of user interface control callbacks   */
/* where appropriate responses to user events are generated.                */
/****************************************************************************/


/****************************************************************************/
/*                          Include Files                                   */
/****************************************************************************/

#include <ddesupp.h>    /* CF_TEXT, DDE_DISCONNECT, DDE_DATAREADY,
                           ConnectToDDEServer(), DisconnectFromDDEServer(),
                           SetUpDDEHotLink(), TerminateDDELink(),
                           ClientDDERead(), ClientDDEWrite(), ClientDDEExecute() */
#include <userint.h>
#include <ansi_c.h>
#include <utility.h>
//#include "server.h"     /* TRUE, FALSE, NAME, TOPIC, dde item names */
//#include "srvclt.h"     /* include file for the user-interface resource file */


#define NAME        "Test Server"   /*  name under which server will be registered */
#define TOPIC       "DDE Test"      /*  topic of the conversation */
#define ALL_CELLS   "AllCells"      /*  data item used for hot link (advisory
                                        loop). The data consists of an array of 20
                                        doubles, being controlled by the numerics on the
                                        server's front panel */

#define NUM_CELLS   20              /*  number of data cells on the server panel    */
#define CELL        "CELL"          /*  to read/write the value of a server cell,   */
                                    /*  specify the cell with the string "CELLx"    */
                                    /*  (where x is the cell number).               */


#define CELL1   "CELL1"
#define CELL2   "CELL2"             /*  data items which the client can read    */
#define CELL3   "CELL3"             /*  and write.  Each item is the the value  */
#define CELL4   "CELL4"             /*  of one of the numerics on the server    */
#define CELL5   "CELL5"             /*  panel.  The data type of the values is  */
#define CELL6   "CELL6"             /*  double (four bytes)                     */
#define CELL7   "CELL7"
#define CELL8   "CELL8"
#define CELL9   "CELL9"
#define CELL10  "CELL10"
#define CELL11  "CELL11"
#define CELL12  "CELL12"
#define CELL13  "CELL13"
#define CELL14  "CELL14"
#define CELL15  "CELL15"
#define CELL16  "CELL16"
#define CELL17  "CELL17"
#define CELL18  "CELL18"
#define CELL19  "CELL19"
#define CELL20  "CELL20"


    /* Commands supported by the server: */
#define CMD_GENERATED_DATA  "GenerateData"
#define CMD_ADD_DATA        "AddData"
#define CMD_SHIFT_LEFT      "ShiftRight"
#define CMD_SHIFT_RIGHT     "ShiftLeft"
#define CMD_QUIT            "Quit"


#define DDE_TIMEOUT        5000     /* timeout threshold (in milliseconds) for
                                       DDE message requests which require it */

#define TRUE    1
#define FALSE   0


/****************************************************************************/
/*                          Global Variables                                */
/****************************************************************************/

static int      panel;  /* panel - client user interface panel handle */
static unsigned convID; /* convID - the client's conversation handle for the DDE connection */


/****************************************************************************/
/*                          Function Prototypes                             */
/****************************************************************************/

int CVICALLBACK ClientDDECallback (unsigned int handle, char *topicName, char *itemName, int xType,
    int dataFmt, int dataSize, void *dataPtr, void *callbackData);


/****************************************************************************/
/*                          Function Definitions                            */
/****************************************************************************/

    /*   Initializes this client program */
void DDE_initClient (void)
{
//    panel = LoadPanel (0, "srvclt.uir", CLIENT);
    //DisplayPanel (panel);   /* Display the client's user interface */

        /* establishes connection with server */
    ConnectToDDEServer (&convID, NAME, TOPIC, ClientDDECallback, 0);

        /* sets up a hot link to the values exported by the server program */
    SetUpDDEHotLink (convID, ALL_CELLS, CF_TEXT, DDE_TIMEOUT);
}

/****************************************************************************/

    /* Performs the client's termination tasks. */
void DeactivateClient (void)
{
    DisableBreakOnLibraryErrors();      /* in case server is no longer there */
    DisconnectFromDDEServer (convID);   /* disconnects from server */
    EnableBreakOnLibraryErrors();
    DiscardPanel (panel);               /* discards the client's user interface */
    return;
}

/****************************************************************************/

    /*  Callback for the 'Hot Link Enabled' check box. Turns the hot link
        on or  off, depending on the value of the check box.
    */
int CVICALLBACK HotLinkCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int enabled = 0;

    if (event == EVENT_COMMIT)
        {
//        GetCtrlVal (panel, CLIENT_HOTLINKENABLED, &enabled);
        if (enabled)
            SetUpDDEHotLink (convID, ALL_CELLS, CF_TEXT, DDE_TIMEOUT);
        else
            TerminateDDELink (convID, ALL_CELLS, CF_TEXT, DDE_TIMEOUT);
        }

    return FALSE;
}

/******************************************************************************/

    /*  Callback for the 'Read Cell Value' button. Reads the value of the
        cell specified in the 'Which Cell' popup ring from the server, and
        sets the value of the 'Value' numeric accordingly.
    */
int CVICALLBACK ReadCellCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char    itemName[20] = "";
    double  cellValue;

    if (event == EVENT_COMMIT)
        {
//        GetCtrlVal (panel, CLIENT_WHICHCELL, itemName); /* The value of each item in the popup ring is the DDE item name understood by the server */
        ClientDDERead (convID, itemName, CF_TEXT, &cellValue, sizeof (cellValue), DDE_TIMEOUT); /* read the value from the server */
        //SetCtrlVal (panel, CLIENT_VALUE, cellValue);    /* displays the newly read value */
        }

    return FALSE;
}

/******************************************************************************/

    /*  Callback for the 'Write Cell Value' button. Writes the value in the
        'Value' numeric control to the server cell specified in the
        'Which Cell' popup ring.
    */
int CVICALLBACK WriteCellCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char    itemName[20] = "";
    double  cellValue;

    if (event == EVENT_COMMIT)
        {
//        GetCtrlVal (panel, CLIENT_WHICHCELL, itemName); /* The value of each item in this popup ring is the DDE item name understood by the server */
//        GetCtrlVal (panel, CLIENT_VALUE, &cellValue);   /* get the value to write */
        ClientDDEWrite (convID, itemName, CF_TEXT, &cellValue, sizeof (cellValue), DDE_TIMEOUT);
        }

    return FALSE;
}

/******************************************************************************/

    /*  Callback for the 'Send Command To Server' button.  Send the command
        specified in the 'Command' popup ring to the server.
    */
int CVICALLBACK SendCommandCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char    commandName[200] = "";

    if (event == EVENT_COMMIT)
        {
            /*  The value of each label/value pair in the CLIENT_COMMAND popup ring
                control is the DDE command string expected by the server */
//        GetCtrlVal(panel, CLIENT_COMMAND, commandName);
        ClientDDEExecute (convID, commandName, DDE_TIMEOUT);   /* sends command message to server */
        }

    return FALSE;
}

/****************************************************************************/

    /*  DDE client callback. This function receives and handles all messages
        sent by the server. The type of message is determined by the 'xType'
        parameter.
    */
int CVICALLBACK ClientDDECallback (unsigned int handle, char *topicName, char *itemName, int xType,
    int dataFmt, int dataSize, void *dataPtr, void *callbackData)
{
    if (handle != convID)       /* checks message validity. Only acknowledges */
        return FALSE;           /* messages pertaining to one conversation (convID) */


    switch (xType)              /* determines message type */
        {
        case DDE_DISCONNECT:
            convID = -1;            /* kills conversation when server unregisters */
            QuitUserInterface (0);  /* go ahead a quit the demo program as well */
            break;

        case DDE_DATAREADY:
            if (strcmp(itemName, ALL_CELLS)
                || dataFmt != CF_TEXT
                || strcmp (topicName, TOPIC))
                return FALSE;   /* checks item, data format and topic validity */

                /* plots new data values from the server */
/*            DeleteGraphPlot (panel, CLIENT_GRAPH, -1, VAL_DELAYED_DRAW);   /* remove any previous plots 
           PlotWaveform (panel, CLIENT_GRAPH, (double *)dataPtr, NUM_CELLS,
                        VAL_DOUBLE, 1.0, 0, 1, 1.0, VAL_CONNECTED_POINTS,
                        VAL_DOTTED_SOLID_DIAMOND, VAL_SOLID, 1, VAL_RED);
*/            break;

        default:
            return FALSE;
        }

    return TRUE;        /* TRUE --> client processed message successfully */
}

/***********************************************************************************/

    /*  This callback handles the 'Close' system-menu item on the client panel.
    */
int CVICALLBACK ClientPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface(0);   /* if close is selected from the system menu, quit.. */

    return 0;
}

/***********************************************************************************/

    /* This callback function handles the 'Auto Scale Graph' check box on the client panel.
    */
int CVICALLBACK AutoScaleCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
        {
        int checkBoxState = 0;

//            GetCtrlVal(panel, CLIENT_AUTOSCALEGRAPH, &checkBoxState);
//            SetAxisRange (panel, CLIENT_GRAPH, VAL_NO_CHANGE, 0.0, 1.0,
//                          checkBoxState ? VAL_AUTOSCALE : VAL_LOCK, 0.0, 1.0);
        }

    return 0;
}
