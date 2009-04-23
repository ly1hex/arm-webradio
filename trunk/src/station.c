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
#include "eth/shoutcast.h"
#include "vs.h"
#include "menu.h"
#include "buffer.h"
#include "station.h"


unsigned int station_item=0;
unsigned int station_status=STATION_CLOSED;
long station_timeout=0;
unsigned int station_try=0;
unsigned int station_bufmin=0, station_bufplay=0, station_bufstart=0;


void station_calcbuf(unsigned int br) //bitrate kbit/s
{
  char tmp[8];
  unsigned int len;

  if((br >= 64) || (br == 0)) //bitrate >= 64
  {
    station_bufmin   = STATION_BUFMIN;
    station_bufplay  = STATION_BUFPLAY;
    station_bufstart = STATION_BUFSTART;
  }
  else
  {
    station_bufmin   = br/8*512;
    station_bufplay  = station_bufmin*4;
    station_bufstart = station_bufmin*6;
  }

  if((br >= 8) && (br <= 320))
  {
    len = strlen(gbuf.station.name);
    sprintf(tmp, " [%i]", br);
    strncat(gbuf.station.name, tmp, MAX_NAME-1-len);
  }

  return;
}


void station_close(void)
{
  if(station_status != STATION_CLOSED)
  {
    shoutcast_close();
    vs_stop();
    station_status = STATION_CLOSED;
    menu_setstatus(MENU_STATE_STOP);
    menu_setinfo("");
    DEBUGOUT("Station: closed\n");
  }

  return;
}


unsigned int station_open(unsigned int item)
{
  unsigned int r=STATION_CLOSED;
  char proto[8];
  IP_Addr ip;
  unsigned int port;
  char file[MAX_URLFILE];

  if(station_getitemaddr(item, gbuf.station.addr) != 0)
  {
    return STATION_CLOSED;
  }

  station_getitem(item, gbuf.menu.name);
  menu_setinfo("");

  menu_drawpopup("Open Station...");
  DEBUGOUT("Station: %i %s\n", station_try, gbuf.station.addr);

  vs_start();

  station_item    = item;
  station_status  = STATION_OPEN;
  station_timeout = getontime()+STATION_TIMEOUT;

  atoaddr(gbuf.station.addr, proto, 0, 0, &ip, &port, file);
  if(strcmp(proto, "http") == 0)
  {
    r = shoutcast_open(ip, port, file);
    if(r == SHOUTCAST_OPENED)
    {
      station_timeout = getontime()+STATION_TIMEOUT;
      menu_setstatus(MENU_STATE_BUF);
      r = STATION_OPENED;
    }
    else if((r == SHOUTCAST_ERROR) ||
            (r == SHOUTCAST_SERVERFULL))
    {
      station_closeitem(); //also clears addr
      r = STATION_CLOSED;
    }
    else
    {
      r = STATION_OPEN;
    }
  }
  else if(strcmp(proto, "rtsp") == 0)
  {
    r = STATION_CLOSED;
  }

  return r;
}


void station_service(void)
{
  switch(station_status)
  {
    case STATION_OPENED:
      station_timeout = getontime()+STATION_TIMEOUT;
      if(vsbuf_len() < station_bufmin) //buffer
      {
        station_status = STATION_BUFFER;
        vs_pause();
        menu_setstatus(MENU_STATE_BUF);
        DEBUGOUT("Station: buffer\n");
      }
      else
      {
        vs_play();
      }
      break;

    case STATION_BUFFER:
      if(vsbuf_len() > station_bufplay)
      {
        station_status = STATION_OPENED;
        vs_play();
        menu_setstatus(MENU_STATE_PLAY);
        DEBUGOUT("Station: play\n");
      }
      if(getdeltatime(station_timeout) > 0)
      {
        station_status = STATION_OPEN;
      }
      break;

    case STATION_OPEN:
      if(vsbuf_len() > station_bufstart)
      {
        station_status = STATION_OPENED;
        vs_play();
        menu_setstatus(MENU_STATE_PLAY);
        DEBUGOUT("Station: play\n");
      }
      if(getdeltatime(station_timeout) > 0)
      {
        station_close();
        if(station_try)
        {
          station_try--;
          station_open(station_item);
          menu_drawwnd(1);
        }
      }
      break;

    case STATION_CLOSED:
      break;
  }

  return;
}


void station_closeitem(void)
{
  station_close();
  station_init();

  return;
}


unsigned int station_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else //play item
  {
    station_try = STATION_TRY;
    if(station_open(item) != STATION_CLOSED)
    {
      return MENU_PLAY;
    }
  }

  return MENU_ERROR;
}


unsigned int station_getitemaddr(unsigned int item, char *addr)
{
  char entry[16];

  *addr = 0;

  if(item)
  {
    sprintf(entry, "FILE%i", item);
    if(ini_getentry(STATION_FILE, entry, addr, MAX_ADDR) == 0)
    {
      return 0;
    }
  }

  return 1;
}


void station_getitem(unsigned int item, char *name)
{
  char entry[16];

  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    sprintf(entry, "TITLE%i", item);
    ini_getentry(STATION_FILE, entry, name, MAX_NAME);
  }

  return;
}


unsigned int station_items(void)
{
  char entry[16];

  if(ini_getentry(STATION_FILE, "NUMBEROFENTRIES", entry, 16) == 0)
  {
    return atoi(entry)+1;
  }

  return 1;
}


void station_init(void)
{
  DEBUGOUT("Station: init\n");

  station_item     = 0;
  station_status   = STATION_CLOSED;
  station_try      = STATION_TRY;
  station_calcbuf(0);

  gbuf.card.name[0] = 0;
  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;

  menu_setstatus(MENU_STATE_STOP);

  return;
}
