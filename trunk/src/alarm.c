#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tools.h"
#include "main.h"
#include "io.h"
#include "menu.h"
#include "settings.h"
#include "alarm.h"


ALARMTIME alarmtimes[ALARMTIMES];


unsigned int alarm_check(TIME *time)
{
  unsigned int i, wday, h, m;

  wday = 1<<time->wday;
  h    = time->h;
  m    = time->m;

  for(i=0; i<ALARMTIMES; i++)
  {
    if(alarmtimes[i].on && (alarmtimes[i].wdays & wday) && (alarmtimes[i].h == h) && (alarmtimes[i].m == m))
    {
      return 1;
    }
  }

  return 0;
}


unsigned int alarm_getfile(char *dst, unsigned int nr)
{
  char entry[16];

  sprintf(entry, "ALARMFILE%i", nr);
  if(ini_getentry(SETTINGS_FILE, entry, dst, MAX_ADDR-1))
  {
    return 1;
  }

  return 0;
}


unsigned int alarm_getvol(void)
{
  char buf[16];

  if(ini_getentry(SETTINGS_FILE, "ALARMVOL", buf, 16-1))
  {
    return atoi(buf);
  }
  
  return 0;
}


void alarm_gettime(unsigned int item, char *time)
{
  char entry[16];

  sprintf(entry, "TIME%i", item);
  ini_getentry(ALARM_FILE, entry, time, MAX_NAME-1);

  return;
}


unsigned int alarm_gettimes(void)
{
  char entry[16];

  if(ini_getentry(ALARM_FILE, "NUMBEROFENTRIES", entry, 16-1))
  {
    return atoi(entry);
  }

  return 0;
}


void alarm_load(void)
{
  unsigned int i, items;
  char data[MAX_NAME], *ptr, c1, c2;

  //reset all alarm times
  memset(alarmtimes, 0, sizeof(alarmtimes));

  items = alarm_gettimes();
  if(items > ALARMTIMES)
  {
    items = ALARMTIMES;
  }

  for(i=0; i<items; i++)
  {
    alarm_gettime(i+1, data);
    ptr = data;
    if(*ptr != '!') //alarm on
    {
      alarmtimes[i].on = 1;
    }
    while(!isdigit(*ptr)){ ptr++; }; //skip non digits
    alarmtimes[i].h = atoi(ptr); //get hour
    while(isdigit(*ptr)) { ptr++; };
    while(!isdigit(*ptr)){ ptr++; };
    alarmtimes[i].m = atoi(ptr); //get minute
    while(isdigit(*ptr)) { ptr++; };
    while(!isalpha(*ptr)){ ptr++; };
    while(*ptr) //get days
    {
      c1 = toupper(*ptr++);
      c2 = toupper(*ptr++);
      if((c1 == 0)   || (c2 == 0))  { break; }
      if((c1 == 'S') && (c2 == 'U')){ alarmtimes[i].wdays |= (1<<0); } //Sunday
      if((c1 == 'M') && (c2 == 'O')){ alarmtimes[i].wdays |= (1<<1); } //Monday
      if((c1 == 'T') && (c2 == 'U')){ alarmtimes[i].wdays |= (1<<2); } //Tuesday
      if((c1 == 'W') && (c2 == 'E')){ alarmtimes[i].wdays |= (1<<3); } //Wednesday
      if((c1 == 'T') && (c2 == 'H')){ alarmtimes[i].wdays |= (1<<4); } //Thursday
      if((c1 == 'F') && (c2 == 'R')){ alarmtimes[i].wdays |= (1<<5); } //Friday
      if((c1 == 'S') && (c2 == 'A')){ alarmtimes[i].wdays |= (1<<6); } //Saturday
    }
  }

  return;
}


void alarm_closeitem(void)
{
  return;
}


unsigned int alarm_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }

  return MENU_UPDATE;
}


void alarm_getitem(unsigned int item, char *name)
{
  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    item--;
    if(alarmtimes[item].on)
    {
      sprintf(name, " %02i:%02i ", alarmtimes[item].h, alarmtimes[item].m);
    }
    else
    {
      sprintf(name, "!%02i:%02i ", alarmtimes[item].h, alarmtimes[item].m);
    }
    if(alarmtimes[item].wdays & (1<<0)){ strcat(name, "Su"); }
    if(alarmtimes[item].wdays & (1<<1)){ strcat(name, "Mo"); }
    if(alarmtimes[item].wdays & (1<<2)){ strcat(name, "Tu"); }
    if(alarmtimes[item].wdays & (1<<3)){ strcat(name, "We"); }
    if(alarmtimes[item].wdays & (1<<4)){ strcat(name, "Th"); }
    if(alarmtimes[item].wdays & (1<<5)){ strcat(name, "Fr"); }
    if(alarmtimes[item].wdays & (1<<6)){ strcat(name, "Sa"); }
  }

  return;
}


unsigned int alarm_items(void)
{
  return ALARMTIMES+1;
}


void alarm_init(void)
{
  DEBUGOUT("Alarm: init\n");

  return;
}
