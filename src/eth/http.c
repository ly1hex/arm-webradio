#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "../station.h"
#include "../share.h"
#include "../card.h"
#include "utils.h"
#include "http_files.h"
#include "http.h"


unsigned int http_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len=0;

  DEBUGOUT("HTTP: TCP app\n");

  if(rx_len)
  {
    if(strncmpi(rx, "GET ", 4) == 0)
    {
      rx     += 4;
      rx_len -= 4;
      tx_len  = http_getfile(tx, rx);
    }
    else
    {
      tx_len = sprintf(tx, HTTP_400_HEADER"Error 400 Bad request\r\n\r\n");
    }

    tcp_send(index, tx_len, 0);
    tcp_close(index);
  }

  return tx_len;
}
