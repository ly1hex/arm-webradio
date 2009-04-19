#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../third_party/fatfs/ff.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../eth.h"
#include "../menu.h"
#include "../buffer.h"
#include "../station.h"
#include "utils.h"
#include "shoutcast.h"


volatile unsigned int shoutcast_status=SHOUTCAST_CLOSED;
IP_Addr shoutcast_ip=0;
char *shoutcast_url;


unsigned int shoutcast_getstatus(void)
{
  return shoutcast_status;
}


void shoutcast_close(void)
{
  if(shoutcast_status != SHOUTCAST_CLOSED)
  {
    shoutcast_status = SHOUTCAST_CLOSE;
  }

  return;
}


unsigned int shoutcast_open(IP_Addr ip, unsigned int port, char *url)
{
  long timeout;
  unsigned int index, trying;
  MAC_Addr mac;

  mac = arp_getmac(ip);
  if(mac == 0)
  {
    return SHOUTCAST_CLOSED;
  }

  shoutcast_status = SHOUTCAST_OPEN;
  shoutcast_ip     = ip;
  shoutcast_url    = url;

  index   = TCP_ENTRIES;
  index   = tcp_open(index, mac, ip, port, SHOUTCAST_PORT);
  timeout = getontime()+SHOUTCAST_TIMEOUT;
  trying  = SHOUTCAST_TRY;
  for(;;)
  {
    eth_service();

    if((shoutcast_status == SHOUTCAST_CLOSED) || 
       (shoutcast_status == SHOUTCAST_OPENED) || 
       (shoutcast_status == SHOUTCAST_ERROR)  ||
       (shoutcast_status == SHOUTCAST_SERVERFULL))
    {
      break;
    }
    if(keys_sw() || (ir_cmd() == SW_ENTER))
    {
        shoutcast_status = SHOUTCAST_ERROR;
        tcp_abort(index);
        break;
    }
    if(getdeltatime(timeout) > 0)
    {
      timeout = getontime()+SHOUTCAST_TIMEOUT;
      if(--trying)
      {
        shoutcast_status = SHOUTCAST_OPEN;
        index = tcp_open(index, mac, ip, port, SHOUTCAST_PORT);
      }
      else
      {
        shoutcast_status = SHOUTCAST_CLOSED;
        tcp_abort(index);
        break;
      }
    }
  }

  return shoutcast_status;
}


//Filter Ogg headers
//It is one whole stream. So we only need the Ogg header at stream start.
//Otherwise there are sometimes problems with duplicate Ogg headers.
#define WORKINGBUF (64)
void shoutcast_putogg(const unsigned char *s, unsigned int len)
{
  static unsigned int state=0;
  long timeout;
  unsigned char c, buf[WORKINGBUF];
  unsigned int buf_len=0;

  timeout = getontime()+2;

  //OggS 0x00 0x0x (0x02=1st, 0x04=last)
  while(len--)
  {
    c = *s++;
    if(c == 'O')
    {
      state = 1;
    }
    else
    {
      switch(state)
      {
        case 1: if(c == 'g') { state=2; }
                else         { state=0; } break;
        case 2: if(c == 'g') { state=3; }
                else         { state=0; } break;
        case 3: if(c == 'S') { state=4; }
                else         { state=0; } break;
        case 4: if(c == 0x00){ state=5; }
                else         { state=0; } break;
        case 5:
          state=0;
          if(c & 0x06) //0x04 || 0x02
          {
            c = 0x00; //0x00 modify header
          }
          break;
      }
    }
    buf[buf_len++] = c;
    if(buf_len == WORKINGBUF)
    {
      buf_len = 0;
      while(32 > vsbuf_free()) //wait for free buffer
      {
        if(getdeltatime(timeout) > 0)
        {
          shoutcast_close();
          return;;
        }
      }
      vsbuf_puts(buf, WORKINGBUF);
    }
  }
  vsbuf_puts(buf, buf_len);

  return;
}


void shoutcast_putdata(const unsigned char *s, unsigned int len)
{
  unsigned int free;
  long timeout;

  timeout = getontime()+2;
  while(len)
  {
    free = vsbuf_free();
    if(free < len)
    {
      vsbuf_puts(s, free);
      s   += free;
      len -= free;
      if(getdeltatime(timeout) > 0)
      {
        shoutcast_close();
        break;
      }
    }
    else
    {
      vsbuf_puts(s, len);
      break;
    }
  }

  return;
}


