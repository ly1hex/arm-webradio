#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "third_party/fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "eth.h"
#include "eth/utils.h"
#include "eth/dhcp.h"
#include "eth/dns.h"
#include "eth/http.h"
#include "eth/nbns.h"
#include "eth/ntp.h"
#include "eth/rtsp.h"
#include "eth/shoutcast.h"
#include "eth/ssdp.h"
#include "eth/upnp.h"
#include "buffer.h"


Device dev;
volatile MAC_Addr requested_mac=0UL;
TCP_Table tcp_table[TCP_ENTRIES];
UDP_Table udp_table[UDP_ENTRIES];
unsigned char eth_rxbuf[ETH_MTUSIZE], eth_txbuf[ETH_MTUSIZE];


void udp_close(unsigned int index)
{
  if(index < UDP_ENTRIES)
  {
    udp_table[index].status = UDP_CLOSED;
  }

  return;
}


unsigned int udp_open(unsigned int index, MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int dst_port, unsigned int src_port, unsigned char *data, unsigned int len)
{
  if(index >= UDP_ENTRIES)
  {
    for(index=0; index<UDP_ENTRIES; index++) //look for free table index
    {
      if(udp_table[index].status == UDP_CLOSED)
      {
         break;
      }
    }
    if(index >= UDP_ENTRIES)
    {
      DEBUGOUT("Eth: UDP table full\n");
      return UDP_ENTRIES;
    }
  }

  udp_table[index].mac        = dst_mac;
  udp_table[index].ip         = dst_ip;
  udp_table[index].port       = dst_port;
  udp_table[index].local_port = src_port;
  udp_table[index].status     = UDP_OPENED;
  udp_table[index].time       = 0;

  if(data)
  {
    memcpy(&eth_txbuf[UDP_DATASTART], data, len);
  }
  udp_send(index, len);

  return index;
}


void udp_send(unsigned int index, unsigned int len)
{
  if(index < UDP_ENTRIES)
  {
    udp_table[index].time = 0;

    make_udp_header(index, len);
    ethernet_put(eth_txbuf, ETH_HEADERLEN+IP_HEADERLEN+UDP_HEADERLEN+len);
  }

  return;
}


void udp_app(unsigned int index)
{
  IP_Header *rx_ip;
  UDP_Header *rx_udp;
  unsigned int len;

  rx_ip  = (IP_Header*)  &eth_rxbuf[IP_OFFSET];
  rx_udp = (UDP_Header*) &eth_rxbuf[UDP_OFFSET];

  len = swap16(rx_udp->len) - UDP_HEADERLEN;

  switch(udp_table[index].local_port)
  {
    case DHCPCLIENT_PORT:
      if(udp_table[index].port == DHCPSERVER_PORT) //only if server port is correct
      {
        dhcp_udpapp(index, &eth_rxbuf[UDP_DATASTART], len, &eth_txbuf[UDP_DATASTART]);
      }
      break;
    case DNS_PORT:
      dns_udpapp(index, &eth_rxbuf[UDP_DATASTART], len, &eth_txbuf[UDP_DATASTART]);
      break;
    case NBNS_PORT:
      if((rx_ip->src_ip&dev.netmask) == (dev.ip&dev.netmask)) //local net only
      {
        nbns_udpapp(index, &eth_rxbuf[UDP_DATASTART], len, &eth_txbuf[UDP_DATASTART]);
      }
      break;
    case NTP_PORT:
      ntp_udpapp(index, &eth_rxbuf[UDP_DATASTART], len, &eth_txbuf[UDP_DATASTART]);
      break;
    case SSDP_PORT:
      if((rx_ip->src_ip&dev.netmask) == (dev.ip&dev.netmask)) //local net only
      {
        ssdp_udpapp(index, &eth_rxbuf[UDP_DATASTART], len, &eth_txbuf[UDP_DATASTART]);
      }
      break;
  }

  return;
}


void udp_service(void)
{
  ETH_Header *rx_eth;
  IP_Header *rx_ip;
  UDP_Header *rx_udp;
  unsigned int index;

  rx_eth = (ETH_Header*) &eth_rxbuf[ETH_OFFSET];
  rx_ip  = (IP_Header*)  &eth_rxbuf[IP_OFFSET];
  rx_udp = (UDP_Header*) &eth_rxbuf[UDP_OFFSET];

  for(index=0; index<UDP_ENTRIES; index++) //look for table index
  {
    if((udp_table[index].status != UDP_CLOSED)             &&
       (rx_udp->src_port == swap16(udp_table[index].port)) &&
       (rx_udp->dst_port == swap16(udp_table[index].local_port)))
    {
      break;
    }
  }

  if(index < UDP_ENTRIES) //connection in table
  {
    udp_table[index].time = 0;
    udp_app(index);
  }
  else  //new connection
  {
    for(index=0; index<UDP_ENTRIES; index++) //look for free table index
    {
      if(udp_table[index].status == UDP_CLOSED)
      {
         break;
      }
    }
    if(index >= UDP_ENTRIES)
    {
      DEBUGOUT("Eth: UDP table full\n");
    }
    else
    {
      DEBUGOUT("Eth: (%i) UDP new %i -> %i (local)\n", index, swap16(rx_udp->src_port), swap16(rx_udp->dst_port));
      udp_table[index].mac        = rx_eth->src_mac;
      udp_table[index].ip         = rx_ip->src_ip;
      udp_table[index].port       = swap16(rx_udp->src_port);
      udp_table[index].local_port = swap16(rx_udp->dst_port);
      udp_table[index].status     = UDP_OPENED;
      udp_table[index].time       = 0;
      udp_app(index);
    }
  }

  return;
}


