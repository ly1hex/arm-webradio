/*---------------------------------------------------
 * Loader (c) Andreas Watterott (www.watterott.net)
 *---------------------------------------------------
 * For more information, see readme.txt
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/inc/hw_ints.h"
#include "lmi/driverlib/sysctl.h"
#include "lmi/driverlib/gpio.h"
#include "lmi/driverlib/interrupt.h"
#include "lmi/driverlib/systick.h"
#include "lmi/driverlib/uart.h"
#include "lmi/driverlib/flash.h"
#include "fatfs/ff.h"
#include "main.h"
#include "../tools.h"
#include "../io.h"
#include "../lcd.h"
#include "../lcd/font_8x12.h"
#include "../mmc_io.h"
#include "../mmc.h"


#define ITEMS                          (4)
#define ITEM_X                         (15)
#define ITEM_Y                         (36)
#define ITEM_HEIGHT                    (20)


volatile unsigned int ms_time=0;
FIL fileobj;
unsigned char flashbuf[FLASHBUF];


#ifdef DEBUG
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
  UARTCharPut(DEBUGUART, c);

  return;
}


#ifdef DEBUG
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
  static unsigned long mmc=1;

  //1000 Hz
  ms_time++;

  //100 Hz
  if(--mmc == 0)
  {
    mmc = 10;
    disk_timerproc();
    keys_timerservice();
  }

  return;
}


void delay_ms(unsigned int ms)
{
  ms += ms_time;

  while(ms_time != ms);

  return;
}


long backup_app(const char* fname)
{
  unsigned long i;
  unsigned int wr;
  long err=1;

  lcd_putline(ITEM_X, ITEM_Y, "Open File...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

  FlashUsecSet(8); //cpu speed = 8 MHz

  if(f_open(&fileobj, fname, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
  {
    err = 0;
    lcd_putline(ITEM_X, ITEM_Y, "Save Flash...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    for(i=APPSTARTADDR; i<FLASHSIZE; i+=1024)
    {
      if(f_write(&fileobj, (unsigned char*)i, FLASHBUF, &wr) != FR_OK)
      {
        err = 2;
        break;
      }
    }
    f_close(&fileobj);
  }
  else
  {
    lcd_putline(ITEM_X, ITEM_Y, "ERROR", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(1000);
  }

  return err;
}


long flash_app(const char* fname)
{
  unsigned long i;
  unsigned int rd;
  long err=1;

  lcd_putline(ITEM_X, ITEM_Y, "Open File...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

  FlashUsecSet(8); //cpu speed = 8 MHz

  if(f_open(&fileobj, fname, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    err = 0;
    if(err == 0)
    {
      lcd_putline(ITEM_X, ITEM_Y, "Erase Flash...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
      for(i=APPSTARTADDR; i<FLASHSIZE; i+=1024)
      {
        if(FlashErase(i) != 0)
        {
          err = 2;
          break;
        }
      }
    }

    if(err == 0)
    {
      lcd_putline(ITEM_X, ITEM_Y, "Flash App...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
      for(i=APPSTARTADDR; i<=FLASHSIZE;)
      {
        if(f_read(&fileobj, flashbuf, FLASHBUF, &rd) == FR_OK)
        {
          if(rd < FLASHBUF)
          {
            memset((flashbuf+(rd-1)), 0xff, (FLASHBUF-rd));
            rd = 0;
          }
          if(FlashProgram((unsigned long*)flashbuf, i, FLASHBUF) != 0)
          {
            err = 3;
            break;
          }
          i += FLASHBUF;
          if(rd == 0)
          {
            break;
          }
        }
        else
        {
          break;
        }
      }
    }
    f_close(&fileobj);
  }
  else
  {
    lcd_putline(ITEM_X, ITEM_Y, "ERROR", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(1000);
  }

  return err;
}


void start_app(void)
{
  DEBUGOUT("Start App...\n");

  //__asm("cpsid   i\n");  //disable interrupts

  SysTickDisable();
  SysTickIntDisable();
  SysTickIntUnregister();

  __asm("ldr     r0, =%0\n"         //load application start address

        "ldr     r1, =0xe000ed08\n" //set vector table address to the beginning of the application
        "str     r0, [r1]\n"

        "ldr     r1, [r0]\n"        //load stack pointer from the application's vector table
        "mov     sp, r1\n"

        "ldr     r0, [r0, #4]\n"    //load the initial PC from the application's vector table and
        "bx      r0\n"              //branch to the application's entry point
        :
        : "i" (APPSTARTADDR));

  return;
}


int main()
{
  int i, item;

  //init brown-out reset
  init_bor(1);

  //init pins and peripherals
  init_pins();

  //init fault ints
#ifdef DEBUG
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

  i = ENC_SW_READ(); //read switch state

  //low speed and enable interrupts
  cpu_speed(1);

  DEBUGOUT("\n"APPNAME" v"APPVERSION"\n");

#ifdef DEBUG
  if(1)
#else
  if(!ENC_SW_READ() && !i) //read switch state twice
#endif
  {
    //init lcd
    DEBUGOUT("Init LCD...\n");
    lcd_init();
    lcd_clear(RGB(0,0,0));
    lcd_fillrect( 0, 0, LCD_WIDTH-1, 10, RGB(180,150,0));
    lcd_puts(38,  2, APPNAME" v"APPVERSION, SMALLFONT, 1, RGB(0,0,0), RGB(180,150,0));
    lcd_fillrect( 0, LCD_HEIGHT-1-10, LCD_WIDTH-1, LCD_HEIGHT-1, RGB(180,150,0));
    lcd_puts(20, LCD_HEIGHT-1-8, "www.watterott.net", SMALLFONT, 1, RGB(0,0,0), RGB(180,150,0));
    lcd_puts(10, 18, "HW:"LM3S_NAME","LCD_NAME, SMALLFONT, 1, RGB(140,140,140), RGB(0,0,0));
    lcd_putline(ITEM_X, ITEM_Y, "Start Loader...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

    //init mmc & mount filesystem
    DEBUGOUT("Init Memory Card...\n");
    lcd_putline(ITEM_X, ITEM_Y, "Init Memory Card...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    fs_mount();

    //menu
    lcd_fillrect(ITEM_X-10, ITEM_Y, LCD_WIDTH-1, LCD_HEIGHT-1-11, RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*0), "Start Application", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*1), "Flash "FIRMWARE_FILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*2), "Flash "FIRMWARE_BAKFILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*3), "Backup Firmware to   "FIRMWARE_BAKFILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(200);
    keys_sw(); //clear keys
    item = 0;
    for(i=-1; keys_sw()==0;)
    {
      if(i)
      {
             if((i > 0) && (item < (ITEMS-1))) { item++; }
        else if((i < 0) && (item > 0))         { item--; }
        lcd_fillrect(ITEM_X-10, ITEM_Y, ITEM_X, ITEM_Y+(ITEMS*ITEM_HEIGHT), RGB(0,0,0));
        lcd_putc(ITEM_X-10, ITEM_Y+(item*ITEM_HEIGHT)-1, 0xFC, SMALLFONT, 1, RGB(255,160,0), RGB(0,0,0)); //play icon
      }
      i = keys_steps();
    }
    lcd_fillrect(ITEM_X-10, ITEM_Y, LCD_WIDTH-1, LCD_HEIGHT-1-11, RGB(0,0,0));

    switch(item)
    {
      case 1: flash_app(FIRMWARE_FILE);     break;
      case 2: flash_app(FIRMWARE_BAKFILE);  break;
      case 3: backup_app(FIRMWARE_BAKFILE); break;
    }

    //unmount file system
    fs_unmount();

    lcd_putline(ITEM_X, ITEM_Y, "Start App...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
  }

  //start application
  start_app();

  DEBUGOUT("ERROR\n");

  while(1);
}