unsigned int shoutcast_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len, i;
  static unsigned int parse_header=0, skip=0, ogg_data=0;

  switch(shoutcast_status)
  {
    case SHOUTCAST_OPENED:
      tcp_send(index, 0, 0); //send ack
      if(ogg_data) //ogg stream
      {
        shoutcast_putogg(rx, rx_len); //save data
      }
      else
      {
        shoutcast_putdata(rx, rx_len); //save data
      }
      break;

    case SHOUTCAST_HEADER:
      if(rx_len)
      {
        //search http header
        if(parse_header == 0)
        {
          i = http_getresponse(rx);
          if(i == 200) //200 OK
          {
            parse_header = 1;
            menu_drawpopup("Station: OK");
          }
          else
          {
            switch(i)
            {
              case 301: //301 Moved Permanently
              case 302: //302 Moved Temporarily
              case 303: //303 See Other
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Addr moved");
                break;
              case 400: //400 Server full
                shoutcast_status = SHOUTCAST_SERVERFULL;
                menu_drawpopup("Station: Server full");
                break;
              case 404: //404 Resource not found
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Not found");
                break;
              case 500: //500 Internal Server Error
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Server error");
                break;
              default: //Error
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Error");
                break;
            }
            parse_header = 0;
            rx_len       = 0;
            tcp_abort(index);
            delay_ms(1000);
          }
        }

        //search http header end
        if(parse_header == 1) //get http header
        {
          if(http_getparam(gbuf.menu.info, MAX_INFO-1, rx, "ICY-BR:") == 0) //get stream bitrate
          {
            i = atoi(gbuf.menu.info);
            if(i >= 128) //bitrate >= 128
            {
              skip = 64; //skip first 64 frames
            }
            else if(i >= 64) //bitrate >= 64
            {
              skip = 32; //skip first 32 frames
            }
            else if(i >= 8)
            {
              skip = 0; //skip nothing
            }
            station_calcbuf(i);
          }
          http_getparam(gbuf.menu.info, MAX_INFO-1, rx, "ICY-NAME:"); //get stream name
          for(; rx_len!=0; rx_len--, rx++)
          {
            if((rx[0] == '\r') && (rx[1] == '\n') && (rx[2] == '\r') && (rx[3] == '\n'))
            {
              rx     += 4;
              rx_len -= 4;
              parse_header = 2;
              break;
            }
          }
        }

        //search audio header and get audio data
        if(parse_header >= 2)
        {
          //Ogg streams start with a special frame with decoding infos
          if(ogg_data == 0)
          {
            if((rx_len >= 4)  &&
               (rx[0] == 'O') && 
               (rx[1] == 'g') &&
               (rx[2] == 'g') &&
               (rx[3] == 'S')) //Ogg sync: OggS
            {
              DEBUGOUT("Shoutcast: Ogg header\n");
              vsbuf_puts(rx, rx_len);
              ogg_data = rx_len;
              rx_len   = 0;
            }
          }
          else if(ogg_data < 8000)
          {
            if(ogg_data < 4000) //Ogg header is around 4-5 kByte
            {
              vsbuf_puts(rx, rx_len);
              ogg_data += rx_len;
              rx_len    = 0;
            }
            else
            {
              for(i=0; i<rx_len; i++) //search Ogg sync
              {
                if(((rx_len-i) >= 4) &&
                    (rx[i+0] == 'O') && 
                    (rx[i+1] == 'g') &&
                    (rx[i+2] == 'g') &&
                    (rx[i+3] == 'S')) //Ogg sync: OggS
                {
                  ogg_data = 8000;
                  break;
                }
              }
              vsbuf_puts(rx, i);
              ogg_data += i;
              rx_len   -= i;
            }
          }

          //skip the first x frames
          if(parse_header >= (2+skip))
          {
            if(ogg_data) //search Ogg sync
            {
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 4)  &&
                   (rx[0] == 'O') && 
                   (rx[1] == 'g') &&
                   (rx[2] == 'g') &&
                   (rx[3] == 'S')) //Ogg sync: OggS
                {
                  break;
                }
              }
            }
            else //search MPEG sync
            {
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 2)         &&
                        (rx[0]       == 0xFF) && 
                       ((rx[1]&0xE0) == 0xE0)) //MPEG sync: 0xFFE
                {
                  break;
                }
              }
            }
            if(rx_len)
            {
              vsbuf_puts(rx, rx_len);
              shoutcast_status = SHOUTCAST_OPENED;
            }
          }
          else
          {
            parse_header++;
          }
        }
        //send ack
        tcp_send(index, 0, 0);
      }
      break;

    case SHOUTCAST_OPEN:
      tx_len = sprintf(tx, "GET %s HTTP/1.0\r\n"
                           "Host: %i.%i.%i.%i\r\n"
                           "User-Agent: "APPNAME"\r\n"
                           "Icy-MetaData: 0\r\n"
                           "Connection: Keep-Alive\r\n"
                           "\r\n", shoutcast_url, ((shoutcast_ip>>0)&0xff), ((shoutcast_ip>>8)&0xff), ((shoutcast_ip>>16)&0xff), ((shoutcast_ip>>24)&0xff));
      shoutcast_status = SHOUTCAST_HEADER;
      parse_header     = 0;
      skip             = 32; //default skip
      ogg_data         = 0;
      station_calcbuf(0);
      tcp_send(index, tx_len, 0);
      break;

    case SHOUTCAST_ERROR:
    case SHOUTCAST_SERVERFULL:
    case SHOUTCAST_CLOSE:
    case SHOUTCAST_CLOSED:
      shoutcast_status = SHOUTCAST_CLOSED;
      tcp_abort(index);
      tcp_send(index, 0, 0);
      break;
  }

  return tx_len;
}
