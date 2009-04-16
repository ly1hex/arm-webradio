#ifndef _BUFFER_H_
#define _BUFFER_H_


//----- DEFINES -----
#define VS_BUFSIZE                     (55000) //bytes (vs buffer)

#define CARD_READBUF                   (512)


typedef union
{
  uint8_t  b8[VS_BUFSIZE];
  uint16_t b16[VS_BUFSIZE/2];
  uint32_t b32[VS_BUFSIZE/4];
} VSBUFFER;

typedef struct
{
  char name[MAX_NAME];
  char info[MAX_INFO];
  char file[MAX_ADDR];
} MENUBUFFER;

typedef struct
{
  char name[MAX_NAME];
  char info[MAX_INFO];
  char addr[MAX_ADDR];
} STATIONBUFFER;

typedef struct
{
  char name[MAX_NAME];
  char info[MAX_INFO];
  char file[MAX_ADDR];
} SHAREBUFFER;

typedef struct
{
  char          name[MAX_NAME];
  char          info[MAX_INFO];
  char          file[MAX_ADDR];
  FIL           fsrc;
  unsigned char buf[CARD_READBUF];
} CARDBUFFER;

typedef union
{
  MENUBUFFER    menu;
  STATIONBUFFER station;
  SHAREBUFFER   share;
  CARDBUFFER    card;
} BUFFER;


//----- GLOBALS -----
extern VSBUFFER vsbuf;
extern volatile unsigned int vsbuf_head, vsbuf_tail;
extern BUFFER gbuf;


//----- PROTOTYPES -----
unsigned int                           vsbuf_free(void);
unsigned int                           vsbuf_len(void);
void                                   vsbuf_puts(const unsigned char *s, unsigned int len);
void                                   vsbuf_putc(unsigned char c);
void                                   vsbuf_gets(unsigned char *s, unsigned int len);
unsigned char                          vsbuf_getc(void);
void                                   vsbuf_init(void);


#endif //_BUFFER_H_
