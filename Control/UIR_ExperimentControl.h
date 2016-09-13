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

#define  ADDRESS                          1       /* callback function: ADDRSIM_panel_CB */
#define  ADDRESS_GRAPH3                   2       /* control type: graph, callback function: (none) */
#define  ADDRESS_GRAPH2                   3       /* control type: graph, callback function: (none) */
#define  ADDRESS_GRAPH1                   4       /* control type: graph, callback function: WFM_ADDRESS_GRAPH1_CB */
#define  ADDRESS_TEXTBOX                  5       /* control type: textBox, callback function: (none) */
#define  ADDRESS_COMMANDBUTTON_test       6       /* control type: command, callback function: WFM_ADDR_test */
#define  ADDRESS_GRAPH_voltages           7       /* control type: graph, callback function: (none) */
#define  ADDRESS_TEXTBOX2                 8       /* control type: textBox, callback function: (none) */
#define  ADDRESS_TEXTMSG                  9       /* control type: textMsg, callback function: (none) */

#define  CCD                              2
#define  CCD_CHECKBOX_acquisition         2       /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_enable              3       /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_serialNo             4       /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_STRING_description           5       /* control type: string, callback function: CCD_editParameters_CB */
#define  CCD_RING_cameraType              6       /* control type: ring, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_enableEMCCD         7       /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_EMCCDgain            8       /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_readoutRate          9       /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_shiftSpeed           10      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_preAmpGain           11      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_nPictures            12      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_exposure             13      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_subimage            14      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_centerX              15      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_centerY              16      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_sizeX                17      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_sizeY                18      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_RING_binning                 19      /* control type: ring, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_evalSubimage        20      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_centerX_2            21      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_centerY_2            22      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_sizeX_2              23      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_sizeY_2              24      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_saveRaw             25      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_sumPictures         26      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_pixelCount          27      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_CHECKBOX_separateFile        28      /* control type: radioButton, callback function: CCD_editParameters_CB */
#define  CCD_RING_imageProcessing         29      /* control type: ring, callback function: CCD_editParameters_CB */
#define  CCD_COMMANDBUTTON_removeC        30      /* control type: command, callback function: CCD_removeCommandFile_CB */
#define  CCD_COMMANDBUTTON_write          31      /* control type: command, callback function: CCD_writeCommandFile_CB */
#define  CCD_TEXTMSG1                     32      /* control type: textMsg, callback function: (none) */
#define  CCD_COMMANDBUTTON_remove         33      /* control type: command, callback function: CCD_removeCamera_CB */
#define  CCD_BINARYSWITCH_shutter         34      /* control type: binary, callback function: CCD_editParameters_CB */
#define  CCD_BINARYSWITCH_cooler          35      /* control type: binary, callback function: CCD_editParameters_CB */
#define  CCD_RINGSLIDE_fanMode            36      /* control type: slide, callback function: CCD_editParameters_CB */
#define  CCD_NUMERIC_temperature          37      /* control type: numeric, callback function: CCD_editParameters_CB */
#define  CCD_TEXTMSG_deg                  38      /* control type: textMsg, callback function: (none) */
#define  CCD_COMMANDBUTTON_send           39      /* control type: command, callback function: CCD_sendCommandFile_CB */
#define  CCD_CHECKBOX_FIRST               40      /* control type: radioButton, callback function: (none) */
#define  CCD_CHECKBOX_EVERY               41      /* control type: radioButton, callback function: (none) */

#define  CONFIG                           3       /* callback function: CONFIG_panelChanged */
#define  CONFIG_BUTTON_abort              2       /* control type: command, callback function: CONFIG_BTN_abort_CB */
#define  CONFIG_BUTTON_done               3       /* control type: command, callback function: BTN_Setup_Done */
#define  CONFIG_BUTTON_applyChanges       4       /* control type: command, callback function: BTN_Setup_Done */
#define  CONFIG_TAB                       5       /* control type: tab, callback function: (none) */
#define  CONFIG_RADIOBUTTON_enableNI      6       /* control type: radioButton, callback function: (none) */

#define  DEBUGOUT                         4       /* callback function: DEBUGOUT_callback */
#define  DEBUGOUT_TEXTBOX                 2       /* control type: textBox, callback function: (none) */

#define  DEVICE                           5
#define  DEVICE_RING_triggerSlope         2       /* control type: ring, callback function: (none) */
#define  DEVICE_COMMANDBUTTON_Done        3       /* control type: command, callback function: CONFIG_GPIB_doneDeviceSettings */
#define  DEVICE_COMMANDBUTTON_Abort       4       /* control type: command, callback function: CONFIG_GPIB_abortDeviceSettings */
#define  DEVICE_RING_triggerSignal        5       /* control type: ring, callback function: (none) */
#define  DEVICE_TEXTMSG_3                 6       /* control type: textMsg, callback function: (none) */
#define  DEVICE_NUMERIC_gpibAddress       7       /* control type: numeric, callback function: (none) */
#define  DEVICE_RING_load                 8       /* control type: ring, callback function: CONGIG_GPIB_deviceSettingsChanged */
#define  DEVICE_DECORATION_7              9       /* control type: deco, callback function: (none) */
#define  DEVICE_DECORATION_5              10      /* control type: deco, callback function: (none) */
#define  DEVICE_TEXTMSG_Output            11      /* control type: textMsg, callback function: (none) */
#define  DEVICE_TEXTMSG_Clock_2           12      /* control type: textMsg, callback function: (none) */
#define  DEVICE_NUMERIC_maxVoltage        13      /* control type: numeric, callback function: CONGIG_GPIB_deviceSettingsChanged */
#define  DEVICE_TEXTMSG_5                 14      /* control type: textMsg, callback function: (none) */
#define  DEVICE_STRING_deviceName         15      /* control type: string, callback function: (none) */

#define  EXTDEVSET                        6
#define  EXTDEVSET_COMMANDBUTTON_abort    2       /* control type: command, callback function: EXTDEV_abort_CB */
#define  EXTDEVSET_COMMANDBUTTON_done     3       /* control type: command, callback function: EXTDEV_done_CB */
#define  EXTDEVSET_TAB_devtype            4       /* control type: tab, callback function: (none) */
#define  EXTDEVSET_STRING_NAME            5       /* control type: string, callback function: (none) */

#define  IDLEWAIT                         7
#define  IDLEWAIT_NUMERICSLIDE            2       /* control type: scale, callback function: (none) */
#define  IDLEWAIT_BTN_skipWaitForIdle     3       /* control type: command, callback function: BTN_skipWaitingForIdle_CB */
#define  IDLEWAIT_BTN_stopSequence        4       /* control type: command, callback function: BTN_stopWaitingForIdle_CB */

#define  IMPORT                           8
#define  IMPORT_RING_sequences            2       /* control type: ring, callback function: IMPORT_SEQUENCE_changed */
#define  IMPORT_STRING_filename           3       /* control type: string, callback function: (none) */
#define  IMPORT_COMMANDBUTTON_browse      4       /* control type: command, callback function: IMPORT_BTN_Browse */
#define  IMPORT_LISTBOX_ccdSettings       5       /* control type: listBox, callback function: (none) */
#define  IMPORT_LISTBOX_gpibCommands      6       /* control type: listBox, callback function: (none) */
#define  IMPORT_LISTBOX_Waveforms         7       /* control type: listBox, callback function: (none) */
#define  IMPORT_CHECKBOX_ChannelNames     8       /* control type: radioButton, callback function: (none) */
#define  IMPORT_CHECKBOX_BoardParams      9       /* control type: radioButton, callback function: (none) */
#define  IMPORT_COMMANDBUTTON_Done        10      /* control type: command, callback function: IMPORT_BTN_Done */
#define  IMPORT_COMMANDBUTTON_Abort       11      /* control type: command, callback function: IMPORT_BTN_Abort */

#define  LOOP                             9
#define  LOOP_NUMERIC_nReps               2       /* control type: numeric, callback function: (none) */
#define  LOOP_COMMANDBUTTON_done          3       /* control type: command, callback function: BLOCK_loopDone */
#define  LOOP_COMMANDBUTTON_abort         4       /* control type: command, callback function: BLOCK_loopAbort */

