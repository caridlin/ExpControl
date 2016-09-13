/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2005. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                           1
#define  PANEL_COMMANDBUTTON_trans_3     2       /* callback function: callback_STOP */
#define  PANEL_COMMANDBUTTON_trans_2     3       /* callback function: callback_F9 */
#define  PANEL_COMMANDBUTTON_quit        4       /* callback function: callback_quit */
#define  PANEL_STRING_status             5


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK callback_F9(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK callback_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK callback_STOP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
