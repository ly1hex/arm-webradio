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
#define VERSION                        "0.06"
#define RELEASE                        //Release version
#define APPNAME                        "WebRadio" //max 15 characters
#if defined DEBUG
# define APPVERSION                    VERSION"d"
#elif defined RELEASE
# define APPVERSION                    VERSION
#else
# define APPVERSION                    VERSION"*"
#endif

#define COMPILE_YEAR  ((((__DATE__ [7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))

#define COMPILE_MONTH ( (__DATE__[2] == 'n') ? (__DATE__[1] == 'a' ? 1 : 6) \
                      : (__DATE__[2] == 'b') ? 2  \
                      : (__DATE__[2] == 'r') ? (__DATE__[1] == 'a' ? 3 : 4) \
                      : (__DATE__[2] == 'y') ? 5  \
                      : (__DATE__[2] == 'l') ? 7  \
                      : (__DATE__[2] == 'g') ? 8  \
                      : (__DATE__[2] == 'p') ? 9  \
                      : (__DATE__[2] == 't') ? 10 \
                      : (__DATE__[2] == 'v') ? 11 : 12)

#define COMPILE_DAY   ((((__DATE__ [4]==' ')?0:(__DATE__[4]-'0'))*10)+(__DATE__[5]-'0')) 

//Max characters
#define MAX_NAME                       (32)  // 31 chars + zero (min 32)  "Station Name"
#define MAX_INFO                       (128) //127 chars + zero (min 32)  "Station info"
#define MAX_ADDR                       (256) //255 chars + zero           "http://192.168.0.100/stream.mp3" or "/test/abc/xyz.mp3"
#define MAX_URLFILE                    (64)  // 63 chars + zero           "/stream.mp3"

#define DRAWALL                        (1<<0)
#define SEC_CHANGED                    (1<<1)
#define MIN_CHANGED                    (1<<2)
#define HOUR_CHANGED                   (1<<3)
#define DAY_CHANGED                    (1<<4)


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

unsigned int                           standby_isactive(void);
unsigned int                           standby(unsigned int param);


#endif //_MAIN_H_
