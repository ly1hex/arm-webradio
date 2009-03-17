#ifndef _SETTINGS_H_
#define _SETTINGS_H_


//----- DEFINES -----
#define SETTINGS_FILE                  "SETTINGS.INI"


//----- PROTOTYPES -----
void                                   settings_read(void);
unsigned int                           settings_openitem(unsigned int item);
void                                   settings_getitem(unsigned int item, char *name);
unsigned int                           settings_items(void);


#endif //_SETTINGS_H_
