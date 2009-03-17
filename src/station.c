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

  return;
}


unsigned int station_open(unsigned int item)
{
  unsigned int r=STATION_CLOSED;
  char proto[8];
  IP_Addr ip;
  unsigned int port;
  char url[MAX_URL];

  if(item == 0)
  {
    return;
  }

  station_item    = item;
  station_status  = STATION_OPEN;
  station_timeout = getontime()+STATION_TIMEOUT;

  vs_start();

  station_getitem(item, gbuf.menu.info);
  menu_setinfo(MENU_STATE_STOP, "");

  station_getitemaddr(item, gbuf.station.addr);
  atoaddr(gbuf.station.addr, proto, 0, 0, &ip, &port, url);

  menu_popup("Open Station...");
  DEBUGOUT("Station: %i %s\n", station_try, gbuf.station.addr);

  if(strcmp(proto, "http") == 0)
  {
    r = shoutcast_open(ip, port, url);
    if(r == SHOUTCAST_OPENED)
    {
      station_timeout = getontime()+STATION_TIMEOUT;
      menu_setinfo(MENU_STATE_BUF, "");
      r = STATION_OPENED;
    }
    else if((r == SHOUTCAST_ERROR) ||
            (r == SHOUTCAST_SERVERFULL))
    {
      station_closeitem();
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
        menu_setinfo(MENU_STATE_BUF, "");
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
        menu_setinfo(MENU_STATE_PLAY, "");
        station_status = STATION_OPENED;
        vs_play();
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
        menu_setinfo(MENU_STATE_PLAY, "");
        station_status = STATION_OPENED;
        vs_play();
        DEBUGOUT("Station: play\n");
      }
      if(getdeltatime(station_timeout) > 0)
      {
        station_closeitem();
        if(station_try)
        {
          station_try--;
          station_open(station_item);
          menu_update(1);
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
  if(station_status != STATION_CLOSED)
  {
    shoutcast_close();
    vs_stop();
    station_status = STATION_CLOSED;
    menu_setinfo(MENU_STATE_STOP, "");
    DEBUGOUT("Station: closed\n");
  }

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
    else
    {
      return MENU_ERROR;
    }
  }

  return MENU_UPDATE;
}


void station_getitemaddr(unsigned int item, char *addr)
{
  char entry[16];

  if(item == 0) //back
  {
    strcpy(addr, "");
  }
  else
  {
    sprintf(entry, "FILE%i", item);
    ini_getentry(STATION_FILE, entry, addr, MAX_ADDR-1);
  }

  return;
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
    ini_getentry(STATION_FILE, entry, name, MAX_NAME-1);
  }

  return;
}


unsigned int station_items(void)
{
  char entry[16];

  if(ini_getentry(STATION_FILE, "NUMBEROFENTRIES", entry, 16-1))
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

  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;

  return;
}
