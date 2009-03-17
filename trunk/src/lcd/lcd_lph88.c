#include <stdint.h>
#include "../third_party/lmi/inc/hw_types.h"
#include "../third_party/lmi/inc/hw_memmap.h"
#include "../third_party/lmi/driverlib/gpio.h"
#include "../main.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_lph88.h"


#if defined(LPH88)


inline void lcd_draw(unsigned int color)
{
  ssi_write(color>>8);
  ssi_write(color);

  return;
}


inline void lcd_drawstop(void)
{
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


inline void lcd_drawstart(void)
{
  lcd_reg(0x22);
  LCD_CS_ENABLE();
  ssi_write(0x76);

  return;
}


inline void lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y)
{
  //set area
#if defined(LCD_MIRROR)
  lcd_cmd(0x16, (((LCD_HEIGHT-1)-start_y)<<8)|(((LCD_HEIGHT-1)-end_y))); //set y
  lcd_cmd(0x17, (((LCD_WIDTH-1)-start_x)<<8)|(((LCD_WIDTH-1)-end_x))); //set x
#else
  lcd_cmd(0x16, (end_y<<8)|(start_y)); //set y
  lcd_cmd(0x17, (end_x<<8)|(start_x)); //set x
#endif

  //set cursor
  lcd_cursor(start_x, start_y);

  return;
}


inline void lcd_cursor(unsigned int x, unsigned int y)
{
#if defined(LCD_MIRROR)
  lcd_cmd(0x21, ((((LCD_WIDTH-1)-x)<<8)|((LCD_HEIGHT-1)-y))); //set cursor pos
#else
  lcd_cmd(0x21, ((x<<8)|y)); //set cursor pos
#endif

  return;
}


inline void lcd_cmd(unsigned int reg, unsigned int param)
{
  lcd_reg(reg);
  lcd_data(param);

  return;
}


inline void lcd_data(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(0x76); //instruction or RAM data
  ssi_write(c>>8);
  ssi_write(c);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


inline void lcd_reg(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(0x74); //index register
  ssi_write(0x00);
  ssi_write(c);
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
  delay_ms(100);
  LCD_RST_DISABLE();
  delay_ms(100);

  lcd_cmd(0x07, 0x0000); //display off
  delay_ms(10);

  //power on sequence
  lcd_cmd(0x02, 0x0400); //lcd drive control
  lcd_cmd(0x0C, 0x0001); //power control 3: VC        //step 1
  lcd_cmd(0x0D, 0x0006); //power control 4: VRH
  lcd_cmd(0x04, 0x0000); //power control 2: CAD
  lcd_cmd(0x0D, 0x0616); //power control 4: VRL
  lcd_cmd(0x0E, 0x0010); //power control 5: VCM
  lcd_cmd(0x0E, 0x1010); //power control 5: VDV
  lcd_cmd(0x03, 0x0000); //power control 1: BT        //step 2
  lcd_cmd(0x03, 0x0000); //power control 1: DC
  lcd_cmd(0x03, 0x000C); //power control 1: AP
  delay_ms(40);
  lcd_cmd(0x0E, 0x2D1F); //power control 5: VCOMG     //step 3
  delay_ms(40);
  lcd_cmd(0x0D, 0x0616); //power control 4: PON       //step 4
  delay_ms(100);

  //display options
#if defined(LCD_MIRROR)
  lcd_cmd(0x05, 0x0008); //Entry mode --
#else
  lcd_cmd(0x05, 0x0038); //Entry mode ++
#endif
  lcd_area(0x00, 0x00, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  //display on sequence (bit2 = reversed colors)
  lcd_cmd(0x07, 0x0005); //display control: D0
  lcd_cmd(0x07, 0x0025); //display control: GON
  lcd_cmd(0x07, 0x0027); //display control: D1
  lcd_cmd(0x07, 0x0037); //display control: DTE

  delay_ms(10);

  return;
}


#endif //LPH88
