/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2004. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  COMMANDS                        1
#define  COMMANDS_TEXTBOX                2
#define  COMMANDS_COMMANDBUTTON_F9       3       /* callback function: callback_F9 */
#define  COMMANDS_STRING                 4
#define  COMMANDS_COMMANDBUTTON          5       /* callback function: callback_Send */

#define  DEMO                            2
#define  DEMO_COMMANDBUTTON_Quit         2       /* callback function: Quit */
#define  DEMO_CANVAS                     3


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK callback_F9(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK callback_Send(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
