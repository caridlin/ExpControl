/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2006. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                           1       /* callback function: PANEL_callback */
#define  PANEL_TEXTMSG_release           2
#define  PANEL_TEXTMSG_Expc              3
#define  PANEL_PICTURE                   4
#define  PANEL_TIMER                     5       /* callback function: TIMER_callback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK PANEL_callback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
