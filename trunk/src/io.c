#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "third_party/lmi/inc/hw_types.h"
#include "third_party/lmi/inc/hw_memmap.h"
#include "third_party/lmi/inc/hw_ints.h"
#include "third_party/lmi/inc/hw_ssi.h"
#include "third_party/lmi/driverlib/sysctl.h"
#include "third_party/lmi/driverlib/gpio.h"
#include "third_party/lmi/driverlib/interrupt.h"
#include "third_party/lmi/driverlib/systick.h"
#include "third_party/lmi/driverlib/timer.h"
#include "third_party/lmi/driverlib/watchdog.h"
#include "third_party/lmi/driverlib/pwm.h"
#include "third_party/lmi/driverlib/qei.h"
#include "third_party/lmi/driverlib/ssi.h"
#include "third_party/lmi/driverlib/uart.h"
#include "third_party/lmi/driverlib/ethernet.h"
#include "tools.h"
#include "main.h"
#include "io.h"


volatile int sw_pressed=0, ir_data=0;
unsigned long ir_address=0, ir_status=IR_DETECT, ir_1us=0;
unsigned long ssi_defspeed=0;


void ethernet_setmac(uint64_t mac)
{
  EthernetDisable(ETH_BASE);
  EthernetMACAddrSet(ETH_BASE, (unsigned char *)&mac);
  EthernetEnable(ETH_BASE);

  return;
}


unsigned int ethernet_data(void)
{
  return EthernetPacketAvail(ETH_BASE);
}


void ethernet_put(unsigned char *pkt, unsigned int len)
{
  if(EthernetPacketPut(ETH_BASE, pkt, len) != len)
  {
    DEBUGOUT("Eth: Tx put err\n");
  }

  return;
}


unsigned int ethernet_get(unsigned char *pkt, unsigned int len)
{
#if defined(DEBUG)
  unsigned long status = EthernetIntStatus(ETH_BASE, 0);
  if(status & ETH_INT_RXER)
  {
    DEBUGOUT("Eth: Rx err\n");
  }
  if(status & ETH_INT_RXOF)
  {
    DEBUGOUT("Eth: Rx overflow\n");
  }
  if(status & ETH_INT_TXER)
  {
    DEBUGOUT("Eth: Tx err\n");
  }
  EthernetIntClear(ETH_BASE, status);
#endif

  return EthernetPacketGetNonBlocking(ETH_BASE, pkt, len);
}


void vs_ssi_wait(void)
{
  unsigned long r;

  while(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY);  //busy?
  //while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TFE));  //transmit fifo empty?

  while(SSIDataGetNonBlocking(SSI0_BASE, &r)); //clear receive fifo

  return;
}


void vs_ssi_writewait(void)
{
  while(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY);  //busy?

  return;
}


void vs_ssi_write(unsigned char c)
{
  SSIDataPut(SSI0_BASE, c);

  return;
}


unsigned char vs_ssi_readwrite(unsigned char c)
{
  unsigned long r;

  SSIDataPut(SSI0_BASE, c);
  SSIDataGet(SSI0_BASE, &r);

  return (unsigned char)r;
}


void vs_ssi_speed(unsigned long speed)
{
  if(speed == 0)
  {
    speed = ssi_defspeed;
  }

  SSIDisable(SSI0_BASE);
  SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, speed, 8);
  SSIEnable(SSI0_BASE);

  return;
}


void ssi_wait(void)
{
  unsigned long r;

  while(HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_BSY);  //busy?
  //while(!(HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TFE));  //transmit fifo empty?

  while(SSIDataGetNonBlocking(SSI1_BASE, &r)); //clear receive fifo

  return;
}


void ssi_write(unsigned char c)
{
  SSIDataPut(SSI1_BASE, c);

  return;
}


unsigned char ssi_read(void)
{
  unsigned long r;

  SSIDataGet(SSI1_BASE, &r);

  return (unsigned char)r;
}


unsigned char ssi_readwrite(unsigned char c)
{
  unsigned long r;

  SSIDataPut(SSI1_BASE, c);
  SSIDataGet(SSI1_BASE, &r);

  return (unsigned char)r;
}


void ssi_speed(unsigned long speed)
{
  if(speed == 0)
  {
    speed = ssi_defspeed;
  }

  SSIDisable(SSI1_BASE);
  SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, speed, 8);
  SSIEnable(SSI1_BASE);

  return;
}


void ssi_off(void)
{
  ssi_wait();

  GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_3); //SCK, SI = gpio
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_3, 0); //SCK, SI = low

  return;
}


void ssi_on(void)
{
  GPIOPinTypeSSI(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_3); //SCK, SI = ssi

  return;
}


