'<ADbasic Header, Headerversion 001.001>
'<Header End>

#define N_DIO_MODULES 3
#define DIO1 1
#define DIO2 2
#define DIO3 3

#define N_AOUT_MODULES 4

#define AOUT1 4
#define AOUT2 5
#define AOUT3 6
#define AOUT4 7

#define CYCLE         par_1
#define STOP_FLAG     par_2
#define MAX_CYCLE_NO  par_3


#define DIO_EVENT_CTR par_4

#define MAX_DIO_EVENT_TIMES 3000
#define N_DIO_EVENTS par_5

#define EVENT_TIMES_DIO data_1
#define DIO_VALUES1 data_2  ' dio values for module 1
#define DIO_VALUES2 data_3  ' dio values for module 2
#define DIO_VALUES3 data_4  ' dio values for module 3

dim EVENT_TIMES_DIO[MAX_DIO_EVENT_TIMES] as long at DM_LOCAL
dim DIO_VALUES1[MAX_DIO_EVENT_TIMES] as long at DM_LOCAL
dim DIO_VALUES2[MAX_DIO_EVENT_TIMES] as long at DM_LOCAL
dim DIO_VALUES3[MAX_DIO_EVENT_TIMES] as long at DM_LOCAL


#define MAX_AO_EVENT_TIMES 2500
#define AO_ARRAY_SIZE      10000

'#define MAX_AO_EVENT_TIMES 2500000
'#define AO_ARRAY_SIZE      10000000

#define N_AO_EVENTS par_10
#define AO_EVENT_CTR par_11


#define EVENT_TIMES_AO data_10
#define AO_VALUES1 data_11
#define AO_VALUES2 data_12
#define AO_VALUES3 data_13
#define AO_VALUES4 data_14


#define TIME_MEAS data_15
#define COPY_BUFFER data_16



dim EVENT_TIMES_AO[MAX_AO_EVENT_TIMES] as long at EM_LOCAL

#define N_TIMES 10000
dim TIME_MEAS[N_TIMES] as long at DM_LOCAL


dim AO_VALUES1[AO_ARRAY_SIZE] as long at DM_LOCAL 
dim AO_VALUES2[AO_ARRAY_SIZE] as long at DM_LOCAL
dim AO_VALUES3[AO_ARRAY_SIZE] as long at DM_LOCAL
dim AO_VALUES4[AO_ARRAY_SIZE] as long 'at EM_LOCAL

dim copy_buffer[AO_ARRAY_SIZE] as long at DM_LOCAL
 








