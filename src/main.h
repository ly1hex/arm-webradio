#ifndef _MAIN_H_
#define _MAIN_H_


//----- DEFINES -----
#if !defined(DEBUG)
//# define DEBUG                        //switch debug output on
#endif

//Debug settings
#if defined(DEBUG)
# define DEBUGOUT                       debugout //debugout
# define DEBUGBAUD                      115200   //uart baud rate
#else
# define DEBUGOUT                       //do nothing
#endif

//Application settings
#define APPNAME                        "WebRadio" //max 15 characters
#define APPVERSION                     "0.02"
//#define APPRELEASE                     //Release version
#if defined(APPRELEASE)
# define APPRELEASE_SYM ""
#else
# define APPRELEASE_SYM "*"
#endif

//Max characters
#define MAX_NAME                       (32)  // 31 chars + zero (min 32)  "Station Name"
#define MAX_INFO                       (128) //127 chars + zero (min 32)  "Station info"
#define MAX_ADDR                       (256) //255 chars + zero           "http://192.168.0.100/stream.mp3" or "/test/abc/xyz.mp3"
#define MAX_URLFILE                    (64)  // 63 chars + zero           "/stream.mp3"

#define SEC_CHANGED                    (1<<0)
#define MIN_CHANGED                    (1<<1)
#define DAY_CHANGED                    (1<<2)


//----- PROTOTYPES -----
void                                   debugout(const char *s, ...);
void                                   uart_puts(const char *s);
void                                   uart_putc(unsigned int c);

void                                   systick(void);
char*                                  getclock(void);
char*                                  getdate(void);
void                                   gettime(TIME* t);
void                                   settime(unsigned long s);
long                                   getdeltatime(long t);
long                                   getontime(void);
void                                   delay_ms(unsigned int ms);
unsigned int                           getmstime(void);

unsigned int                           standby_state(void);
unsigned int                           standby(unsigned int param);


#endif //_MAIN_H_
