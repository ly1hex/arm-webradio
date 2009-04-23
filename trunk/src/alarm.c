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
  if(ini_getentry(SETTINGS_FILE, entry, dst, MAX_ADDR) == 0)
  {
    return 0;
  }

  return 1;
}


unsigned int alarm_getvol(void)
{
  char buf[16];

  if(ini_getentry(SETTINGS_FILE, "ALARMVOL", buf, 16) == 0)
  {
    return atoi(buf);
  }
  
  return 50;
}


unsigned int alarm_settime(unsigned int item, ALARMTIME *time)
{
  char entry[16], buf[32];

  sprintf(entry, "TIME%i", item);

  sprintf(buf, " %02i:%02i:", time->h, time->m);
  if(time->on == 0)
  {
    buf[0] = '!';
  }
  if(time->wdays & (1<<1)){ strcat(buf, "Mo"); }
  if(time->wdays & (1<<2)){ strcat(buf, "Tu"); }
  if(time->wdays & (1<<3)){ strcat(buf, "We"); }
  if(time->wdays & (1<<4)){ strcat(buf, "Th"); }
  if(time->wdays & (1<<5)){ strcat(buf, "Fr"); }
  if(time->wdays & (1<<6)){ strcat(buf, "Sa"); }
  if(time->wdays & (1<<0)){ strcat(buf, "Su"); }

  if(ini_setentry(ALARM_FILE, entry, buf) == 0)
  {
    return 0;
  }

  return 1;
}


unsigned int alarm_gettime(unsigned int item, ALARMTIME *time)
{
  char entry[16], buf[MAX_NAME], *ptr, c1, c2;

  memset(time, 0, sizeof(ALARMTIME));

  sprintf(entry, "TIME%i", item);
  if(ini_getentry(ALARM_FILE, entry, buf, MAX_NAME) == 0)
  {
    ptr = buf;
    if(*ptr != '!') //alarm on
    {
      time->on = 1;
    }
    while(!isdigit(*ptr)){ ptr++; }; //skip non digits
    time->h = atoi(ptr);             //get hour
    while(isdigit(*ptr)) { ptr++; }; //skip numbers
    while(!isdigit(*ptr)){ ptr++; }; //skip non numbers
    time->m = atoi(ptr);             //get min
    while(isdigit(*ptr)) { ptr++; }; //skip numbers
    while(!isalpha(*ptr)){ ptr++; }; //skip non numbers
    while(*ptr) //get days
    {
      c1 = toupper(*ptr++);
      c2 = toupper(*ptr++);
      if((c1 == 0)   || (c2 == 0))  { break; }
      if((c1 == 'S') && (c2 == 'U')){ time->wdays |= (1<<0); } //Sunday
      if((c1 == 'M') && (c2 == 'O')){ time->wdays |= (1<<1); } //Monday
      if((c1 == 'T') && (c2 == 'U')){ time->wdays |= (1<<2); } //Tuesday
      if((c1 == 'W') && (c2 == 'E')){ time->wdays |= (1<<3); } //Wednesday
      if((c1 == 'T') && (c2 == 'H')){ time->wdays |= (1<<4); } //Thursday
      if((c1 == 'F') && (c2 == 'R')){ time->wdays |= (1<<5); } //Friday
      if((c1 == 'S') && (c2 == 'A')){ time->wdays |= (1<<6); } //Saturday
    }
    return 0;
  }

  return 1;
}


void alarm_load(void)
{
  unsigned int i;
  char data[MAX_NAME], *ptr, c1, c2;

  //reset all alarm times
  memset(alarmtimes, 0, sizeof(alarmtimes));

  for(i=0; i<ALARMTIMES; i++)
  {
    alarm_gettime(i+1, &alarmtimes[i]);
  }

  return;
}


unsigned int alarm_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else
  {
    if(dlg_alarmtime(&alarmtimes[item-1]) == 0) //time modified -> save to ini
    {
      if(alarm_settime(item, &alarmtimes[item-1]) != 0)
      {
        return MENU_ERROR;
      }
    }
  }

  return MENU_NOP;
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
    if(alarmtimes[item].wdays & (1<<1)){ strcat(name, "Mo"); }
    if(alarmtimes[item].wdays & (1<<2)){ strcat(name, "Tu"); }
    if(alarmtimes[item].wdays & (1<<3)){ strcat(name, "We"); }
    if(alarmtimes[item].wdays & (1<<4)){ strcat(name, "Th"); }
    if(alarmtimes[item].wdays & (1<<5)){ strcat(name, "Fr"); }
    if(alarmtimes[item].wdays & (1<<6)){ strcat(name, "Sa"); }
    if(alarmtimes[item].wdays & (1<<0)){ strcat(name, "Su"); }
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