void ssi_defaultspeed(unsigned long speed)
{
  ssi_defspeed = speed;

  ssi_speed(ssi_defspeed);
  vs_ssi_speed(ssi_defspeed);

  return;
}


void pwm_led(unsigned int power)
{
  unsigned long period, duty;

  if(power) //switch on
  {
    if(power < LCD_PWMMIN)
    {
      power = LCD_PWMMIN;
    }

    period = SysCtlClockGet() / LCD_PWMFREQ;
    duty   = (power*(period/2)) / 100;

    PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, period); //freq Hz
    PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, duty); //duty %
    PWMGenEnable(PWM_BASE, PWM_GEN_2); //GEN2 on
    PWMOutputState(PWM_BASE, PWM_OUT_5_BIT, 1); //PWM5 on
    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_7); //PWM5 assign to PWM
  }
  else //switch off
  {
    PWMOutputState(PWM_BASE, PWM_OUT_5_BIT, 0); //PWM5 off
    PWMGenDisable(PWM_BASE, PWM_GEN_2); //GEN2 off
    GPIOPinTypePWM(GPIO_PORTE_BASE, 0); //PWM5 release from PWM
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_7, 0); //PWM5 low
  }

  return;
}


int ir_cmd(void)
{
  int data, cmd=0;
  static int last_data=0;

  data    = ir_data;
  ir_data = 0;

  if(data != 0)
  {
    DEBUGOUT("IR: %i\n", (data&0x3F));

    switch(data&0x3F)
    {
      case 12: //standby
        if((last_data^(1<<11)) == data)
        {
          cmd  = SW_POWER;
          data = 0;
        }
        break;
      case 16: //volume++
        cmd = SW_VOL_P;
        break;
      case 17: //volume--
        cmd = SW_VOL_M;
        break;
      case 32: //up
        cmd = SW_UP;
        break;
      case 33: //down
        cmd = SW_DOWN;
        break;
      case 38: //OK
        cmd = SW_ENTER;
        break;
    }
    last_data = data;
  }

  return cmd;
}


int ir_rawdata(void)
{
  int data;

  data    = ir_data;
  ir_data = 0;

  return data;
}


void ir_timer(void)
{
#if defined(IR_RECEIVER)
  static unsigned int bit=0, data=0, last_data=0;

  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  if(bit == 0)
  {
    TimerLoadSet(TIMER0_BASE, TIMER_A, IR_BITTIME*ir_1us);
  }

  if(IR_READ() == 0) //1 bit
  {
    data <<= 1;
    data  |= 1;
  }
  else //0 bit
  {
    data <<= 1;
    //data  |= 0;
  }

  if(++bit == 12)
  {
    bit = 0;
    if(((data&0x07C0) == ir_address) || (ir_address == (IR_ALLADDR<<6)))
    {
      if(data != last_data)
      {
        last_data = data;
        if(ir_data == 0)
        {
          ir_data = data|0x8000;
        }
      }
      else
      {
        last_data = 0; //if button hold down get every 2nd cmd
      }
    }
    //DEBUGOUT("IR: raw %x\n", data);
    data = 0;
    ir_status = IR_DETECT;
    GPIOPinIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFFUL);
    GPIOPinIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
  }
#endif
  return;
}


void ir_edge(void)
{
#if defined(IR_RECEIVER)
  unsigned long time, delta;
  static unsigned long last_time=0UL;

  GPIOPinIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);

  time = TimerValueGet(TIMER0_BASE, TIMER_A);
  if(time > last_time)
  {
    last_time = time;
    return;
  }

  switch(ir_status)
  {
    case IR_DETECT: //high for 2x bit time before start bit
      if(IR_READ() == 0) //low
      {
        delta = (last_time-time)/ir_1us;
        if(delta > (IR_BITTIME*2))
        {
          ir_status = IR_STARTBIT1;
        }
      }
      last_time = time;
      break;

    case IR_STARTBIT1:
      if(IR_READ()) //high
      {
        delta = (last_time-time)/ir_1us;
        if((delta >= ((IR_BITTIME/2)-IR_MAXERR)) && 
           (delta <= ((IR_BITTIME/2)+IR_MAXERR)))
        {
          last_time = time;
          ir_status = IR_STARTBIT2;
          break;
        }
      }
      last_time = time;
      ir_status = IR_DETECT;
      break;

    case IR_STARTBIT2:
      if(IR_READ() == 0) //low
      {
        delta = (last_time-time)/ir_1us;
        if((delta > ((IR_BITTIME/2)-IR_MAXERR)) && 
           (delta < ((IR_BITTIME/2)+IR_MAXERR)))
        {
          last_time = 0xFFFFFFFFUL;
          ir_status = IR_DATABIT;
          TimerLoadSet(TIMER0_BASE, TIMER_A, (IR_BITTIME+(IR_BITTIME/4))*ir_1us);
          TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
          GPIOPinIntDisable(GPIO_PORTD_BASE, GPIO_PIN_2);
          TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
          break;
        }
      }
      last_time = time;
      ir_status = IR_DETECT;
      break;

    case IR_DATABIT:
      ir_status = IR_DETECT;
      last_time = time;
      break;
  }
