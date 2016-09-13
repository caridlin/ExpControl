#ifndef GUI_EXTDEV
#define GUI_EXTDEV


void EXTDEV_initPanel (void);
void EXTDEV_TABLE_displayAllExtDevs (int panel, int control, t_sequence *seq, int activeNr, int sort);
void EXTDEV_TABLE_init (int panel, int control);
t_ext_device *EXTDEV_ptr (t_sequence *seq, int extdevID);
t_ext_device *EXTDEV_ptrByID (t_sequence *seq, int ID);
char *EXTDEV_typeStr (t_ext_device *extdev);
void EXTDEV_free (t_ext_device *w);



#endif // GUI_EXTDEV      
