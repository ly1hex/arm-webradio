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


#if defined(L2F50)


void lcd_draw(unsigned int color)
{
  return;
}


void lcd_drawstop(void)
{
  return;
}


void lcd_drawstart(void)
{
  return;
}


void lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y)
{
  return;
}


void lcd_cursor(unsigned int x, unsigned int y)
{
  return;
}


void lcd_cmd(unsigned int reg, unsigned int param)
{
  return;
}


void lcd_data(unsigned int c)
{
  return;
}


void lcd_reset(void)
{
  return;
}


#endif //L2F50