#define  MAIN                             10      /* callback function: MAIN_panelChanged */
#define  MAIN_STRING_running_seq          2       /* control type: string, callback function: (none) */
#define  MAIN_STRING_sequence             3       /* control type: string, callback function: (none) */
#define  MAIN_NUMERIC_nTotalRep           4       /* control type: numeric, callback function: (none) */
#define  MAIN_NUMERIC_totalRun            5       /* control type: numeric, callback function: (none) */
#define  MAIN_NUMERICSLIDE_progress       6       /* control type: scale, callback function: (none) */
#define  MAIN_NUMERIC_nRep                7       /* control type: scale, callback function: (none) */
#define  MAIN_NUMERIC_run                 8       /* control type: scale, callback function: (none) */
#define  MAIN_LED_gpib                    9       /* control type: LED, callback function: (none) */
#define  MAIN_STRING_gpibCommand          10      /* control type: string, callback function: (none) */
#define  MAIN_LED_TCP_receive             11      /* control type: LED, callback function: (none) */
#define  MAIN_LED_TCP_send                12      /* control type: LED, callback function: (none) */
#define  MAIN_DECORATION_5                13      /* control type: deco, callback function: (none) */
#define  MAIN_DECORATION_4                14      /* control type: deco, callback function: (none) */
#define  MAIN_TEXTMSG                     15      /* control type: textMsg, callback function: (none) */
#define  MAIN_LED_watchdog                16      /* control type: LED, callback function: (none) */
#define  MAIN_STRING_remaining            17      /* control type: string, callback function: (none) */
#define  MAIN_STRING_elapsed              18      /* control type: string, callback function: (none) */
#define  MAIN_STRING_durationTotal        19      /* control type: string, callback function: (none) */
#define  MAIN_STRING_durationSeq          20      /* control type: string, callback function: (none) */
#define  MAIN_TREE_sweeps                 21      /* control type: tree, callback function: (none) */
#define  MAIN_RADIOBUTTON_suppress        22      /* control type: radioButton, callback function: OUTPUT_suppressChanged_CB */
#define  MAIN_COMMANDBUTTON_boot          23      /* control type: command, callback function: CONFIG_BTN_adwinBoot */
#define  MAIN_RADIOBUTTON_autoSave        24      /* control type: radioButton, callback function: MAIN_autosaveSequences */
#define  MAIN_NUMERIC_currentStart        25      /* control type: numeric, callback function: (none) */
#define  MAIN_TEXTBOX_DIOTimebase         26      /* control type: string, callback function: (none) */
#define  MAIN_NUMERIC_nextStart           27      /* control type: numeric, callback function: (none) */
#define  MAIN_TEXTBOX_AOTimebase          28      /* control type: string, callback function: (none) */
#define  MAIN_RING_TriggerSource          29      /* control type: ring, callback function: SEQUENCE_parametersChanged */
#define  MAIN_NUMERIC_seqDuration         30      /* control type: numeric, callback function: (none) */
#define  MAIN_STRING_activeSeqPath        31      /* control type: string, callback function: SEQUENCE_parametersChanged */
#define  MAIN_STRING_seqShortName         32      /* control type: string, callback function: SEQUENCE_parametersChanged */
#define  MAIN_TEXTMSG_ms                  33      /* control type: textMsg, callback function: (none) */
#define  MAIN_BTN_stopSequence            34      /* control type: command, callback function: BTN_stopSequence */
#define  MAIN_BTN_startSequence           35      /* control type: command, callback function: BTN_StartSequence */
#define  MAIN_NUM_copies                  36      /* control type: numeric, callback function: SEQUENCE_parametersChanged */
#define  MAIN_COMMANDBUTTON_remote        37      /* control type: command, callback function: BTN_remoteStart */
#define  MAIN_LISTBOX_SequenceQueue       38      /* control type: listBox, callback function: Sequence_Queue_Clicked */
#define  MAIN_LISTBOX_Sequences           39      /* control type: listBox, callback function: Sequence_List_Clicked */
#define  MAIN_NUM_repetitions             40      /* control type: numeric, callback function: SEQUENCE_parametersChanged */
#define  MAIN_TEXTBOX_Output2             41      /* control type: textBox, callback function: (none) */
#define  MAIN_TEXTBOX_Output1             42      /* control type: textBox, callback function: (none) */
#define  MAIN_TIMER_autoQuit              43      /* control type: timer, callback function: MAIN_Timer_autoQuit */
#define  MAIN_TEXTMSG_2                   44      /* control type: textMsg, callback function: (none) */
#define  MAIN_DECORATION                  45      /* control type: deco, callback function: (none) */
#define  MAIN_DECORATION_2                46      /* control type: deco, callback function: (none) */
#define  MAIN_BUTTON_showSweptPars        47      /* control type: command, callback function: MAIN_showSweepParameter_CB */
#define  MAIN_COMMANDBUTTON_top2          48      /* control type: pictButton, callback function: SEQUENCE_moveInQueue_CB */
#define  MAIN_COMMANDBUTTON_bottom2       49      /* control type: pictButton, callback function: SEQUENCE_moveInQueue_CB */
#define  MAIN_COMMANDBUTTON_down2         50      /* control type: pictButton, callback function: SEQUENCE_moveInQueue_CB */
#define  MAIN_COMMANDBUTTON_up2           51      /* control type: pictButton, callback function: SEQUENCE_moveInQueue_CB */
#define  MAIN_COMMANDBUTTON_top           52      /* control type: pictButton, callback function: SEQUENCE_moveInList_CB */
#define  MAIN_COMMANDBUTTON_bottom        53      /* control type: pictButton, callback function: SEQUENCE_moveInList_CB */
#define  MAIN_COMMANDBUTTON_down          54      /* control type: pictButton, callback function: SEQUENCE_moveInList_CB */
#define  MAIN_COMMANDBUTTON_up            55      /* control type: pictButton, callback function: SEQUENCE_moveInList_CB */
#define  MAIN_NUMERIC_of                  56      /* control type: numeric, callback function: (none) */
#define  MAIN_TEXTMSG_3                   57      /* control type: textMsg, callback function: (none) */
#define  MAIN_TEXTMSG_4                   58      /* control type: textMsg, callback function: (none) */
#define  MAIN_TEXTBOX_description         59      /* control type: textBox, callback function: SEQUENCE_parametersChanged */
#define  MAIN_TEXTMSG_5                   60      /* control type: textMsg, callback function: (none) */
#define  MAIN_COMMANDBUTTON_save          61      /* control type: command, callback function: MAIN_saveSequenceNow_CB */
#define  MAIN_BTN_stopIdle                62      /* control type: command, callback function: MAIN_stopIdle_CB */
#define  MAIN_BTN_startIdle               63      /* control type: command, callback function: MAIN_startIdle_CB */
#define  MAIN_NUMERIC_offset              64      /* control type: numeric, callback function: MAIN_ADWIN_testOffsetSend_CB */
#define  MAIN_NUMERIC_channel             65      /* control type: numeric, callback function: MAIN_ADWIN_testOffsetSend_CB */
#define  MAIN_RADIOBUTTON_calib           66      /* control type: radioButton, callback function: (none) */
#define  MAIN_TEXTMSG_6                   67      /* control type: textMsg, callback function: (none) */
#define  MAIN_TEXTMSG_7                   68      /* control type: textMsg, callback function: (none) */
#define  MAIN_TEXTMSG_startNr             69      /* control type: textMsg, callback function: (none) */
#define  MAIN_TEXTMSG_SequenceState       70      /* control type: textMsg, callback function: (none) */
#define  MAIN_DECORATION_3                71      /* control type: deco, callback function: (none) */
#define  MAIN_TEXTMSG_8                   72      /* control type: textMsg, callback function: (none) */
#define  MAIN_DECORATION1                 73      /* control type: deco, callback function: (none) */

#define  MAIN2                            11      /* callback function: MAIN2_panelChanged */
#define  MAIN2_TAB0                       2       /* control type: tab, callback function: MAIN_tabClicked */

#define  MENUPANE10                       12

#define  MENUPANEL1                       13

#define  MENUPANEL2                       14

#define  MENUPANEL3                       15

#define  MENUPANEL4                       16

#define  MENUPANEL5                       17

#define  MENUPANEL6                       18

#define  MENUPANEL7                       19

#define  MENUPANEL8                       20

#define  MENUPANEL9                       21

#define  NEW_GPIB                         22
#define  NEW_GPIB_RING_commandType        2       /* control type: ring, callback function: (none) */
#define  NEW_GPIB_BTN_abort               3       /* control type: command, callback function: GPIB_newCommand_Abort */
#define  NEW_GPIB_BTN_done                4       /* control type: command, callback function: GPIB_newCommand_Done */

#define  PETER                            23
#define  PETER_TEXTBOX_paul               2       /* control type: textBox, callback function: TESTfunction */

#define  RENAME                           24
#define  RENAME_STRING                    2       /* control type: string, callback function: (none) */
#define  RENAME_BTN_abort                 3       /* control type: command, callback function: MAIN_RENAMESEQ_abort */
#define  RENAME_BTN_done                  4       /* control type: command, callback function: MAIN_RENAMESEQ_done */

#define  SEQLIST                          25
#define  SEQLIST_TREE                     2       /* control type: tree, callback function: (none) */

#define  SIGNAL2                          26
#define  SIGNAL2_STRING_stopSequence      2       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_20MHz             3       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_triggerSeq_3      4       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_triggerSeq_2      5       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_triggerSeq        6       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_counterDet1_2     7       /* control type: string, callback function: (none) */
#define  SIGNAL2_STRING_counterDet1       8       /* control type: string, callback function: (none) */
#define  SIGNAL2_TEXTBOX                  9       /* control type: textBox, callback function: (none) */
#define  SIGNAL2_TEXTBOX_2                10      /* control type: textBox, callback function: (none) */
#define  SIGNAL2_TEXTMSG                  11      /* control type: textMsg, callback function: (none) */

