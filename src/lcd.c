#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "third_party/lmi/inc/hw_types.h"
#include "third_party/lmi/inc/hw_memmap.h"
#include "third_party/lmi/driverlib/gpio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "lcd/lcd_l2f50.h"
#include "lcd/lcd_ls020.h"
#include "lcd/lcd_lph88.h"
#include "lcd/font_8x8.h"
#include "lcd/font_8x12.h"
#include "lcd/font_clock.h"


void lcd_img32(int x, unsigned int y, const unsigned char *img, unsigned int color, unsigned int bg_color)
{
  int i, start_x, start_y, end_x, end_y;
  unsigned long *ptr, data, mask, start_bit, end_bit;

  if((x <= -32) || (x >= LCD_WIDTH))
  {
    return;
  }

  if(x < 0)
  {
    start_x   = 0;
    start_y   = y;
    end_x     = 31+x;
    end_y     = y+31;
    start_bit = 1UL<<(31+x);
    end_bit   = 0UL;
  }
  else if(x > (LCD_WIDTH-32))
  {
    start_x   = x;
    start_y   = y;
    end_x     = LCD_WIDTH-1;
    end_y     = y+31;
    start_bit = 1UL<<31;
    end_bit   = 1UL<<(30-(end_x-start_x));
  }
  else
  {
    start_x   = x;
    start_y   = y;
    end_x     = x+31;
    end_y     = y+31;
    start_bit = 1UL<<31;
    end_bit   = 0UL;
  }

  lcd_area(start_x, start_y, end_x, end_y);

  lcd_drawstart();
  ptr = (unsigned long*)img;
  for(i=32; i!=0; i--)
  {
    data = *ptr++;

data = ((data&0xFF000000UL)>>24)|((data&0x00FF0000UL)>>8)|((data&0x0000FF00UL)<<8)|((data&0x000000FFUL)<<24); //swap32

    for(mask=start_bit; mask>end_bit; mask>>=1)
    {
      if(data & mask)
      {
        lcd_draw(color);
      }
      else
      {
        lcd_draw(bg_color);
      }
    }
  }
  lcd_drawstop();

  return;
}


void lcd_putline(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bg_color)
{
  unsigned int last_x=x;

  while(*s)
  {
    x = lcd_putc(x, y, *s++, font, color, bg_color);
    if(x >= LCD_WIDTH)
    {
      break;
    }
    last_x = x;
  }

  switch(font)
  {
    case 1: font=FONT1_HEIGHT; break;
    case 2: font=FONT2_HEIGHT; break;
    case 3: font=FONT3_HEIGHT; break;
  }

  lcd_rect(last_x, y, (LCD_WIDTH-1), (y+font)-1, bg_color);

  return;
}


unsigned int lcd_puts(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bg_color)
{
  while(*s)
  {
    x = lcd_putc(x, y, *s++, font, color, bg_color);
    if(x >= LCD_WIDTH)
    {
      break;
    }
  }

  return x;
}


unsigned int lcd_putc(unsigned int x, unsigned int y, unsigned int c, unsigned int font, unsigned int color, unsigned int bg_color)
{
  unsigned int ret, width, height, size;
  unsigned long *ptr, data, mask;

  switch(font)
  {
    case 0:
      c     -= FONT1_START;
      ptr    = (unsigned long*)&font1[c*(FONT1_WIDTH*FONT1_HEIGHT/8)];
      width  = FONT1_WIDTH;
      height = FONT1_HEIGHT;
      break;
    case 1:
      c     -= FONT2_START;
      ptr    = (unsigned long*)&font2[c*(FONT2_WIDTH*FONT2_HEIGHT/8)];
      width  = FONT2_WIDTH;
      height = FONT2_HEIGHT;
      break;
    case 2:
      c     -= FONT3_START;
      ptr    = (unsigned long*)&font3[c*(FONT3_WIDTH*FONT3_HEIGHT/8)];
      width  = FONT3_WIDTH;
      height = FONT3_HEIGHT;
      break;
  }

  ret = x+width;
  if(ret >= LCD_WIDTH)
  {
    return LCD_WIDTH;
  }

  lcd_area(x, y, (x+width-1), (y+height-1));
  lcd_drawstart();
  for(size=(width*height)/32; size!=0; size--)
  {
    data = *ptr++;

data = ((data&0xFF000000UL)>>24)|((data&0x00FF0000UL)>>8)|((data&0x0000FF00UL)<<8)|((data&0x000000FFUL)<<24); //swap32

    for(mask=0x80000000UL; mask!=0UL; mask>>=1)
    {
      if(data & mask)
      {
        lcd_draw(color);
      }
      else
      {
        lcd_draw(bg_color);
      }
    }
  }
  lcd_drawstop();

  return ret;
}


