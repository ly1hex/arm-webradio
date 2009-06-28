#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lcd.h"
#include "tools.h"


void strshrinkpath(char *path)
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


char* strrmvspace(char *dst, const char *src)
{
  unsigned int i;

  if(*src == 0)
  {
    *dst = 0;
  }
  else
  {
    //at start
    for(i=0; isspace(src[i]); i++);
    strcpy(dst, &src[i]);
  
    //at end
    i=strlen(dst)-1;
    for(i=strlen(dst)-1; isspace(dst[i]); i--)
    {
      dst[i] = 0;
    }
  }

  return dst;
}


char* strtoupper(char *dst, const char *src)
{
  char c;

  while(*src)
  {
    c    = *src++;
    *dst++ = toupper(c);
  }
  *dst = 0;

  return dst;
}


int strstrk(char *dst, const char *src, const char *key) //key = "first\0second\third\0\0"
{
  int fnd=0;
  const char *k, *s;
  char c;

  *dst = 0;

  s = src;
  k = key;
  for(;;)
  {
    s = strstri(s, k);
    if(s != 0)
    {
      s += strlen(k);
      while(*s && (*s ==  ' ')){ s++; } //skip spaces
      while(*k){ k++; } k++; //get next key
      if(*k == 0) //last key
      {
        while(*s)
        {
          c = *s++;
          if((c == '<')  ||
             (c == '>')  ||
             (isspace(c)))
          {
            break;
          }
          else
          {
            *dst++ = c;
          }
        }
        *dst = 0;
        fnd  = 1;
        break;
      }
    }
    else
    {
      break;
    }
  }

  return fnd;
}


const char* strstri(const char *s, const char *pattern)
{
  size_t len;

  if(*pattern)
  {
    len = strlen(pattern);
    while(*s)
    {
      if(strncmpi(s, pattern, len) == 0)
      {
        return s;
      }
      s++;
    }
  }

  return 0;
}


int strncmpi(const char *s1, const char *s2, size_t n)
{
  unsigned char c1, c2;

  if(n == 0)
  {
    return 0;
  }
  
  do
  {
    c1 = tolower(*s1++);
    c2 = tolower(*s2++);
    if((c1 == 0) || (c2 == 0))
    {
      break;
    }
  }
  while((c1 == c2) && --n);
  
  return c1-c2;
}


void uitoa(unsigned long n, char *str)
{
  char *ptr, c;

  ptr = str;

  //generate digits in reverse order
  do
  {
    *ptr++ = '0' + n%10;
    n /= 10;
  }while(n > 0); 
  *ptr = 0;

  //reverse
  ptr--;
  while(ptr>str)
  {
   c = *ptr;
   *ptr-- = *str;
   *str++ = c;
  }

  return;
}


unsigned long atoui_hex(const char *s)
{
  unsigned long value=0;

  if(!s)
  {
    return 0;
  }  

  while(*s)
  {
    if((*s >= '0') && (*s <= '9'))
    {
      value = (value*16) + (*s-'0');
    }
    else if((*s >= 'A') && ( *s <= 'F'))
    {
      value = (value*16) + (*s-'A'+10);
    }
    else if((*s >= 'a') && (*s <= 'f'))
    {
      value = (value*16) + (*s-'a'+10);
    }
    else
    {
      break;
    }
    s++;
  }

  return value;
}


unsigned long atoui(const char *s)
{
  unsigned long value=0;

  if(!s)
  {
    return 0;
  }  

  while(*s)
  {
    if((*s >= '0') && (*s <= '9'))
    {
      value = (value*10) + (*s-'0');
    }
    else
    {
      break;
    }
    s++;
  }

  return value;
}


unsigned int atorgb(const char *s)
{
  unsigned int r, g, b;

  while(!isdigit(*s)){ s++; };
  r = atoi(s);
  while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
  g = atoi(s);
  while(isdigit(*s)){ s++; }; while(!isdigit(*s)){ s++; };
  b = atoi(s);

  return RGB(r,g,b);
}


void sectotime(unsigned long s, TIME *time) //seconds from 1970
{
  unsigned long t;
  const unsigned int month_start[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
                                      //31 28  31  30   31   30   31   31   30   31   30   31

  //seconds
  t           = s / 60;
  time->s     = s - (t*60);
  s           = t;

  //minutes
  t           = s / 60;
  time->m     = s - (t*60);
  s           = t;

  //hours
  t           = s / 24;
  time->h     = s - (t*24);
  s           = t;

  //day of week
  time->wday   = (s+4) % 7; //day 0 was thursday

  //leap years since 1968
  s += 366 + 365;
  t = s / ((4*365)+1);
  if((s - (t * ((4*365)+1))) > (31+29))
  {
      t++;
  }

  //year
  time->year = ((s-t) / 365) + 1968;
  s -= ((time->year-1968) * 365) + t;

  //month
  for(t=0; t<12; t++)
  {
      if(month_start[t] > s)
      {
          break;
      }
  }
  time->month = t;


  //day of month
  time->day = (s-month_start[t-1]) + 1;

  return;
}
