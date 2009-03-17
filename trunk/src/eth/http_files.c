#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "upnp.h"
#include "http.h"
#include "http_files.h"


const unsigned char INDEX_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" "APPVERSION"</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  " <pre>\r\n" \
  " general\r\n" \
  "  ontime: $ONTIME sec\r\n" \
  "  date:   $DATE\r\n" \
  "  clock:  $CLOCK\r\n" \
  " eth\r\n" \
  "  name:   $NAME\r\n" \
  "  mac:    $MAC\r\n" \
  "  dhcp:   $DHCP\r\n" \
  "  ip:     $IP\r\n" \
  "  nmask:  $NETMASK\r\n" \
  "  rout:   $ROUTER\r\n" \
  "  dns:    $DNS\r\n" \
  "  ntp:    $NTP\r\n" \
  "  tdiff:  $TDIFF sec\r\n" \
  " upnp\r\n" \
  "  uuid:   $UPNP_UUID\r\n" \
  "  port:   $UPNP_PORT\r\n" \
  " </pre>\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char STATION_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" - Station</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char SHARE_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" - Share</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char CARD_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" - Card</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char ALARM_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" - Alarm</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char SETTINGS_HTM[] =
{
  "<html>\r\n" \
  "<head>\r\n" \
  " <title>"APPNAME"</title>\r\n" \
  " <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n" \
  " <link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\r\n" \
  "</head>\r\n" \
  "<body>\r\n" \
  " <table id=\"maintable\" width=\"100%\">\r\n" \
  "  <tr><td id=\"head\"><b>"APPNAME" - Settings</b><br><a href=\"/\">Home</a> - <a href=\"/station.htm\">Station</a> - <a href=\"/share.htm\">Share</a> - <a href=\"/card.htm\">Card</a> - <a href=\"/alarm.htm\">Alarm</a> - <a href=\"/settings.htm\">Settings</a></td></tr>\r\n" \
  "  <tr><td id=\"content\">\r\n" \

  "  </td></tr>\r\n" \
  "  <tr><td id=\"footer\"><a href=\"http://www.watterott.net\">www.watterott.net</a></td></tr>\r\n" \
  " </table>\r\n" \
  "</body>\r\n" \
  "</html>\r\n" \
  "\r\n\r\n"
};


const unsigned char STYLE_CSS[] =
{
  "html, body {\r\n" \
  " border:none;\r\n" \
  " width:100%;\r\n" \
  " height:100%;\r\n" \
  " padding:0px;\r\n" \
  " margin:0px;\r\n" \
  " background-color:#808080;\r\n" \
  " color:#000000; }\r\n" \
  "body {\r\n" \
  " font-family:arial, helvetica, sans-serif;\r\n" \
  " font-size:0.9em; }\r\n" \
  "pre, ul, li, table, td, th {\r\n" \
  " font-size:100%; }\r\n" \
  "h1 {\r\n" \
  " font-size:130%;\r\n" \
  " font-weight:bold; }\r\n" \
  "h2 {\r\n" \
  " font-size:120%;\r\n" \
  " font-weight:bold; }\r\n" \
  "h3 {\r\n" \
  " font-size:105%;\r\n" \
  " font-weight:bold; }\r\n" \
  "h4 {\r\n" \
  " font-size:102%;\r\n" \
  " font-weight:normal; }\r\n" \
  "a, a:link {\r\n" \
  " color:#0000BB;\r\n" \
  " font-size:100%;\r\n" \
  " text-decoration:none; }\r\n" \
  "a:hover {\r\n" \
  " color:#F00000;\r\n" \
  " text-decoration:underline; }\r\n" \
  "#maintable {\r\n" \
  " border:none;\r\n" \
  " border-spacing:0px;\r\n" \
  " height:100%;\r\n" \
  " width:100%;\r\n" \
  " padding:20px;\r\n" \
  " margin:0px;\r\n" \
  " background-color:#808080;\r\n" \
  " color:#000000; }\r\n" \
  "#head {\r\n" \
  " border-bottom:2px solid #808080;\r\n" \
  " height:40px;\r\n" \
  " padding:4px;\r\n" \
  " margin:0px;\r\n" \
  " vertical-align:top;\r\n" \
  " text-align:center;\r\n" \
  " background-color:#F0F2F4;\r\n" \
  " color:#808080; }\r\n" \
  "#content {\r\n" \
  " padding:20px;\r\n" \
  " margin:0px;\r\n" \
  " background-color:#F0F2F4;\r\n" \
  " vertical-align:top;\r\n" \
  " color:#000000; }\r\n" \
  "#footer {\r\n" \
  " border-top:2px solid #808080;\r\n" \
  " height:20px;\r\n" \
  " padding:4px;\r\n" \
  " margin:0px;\r\n" \
  " vertical-align:bottom;\r\n" \
  " text-align:center;\r\n" \
  " background-color:#F0F2F4;\r\n" \
  " color:#808080; }\r\n" \
  "#footer a, #footer a:link {\r\n" \
  " color:#808080;\r\n" \
  " text-decoration:none; }\r\n" \
  "#footer a:hover {\r\n" \
  " color:#808080;\r\n" \
  " text-decoration:underline; }\r\n" \
  "\r\n\r\n"
};