void lcd_rect(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color)
{
  unsigned int size;

  if((start_x > end_x) ||
     (start_y > end_y))
  {
    return;
  }

  lcd_area(start_x, start_y, end_x, end_y);
  lcd_drawstart();
  for(size=((1+(end_x-start_x))*(1+(end_y-start_y))); size!=0; size--)
  {
    lcd_draw(color);
  }
  lcd_drawstop();

  return;
}


void lcd_rectedge(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color)
{
  lcd_rect(start_x, start_y, start_x, end_y, color);
  lcd_rect(start_x, end_y, end_x, end_y, color);
  lcd_rect(end_x, start_y, end_x, end_y, color);
  lcd_rect(start_x, start_y, end_x, start_y, color);

  return;
}


void lcd_line(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color)
{
  unsigned int x, y, dx, dy, xinc, xinc_e, yinc, yinc_e, eadd, esub, steps;
  int e;

  if((start_x == end_x) ||
     (start_y == end_y)) //horizontal or vertical line
  {
    lcd_rect(start_x, start_y, end_x, end_y, color);
  }
  else
  {
    if(start_x > end_x) //swap coordinates
    {
      x       = start_x;
      start_x = end_x;
      end_x   = x;
      y       = start_y;
      start_y = end_y;
      end_y   = y;
    }

    if(start_y < end_y) // "\"
    {
      x   = start_x;
      y   = start_y;
      dx  = end_x - start_x;
      dy  = end_y - start_y;
      xinc = +1;
      yinc = +1;
    }
    else                // "/"
    {
      x   = start_x;
      y   = end_y;
      dx  = end_x   - start_x;
      dy  = start_y - end_y;
      xinc = +1;
      yinc = -1;
    }

    if(dx > dy)
    {
      eadd   = dx;
      esub   = dy;
      xinc_e = 0;
      yinc_e = yinc;
      yinc   = 0;
      steps  = dx;
    }
    else
    {
      eadd   = dy;
      esub   = dx;
      xinc_e = xinc;
      xinc   = 0;
      yinc_e = 0;
      steps  = dy;
    }

    lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));
    lcd_pixel(x, y, color);
    for(e=eadd/2; steps!=0; steps--)
    {
      x += xinc;
      y += yinc;
      e -= esub;
      if(e < 0)
      {
        x += xinc_e;
        y += yinc_e;
        e += eadd;
      }
      lcd_pixel(x, y, color);
    }
  }

  return;
}


void lcd_pixel(unsigned int x, unsigned int y, unsigned int color)
{
  if((x >= LCD_WIDTH) ||
     (y >= LCD_HEIGHT))
  {
    return;
  }

  lcd_cursor(x, y);
  lcd_drawstart();
  lcd_draw(color);
  lcd_drawstop();

  return;
}


void lcd_clear(unsigned int color)
{
  unsigned int i;

  lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));
  lcd_drawstart();
  for(i=(LCD_WIDTH*LCD_HEIGHT); i!=0; i--)
  {
    lcd_draw(color);
  }
  lcd_drawstop();

  return;
}


void lcd_init(void)
{
  pwm_led(50); //led backlight on

  ssi_speed(2000000); //2 MHz
  lcd_reset();
  lcd_reset();
  lcd_clear(RGB(0,0,0));
  ssi_speed(0); //ssi speed up (0 = default speed)

  return;
}
