#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "third_party/fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd/img.h"
#include "lcd.h"
#include "station.h"
#include "share.h"
#include "card.h"
#include "alarm.h"
#include "settings.h"
#include "buffer.h"
#include "menu.h"


#define SUB_STATION  (0)
#define SUB_SHARE    (1)
#define SUB_CARD     (2)
#define SUB_ALARM    (3)
#define SUB_SETTINGS (4)
#define SUB_BACK     (5)
//#define SUB_STANDBY  (6)
#define MAINITEMS    (6)
const MAINMENUITEM mainmenu[MAINITEMS] =
{
  {"Station ", {&img_station[0][0],  &img_station[1][0],  &img_station[2][0]},  station_init, station_items,  station_getitem,  station_openitem,  station_closeitem, station_service},
  {" Share  ", {&img_share[0][0],    &img_share[1][0],    &img_share[2][0]},    share_init,   share_items,    share_getitem,    share_openitem,    share_closeitem,   share_service},
  {"  Card  ", {&img_card[0][0],     &img_card[1][0],     &img_card[2][0]},     card_init,    card_items,     card_getitem,     card_openitem,     card_closeitem,    card_service},
  {" Alarm  ", {&img_clock[0][0],    &img_clock[1][0],    &img_clock[2][0]},    alarm_init,   alarm_items,    alarm_getitem,    alarm_openitem,    alarm_closeitem,   0},
  {"Settings", {&img_settings[0][0], &img_settings[1][0], &img_settings[2][0]}, 0,            settings_items, settings_getitem, settings_openitem, 0,                 0},
//  {"  Back  ", {&img_back[0][0],     &img_back[1][0],     &img_back[2][0]},     0,            0,              0,                0,                 0,                 0},
  {"Standby ", {&img_power[0][0],    &img_power[1][0],    &img_power[2][0]},    0,            0,              0,                standby,           0,                 0},
};

#define MODE_INFO (0) //normal screen
#define MODE_MAIN (1) //animated main menu
#define MODE_SUB  (2) //list menu
unsigned int menu_mode=0, menu_sub=0, menu_items=0, menu_first=0, menu_last=0, menu_sel=0, menu_lastsel=0;
unsigned int menu_status=0;
unsigned int bgcolor=0, fgcolor=0, selcolor=0, edgecolor=0;


unsigned int menu_openfile(char *file)
{
  unsigned int r=0;

  if(file[0])
  {
    if(mainmenu[menu_sub].close)
    {
      mainmenu[menu_sub].close();
    }

    if(isdigit(file[0])) //station number
    {
      menu_sub   = SUB_STATION;
      menu_items = 1;
      menu_first = 0;
      menu_last  = 0;
      menu_sel   = atoi(file);
      mainmenu[menu_sub].init();
      if(station_open(menu_sel) == STATION_OPENED)
      {
        r = 1;
      }
    }
    else //path to card file
    {
      menu_sub   = SUB_CARD;
      menu_items = 1;
      menu_first = 0;
      menu_last  = 0;
      menu_sel   = 0;
      if(file != gbuf.card.file)
      {
        mainmenu[menu_sub].init();
      }
      if(card_openfile(file) == MENU_PLAY)
      {
        r = 1;
      }
    }

    menu_update(1);
  }

  return r;
}


