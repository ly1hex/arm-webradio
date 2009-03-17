#ifndef _VS_H_
#define _VS_H_


//----- DEFINES -----
#define DEFAULT_VS                     (1053)   //1033 or 1053
#define DEFAULT_VOLUME                 (40)     //   0 -   100 %
#define DEFAULT_BASSAMP                (5)      //   0 -    15 dB
#define DEFAULT_BASSFREQ               (100)    //  20 -   150 Hz
#define DEFAULT_TREBLEAMP              (0)      //  -8 -     7 dB
#define DEFAULT_TREBLEFREQ             (15000)  //1000 - 15000 Hz

//Clock
#define VS_XTAL                        (12288000UL)
//Opcode
#define VS_READ                        (0x03)
#define VS_WRITE                       (0x02)
//Register
#define VS_MODE                        (0x00)   //Mode control
#define SM_RESET                       (1<< 2)  //Soft Reset
#define SM_CANCEL                      (1<< 3)  //Cancel Decoding
#define SM_STREAM                      (1<< 6)  //Stream Mode
#define SM_SDINEW                      (1<<11)  //VS1002 native SPI modes
#define VS_STATUS                      (0x01)   //Status
#define VS_BASS                        (0x02)   //Built-in bass/treble enhancer
#define VS_CLOCKF                      (0x03)   //Clock freq + multiplier
#define SC_MUL_2X                      (0x2000)
#define SC_MUL_4X                      (0xA000)
#define VS_DECODETIME                  (0x04)   //Decode time in seconds
#define VS_AUDATA                      (0x05)   //Misc. audio data
#define VS_WRAM                        (0x06)   //RAM write/read
#define VS_WRAMADDR                    (0x07)   //Base address for RAM write/read
#define VS_HDAT0                       (0x08)   //Stream header data 0
#define VS_HDAT1                       (0x09)   //Stream header data 1
#define VS_AIADDR                      (0x0A)   //Start address of application
#define VS_VOL                         (0x0B)   //Volume control
//RAM Data
#define VS_RAM_ENDFILLBYTE             (0x1E06)  //End fill byte


//----- PROTOTYPES -----
void                                   vs_plugin(unsigned int vs);
int                                    vs_treblefreq(void);
void                                   vs_settreblefreq(int freq); //1000 - 15000 Hz
int                                    vs_trebleamp(void);
void                                   vs_settrebleamp(int amp);   //  -8 -     7 dB
int                                    vs_bassfreq(void);
void                                   vs_setbassfreq(int freq);   //  20 -   150 Hz
int                                    vs_bassamp(void);
void                                   vs_setbassamp( int amp);    //   0 -    15 dB
int                                    vs_volume(void);
void                                   vs_setvolume(int vol);      //   0 -   100 %, 0=off
unsigned int                           vs_request(void);
void                                   vs_requesthandler(void);
void                                   vs_data(unsigned int c);
void                                   vs_write_bass(void);
void                                   vs_write_volume(void);
unsigned long                          vs_read_ram(unsigned int addr);
void                                   vs_write_reg(unsigned int reg, unsigned int data);
unsigned int                           vs_read_reg(unsigned int reg);
void                                   vs_write(unsigned int c);
unsigned int                           vs_read(void);
void                                   vs_pause(void);
void                                   vs_play(void);
void                                   vs_stopstream(void);
void                                   vs_stop(void);
void                                   vs_start(void);
void                                   vs_init(unsigned int vs);


#endif //_VS_H_