void tcp_abort(unsigned int index)
{
  if(index < TCP_ENTRIES)
  {
    tcp_table[index].status = TCP_ABORT;
    tcp_table[index].flags  = TCP_FLAG_RST;
  }

  return;
}


void tcp_close(unsigned int index)
{
  if(index < TCP_ENTRIES)
  {
    tcp_table[index].status  = TCP_CLOSE;
    tcp_table[index].flags  |= TCP_FLAG_FIN;
  }

  return;
}


unsigned int tcp_open(unsigned int index, MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int dst_port, unsigned int src_port)
{
#if defined(TCP_MSS)
  TCP_Header *tx_tcp;
#endif

  if(index >= TCP_ENTRIES)
  {
    for(index=0; index<TCP_ENTRIES; index++)
    {
      if(tcp_table[index].status == TCP_CLOSED) //empty entry found
      {
        break;
      }
    }
    if(index >= TCP_ENTRIES)
    {
      DEBUGOUT("Eth: TCP table full\n");
      return TCP_ENTRIES;
    }
  }

  tcp_table[index].mac        = dst_mac;
  tcp_table[index].ip         = dst_ip;
  tcp_table[index].port       = dst_port;
  tcp_table[index].local_port = src_port;
  tcp_table[index].acknum     = 0UL;
  tcp_table[index].seqnum     = swap32(generate_id()); //1UL;
  tcp_table[index].flags      = TCP_FLAG_SYN;
  tcp_table[index].status     = TCP_OPEN;
  tcp_table[index].time       = 0;
  tcp_table[index].error      = 0;

#if defined(TCP_MSS)
  tx_tcp = (TCP_Header*) &eth_txbuf[TCP_OFFSET];
  tx_tcp->options[0] = 0x02; //kind = 2 (Maximum Segment Size)
  tx_tcp->options[1] = 0x04; //len  = 4 bytes
  tx_tcp->options[2] = (SWAP16(TCP_MSS)>>0)&0xff;
  tx_tcp->options[3] = (SWAP16(TCP_MSS)>>8)&0xff;
  tcp_send(index, 4, 4);
#else
  tcp_send(index, 0, 0);
#endif

  return index;
}

void tcp_send(unsigned int index, unsigned int len, unsigned int options)
{
  if(index < TCP_ENTRIES)
  {
    tcp_table[index].time = 0;

    make_tcp_header(index, len, options);
    ethernet_put(eth_txbuf, ETH_HEADERLEN+IP_HEADERLEN+TCP_HEADERLEN+len);

    tcp_table[index].seqnum += len-options;
  }

  return;
}


void tcp_app(unsigned int index)
{
  IP_Header *rx_ip;
  TCP_Header *rx_tcp;
  unsigned int len, hd_len;

  rx_ip  = (IP_Header*)  &eth_rxbuf[IP_OFFSET];
  rx_tcp = (TCP_Header*) &eth_rxbuf[TCP_OFFSET];
  hd_len = (rx_ip->hd_len*4) + (rx_tcp->len*4);
  len    = swap16(rx_ip->len) - hd_len;

  switch(tcp_table[index].local_port)
  {
    case SHOUTCAST_CLIENTPORT1:
    case SHOUTCAST_CLIENTPORT2:
    case SHOUTCAST_CLIENTPORT3:
      shoutcast_tcpapp(index, &eth_rxbuf[ETH_HEADERLEN+hd_len], len, &eth_txbuf[TCP_DATASTART]);
      break;
    case RTSP_CLIENTPORT1:
    case RTSP_CLIENTPORT2:
    case RTSP_CLIENTPORT3:
      rtsp_tcpapp(index, &eth_rxbuf[ETH_HEADERLEN+hd_len], len, &eth_txbuf[TCP_DATASTART]);
      break;
    case HTTP_SERVERPORT:
      if((rx_ip->src_ip&dev.netmask) == (dev.ip&dev.netmask)) //local net only
      {
        http_tcpapp(index, &eth_rxbuf[ETH_HEADERLEN+hd_len], len, &eth_txbuf[TCP_DATASTART]);
      }
      break;
    case UPNP_PORT:
      if((rx_ip->src_ip&dev.netmask) == (dev.ip&dev.netmask)) //local net only
      {
        upnp_tcpapp(index, &eth_rxbuf[ETH_HEADERLEN+hd_len], len, &eth_txbuf[TCP_DATASTART]);
      }
      break;
  }

  return;
}


