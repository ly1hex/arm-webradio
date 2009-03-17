#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "dhcp.h"


volatile unsigned int dhcp_status=DHCP_CLOSED;
volatile long dhcp_timeout=0L;
unsigned long dhcp_id=0UL;
IP_Addr dhcp_server=0UL;
IP_Addr dhcp_ip=0UL;
IP_Addr dhcp_netmask=0UL;
IP_Addr dhcp_router=0UL;
IP_Addr dhcp_dns=0UL;
IP_Addr dhcp_ntp=0UL;


unsigned int dhcp_request(unsigned int index, unsigned int msg)
{
  DHCP_Header *tx_dhcp;
  unsigned int i=0;

  dhcp_timeout = getontime()+DHCP_TIMEOUT;

  tx_dhcp = (DHCP_Header*) &eth_txbuf[DHCP_OFFSET];

  memset(tx_dhcp, 0, DHCP_HEADERLEN);

  tx_dhcp->op           = DHCP_OP_REQUEST;
  tx_dhcp->htype        = DHCP_HTYPE_ETH;
  tx_dhcp->hlen         = DHCP_HLEN_MAC;
  tx_dhcp->xid          = swap32(dhcp_id);
  tx_dhcp->chaddr.mac   = eth_mac();
  tx_dhcp->mcookie      = SWAP32(DHCP_MCOOKIE);

  tx_dhcp->options[i++] = DHCP_OPTION_MSGTYPE; 
  tx_dhcp->options[i++] = 1;   //Len
  tx_dhcp->options[i++] = msg; //Type

  tx_dhcp->options[i++] = DHCP_OPTION_PARAMLIST;
  tx_dhcp->options[i++] = 4; //Len
  tx_dhcp->options[i++] = DHCP_OPTION_NETMASK;
  tx_dhcp->options[i++] = DHCP_OPTION_ROUTER;
  tx_dhcp->options[i++] = DHCP_OPTION_TIMESERVER; 
  tx_dhcp->options[i++] = DHCP_OPTION_DNS;

  tx_dhcp->options[i++] = DHCP_OPTION_CLIENTID;
  tx_dhcp->options[i++] = 6; //Len
  tx_dhcp->options[i++] = (eth_mac()>> 0)&0xff;
  tx_dhcp->options[i++] = (eth_mac()>> 8)&0xff;
  tx_dhcp->options[i++] = (eth_mac()>>16)&0xff;
  tx_dhcp->options[i++] = (eth_mac()>>24)&0xff;
  tx_dhcp->options[i++] = (eth_mac()>>32)&0xff;
  tx_dhcp->options[i++] = (eth_mac()>>40)&0xff;

  if(msg == DHCP_MSG_REQUEST)
  {
    tx_dhcp->options[i++] = DHCP_OPTION_SERVERID;
    tx_dhcp->options[i++] = 4; //Len
    tx_dhcp->options[i++] = (dhcp_server>> 0)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>> 8)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>>16)&0xff;
    tx_dhcp->options[i++] = (dhcp_server>>24)&0xff;

    tx_dhcp->options[i++] = DHCP_OPTION_REQUESTEDIP;
    tx_dhcp->options[i++] = 4; //Len
    tx_dhcp->options[i++] = (dhcp_ip>> 0)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>> 8)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>>16)&0xff;
    tx_dhcp->options[i++] = (dhcp_ip>>24)&0xff;
  }

  tx_dhcp->options[i++] = DHCP_OPTION_HOSTNAME; //Host Name
  tx_dhcp->options[i++] = strlen(eth_name()); //Len
  i += sprintf(&tx_dhcp->options[i], "%s", eth_name()); //Name

  tx_dhcp->options[i++] = 0xff; //END Option
  tx_dhcp->options[i++] = 0x00;
  tx_dhcp->options[i++] = 0x00;

  index = udp_open(index, 0x000000000000ULL, 0xFFFFFFFFUL, DHCPSERVER_PORT, DHCPCLIENT_PORT, 0, DHCP_HEADERLEN+4+i); //header + mcookie + options

  return index;
}