#define  SIMULATE                         27      /* callback function: SIMULATE_panelCallback */
#define  SIMULATE_RING_DAC                2       /* control type: ring, callback function: NUM_DAC_Changed */
#define  SIMULATE_TEXTMSG_ms              3       /* control type: textMsg, callback function: (none) */
#define  SIMULATE_TEXTMSG_ms_2            4       /* control type: textMsg, callback function: (none) */
#define  SIMULATE_NUMERIC_tFrom           5       /* control type: numeric, callback function: SIMULATE_axisChanged_CB */
#define  SIMULATE_NUMERIC_tTo             6       /* control type: numeric, callback function: SIMULATE_axisChanged_CB */
#define  SIMULATE_COMMANDBUTTON_showAll   7       /* control type: command, callback function: SIMULATE_showAll_CB */
#define  SIMULATE_NUMERIC_repetition      8       /* control type: numeric, callback function: SIMUALTE_changeRepetition_CB */
#define  SIMULATE_COMMANDBUTTON_update    9       /* control type: command, callback function: SIMUALTE_update_CB */
#define  SIMULATE_GRAPH_DAC1              10      /* control type: graph, callback function: (none) */
#define  SIMULATE_GRAPH_DIO               11      /* control type: graph, callback function: (none) */
#define  SIMULATE_STRING_channelName      12      /* control type: string, callback function: (none) */
#define  SIMULATE_TEXTBOX_info            13      /* control type: textBox, callback function: (none) */
#define  SIMULATE_DECORATION              14      /* control type: deco, callback function: (none) */

#define  STARTFROM                        28      /* callback function: PANEL_STARTFROM_callback */
#define  STARTFROM_NUMERIC_from           2       /* control type: numeric, callback function: (none) */
#define  STARTFROM_NUMERIC_to             3       /* control type: numeric, callback function: (none) */
#define  STARTFROM_BTN_startSequence      4       /* control type: command, callback function: STARTFROM_BTN_StartSequence */

#define  SWEEPS                           29
#define  SWEEPS_TABLE                     2       /* control type: table, callback function: (none) */
#define  SWEEPS_COMMANDBUTTON_close       3       /* control type: command, callback function: MAIN_SWEEP_close_CB */

#define  TABLECONF                        30
#define  TABLECONF_RADIOBUTTON_DIO        2       /* control type: radioButton, callback function: TABLECONFIG_changed */
#define  TABLECONF_LISTBOX                3       /* control type: listBox, callback function: (none) */
#define  TABLECONF_COMMANDBUTTON_done     4       /* control type: command, callback function: TABLECONFIG_done */
#define  TABLECONF_COMMANDBUTTON_3        5       /* control type: command, callback function: (none) */
#define  TABLECONF_COMMANDBUTTON_2        6       /* control type: command, callback function: (none) */
#define  TABLECONF_COMMANDBUTTON          7       /* control type: command, callback function: (none) */
#define  TABLECONF_STRING_DIO             8       /* control type: string, callback function: (none) */
#define  TABLECONF_TEXTMSG_dig            9       /* control type: textMsg, callback function: (none) */

#define  TITEL                            31
#define  TITEL_TEXTMSG_release            2       /* control type: textMsg, callback function: (none) */
#define  TITEL_TEXTMSG_date               3       /* control type: textMsg, callback function: (none) */
#define  TITEL_TEXTMSG_Kuhr               4       /* control type: textMsg, callback function: (none) */
#define  TITEL_STRING_status              5       /* control type: string, callback function: (none) */
#define  TITEL_TEXTMSG_Expc               6       /* control type: textMsg, callback function: (none) */
#define  TITEL_PICTURE                    7       /* control type: picture, callback function: (none) */
#define  TITEL_COMMANDBUTTON_Close        8       /* control type: command, callback function: TITLE_close_CB */

#define  VAR_PULSE                        32
#define  VAR_PULSE_TEXTMSG                2       /* control type: textMsg, callback function: (none) */
#define  VAR_PULSE_TEXTMSG_3              3       /* control type: textMsg, callback function: (none) */
#define  VAR_PULSE_NUMERIC_stepRep        4       /* control type: numeric, callback function: (none) */
#define  VAR_PULSE_COMMANDBUTTON_done     5       /* control type: command, callback function: BLOCK_variablePulseDone */
#define  VAR_PULSE_COMMANDBUTTON_abort    6       /* control type: command, callback function: BLOCK_variablePulseAbort */
#define  VAR_PULSE_TEXTMSG_2              7       /* control type: textMsg, callback function: (none) */
#define  VAR_PULSE_TEXTMSG_4              8       /* control type: textMsg, callback function: (none) */
#define  VAR_PULSE_TEXTMSG_5              9       /* control type: textMsg, callback function: (none) */
#define  VAR_PULSE_NUMERIC_changeStart    10      /* control type: numeric, callback function: (none) */
#define  VAR_PULSE_NUMERIC_changeStop     11      /* control type: numeric, callback function: (none) */

#define  VARTIME                          33      /* callback function: VARTIME_panelCallback */
#define  VARTIME_TEXTMSG_ms               2       /* control type: textMsg, callback function: (none) */
#define  VARTIME_TEXTMSG_ms_2             3       /* control type: textMsg, callback function: (none) */
#define  VARTIME_NUMERIC_varTimeStep      4       /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_TEXTMSG_rep3             5       /* control type: textMsg, callback function: (none) */
#define  VARTIME_TEXTMSG_rep              6       /* control type: textMsg, callback function: (none) */
#define  VARTIME_BTN_DONE                 7       /* control type: command, callback function: VARTIME_BTNDone */
#define  VARTIME_BTN_ABORT                8       /* control type: command, callback function: VARTIME_BTNAbort */
#define  VARTIME_NUMERIC_StartTime        9       /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_NUMERIC_IncrementTime    10      /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_NUMERIC_varTimeModulo    11      /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_TEXTMSG_rep2             12      /* control type: textMsg, callback function: (none) */
#define  VARTIME_NUMERIC_varTimeExecOf    13      /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_NUMERIC_varTimeExec      14      /* control type: numeric, callback function: VARTIME_edited */
#define  VARTIME_CHECKBOX_varDuration     15      /* control type: radioButton, callback function: (none) */

#define  WFMSMALL                         34      /* callback function: DIGITALBLOCKS_smallWfmPanelClicked */
#define  WFMSMALL_NUMERIC_stepRep         2       /* control type: numeric, callback function: DIGITALBLOCKS_smallWfmPanelStepCtrlClicked */
#define  WFMSMALL_NUMERIC_stepFrom        3       /* control type: numeric, callback function: DIGITALBLOCKS_smallWfmPanelStepCtrlClicked */
#define  WFMSMALL_NUMERIC_stepTo          4       /* control type: numeric, callback function: DIGITALBLOCKS_smallWfmPanelStepCtrlClicked */
#define  WFMSMALL_COMMANDBUTTON_done      5       /* control type: command, callback function: DIGITALBLOCKS_smallWfmPanelDoneBtnClicked */
#define  WFMSMALL_GRAPH_WFM               6       /* control type: graph, callback function: DIGITALBLOCKS_smallWfmPanelCtrlClicked */
#define  WFMSMALL_TEXTMSG_arrow           7       /* control type: textMsg, callback function: DIGITALBLOCKS_smallWfmPanelCtrlClicked */
#define  WFMSMALL_TEXTMSG_repetitions     8       /* control type: textMsg, callback function: DIGITALBLOCKS_smallWfmPanelCtrlClicked */
#define  WFMSMALL_TEXTMSG_stepTitle       9       /* control type: textMsg, callback function: DIGITALBLOCKS_smallWfmPanelCtrlClicked */
#define  WFMSMALL_STRING_name             10      /* control type: string, callback function: (none) */

#define  WFMTYPE                          35
#define  WFMTYPE_RINGSLIDE_wfmtype        2       /* control type: slide, callback function: (none) */
#define  WFMTYPE_COMMANDBUTTON_done       3       /* control type: command, callback function: WFMTYPE_done_CB */
#define  WFMTYPE_COMMANDBUTTON_abort      4       /* control type: command, callback function: WFMTYPE_abort_CB */

     /* tab page panel controls */
#define  ADDR_DMD_GRAPH_waveform          2       /* control type: graph, callback function: (none) */
#define  ADDR_DMD_RING_shapes             3       /* control type: ring, callback function: WFM_DMD_attributeChanged_CB */
#define  ADDR_DMD_NUMERIC_repetition      4       /* control type: numeric, callback function: WFM_ADDR_DMD_changeRepetitionFunc */
#define  ADDR_DMD_TABLE_parameters        5       /* control type: table, callback function: WFM_ADDR_DMD_TABLE_parameters_CB */
#define  ADDR_DMD_TEXTBOX_description     6       /* control type: textBox, callback function: (none) */
#define  ADDR_DMD_BUTTON_transmitseries   7       /* control type: command, callback function: WFM_DMD_transmitSeriesToDMD_CB */
#define  ADDR_DMD_BUTTON_transmit         8       /* control type: command, callback function: WFM_DMD_transmitToDMD_CB */
#define  ADDR_DMD_CHECKBOX_enableDMD      9       /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  ADDR_DMD_CHECKBOX_DMDsmoothing   10      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  ADDR_DMD_CHECKBOX_DMDerrdiff     11      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  ADDR_DMD_CHECKBOX_DMDfeedback    12      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */

     /* tab page panel controls */
