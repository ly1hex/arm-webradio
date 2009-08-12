#ifndef _TOOLS_H_
#define _TOOLS_H_


//----- DEFINES -----
typedef struct
{
  unsigned int year;
  unsigned int month; //1-12
  unsigned int day;   //1-31
  unsigned int wday;  //0-6 (0=Sunday)
  unsigned int h;
  unsigned int m;
  unsigned int s;
} TIME;


//----- PROTOTYPES -----
void                                   strshrinkpath(char *path);
char*                                  strrmvspace(char *dst, const char *src);
char*                                  strtoupper(char *dst, const char *src);
int                                    strstrk(char *dst, const char *src, const char *key);
const char*                            strstri(const char *s, const char *pattern);
int                                    strncmpi(const char *s1, const char *s2, size_t n);

void                                   uitoa(unsigned long n, char *str);

unsigned long                          atoui_hex(const char *s);
unsigned long                          atoui(const char *s);
unsigned int                           atorgb(const char *s);

void                                   sectotime(unsigned long s, TIME *time);
void                                   daytime(char *s, TIME *time);


#endif //_TOOLS_H_