unsigned int menu_sw(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      if(menu_items == 0)
      {
        menu_mode    = MODE_MAIN;
        menu_items   = 0;
        menu_sel     = menu_sub;
        menu_lastsel = (menu_sel==(MAINITEMS-1))?0:(menu_sel+1);
      }
      else
      {
        if(mainmenu[menu_sub].close)
        {
          mainmenu[menu_sub].close();
        }
        menu_mode    = MODE_SUB;
        if(menu_last == 0)
        {
          menu_items   = mainmenu[menu_sub].items();
          menu_first   = menu_sel;
          menu_last    = menu_sel+(MENU_LINES-1);
          if(menu_last >= menu_items)
          {
            menu_last = menu_items-1;
          }
        }
      }
      break;

    case MODE_MAIN:
      if(mainmenu[menu_sel].items)
      {
        menu_mode  = MODE_SUB;
        menu_sub   = menu_sel;
        if(mainmenu[menu_sub].init)
        {
          mainmenu[menu_sub].init();
        }
        menu_items   = mainmenu[menu_sub].items();
        menu_first   = 0;
        menu_last    = (menu_items >= MENU_LINES)?(MENU_LINES-1):(menu_items-1);
        menu_sel     = 0;
      }
      else
      {
        if(mainmenu[menu_sel].open)
        {
          mainmenu[menu_sel].open(0);
          //stay in main mode
          menu_lastsel = (menu_sel==(MAINITEMS-1))?0:(menu_sel+1);
        }
        else //back
        {
          menu_mode  = MODE_INFO;
          menu_items = 0;
        }
      }
      break;

    case MODE_SUB:
      if(mainmenu[menu_sub].open)
      {
        switch(mainmenu[menu_sub].open(menu_sel))
        {
          case MENU_BACK:
            menu_mode    = MODE_MAIN;
            menu_items   = 0;
            menu_sel     = menu_sub;
            menu_lastsel = (menu_sel==(MAINITEMS-1))?0:(menu_sel+1);
            break;
          case MENU_PLAY:
            menu_mode = MODE_INFO;
            break;
          case MENU_UPDATE:
          case MENU_ERROR:
            menu_items = mainmenu[menu_sub].items();
            menu_first = 0;
            menu_last  = (menu_items>=MENU_LINES)?(MENU_LINES-1):(menu_items-1);
            menu_sel   = 0;
            break;
        }
      }
      break;
  }

  return 1;
}


unsigned int menu_swlong(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
//standby
      break;
    case MODE_MAIN:
      menu_mode  = MODE_INFO;
      menu_items = 0;
      break;
    case MODE_SUB:
      menu_mode    = MODE_MAIN;
      menu_sel     = menu_sub;
      menu_lastsel = (menu_sub==MAINITEMS-1)?0:menu_sub+1;
      break;
  }

  return 1;
}


void menu_up(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      vs_setvolume(vs_volume()+2);
      menu_drawvol();
      break;

    case MODE_MAIN:
      if(menu_sel < (MAINITEMS-1))
      {
        menu_sel++;
      }
      else
      {
        menu_sel = 0;
      }
      menu_drawmain(0);
      break;

    case MODE_SUB:
      if(menu_sel < menu_last)
      {
        menu_sel++;
      }
      else if(menu_sel < (menu_items-1))
      {
        menu_sel++;
        menu_first++;
        menu_last++;
      }
      break;
  }

  return;
}


void menu_down(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      vs_setvolume(vs_volume()-2);
      menu_drawvol();
      break;

    case MODE_MAIN:
      if(menu_sel > 0)
      {
        menu_sel--;
      }
      else
      {
        menu_sel = (MAINITEMS-1);
      }
      menu_drawmain(0);
      break;

    case MODE_SUB:
      if(menu_sel > menu_first)
      {
        menu_sel--;
      }
      else if(menu_sel > 0)
      {
        menu_sel--;
        menu_first--;
        menu_last--;
      }
      break;
  }

  return;
}


void menu_steps(int steps)
{
  if(steps != 0)
  {
    if(menu_mode == MODE_INFO)
    {
      vs_setvolume(vs_volume()+steps);
      menu_drawvol();
    }
    else
    {
      if(steps > 0)
      {
        if(steps > MENU_LINES)
        {
          steps = MENU_LINES;
        }
        while(steps--)
        {
          menu_up();
        }
      }
      else
      {
        if(steps < -MENU_LINES)
        {
          steps = -MENU_LINES;
        }
        while(steps++)
        {
          menu_down();
        }
      }
    }
  }

  return;
}


