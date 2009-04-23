#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"


unsigned int uuid_test(char *uuid) //uuid: xxxx-xx-xx-xx-xxxxxx
{
  unsigned int i;

  for(i=20; i!=0; i--)
  {
    if(*uuid++ != 0)
    {
      return 1;
    }
  }

  return 0;
}


void uuid_generate(char *uuid) //uuid: xxxx-xx-xx-xx-xxxxxx
{
  unsigned long a, b, c, d, e;

  a = generate_id();
  b = generate_id()>>8;
  c = generate_id()>>16;
  d = generate_id()>>24;
  e = generate_id();

  sprintf(uuid, "%04X-%02X-%02X-%02X-%06X", (a&0xFFFF), (b&0xFF), (c&0xFF), (d&0xFF), (e&0xFFFFFFUL));

  return;
}


unsigned int nbns_decode(char *dst, char *src)
{
  unsigned int i, j;
  char c;

  for(i=0, j=0; i<15; i++)
  {
    c  = (src[j++]-'A')<<4;
    c |= (src[j++]-'A')<<0;
    if(c == ' ')
    {
      break;
    }
    dst[i] = toupper(c);
  }
  dst[i] = 0;

  return (((src[30]-'A')<<4)|(src[31]-'A')); //0x00 = Workstation
}


void nbns_encode(char *dst, char *src, unsigned int type)
{
  char c;
  unsigned int i, j;

  //encode name
  for(i=0, j=0; (i<15) && src[i]; i++)
  {
    c = toupper(src[i]);
    dst[j++] = 'A'+((c>>4)&0x0f);
    dst[j++] = 'A'+((c>>0)&0x0f);
  }

  //add spaces
  for(; i<15; i++)
  {
    dst[j++] = 'A'+((' '>>4)&0x0f);
    dst[j++] = 'A'+((' '>>0)&0x0f);
  }

  //set type (0x00 = Workstation)
  dst[j++] = 'A'+((type>>4)&0x0f);
  dst[j++] = 'A'+((type>>0)&0x0f);

  return;
}


unsigned int http_getparam(char *dst, size_t dst_len, const char *src, const char *param)
{
  src = strstri(src, param);
  if(src)
  {
    src += strlen(param);
    while(*src && (*src==' ')){ src++; } //skip spaces
    if(*src && !isspace(*src))
    {
      for(; dst_len!=0; dst_len--)
      {
        if((*src==0) || (*src=='\n') || (*src=='\r'))
        {
          break;
        }
        *dst++ = *src++;
      }
      *dst = 0;
      return 0;
    }
  }

  return 1;
}


unsigned int http_getresponse(const char *src)
{
  unsigned int search = 16;

  if((strncmpi(src, "ICY", 3)  == 0) ||
     (strncmpi(src, "HTTP", 4) == 0) ||
     (strncmpi(src, "RTSP", 4) == 0))
  {
    while(*src && (*src!=' ') && search){ src++; search--; } //skip proto name
    while(*src && (*src==' ') && search){ src++; search--; } //skip spaces
    if(search)
    {
      return atoi(src);
    }
  }

  return 0;
}


unsigned long generate_id(void)
{
  srand(getontime()+getmstime());

  if(getmstime()&0x01)
  {
    return eth_mac()+rand();
  }
  else
  {
    return eth_mac()-rand();
  }
}


