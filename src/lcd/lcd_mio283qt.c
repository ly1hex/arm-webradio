#include <stdint.h>
#include <stdlib.h>
#include "../third_party/lmi/inc/hw_types.h"
#include "../third_party/lmi/inc/hw_memmap.h"
#include "../third_party/lmi/driverlib/gpio.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_mio283qt.h"


#if defined(MIO283QT)


void lcd_draw(unsigned int color)
{
  ssi_write(color>>8);
  ssi_write(color);

  return;
}


void lcd_drawstop(void)
{
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_drawstart(void)
{
  LCD_CS_ENABLE();
  ssi_write(LCD_REGISTER);
  ssi_write(0x00);
  ssi_write(0x22);
  ssi_wait();
  LCD_CS_DISABLE();

  LCD_CS_ENABLE();
  ssi_write(LCD_DATA);

  return;
}


void lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y)
{
#if defined(LCD_MIRROR)
  lcd_cmd(0x50, (LCD_HEIGHT-1)-end_y);   //set y
  lcd_cmd(0x51, (LCD_HEIGHT-1)-start_y); //set y
  lcd_cmd(0x52, (LCD_WIDTH-1)-end_x);    //set x
  lcd_cmd(0x53, (LCD_WIDTH-1)-start_x);  //set x
#else
  lcd_cmd(0x50, start_y); //set y
  lcd_cmd(0x51, end_y);   //set y
  lcd_cmd(0x52, start_x); //set x
  lcd_cmd(0x53, end_x);   //set x
#endif

  //set cursor
  lcd_cursor(start_x, start_y);

  return;
}


void lcd_cursor(unsigned int x, unsigned int y)
{
#if defined(LCD_MIRROR)
  lcd_cmd(0x20, (LCD_HEIGHT-1)-y); //set y
  lcd_cmd(0x21, (LCD_WIDTH-1)-x);  //set x
#else
  lcd_cmd(0x20, y); //set y
  lcd_cmd(0x21, x); //set x
#endif

  return;
}


void lcd_cmd(unsigned int reg, unsigned int param)
{
  LCD_CS_ENABLE();
  ssi_write(LCD_REGISTER);
  ssi_write(reg>>8);
  ssi_write(reg);
  ssi_wait();
  LCD_CS_DISABLE();

  lcd_data(param);

  return;
}


void lcd_data(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(LCD_DATA);
  ssi_write(c>>8);
  ssi_write(c);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_reset(void)
{
  //reset
  LCD_CS_DISABLE();
  LCD_RST_ENABLE();
  delay_ms(100);
  LCD_RST_DISABLE();
  delay_ms(100);

  //display power control
  lcd_cmd(0x00, 0x0000);
  lcd_cmd(0x07, 0x0001);
  delay_ms(5);
  lcd_cmd(0x07, 0x0101);
  lcd_cmd(0x17, 0x0001); //PSE=1
  delay_ms(5);
  lcd_cmd(0x19, 0x0000);
  delay_ms(5);
  lcd_cmd(0x10, 0x10B0);
  lcd_cmd(0x11, 0x0007);
  lcd_cmd(0x12, 0x011B); //PON=1
  lcd_cmd(0x13, 0x0B00);
  //lcd_cmd(0x29, 0x0012); //VCM1
  //lcd_cmd(0x2A, 0x0095); //VCM2
  delay_ms(5);
  lcd_cmd(0x12, 0x013B); //PSON=1
  delay_ms(5);
  //lcd_cmd(0x01, 0x0100); //mirror
  lcd_cmd(0x60, 0x2700);
  lcd_cmd(0x61, 0x0001);

  //display on
  lcd_cmd(0x07, 0x0021);
  delay_ms(5);
  lcd_cmd(0x07, 000061);
  delay_ms(5);
  lcd_cmd(0x07, 0x0173);
  delay_ms(5);

  //display options
#if defined(LCD_MIRROR)
  lcd_cmd(0x03, 0x1008); //entry mode
#else
  lcd_cmd(0x03, 0x1038); //entry mode
#endif

  lcd_area(0x00, 0x00, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


#endif //MIO283QT