void tcp_service(void)
{
  ETH_Header *rx_eth;
  IP_Header *rx_ip;
  TCP_Header *rx_tcp, *tx_tcp;
  unsigned int index, len, hd_len;

  rx_ip  = (IP_Header*)  &eth_rxbuf[IP_OFFSET];
  rx_tcp = (TCP_Header*) &eth_rxbuf[TCP_OFFSET];

  hd_len = (rx_ip->hd_len*4)+(rx_tcp->len*4);
  len    = swap16(rx_ip->len)-hd_len;

  for(index=0; index<TCP_ENTRIES; index++) //look for table index
  {
    if((tcp_table[index].status != TCP_CLOSED)             &&
       (rx_tcp->src_port == swap16(tcp_table[index].port)) &&
       (rx_tcp->dst_port == swap16(tcp_table[index].local_port)))
    {
      break;
    }
  }

  if(index < TCP_ENTRIES) //connection in table
  {
    tcp_table[index].time = 0; //reset connection timeout
    switch(tcp_table[index].status)
    {
      case TCP_OPENED: //start app
        if(rx_tcp->flags&TCP_FLAG_RST) //abort connection
        {
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_OPENED -> RST -> TCP_CLOSED\n", index);
        }
        else if(rx_tcp->flags&TCP_FLAG_FIN) //close connection
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len+1;
          tcp_table[index].flags  = TCP_FLAG_ACK;
          tcp_send(index, 0, 0);
          tcp_table[index].status = TCP_FIN;
          DEBUGOUT("Eth: (%i) TCP_OPENED -> FIN -> send ACK -> TCP_FIN\n", index);
        }
        else if(rx_tcp->flags&TCP_FLAG_SYN) //open connection
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len+1;
          tcp_table[index].flags  = TCP_FLAG_SYN|TCP_FLAG_ACK;
#if defined(TCP_MSS)
          tx_tcp = (TCP_Header*) &eth_txbuf[TCP_OFFSET];
          tx_tcp->options[0] = 0x02; //kind = 2 (Maximum Segment Size)
          tx_tcp->options[1] = 0x04; //len  = 4 bytes
          tx_tcp->options[2] = (SWAP16((ETH_MTUSIZE-40))>>0)&0xff;
          tx_tcp->options[3] = (SWAP16((ETH_MTUSIZE-40))>>8)&0xff;
          tcp_send(index, 4, 4);
#else
          tcp_send(index, 0, 0);
#endif
          tcp_table[index].seqnum++;
          tcp_table[index].status = TCP_OPENED;
          DEBUGOUT("Eth: (%i) TCP_OPENED -> SYN -> send SYN+ACK -> TCP_OPENED\n", index);
        }
        else
        {
          if(swap32(rx_tcp->seqnum) != tcp_table[index].acknum)
          {
            if(swap32(rx_tcp->seqnum) < tcp_table[index].acknum) //dup frame -> send ack
            {
              unsigned long ack;
              ack = tcp_table[index].acknum;
              tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len;
              tcp_table[index].flags  = TCP_FLAG_ACK;
              tcp_send(index, 0, 0);
              tcp_table[index].acknum = ack;
              DEBUGOUT("Eth: (%i) TCP_OPENED seq<ack\n", index);
            }
            else                                                 //frame lost -> send last ack
            {
              if(++tcp_table[index].error > TCP_MAXERROR)
              {
                tcp_abort(index);
              }
              else
              {
                tcp_table[index].flags = TCP_FLAG_ACK;
                tcp_send(index, 0, 0);
              }
              DEBUGOUT("Eth: (%i) TCP_OPENED seq>ack\n", index);
            }
          }
          else
          {
            tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len; //next seq nr
            tcp_table[index].flags  = TCP_FLAG_ACK;
            tcp_table[index].error  = 0;
            tcp_app(index);
          }
        }
        break;

      case TCP_OPEN: //wait for SYN && ACK -> sent ACK
        if((rx_tcp->flags&TCP_FLAG_RST) || (rx_tcp->flags&TCP_FLAG_FIN))
        {
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_OPEN -> RST|FIN -> TCP_CLOSED\n", index);
        }
        else if((rx_tcp->flags&TCP_FLAG_SYN) && 
                (rx_tcp->flags&TCP_FLAG_ACK)) //i am client -> ACK
        {
          DEBUGOUT("Eth: (%i) TCP_OPEN -> SYN+ACK -> send ACK -> TCP_OPENED\n", index);
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len+1;
          tcp_table[index].seqnum = swap32(rx_tcp->acknum);
          tcp_table[index].flags  = TCP_FLAG_ACK;
          tcp_table[index].status = TCP_OPENED;
          tcp_send(index, 0, 0);
          tcp_app(index);
        }
        else
        {
          DEBUGOUT("Eth: (%i) TCP_OPEN -> send RST -> TCP_CLOSED\n", index);
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len;
          tcp_table[index].seqnum = swap32(rx_tcp->acknum);
          tcp_table[index].flags  = TCP_FLAG_RST;
          tcp_send(index, 0, 0);
          tcp_table[index].status = TCP_CLOSED;
        }
        break;

      case TCP_ABORT: //sent RST
        tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len;
        tcp_table[index].seqnum = swap32(rx_tcp->acknum);
        tcp_table[index].flags  = TCP_FLAG_RST;
        tcp_send(index, 0, 0);
        tcp_table[index].status = TCP_CLOSED;
        DEBUGOUT("Eth: (%i) TCP_ABORT -> send RST -> TCP_CLOSED\n", index);
        break;

      case TCP_CLOSE: //sent FIN
        if(rx_tcp->flags&TCP_FLAG_RST)
        {
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_CLOSE -> RST -> TCP_CLOSED\n", index);
        }
        else if(rx_tcp->flags&TCP_FLAG_FIN)
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len+1;
          tcp_table[index].flags  = TCP_FLAG_ACK;
          tcp_send(index, 0, 0);
          tcp_table[index].status = TCP_FIN;
          DEBUGOUT("Eth: (%i) TCP_CLOSE -> FIN -> send ACK -> TCP_FIN\n", index);
        }
        else //I am closing -> send FIN && ACK
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len;
          tcp_table[index].flags  = TCP_FLAG_FIN|TCP_FLAG_ACK;
          tcp_send(index, 0, 0);
          tcp_table[index].seqnum++;
          tcp_table[index].status = TCP_FIN;
          DEBUGOUT("Eth: (%i) TCP_CLOSE -> send FIN+ACK -> TCP_FIN\n", index);
        }
        break;

      case TCP_FIN: //wait for FIN -> sent ACK
        if(rx_tcp->flags&TCP_FLAG_RST)
        {
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_FIN -> RST -> TCP_CLOSED\n", index);
        }
        else if((rx_tcp->flags&TCP_FLAG_ACK) && 
                (tcp_table[index].flags&TCP_FLAG_FIN)) //I am closing -> send FIN && ACK
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len;
          tcp_table[index].flags  = TCP_FLAG_FIN|TCP_FLAG_ACK;
          tcp_send(index, 0, 0);
          tcp_table[index].seqnum++;
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_FIN -> ACK -> send FIN+ACK -> TCP_CLOSED\n", index);
        }
        else if(rx_tcp->flags&TCP_FLAG_FIN)
        {
          tcp_table[index].acknum = swap32(rx_tcp->seqnum)+len+1;
          tcp_table[index].flags  = TCP_FLAG_ACK;
          tcp_send(index, 0, 0);
          tcp_table[index].status = TCP_CLOSED;
          DEBUGOUT("Eth: (%i) TCP_FIN -> FIN -> send ACK -> TCP_CLOSED\n", index);
        }
        break;
    }
  }
  else //new connection
  {
    for(index=0; index<TCP_ENTRIES; index++) //look for free table index
    {
      if(tcp_table[index].status == TCP_CLOSED)
      {
         break;
      }
    }
    rx_eth = (ETH_Header*) &eth_rxbuf[ETH_OFFSET];

    if(index >= TCP_ENTRIES)
    {
      DEBUGOUT("Eth: TCP table full\n");
    }
    else if(rx_tcp->flags&TCP_FLAG_SYN) //i am server -> SYN+ACK
    {
      tcp_table[index].mac        = rx_eth->src_mac;
      tcp_table[index].ip         = rx_ip->src_ip;
      tcp_table[index].port       = swap16(rx_tcp->src_port);
      tcp_table[index].local_port = swap16(rx_tcp->dst_port);
      tcp_table[index].acknum     = swap32(rx_tcp->seqnum)+len+1;
      tcp_table[index].seqnum     = swap32(generate_id()); //1UL;
      tcp_table[index].flags      = TCP_FLAG_SYN|TCP_FLAG_ACK;
      tcp_table[index].status     = TCP_OPENED;
      tcp_table[index].time       = 0;
      tcp_table[index].error      = 0;
#if defined(TCP_MSS)
      tx_tcp = (TCP_Header*) &eth_txbuf[TCP_OFFSET];
      tx_tcp->options[0] = 0x02; //kind = 2 (Maximum Segment Size)
      tx_tcp->options[1] = 0x04; //len  = 4 bytes
      tx_tcp->options[2] = (SWAP16((ETH_MTUSIZE-40))>>0)&0xff;
      tx_tcp->options[3] = (SWAP16((ETH_MTUSIZE-40))>>8)&0xff;
      tcp_send(index, 4, 4);
#else
      tcp_send(index, 0, 0);
#endif
      tcp_table[index].seqnum++;
      DEBUGOUT("Eth: (%i) TCP new: %i -> %i\n", index, tcp_table[index].port, tcp_table[index].local_port);
    }
    /*else if(rx_tcp->flags&TCP_FLAG_FIN) //close connetion
    {
      tcp_table[index].mac        = rx_eth->src_mac;
      tcp_table[index].ip         = rx_ip->src_ip;
      tcp_table[index].port       = swap16(rx_tcp->src_port);
      tcp_table[index].local_port = swap16(rx_tcp->dst_port);
      tcp_table[index].acknum     = swap32(rx_tcp->seqnum)+len+1;
      tcp_table[index].seqnum     = swap32(rx_tcp->acknum);
      tcp_table[index].flags      = TCP_FLAG_FIN|TCP_FLAG_ACK;
      tcp_table[index].status     = TCP_CLOSED;
      tcp_table[index].time       = 0;
      tcp_table[index].error      = 0;
      tcp_send(index, 0, 0);
      DEBUGOUT("Eth: (%i) TCP new close: %i -> %i\n", index, tcp_table[index].port, tcp_table[index].local_port);
    }*/
    else  //abort connetion
    {
      tcp_table[index].mac        = rx_eth->src_mac;
      tcp_table[index].ip         = rx_ip->src_ip;
      tcp_table[index].port       = swap16(rx_tcp->src_port);
      tcp_table[index].local_port = swap16(rx_tcp->dst_port);
      tcp_table[index].acknum     = swap32(rx_tcp->seqnum)+len+1;
      tcp_table[index].seqnum     = swap32(rx_tcp->acknum);
      tcp_table[index].flags      = TCP_FLAG_RST|TCP_FLAG_ACK;
      tcp_table[index].status     = TCP_CLOSED;
      tcp_table[index].time       = 0;
      tcp_table[index].error      = 0;
      tcp_send(index, 0, 0);
      DEBUGOUT("Eth: (%i) TCP new abort: %i -> %i\n", index, tcp_table[index].port, tcp_table[index].local_port);
    }
  }

  return;
}


