


#define LECROY_MAX_POINTS 500000


t_digitizeParameters *digitizeParameters_LeCroyLW120 (int channel, unsigned long timebase);

t_gpibDevice *GPIB_deviceParameters_LeCroyLW120  (void);


#ifdef EXPERIMENTCONTROL

//int GPIB_transmitWaveform_LeCroy (t_sequence *seq, t_gpibCommand *g, int transmitData);

#endif