const unsigned char FAVICON_ICO[] =
{
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x61, 0xFE,
  0x00, 0x00, 0x4C, 0xAA, 0x00, 0x00, 0x52, 0xFE, 0x00, 0x00, 0x52, 0xC6,
  0x00, 0x00, 0x4C, 0xBA, 0x00, 0x00, 0x61, 0xC6, 0x00, 0x00, 0x7F, 0xFE,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xEF, 0xF7,
  0x00, 0x00, 0xF0, 0x07, 0x00, 0x00, 0xFF, 0xF3, 0x00, 0x00, 0xFF, 0xFB,
  0x00, 0x00, 0xFF, 0xF9, 0x00, 0x00
};


const unsigned char DEVICE_XML[] =
{
  "<?xml version=\"1.0\"?>\r\n" \
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n" \
  "  <specVersion>\r\n" \
  "    <major>1</major>\r\n" \
  "    <minor>0</minor>\r\n" \
  "  </specVersion>\r\n" \
  "  <URLBase>http://$IP:$UPNP_PORT</URLBase>\r\n" \
  "  <device>\r\n" \
  "    <deviceType>urn:schemas-upnp-org:device:"APPNAME":1</deviceType>\r\n" \
  "    <friendlyName>$NAME</friendlyName>\r\n" \
  "    <modelName>"APPNAME" v"APPVERSION"</modelName>\r\n" \
  "    <UDN>uuid:$UPNP_UUID</UDN>\r\n" \
  "    <serviceList>\r\n" \
  "      <service>\r\n" \
  "        <serviceType>urn:schemas-upnp-org:service:REMOTE:1</serviceType>\r\n" \
  "        <serviceId>urn:upnp-org:serviceId:REMOTE</serviceId>\r\n" \
  "        <SCPDURL>/service.xml</SCPDURL>\r\n" \
  "        <controlURL>/control</controlURL>\r\n" \
  "        <eventSubURL>/event</eventSubURL>\r\n" \
  "      </service>\r\n" \
  "    </serviceList>\r\n" \
  "  </device>\r\n" \
  "</root>\r\n" \
  "\r\n\r\n"
};


const unsigned char SERVICE_XML[] =
{
  "<?xml version=\"1.0\"?>\r\n" \
  "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n" \
  "  <specVersion>\r\n" \
  "    <major>1</major>\r\n" \
  "    <minor>0</minor>\r\n" \
  "  </specVersion>\r\n" \
  "  <actionList>\r\n" \
  "    <action>\r\n" \
  "      <name>SETVOLUME</name>\r\n" \
  "      <argumentList>\r\n" \
  "        <argument>\r\n" \
  "          <name>VOLUME</name>\r\n" \
  "          <relatedStateVariable>VOLUME</relatedStateVariable>\r\n" \
  "          <direction>in</direction>\r\n" \
  "        </argument>\r\n" \
  "     </argumentList>\r\n" \
  "    </action>\r\n" \
  "    <action>\r\n" \
  "      <name>GETVOLUME</name>\r\n" \
  "      <argumentList>\r\n" \
  "         <argument>\r\n" \
  "          <name>VOLUME</name>\r\n" \
  "          <relatedStateVariable>VOLUME</relatedStateVariable>\r\n" \
  "          <direction>out</direction>\r\n" \
  "        </argument>\r\n" \
  "       </argumentList>\r\n" \
  "    </action>\r\n" \
  "  </actionList>\r\n" \
  "  <serviceStateTable>\r\n" \
  "    <stateVariable sendEvents=\"no\">\r\n" \
  "      <name>VOLUME</name>\r\n" \
  "      <dataType>ui1</dataType>\r\n" \
  "      <allowedValueRange>\r\n" \
  "        <minimum>0</minimum>\r\n" \
  "        <maximum>100</maximum>\r\n" \
  "      </allowedValueRange>\r\n" \
  "    </stateVariable>\r\n" \
  "  </serviceStateTable>\r\n" \
  "</scpd>\r\n" \
  "\r\n\r\n"
};


#define HTTPFILES (11)
const HTTPFILE httpfiles[HTTPFILES] = 
{
  {"/ ",             HTML_FILE, INDEX_HTM,    sizeof(INDEX_HTM)},
  {"/index.htm ",    HTML_FILE, INDEX_HTM,    sizeof(INDEX_HTM)},
  {"/station.htm ",  HTML_FILE, STATION_HTM,  sizeof(STATION_HTM)},
  {"/share.htm ",    HTML_FILE, SHARE_HTM,    sizeof(SHARE_HTM)},
  {"/card.htm ",     HTML_FILE, CARD_HTM,     sizeof(CARD_HTM)},
  {"/alarm.htm ",    HTML_FILE, ALARM_HTM,    sizeof(ALARM_HTM)},
  {"/settings.htm ", HTML_FILE, SETTINGS_HTM, sizeof(SETTINGS_HTM)},
  {"/style.css ",    CSS_FILE,  STYLE_CSS,    sizeof(STYLE_CSS)},
  {"/favicon.ico ",  ICON_FILE, FAVICON_ICO,  sizeof(FAVICON_ICO)},
  {"/device.xml ",   XML_FILE,  DEVICE_XML,   sizeof(DEVICE_XML)},
  {"/service.xml ",  XML_FILE,  SERVICE_XML,  sizeof(SERVICE_XML)}
};


