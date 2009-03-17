#ifndef _UPNP_H_
#define _UPNP_H_


//----- DEFINES -----
#define UPNP_PORT                      (8080)


//----- PROTOTYPES -----
unsigned int                           upnp_port(void);
char*                                  upnp_uuid(void);
unsigned int                           upnp_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_UPNP_H_