#endif
  return;
}


unsigned int ir_getaddr(void)
{
  return (ir_address>>6);
}


void ir_setaddr(unsigned int addr)
{
  if(addr <= 31)
  {
    ir_address = addr<<6;
  }

  return;
}


void ir_init(void)
{
#if defined(IR_RECEIVER)
  ir_data    = 0;
  ir_status  = 0;
  ir_1us     = SysCtlClockGet() / 1000000UL;

  GPIOPinIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
  TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFFUL);
  GPIOPinIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
#endif
  return;
}


int keys_steps(void) //encoder: four step, qei config: two step (see errata)
{
  long i;

  i = QEIPositionGet(QEI_BASE);
  if(i)
  {
    QEIPositionSet(QEI_BASE, i&0x01);
    return (int)(long)i>>1;
  }

  return 0;
}


int keys_sw(void)
{
  int sw;

  sw = sw_pressed;
  sw_pressed = 0;

  return sw;
}


void keys_timerservice(void) //100 Hz
{
  static unsigned int state=0;

  if(ENC_SW_READ())
  {
    if(state > SW_LONGTIME) //1.0 s
    {
      if(sw_pressed == 0)
      {
        sw_pressed = SW_PRESSEDLONG;
      }
    }
    else if(state > SW_SHORTTIME)
    {
      if(sw_pressed == 0)
      {
        sw_pressed = SW_PRESSED;
      }
    }
    state = 0;
  }
  else
  {
    state++;
  }

  return;
}


void cpu_speed(unsigned int low_speed)
{
  delay_ms(10);

  IntMasterDisable();
  pwm_led(0);

  if(low_speed)
  {
//for rev A1 & A2 set LDO=2.75V for correct PLL function -> reset to 2.50V
#if defined(LM3S_REV_A1) || defined(LM3S_REV_A2)
    SysCtlLDOSet(SYSCTL_LDO_2_50V);
#endif
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ); //8 MHz
    SysTickDisable();
    SysTickPeriodSet(SysCtlClockGet() / 1000); //1 kHz
    SysTickEnable();
    pwm_led(LCD_PWMSTANDBY);
    ssi_defaultspeed(DEFAULT_SSILOWSPEED);
  }
  else
  {
//for rev A1 & A2 set LDO=2.75V for correct PLL function
#if defined(LM3S_REV_A1) || defined(LM3S_REV_A2)
    SysCtlLDOSet(SYSCTL_LDO_2_75V);
#endif
    SysCtlClockSet(LM3S_SYSDIV | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ); //speed up
    SysTickDisable();
    SysTickPeriodSet(SysCtlClockGet() / 1000); //1 kHz
    SysTickEnable();
    pwm_led(100);
    ssi_defaultspeed(DEFAULT_SSISPEED);
  }

  ir_init();
  IntMasterEnable();

  delay_ms(10);

#if defined(DEBUG)
  UARTDisable(UART1_BASE);
  UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), DEBUGBAUD, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
  UARTEnable(UART1_BASE);
#endif

  return;
}


void init_bor(unsigned int on)
{
  unsigned long reset;

  SysCtlIntClear(SYSCTL_INT_BOR);

  if(on)
  {
    reset = SYSCTL_BOR_RESET;
  }
  else
  {
    reset = 0;
  }

  SysCtlBrownOutConfigSet(reset | SYSCTL_BOR_RESAMPLE, 4000); //delay 4000 (<8192)

  return;
}


void init_periph(void)
{
  //init uart1 (debug output)
#if defined(DEBUG)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  SysCtlPeripheralReset(SYSCTL_PERIPH_UART1);
  UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), DEBUGBAUD, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
  UARTEnable(UART1_BASE);
#endif

  //init quadrature encoder
  SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI);
  SysCtlPeripheralReset(SYSCTL_PERIPH_QEI);
  QEIConfigure(QEI_BASE, QEI_CONFIG_CAPTURE_A | QEI_CONFIG_NO_RESET | QEI_CONFIG_QUADRATURE | QEI_CONFIG_SWAP, 0xFFFFFFFF);
  GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_1); //PHA
  GPIOPinTypeQEI(GPIO_PORTF_BASE, GPIO_PIN_0); //PHB
  QEIPositionSet(QEI_BASE, 0);
  QEIEnable(QEI_BASE);

  //init ir receiver
