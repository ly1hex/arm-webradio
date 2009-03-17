#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "../menu.h"
#include "utils.h"
#include "http.h"
#include "http_files.h"
#include "upnp.h"


#define CONTROL_ACTION_RESP "" \
 "<?xml version='1.0'?>\r\n" \
 "<s:Envelope xmlns:s='http://schemas.xmlsoap.org/soap/envelope/' s:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>\r\n" \
 "  <s:Body>\r\n" \
 "    <u:%sResponse xmlns:u='urn:schemas-upnp-org:service:REMOTE:1'>\r\n" \
 "      <%s>%s</%s>\r\n" \
 "    </u:%sResponse>\r\n" \
 "  </s:Body>\r\n" \
 "</s:Envelope>\r\n"


char upnp_id[20+1] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //uuid: xxxx-xx-xx-xx-xxxxxx


unsigned int upnp_port(void)
{
  return UPNP_PORT;
}


char *upnp_uuid(void)
{
  return upnp_id;
}


unsigned int upnp_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len=0;
  const char *s;
  char tmp[16];

  DEBUGOUT("UPnP: TCP app\n");

  if(rx_len)
  {

    if(strncmpi(rx, "GET ", 4) == 0)
    {
      rx     += 4;
      rx_len -= 4;
      tx_len  = http_getfile(tx, rx);
    }
    else if(strncmpi(rx, "POST /control", 13) == 0)
    {
      rx     += 13;
      rx_len -= 13;
  
      s = strstri(rx, "REMOTE:1#");
      if(s != 0)
      {
        s += 9;
  
        tx_len  = sprintf(tx, HTTP_XML_HEADER);
        tx += tx_len;
  
        if(strncmpi(s, "SETVOLUME", 7) == 0)
        {
          strstrk(tmp, rx, "<s:Envelope\0<s:Body\0<u:SETVOLUME\0<VOLUME>\0\0");
          vs_setvolume(atoi(tmp));
          menu_drawvol();
          sprintf(tmp, "%i", vs_volume());
          tx_len += sprintf(tx, CONTROL_ACTION_RESP, "SETVOLUME", "VOLUME", tmp, "VOLUME", "SETVOLUME");
        }
        else if(strncmpi(s, "GETVOLUME", 7) == 0)
        {
          sprintf(tmp, "%i", vs_volume());
          tx_len += sprintf(tx, CONTROL_ACTION_RESP, "GETVOLUME", "VOLUME", tmp, "VOLUME", "GETVOLUME");
        }
      }
    }
    else
    {
      tx_len = sprintf(tx, HTTP_400_HEADER);
    }

    tcp_send(index, tx_len, 0);
    tcp_close(index);
  }

  return tx_len;
}
