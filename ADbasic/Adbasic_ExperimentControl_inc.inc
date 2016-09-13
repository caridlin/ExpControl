'<ADbasic Header, Headerversion 001.001>
'<Header End>


'========================================
'    general definitions
'========================================





#define TIME_NEEDED_FOR_COPY 150
#define AO_VALUES_TO_COPY 32

'
'      process 1:    
'

#define P1_CYCLE_NO      par_1
#define P1_MAX_CYCLE_NO  par_2
#define P1_STOP_FLAG     par_3
#define P1_ERRORCODE     par_4

#define ERRORCODE_AO_BUFFER_EMPTY -1
#define ERRORCODE_PROCESS_TOO_LONG -2


'========================================
'  digital output
'========================================
#define P1_nDIOelements        par_5
#define P1_DIOelementIndex     par_6  
#define P1_DIObuffer data_1  ' dio values for module 1
#define P1_MAX_DIO_buffer 32768  'N_DIO_MODULES * P1_MAX_DIO_TIMESTAMPS 
dim P1_DIObuffer[P1_MAX_DIO_buffer] as long at DM_LOCAL
#define P1_PAR_MAX_DIO_buffer par_7

'
'   array containing the DIO_modules
' 
#define P1_DIOmodules data_2
dim P1_DIOmodules[13] as long at DM_LOCAL
#define P1_nDIOmodules par_8


'========================================
'  analog output
'========================================
#define P1_AOnElements       par_10
#define P1_AOeventIndex      par_11
#define AO_readIndex         par_12
#define AO_writeIndex        par_13
#define P1_PROCESS_DELAY    1000 '300

#define P1_AOextBuffer       data_3
#define P1_AOlocalBuffer     data_4
#define P1_AOvaluesAtEndOfSequence data_5
#define P1_AOoffsetsDuringSequence data_8

' 
#define P1_AOmodules data_6
dim P1_AOmodules[13] as long at DM_LOCAL
#define P1_nAOmodules par_14


#define N_MAX_DAC_CHANNELS 96
dim P1_AOvaluesAtEndOfSequence[N_MAX_DAC_CHANNELS] as long at PM_LOCAL

#define  MAX_OFFSET_INDEX 208
dim P1_AOoffsetsDuringSequence[MAX_OFFSET_INDEX] as long at DM_LOCAL


#define P1_MAX_AOextBuffer 50000000
#define P1_MAX_AOlocalBuffer 32768
#define P1_PAR_MAX_AO_buffer par_15


dim P1_AOextBuffer[P1_MAX_AOextBuffer] as long at DRAM_EXTERN
dim P1_AOlocalBuffer[P1_MAX_AOlocalBuffer] as long at PM_LOCAL


'===========================================
'          trigger
'===========================================
#define P1_trigger    par_26
#define P1_TRIGGER_TIME_DURING_SEQ data_7
#define P1_TRIGGERTIME_INDEX par_27
#define P1_PAR_N_TRIGGERTIMES par_28
#define P1_MAX_TRIGGERTIMES 301

#define P1_PAR_MAX_TRIGGERTIMES par_29



dim P1_TRIGGER_TIME_DURING_SEQ[P1_MAX_TRIGGERTIMES] as long at DM_LOCAL


#define TRIGGER_DIGIO0_RAISING_EDGE   00011b 
#define TRIGGER_DIGIO1_RAISING_EDGE   110000b 

'#define TRIGGER_DIGIO1_RAISING_EDGE   110000b 


'========================================
'  Process 2: direct output
'========================================
 
#define P2_OUTPUT_FLAG par_20
#define P2_OUTPUT_ALL 1
#define P2_OUTPUT_SINGLE_CHANNEL_ANALOG 2
#define P2_OUTPUT_SINGLE_CHANNEL_DIGITAL 3

#define P2_OUTPUT_MODULE par_21
#define P2_OUTPUT_CHANNEL par_22
#define P2_OUTPUT_VALUE par_23


#define P2_counter par_24
#define P2_EventCounter par_25


#define t1 par_79
#define t2 par_80
'
' array for measurement of times
'
#define TIME_MEAS data_15
#define N_TIMES 10000
dim TIME_MEAS[N_TIMES] as long at DM_LOCAL


#define copy_buffer data_22
#define AO_ARRAY_SIZE 5000
dim copy_buffer[AO_ARRAY_SIZE] as long at DM_LOCAL






#define nValuesToCopy par_40
#define module par_41
#define tmp par_42
#define value par_43
#define channel par_44
#define extBufferReadIndex par_45
#define extBufferWriteIndex par_46
#define oIndex par_47



