#ifndef _IO_H_
#define _IO_H_


//----- DEFINES -----
//CPU Freq.
// 50.0 MHz - SYSCTL_SYSDIV_4
// 40.0 MHz - SYSCTL_SYSDIV_5
// 33.3 MHz - SYSCTL_SYSDIV_6
// 28.6 MHz - SYSCTL_SYSDIV_7
// 25.0 MHz - SYSCTL_SYSDIV_8
// 22.2 MHz - SYSCTL_SYSDIV_9
// 20.0 MHz - SYSCTL_SYSDIV_10
#define LM3S_SYSDIV                    SYSCTL_SYSDIV_6

//LM3S6950 Revision: LM3S_REV_A1, LM3S_REV_A2 or LM3S_REV_B0
#define LM3S_REV_A2

//Display: L2F50, LPH88, LS020
#define LS020
//#define LCD_MIRROR                     //mirror display
#define LCD_PWMFREQ                    (60000) //60000 Hz LED PWM Freq
#define LCD_PWMMIN                     (5)     // 5 % (1...100%)
#define LCD_PWMSTANDBY                 (10)    //10 % (1...100%)

//SSI Speed: LCD, SD, F-RAM, VS
#define DEFAULT_SSISPEED               (8000000) //8 MHz
#define DEFAULT_SSILOWSPEED            (2000000) //2 MHz (init and standby)

//Rotary encoder switch
#define SW_SHORTTIME                   (8)   //  8*10ms =   80ms
#define SW_LONGTIME                    (120) //120*10ms = 1200ms

//IR
#define IR_RECEIVER                    //ir support on
#define IR_BITTIME                     (1778) //1.778 ms
#define IR_MAXERR                      (150)  //150 us (max. half bit time error)
#define IR_DETECT                      (0)
#define IR_STARTBIT1                   (1)
#define IR_STARTBIT2                   (2)
#define IR_DATABIT                     (3)
#define IR_TV1                         (0)
#define IR_TV2                         (1)
#define IR_VCR1                        (5)
#define IR_VCR2                        (6)
#define IR_ALLADDR                     (0x1F)

//Encoder
#define ENC_PHA_READ()                 GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1)
#define ENC_PHB_READ()                 GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)
#define ENC_SW_READ()                  GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4)
//IR
#define IR_READ()                      GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)
//LCD
#define LCD_RST_DISABLE()              GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5)
#define LCD_RST_ENABLE()               GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0)
#define LCD_CS_DISABLE()               GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_PIN_6)
#define LCD_CS_ENABLE()                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0)
#define LCD_RS_DISABLE()               GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)
#define LCD_RS_ENABLE()                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0)
//SD
#define MMC_POWERON()                  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);          GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD)
#define MMC_POWEROFF()                 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0); GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD)
#define MMC_SELECT()                   GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0)
#define MMC_DESELECT()                 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1)
#define MMC_SCK_LOW()                  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0)
#define MMC_SCK_HIGH()                 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0)
#define MMC_SI_LOW()                   GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0)
#define MMC_SI_HIGH()                  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_PIN_3)
//VS
#define VS_DREQ_READ()                 GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_1)
#define VS_RST_DISABLE()               GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_PIN_7)
#define VS_RST_ENABLE()                GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, 0)
#define VS_CS_DISABLE()                GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6)
#define VS_CS_ENABLE()                 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0)
#define VS_DCS_DISABLE()               GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3)
#define VS_DCS_ENABLE()                GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0)
//USB Power
#define USB_OFF()                      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1)
#define USB_ON()                       GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); delay_ms(500)
//FM
#define FM_CS_DISABLE()                GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_PIN_1)
#define FM_CS_ENABLE()                 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0)

//Commandos
#define SW_PRESSED                    (1)
#define SW_PRESSEDLONG                (2)
#define SW_VOL_P                      (3)
#define SW_VOL_M                      (4)
#define SW_UP                         (5)
#define SW_DOWN                       (6)
#define SW_ENTER                      (7)
#define SW_POWER                      (8)


//----- PROTOTYPES -----
void                                   ethernet_setmac(uint64_t mac);
unsigned int                           ethernet_data(void);
void                                   ethernet_put(unsigned char *pkt, unsigned int len);
unsigned int                           ethernet_get(unsigned char *pkt, unsigned int len);

void                                   vs_ssi_wait(void);
void                                   vs_ssi_writewait(void);
void                                   vs_ssi_write(unsigned char c);
unsigned char                          vs_ssi_readwrite(unsigned char c);
void                                   vs_ssi_speed(unsigned long speed);

void                                   ssi_wait(void);
void                                   ssi_write(unsigned char c);
unsigned char                          ssi_readwrite(unsigned char c);
void                                   ssi_speed(unsigned long speed);
void                                   ssi_off(void);
void                                   ssi_on(void);
void                                   ssi_defaultspeed(unsigned long speed);

void                                   pwm_led(unsigned int power);

int                                    ir_cmd(void);
void                                   ir_showdata(void);
void                                   ir_timer(void);
void                                   ir_edge(void);
unsigned int                           ir_addr(void);
void                                   ir_setaddr(unsigned int addr);
void                                   ir_init(void);

int                                    keys_steps(void);
int                                    keys_sw(void);
void                                   keys_timerservice(void);

void                                   cpu_speed(unsigned int low_speed);
void                                   init_bor(unsigned int on);
void                                   init_periph(void);
void                                   init_pins(void);

void                                   GPIOSetOutputOD(unsigned long port, unsigned char pins);
void                                   GPIOSetOutput(unsigned long port, unsigned char pins);
void                                   GPIOSetInput(unsigned long port, unsigned char pins);


#endif //_IO_H_
