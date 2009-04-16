#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "eth.h"
#include "eth/utils.h"
#include "menu.h"
#include "buffer.h"
#include "share.h"


void share_service(void)
{
  return;
}


void share_closeitem(void)
{
  return;
}


unsigned int share_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else //play
  {
    return MENU_PLAY;
  }

  return MENU_UPDATE;
}


void share_getitem(unsigned int item, char *name)
{
  char entry[16];

  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    sprintf(entry, "TITLE%i", item);
    ini_getentry(SHARE_FILE, entry, name, MAX_NAME-1);
  }

  return;
}

unsigned int share_items(void)
{
  char entry[16];

  if(ini_getentry(SHARE_FILE, "NUMBEROFENTRIES", entry, 16-1))
  {
    return atoi(entry)+1;
  }

  return 1;
}


void share_init(void)
{
  DEBUGOUT("Share: init\n");

  gbuf.card.name[0] = 0;
  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;

  return;
}