#define F_NR     (0)
#define F_STR    (1)
#define F_MAC    (2)
#define F_IP     (3)
#define F_ONOFF  (4)
#define HTTPVARS (14)
const HTTPVAR httpvars[HTTPVARS] =
{
  {"ONTIME",    F_NR,    (void*)getontime},
  {"DATE",      F_STR,   (void*)getdate},
  {"CLOCK",     F_STR,   (void*)getclock},
  {"NAME",      F_STR,   (void*)eth_name},
  {"MAC",       F_MAC,   (void*)eth_mac},
  {"DHCP",      F_ONOFF, (void*)eth_dhcp},
  {"IP",        F_IP,    (void*)eth_ip},
  {"NETMASK",   F_IP,    (void*)eth_netmask},
  {"ROUTER",    F_IP,    (void*)eth_router},
  {"DNS",       F_IP,    (void*)eth_dns},
  {"NTP",       F_IP,    (void*)eth_ntp},
  {"TDIFF",     F_NR,    (void*)eth_timediff},
  {"UPNP_UUID", F_STR,   (void*)upnp_uuid},
  {"UPNP_PORT", F_NR,    (void*)upnp_port}
};


unsigned int http_printf(char *dst, unsigned int format, ...)
{
  va_list ap;

  va_start(ap, format);

  switch(format)
  {
    case F_NR:
      itoa(va_arg(ap, long), dst, 10);
      break;
    case F_STR:
      strcpy(dst, va_arg(ap, char *));
      break;
    case F_MAC:
      strcpy(dst, mactoa(va_arg(ap, MAC_Addr)));
      break;
    case F_IP:
      strcpy(dst, iptoa(va_arg(ap, IP_Addr)));
      break;
    case F_ONOFF:
      if(va_arg(ap, int))
      {
        strcpy(dst, "on");
      }
      else
      {
        strcpy(dst, "off");
      }
      break;
  }

  va_end(ap);

  return strlen(dst);
}


unsigned int http_parse(char *dst, const char *src)
{
  unsigned int i, l, len=0;
  char c, str[16];

  for(;;)
  {
    c = *src++;

    if(c == 0)
    {
      break;
    }
    else if(c == '$')
    {
      for(i=0; i<HTTPVARS; i++)
      {
        l = strlen(httpvars[i].name);
        if(strncmp(src, httpvars[i].name, l) == 0)
        {
          src += l;
          i = http_printf(dst, httpvars[i].format, httpvars[i].func());
          dst += i;
          len += i;
          i    = 0xff;
          break;
        }
      }
      if(i == 0xff)
      {
        continue;
      }
    }
    *dst++ = c;
    len++;
  }

  return len;
}


unsigned int http_getfile(unsigned char *dst, const char *name)
{
  unsigned int i, file, len=0;
  char tmp[8];

  while(*name && (*name == ' ')){ name++; } //skip spaces

  for(file=0; file<HTTPFILES; file++)
  {
    if(strncmpi(name, httpfiles[file].name, strlen(httpfiles[file].name)) == 0)
    {
      i = 0;
      switch(httpfiles[file].type)
      {
        case HTML_FILE:
          strcpy(dst, HTTP_HTML_HEADER);
          len = strlen(HTTP_HTML_HEADER);
          dst += len;
          i = http_parse(dst, httpfiles[file].data);
          break;

        case XML_FILE:
          strcpy(dst, HTTP_XML_HEADER);
          len = strlen(HTTP_XML_HEADER);
          dst += len;
          i = http_parse(dst, httpfiles[file].data);
          break;

        case CSS_FILE:
          strcpy(dst, HTTP_CSS_HEADER);
          len = strlen(HTTP_CSS_HEADER);
          dst += len;
          memcpy(dst, httpfiles[file].data, httpfiles[file].len);
          i = httpfiles[file].len;
          break;

        case ICON_FILE:
          strcpy(dst, HTTP_ICON_HEADER);
          len = strlen(HTTP_ICON_HEADER);
          dst += len;
          memcpy(dst, httpfiles[file].data, httpfiles[file].len);
          i = httpfiles[file].len;
          break;
      }
      if(i) //file found -> set content length
      {
        len += i;
        itoa(i, tmp, 10);
        dst -= 10;
        for(i=0; tmp[i]!=0; i++)
        {
          *dst++ = tmp[i];
        }
      }
      break;
    }
  }

  if((file >= HTTPFILES) || (len == 0)) //file not found
  {
    len = sprintf(dst, HTTP_404_HEADER"Error 404 Not found\r\n\r\n");
  }

  return len;
}
