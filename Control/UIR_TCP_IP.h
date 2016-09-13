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

#define  TCP_C                           1
#define  TCP_C_RECEIVE                   2
#define  TCP_C_CONNECTED                 3
#define  TCP_C_CLEAR                     4       /* callback function: ClearScreenClientCB */
#define  TCP_C_CLIENT_NAME               5
#define  TCP_C_CLIENT_IP                 6
#define  TCP_C_SERVER_NAME               7
#define  TCP_C_SERVER_IP                 8
#define  TCP_C_DECORATION                9
#define  TCP_C_DECORATION_2              10
#define  TCP_C_BTN_connectToServer       11      /* callback function: ClientConnectCB */

#define  TCP_S                           2
#define  TCP_S_RECEIVE                   2
#define  TCP_S_CONNECTED                 3
#define  TCP_S_ONLINE                    4
#define  TCP_S_CLEAR                     5       /* callback function: ClearScreenServerCB */
#define  TCP_S_CLIENT_NAME               6
#define  TCP_S_CLIENT_IP                 7
#define  TCP_S_SERVER_NAME               8
#define  TCP_S_SERVER_IP                 9
#define  TCP_S_DECORATION                10
#define  TCP_S_DECORATION_2              11


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK ClearScreenClientCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClearScreenServerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClientConnectCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