void menu_service(unsigned int draw)
{
  unsigned int redraw=0;

  menu_steps(keys_steps());

  switch(keys_sw())
  {
    case SW_PRESSED:
      redraw |= menu_sw();
      break;
    case SW_PRESSEDLONG:
      redraw |= menu_swlong();
      break;
  }

  switch(ir_cmd())
  {
    case SW_VOL_P:
      if(menu_mode == MODE_INFO)
      {
        vs_setvolume(vs_volume()+4);
        menu_drawvol();
      }
      else if(menu_mode == MODE_SUB)
      {
        menu_steps(+MENU_LINES);
      }
      break;
    case SW_VOL_M:
      if(menu_mode == MODE_INFO)
      {
        vs_setvolume(vs_volume()-4);
        menu_drawvol();
      }
      else if(menu_mode == MODE_SUB)
      {
        menu_steps(-MENU_LINES);
      }
      break;
    case SW_UP:
      menu_steps(-1);
      break;
    case SW_DOWN:
      menu_steps(1);
      break;
    case SW_ENTER:
      redraw |= menu_sw();
      break;
    case SW_POWER:
      if(mainmenu[menu_sub].close)
      {
        mainmenu[menu_sub].close();
      }
      standby(0);
      redraw = 1;
      break;
  }

  menu_update(redraw);

  if((redraw == 0) && (menu_mode == MODE_INFO))
  {
    if(draw & SEC_CHANGED)
    {
      menu_drawclock();
    }
    if(draw & DAY_CHANGED)
    {
      menu_drawdate();
    }
  }

  if(mainmenu[menu_sub].service)
  {
    mainmenu[menu_sub].service();
  }

  return;
}


void menu_alarm(void)
{
  unsigned int i;

  //set alarm volume
  i = alarm_getvol();
  if(i)
  {
    vs_setvolume(i);
    menu_drawvol();
  }

  //open alarm file
  if(menu_status == MENU_STATE_STOP)
  {
    menu_mode = MODE_INFO;
    menu_update(1);
    menu_popup("Alarm");
    for(i=1; i<=ALARM_FILEITEMS; i++) //open alarm file
    {
      if(alarm_getfile(gbuf.menu.file, i))
      {
        if(menu_openfile(gbuf.menu.file))
        {
          break;
        }
      }
    }
    menu_items   = mainmenu[menu_sub].items();
    menu_first   = 0;
    menu_last    = (menu_items >= MENU_LINES)?(MENU_LINES-1):(menu_items-1);
    menu_update(1);
  }

  return;
}


void menu_popup(char *s)
{
  lcd_rectedge(4, (LCD_HEIGHT/2)-11, LCD_WIDTH-1-4, (LCD_HEIGHT/2)+11, edgecolor);
  lcd_rect(5, (LCD_HEIGHT/2)-10, LCD_WIDTH-1-5, (LCD_HEIGHT/2)+10, fgcolor);
  lcd_puts(10, (LCD_HEIGHT/2)- 4, s, SMALLFONT, bgcolor, fgcolor);

  return;
}