#if defined(IR_RECEIVER)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER0);
  TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER); //32bit periodic timer
  TimerIntRegister(TIMER0_BASE, TIMER_BOTH, ir_timer);
  TimerEnable(TIMER0_BASE, TIMER_BOTH);
  GPIOPortIntRegister(GPIO_PORTD_BASE, ir_edge);
  GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_BOTH_EDGES); //GPIO_FALLING_EDGE GPIO_RISING_EDGE GPIO_BOTH_EDGES
  GPIOPinIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
  GPIOPortIntRegister(GPIO_PORTD_BASE, ir_edge);
#endif

  //init pwm for backlight
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
  SysCtlPeripheralReset(SYSCTL_PERIPH_PWM);
  PWMGenConfigure(PWM_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

  //init ssi0: VS
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
  SysCtlPeripheralReset(SYSCTL_PERIPH_SSI0);
  SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, DEFAULT_SSILOWSPEED, 8);
  SSIEnable(SSI0_BASE);
  SSIDataPut(SSI0_BASE, 0xff); SSIDataPut(SSI0_BASE, 0xff); SSIDataPut(SSI0_BASE, 0xff); //dummy write to set ssi fifo bits

  //init ssi1: LCD, SD, F-RAM
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
  SysCtlPeripheralReset(SYSCTL_PERIPH_SSI1);
  SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, DEFAULT_SSILOWSPEED, 8);
  SSIEnable(SSI1_BASE);
  SSIDataPut(SSI1_BASE, 0xff); SSIDataPut(SSI1_BASE, 0xff); SSIDataPut(SSI1_BASE, 0xff); //dummy write to set ssi fifo bits

  //set default ssi speed
  ssi_defaultspeed(DEFAULT_SSILOWSPEED);

  //init ethernet
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
  SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);
  EthernetInitExpClk(ETH_BASE, SysCtlClockGet());
  EthernetConfigSet(ETH_BASE, ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN | ETH_CFG_TX_PADEN | ETH_CFG_RX_AMULEN); //duplex, crc, padding, multicast
  EthernetEnable(ETH_BASE);

  return;
}


void init_pins(void)
{
  //GPIO A: VS
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOSetInput(GPIO_PORTA_BASE, GPIO_PIN_1 | GPIO_PIN_4); //VS_SO, VS_DREQ = input
  GPIOSetOutput(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7); //VS_SCK, VS_DCS, VS_SI, VS_CS, VS_RST = output
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_3 | GPIO_PIN_6); //VS_SCK, VS_SI, VS_RST = low / VS_DCS, VS_CS = high
  GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5); //VS_SCK, VS_SO, VS_SI = ssi

  //GPIO B: SD, LCD
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOSetOutput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6); //SD_PWR, SD_CS, LCD_RS, LCD_RST, LCD_CS -> ouput
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_6); //SD_PWR, VS_RST = low / SD_CS, LCD_RS, LCD_CS = high

  //GPIO C: JTAG
#if defined(LM3S_REV_A1) || defined(LM3S_REV_A2) //for rev A1 & A2 set JTAG pullups on
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); //pullup
#endif

  //GPIO D: Encoder, IR
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  GPIOSetInput(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_2); //PHA, IR = input
#if defined(DEBUG)
  GPIOSetOutput(GPIO_PORTD_BASE, GPIO_PIN_3); //debug = output
  GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_3);
#endif

  //GPIO E: LCD, SD, F-RAM, Switch
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  GPIOSetInput(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_4); //SO, SW = input
  GPIOSetOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_7); //SCK, FM_CS, SI, LCD_PWM = output
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_7, GPIO_PIN_1); //SCK, SI, LCD_PWM = low / FM_CS = high
  GPIOPinTypeSSI(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3); //SCK, SO, SI = ssi

  //GPIO F: Encoder, USB-Power, Ethernet
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOSetInput(GPIO_PORTF_BASE, GPIO_PIN_0); //PHA = input
  GPIOSetOutput(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3); //LED1, LED2 = output
  GPIOSetOutputOD(GPIO_PORTF_BASE, GPIO_PIN_1); //USB_PWR = open-drain output
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); //USB_PWR = high
  GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW); //LED1, LED2 = ethernet

  return;
}


void GPIOSetOutputOD(unsigned long port, unsigned char pins)
{
  GPIOPinTypeGPIOOutputOD(port, pins); //open-drain output
  GPIOPadConfigSet(port, pins, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD); //open-drain

  return;
}


void GPIOSetOutput(unsigned long port, unsigned char pins)
{
  GPIOPinTypeGPIOOutput(port, pins); //output
  GPIOPadConfigSet(port, pins, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); //push-pull

  return;
}


void GPIOSetInput(unsigned long port, unsigned char pins)
{
  GPIOPinTypeGPIOInput(port, pins); //input
  GPIOPadConfigSet(port, pins, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); //pullup

  return;
}