#define  ADDR_SS_TABLE_SINGLESITE         2       /* control type: table, callback function: WFM_ADDR_TABLE_clicked_CB */
#define  ADDR_SS_NUMERIC_picsPerRep       3       /* control type: numeric, callback function: WFM_ADDR_SS_getParameters_CB */
#define  ADDR_SS_NUMERIC_duration         4       /* control type: numeric, callback function: WFM_ADDR_SS_getParameters_CB */
#define  ADDR_SS_TEXTMSG_ms               5       /* control type: textMsg, callback function: (none) */
#define  ADDR_SS_COMMANDBUTTON_up         6       /* control type: pictButton, callback function: WFM_ADDR_shift */
#define  ADDR_SS_COMMANDBUTTON_left       7       /* control type: pictButton, callback function: WFM_ADDR_shift */
#define  ADDR_SS_COMMANDBUTTON_right      8       /* control type: pictButton, callback function: WFM_ADDR_shift */
#define  ADDR_SS_COMMANDBUTTON_down       9       /* control type: pictButton, callback function: WFM_ADDR_shift */
#define  ADDR_SS_CHECKBOX_stepMatrix      10      /* control type: radioButton, callback function: WFM_ADDR_SS_getParameters_CB */
#define  ADDR_SS_NUMERIC_matrixNum        11      /* control type: numeric, callback function: WFM_ADDR_changeMatrix */
#define  ADDR_SS_NUMERIC_disorder         12      /* control type: numeric, callback function: WFM_ADDR_changeDisorder */
#define  ADDR_SS_CHECKBOX_disorder        13      /* control type: radioButton, callback function: WFM_ADDR_changeDisorder */
#define  ADDR_SS_RING_disorder            14      /* control type: ring, callback function: WFM_ADDR_changeDisorder */

     /* tab page panel controls */
#define  ADDR_TRAJ_TABLE_parameters       2       /* control type: table, callback function: WFM_ADDR_getParameters_CB */
#define  ADDR_TRAJ_GRAPH_2                3       /* control type: graph, callback function: (none) */

     /* tab page panel controls */
#define  BOARDPAR_STRING_AOdeviceName     2       /* control type: string, callback function: (none) */
#define  BOARDPAR_NUMERIC_AOdeviceNo2     3       /* control type: numeric, callback function: (none) */
#define  BOARDPAR_NUMERIC_AOdeviceNo1     4       /* control type: numeric, callback function: (none) */
#define  BOARDPAR_TEXTMSG_Channel1_2      5       /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_NUMERIC_DIOdeviceNo2    6       /* control type: numeric, callback function: (none) */
#define  BOARDPAR_NUMERIC_DIOdeviceNo1    7       /* control type: numeric, callback function: (none) */
#define  BOARDPAR_TEXTMSG_Channel1        8       /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_STRING_DIOdeviceName    9       /* control type: string, callback function: (none) */
#define  BOARDPAR_RADIOBUTTON_invert      10      /* control type: radioButton, callback function: (none) */
#define  BOARDPAR_NUMERIC_AOtimebase      11      /* control type: numeric, callback function: (none) */
#define  BOARDPAR_NUMERIC_DIOtimebase     12      /* control type: numeric, callback function: (none) */
#define  BOARDPAR_TEXTMSG_2               13      /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_TEXTMSG_3               14      /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_TEXTMSG_4               15      /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_TEXTMSG                 16      /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_TEXTMSG_6               17      /* control type: textMsg, callback function: (none) */
#define  BOARDPAR_DECORATION_2            18      /* control type: deco, callback function: (none) */
#define  BOARDPAR_DECORATION              19      /* control type: deco, callback function: (none) */
#define  BOARDPAR_TEXTBOX_adWinOutput     20      /* control type: textBox, callback function: (none) */
#define  BOARDPAR_COMMANDBUTTON_getInfo   21      /* control type: command, callback function: CONFIG_BTN_adwinGetSystemInfo */
#define  BOARDPAR_COMMANDBUTTON_boot      22      /* control type: command, callback function: CONFIG_BTN_adwinBoot */
#define  BOARDPAR_NUMERIC_DIGOUT1         23      /* control type: numeric, callback function: (none) */
#define  BOARDPAR_TEXTMSG_5               24      /* control type: textMsg, callback function: (none) */

     /* tab page panel controls */
#define  CCDTAB_COMMANDBUTTON_add         2       /* control type: command, callback function: CCD_addCamera_CB */

     /* tab page panel controls */
#define  CONFIGdev_COMMANDBUTTON_new      2       /* control type: command, callback function: EXTDEV_newExtDev_CALLBACK */
#define  CONFIGdev_COMMANDBUTTON_delete   3       /* control type: command, callback function: EXTDEV_deleteExtdev_CALLBACK */
#define  CONFIGdev_TABLE_extdev           4       /* control type: table, callback function: EXTDEV_TABLE_callback */

     /* tab page panel controls */
#define  CONFIGgen_NUMERIC_NAO_Channels   2       /* control type: numeric, callback function: (none) */
#define  CONFIGgen_NUMERIC_autoFocusBloc  3       /* control type: numeric, callback function: (none) */
#define  CONFIGgen_NUMERIC_PIFOC_Channel  4       /* control type: numeric, callback function: (none) */
#define  CONFIGgen_NUMERIC_NDIO_Channels  5       /* control type: numeric, callback function: (none) */
#define  CONFIGgen_BTN_watchdogActivate   6       /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_BUTTON_updateFocus     7       /* control type: command, callback function: BTN_updateAutofocusVals */
#define  CONFIGgen_BTN_autoRepeat         8       /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_BTN_enableTCP          9       /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_BTN_enableGPIB         10      /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_STRING_analogBoardID   11      /* control type: string, callback function: (none) */
#define  CONFIGgen_STRING_digitalBoardID  12      /* control type: string, callback function: (none) */
#define  CONFIGgen_TEXTMSG_7              13      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_11             14      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_NUMERIC_autoFocusStep  15      /* control type: numeric, callback function: CONFIG_autoFocusSettingsChanged */
#define  CONFIGgen_NUMERIC_autoFocusCent  16      /* control type: numeric, callback function: CONFIG_autoFocusSettingsChanged */
#define  CONFIGgen_TEXTMSG_14             17      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_13             18      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_8              19      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_3              20      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_NUMERIC_lastPifocVolt  21      /* control type: numeric, callback function: (none) */
#define  CONFIGgen_TEXTMSG_5              22      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_NUMERIC_numFocusImage  23      /* control type: numeric, callback function: CONFIG_autoFocusSettingsChanged */
#define  CONFIGgen_NUMERIC_btnWidth       24      /* control type: numeric, callback function: (none) */
#define  CONFIGgen_NUMERIC_btnHeight      25      /* control type: numeric, callback function: (none) */
#define  CONFIGgen_PICTURE                26      /* control type: picture, callback function: (none) */
#define  CONFIGgen_DECORATION             27      /* control type: deco, callback function: (none) */
#define  CONFIGgen_TEXTMSG_autoFocusVals  28      /* control type: textBox, callback function: (none) */
#define  CONFIGgen_TEXTBOX_watchdogStat   29      /* control type: textBox, callback function: (none) */
#define  CONFIGgen_BTN_launchDataSocket   30      /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_DECORATION_5           31      /* control type: deco, callback function: (none) */
#define  CONFIGgen_DECORATION_2           32      /* control type: deco, callback function: (none) */
#define  CONFIGgen_BUTTON_createFilename  33      /* control type: radioButton, callback function: CONFIG_BUTTON_clicked */
#define  CONFIGgen_BUTTON_autosaveSeq     34      /* control type: radioButton, callback function: CONFIG_BUTTON_clicked */
#define  CONFIGgen_STRING_pifocLogPath    35      /* control type: string, callback function: (none) */
#define  CONFIGgen_STRING_idleSeqDatPath  36      /* control type: string, callback function: (none) */
#define  CONFIGgen_STRING_pathMagick      37      /* control type: string, callback function: (none) */
#define  CONFIGgen_STRING_pathImageFile   38      /* control type: string, callback function: (none) */
#define  CONFIGgen_BUTTON_browsePifocPat  39      /* control type: command, callback function: CONFIG_pifocFocusVoltageLogFilePath_callback */
#define  CONFIGgen_BUTTON_browseIdlePath  40      /* control type: command, callback function: CONFIG_idleSequenceDataPath_callback */
#define  CONFIGgen_STRING_autosavePath    41      /* control type: string, callback function: (none) */
#define  CONFIGgen_BUTTON_browseConvPath  42      /* control type: command, callback function: CONFIG_convertPathBrowse_callback */
#define  CONFIGgen_BUTTON_browseImagPath  43      /* control type: command, callback function: CONFIG_imageFilePathBrowse_callback */
#define  CONFIGgen_BUTTON_browseSeqPath   44      /* control type: command, callback function: CONFIG_savePathBrowse_callback */
#define  CONFIGgen_DECORATION_4           45      /* control type: deco, callback function: (none) */
#define  CONFIGgen_RINGSLIDE_hardware     46      /* control type: slide, callback function: (none) */
#define  CONFIGgen_CHECKBOX_isIdleSeq     47      /* control type: radioButton, callback function: CONFIG_isOfTypeIdleChanged */
#define  CONFIGgen_CHECKBOX_autofocus     48      /* control type: radioButton, callback function: (none) */
#define  CONFIGgen_NUMERIC_manualPifocV   49      /* control type: numeric, callback function: (none) */
#define  CONFIGgen_DECORATION_6           50      /* control type: deco, callback function: (none) */
#define  CONFIGgen_TEXTMSG_2              51      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_12             52      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG                53      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_TEXTMSG_9              54      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_DECORATION_7           55      /* control type: deco, callback function: (none) */
#define  CONFIGgen_TEXTMSG_10             56      /* control type: textMsg, callback function: (none) */
#define  CONFIGgen_DECORATION_8           57      /* control type: deco, callback function: (none) */

     /* tab page panel controls */