#define ITEM_LEFTTOP (2)
void menu_drawsub(unsigned int redraw)
{
  unsigned int i, x;
  static unsigned int last_first=0;
  char tmp[MAX_NAME];

  if(redraw == 0)
  {
    if(menu_sel == menu_lastsel)
    {
      return;
    }
  }

  //clear last selection
  i = (menu_lastsel-last_first)*MENU_LINEHEIGHT;
  lcd_rectedge(0, i, LCD_WIDTH-5, i+MENU_LINEHEIGHT, bgcolor);

  //selection
  i = (menu_sel-menu_first)*MENU_LINEHEIGHT;
  lcd_rectedge(0, i, LCD_WIDTH-5, i+MENU_LINEHEIGHT, selcolor);

  //draw items
  if((menu_first != last_first) || redraw)
  {
    last_first = menu_first;
    for(i=0; (i < MENU_LINES) && ((menu_first+i) < menu_items); i++)
    {
      mainmenu[menu_sub].get(menu_first+i, tmp);
      x = lcd_puts(ITEM_LEFTTOP, ITEM_LEFTTOP+(i*MENU_LINEHEIGHT), tmp, NORMALFONT, fgcolor, bgcolor);
      if(x < (LCD_WIDTH-5))
      {
        lcd_rect(x, i*MENU_LINEHEIGHT+1, LCD_WIDTH-5-1, (i*MENU_LINEHEIGHT)+MENU_LINEHEIGHT-1, bgcolor);
      }
    }
  }

  //scrollbar
  i = (menu_sel*(LCD_HEIGHT-1-8))/(menu_items-1);
  lcd_rect(LCD_WIDTH-4, 0, LCD_WIDTH-1, LCD_HEIGHT-1, edgecolor);
  lcd_rect(LCD_WIDTH-4, i, LCD_WIDTH-1, i+8, fgcolor);

  menu_lastsel = menu_sel;

  return;
}


void menu_drawmain(unsigned int redraw)
{
  int x[4], add;
  unsigned int state[4], item[4];
  unsigned int i, stop;

  if(redraw == 0)
  {
    if(menu_sel == menu_lastsel)
    {
      return;
    }
  }

  x[0]     = 0;
  x[1]     = MENU_MIDDLEITEM;
  x[2]     = MENU_RIGHTITEM;
  state[0] = 2;
  state[1] = 0;
  state[2] = 2;
  state[3] = 2;

  item[1] = menu_lastsel;

  if(menu_lastsel == 0)
    item[0] = MAINITEMS-1;
  else
    item[0] = menu_lastsel-1;

  if(menu_lastsel == (MAINITEMS-1))
    item[2] = 0;
  else
    item[2] = menu_lastsel+1;

  if(((menu_sel > menu_lastsel) && ((menu_sel-menu_lastsel) == 1)) ||
    ((menu_sel == 0) && ((menu_lastsel == (MAINITEMS-1))) ))            //move ->
  {
    if(item[2] == (MAINITEMS-1))
      item[3] = 0;
    else
      item[3] = item[2]+1;

    x[3] = MENU_RIGHTITEM+MENU_MIDDLEITEM;
    add  = -2;
  }
  else                                                                  //move <-
  {
    if(item[0] == 0)
      item[3] = MAINITEMS-1;
    else
      item[3] = item[0]-1;

    x[3] = -MENU_MIDDLEITEM;
    add  = +2;
  }

  stop = 0;
  while(stop == 0)
  {
    for(i=0; i<4; i++)
    {
      x[i] += add;
      switch(x[i])
      {
        case MENU_MIDDLEITEM/3:
          state[i]=2;
          break;
        case MENU_MIDDLEITEM/2:
          state[i]=1;
          break;
        case MENU_MIDDLEITEM-10:
          state[i]=0;
          break;
        case MENU_MIDDLEITEM:
          state[i]=0;
          stop=1;
          break;
        case MENU_MIDDLEITEM+10:
          state[i]=0;
          break;
        case MENU_MIDDLEITEM+(MENU_MIDDLEITEM/3):
          state[i]=1;
          break;
        case MENU_MIDDLEITEM+(MENU_MIDDLEITEM/2):
          state[i]=2;
          break;
      }

      lcd_img32(x[i], MENU_TOP, mainmenu[item[i]].img[state[i]], fgcolor, bgcolor);
    }
  }

  lcd_putline(MENU_TEXTX, MENU_TEXTY, mainmenu[menu_sel].name, NORMALFONT, fgcolor, bgcolor);

  menu_lastsel = menu_sel;

  return;
}


void menu_drawclock(void)
{
  lcd_puts(25, 100, getclock(), TIMEFONT, fgcolor, bgcolor);

  return;
}


