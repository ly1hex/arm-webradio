#ifndef _MAIN_H_
#define _MAIN_H_


//----- DEFINES -----
//Debug settings
#ifndef DEBUG
//# define DEBUG                        //switch debug output on
#endif
#define DEBUGBAUD                       115200   //uart baud rate: 115200
#define DEBUGUART                       UART2_BASE //uart: UART1_BASE UART2_BASE
#ifdef DEBUG
# define DEBUGOUT                       debugout //debugout
#else
# define DEBUGOUT                       //do nothing
#endif

//Application settings
#define VERSION                        "0.04"
#define RELEASE                        //Release version
#define APPNAME                        "WebRadio" //max 15 characters
#if defined DEBUG
# define APPVERSION                    VERSION"d"
#elif defined RELEASE
# define APPVERSION                    VERSION
#else
# define APPVERSION                    VERSION"*"
#endif

//Max characters
#define MAX_NAME                       (32)  // 31 chars + zero (min 32)  "Station Name"
#define MAX_INFO                       (128) //127 chars + zero (min 32)  "Station info"
#define MAX_ADDR                       (256) //255 chars + zero           "http://192.168.0.100/stream.mp3" or "/test/abc/xyz.mp3"
#define MAX_URLFILE                    (64)  // 63 chars + zero           "/stream.mp3"

#define SEC_CHANGED                    (1<<0)
#define MIN_CHANGED                    (1<<1)
#define HOUR_CHANGED                   (1<<2)
#define DAY_CHANGED                    (1<<3)


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