#define  CONFIGgpib_COMMANDBUTTON         2       /* control type: command, callback function: CONFIG_GPIB_autoDetectDevices */
#define  CONFIGgpib_TEXTMSG_3             3       /* control type: textMsg, callback function: (none) */
#define  CONFIGgpib_LISTBOX_gpibDevices   4       /* control type: listBox, callback function: CONFIG_GPIB_deviceSelected */
#define  CONFIGgpib_COMMANDBUTTON_Reset   5       /* control type: command, callback function: GPIB_BTN_Reset */
#define  CONFIGgpib_BUTTON_Settings       6       /* control type: command, callback function: CONFIG_GPIB_EditDeviceSettings */
#define  CONFIGgpib_TEXTBOX_supported     7       /* control type: textBox, callback function: (none) */

     /* tab page panel controls */
#define  GPIB_LISTBOX_GpibCommands        2       /* control type: listBox, callback function: GPIB_listboxClicked */
#define  GPIB_STRING_Name                 3       /* control type: string, callback function: GPIB_NameChanged */
#define  GPIB_RING_ExtDevice              4       /* control type: ring, callback function: GPIB_AttributeChanged */
#define  GPIB_RING_device                 5       /* control type: ring, callback function: GPIB_AttributeChanged */
#define  GPIB_BUTTON_autodetect           6       /* control type: command, callback function: GPIB_autodetect_CB */
#define  GPIB_BUTTON_DeviceSettings       7       /* control type: command, callback function: CONFIG_GPIB_EditDeviceSettings */
#define  GPIB_TEXTMSG_transmit2           8       /* control type: textMsg, callback function: (none) */
#define  GPIB_BTN_transBeforeFirst        9       /* control type: radioButton, callback function: GPIB_AttributeChanged */
#define  GPIB_BTN_transBeforeEvery        10      /* control type: radioButton, callback function: GPIB_AttributeChanged */
#define  GPIB_BTN_transAfterEvery         11      /* control type: radioButton, callback function: GPIB_AttributeChanged */
#define  GPIB_BTN_transAfterStart         12      /* control type: radioButton, callback function: GPIB_AttributeChanged */
#define  GPIB_BTN_transAfterLast          13      /* control type: radioButton, callback function: GPIB_AttributeChanged */
#define  GPIB_COMMANDBUTTON               14      /* control type: command, callback function: GPIB_TEST */
#define  GPIB_BUTTON_transmitWFM          15      /* control type: command, callback function: GPIB_transmitCommand_CB */
#define  GPIB_TAB                         16      /* control type: tab, callback function: GPIB_TabChanged */
#define  GPIB_BTN_DeleteGPIBCommand       17      /* control type: command, callback function: GPIB_deleteCommand_CB */
#define  GPIB_BTN_NewGpibCommand          18      /* control type: command, callback function: GPIB_newCommand */
#define  GPIB_NUMERIC_maxDuration         19      /* control type: numeric, callback function: GPIB_AttributeChanged */
#define  GPIB_TEXTMSG                     20      /* control type: textMsg, callback function: (none) */
#define  GPIB_TEXTMSG_2                   21      /* control type: textMsg, callback function: (none) */
#define  GPIB_CHECKBOX_GPIB               22      /* control type: radioButton, callback function: GPIB_AttributeChanged */

     /* tab page panel controls */
#define  GPIBcom_CHECKBOX_sendCommand1    2       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_reply1       3       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_askError1    4       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_DECORATION               5       /* control type: deco, callback function: (none) */
#define  GPIBcom_CHECKBOX_sendCommand2    6       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_reply2       7       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_DECORATION_2             8       /* control type: deco, callback function: (none) */
#define  GPIBcom_RADIOBUTTON_askError2    9       /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_CHECKBOX_sendCommand3    10      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_reply3       11      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_askError3    12      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_CHECKBOX_sendCommand4    13      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_DECORATION_3             14      /* control type: deco, callback function: (none) */
#define  GPIBcom_RADIOBUTTON_reply4       15      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_RADIOBUTTON_askError4    16      /* control type: radioButton, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_DECORATION_4             17      /* control type: deco, callback function: (none) */
#define  GPIBcom_TEXTBOX4                 18      /* control type: textBox, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_TEXTBOX3                 19      /* control type: textBox, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_TEXTBOX2                 20      /* control type: textBox, callback function: GPIBcomand_attributeChanged */
#define  GPIBcom_TEXTBOX1                 21      /* control type: textBox, callback function: GPIBcomand_attributeChanged */

     /* tab page panel controls */
#define  GPIBfreq_NUMERIC_startFreq       2       /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RING_startMultiply      3       /* control type: ring, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_stopFreq        4       /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RING_stopMultiply       5       /* control type: ring, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_offset          6       /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_centerFreq      7       /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RING_centerMultiply     8       /* control type: ring, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_span            9       /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RING_modDevMultiply     10      /* control type: ring, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_CHECKBOX_askForErrors   11      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RING_spanMultiply       12      /* control type: ring, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_stepNRep        13      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_repetitions     14      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_RADIOBTN_enableStep     15      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_amDepth         16      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_feedbackU_2     17      /* control type: numeric, callback function: (none) */
#define  GPIBfreq_NUMERIC_feedbackOffs2   18      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_feedbackDev_2   19      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_feedbackU       20      /* control type: numeric, callback function: (none) */
#define  GPIBfreq_NUMERIC_feedbackOffs    21      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_feedbackDev     22      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_CHECKBOX_fetchFreq      23      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_freqDev         24      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_outputPower     25      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RINGSLIDE_changeFreq    26      /* control type: slide, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_NUMERIC_divideFreq      27      /* control type: numeric, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RADIOBUTTON_RFon        28      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_dBm             29      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_DECORATION_8            30      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION              31      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_2            32      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_9            33      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_7            34      /* control type: deco, callback function: (none) */
#define  GPIBfreq_RADIOBUTTON_feedback2   35      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_DECORATION_6            36      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_5            37      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_4            38      /* control type: deco, callback function: (none) */
#define  GPIBfreq_DECORATION_3            39      /* control type: deco, callback function: (none) */
#define  GPIBfreq_RADIOBUTTON_feedback    40      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RADIOBUTTON_AM          41      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RADIOBUTTON_freqMod     42      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_RADIOBUTTON_pulseMode   43      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_feedback1_2     44      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback2_2     45      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback3_2     46      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback4_2     47      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_COMMANDBUTTON_test2     48      /* control type: command, callback function: GPIB_freq_TestDataAcquisition_CB */
#define  GPIBfreq_STRING_feedbackInput2   49      /* control type: string, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_feedback5_2     50      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback6_2     51      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_CHECKBOX_addFreqOffs    52      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_CHECKBOX_centerSpan     53      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_CHECKBOX_startStop      54      /* control type: radioButton, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_pct             55      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_MHz             56      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback1       57      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback2       58      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback3       59      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback4       60      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_COMMANDBUTTON_test1     61      /* control type: command, callback function: GPIB_freq_TestDataAcquisition_CB */
#define  GPIBfreq_STRING_feedbackInput    62      /* control type: string, callback function: GPIBfreq_attributeChanged */
#define  GPIBfreq_TEXTMSG_feedback5       63      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_TEXTMSG_feedback6       64      /* control type: textMsg, callback function: (none) */
#define  GPIBfreq_NUMERIC_EXT_channel     65      /* control type: numeric, callback function: GPIBfreq_attributeChanged */

     /* tab page panel controls */
#define  GPIBpow_NUMERIC_channel          2       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_RADIOBTN_enableStepU     3       /* control type: radioButton, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_uStart           4       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_uStop            5       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_RADIOBTN_enableStepI     6       /* control type: radioButton, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_iStart           7       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_iStop            8       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_stepNRep         9       /* control type: numeric, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_CHECKBOX_askForErrors    10      /* control type: radioButton, callback function: GPIBpower_attributeChanged */
#define  GPIBpow_NUMERIC_uMin             11      /* control type: numeric, callback function: (none) */
#define  GPIBpow_NUMERIC_uMax             12      /* control type: numeric, callback function: (none) */
#define  GPIBpow_NUMERIC_iMax             13      /* control type: numeric, callback function: (none) */
#define  GPIBpow_TEXTMSG_V1               14      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_mA1              15      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_repetitions      16      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_V2_4             17      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_mA3              18      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_V2_2             19      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_V2               20      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_TEXTMSG_mA2              21      /* control type: textMsg, callback function: (none) */
#define  GPIBpow_CHECKBOX_waitStabiliz    22      /* control type: radioButton, callback function: GPIBpower_attributeChanged */

     /* tab page panel controls */
#define  GPIBpulses_RING_PULSES           2       /* control type: ring, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_NUMERIC_DELAY_START   3       /* control type: numeric, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_NUMERIC_DELAY_STOP    4       /* control type: numeric, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_NUMERIC_LENGTH_START  5       /* control type: numeric, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_NUMERIC_LENGTH_STOP   6       /* control type: numeric, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_CHECKBOX_LENGT_ENABLE 7       /* control type: radioButton, callback function: GPIBPulse_attributeChanged */
#define  GPIBpulses_CHECKBOX_DELAY_ENABLE 8       /* control type: radioButton, callback function: GPIBPulse_attributeChanged */

     /* tab page panel controls */