void menu_drawdate(void)
{
  lcd_puts(37, 85, getdate(), SMALLFONT, edgecolor, bgcolor);

  return;
}


void menu_drawvol(void)
{
  unsigned int x;

  if(menu_mode == MODE_INFO)
  {
    x = (vs_volume()/2);
    lcd_rect(LCD_WIDTH-1-5-50,   2, LCD_WIDTH-1-5-50+x, 8, fgcolor);
    lcd_rect(LCD_WIDTH-1-5-50+x, 2, LCD_WIDTH-1-5,      8, bgcolor);
  }

  return;
}


void menu_drawstatus(void)
{
  if(menu_mode == MODE_INFO)
  {
    switch(menu_status)
    {
      case MENU_STATE_STOP:
        lcd_putc(90, 2, 0xFE, SMALLFONT, bgcolor, edgecolor);
        break;
      case MENU_STATE_BUF:
        lcd_putc(90, 2, 0xFD, SMALLFONT, bgcolor, edgecolor);
        break;
      case MENU_STATE_PLAY:
        lcd_putc(90, 2, 0xFC, SMALLFONT, bgcolor, edgecolor);
        break;
    }
  }

  return;
}


void menu_setinfo(unsigned int status, const char *info)
{
  menu_status = status;
  menu_drawstatus();

  if(*info)
  {
    strncpy(gbuf.menu.info, info, MAX_INFOTXT-1);
    DEBUGOUT("Menu: info: %s\n", info);
    if(menu_mode == MODE_INFO)
    {
      menu_drawinfo(1);
    }
  }

  return;
}


void menu_drawinfo(unsigned int redraw)
{
  if(redraw == 0)
  {
    return;
  }

  menu_drawstatus();
  menu_drawvol();
  menu_drawdate();
  menu_drawclock();

  lcd_putline(5, 30, gbuf.menu.file, NORMALFONT, fgcolor, bgcolor);
  lcd_putline(5, 60, gbuf.menu.info, NORMALFONT, fgcolor, bgcolor);

  return;
}


void menu_drawbg(void)
{
  lcd_clear(bgcolor);

  switch(menu_mode)
  {
    case MODE_INFO:
      lcd_rect(0, 0, LCD_WIDTH-1, 10, edgecolor);
      lcd_puts( 5,   2, APPNAME, SMALLFONT, bgcolor, edgecolor);
      lcd_puts( 5,  20, "Play:", SMALLFONT, edgecolor, bgcolor);
      lcd_puts( 5,  50, "Info:", SMALLFONT, edgecolor, bgcolor);
      break;
    case MODE_MAIN:
      break;
    case MODE_SUB:
      break;
  }

  return;
}


void menu_update(unsigned int redraw)
{
  if(redraw)
  {
    menu_drawbg();
  }

  switch(menu_mode)
  {
    case MODE_INFO: menu_drawinfo(redraw); break;
    case MODE_MAIN: menu_drawmain(redraw); break;
    case MODE_SUB:  menu_drawsub(redraw);  break;
  }

  return;
}


void menu_setcolors(unsigned int bg, unsigned int fg, unsigned int sel, unsigned int edge)
{
  bgcolor   = bg;
  fgcolor   = fg;
  selcolor  = sel;
  edgecolor = edge;

  return;
}


void menu_init(void)
{
  DEBUGOUT("Menu: init\n");

  gbuf.menu.info[0]             = 0;
  gbuf.menu.info[MAX_INFOTXT-1] = 0;
  gbuf.menu.file[0]             = 0;
  gbuf.menu.file[MAX_FILE-1]    = 0;

  menu_setinfo(MENU_STATE_STOP, " ");
  menu_update(1);

  //auto start
  if(ini_getentry(SETTINGS_FILE, "AUTOSTART", gbuf.menu.file, MAX_FILE-1))
  {
    menu_openfile(gbuf.menu.file);
  }

  return;
}
