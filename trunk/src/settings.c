#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include "menu.h"
#include "alarm.h"
#include "settings.h"


#define INI_BUFLEN (64)
void settings_read(void)
{
  char buf[INI_BUFLEN];
  unsigned int bg=DEFAULT_BGCOLOR, fg=DEFAULT_FGCOLOR, sel=DEFAULT_SELCOLOR, edge=DEFAULT_EDGECOLOR;

  //colors
  if(ini_getentry(SETTINGS_FILE, "COLORBG", buf, INI_BUFLEN-1))
  {
    bg = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLORFG", buf, INI_BUFLEN-1))
  {
    fg = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLORSEL", buf, INI_BUFLEN-1))
  {
    sel = atorgb(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "COLOREDGE", buf, INI_BUFLEN-1))
  {
    edge = atorgb(buf);
  }
  menu_setcolors(bg, fg, sel, edge);

  //alarm times
  alarm_load();

  //ir
  if(ini_getentry(SETTINGS_FILE, "IR", buf, INI_BUFLEN-1))
  {
    ir_setaddr(atoi(buf));
  }
  ir_init();

  //vs
  if(ini_getentry(SETTINGS_FILE, "VS", buf, INI_BUFLEN-1))
  {
    vs_init(atoi(buf));
  }
  else
  {
    vs_init(DEFAULT_VS);
  }
  if(ini_getentry(SETTINGS_FILE, "VOLUME", buf, INI_BUFLEN-1))
  {
    vs_setvolume(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "BASSFREQ", buf, INI_BUFLEN-1))
  {
    vs_setbassfreq(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "BASSAMP", buf, INI_BUFLEN-1))
  {
    vs_setbassamp(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "TREBLEFREQ", buf, INI_BUFLEN-1))
  {
    vs_settreblefreq(atoi(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "TREBLEAMP", buf, INI_BUFLEN-1))
  {
    vs_settrebleamp(atoi(buf));
  }

  //ethernet
  if(ini_getentry(SETTINGS_FILE, "NAME", buf, INI_BUFLEN-1))
  {
    eth_setname(buf);
  }
  if(ini_getentry(SETTINGS_FILE, "MAC", buf, INI_BUFLEN-1))
  {
    eth_setmac(atomac(buf));
  }
  if(ini_getentry(SETTINGS_FILE, "DHCP", buf, INI_BUFLEN-1))
  {
    if(atoi(buf))
    {
      eth_setdhcp(1);
    }
  }
  else
  {
    eth_setdhcp(1);
  }
  if(eth_ip() == 0UL)
  {
    if(ini_getentry(SETTINGS_FILE, "IP", buf, INI_BUFLEN-1))
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
    if(ini_getentry(SETTINGS_FILE, "NETMASK", buf, INI_BUFLEN-1))
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
    if(ini_getentry(SETTINGS_FILE, "ROUTER", buf, INI_BUFLEN-1))
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
    if(ini_getentry(SETTINGS_FILE, "DNS", buf, INI_BUFLEN-1))
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
    if(ini_getentry(SETTINGS_FILE, "NTP", buf, INI_BUFLEN-1))
    {
      eth_setntp(atoip(buf));
    }
    else
    {
      eth_setntp(atoip(DEFAULT_NTP));
    }
  }
  if(ini_getentry(SETTINGS_FILE, "TIMEDIFF", buf, INI_BUFLEN-1))
  {
    eth_settimediff(atoi(buf));
  }
  else
  {
    eth_settimediff(DEFAULT_TIMEDIFF);
  }

  return;
}


unsigned int settings_openitem(unsigned int item)
{
  if(item == 0)
  {
    return MENU_BACK;
  }
  else if(item == 7)
  {
    ir_showdata();
  }

  return MENU_UPDATE;
}


/*
General
  Play Mode
  Time
  AutoStart
IR
  IR Addr
  Show raw IR data
Alarm
  Vol
  File
Audio
  Volume
  Bass Freq
  Bass Amp
  Treble Freq
  Treble Amp
Ethernet
  Name
  MAC
  DHCP on/off
  IP
  NetMask
  Router
  DNS
  NTP
  TimeDiff
*/

void settings_getitem(unsigned int item, char *name)
{
  char tmp[MAX_FILE];

  if(item == 0)
  {
    strcpy(name, "<< back <<");
  }
  else
  {
    switch(item)
    {
      case 1:
        strcpy(name, "--- General ---");
        break;
      case 2:
        strcpy(name, "Play Mode");
        break;
      case 3:
        strcpy(name, "Time");
        break;
      case 4:
        ini_getentry(SETTINGS_FILE, "AUTOSTART", tmp, MAX_FILE-1);
        snprintf(name, MAX_NAME-1, "AutoStart %s", tmp);
        break;
      case 5:
        strcpy(name, "--- IR ---");
        break;
      case 6:
        sprintf(name, "IR Addr %i", ir_addr());
        break;
      case 7:
        sprintf(name, "Show raw IR data");
        break;
      case 8:
        strcpy(name, "--- Alarm ---");
        break;
      case 9:
        sprintf(name, "Vol %i", alarm_getvol());
        break;
      case 10:
        alarm_getfile(tmp, 1);
        snprintf(name, MAX_NAME-1, "File1 %s", tmp);
        break;
      case 11:
        alarm_getfile(tmp, 2);
        snprintf(name, MAX_NAME-1, "File2 %s", tmp);
        break;
      case 12:
        alarm_getfile(tmp, 3);
        snprintf(name, MAX_NAME-1, "File3 %s", tmp);
        break;
      case 13:
        strcpy(name, "--- Audio ---");
        break;
      case 14:
        sprintf(name, "Volume %i", vs_volume());
        break;
      case 15:
        sprintf(name, "Bass Freq %i Hz", vs_bassfreq());
        break;
      case 16:
        sprintf(name, "Bass Amp %i dB", vs_bassamp());
        break;
      case 17:
        sprintf(name, "Treble Freq %i Hz", vs_treblefreq());
        break;
      case 18:
        sprintf(name, "Treble Amp %i dB", vs_trebleamp());
        break;
      case 19:
        strcpy(name, "--- Ethernet ---");
        break;
      case 20:
        sprintf(name, "Name %s", eth_name());
        break;
      case 21:
        sprintf(name, "MAC %s", mactoa(eth_mac()));
        break;
      case 22:
        sprintf(name, "DHCP %i", eth_dhcp());
        break;
      case 23:
        sprintf(name, "IP %s", iptoa(eth_ip()));
        break;
      case 24:
        sprintf(name, "NM %s", iptoa(eth_netmask()));
        break;
      case 25:
        sprintf(name, "ROUT %s", iptoa(eth_router()));
        break;
      case 26:
        sprintf(name, "DNS %s", iptoa(eth_dns()));
        break;
      case 27:
        sprintf(name, "NTP %s", iptoa(eth_ntp()));
        break;
      case 28:
        sprintf(name, "Time Diff %i", eth_timediff());
        break;
    }
  }

  return;
}


unsigned int settings_items(void)
{
  return 28+1;
}