void icmp_service(void)
{
  ETH_Header *rx_eth;
  IP_Header *rx_ip;
  ICMP_Header *rx_icmp, *tx_icmp;
  unsigned int len;

  rx_eth  = (ETH_Header*)  &eth_rxbuf[ETH_OFFSET];
  rx_ip   = (IP_Header*)   &eth_rxbuf[IP_OFFSET];
  rx_icmp = (ICMP_Header*) &eth_rxbuf[ICMP_OFFSET];
  tx_icmp = (ICMP_Header*) &eth_txbuf[ICMP_OFFSET];

  switch(rx_icmp->type)
  {
    case ICMP_ECHO_REQ:
      len = swap16(rx_ip->len)-IP_HEADERLEN-ICMP_HEADERLEN;
      make_ip_header(rx_eth->src_mac, rx_ip->src_ip, (IP_HEADERLEN+ICMP_HEADERLEN+len), IP_PROTO_ICMP);

      tx_icmp->type     = ICMP_ECHO_REP;
      tx_icmp->code     = 0x00;
      tx_icmp->checksum = SWAP16(0x0000);
      memcpy(&eth_txbuf[ICMP_DATASTART], &eth_rxbuf[ICMP_DATASTART], len);
      tx_icmp->checksum = checksum_ip((unsigned char*)tx_icmp, ICMP_HEADERLEN+len);

      ethernet_put(eth_txbuf, ETH_HEADERLEN+IP_HEADERLEN+ICMP_HEADERLEN+len);
      DEBUGOUT("Eth: Ping\n");
      break;
  }

  return;
}


