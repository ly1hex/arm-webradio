/*---------------------------------------------------
 * WebRadio (c) Andreas Watterott (www.watterott.net)
 *---------------------------------------------------
 * For more information, see readme.txt
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "third_party/lmi/inc/hw_types.h"
#include "third_party/lmi/inc/hw_memmap.h"
#include "third_party/lmi/inc/hw_ints.h"
#include "third_party/lmi/driverlib/sysctl.h"
#include "third_party/lmi/driverlib/gpio.h"
#include "third_party/lmi/driverlib/interrupt.h"
#include "third_party/lmi/driverlib/systick.h"
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "mmc.h"
#include "vs.h"
#include "eth.h"
#include "menu.h"
#include "alarm.h"


volatile unsigned int status=0;
volatile long on_time=0;
volatile unsigned int ms_time=0;
unsigned long sec_time=0;
TIME time;
char date_str[14] = {'T','h',' ','0','1','.','0','1','.','1','9','7','0',0}; //Th 01.01.1970
char clock_str[9] = {'0','0',':','0','0',':','0','0',0}; //00:00:00

const char day_tab[7][3] = 
{
  {"Su"},{"Mo"},{"Tu"},{"We"},{"Th"},{"Fr"},{"Sa"} //English
  //{"So"},{"Mo"},{"Di"},{"Mi"},{"Do"},{"Fr"},{"Sa"} //German
};

const char clock_tab[60][3] = 
{
  {"00"},{"01"},{"02"},{"03"},{"04"},{"05"},{"06"},{"07"},{"08"},{"09"},
  {"10"},{"11"},{"12"},{"13"},{"14"},{"15"},{"16"},{"17"},{"18"},{"19"},
  {"20"},{"21"},{"22"},{"23"},{"24"},{"25"},{"26"},{"27"},{"28"},{"29"},
  {"30"},{"31"},{"32"},{"33"},{"34"},{"35"},{"36"},{"37"},{"38"},{"39"},
  {"40"},{"41"},{"42"},{"43"},{"44"},{"45"},{"46"},{"47"},{"48"},{"49"},
  {"50"},{"51"},{"52"},{"53"},{"54"},{"55"},{"56"},{"57"},{"58"},{"59"}
};


#if defined(DEBUG)
void __error__(char *pcFilename, unsigned long ulLine) //called if the DriverLib encounters an error
{
  DEBUGOUT("DLib: %s:%i\n", pcFilename, ulLine);

  return;
}
#endif


void debugout(const char *s, ...)
{
  unsigned int i, move;
  char c, str[16], *ptr;
  va_list ap;

  va_start(ap, s);

  for(;;)
  {
    c = *s++;

    if(c == 0)
    {
      break;
    }
    else if(c == '%')
    {
      c = *s++;
      if(isdigit(c) > 0)
      {
        move = c-'0';
        c = *s++;
      }
      else
      {
        move = 0;
      }

      switch(c)
      {
        case 's':
          ptr = va_arg(ap, char *);
          uart_puts(ptr);
          break;
        case 'b': //bin
          itoa(va_arg(ap, long), str, 2);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'i': //dec
          itoa(va_arg(ap, long), str, 10);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'u': //unsigned dec
          uitoa(va_arg(ap, unsigned long), str);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'x': //hex
          itoa(va_arg(ap, long), str, 16);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
      }
    }
    else
    {
      uart_putc(c);
    }
  }

  va_end(ap);

  return;
}


void uart_puts(const char *s)
{
  while(*s)
  {
    uart_putc(*s++);
  }

  return;
}


void uart_putc(unsigned int c)
{
  UARTCharPut(UART1_BASE, c);

  return;
}


#if defined(DEBUG)
void nmi_fault(void)   { DEBUGOUT("NMI fault\n");    return; }
void hard_fault(void)  { DEBUGOUT("HARD fault\n");   return; }
void mpu_fault(void)   { DEBUGOUT("MPU fault\n");    return; }
void bus_fault(void)   { DEBUGOUT("BUS fault\n");    return; }
void usage_fault(void) { DEBUGOUT("USAGE fault\n");  return; }
void svcall_fault(void){ DEBUGOUT("SVCALL fault\n"); return; }
void debug_fault(void) { DEBUGOUT("DEBUG fault\n");  return; }
void pendsv_fault(void){ DEBUGOUT("PENDSV fault\n"); return; }
#endif


void systick(void) //1000 Hz
{
  static unsigned long mmc=1, sec=1;
  unsigned int s;

  //1000 Hz
  ms_time++;

  //100 Hz
  if(--mmc == 0)
  {
    mmc = 10;
    disk_timerproc();
    keys_timerservice();
  }

  //1 Hz
  if(--sec == 0)
  {
    sec = 1000;
    on_time++;
    sec_time++;

    s  = status;
    s |= SEC_CHANGED;
    if(++time.s == 60)
    {
      time.s = 0;
      s |= MIN_CHANGED;
      if(++time.m == 60)
      {
        time.m = 0;
        if(++time.h == 24)
        {
          time.h = 0;
          s |= DAY_CHANGED;
        }
        clock_str[0] = clock_tab[time.h][0];
        clock_str[1] = clock_tab[time.h][1];
      }
      clock_str[3] = clock_tab[time.m][0];
      clock_str[4] = clock_tab[time.m][1];
    }
    clock_str[6] = clock_tab[time.s][0];
    clock_str[7] = clock_tab[time.s][1];
    status = s;
  }

  return;
}


char* getclock(void)
{
  return clock_str;
}


char* getdate(void)
{
  return date_str;
}


void settime(unsigned long s)
{
  TIME t;

  sectotime(s, &t);

  SysTickIntDisable();
  time.year  = t.year;
  time.month = t.month;
  time.day   = t.day;
  time.wday  = t.wday;
  time.h     = t.h;
  time.m     = t.m;
  time.s     = t.s;
  sec_time   = s;
  SysTickIntEnable();

  date_str[0] = day_tab[time.wday][0];
  date_str[1] = day_tab[time.wday][1];
  date_str[3] = clock_tab[time.day][0];
  date_str[4] = clock_tab[time.day][1];
  date_str[6] = clock_tab[time.month][0];
  date_str[7] = clock_tab[time.month][1];
  itoa(time.year, &date_str[9], 10);

  clock_str[0] = clock_tab[time.h][0];
  clock_str[1] = clock_tab[time.h][1];
  clock_str[3] = clock_tab[time.m][0];
  clock_str[4] = clock_tab[time.m][1];
  clock_str[6] = clock_tab[time.s][0];
  clock_str[7] = clock_tab[time.s][1];

  DEBUGOUT("Set time: %s %s\n", getclock(), getdate());

  return;
}


long getdeltatime(long time)
{
  long s;

  s = on_time;

  return (s-time);
}


long getontime(void)
{
  long s;

  s = on_time;

  return s;
}


void delay_ms(unsigned int ms)
{
  ms += ms_time;

  while(ms_time != ms);

  return;
}


unsigned int getmstime(void)
{
  return ms_time;
}


unsigned int standby(unsigned int param)
{
  unsigned int i, alarm=0;
  char tmp[6];

  DEBUGOUT("Standby\n");

  vs_stop();
  fs_unmount();
  lcd_clear(RGB(0,0,0));
  tmp[0] = clock_str[0];
  tmp[1] = clock_str[1];
  tmp[2] = clock_str[2];
  tmp[3] = clock_str[3];
  tmp[4] = clock_str[4];
  tmp[5] = 0;
  lcd_puts(50, 50, tmp, TIMEFONT, RGB(255,255,255), RGB(0,0,0));

  cpu_speed(1); //low speed

  for(;;)
  {
    eth_service();

    if(!ethernet_data())
    {
      i = status;
      status &= ~i;
      if(i & SEC_CHANGED)
      {
        if(i & MIN_CHANGED)
        {
          if(i & DAY_CHANGED)
          {
            settime(sec_time);
          }
          if(alarm_check(&time))
          {
            alarm = 1;
          }
          tmp[0] = clock_str[0];
          tmp[1] = clock_str[1];
          tmp[3] = clock_str[3];
          tmp[4] = clock_str[4];
          lcd_puts(50, 50, tmp, TIMEFONT, RGB(255,255,255), RGB(0,0,0));
        }
      }
    }

    if(alarm)
    {
      break;
    }
    else if(keys_sw() || (ir_cmd() == SW_POWER))
    {
      if((time.h >= 19) || (time.h <= 4))
      {
        lcd_puts(20, 20, "Good Night !", NORMALFONT, RGB(255,255,255), RGB(0,0,0));
      }
      else if(time.h >= 17)
      {
        lcd_puts(20, 20, "Good Evening !", NORMALFONT, RGB(255,255,255), RGB(0,0,0));
      }
      else if(time.h >= 12)
      {
        lcd_puts(20, 20, "Good Afternoon !", NORMALFONT, RGB(255,255,255), RGB(0,0,0));
      }
      else if(time.h >= 10)
      {
        lcd_puts(20, 20, "Good Day !", NORMALFONT, RGB(255,255,255), RGB(0,0,0));
      }
      else if(time.h >= 5)
      {
        lcd_puts(20, 20, "Good Morning !", NORMALFONT, RGB(255,255,255), RGB(0,0,0));
      }
      delay_ms(1000);
      break;
    }
  }

  fs_mount();

  cpu_speed(0); //high speed

  //clear cmds
  keys_sw();
  keys_steps();
  ir_cmd();

  if(alarm)
  {
    menu_alarm();
  }

  return 0;
}


int main()
{
  unsigned int i, draw=0;
  unsigned long r;

  //get reset cause
  r = SysCtlResetCauseGet();
  if(r)
  {
    SysCtlResetCauseClear(r);
  }

  //init pins and peripherals
  init_pins();

  //init brown-out reset
  init_bor(1);

  //init fault ints
#if defined(DEBUG)
  IntRegister(FAULT_NMI,    nmi_fault);    IntEnable(FAULT_NMI);
  IntRegister(FAULT_HARD,   hard_fault);   IntEnable(FAULT_HARD);
  IntRegister(FAULT_MPU,    mpu_fault);    IntEnable(FAULT_MPU);
  IntRegister(FAULT_BUS,    bus_fault);    IntEnable(FAULT_BUS);
  IntRegister(FAULT_USAGE,  usage_fault);  IntEnable(FAULT_USAGE);
  IntRegister(FAULT_SVCALL, svcall_fault); IntEnable(FAULT_SVCALL);
  IntRegister(FAULT_DEBUG,  debug_fault);  IntEnable(FAULT_DEBUG);
  IntRegister(FAULT_PENDSV, pendsv_fault); IntEnable(FAULT_PENDSV);
#endif

  //init systick
  SysTickDisable();
  SysTickPeriodSet(SysCtlClockGet() / 1000); //1 kHz
  SysTickIntRegister(systick);
  SysTickIntEnable();
  SysTickEnable();

  //init peripherals
  init_periph();

  //low speed
  cpu_speed(1);

  //enable interrupts
  IntMasterEnable();

  //show hardware config
  #if defined(LM3S_REV_A1)             //LM3S Rev
  # define LM3S_NAME "LM3S-A1"
  #elif defined(LM3S_REV_A2)
  # define LM3S_NAME "LM3S-A2"
  #elif defined(LM3S_REV_B0)
  # define LM3S_NAME "LM3S-B0"
  #else
  # warning "LM3S Rev not defined"
  #endif
  #if defined(LPH88)                   //LCD
  # define LCD_NAME "S65-LPH88"
  #elif defined(LS020)
  # define LCD_NAME "S65-LS020"
  #elif defined(L2F50)
  # define LCD_NAME "S65-L2F50"
  #else
  # warning "LCD not defined"
  #endif
  DEBUGOUT("\n---\n");
  DEBUGOUT(APPNAME" v"APPVERSION""APPRELEASE_SYM" ("__DATE__" "__TIME__")\n");
  DEBUGOUT("Hardware: "LM3S_NAME", "LCD_NAME"\n");

  //init lcd
  lcd_init();

  //show start-up screen
  lcd_clear(DEFAULT_BGCOLOR);
  lcd_rect( 0,   0, LCD_WIDTH-1, 13, DEFAULT_EDGECOLOR);
  lcd_puts(30,   3, APPNAME" v"APPVERSION""APPRELEASE_SYM, SMALLFONT, DEFAULT_BGCOLOR, DEFAULT_EDGECOLOR);
  lcd_rect( 0, 118, LCD_WIDTH-1, LCD_HEIGHT-1, DEFAULT_EDGECOLOR);
  lcd_puts(20, 121, "www.watterott.net", SMALLFONT, DEFAULT_BGCOLOR, DEFAULT_EDGECOLOR);
  lcd_puts(10,  20, "Hardware:", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  lcd_puts(15,  30, LM3S_NAME", "LCD_NAME, SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  if(r)
  {
    i = lcd_puts(10,  45, "Reset:", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    if(r & SYSCTL_CAUSE_LDO)
    {
      i = lcd_puts(i, 45, "LDO", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
    if(r & SYSCTL_CAUSE_SW)
    {
      i = lcd_puts(i, 45, "SW", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
    if(r & SYSCTL_CAUSE_WDOG)
    {
      i = lcd_puts(i, 45, "WD", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
    if(r & SYSCTL_CAUSE_BOR)
    {
      i = lcd_puts(i, 45, "BOR", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
    if(r & SYSCTL_CAUSE_POR)
    {
      i = lcd_puts(i, 45, "POR", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
    if(r & SYSCTL_CAUSE_EXT)
    {
      i = lcd_puts(i, 45, "EXT", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR) + 4;
    }
  }

  i = 60;

  //init mmc & mount filesystem
  lcd_puts(10,  i, "Init Memory Card...", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  fs_mount();

  //init ethernet
  lcd_puts(10,  i, "Init Ethernet...", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  eth_init();

  //load settings
  lcd_puts(10, i, "Load Settings...", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  settings_read();

  //set clock
  lcd_puts(10, i, "NTP: Get Time...", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
#if defined(DEBUG)
  settime(0);
#else
  settime(ntp_gettime());
#endif

  //advertise UPnP device
  lcd_puts(10, 100, "SSDP: Advertise...", SMALLFONT, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  ssdp_advertise();

  //cpu high speed
  cpu_speed(0);

  //init menu
  menu_init();

  DEBUGOUT("Ready...\n");

  for(;;)
  {
    eth_service();

    if(!ethernet_data())
    {
      i = status;
      status &= ~i;
      if(i & SEC_CHANGED)
      {
        draw = SEC_CHANGED;
        if(i & MIN_CHANGED)
        {
          if(i & DAY_CHANGED)
          {
            draw |= DAY_CHANGED;
            settime(sec_time);
          }
          if(alarm_check(&time))
          {
            DEBUGOUT("Ready...\n");
            menu_alarm();
          }
        }
      }
    }
    menu_service(draw);
    draw = 0;
  }
}
