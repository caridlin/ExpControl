/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  BITMAPS                          1
#define  BITMAPS_PICTURE_no               2       /* control type: picture, callback function: (none) */
#define  BITMAPS_PICTURE_yes              3       /* control type: picture, callback function: (none) */

#define  POPUP_Incr                       2
#define  POPUP_Incr_NUMERIC_increment     2       /* control type: numeric, callback function: (none) */
#define  POPUP_Incr_COMMANDBUTTON_done    3       /* control type: command, callback function: POINTS_POPUP_incr_done */
#define  POPUP_Incr_COMMANDBUTTON_abort   4       /* control type: command, callback function: POINTS_POPUP_incr_done */
#define  POPUP_Incr_TEXTMSG               5       /* control type: textMsg, callback function: (none) */

#define  TMENU                            3


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  TABLE                            1
#define  TABLE_MENU                       2
#define  TABLE_MENU_INSERT_ABOVE          3
#define  TABLE_MENU_INSERT_BELOW          4
#define  TABLE_MENU_SEPARATOR             5
#define  TABLE_MENU_DELETE                6
#define  TABLE_MENU_SEPARATOR_2           7
#define  TABLE_MENU_VARTIME_ON            8
#define  TABLE_MENU_VARTIME_OFF           9
#define  TABLE_MENU_SEPARATOR_3           10
#define  TABLE_MENU_VARVALUE_ON           11
#define  TABLE_MENU_VARVALUE_OFF          12
#define  TABLE_MENU_SEPARATOR_4           13
#define  TABLE_MENU_INCTIME               14


     /* Callback Prototypes: */

int  CVICALLBACK POINTS_POPUP_incr_done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