#define  GPIBwfm_GRAPH_WFM                2       /* control type: graph, callback function: (none) */
#define  GPIBwfm_NUMERIC_nPoints          3       /* control type: numeric, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_RING_WFM                 4       /* control type: ring, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_NUMERIC_resolution       5       /* control type: numeric, callback function: (none) */
#define  GPIBwfm_TEXTMSG_ms2              6       /* control type: textMsg, callback function: (none) */
#define  GPIBwfm_NUMERIC_duration         7       /* control type: numeric, callback function: (none) */
#define  GPIBwfm_NUMERIC_channel          8       /* control type: numeric, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_TEXTMSG_2                9       /* control type: textMsg, callback function: (none) */
#define  GPIBwfm_CHECKBOX_askForErrors    10      /* control type: radioButton, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_CHECKBOX_alwaysMaxPts    11      /* control type: radioButton, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_RING_transferMode        12      /* control type: slide, callback function: GPIBWfm_attributeChanged */
#define  GPIBwfm_DECORATION               13      /* control type: deco, callback function: (none) */

     /* tab page panel controls */
#define  LCONFTAB_CONFTEXT                2       /* control type: textBox, callback function: (none) */
#define  LCONFTAB_SENDCONFBUTTON          3       /* control type: command, callback function: onSendConfButton */
#define  LCONFTAB_CHECKBOX_FIRST          4       /* control type: radioButton, callback function: (none) */
#define  LCONFTAB_CHECKBOX_EVERY          5       /* control type: radioButton, callback function: (none) */

     /* tab page panel controls */
#define  SEQUENCE_TABLE_dac2              2       /* control type: table, callback function: TABLE_dac2_CB */
#define  SEQUENCE_BTN_Rescue              3       /* control type: command, callback function: WFM_Rescue_CALLBACK */
#define  SEQUENCE_NUMERIC_repOf           4       /* control type: numeric, callback function: SEQUENCE_parametersChanged */
#define  SEQUENCE_NUMERIC_rep             5       /* control type: numeric, callback function: (none) */
#define  SEQUENCE_SLIDE_progress2         6       /* control type: scale, callback function: (none) */
#define  SEQUENCE_TABLE_header2           7       /* control type: table, callback function: TABLE_header2_CB */
#define  SEQUENCE_TABLE_header1           8       /* control type: table, callback function: TABLE_header1_CB */
#define  SEQUENCE_TABLE_dac1              9       /* control type: table, callback function: TABLE_dac1_CB */
#define  SEQUENCE_SPLITTER                10      /* control type: splitter, callback function: DIGITALBLOCKS_splitterMoved */
#define  SEQUENCE_TABLE_digital2          11      /* control type: table, callback function: TABLE_digital2_CB */
#define  SEQUENCE_TABLE_digital1          12      /* control type: table, callback function: TABLE_digital1_CB */
#define  SEQUENCE_VSPLITTER               13      /* control type: splitter, callback function: DIGITALBLOCKS_VsplitterMoved */
#define  SEQUENCE_DECORATION1             14      /* control type: deco, callback function: (none) */
#define  SEQUENCE_BTN_startSequence       15      /* control type: command, callback function: BTN_StartSequence */
#define  SEQUENCE_BTN_stopSequence        16      /* control type: command, callback function: BTN_stopSequence */
#define  SEQUENCE_NUMERICSLIDE_progress   17      /* control type: scale, callback function: (none) */

     /* tab page panel controls */
#define  TELNET_STRING_PORT               2       /* control type: string, callback function: (none) */
#define  TELNET_STRING_IP                 3       /* control type: string, callback function: (none) */

     /* tab page panel controls */
#define  WFM_STRING_WFM_Name              2       /* control type: string, callback function: WFM_MAIN_WFMNameChanged */
#define  WFM_BTN_DeleteUnused             3       /* control type: command, callback function: WFM_deleteUnusedWaveforms_CALLBACK */
#define  WFM_BTN_DeleteWaveform           4       /* control type: command, callback function: WFM_deleteWaveform_CALLBACK */
#define  WFM_BTN_NewWFM                   5       /* control type: command, callback function: WFM_newWaveform_CALLBACK */
#define  WFM_RING_add                     6       /* control type: ring, callback function: WFM_addChanged */
#define  WFM_TAB0                         7       /* control type: tab, callback function: WFM_tabChanged_CALLBACK */
#define  WFM_TABLE_wfm                    8       /* control type: table, callback function: WFM_TABLE_callback */

     /* tab page panel controls */
#define  WFM_ADDR_TABLE_calibration       2       /* control type: table, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_NUMERIC_centerX         3       /* control type: numeric, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_NUMERIC_centerY         4       /* control type: numeric, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_STRING_testFile2        5       /* control type: string, callback function: (none) */
#define  WFM_ADDR_COMMANDBUTTON_browse2   6       /* control type: command, callback function: WFM_ADDR_selectTestFile2_CB */
#define  WFM_ADDR_STRING_testFile         7       /* control type: string, callback function: (none) */
#define  WFM_ADDR_COMMANDBUTTON_browse    8       /* control type: command, callback function: WFM_ADDR_selectTestFile_CB */
#define  WFM_ADDR_COMMANDBUTTON_testPos   9       /* control type: command, callback function: WFM_ADDR_testPositionFeedback_CB */
#define  WFM_ADDR_COMMANDBUTTON_show      10      /* control type: command, callback function: WFM_ADDR_showImageFile_CB */
#define  WFM_ADDR_NUMERIC_imageSize       11      /* control type: numeric, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_RING_link               12      /* control type: ring, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_CHECKBOX_addOffsetNo1   13      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_CHECKBOX_addOffset      14      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_CHECKBOX_enableFeedb2   15      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_CHECKBOX_enableFeedbk   16      /* control type: radioButton, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_NUMERIC_camera          17      /* control type: numeric, callback function: WFM_ADDR_getParameters_CB */
#define  WFM_ADDR_TEXTMSG_center          18      /* control type: textMsg, callback function: (none) */
#define  WFM_ADDR_TAB                     19      /* control type: tab, callback function: WFM_ADDR_getParameters_CB */

     /* tab page panel controls */
#define  WFM_FUNC_GRAPH_Waveform          2       /* control type: graph, callback function: (none) */
#define  WFM_FUNC_RING_functions          3       /* control type: ring, callback function: WFM_FUNC_attributeChanged_CB */
#define  WFM_FUNC_CANVAS                  4       /* control type: canvas, callback function: (none) */
#define  WFM_FUNC_BTN_Digitize            5       /* control type: command, callback function: WFM_Digitize */
#define  WFM_FUNC_NUMERIC_repetition      6       /* control type: numeric, callback function: WFM_changeRepetitionFunc */
#define  WFM_FUNC_TABLE                   7       /* control type: table, callback function: TABLE_parameters_CB */
#define  WFM_FUNC_TEXTMSG_us              8       /* control type: textMsg, callback function: (none) */
#define  WFM_FUNC_TEXTMSG_durationTo      9       /* control type: textMsg, callback function: (none) */
#define  WFM_FUNC_RADIOBUTTON_durVary     10      /* control type: radioButton, callback function: WFM_FUNC_attributeChanged_CB */
#define  WFM_FUNC_NUMERIC_timebase        11      /* control type: numeric, callback function: (none) */
#define  WFM_FUNC_NUMERIC_timebaseStre    12      /* control type: numeric, callback function: WFM_FUNC_attributeChanged_CB */
#define  WFM_FUNC_NUMERIC_durationFrom    13      /* control type: numeric, callback function: WFM_FUNC_attributeChanged_CB */
#define  WFM_FUNC_TEXTMSG_duration        14      /* control type: textMsg, callback function: (none) */
#define  WFM_FUNC_NUMERIC_durationTo      15      /* control type: numeric, callback function: WFM_FUNC_attributeChanged_CB */
#define  WFM_FUNC_TEXTMSG_durationFrom    16      /* control type: textMsg, callback function: (none) */
#define  WFM_FUNC_DECORATION_3            17      /* control type: deco, callback function: (none) */
#define  WFM_FUNC_TEXTMSG_function        18      /* control type: textMsg, callback function: (none) */
#define  WFM_FUNC_RADIOBUTTON_reverseT    19      /* control type: radioButton, callback function: WFM_FUNC_attributeChanged_CB */

     /* tab page panel controls */
