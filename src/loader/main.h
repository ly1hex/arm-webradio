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
#define VERSION                        "0.01"
#define RELEASE                        //Release version
#define APPNAME                        "Loader"
#if defined DEBUG
# define APPVERSION                    VERSION"d"
#elif defined RELEASE
# define APPVERSION                    VERSION
#else
# define APPVERSION                    VERSION"*"
#endif

//Loader settings
#define APPSTARTADDR                   (0x5000)  // 20kByte
#define FLASHSIZE                      (0x40000) //256 kByte
#define FLASHBUF                       (1024)
#define FIRMWARE_FILE                  "/FIRMWARE.BIN"
#define FIRMWARE_BAKFILE               "/FIRMWARE.BAK"


//----- PROTOTYPES -----
void                                   debugout(const char *s, ...);
void                                   uart_puts(const char *s);
void                                   uart_putc(unsigned int c);

void                                   systick(void);
void                                   delay_ms(unsigned int ms);

long                                   backup_app(const char* fname);
long                                   flash_app(const char* fname);
void                                   start_app(void);


#endif //_MAIN_H_