//proto://user:password@xxx.xxx.xxx.xxx/abc
//proto://user:password@domain/abc
void atoaddr(char *s, char *proto, char *user, char* pwrd, IP_Addr *ip, unsigned int *port, char *file)
{
  if(proto){ *proto = 0; }
  if(user) { *user  = 0; }
  if(pwrd) { *pwrd  = 0; }
  if(ip)   { *ip    = 0; }
  if(port) { *port  = 0; }
  if(file) { *file++ = '/'; *file = 0; }

  while(*s && (*s==' ')){ s++; } //skip spaces

  //get proto
  if(strncmpi(s, "ftp://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "ftp");
    }
  }
  else if(strncmpi(s, "http://", 7) == 0)
  {
    s += 7;
    if(proto)
    {
      strcpy(proto, "http");
    }
    *port = 80;
  }
  else if(strncmpi(s, "mms://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "mms");
    }
  }
  else if(strncmpi(s, "rtsp://", 7) == 0)
  {
    s += 7;
    if(proto)
    {
      strcpy(proto, "rtsp");
    }
    *port = 554;
  }
  else if(strncmpi(s, "smb://", 6) == 0)
  {
    s += 6;
    if(proto)
    {
      strcpy(proto, "smb");
    }
  }
  else
  {
    return;
  }

  //get user & password
  if(strstr(s, "@") != 0)
  {
    while(*s && (*s!=':'))
    {
      if(user)
      {
        *user++ = *s;
        *user   = 0;
      }
      s++;
    }
    s++; //skip ":"
    while(*s && (*s!='@'))
    {
      if(pwrd)
      {
        *pwrd++ = *s;
        *pwrd   = 0;
      }
      s++;
    }
    s++; //skip "@"
  }

  //get ip
  if(ip)
  {
    *ip = atoip(s);
  }
  while(*s && (*s!=':') && (*s!='/')){ s++; }

  //get port
  if(*s==':')
  {
    s++; //skip ":"
    if(port)
    {
      *port = atoi(s);
    }
    while(isdigit(*s)){ s++; }; //skip port
  }

  //get file
  if(*s == '/')
  {
    s++; //skip "/"
    while(*s && !isspace(*s))
    {
      if(file)
      {
        *file++ = *s;
        *file   = 0;
      }
      s++;
    }
  }

  return;
}


char* mactoa(MAC_Addr mac_addr) //xx:xx:xx:xx:xx:xx
{
  static char addr[18];
  MAC mac;

  mac.b64 = mac_addr;
  sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X", mac.b8[0], mac.b8[1], mac.b8[2], mac.b8[3], mac.b8[4], mac.b8[5]);

  return addr;
}


MAC_Addr atomac(char *s)
{
  MAC_Addr mac=0;
  uint64_t i;

  i = atoui_hex(s);
  mac |= i<<0;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atoui_hex(s);
  mac |= i<<8;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atoui_hex(s);
  mac |= i<<16;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atoui_hex(s);
  mac |= i<<24;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atoui_hex(s);
  mac |= i<<32;
  while(isxdigit(*s)){ s++; }; while(!isxdigit(*s)){ s++; };
  i = atoui_hex(s);
  mac |= i<<40;

  return mac;
}


char* iptoa(IP_Addr ip_addr) //xxx.xxx.xxx.xxx
{
  static char addr[16];
  IP ip;

  ip.b32 = ip_addr;
  sprintf(addr, "%03i.%03i.%03i.%03i", ip.b8[0], ip.b8[1], ip.b8[2], ip.b8[3]);

  return addr;
}


IP_Addr atoip(char *s)
{
  IP_Addr ip=0;

  if(isdigit(*s)) //ip
  {
    ip |= atoui(s)<<0;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atoui(s)<<8;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atoui(s)<<16;
    while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
    ip |= atoui(s)<<24;
  }
  else //get ip -> dns resolve
  {
    ip = dns_getip(s);
  }

  return ip;
}


unsigned long long swap64(unsigned long long i)
{
  return (((i&0xFF00000000000000ULL)>>56)|
          ((i&0x00FF000000000000ULL)>>40)|
          ((i&0x0000FF0000000000ULL)>>24)|
          ((i&0x000000FF00000000ULL)>> 8)|
          ((i&0x00000000FF000000ULL)<< 8)|
          ((i&0x0000000000FF0000ULL)<<24)|
          ((i&0x000000000000FF00ULL)<<40)|
          ((i&0x00000000000000FFULL)<<56));
}


unsigned long swap32(unsigned long i)
{
  return (((i&0xFF000000UL)>>24)|
          ((i&0x00FF0000UL)>> 8)|
          ((i&0x0000FF00UL)<< 8)|
          ((i&0x000000FFUL)<<24));
}


unsigned int swap16(unsigned int i)
{
  return (((i&0x00FF)<<8)|
          ((i&0xFF00)>>8));
}
