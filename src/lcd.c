#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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


void lcd_img32(int x, unsigned int y, const unsigned char *img, unsigned int color, unsigned int bgcolor)
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
        lcd_draw(bgcolor);
      }
    }
  }
  lcd_drawstop();

  return;
}


void lcd_putlinebr(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor)
{
  unsigned int start_x=x, font_height;
  char c;

  switch(font)
  {
    case SMALLFONT:  font_height=SMALLFONT_HEIGHT-1;  break;
    case NORMALFONT: font_height=NORMALFONT_HEIGHT-1; break;
    case TIMEFONT:   font_height=TIMEFONT_HEIGHT-1;   break;
  }

  lcd_rect(0, y, x-1, (y+font_height), bgcolor); //clear before text

  while(*s)
  {
    c = *s++;
    if(c == '\n') //new line
    {
      lcd_rect(x, y, (LCD_WIDTH-1), (y+font_height), bgcolor); //clear after text
      x  = start_x;
      y += font_height+2;
      lcd_rect(0, y, x-1, (y+font_height), bgcolor); //clear before text
      continue;
    }

    x = lcd_putc(x, y, c, font, color, bgcolor);

    if(x >= LCD_WIDTH) //new line
    {
      lcd_rect(x, y, (LCD_WIDTH-1), (y+font_height), bgcolor); //clear after text
      x  = start_x;
      y += font_height+2;
      lcd_rect(0, y, x-1, (y+font_height), bgcolor); //clear before text
      x = lcd_putc(x, y, c, font, color, bgcolor);
    }
  }

  return;
}


void lcd_putline(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor)
{
  unsigned int last_x=x, font_height;

  switch(font)
  {
    case SMALLFONT:  font_height=SMALLFONT_HEIGHT-1;  break;
    case NORMALFONT: font_height=NORMALFONT_HEIGHT-1; break;
    case TIMEFONT:   font_height=TIMEFONT_HEIGHT-1;   break;
  }

  lcd_rect(0, y, x-1, (y+font_height), bgcolor); //clear before text

  while(*s)
  {
    x = lcd_putc(x, y, *s++, font, color, bgcolor);
    if(x >= LCD_WIDTH)
    {
      break;
    }
    last_x = x;
  }

  lcd_rect(last_x, y, (LCD_WIDTH-1), (y+font_height), bgcolor); //clear after text

  return;
}


unsigned int lcd_puts(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor)
{
  while(*s)
  {
    x = lcd_putc(x, y, *s++, font, color, bgcolor);
    if(x >= LCD_WIDTH)
    {
      break;
    }
  }

  return x;
}


unsigned int lcd_putc(unsigned int x, unsigned int y, unsigned int c, unsigned int font, unsigned int color, unsigned int bgcolor)
{
  unsigned int ret, width, height, size;
  unsigned long *ptr, data, mask;

  switch(font)
  {
    case SMALLFONT:
      c     -= SMALLFONT_START;
      ptr    = (unsigned long*)&SMALLFONT_NAME[c*(SMALLFONT_WIDTH*SMALLFONT_HEIGHT/8)];
      width  = SMALLFONT_WIDTH;
      height = SMALLFONT_HEIGHT;
      break;
    case NORMALFONT:
      c     -= NORMALFONT_START;
      ptr    = (unsigned long*)&NORMALFONT_NAME[c*(NORMALFONT_WIDTH*NORMALFONT_HEIGHT/8)];
      width  = NORMALFONT_WIDTH;
      height = NORMALFONT_HEIGHT;
      break;
    case TIMEFONT:
      c     -= TIMEFONT_START;
      ptr    = (unsigned long*)&TIMEFONT_NAME[c*(TIMEFONT_WIDTH*TIMEFONT_HEIGHT/8)];
      width  = TIMEFONT_WIDTH;
      height = TIMEFONT_HEIGHT;
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
        lcd_draw(bgcolor);
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
  int x, y, dx, dy, xinc, xinc_e, yinc, yinc_e, eadd, esub, steps;
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
      y   = start_y;
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
      yinc_e = 0;
      xinc   = 0;
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
  lcd_clear(RGB(0,0,0));
  ssi_speed(0); //ssi speed up (0 = default speed)

  return;
}