#define  WFM_wFILE_STRING_Filename        2       /* control type: string, callback function: (none) */
#define  WFM_wFILE_GRAPH_Waveform         3       /* control type: graph, callback function: (none) */
#define  WFM_wFILE_COMMANDBUTTON_Browse   4       /* control type: command, callback function: WFM_browse_CALLBACK */
#define  WFM_wFILE_NUMERIC_nPoints        5       /* control type: numeric, callback function: (none) */
#define  WFM_wFILE_BTN_Digitize           6       /* control type: command, callback function: WFM_Digitize */
#define  WFM_wFILE_NUMERIC_durationTo     7       /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_NUMERIC_durationFrom   8       /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_NUMERIC_offsetTo       9       /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_NUMERIC_offsetFrom     10      /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_NUMERIC_amplitudeTo    11      /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_RADIOBUTTON_durVary    12      /* control type: radioButton, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_TEXTMSG_duration       13      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_NUMERIC_amplitudeFrom  14      /* control type: numeric, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_RADIOBUTTON_offsVary   15      /* control type: radioButton, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_TEXTMSG_offset         16      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_RADIOBUTTON_amplVary   17      /* control type: radioButton, callback function: WFM_FILE_AttributeChanged */
#define  WFM_wFILE_TEXTMSG_durationFrom   18      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_amplitude      19      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_DECORATION             20      /* control type: deco, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_amplFrom       21      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_amplTo         22      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_offsetFrom     23      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_offsetTo       24      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_DECORATION_2           25      /* control type: deco, callback function: (none) */
#define  WFM_wFILE_TEXTMSG_durationTo     26      /* control type: textMsg, callback function: (none) */
#define  WFM_wFILE_DECORATION_3           27      /* control type: deco, callback function: (none) */

     /* tab page panel controls */
#define  WFM_wPTS_TABLE_points            2       /* control type: table, callback function: WAVEFORMS_EditTable_Points */
#define  WFM_wPTS_GRAPH_Waveform          3       /* control type: graph, callback function: (none) */
#define  WFM_wPTS_TEXTMSG_invalid         4       /* control type: textMsg, callback function: (none) */
#define  WFM_wPTS_BTN_Digitize            5       /* control type: command, callback function: WFM_Digitize */
#define  WFM_wPTS_NUMERIC_repetition      6       /* control type: numeric, callback function: WFM_changeRepetitionPoints */
#define  WFM_wPTS_STRING_transferFct      7       /* control type: string, callback function: (none) */
#define  WFM_wPTS_NUMERIC_timebaseStre    8       /* control type: numeric, callback function: WAVEFORMS_Points_attributeChanged_CB */
#define  WFM_wPTS_NUMERIC_timebase        9       /* control type: numeric, callback function: (none) */

     /* tab page panel controls */
#define  WFM_wSTEP_NUMERIC_stepRep        2       /* control type: numeric, callback function: WFM_STEP_AttributeChanged */
#define  WFM_wSTEP_NUMERIC_stepTo         3       /* control type: numeric, callback function: WFM_STEP_AttributeChanged */
#define  WFM_wSTEP_NUMERIC_stepFrom       4       /* control type: numeric, callback function: WFM_STEP_AttributeChanged */
#define  WFM_wSTEP_TEXTMSG_change1        5       /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_and            6       /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_to             7       /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_from           8       /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_between        9       /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_V1             10      /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_V2             11      /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_msg1           12      /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_msg2           13      /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_TEXTMSG_arrow          14      /* control type: textMsg, callback function: (none) */
#define  WFM_wSTEP_BTN_alternate          15      /* control type: radioButton, callback function: WFM_STEP_AttributeChanged */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENU1                            1
#define  MENU1_CHANNELS                   2
#define  MENU1_CHANNELS_ALWAYSON          3
#define  MENU1_CHANNELS_ALWAYSOFF         4
#define  MENU1_CHANNELS_STANDARD          5
#define  MENU1_CHANNELS_SEPARATOR         6
#define  MENU1_CHANNELS_LOCK              7
#define  MENU1_CHANNELS_SEPARATOR_3       8
#define  MENU1_CHANNELS_INVERT_LOGIC      9
#define  MENU1_CHANNELS_SEPARATOR_2       10
#define  MENU1_CHANNELS_FLIP_SELECTION    11
#define  MENU1_CHANNELS_SEPARATOR_4       12
#define  MENU1_CHANNELS_HIDE_CHANNELS     13
#define  MENU1_CHANNELS_HIDE_UNUSED_CHANNELS 14
#define  MENU1_CHANNELS_SHOW_CHANNELS     15
#define  MENU1_CHANNELS_SAVE_CONFIG       16
#define  MENU1_CHANNELS_SAVE_CONFIG_SUBMENU 17
#define  MENU1_CHANNELS_SAVE_CONFIG_CONF1 18
#define  MENU1_CHANNELS_SAVE_CONFIG_CONF2 19
#define  MENU1_CHANNELS_SAVE_CONFIG_CONF3 20
#define  MENU1_CHANNELS_GET_CONFIG        21
#define  MENU1_CHANNELS_GET_CONFIG_SUBMENU 22
#define  MENU1_CHANNELS_GET_CONFIG_CONF1  23
#define  MENU1_CHANNELS_GET_CONFIG_CONF2  24
#define  MENU1_CHANNELS_GET_CONFIG_CONF3  25
#define  MENU1_CHANNELS_SHOW_ALL          26

#define  MENU10                           2
#define  MENU10_SEQ_QUEUE                 2
#define  MENU10_SEQ_QUEUE_INSERT_IDLE_BEFORE 3
#define  MENU10_SEQ_QUEUE_INSERT_IDLE_BEHIND 4
#define  MENU10_SEQ_QUEUE_REMOVE          5
#define  MENU10_SEQ_QUEUE_EDIT_COPY       6
#define  MENU10_SEQ_QUEUE_SEPARATOR       7
#define  MENU10_SEQ_QUEUE_REMOVE_ALL      8

#define  MENU2                            3
#define  MENU2_PULSEPROP                  2
#define  MENU2_PULSEPROP_ALTERNATE        3
#define  MENU2_PULSEPROP_SEPARATOR        4
#define  MENU2_PULSEPROP_EDIT             5
#define  MENU2_PULSEPROP_VARPULSE         6
#define  MENU2_PULSEPROP_VARPULSE_SUBMENU 7
#define  MENU2_PULSEPROP_VARPULSE_ON      8
#define  MENU2_PULSEPROP_VARPULSE_OFF     9

#define  MENU3                            4
#define  MENU3_BLKEDIT                    2
#define  MENU3_BLKEDIT_NEW_BEFORE         3
#define  MENU3_BLKEDIT_NEW_BEHIND         4
#define  MENU3_BLKEDIT_SEPARATOR_2        5
#define  MENU3_BLKEDIT_DISABLE            6
#define  MENU3_BLKEDIT_ENABLE             7
#define  MENU3_BLKEDIT_SEPARATOR_8        8
#define  MENU3_BLKEDIT_DELETE             9
#define  MENU3_BLKEDIT_SEPARATOR_7        10
#define  MENU3_BLKEDIT_VARTIME            11
#define  MENU3_BLKEDIT_SEPARATOR          12
#define  MENU3_BLKEDIT_COPY               13
#define  MENU3_BLKEDIT_CUT                14
#define  MENU3_BLKEDIT_PASTE_BEFORE       15
#define  MENU3_BLKEDIT_PASTE_BEHIND       16
#define  MENU3_BLKEDIT_SEPARATOR_4        17
#define  MENU3_BLKEDIT_GROUP              18
#define  MENU3_BLKEDIT_UNGROUP            19
#define  MENU3_BLKEDIT_EXPAND             20
#define  MENU3_BLKEDIT_COMPRESS           21
#define  MENU3_BLKEDIT_SEPARATOR_6        22
#define  MENU3_BLKEDIT_SET_LOOP           23
#define  MENU3_BLKEDIT_SET_LOOPREPS       24
#define  MENU3_BLKEDIT_DELETE_LOOP        25
#define  MENU3_BLKEDIT_SEPARATOR_5        26
#define  MENU3_BLKEDIT_TRIGGER            27
#define  MENU3_BLKEDIT_SEPARATOR_9        28
#define  MENU3_BLKEDIT_GPIB               29
#define  MENU3_BLKEDIT_GPIB_SUBMENU       30
#define  MENU3_BLKEDIT_GPIB_ITEM1         31
#define  MENU3_BLKEDIT_GPIB_ITEM2         32
#define  MENU3_BLKEDIT_SEPARATOR_3        33
#define  MENU3_BLKEDIT_TIMEREF            34
#define  MENU3_BLKEDIT_SHOW_ABSOLUTE      35
#define  MENU3_BLKEDIT_SEPARATOR_10       36
#define  MENU3_BLKEDIT_LOOP_TRIG          37
#define  MENU3_BLKEDIT_RECV_LOOP_TRIG     38

#define  MENU4                            5
#define  MENU4_DAC                        2
#define  MENU4_DAC_EDIT                   3
#define  MENU4_DAC_WAVEFORM               4
#define  MENU4_DAC_WAVEFORM_SUBMENU       5
#define  MENU4_DAC_WAVEFORM_ITEM1         6
#define  MENU4_DAC_WAVEFORM_ITEM2         7
#define  MENU4_DAC_WAVEFORM_ITEM3         8
#define  MENU4_DAC_UNCHANGED              9
#define  MENU4_DAC_CONST                  10
#define  MENU4_DAC_HOLD                   11

