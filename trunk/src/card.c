#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "mmc.h"
#include "vs.h"
#include "menu.h"
#include "buffer.h"
#include "card.h"


unsigned int card_status=CARD_CLOSED;
long card_timeout=0;


void card_service(void)
{
  unsigned int rd;

  if(card_status != CARD_PLAY)
  {
    return;
  }

  while(vsbuf_free() >= CARD_READBUF)
  {
    if(f_read(&gbuf.card.fsrc, gbuf.card.buf, CARD_READBUF, &rd) == FR_OK)
    {
      if(rd)
      {
        vsbuf_puts(gbuf.card.buf, rd);
        card_timeout = getontime()+CARD_TIMEOUT;
      }
      else
      {
        break;
      }
    }
    else
    {
      break;
    }
  }

  if(vsbuf_len() < 32) //song end
  {
    DEBUGOUT("Card: buf < 32\n");
    card_closeitem();
  }
  else if(getdeltatime(card_timeout) > 0) //time out
  {
    DEBUGOUT("Card: timeout\n");
    card_closeitem();
  }
  else
  {
    vs_play();
  }

  return;
}


void shrink_path(char *path)
{
  unsigned int i;

  for(i=strlen(path); i!=0; i--)
  {
    if(path[i] == '/')
    {
      break;
    }
  }
  path[i] = 0;

  return;
}


void card_closeitem(void)
{
  if(card_status != CARD_CLOSED)
  {
    card_status = CARD_CLOSED;
    vs_stop();
    f_close(&gbuf.card.fsrc);
    shrink_path(gbuf.card.file);
    menu_setstatus(MENU_STATE_STOP);
    menu_setinfo("");
    DEBUGOUT("Card: closed\n");
  }

  return;
}


unsigned int card_openfile(const char *file)
{
  if(file != gbuf.card.file)
  {
    strcpy(gbuf.card.file, file);
  }

  if(f_open(&gbuf.card.fsrc, gbuf.card.file, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    card_status = CARD_PLAY;
    vs_start();
    menu_setstatus(MENU_STATE_PLAY);
    menu_setname(file);
    menu_setinfo("");
    return MENU_PLAY;
  }

  return MENU_ERROR;
}


unsigned int card_openitem(unsigned int item)
{
  char tmp[MAX_ADDR];

  if(item == 0) //back
  {
    if(gbuf.card.file[0] == 0)
    {
      return MENU_BACK;
    }
    else //up dir
    {
      shrink_path(gbuf.card.file);
    }
  }
  else
  {
    item--;
    if(fs_isdir(gbuf.card.file, item)) //open dir
    {
      fs_getitem(gbuf.card.file, item, tmp, MAX_ADDR);
      strcat(gbuf.card.file, tmp);
    }
    else //play item
    {
      fs_getitemtag(gbuf.card.file, item, gbuf.menu.name, MAX_NAME);
      menu_setinfo("");
      fs_getitem(gbuf.card.file, item, tmp, MAX_ADDR);
      strcat(gbuf.card.file, "/");
      strcat(gbuf.card.file, tmp);
      DEBUGOUT("Card: %s\n", gbuf.card.file);
      if(f_open(&gbuf.card.fsrc, gbuf.card.file, FA_OPEN_EXISTING | FA_READ) == FR_OK)
      {
        card_status = CARD_PLAY;
        vs_start();
        menu_setstatus(MENU_STATE_PLAY);
        return MENU_PLAY;
      }
      else
      {
        shrink_path(gbuf.card.file);
        return MENU_ERROR;
      }
    }
  }

  return MENU_UPDATE;
}


void card_getitem(unsigned int item, char *name)
{
  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    fs_getitemtag(gbuf.card.file, item-1, name, MAX_NAME);
  }

  return;
}


unsigned int card_items(void)
{
  return fs_items(gbuf.card.file)+1;
}


void card_init(void)
{
  DEBUGOUT("Card: init\n");

  card_status = CARD_CLOSED;

  gbuf.card.name[0] = 0;
  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;

  menu_setstatus(MENU_STATE_STOP);

  return;
}
