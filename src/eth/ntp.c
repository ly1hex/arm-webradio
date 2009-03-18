#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "ntp.h"


volatile unsigned long ntp_time=0UL;


unsigned int ntp_request(unsigned int index)
{
  MAC_Addr mac;
  NTP_Header *tx_ntp;

  mac = arp_getmac(eth_ntp());

  tx_ntp = (NTP_Header*) &eth_txbuf[NTP_OFFSET];

  memset(tx_ntp, 0, NTP_HEADERLEN);

  tx_ntp->flags = (0<<6)|(1<<3)|(3<<0); //LI=0 | VN=1 | Mode=3 -> Client

  index = udp_open(index, mac, eth_ntp(), NTP_PORT, NTP_PORT, 0, NTP_HEADERLEN);

  return index;
}


unsigned long ntp_gettime(void)
{
  long timeout, timeout_ntp;
  unsigned int index;

  ntp_time = 0UL;

  index = ntp_request(UDP_ENTRIES);

  timeout     = getontime()+ETH_TIMEOUT;
  timeout_ntp = getontime()+NTP_TIMEOUT;
  for(;;)
  {
    eth_service();

    if(ntp_time != 0UL)
    {
      break;
    }
    if(getdeltatime(timeout_ntp) > 0)
    {
      timeout_ntp = getontime()+NTP_TIMEOUT;
      index = ntp_request(index);
    }
    if(getdeltatime(timeout) > 0)
    {
      break;
    }
  }

  udp_close(index);

  return ntp_time;
}


void ntp_udpapp(unsigned int index)
{
  NTP_Header *rx_ntp;
  unsigned long time;

  DEBUGOUT("NTP: UDP app\n");
  
  rx_ntp = (NTP_Header*) &eth_rxbuf[NTP_OFFSET];

  if((rx_ntp->flags&0x07) == 4) //Mode=4 -> Server
  {
    time  = swap32(rx_ntp->trn_ts);
    time -= 2208988800UL; //seconds: 1900-1970
    time += eth_timediff();
    ntp_time = time;
  }

  return;
}