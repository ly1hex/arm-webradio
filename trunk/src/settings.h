#ifndef _SETTINGS_H_
#define _SETTINGS_H_


//----- DEFINES -----
#define SETTINGS_FILE                  "SETTINGS.INI"


typedef struct
{
  const char *name; //name
  const char *ini; //name
  const int format; //format
  const int p1; //param 1
  const int p2; //param 2
  const int p3; //param 3
  void(*set)(void*);
} SETTINGSMENU;


//----- PROTOTYPES -----
void                                   settings_read(void);
unsigned int                           settings_openitem(unsigned int item);
void                                   settings_getitem(unsigned int item, char *name);
unsigned int                           settings_items(void);
void                                   settings_init(void);


#endif //_SETTINGS_H_
