'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 2
' Initial_Processdelay           = 10000
' Eventsource                    = Timer
' Control_long_Delays_for_Stop   = No
' Priority                       = Low
' Priority_Low_Level             = 1
' Version                        = 1
' ADbasic_Version                = 6.0.0
' Optimize                       = Yes
' Optimize_Level                 = 2
' Stacksize                      = 1000
' Info_Last_Save                 = ATOMARRAY-PC  AtomArray-PC\Atom Array
'<Header End>
#include  .\ADBasic_ExperimentControl_inc.inc
#Include ADwinPro_All.inc



Dim i as long

Init:
 
  P2_OUTPUT_FLAG=0
  
  P2_counter = 0
  P2_EventCounter = 0
   
  P1_PAR_MAX_DIO_buffer = P1_MAX_DIO_buffer
  P1_PAR_MAX_AO_buffer =  P1_MAX_AOextBuffer
  P1_PAR_MAX_TRIGGERTIMES = P1_MAX_TRIGGERTIMES
  
  
Event:
  Processdelay = 10000 '3000
  inc P2_counter  
  
  selectcase P2_OUTPUT_FLAG
    case P2_OUTPUT_ALL  ' output all simultaneously
      
      P2_OUTPUT_FLAG=0
      par_60 = 1 'hannes debug
      
    case P2_OUTPUT_SINGLE_CHANNEL_DIGITAL
      P2_Digout (P2_OUTPUT_MODULE, P2_OUTPUT_CHANNEL, P2_OUTPUT_VALUE)
      inc P2_eventCounter
      P2_OUTPUT_FLAG=0
      par_60 = 2 'hannes debug
      par_58 = P2_OUTPUT_VALUE 'hannes debug

    case P2_OUTPUT_SINGLE_CHANNEL_ANALOG 
      P2_Dac(P2_OUTPUT_MODULE, P2_OUTPUT_CHANNEL, P2_OUTPUT_VALUE)
      inc P2_eventCounter
      P2_OUTPUT_FLAG=0
      par_60 = 3 'hannes debug
      par_57 = P2_OUTPUT_VALUE 'hannes debug
      
  endselect

    
