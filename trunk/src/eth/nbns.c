#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "nbns.h"


void nbns_reply(unsigned int index, unsigned int id)
{
  NBNS_Header *tx_nbns;

  if(index >= UDP_ENTRIES)
  {
    return;
  }

  tx_nbns = (NBNS_Header*) &eth_txbuf[NBNS_OFFSET];

  memset(tx_nbns, 0, NBNS_HEADERLEN+NBNSA_HEADERLEN);

  tx_nbns->id            = swap16(id);
  tx_nbns->flags_op      = SWAP16(NBNS_FLAG_RESPONSE|NBNS_FLAG_AUTHORITY);
  tx_nbns->ancount       = SWAP16(0x0001);
  tx_nbns->data.an.len   = 32;
  tx_nbns->data.an.type  = SWAP16(NBNSA_TYPE_NB);
  tx_nbns->data.an.clas  = SWAP16(NBNSA_CLASS_IN);
  tx_nbns->data.an.ttl   = SWAP32(3600); //3600 sec = 1 hour
  tx_nbns->data.an.rdlen = SWAP16(6);
  tx_nbns->data.an.flags = SWAP16(0);
  tx_nbns->data.an.addr  = eth_ip();

  nbns_encode(tx_nbns->data.an.name, eth_name(), 0x00); //0x00 = Workstation
  tx_nbns->data.an.name[32] = 0;

  udp_send(index, NBNS_HEADERLEN+NBNSA_HEADERLEN);

  DEBUGOUT("Eth: NBNS reply\n");

  return;
}


void nbns_udpapp(unsigned int index)
{
  NBNS_Header *rx_nbns;
  unsigned int type;
  char name[16];

  DEBUGOUT("NBNS: UDP app\n");

  rx_nbns = (NBNS_Header*) &eth_rxbuf[NBNS_OFFSET];

  if(((swap16(rx_nbns->flags_op)&NBNS_FLAG_RESPONSE) == 0) &&
     ((swap16(rx_nbns->flags_op)&NBNS_OPMASK)        == 0) &&
      (swap16(rx_nbns->qdcount)      == 1)                 &&
      (rx_nbns->data.qd.len          == 32)                &&
      (swap16(rx_nbns->data.qd.type) == NBNSQ_TYPE_NB)     &&
      (swap16(rx_nbns->data.qd.clas) == NBNSQ_CLASS_IN))
  {
    type = nbns_decode(name, rx_nbns->data.an.name);
    DEBUGOUT("Eth: NBNS %i %s\n", type, name);
    if(type == 0x00) //0x00 = Workstation
    {
      if(strcmp(name, eth_name()) == 0)
      {
        nbns_reply(index, swap16(rx_nbns->id));
      }
    }
  }

  return;
}
