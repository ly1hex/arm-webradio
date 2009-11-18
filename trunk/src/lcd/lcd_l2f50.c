#include <stdint.h>
#include <stdlib.h>
#include "../third_party/lmi/inc/hw_types.h"
#include "../third_party/lmi/inc/hw_memmap.h"
#include "../third_party/lmi/driverlib/gpio.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_l2f50.h"


#ifdef L2F50


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
  lcd_cmd(0x5C);
  LCD_RS_DISABLE(); //data
  LCD_CS_ENABLE();

  return;
}


void lcd_area(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
  //set area
#ifdef LCD_MIRROR
  lcd_cmd(0x15);     //column address set 
  lcd_data(0x08+x0); //start column
  lcd_data(0x01);    //start column
  lcd_data(0x08+x1); //end column
  lcd_data(0x01);    //end column

  lcd_cmd(0x75); //page address set 
  lcd_data(y0);  //start page
  lcd_data(y1);  //end page
#else
  lcd_cmd(0x15);     //column address set 
  lcd_data(0x08+y0); //start column
  lcd_data(0x01);    //start column
  lcd_data(0x08+y1); //end column
  lcd_data(0x01);    //end column

  lcd_cmd(0x75);               //page address set 
  lcd_data((LCD_WIDTH-1)-x1);  //start page
  lcd_data((LCD_WIDTH-1)-x0);  //end page
#endif

  //set cursor
  lcd_cursor(x0, y0);

  return;
}


void lcd_cursor(unsigned int x, unsigned int y)
{
  return;
}


void lcd_cmd(unsigned int c)
{
  LCD_RS_ENABLE(); //cmd
  LCD_CS_ENABLE();
  ssi_write(c);
  ssi_write(0x00);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_data(unsigned int c)
{
  LCD_RS_DISABLE(); //data
  LCD_CS_ENABLE();
  ssi_write(c);
  ssi_write(0x00);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_reset(void)
{
  //reset
  LCD_CS_DISABLE();
  LCD_RS_DISABLE();
  LCD_RST_ENABLE();
  delay_ms(50);
  LCD_RST_DISABLE();
  delay_ms(100);

  lcd_cmd(0xBC);  //data control
  lcd_data(0x2A); //565 mode

  lcd_cmd(0xCA);  //display control 
  lcd_data(0x4C); //P1
  lcd_data(0x01); //P2
  lcd_data(0x53); //P3
  lcd_data(0x00); //P4
  lcd_data(0x02); //P5
  lcd_data(0xB4); //P6
  lcd_data(0xB0); //P7
  lcd_data(0x02); //P8
  lcd_data(0x00); //P9
  lcd_data(0x00); //P10
  lcd_data(0x00); //P11

  lcd_cmd(0x94); //sleep out

  delay_ms(2);

  lcd_cmd(0xAF); //display on

  lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


#endif //L2F50