#define  MENU5                            6
#define  MENU5_SEQLIST                    2
#define  MENU5_SEQLIST_CLOSE_SEQ          3
#define  MENU5_SEQLIST_CLOSE_ALL          4
#define  MENU5_SEQLIST_SAVE               5
#define  MENU5_SEQLIST_SAVEAS             6
#define  MENU5_SEQLIST_RENAME             7
#define  MENU5_SEQLIST_SHOW               8
#define  MENU5_SEQLIST_SEPARATOR_2        9
#define  MENU5_SEQLIST_IDLE               10
#define  MENU5_SEQLIST_SEPARATOR_4        11
#define  MENU5_SEQLIST_ADD_QUEUE          12
#define  MENU5_SEQLIST_ADD_QUEUE_MULTI    13
#define  MENU5_SEQLIST_SEPARATOR_3        14
#define  MENU5_SEQLIST_READONLY           15
#define  MENU5_SEQLIST_SEPARATOR          16
#define  MENU5_SEQLIST_SHORTCUT           17
#define  MENU5_SEQLIST_SHORTCUT_SUBMENU   18
#define  MENU5_SEQLIST_SHORTCUT_F5        19
#define  MENU5_SEQLIST_SHORTCUT_F6        20
#define  MENU5_SEQLIST_SHORTCUT_F7        21
#define  MENU5_SEQLIST_SHORTCUT_F8        22

#define  MENU6                            7
#define  MENU6_WFM                        2
#define  MENU6_WFM_SORT                   3
#define  MENU6_WFM_SORT_SUBMENU           4
#define  MENU6_WFM_SORT_ID                5
#define  MENU6_WFM_SORT_NAME              6
#define  MENU6_WFM_SORT_CHANNEL           7
#define  MENU6_WFM_DELETE                 8

#define  MENU7                            8
#define  MENU7_CHANNELS                   2
#define  MENU7_CHANNELS_HIDE_CHANNELS     3
#define  MENU7_CHANNELS_SHOW_CHANNELS     4
#define  MENU7_CHANNELS_SAVE_CONFIG       5
#define  MENU7_CHANNELS_SAVE_CONFIG_SUBMENU 6
#define  MENU7_CHANNELS_SAVE_CONFIG_CONF1 7
#define  MENU7_CHANNELS_SAVE_CONFIG_CONF2 8
#define  MENU7_CHANNELS_SAVE_CONFIG_CONF3 9
#define  MENU7_CHANNELS_GET_CONFIG        10
#define  MENU7_CHANNELS_GET_CONFIG_SUBMENU 11
#define  MENU7_CHANNELS_GET_CONFIG_CONF1  12
#define  MENU7_CHANNELS_GET_CONFIG_CONF2  13
#define  MENU7_CHANNELS_GET_CONFIG_CONF3  14
#define  MENU7_CHANNELS_HIDE_UNUSED_CHANNELS 15
#define  MENU7_CHANNELS_SHOW_ALL          16
#define  MENU7_CHANNELS_SET_ALL_ZERO      17

#define  MENU8                            9
#define  MENU8_DISPLAY                    2
#define  MENU8_DISPLAY_SHOW_ABSTIME       3
#define  MENU8_DISPLAY_SHOW_GPIB          4

#define  MENU9                            10
#define  MENU9_ADDR                       2
#define  MENU9_ADDR_CLEAR                 3
#define  MENU9_ADDR_SELECT                4

#define  MENUBAR                          11
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_NEW                 3       /* callback function: MENU_New */
#define  MENUBAR_FILE_Open                4       /* callback function: MENU_open */
#define  MENUBAR_FILE_SEPARATOR_3         5
#define  MENUBAR_FILE_SAVE                6       /* callback function: MENU_Save */
#define  MENUBAR_FILE_SAVE_AS             7       /* callback function: MENU_SaveAs */
#define  MENUBAR_FILE_SAVE_COPY_AS        8       /* callback function: MENU_SaveCopyAs */
#define  MENUBAR_FILE_CLOSE               9       /* callback function: MENU_Close */
#define  MENUBAR_FILE_SEPARATOR_2         10
#define  MENUBAR_FILE_IMPORT_NAMES        11      /* callback function: MENU_ImportNames */
#define  MENUBAR_FILE_SEPARATOR_10        12
#define  MENUBAR_FILE_QUIT                13      /* callback function: MAIN_MENU_QuitProgram */
#define  MENUBAR_SEQUENCDE                14
#define  MENUBAR_SEQUENCDE_START          15      /* callback function: MENU_SequenceStart */
#define  MENUBAR_SEQUENCDE_STARTFROM      16      /* callback function: MENU_SequenceStartFrom */
#define  MENUBAR_SEQUENCDE_SEPARATOR_4    17
#define  MENUBAR_SEQUENCDE_SIMULATE       18      /* callback function: MENU_Simulate */
#define  MENUBAR_SEQUENCDE_SEPARATOR_5    19
#define  MENUBAR_SEQUENCDE_DETECTLOOPS    20      /* callback function: MENU_detectLoops */
#define  MENUBAR_SPECIAL                  21
#define  MENUBAR_SPECIAL_CONFIG           22      /* callback function: MENU_config */
#define  MENUBAR_SPECIAL_SEPARATOR_7      23
#define  MENUBAR_SPECIAL_INITBOARDS       24      /* callback function: MENU_InitBoards */
#define  MENUBAR_ABOUT                    25      /* callback function: MENU_About */


     /* Callback Prototypes: */

int  CVICALLBACK ADDRSIM_panel_CB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BLOCK_loopAbort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BLOCK_loopDone(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BLOCK_variablePulseAbort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BLOCK_variablePulseDone(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_remoteStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_Setup_Done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_skipWaitingForIdle_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_StartSequence(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_stopSequence(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_stopWaitingForIdle_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BTN_updateAutofocusVals(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_addCamera_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_editParameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_removeCamera_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_removeCommandFile_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_sendCommandFile_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CCD_writeCommandFile_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_autoFocusSettingsChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_BTN_abort_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_BTN_adwinBoot(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_BTN_adwinGetSystemInfo(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_BUTTON_clicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_convertPathBrowse_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_GPIB_abortDeviceSettings(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_GPIB_autoDetectDevices(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_GPIB_deviceSelected(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_GPIB_doneDeviceSettings(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_GPIB_EditDeviceSettings(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_idleSequenceDataPath_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_imageFilePathBrowse_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_isOfTypeIdleChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_panelChanged(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_pifocFocusVoltageLogFilePath_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONFIG_savePathBrowse_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CONGIG_GPIB_deviceSettingsChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DEBUGOUT_callback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_smallWfmPanelClicked(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_smallWfmPanelCtrlClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_smallWfmPanelDoneBtnClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_smallWfmPanelStepCtrlClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_splitterMoved(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DIGITALBLOCKS_VsplitterMoved(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EXTDEV_abort_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EXTDEV_deleteExtdev_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EXTDEV_done_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EXTDEV_newExtDev_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EXTDEV_TABLE_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_AttributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_autodetect_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_BTN_Reset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_deleteCommand_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_freq_TestDataAcquisition_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_listboxClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_NameChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_newCommand(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_newCommand_Abort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_newCommand_Done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_TabChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_TEST(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIB_transmitCommand_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIBcomand_attributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIBfreq_attributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIBpower_attributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIBPulse_attributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GPIBWfm_attributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IMPORT_BTN_Abort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IMPORT_BTN_Browse(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IMPORT_BTN_Done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IMPORT_SEQUENCE_changed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN2_panelChanged(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_ADWIN_testOffsetSend_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_autosaveSequences(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK MAIN_MENU_QuitProgram(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK MAIN_panelChanged(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_RENAMESEQ_abort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_RENAMESEQ_done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_saveSequenceNow_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_showSweepParameter_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_startIdle_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_stopIdle_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_SWEEP_close_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_tabClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MAIN_Timer_autoQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK MENU_About(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_Close(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_config(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_detectLoops(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_ImportNames(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_InitBoards(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_New(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_open(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_Save(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_SaveAs(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_SaveCopyAs(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_SequenceStart(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_SequenceStartFrom(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MENU_Simulate(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK NUM_DAC_Changed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onSendConfButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OUTPUT_suppressChanged_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PANEL_STARTFROM_callback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Sequence_List_Clicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SEQUENCE_moveInList_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SEQUENCE_moveInQueue_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SEQUENCE_parametersChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Sequence_Queue_Clicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SIMUALTE_changeRepetition_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SIMUALTE_update_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SIMULATE_axisChanged_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SIMULATE_panelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SIMULATE_showAll_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK STARTFROM_BTN_StartSequence(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_dac1_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_dac2_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_digital1_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_digital2_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_header1_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_header2_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLE_parameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLECONFIG_changed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TABLECONFIG_done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TESTfunction(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TITLE_close_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VARTIME_BTNAbort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VARTIME_BTNDone(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VARTIME_edited(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VARTIME_panelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WAVEFORMS_EditTable_Points(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WAVEFORMS_Points_attributeChanged_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_addChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_changeDisorder(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_changeMatrix(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_DMD_changeRepetitionFunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_DMD_TABLE_parameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_getParameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_selectTestFile2_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_selectTestFile_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_shift(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_showImageFile_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_SS_getParameters_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_TABLE_clicked_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_test(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDR_testPositionFeedback_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_ADDRESS_GRAPH1_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_browse_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_changeRepetitionFunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_changeRepetitionPoints(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_deleteUnusedWaveforms_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_deleteWaveform_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_Digitize(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_DMD_attributeChanged_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_DMD_transmitSeriesToDMD_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_DMD_transmitToDMD_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_FILE_AttributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_FUNC_attributeChanged_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_MAIN_WFMNameChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_newWaveform_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_Rescue_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_STEP_AttributeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_tabChanged_CALLBACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFM_TABLE_callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFMTYPE_abort_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WFMTYPE_done_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
