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
#include "vs.h"
#include "eth.h"
#include "eth/utils.h"
#include "eth/dhcp.h"
#include "alarm.h"
#include "menu.h"
#include "menu_dlg.h"
#include "buffer.h"
#include "settings.h"


#define F_NONE        (0) //-
#define F_NR          (1) //p1-p2
#define F_OR          (2) //p1 or p2
#define F_STR         (3) //p1=max len
#define F_MAC         (4) //-
#define F_IP          (5) //-
#define F_RGB         (6) //-
#define F_RUN         (7) //-
#define F_TIME        (8) //-
#define F_INFO        (10) //-
#define SETTINGSITEMS (35)
const SETTINGSMENU settingsmenu[SETTINGSITEMS] =
{
  //name               ini-entry     format        p1     p2    p3  set-func
  {"Info...",           "",           F_INFO,       0,     0,    0, 0},
  {"PlayMode ",         "PLAYMODE",   F_NONE,       0,     0,    0, 0},
  {"Volume   ",         "VOLUME",     F_NR,         0,   100,    1, (void*)vs_setvolume},
  {"AutoStart",         "AUTOSTART",  F_STR,        0,     0,    0, 0},
  {"--- Alarm ---",     "",           F_NONE,       0,     0,    0, 0},
  {"Vol  ",             "ALARMVOL",   F_NR,         0,   100,    1, 0},
  {"File1",             "ALARMFILE1", F_STR,        0,     0,    0, 0},
  {"File2",             "ALARMFILE2", F_STR,        0,     0,    0, 0},
  {"File3",             "ALARMFILE3", F_STR,        0,     0,    0, 0},
  {"--- Audio ---",     "",           F_NONE,       0,     0,    0, 0},
  {"Bass-Freq Hz  ",    "BASSFREQ",   F_NR,        20,   150,   10, (void*)vs_setbassfreq},
  {"Bass-Amp dB   ",    "BASSAMP",    F_NR,         0,    15,    1, (void*)vs_setbassamp},
  {"Treble-Freq Hz",    "TREBLEFREQ", F_NR,      1000, 15000, 1000, (void*)vs_settreblefreq},
  {"Treble-Amp  dB",    "TREBLEAMP",  F_NR,        -8,     7,    1, (void*)vs_settrebleamp},
  {"VS",                "VS",         F_OR,      1033,  1053,    0, (void*)vs_init},
  {"--- Ethernet ---",  "",           F_NONE,       0,     0,    0, 0},
  {"Name",              "NAME",       F_STR,       15,     0,    0, (void*)eth_setname},
  {"MAC",               "MAC",        F_MAC,        0,     0,    0, 0}, //(void*)eth_setmac},
  {"DHCP",              "DHCP",       F_OR,         0,     1,    0, (void*)eth_setdhcp},
  {"IP  ",              "IP",         F_IP,         0,     0,    0, (void*)eth_setip},
  {"Mask",              "NETMASK",    F_IP,         0,     0,    0, (void*)eth_setnetmask},
  {"Rout",              "ROUTER",     F_IP,         0,     0,    0, (void*)eth_setrouter},
  {"DNS ",              "DNS",        F_IP,         0,     0,    0, (void*)eth_setdns},
  {"NTP ",              "NTP",        F_IP,         0,     0,    0, (void*)eth_setntp},
  {"Time-Diff sec",     "TIMEDIFF",   F_NR,    -43200, 43200, 3600, (void*)eth_settimediff},
  {"Summer-Time  ",     "SUMMER",     F_OR,         0,     1,    0, (void*)eth_setsummer},
  {"Get Time from NTP", "",           F_TIME,       0,     0,    0, 0},
  {"--- IR ---",        "",           F_NONE,       0,     0,    0, 0},
  {"IR Addr",           "IR",         F_NR,         0,    31,    1, (void*)ir_setaddr},
  {"Show raw IR Data",  "",           F_RUN,        0,     0,    0, (void*)dlg_rawir},
  {"--- Colors ---",    "",           F_NONE,       0,     0,    0, 0},
  {"BG  ",              "COLORBG",    F_RGB,        0,     0,    0, (void*)menu_setbgcolor},
  {"FG  ",              "COLORFG",    F_RGB,        0,     0,    0, (void*)menu_setfgcolor},
  {"Sel ",              "COLORSEL",   F_RGB,        0,     0,    0, (void*)menu_setselcolor},
  {"Edge",              "COLOREDGE",  F_RGB,        0,     0,    0, (void*)menu_setedgecolor}
};