void arp_request(IP_Addr ip)
{
  if((ip&dev.netmask) != (dev.ip&dev.netmask))
  {
    ip = dev.router;
  }

  make_arp_header(0ULL, 0ULL, ip, ARP_OP_REQUEST);
  ethernet_put(eth_txbuf, ETH_HEADERLEN+ARP_HEADERLEN);

  return;
}


MAC_Addr arp_getmac(IP_Addr ip)
{
  long timeout, timeout_arp;

  requested_mac = 0ULL;

  arp_request(ip);

  timeout     = getontime()+ETH_TIMEOUT;
  timeout_arp = getontime()+1;
  for(;;)
  {
    eth_service();

    if(requested_mac != 0ULL)
    {
      break;
    }
    if(getdeltatime(timeout_arp) > 0)
    {
      timeout_arp = getontime()+1;
      arp_request(ip);
    }
    if(getdeltatime(timeout) > 0)
    {
      break;
    }
  }

  return requested_mac;
}


void arp_service(void)
{
  ETH_Header *rx_eth;
  ARP_Header *rx_arp, *tx_arp;

  rx_eth = (ETH_Header*) &eth_rxbuf[ETH_OFFSET];
  rx_arp = (ARP_Header*) &eth_rxbuf[ARP_OFFSET];
  tx_arp = (ARP_Header*) &eth_txbuf[ARP_OFFSET];

  if((rx_arp->hw_type  == ARP_HW_TYPE)  &&
     (rx_arp->pro_type == ARP_PRO_TYPE) &&
     (rx_arp->hw_len   == ARP_HW_LEN)   &&
     (rx_arp->pro_len  == ARP_PRO_LEN)  &&
     (rx_arp->dst_ip   == dev.ip))
  {
    switch(rx_arp->op)
    {
      case ARP_OP_REQUEST: //arp request
        make_arp_header(rx_eth->src_mac, rx_arp->src_mac, rx_arp->src_ip, ARP_OP_REPLY);
        ethernet_put(eth_txbuf, ETH_HEADERLEN+ARP_HEADERLEN);
        break;

      case ARP_OP_REPLY: //arp reply
        requested_mac = rx_arp->src_mac;
        break;
    }
  }

  return;
}


