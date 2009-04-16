#ifndef _MENU_DLG_H_
#define _MENU_DLG_H_


//----- DEFINES -----


//----- PROTOTYPES -----

int                                    dlg_str(const char* title, const char *value, char *buf, unsigned int len);
int                                    dlg_rawir(unsigned int i);
int                                    dlg_msg(const char* title, const char *msg);
int                                    dlg_service(void);


#endif //_MENU_DLG_H_
