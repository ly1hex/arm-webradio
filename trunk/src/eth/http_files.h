#ifndef _HTTP_FILES_H_
#define _HTTP_FILES_H_


//----- DEFINES -----
#define HTML_FILE                      (0)
#define XML_FILE                       (1)
#define CSS_FILE                       (2)
#define ICON_FILE                      (3)

typedef struct
{
  const char *name;
  unsigned int type;
  const unsigned char *data;
  unsigned int len;
} HTTPFILE;

typedef struct
{
  const char *name; //name
  const unsigned int format; //format
  void*(*func)(void); 
} HTTPVAR;


//----- PROTOTYPES -----
unsigned int                           http_printf(char *dst, unsigned int format, ...);
unsigned int                           http_parse(char *dst, const char *src);
unsigned int                           http_getfile(unsigned char *dst, const char *name);


#endif //_HTTP_FILES_H_