void make_udp_header(unsigned int index, unsigned int len)
{
  UDP_Header *tx_udp;

  tx_udp = (UDP_Header*) &eth_txbuf[UDP_OFFSET];

  make_ip_header(udp_table[index].mac, udp_table[index].ip, (IP_HEADERLEN+UDP_HEADERLEN+len), IP_PROTO_UDP);

  tx_udp->src_port = swap16(udp_table[index].local_port);
  tx_udp->dst_port = swap16(udp_table[index].port);
  tx_udp->len      = swap16(UDP_HEADERLEN+len);
  tx_udp->checksum = SWAP16(0x0000);

  return;
}


unsigned int checksum_tcp(unsigned char *s, unsigned int len, IP_Addr dst_ip)
{
  unsigned long sum=0;

  //include TCP Pseudo-Header
  sum += (uint16_t)(dev.ip);
  sum += (uint16_t)(dev.ip>>16);
  sum += (uint16_t)(dst_ip);
  sum += (uint16_t)(dst_ip>>16);
  sum += swap16(len);
  sum += SWAP16(IP_PROTO_TCP);

  for(; len > 1; len-=2)
  {
    sum += *((uint16_t*)s);
    s   += 2;
  }

  if(len) //add left-over byte
  {
    sum += *(unsigned char*)s;
  }

  while(sum>>16)
  {
    sum = (sum&0xFFFF)+(sum>>16);
  }

  return ~sum;
}


void make_tcp_header(unsigned int index, unsigned int len, unsigned int options)
{
  TCP_Header *tx_tcp;

  tx_tcp = (TCP_Header*) &eth_txbuf[TCP_OFFSET];

  make_ip_header(tcp_table[index].mac, tcp_table[index].ip, (IP_HEADERLEN+TCP_HEADERLEN+len), IP_PROTO_TCP);

  tx_tcp->src_port = swap16(tcp_table[index].local_port);
  tx_tcp->dst_port = swap16(tcp_table[index].port);
  tx_tcp->seqnum   = swap32(tcp_table[index].seqnum);
  if(tcp_table[index].flags & TCP_FLAG_ACK)
  {
    tx_tcp->acknum = swap32(tcp_table[index].acknum);
  }
  else
  {
    tx_tcp->acknum = 0UL;
  }
  tx_tcp->len      = (TCP_HEADERLEN+options)/4;
  tx_tcp->reserved = 0x00;
  tx_tcp->flags    = tcp_table[index].flags;

  if(tcp_table[index].local_port > 1000) //station stream
  {
    unsigned int free = vsbuf_free();
    if     (free > (TCP_WINDOW*2)){ tx_tcp->window = SWAP16(TCP_WINDOW/1);  }
    else if(free > (TCP_WINDOW/1)){ tx_tcp->window = SWAP16(TCP_WINDOW/2);  }
    else if(free > (TCP_WINDOW/2)){ tx_tcp->window = SWAP16(TCP_WINDOW/4);  }
    else if(free > (TCP_WINDOW/4)){ tx_tcp->window = SWAP16(TCP_WINDOW/8);  }
    else if(free > (TCP_WINDOW/8)){ tx_tcp->window = SWAP16(TCP_WINDOW/16); }
    else                          { tx_tcp->window = SWAP16(32);            }
  }
  else
  {
    tx_tcp->window = SWAP16(TCP_WINDOW);
  }

  tx_tcp->checksum = SWAP16(0x0000);
  tx_tcp->urgent   = SWAP16(0x0000);
  tx_tcp->checksum = checksum_tcp((unsigned char*)tx_tcp, TCP_HEADERLEN+len, tcp_table[index].ip);

  return;
}


unsigned int checksum_ip(unsigned char *s, unsigned int len)
{
  unsigned long sum=0;

  for(; len > 1; len-=2)
  {
    sum += *((uint16_t*)s);
    s   += 2;
  }

  if(len) //add left-over byte
  {
    sum += *(unsigned char*)s;
  }

  while(sum>>16)
  {
    sum = (sum&0xFFFF)+(sum>>16);
  }

  return ~sum;
}


