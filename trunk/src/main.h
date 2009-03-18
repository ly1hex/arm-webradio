#ifndef _MAIN_H_
#define _MAIN_H_


//----- DEFINES -----
#if !defined(DEBUG)
//# define DEBUG
#endif

#if defined(DEBUG)
# define DEBUGOUT                       debugout //debugout menu_debug
# define DEBUGBAUD                      115200
#else
# define DEBUGOUT                       //debugout menu_debug
#endif

#define APPNAME                        "WebRadio" //max 15 characters
#define APPVERSION                     "0.01"
//#define APPRELEASE                     //Release version

//Max characters
#define MAX_NAME                       (32)  // 31 chars + zero (min 32)  "Test Station"
#define MAX_INFOTXT                    (64)  // 63 chars + zero (min 32)  "Station info"
#define MAX_URL                        (64)  // 63 chars + zero           "/stream.mp3"
#define MAX_ADDR                       (128) //127 chars + zero           "http://192.168.0.100/stream.mp3"
#define MAX_FILE                       (128) //127 chars + zero (min 128) "/test/abc/xyz.mp3"

#define SEC_CHANGED                    (1<<0)
#define MIN_CHANGED                    (1<<1)
#define DAY_CHANGED                    (1<<2)


//----- PROTOTYPES -----
void                                   systick(void);
void                                   debugout(const char *s, ...);
void                                   uart_puts(const char *s);
void                                   uart_putc(unsigned int c);

char*                                  getclock(void);
char*                                  getdate(void);
void                                   settime(unsigned long s);
long                                   getdeltatime(long time);
long                                   getontime(void);
void                                   delay_ms(unsigned int ms);
unsigned int                           getmstime(void);

unsigned int                           standby(unsigned int param);


#endif //_MAIN_H_
