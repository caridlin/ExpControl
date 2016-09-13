


t_gpibDevice *GPIB_deviceParameters_Agilent33250A (void);

t_gpibDevice *GPIB_deviceParameters_Keithley3390 (void);

t_gpibDevice *GPIB_deviceParameters_Agilent33120A (void);

t_gpibDevice *GPIB_deviceParameters_Agilent83751A (void);

t_gpibDevice *GPIB_deviceParameters_Agilent6612C(void);

t_gpibDevice *GPIB_deviceParameters_Agilent33220A (void);

t_gpibDevice *GPIB_deviceParameters_AgilentN5182A (void);

t_gpibDevice *GPIB_deviceParameters_AgilentN5182B (void);  

t_gpibDevice *GPIB_deviceParameters_AgilentE3643A(void);

t_gpibDevice *GPIB_deviceParameters_AgilentE4432B(void);       


int GPIB_showDeviceMessage_Agilent33250A (t_gpibCommand *g, char *str, int showMessage);


int GPIB_transmitFreq_Agilent83751A (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2);

int GPIB_transmitFreq_Agilent5182B (t_sequence *seq, t_gpibCommand *g, int dummy, int dummy2);


