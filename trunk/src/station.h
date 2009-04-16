#ifndef _STATION_H_
#define _STATION_H_


//----- DEFINES -----
#define STATION_FILE                   "STATION.PLS"

#define STATION_TIMEOUT                (12) //s (play->buffer->play)
#define STATION_TRY                    (4)  //x times

#define STATION_BUFMIN                 (VS_BUFSIZE*10UL/100UL) //10% go into buffer mode
#define STATION_BUFPLAY                (VS_BUFSIZE*60UL/100UL) //60% go back to play mode
#define STATION_BUFSTART               (VS_BUFSIZE*90UL/100UL) //90% 1st play buffer

#define STATION_CLOSED                 (0)
#define STATION_OPENED                 (1)
#define STATION_BUFFER                 (2)
#define STATION_OPEN                   (3)


//----- PROTOTYPES -----
void                                   station_calcbuf(unsigned int br);
void                                   station_close(void);
unsigned int                           station_open(unsigned int item);
void                                   station_service(void);
void                                   station_closeitem(void);
unsigned int                           station_openitem(unsigned int item);
unsigned int                           station_getitemaddr(unsigned int item, char *addr);
void                                   station_getitem(unsigned int item, char *name);
unsigned int                           station_items(void);
void                                   station_init(void);


#endif //_STATION_H_