void make_ip_header(MAC_Addr dst_mac, IP_Addr dst_ip, unsigned int len, unsigned int proto)
{
  IP_Header *tx_ip;

  tx_ip = (IP_Header*) &eth_txbuf[IP_OFFSET];

  make_eth_header(dst_mac, ETH_TYPE_IP);

  tx_ip->ver      = 0x04; //v4
  tx_ip->hd_len   = IP_HEADERLEN/4;
#if defined(ETH_USE_DSCP)
  tx_ip->tos      = 0xB8; //0xB8 = Expedited Forwarding (QoS: DSCP)
#else
  tx_ip->tos      = 0x00;
#endif
  tx_ip->len      = swap16(len);
  tx_ip->id       = SWAP16(0x0000);
  tx_ip->flg_offs = SWAP16(0x4000); //do not fragment
  tx_ip->ttl      = 128; //max. hops
  tx_ip->proto    = proto; //protocol
  tx_ip->checksum = SWAP16(0x0000);
  tx_ip->dst_ip   = dst_ip;
  tx_ip->src_ip   = dev.ip;

  tx_ip->checksum = checksum_ip((unsigned char*)tx_ip, IP_HEADERLEN);

  return;
}


void make_arp_header(MAC_Addr dst_mac, MAC_Addr arp_dst_mac, IP_Addr arp_dst_ip, unsigned int op)
{
  ARP_Header *tx_arp;

  tx_arp = (ARP_Header*) &eth_txbuf[ARP_OFFSET];

  make_eth_header(dst_mac, ETH_TYPE_ARP);

  tx_arp->hw_type  = ARP_HW_TYPE;
  tx_arp->pro_type = ARP_PRO_TYPE;
  tx_arp->hw_len   = ARP_HW_LEN;
  tx_arp->pro_len  = ARP_PRO_LEN;
  tx_arp->op       = op;
  tx_arp->src_mac  = dev.mac;
  tx_arp->src_ip   = dev.ip;
  tx_arp->dst_mac  = arp_dst_mac;
  tx_arp->dst_ip   = arp_dst_ip;

  return;
}


void make_eth_header(MAC_Addr dst_mac, unsigned int type)
{
  ETH_Header *tx_eth;

  tx_eth = (ETH_Header*) &eth_txbuf[ETH_OFFSET];

  if(dst_mac != 0ULL)
  {
    tx_eth->dst_mac = dst_mac;
  }
  else
  {
    tx_eth->dst_mac = 0xFFFFFFFFFFFFULL; //broadcast
  }

  tx_eth->src_mac = dev.mac;
  tx_eth->type    = type;

  return;
}


void eth_timerservice(void) //called every second
{
  unsigned int index;
#if defined(TCP_MSS)
  TCP_Header *tx_tcp;
#endif

  //clean up TCP table
  for(index=0; index<TCP_ENTRIES; index++)
  {
    if(tcp_table[index].status == TCP_CLOSED)
    {
      continue;
    }
    else if(++tcp_table[index].time > TCP_TIMEOUT)
    {
      if(++tcp_table[index].error > TCP_MAXERROR) //abort connection
      {
        tcp_table[index].flags  = TCP_FLAG_RST;
        tcp_table[index].status = TCP_CLOSED;
        tcp_send(index, 0, 0);
      }
      else
      {
        tcp_table[index].time = 0;
        switch(tcp_table[index].status)
        {
          case TCP_OPENED: //send last ack
            tcp_table[index].flags = TCP_FLAG_ACK;
            tcp_send(index, 0, 0);
            DEBUGOUT("Eth: (%i) TCP_OPENED (timer)\n", index);
            break;
          case TCP_OPEN: //send syn
            tcp_table[index].flags = TCP_FLAG_SYN;
#if defined(TCP_MSS)
            tx_tcp = (TCP_Header*) &eth_txbuf[TCP_OFFSET];
            tx_tcp->options[0] = 0x02; //kind = 2 (Maximum Segment Size)
            tx_tcp->options[1] = 0x04; //len  = 4 bytes
            tx_tcp->options[2] = (SWAP16((ETH_MTUSIZE-40))>>0)&0xff;
            tx_tcp->options[3] = (SWAP16((ETH_MTUSIZE-40))>>8)&0xff;
            tcp_send(index, 4, 4);
#else
            tcp_send(index, 0, 0);
#endif
            DEBUGOUT("Eth: (%i) TCP_OPEN (timer)\n", index);
            break;
          case TCP_ABORT: //close connection
            tcp_table[index].flags  = TCP_FLAG_RST;
            tcp_send(index, 0, 0);
            tcp_table[index].status = TCP_CLOSED;
            DEBUGOUT("Eth: (%i) TCP_ABORT (timer)\n", index);
            break;
          case TCP_CLOSE: //send fin
            tcp_table[index].flags  = TCP_FLAG_FIN|TCP_FLAG_ACK;
            tcp_send(index, 0, 0);
            tcp_table[index].seqnum++;
            tcp_table[index].flags  = TCP_FLAG_RST;
            tcp_send(index, 0, 0);
            tcp_table[index].status = TCP_CLOSED;
            DEBUGOUT("Eth: (%i) TCP_CLOSE (timer)\n", index);
            break;
          case TCP_FIN:
            tcp_table[index].status = TCP_CLOSED;
            DEBUGOUT("Eth: (%i) TCP_FIN (timer)\n", index);
            break;
        }
      }
    }
  }

  //clean up UDP table
  for(index=0; index<UDP_ENTRIES; index++)
  {
    if(udp_table[index].status == UDP_CLOSED)
    {
      continue;
    }
    else if(++udp_table[index].time > UDP_TIMEOUT)
    {
      udp_table[index].status = UDP_CLOSED; //close connection
    }
  }

  return;
}


