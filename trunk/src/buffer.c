#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "third_party/fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "eth.h"
#include "buffer.h"


BUFFER gbuf;
VSBUFFER vsbuf;
volatile unsigned int vsbuf_head=0, vsbuf_tail=0;


unsigned int vsbuf_free(void)
{
  unsigned int head, tail;

  head = vsbuf_head;
  tail = vsbuf_tail;

  if(head > tail)
  {
    return (VS_BUFSIZE-(head-tail))-1;
  }
  else if(head < tail)
  {
    return (tail-head)-1;
  }

  return (VS_BUFSIZE-1);
}


unsigned int vsbuf_len(void)
{
  unsigned int head, tail;

  head = vsbuf_head;
  tail = vsbuf_tail;

  if(head > tail)
  {
    return (head-tail);
  }
  else if(head < tail)
  {
    return (VS_BUFSIZE-(tail-head));
  }

  return 0;
}


void vsbuf_puts(const unsigned char *s, unsigned int len)
{
  unsigned int head;

  head = vsbuf_head;
  while(len--)
  {
    vsbuf.b8[head++] = *s++;
    if(head >= VS_BUFSIZE)
    {
      head = 0;
    }
  }
  vsbuf_head = head;

  return;
}


void vsbuf_putc(unsigned char c)
{
  unsigned int head;

  head = vsbuf_head;
  vsbuf.b8[head++] = c;
  if(head >= VS_BUFSIZE)
  {
    head = 0;
  }
  vsbuf_head = head;

  return;
}


void vsbuf_gets(unsigned char *s, unsigned int len)
{
  while(len--)
  {
    *s++ = vsbuf_getc();
  }

  return;
}


unsigned char vsbuf_getc(void)
{
  unsigned char c;
  unsigned int head, tail;

  head = vsbuf_head;
  tail = vsbuf_tail;
  if(head != tail)
  {
    c = vsbuf.b8[tail++];
    if(tail >= VS_BUFSIZE)
    {
      tail = 0;
    }
    vsbuf_tail = tail;
  }
  else
  {
    c = 0;
  }

  return c;
}


void vsbuf_sethead(unsigned int head)
{
  vsbuf_head = head;

  return;
}


void vsbuf_init(void)
{
  vsbuf_head = 0;
  vsbuf_tail = 0;

  return;
}
