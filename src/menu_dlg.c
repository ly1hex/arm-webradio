#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "mmc.h"
#include "settings.h"
#include "menu.h"
#include "menu_dlg.h"


// "/ \ : * ? " < > |" not allowed
#define CHARACTERS (11+26+19)
const char characters[CHARACTERS] = 
{
  ' ', '0','1','2','3','4','5','6','7','8','9',
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '!','#','$','%','&','\'','`','(',')','+','-',',','.',';','=','[',']','_','/'
};


char next_char(char c, unsigned int dir) //char, direction (0=-, 1=+)
{
  unsigned int i;

  //find char
  for(i=0; i<CHARACTERS; i++)
  {
    if(c == characters[i])
    {
      break;
    }
  }

  if(dir) //++
  {
    if(i == (CHARACTERS-1))
    {
      i = 0;
    }
    else
    {
      i++;
    }
  }
  else //--
  {
    if(i == 0)
    {
      i = CHARACTERS-1;
    }
    else
    {
      i--;
    }
  }

  return characters[i];
}


#define CTRL_SELECTION (0)
#define CTRL_ACTICE    (1)
#define CTRL_MODIFY    (2)
int dlg_str(const char* title, const char *value, char *buf, unsigned int len)
{
  unsigned int i, close=0, state=CTRL_SELECTION;
  const unsigned int ctrls=3;
  CONTROL ctrl[ctrls];
  int sel=0, sw;

  //prepare buffer
  memset(buf, 0, len);
  buf[0]          = '<';
  buf[1]          = 0;
  strtoupper((buf+1), value);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 1,  5, 60,  0, " OK ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 0, 50, 60,  0, "Abort");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0,  5, 30, 20, buf);

  do
  {
    sw = dlg_service();

    if(sw)
    {
      switch(state)
      {
        case CTRL_SELECTION:
          switch(sw)
          {
            case SW_ENTER:
              state = CTRL_ACTICE;
              if     (sel == 0) { close = 1; } //OK
              else if(sel == 1) { close = 2; } //Abort
              break;
            case SW_UP:
            case SW_VOL_P:
              ctrl[sel].sel = 0;
              sel++;
              break;
            case SW_DOWN:
            case SW_VOL_M:
              ctrl[sel].sel = 0;
              sel--;
              break;
          }
          if(sel < 0)          { sel = 0; }
          else if(sel >= ctrls){ sel = ctrls-1; }
          ctrl[sel].sel = 1;
          break;

        case CTRL_ACTICE:
          switch(sw)
          {
            case SW_ENTER:
              if(ctrl[sel].p2 == 0)
              {
                state = CTRL_SELECTION;
              }
              else
              {
                state = CTRL_MODIFY;
              }
              break;
            case SW_UP:
            case SW_VOL_P:
              if(ctrl[sel].p2 < len)
              {
                ctrl[sel].p2++;
                if(ctrl[sel].p2 >= (ctrl[sel].p1+20))
                {
                  ctrl[sel].p1++;
                }
                if(ctrl[sel].val[ctrl[sel].p2] == 0)
                {
                  ctrl[sel].val[ctrl[sel].p2]   = ' ';
                  ctrl[sel].val[ctrl[sel].p2+1] = 0;
                }
              }
              break;
            case SW_DOWN:
            case SW_VOL_M:
              if(ctrl[sel].p2 > 0)
              {
                ctrl[sel].p2--;
                if(ctrl[sel].p2 < ctrl[sel].p1)
                {
                  ctrl[sel].p1--;
                }
              }
              break;
          }
          break;

        case CTRL_MODIFY:
          switch(sw)
          {
            case SW_ENTER:
              state = CTRL_ACTICE;
              break;
            case SW_UP:
            case SW_VOL_P:
              ctrl[sel].val[ctrl[sel].p2] = next_char(ctrl[sel].val[ctrl[sel].p2], 1); //++
              break;
            case SW_DOWN:
            case SW_VOL_M:
              ctrl[sel].val[ctrl[sel].p2] = next_char(ctrl[sel].val[ctrl[sel].p2], 0); //--
              break;
          }
          break;
      }

      for(i=0; i<ctrls; i++)
      {
        menu_drawctrl(&ctrl[i]);
      }
    }
  }while(close == 0); //(!keys_sw() && (ir_cmd() != SW_ENTER));

  //remove < symbol
  buf[0] = ' ';
  strrmvspace(buf, buf);

  if(close == 1) //1=OK, 2=ABORT
  {
    return 1;
  }

  return 0;
}


int dlg_rawir(unsigned int i)
{
  unsigned int curr_addr;
  int data;
  char tmp[32];

  menu_drawdlg("Raw IR Data", "Press a key on the\nremote control...");

  curr_addr = ir_addr();
  ir_setaddr(IR_ALLADDR);

  do
  {
    data = ir_rawdata();
    if(data & 0x8000)
    {
      sprintf(tmp, " Addr  %i\n Cmd   %i", ((data&0x07C0)>>6), (data&0x3F));
      menu_drawdlg("Raw IR data", tmp);
    }
  }while(!keys_sw());

  ir_setaddr(curr_addr);

  return 0;
}


int dlg_msg(const char* title, const char *msg)
{
  menu_drawdlg(title, msg);

  while(dlg_service() != SW_ENTER);

  return 0;
}


int dlg_service(void)
{
  int ret=0, step, sw, ir;

  eth_service();

  step = keys_steps();
  sw   = keys_sw();
  ir   = ir_cmd();

  if(step > 0)
  {
    ret = SW_UP;
  }
  else if(step < 0)
  {
    ret = SW_DOWN;
  }
  else
  {
    if(sw == SW_PRESSED)
    {
      ret = SW_ENTER;
    }
    else
    {
      ret = ir;
    }
  }

  return ret;
}
