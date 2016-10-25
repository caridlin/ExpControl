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

#define  ALERT                           1       /* callback function: ALERT_Panel_CB */
#define  ALERT_PICTURE_watchdog          2
#define  ALERT_TEXTMSG                   3

#define  CHANNEL                         2
#define  CHANNEL_DECORATION_frame        2
#define  CHANNEL_COMMANDBUTTON_relock    3       /* callback function: CANNEL_reset_CB */
#define  CHANNEL_BUTTON_configure        4       /* callback function: CHANNEL_configure_CB */
#define  CHANNEL_RADIOBUTTON_watch       5       /* callback function: CHANNEL_watch_CB */
#define  CHANNEL_LED_status              6
#define  CHANNEL_GRAPH                   7
#define  CHANNEL_STRING_name             8
#define  CHANNEL_TEXTMSG_channel         9

#define  CHCONF                          3
#define  CHCONF_STRING_name              2
#define  CHCONF_STRING_module            3
#define  CHCONF_STRING_channel           4
#define  CHCONF_NUMERIC_interval         5       /* callback function: CHANNEL_editParams_CB */
#define  CHCONF_NUMERIC_averages         6       /* callback function: CHANNEL_editParams_CB */
#define  CHCONF_NUMERIC_nPoints          7       /* callback function: CHANNEL_editParams_CB */
#define  CHCONF_RINGSLIDE_mode           8       /* callback function: CHANNEL_editParams_CB */
#define  CHCONF_NUMERIC_thresholdUp      9
#define  CHCONF_NUMERIC_thresholdLow     10
#define  CHCONF_NUMERIC_thrAverages      11
#define  CHCONF_NUMERIC_thresholdDiff    12
#define  CHCONF_COMMANDBUTTON_abort      13      /* callback function: CHANNEL_config_abort_CB */
#define  CHCONF_COMMANDBUTTON_done       14      /* callback function: CHANNEL_config_done_CB */
#define  CHCONF_TEXTMSG                  15
#define  CHCONF_TEXTMSG_3                16
#define  CHCONF_TEXTMSG_V3               17
#define  CHCONF_TEXTMSG_V1               18
#define  CHCONF_TEXTMSG_V2               19
#define  CHCONF_TEXTMSG_points           20
#define  CHCONF_NUMERIC_channel          21
#define  CHCONF_RADIOBUTTON              22
#define  CHCONF_BTN_MonitorCh            23      /* callback function: CHANNEL_config_channelMode_CB */
#define  CHCONF_BTN_watchdogCh           24      /* callback function: CHANNEL_config_channelMode_CB */
#define  CHCONF_NUMERIC_saveMinutes      25
#define  CHCONF_TEXTMSG_2                26
#define  CHCONF_STRING_saveFilename      27
#define  CHCONF_BTN_browseFilename       28
#define  CHCONF_NUMERIC_scaleToUnit      29
#define  CHCONF_NUMERIC_scaleVolts       30
#define  CHCONF_STRING_scaleUnit         31
#define  CHCONF_BTN_changeUnits          32
#define  CHCONF_TEXTMSG_min2             33
#define  CHCONF_STRING_time              34

#define  CONFIG                          4
#define  CONFIG_DECORATION               2
#define  CONFIG_STRING_soundFilename     3
#define  CONFIG_STRING_dataSocket        4
#define  CONFIG_STRING_resource          5
#define  CONFIG_COMMANDBUTTON_Browse     6       /* callback function: CONFIG_callback_browseIAK */
#define  CONFIG_RADIOBUTTON_sound        7
#define  CONFIG_RADIOBUTTON_beeper       8       /* callback function: CONFIG_editActive_CB */
#define  CONFIG_STRING_moduleBeeper      9
#define  CONFIG_STRING_chBeeper          10
#define  CONFIG_DECORATION_4             11
#define  CONFIG_COMMANDBUTTON_delete     12      /* callback function: CONFIG_deleteChannel_CB */
#define  CONFIG_COMMANDBUTTON_config     13      /* callback function: CONFIG_configureChannel_CB */
#define  CONFIG_TREE_channels            14      /* callback function: CONFIG_tree_CB */
#define  CONFIG_DECORATION_3             15
#define  CONFIG_NUMERIC_channelWidth     16
#define  CONFIG_COMMANDBUTTON_abort      17      /* callback function: CONFIG_abort_CB */
#define  CONFIG_COMMANDBUTTON_apply      18      /* callback function: CONFIG_apply_CB */
#define  CONFIG_COMMANDBUTTON_done       19      /* callback function: CONFIG_done_CB */
#define  CONFIG_STRING_iak               20
#define  CONFIG_TEXTMSG_4                21
#define  CONFIG_NUMERIC_channelHeight    22
#define  CONFIG_COMMANDBUTTON_new        23      /* callback function: CONFIG_newChannel_CB */
#define  CONFIG_TEXTMSG_3                24
#define  CONFIG_TEXTMSG_2                25
#define  CONFIG_TEXTMSG_5                26
#define  CONFIG_BTN_playSound            27      /* callback function: CONFIG_playSound_CB */
#define  CONFIG_BTN_soundBrowse          28      /* callback function: CONFIG_browse_CB */
#define  CONFIG_DECORATION_5             29

#define  MAIN                            5       /* callback function: WATCHDOG_panel_CB */
#define  MAIN_LED_unlocked               2
#define  MAIN_LED_received               3       /* callback function: WATCHDOG_LED_received_CB */
#define  MAIN_BINARYSWITCH_Beep          4       /* callback function: WATCHDOG_beep_CB */
#define  MAIN_COMMANDBUTTON_config       5       /* callback function: WATCHDOG_editConfig_CB */
#define  MAIN_BUTTON_reset               6       /* callback function: WATCHDOG_reset_CB */
#define  MAIN_TIMER                      7       /* callback function: TIMER_callback */
#define  MAIN_TEXTBOX                    8
#define  MAIN_COMMANDBUTTON              9       /* callback function: WATCHDOG_connectToServerCB_CB */
#define  MAIN_TEXTBOX_status             10
#define  MAIN_PICTURE_watchdog           11
#define  MAIN_TIMER_sound                12      /* callback function: TIMER_sound_CB */
#define  MAIN_TIMER_checkConnection      13      /* callback function: TIMER_checkConnection_CB */

#define  MONITOR                         6       /* callback function: CHANNEL_monitor_panelCallback */
#define  MONITOR_COMMANDBUTTON_quit      2       /* callback function: CHANNEL_quit_CB */
#define  MONITOR_GRAPH                   3
#define  MONITOR_BUTTON_configure        4       /* callback function: CHANNEL_configure_CB */
#define  MONITOR_NUMERIC_volts           5
#define  MONITOR_NUMERIC_value           6
#define  MONITOR_TEXTMSG_unit            7


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK ALERT_Panel_CB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CANNEL_reset_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_config_abort_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_config_channelMode_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_config_done_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_configure_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_editParams_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_monitor_panelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_quit_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANNEL_watch_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_abort_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_apply_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_browse_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_callback_browseIAK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_configureChannel_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_deleteChannel_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_done_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_editActive_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_newChannel_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_playSound_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_tree_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_checkConnection_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_sound_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_beep_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_connectToServerCB_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_editConfig_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_LED_received_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_panel_CB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WATCHDOG_reset_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
