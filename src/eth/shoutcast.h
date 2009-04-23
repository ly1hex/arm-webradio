#ifndef _SHOUTCAST_H_
#define _SHOUTCAST_H_


//----- DEFINES -----
#define SHOUTCAST_PORT                 (1000)

#define SHOUTCAST_TIMEOUT              (10) //s

#define SHOUTCAST_TRY                  (3)    //times

#define SHOUTCAST_CLOSED               (0)
#define SHOUTCAST_CLOSE                (1)
#define SHOUTCAST_OPENED               (2)
#define SHOUTCAST_ERROR                (3)
#define SHOUTCAST_SERVERFULL           (4)
#define SHOUTCAST_HEADER               (5)
#define SHOUTCAST_OPEN                 (6)


//----- PROTOTYPES -----
unsigned int                           shoutcast_getstatus(void);
void                                   shoutcast_close(void);
unsigned int                           shoutcast_open(IP_Addr ip, unsigned int port, char *file);
void                                   shoutcast_putogg(const unsigned char *s, unsigned int len);
void                                   shoutcast_putdata(const unsigned char *s, unsigned int len);
unsigned int                           shoutcast_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_SHOUTCAST_H_