void eth_service(void)
{
  long time;
  static long last_time=0L;
  ETH_Header *rx_eth;
  IP_Header *rx_ip;

  //run timer service ervery second
  time = getontime();
  if(time != last_time)
  {
    last_time = time;
    eth_timerservice();
  }

  //check for data
  if(ethernet_get(eth_rxbuf, ETH_MTUSIZE) == 0)
  {
    return;
  }

  rx_eth = (ETH_Header*) &eth_rxbuf[ETH_OFFSET];
  rx_ip  = (IP_Header*) &eth_rxbuf[IP_OFFSET];
  switch(rx_eth->type)
  {
    case ETH_TYPE_IP:
      if((rx_ip->dst_ip == dev.ip)       || /*dst = dev*/
         (rx_ip->dst_ip == 0xFFFFFFFFUL) || /*dst = broadcast*/
         (((rx_ip->dst_ip&dev.netmask) == (dev.ip&dev.netmask)) && ((rx_ip->dst_ip&(~dev.netmask)) == (~dev.netmask))) || /*dst = broadcast local*/
         (dev.ip == 0UL)) /*dev = 0 (during DHCP)*/
      {
        switch(rx_ip->proto)
        {
          case IP_PROTO_ICMP: icmp_service(); break;
          case IP_PROTO_TCP:  tcp_service();  break;
          case IP_PROTO_UDP:  udp_service();  break;
        }
      }
      else if( ((rx_eth->dst_mac&0x000080FFFFFFULL)      == SWAP64(0x01005E0000000000ULL)) &&
               ((rx_ip->dst_ip&0xF0UL)                   == 0xE0UL) &&
              (((rx_eth->dst_mac&0xFFFF7F000000ULL)>>16) == (rx_ip->dst_ip&0xFFFF7F00UL))) //multicast
      {
        switch(rx_ip->proto)
        {
          //case IP_PROTO_ICMP: icmp_service(); break;
          //case IP_PROTO_TCP:  tcp_service();  break;
          case IP_PROTO_UDP:  udp_service();  break;
        }
      }
      break;

    case ETH_TYPE_ARP:
      arp_service();
      break;
  }

  return;
}


void eth_setname(char *name) //set NB & UPnP name (upper case)
{
  unsigned int i, len;

  len = strlen(name);
  if(len == 0)
  {
    return;
  }
  else if(len >= 16)
  {
    len = 15;
  }

  for(i=0; i<len; i++)
  {
    dev.name[i] = toupper(name[i]); //copy name and convert to uppercase
  }
  dev.name[i] = 0;

  return;
}

void         eth_setsummer(int on)          { dev.summer   = on; }
void         eth_settimediffh(int hours)    { dev.timediff = hours*3600; }
void         eth_settimediff(int seconds)   { dev.timediff = seconds; }
void         eth_setntp(IP_Addr ntp)        { dev.ntp      = ntp; }
void         eth_setdns(IP_Addr dns)        { dev.dns      = dns; }
unsigned int eth_setdhcp(int on)            { dev.dhcp     = on; 
                                              if(on){ return dhcp_getcfg(); }
                                              else  { return 1; } }
void         eth_setrouter(IP_Addr r)       { dev.router   = r; }
void         eth_setnetmask(IP_Addr nm)     { dev.netmask  = nm; }
void         eth_setip(IP_Addr ip)          { dev.ip       = ip; }
void         eth_setmac(MAC_Addr mac)       { dev.mac      = mac; ethernet_setmac(mac); }


char*        eth_getname(void)              { return dev.name; }
int          eth_getsummer(void)            { return dev.summer; }
int          eth_gettimediffh(void)         { return dev.timediff/3600; }
int          eth_gettimediff(void)          { return dev.timediff; }
IP_Addr      eth_getntp(void)               { return dev.ntp; }
IP_Addr      eth_getdns(void)               { return dev.dns; }
int          eth_getdhcp(void)              { return dev.dhcp; }
IP_Addr      eth_getrouter(void)            { return dev.router; }
IP_Addr      eth_getnetmask(void)           { return dev.netmask; }
IP_Addr      eth_getip(void)                { return dev.ip; }
MAC_Addr     eth_getmac(void)               { return dev.mac; }


void eth_init(void)
{
  DEBUGOUT("Eth: init\n");

  requested_mac = 0UL;
  memset(&dev, 0, sizeof(dev));
  memset(&tcp_table, 0, sizeof(tcp_table));
  memset(&udp_table, 0, sizeof(udp_table));

  eth_setname(APPNAME);
  eth_setmac(atomac(DEFAULT_MAC));

  return;
}
