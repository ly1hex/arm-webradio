#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../third_party/fatfs/ff.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../vs.h"
#include "../eth.h"
#include "../menu.h"
#include "../buffer.h"
#include "../station.h"
#include "utils.h"
#include "rtsp.h"


volatile unsigned int rtsp_status=RTSP_CLOSED;
unsigned int rtsp_localport=0, rtsp_pktlen=0;


void rtsp_close(void)
{
  if(rtsp_status != RTSP_CLOSED)
  {
    rtsp_status = RTSP_CLOSE;
  }

  return;
}


unsigned int rtsp_open(void)
{
  long timeout;
  unsigned int index, trying;

  rtsp_status = RTSP_OPEN;
  rtsp_pktlen = 0;
  //calc next local port
  switch(rtsp_localport)
  {
    case RTSP_CLIENTPORT1: rtsp_localport = RTSP_CLIENTPORT2; break;
    case RTSP_CLIENTPORT2: rtsp_localport = RTSP_CLIENTPORT3; break;
    default:               rtsp_localport = RTSP_CLIENTPORT1; break;
  }

  if(gbuf.station.port == 0)
  {
    gbuf.station.port = RTSP_SERVERPORT;
  }

  index   = TCP_ENTRIES;
  index   = tcp_open(index, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, rtsp_localport);
  timeout = getontime()+RTSP_TIMEOUT;
  trying  = RTSP_TRY;
  for(;;)
  {
    eth_service();

    if((rtsp_status == RTSP_CLOSED) || 
       (rtsp_status == RTSP_OPENED) || 
       (rtsp_status == RTSP_ERROR))
    {
      break;
    }
    if(keys_sw() || (ir_cmd() == SW_ENTER))
    {
        rtsp_status = RTSP_ERROR;
        tcp_abort(index);
        break;
    }
    if(getdeltatime(timeout) > 0)
    {
      timeout = getontime()+RTSP_TIMEOUT;
      if(--trying)
      {
        rtsp_status = RTSP_OPEN;
        index = tcp_open(index, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, rtsp_localport);
      }
      else
      {
        rtsp_status = RTSP_CLOSED;
        tcp_abort(index);
        break;
      }
    }
  }

  return rtsp_status;
}


void rtsp_putdata(const unsigned char *s, unsigned int len)
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
        rtsp_close();
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


