#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../tools.h"
#include "../main.h"
#include "../eth.h"
#include "utils.h"
#include "upnp.h"
#include "ssdp.h"


void ssdp_advertise(void)
{
  unsigned int index, len;
  char *ssdp;

  if(uuid_test(upnp_uuid()) == 0)
  {
    uuid_generate(upnp_uuid());
  }

  ssdp = (char*) &eth_txbuf[SSDP_OFFSET];

  len = sprintf(ssdp, "NOTIFY * HTTP/1.0\r\n"
                      "Host: %i.%i.%i.%i:%i\r\n"
                      "Cach-Control: max-age=900\r\n"
                      "Location: http://%i.%i.%i.%i:%i/device.xml\r\n"
                      "NT: upnp:rootdevice\r\n"
                      "USN: uuid:%s::upnp:rootdevice\r\n"
                      "NTS: ssdp:alive\r\n"
                      "Server: "APPNAME"/"APPVERSION" UPnP/1.0 %s\r\n\r\n", ((SSDP_MULTICAST>>0)&0xff), ((SSDP_MULTICAST>>8)&0xff), ((SSDP_MULTICAST>>16)&0xff), ((SSDP_MULTICAST>>24)&0xff), SSDP_PORT, ((eth_ip()>>0)&0xff), ((eth_ip()>>8)&0xff), ((eth_ip()>>16)&0xff), ((eth_ip()>>24)&0xff), UPNP_PORT, upnp_uuid(), eth_name());

  index = udp_open(UDP_ENTRIES, MULTICAST_MAC(SSDP_MULTICAST), SSDP_MULTICAST, SSDP_PORT, SSDP_PORT, 0, len);
  udp_close(index);

  DEBUGOUT("SSDP: advertise\n");

  return;
}


void ssdp_udpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  char *ssdp;

  DEBUGOUT("SSDP: UDP app\n");

  ssdp = (char*) rx;

  if(strncmpi(ssdp, "M-SEARCH *", 10) == 0)
  {
    ssdp_advertise();
  }

  return;
}