unsigned int dhcp_getcfg(void)
{
  long timeout;
  IP_Addr ip;
  unsigned int index;

  menu_popup("DHCP: Discover...");

  ip = eth_ip(); //save current device ip
  eth_setip(0UL); //set device ip to zero

  dhcp_status    = DHCP_DISCOVER;
  dhcp_id        = generate_id();
  dhcp_server    = 0UL;
  dhcp_ip        = 0UL;
  dhcp_netmask   = 0UL;
  dhcp_router    = 0UL;
  dhcp_dns       = 0UL;
  dhcp_ntp       = 0UL;

  index = dhcp_request(UDP_ENTRIES, DHCP_MSG_DISCOVER); 

  timeout = getontime()+ETH_TIMEOUT;
  for(;;)
  {
    eth_service();

    if(dhcp_status == DHCP_ACK)
    {
      break;
    }
    if(getdeltatime(dhcp_timeout) > 0)
    {
      switch(dhcp_status)
      {
        case DHCP_DISCOVER:
          index = dhcp_request(index, DHCP_MSG_DISCOVER); 
          break;
        case DHCP_REQUEST:
          index = dhcp_request(index, DHCP_MSG_REQUEST);
          break;
      }
    }
    if(getdeltatime(timeout) > 0)
    {
      break;
    }
  }

  udp_close(index);

  menu_update(1);

  if(dhcp_status == DHCP_ACK) //DHCP request sucessful
  {
    eth_setip(dhcp_ip);
    if(dhcp_netmask != 0UL)
    {
      eth_setnetmask(dhcp_netmask);
    }
    if(dhcp_router != 0UL)
    {
      eth_setrouter(dhcp_router);
    }
    if(dhcp_dns != 0UL)
    {
      eth_setdns(dhcp_dns);
    }
    if(dhcp_ntp != 0UL)
    {
      eth_setntp(dhcp_ntp);
    }
    return 1;
  }

  dhcp_status = DHCP_CLOSED;

  eth_setip(ip); //set last ip

  return 0;
}


void dhcp_udpapp(unsigned int index)
{
  DHCP_Header *rx_dhcp;
  unsigned char *ptr, c, len;

  DEBUGOUT("DHCP: UDP app\n");

  rx_dhcp = (DHCP_Header*) &eth_rxbuf[DHCP_OFFSET];

  if((rx_dhcp->op              == DHCP_OP_REPLAY) &&
     (rx_dhcp->htype           == DHCP_HTYPE_ETH) &&
     (rx_dhcp->hlen            == DHCP_HLEN_MAC)  &&
     (swap32(rx_dhcp->xid)     == dhcp_id)        &&
     (rx_dhcp->yiaddr          != 0UL)            &&
     (rx_dhcp->chaddr.mac      == eth_mac())      &&
     (swap32(rx_dhcp->mcookie) == DHCP_MCOOKIE))
  {
    switch(dhcp_status)
    {
      case DHCP_DISCOVER:
        if((rx_dhcp->options[0] == DHCP_OPTION_MSGTYPE) &&
           (rx_dhcp->options[1] == 1) &&
           (rx_dhcp->options[2] == DHCP_MSG_OFFER))        //DHCP Offer
        {
          dhcp_ip = rx_dhcp->yiaddr; //get ip
    
          //parse options
          ptr = rx_dhcp->options;
          while(*ptr != 0xff)
          {
            c  = *ptr++;
            if(c == 0xff) //END
            {
              break;
            }
            len = *ptr++;
    
            switch(c)
            {
              case DHCP_OPTION_NETMASK:
                if(len == 4)
                {
                  dhcp_netmask = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0);
                }
                break;
              case DHCP_OPTION_ROUTER:
                if(len == 4)
                {
                  dhcp_router = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0);
                }
                break;
              case DHCP_OPTION_TIMESERVER:
                if(len == 4)
                {
                  dhcp_ntp = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0);
                }
                break;
              case DHCP_OPTION_DNS:
                if(len == 4)
                {
                  dhcp_dns = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0);
                }
                break;
              case DHCP_OPTION_SERVERID:
                if(len == 4)
                {
                  dhcp_server = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|(ptr[0]<<0);
                }
                break;
            }
            while(len--){ ptr++; }
          }
          dhcp_request(index, DHCP_MSG_REQUEST);
          dhcp_status = DHCP_REQUEST;
          menu_popup("DHCP: Request...");
        }
        break;

      case DHCP_REQUEST:
        if((rx_dhcp->options[0] == DHCP_OPTION_MSGTYPE) &&
           (rx_dhcp->options[1] == 1)                   &&
           (rx_dhcp->options[2] == DHCP_MSG_ACK))          //DHCP Ack
        {
          dhcp_status = DHCP_ACK;
          menu_popup("DHCP: Ack");
        }
        break;
    }
  }

  return;
}