void rtsp_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
/*  unsigned int tx_len, i;
  static char session[32]={0,};
  static unsigned int seq=0, content_len=0;
  RTSP_Header *rtsp;

  switch(rtsp_status)
  {
    case RTSP_OPENED:
      tcp_send(index, 0, 0); //send ack
      //save audio data
      while(rx_len)
      {
        //read rtsp header
        if(rtsp_pktlen == 0)
        {
          rtsp = (RTSP_Header*) rx;
          if((rx_len >= (RTSP_HEADERLEN+RTP_HEADERLEN+RTPASF_HEADERLEN)) &&
             (rtsp->magic == 0x24))
          {
            rtsp_pktlen = swap16(rtsp->len) - RTP_HEADERLEN - RTPASF_HEADERLEN; //get data len
            //skip header
            rx     += RTSP_HEADERLEN + RTP_HEADERLEN + RTPASF_HEADERLEN;
            rx_len -= RTSP_HEADERLEN + RTP_HEADERLEN + RTPASF_HEADERLEN;
          }
          else
          {
            rx++;
            rx_len--;
            continue;
          }
        }
        //save data
        if(rx_len > rtsp_pktlen)
        {
          rtsp_putdata(rx, rtsp_pktlen);
          rx     += rtsp_pktlen;
          rx_len -= rtsp_pktlen;
          rtsp_pktlen = 0;
        }
        else
        {
          rtsp_putdata(rx, rx_len);
          rtsp_pktlen -= rx_len;
          rx_len = 0;
          break;
        }
      }
      break;

    case RTSP_PLAY:
      if(rx_len)
      {
        if(http_response(rx) == 200) //OK
        {
          rtsp_status = RTSP_OPENED;
          tcp_send(index, 0, 0); //send ack
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(index);
        }
      }
      break;

    case RTSP_SETUP:
      if(rx_len)
      {
        if(http_response(rx) == 200) //OK
        {
          rtsp_status = RTSP_PLAY;

          //get session id
          http_hdparam(session, 32-1, rx, "SESSION:");
          //cut off additional parameters
          for(i=0; isdigit(session[i]) && (i<(32-1)); i++);
          session[i] = 0;
  
          seq++;
          tx_len = sprintf(tx, "PLAY rtsp://%s%s RTSP/1.0\r\n"
                               "CSeq: %i\r\n"
                               "Session: %s\r\n"
                               "Range: npt=0.000-\r\n"
                               "User-Agent: "APPNAME"\r\n"
                               "\r\n", iptoa(gbuf.station.ip), gbuf.station.file, seq, session);
          tcp_send(index, tx_len, 0);
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(index);
        }
      }
      break;

    case RTSP_DESCRIBE2:
      if(rx_len)
      {
        //save ASF header
        i = strlen(vsbuf.b8);
        memcpy((vsbuf.b8+i), rx, rx_len);
        vsbuf.b8[i+rx_len] = 0;

        content_len -= rx_len;
        if(content_len == 0)
        {
          if(strncmpi(vsbuf.b8, "BASE32", 6) == 0)
          {
            //i = base32_decode(vsbuf.b8, (vsbuf.b8+6), VS_BUFSIZE-1);
            i = 0;
          }
          else if(strncmpi(vsbuf.b8, "BASE64", 6) == 0)
          {
            i = base64_decode(vsbuf.b8, (vsbuf.b8+6), VS_BUFSIZE-1);
          }
          else
          {
            i = 0;
          }

          if(i)
          {
            vsbuf_sethead(i);
            DEBUGOUT("RTSP: ASF header: %i\n", i);
  
            rtsp_status = RTSP_SETUP;
            seq++;
            tx_len = sprintf(tx, "SETUP rtsp://%s%s/audio RTSP/1.0\r\n"
                                 "CSeq: %i\r\n"
                                 "Transport: RTP/AVP/TCP;unicast;interleaved=0-1;mode=PLAY\r\n"
                                 "User-Agent: "APPNAME"\r\n"
                                 "\r\n", iptoa(gbuf.station.ip), gbuf.station.file, seq);
            tcp_send(index, tx_len, 0);
          }
          else
          {
            rtsp_status = RTSP_ERROR;
            tcp_abort(index);
          }
        }
        else
        {
          tcp_send(index, 0, 0);
        }
      }
      break;

    case RTSP_DESCRIBE1:
      if(rx_len)
      {
        if(http_response(rx) == 200) //OK
        {
          content_len = http_hdparamcontentlen(rx);
          if(content_len)
          {
            //search SDP start (skip http header)
            rx = http_skiphd(rx, &rx_len);
            //get ASF header
            if(rx_len)
            {
              rtsp_status = RTSP_DESCRIBE2;
              content_len -= rx_len;
              vsbuf_init();
              http_hdparam(vsbuf.b8, VS_BUFSIZE-1, rx, "WMS-HDR.ASFV1;");
            }
          }
          tcp_send(index, 0, 0);
        }
        else
        {
          rtsp_status = RTSP_ERROR;
          tcp_abort(index);
        }
      }
      break;

    case RTSP_GET:
      if(rx_len)
      {
        i = http_response(rx);
        if(i == 200) //OK
        {
          menu_drawpopup("Station: OK");
          rtsp_status = RTSP_DESCRIBE1;
          seq++;
          tx_len = sprintf(tx, "DESCRIBE rtsp://%s%s RTSP/1.0\r\n"
                               "CSeq: %i\r\n"
                               "User-Agent: "APPNAME"\r\n"
                               "\r\n", iptoa(gbuf.station.ip), gbuf.station.file, seq);
          tcp_send(index, tx_len, 0);
        }
        else
        {
          switch(i)
          {
            case 301: //301 Moved Permanently
            case 302: //302 Moved Temporarily
            case 303: //303 See Other
              menu_drawpopup("Station: Addr moved");
              break;
            case 500: //500 Internal Server Error
              menu_drawpopup("Station: Server error");
              break;
            default: //Error
              menu_drawpopup("Station: Error");
              break;
          }
          rtsp_status = RTSP_ERROR;
          tcp_abort(index);
          delay_ms(1000); //for reading popup
        }
      }
      break;

    case RTSP_OPEN:
      rtsp_pktlen = 0;
      rtsp_status = RTSP_GET;
      station_setbitrate(0);
      menu_setformat(FORMAT_WMA);
      seq = 1;
      tx_len = sprintf(tx, "GET rtsp://%s%s RTSP/1.0\r\n"
                           "CSeq: %i\r\n"
                           "User-Agent: "APPNAME"\r\n"
                           "\r\n", iptoa(gbuf.station.ip), gbuf.station.file, seq);
      tcp_send(index, tx_len, 0);
      break;

    case RTSP_CLOSE:
      rtsp_status = RTSP_CLOSED;
      seq++;
      tx_len = sprintf(tx, "TEARDOWN rtsp://%s%s RTSP/1.0\r\n"
                           "CSeq: %i\r\n"
                           "User-Agent: "APPNAME"\r\n"
                           "\r\n", iptoa(gbuf.station.ip), gbuf.station.file, seq);
      tcp_send(index, tx_len, 0);
      tcp_close(index);
      break;

    case RTSP_ERROR:
    case RTSP_CLOSED:
      rtsp_status = RTSP_CLOSED;
      tcp_abort(index);
      tcp_send(index, 0, 0);
      break;
  }
*/
  return;
}