#define INI_BUFLEN (64)
void settings_read(void)
{
  char buf[INI_BUFLEN];
  unsigned int bg=DEFAULT_BGCOLOR, fg=DEFAULT_FGCOLOR, sel=DEFAULT_SELCOLOR, edge=DEFAULT_EDGECOLOR;

  //colors
  if(ini_getentry(SETTINGS_FILE, "COLORBG", buf, INI_BUFLEN) == 0)
  {
    bg = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLORFG", buf, INI_BUFLEN) == 0)
  {
    fg = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLORSEL", buf, INI_BUFLEN) == 0)
  {
    sel = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLOREDGE", buf, INI_BUFLEN) == 0)
  {
    edge = atorgb(buf);
  }

  menu_setbgcolor(bg);
  menu_setfgcolor(fg);
  menu_setselcolor(sel);
  menu_setedgecolor(edge);

  //alarm times
  alarm_load();

  //ir
  if(ini_getentry(SETTINGS_FILE, "IR", buf, INI_BUFLEN) == 0)
  {
    ir_setaddr(atoi(buf));
  }
  ir_init();

  //vs
  if(ini_getentry(SETTINGS_FILE, "VS", buf, INI_BUFLEN) == 0)
  {
    vs_init(atoi(buf));
  }
  else
  {
    vs_init(DEFAULT_VS);
  }
  if(ini_getentry(SETTINGS_FILE, "VOLUME", buf, INI_BUFLEN) == 0)
  {
    vs_setvolume(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "BASSFREQ", buf, INI_BUFLEN) == 0)
  {
    vs_setbassfreq(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "BASSAMP", buf, INI_BUFLEN) == 0)
  {
    vs_setbassamp(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "TREBLEFREQ", buf, INI_BUFLEN) == 0)
  {
    vs_settreblefreq(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "TREBLEAMP", buf, INI_BUFLEN) == 0)
  {
    vs_settrebleamp(atoi(buf));
  }

  //ethernet
  if(ini_getentry(SETTINGS_FILE, "NAME", buf, INI_BUFLEN) == 0)
  {
    eth_setname(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "MAC", buf, INI_BUFLEN) == 0)
  {
    eth_setmac(atomac(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "DHCP", buf, INI_BUFLEN) == 0)
  {
    eth_setdhcp((atoi(buf))?1:0);
  }
  else
  {
    eth_setdhcp(DEFAULT_DHCP);
  }
  if(eth_ip() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "IP", buf, INI_BUFLEN) == 0)
    {
      eth_setip(atoip(buf));
    }
    else
    {
      eth_setip(atoip(DEFAULT_IP));
    }
  }
  if(eth_netmask() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "NETMASK", buf, INI_BUFLEN) == 0)
    {
      eth_setnetmask(atoip(buf));
    }
    else
    {
      eth_setnetmask(atoip(DEFAULT_NETMASK));
    }
  }
  if(eth_router() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "ROUTER", buf, INI_BUFLEN) == 0)
    {
      eth_setrouter(atoip(buf));
    }
    else
    {
      eth_setrouter(atoip(DEFAULT_ROUTER));
    }
  }
  if(eth_dns() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "DNS", buf, INI_BUFLEN) == 0)
    {
      eth_setdns(atoip(buf));
    }
    else
    {
      eth_setdns(atoip(DEFAULT_DNS));
    }
  }
  if(eth_ntp() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "NTP", buf, INI_BUFLEN) == 0)
    {
      eth_setntp(atoip(buf));
    }
    else
    {
      eth_setntp(atoip(DEFAULT_NTP));
    }
  }
  if(ini_getentry(SETTINGS_FILE, "TIMEDIFF", buf, INI_BUFLEN) == 0)
  {
    eth_settimediff(atoi(buf));
  }
  else
  {
    eth_settimediff(DEFAULT_TIMEDIFF);
  }
  if(ini_getentry(SETTINGS_FILE, "SUMMER", buf, INI_BUFLEN) == 0)
  {
    eth_setsummer((atoi(buf))?1:0);
  }
  else
  {
    eth_setsummer(DEFAULT_SUMMER);
  }

  return;
}


