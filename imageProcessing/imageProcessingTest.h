/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2010. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  IMAGE                            1
#define  IMAGE_COMMANDBUTTON              2       /* callback function: IMAGES_quit_CB */
#define  IMAGE_GRAPH                      3
#define  IMAGE_NUMERIC_nPic               4
#define  IMAGE_TEXTBOX                    5
#define  IMAGE_NUMERIC_phi2               6       /* callback function: IMAGE_changeParameters_CB */
#define  IMAGE_NUMERIC_phi1               7       /* callback function: IMAGE_changeParameters_CB */
#define  IMAGE_NUMERIC_theta2             8       /* callback function: IMAGE_changeParameters_CB */
#define  IMAGE_NUMERIC_theta1             9       /* callback function: IMAGE_changeParameters_CB */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK IMAGE_changeParameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IMAGES_quit_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
