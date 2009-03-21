#ifndef _MMC_H_
#define _MMC_H_


//----- DEFINES -----
//MMC/SD command
#define CMD0                           (0x40+ 0) //GO_IDLE_STATE
#define CMD1                           (0x40+ 1) //SEND_OP_COND
#define	ACMD41                         (0xC0+41) //SEND_OP_COND (SDC)
#define CMD8                           (0x40+ 8) //SEND_IF_COND
#define CMD9                           (0x40+ 9) //SEND_CSD
#define CMD10                          (0x40+10) //SEND_CID
#define CMD12                          (0x40+12) //STOP_TRANSMISSION
#define ACMD13                         (0xC0+13) //SD_STATUS (SDC)
#define CMD16                          (0x40+16) //SET_BLOCKLEN
#define CMD17                          (0x40+17) //READ_SINGLE_BLOCK
#define CMD18                          (0x40+18) //READ_MULTIPLE_BLOCK
#define CMD23                          (0x40+23) //ET_BLOCK_COUNT
#define	ACMD23                         (0xC0+23) //SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24                          (0x40+24) //WRITE_BLOCK
#define CMD25                          (0x40+25) //WRITE_MULTIPLE_BLOCK
#define CMD41                          (0x40+41) //SEND_OP_COND (ACMD)
#define CMD55                          (0x40+55) //APP_CMD
#define CMD58                          (0x40+58) //READ_OCR


//----- PROTOTYPES -----
unsigned int                           ini_getentry(const char *filename, const char *entry, char *value, unsigned int len);

unsigned int                           fs_isdir(char *path, unsigned int item);
void                                   fs_getitemtag(char *path, unsigned int item, char *name);
void                                   fs_getitem(char *path, unsigned int item, char *name);
unsigned int                           fs_items(char *path);
unsigned int                           fs_checkitem(FILINFO *finfo);
void                                   fs_unmount(void);
void                                   fs_mount(void);


#endif //_MMC_H_