unsigned int settings_openitem(unsigned int item)
{
  char value[INI_BUFLEN];
  char buf[MAX_ADDR];
  unsigned int abort=1;
  int i;
  IP_Addr ip;

  buf[0]          = 0;
  buf[MAX_ADDR-1] = 0;

  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else
  {
    item--;
    ini_getentry(SETTINGS_FILE, settingsmenu[item].ini, value, INI_BUFLEN);
    buf[0] = 0;
    switch(settingsmenu[item].format)
    {
      case F_NR:  //p1-p2
        i = atoi(value);
        abort = dlg_nr(settingsmenu[item].name, &i, settingsmenu[item].p1, settingsmenu[item].p2, settingsmenu[item].p3);
        if(abort == 0)
        {
          itoa(i, buf, 10);
          if(settingsmenu[item].set)
          {
            settingsmenu[item].set((void*)(int)i);
          }
        }
        break;

      case F_STR: //p1=max len
        abort = dlg_str(settingsmenu[item].name, value, settingsmenu[item].p1, buf, MAX_ADDR);
        if(abort == 0)
        {
          if(settingsmenu[item].set)
          {
            settingsmenu[item].set(buf);
          }
        }
        break;

      case F_MAC:
        abort = dlg_str(settingsmenu[item].name, value, settingsmenu[item].p1, buf, MAX_ADDR);
        if(abort == 0)
        {
          if(settingsmenu[item].set)
          {
            //settingsmenu[item].set((void*)(MAC_Addr)atomac(buf));
          }
        }
        break;

      case F_IP:
        ip = atoip(value);
        abort = dlg_ip(settingsmenu[item].name, &ip);
        if(abort == 0)
        {
          strcpy(buf, iptoa(ip));
          if(settingsmenu[item].set)
          {
            settingsmenu[item].set((void*)(IP_Addr)ip);
          }
        }
        break;

      case F_RGB:
        i = atorgb(value);
        abort = dlg_rgb(settingsmenu[item].name, &i);
        if(abort == 0)
        {
          sprintf(buf, "%03i,%03i,%03i", GET_RED(i), GET_GREEN(i), GET_BLUE(i));
          if(settingsmenu[item].set)
          {
            settingsmenu[item].set((void*)(unsigned int)i);
          }
        }
        break;

      case F_OR:  //p1 or p2
        i = atoi(value);
        abort = dlg_or(settingsmenu[item].name, &i, settingsmenu[item].p1, settingsmenu[item].p2);
        if(abort == 0)
        {
          itoa(i, buf, 10);
          if(settingsmenu[item].set)
          {
            settingsmenu[item].set((void*)(int)i);
          }
        }
        break;

      case F_RUN:
        if(settingsmenu[item].set)
        {
          settingsmenu[item].set(0);
        }
        break;

      case F_TIME:
        menu_drawpopup("Get time...");
        settime(ntp_gettime());
        break;

      case F_INFO:
        snprintf(buf, MAX_ADDR-1, __DATE__" "__TIME__"\n"
                                  "\n"
                                  "MAC %s\n"
                                  "IP   %i.%i.%i.%i\n"
                                  "Mask %i.%i.%i.%i\n"
                                  "Rout %i.%i.%i.%i\n"
                                  "DNS  %i.%i.%i.%i\n"
                                  "NTP  %i.%i.%i.%i",
                 mactoa(eth_mac()),
                 (eth_ip()     >>0)&0xFF, (eth_ip()     >>8)&0xFF, (eth_ip()     >>16)&0xFF, (eth_ip()     >>24)&0xFF,
                 (eth_netmask()>>0)&0xFF, (eth_netmask()>>8)&0xFF, (eth_netmask()>>16)&0xFF, (eth_netmask()>>24)&0xFF,
                 (eth_router() >>0)&0xFF, (eth_router() >>8)&0xFF, (eth_router() >>16)&0xFF, (eth_router() >>24)&0xFF,
                 (eth_dns()    >>0)&0xFF, (eth_dns()    >>8)&0xFF, (eth_dns()    >>16)&0xFF, (eth_dns()    >>24)&0xFF,
                 (eth_ntp()    >>0)&0xFF, (eth_ntp()    >>8)&0xFF, (eth_ntp()    >>16)&0xFF, (eth_ntp()    >>24)&0xFF);
        dlg_msg(APPNAME" v"APPVERSION""APPRELEASE_SYM, buf);
        break;
    }
    if(abort == 0)
    {
      ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, buf);
    }

  }

  return MENU_NOP;
}


void settings_getitem(unsigned int item, char *name)
{
  char buf[INI_BUFLEN];

  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    item--;
    if((settingsmenu[item].ini[0] != 0) &&
       (settingsmenu[item].format != F_NONE))
    {
      if(ini_getentry(SETTINGS_FILE, settingsmenu[item].ini, buf, INI_BUFLEN) == 0)
      {
        snprintf(name, MAX_NAME-1, "%s %s", settingsmenu[item].name, buf);
        name[MAX_NAME-1] = 0;
      }
      else
      {
        strcpy(name, settingsmenu[item].name);
      }
    }
    else
    {
      strcpy(name, settingsmenu[item].name);
    }
  }

  return;
}


unsigned int settings_items(void)
{
  return SETTINGSITEMS+1;
}


void settings_init(void)
{
  DEBUGOUT("Settings: init\n");

  gbuf.card.name[0] = 0;
  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;

  return;
}
