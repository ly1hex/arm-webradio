#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "third_party/lmi/inc/hw_types.h"
#include "third_party/lmi/inc/hw_memmap.h"
#include "third_party/lmi/driverlib/gpio.h"
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "menu.h"
#include "mmc.h"


volatile DSTATUS Stat=STA_NOINIT;
volatile unsigned int Timer1=0, Timer2=0; //100Hz decrement timer
BYTE CardType=0; //b0:MMC, b1:SDC, b2:Block addressing
FATFS fatfs;
FIL file;
char lfn[_DF1S ? (_MAX_LFN * 2 + 1) : (_MAX_LFN + 1)];


void xmit_spi(BYTE dat)
{
  ssi_readwrite(dat);

  return;
}


BYTE rcvr_spi(void)
{
  DWORD dat;

  dat = ssi_readwrite(0xFF);

  return (BYTE)dat;
}


void rcvr_spi_m(BYTE *dst)
{
  *dst = rcvr_spi();

  return;
}

BYTE wait_ready(void)
{
  BYTE res;

  Timer2 = 50;	/* Wait for ready in timeout of 500ms */
  rcvr_spi();
  do
    res = rcvr_spi();
  while ((res != 0xFF) && Timer2);

  return res;
}


void release_spi(void)
{
  MMC_DESELECT();
  rcvr_spi();
  
  return;
}

void power_on(void)
{
  MMC_POWERON();
  MMC_DESELECT();

  return;
}


void power_off(void)
{
  MMC_SELECT(); //Wait for card ready
  wait_ready();
  release_spi();

//  MMC_POWEROFF();
  Stat |= STA_NOINIT; //Set STA_NOINIT

  return;
}


int chk_power(void)
{
  return 1; //Socket power state: 0=off, 1=on
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/
BOOL rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 10;
	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);
	if(token != 0xFE) return FALSE;	/* If not valid data token, retutn with error */

	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return TRUE;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
BOOL xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp, wc;


	if (wait_ready() != 0xFF) return FALSE;

	xmit_spi(token);					/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
		wc = 0;
		do {							/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Reveive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	MMC_DESELECT();
	MMC_SELECT();
	if (wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 16;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(BYTE drv)
{
    BYTE n, cmd, ty, ocr[4], init_try;

    if (drv) return STA_NOINIT; //Supports only single drive
    if (Stat & STA_NODISK) return Stat; //No card in the socket

    ssi_off(); //SCK, SI = low (and released from ssi)
    MMC_SELECT(); //CS = low
    MMC_POWEROFF(); //sd power off
    delay_ms(70);
    init_bor(0); //brown-out reset off
    delay_ms(10);
    MMC_POWERON(); //sd power on
    MMC_DESELECT(); //CS = high
    delay_ms(10);
    init_bor(1); //brown-out reset on
    delay_ms(40);

    //80 dummy clocks
    MMC_SI_HIGH();
    MMC_SCK_LOW();
    for(n=80; n; n--)
    {
      volatile unsigned int i;
      MMC_SCK_HIGH();
      for(i=10000; i; i--);
      MMC_SCK_LOW();
      for(i=10000; i; i--);
    }

    ssi_on(); //SCK, SI = ssi
    ssi_speed(250000); //ssi speed 250 kHz

    init_try = 3;
    do{
	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 100;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? 12 : 4;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = 2; cmd = ACMD41;	/* SDSC */
			} else {
				ty = 1; cmd = CMD1;		/* MMC */
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}

    }while((ty==0) && --init_try);

    CardType = ty;
    release_spi();

    //ssi speed up
    ssi_speed(0); //0 = default speed

    if (ty) {			/* Initialization succeded */
            Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
    } else {			/* Initialization failed */
            power_off();
    }

    return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & 8)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & 8)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & 6) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#if _USE_IOCTL != 0
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;


	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER) {
		switch (*ptr) {
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			if (chk_power())
				power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			power_on();				/* Power on */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = (BYTE)chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else {
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			MMC_SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC ver 1.XX or MMC*/
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & 4) {			/* SDC ver 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC ver 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & 2) {			/* SDC ver 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */
void disk_timerproc (void)
{
  unsigned int n;

  n = Timer1;						/* 100Hz decrement timer */
  if (n) Timer1 = --n;
  n = Timer2;
  if (n) Timer2 = --n;
}


DWORD get_fattime(void) //User Provided Timer Function for FatFs module
{
  return ((2008UL-1980) << 25) | // Year = 2008
          (         6UL << 21) | // Month = June
          (         5UL << 16) | // Day = 5
          (         11U << 11) | // Hour = 11
          (         38U <<  5) | // Min = 38
          (          0U >>  1);  // Sec = 0
}




unsigned int fs_isdir(const char *path, unsigned int item)
{
  FILINFO finfo;
  DIR dir;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        if(item == i)
        {
          if(finfo.fattrib & AM_DIR)
          {
            return 0;
          }
          break;
        }
        i++;
      }
    }
  }

  return 1;
}


/*
MP3 ID3 Tag v1
offset len
0 	3 	TAG -> ID3v1
3 	30 	Song title
33 	30 	Artist
63 	30 	Album
93 	4 	Year
97 	30 	Comment
127 	1 	Genre
*/
void fs_getitemtag(const char *path, unsigned int item, char *dst, unsigned int len)
{
  char tmp[MAX_ADDR];
  unsigned int rd;

  fs_getitem(path, item, dst, len);

  if(dst[0] == '/') //directory
  {
    return;
  }

  rd = strlen(dst);
  if((toupper(dst[rd-3]) == 'M') &&
     (toupper(dst[rd-2]) == 'P') &&
     (toupper(dst[rd-1]) == '3')) //MP3 file
  {
    strcpy(tmp, path);
    strcat(tmp, "/");
    strcat(tmp, dst);
    if(f_open(&file, tmp, FA_OPEN_EXISTING | FA_READ) == FR_OK)
    {
      if(f_lseek(&file, file.fsize-128) == FR_OK)
      {
        if(f_read(&file, tmp, 128, &rd) == FR_OK)
        {
          if((rd == 128) &&
             (tmp[0] == 'T') &&
             (tmp[1] == 'A') &&
             (tmp[2] == 'G'))
          {
            strncpy(dst, tmp+3, 30);
            dst[30] = 0;
          }
        }
      }
      f_close(&file);
    }
  }

  return;
}


void fs_getitem(const char *path, unsigned int item, char *dst, unsigned int len)
{
  FILINFO finfo;
  DIR dir;
  char *fname;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  *dst = 0;

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        if(item == i)
        {
#if _USE_LFN
          fname = (*finfo.lfname)?finfo.lfname:finfo.fname;
#else
          fname = finfo.fname;
#endif
          if(finfo.fattrib & AM_DIR)
          {
            *dst = '/';
            strncpy(dst+1, fname, len-1-1);
            dst[len-1] = 0;
          }
          else
          {
            strncpy(dst, fname, len-1);
            dst[len-1] = 0;
          }
          break;
        }
        i++;
      }
    }
  }

  return;
}


unsigned int fs_items(const char *path)
{
  FILINFO finfo;
  DIR dir;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  if(Stat & STA_NOINIT)
  {
    return 0;
  }

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        i++;
      }
    }
  }

  return i;
}


unsigned int fs_checkitem(FILINFO *finfo)
{
  unsigned int len;
  char c1, c2, c3;

  if(!(finfo->fattrib & (AM_HID|AM_SYS))) //no system and hidden files
  {
    if(finfo->fattrib & AM_DIR) //directory
    {
      return 0;
    }
    else //file
    {
      len = strlen(finfo->fname);
      c1 = toupper(finfo->fname[len-3]);
      c2 = toupper(finfo->fname[len-2]);
      c3 = toupper(finfo->fname[len-1]);

      if(finfo->fname[len-4] == '.')
      {
        if(((c1 == 'A') && (c2 == 'A') && (c3 == 'C')) || //AAC
           ((c1 == 'M') && (c2 == 'P') && (c3 == '3')) || //MP3
           ((c1 == 'O') && (c2 == 'G') && (c3 == 'G')) || //OGG
           ((c1 == 'W') && (c2 == 'A') && (c3 == 'V')) || //WAV
           ((c1 == 'W') && (c2 == 'M') && (c3 == 'A')))   //WMA
        {
          return 0;
        }
      }
    }
  }

  return 1;
}


void fs_unmount(void)
{
  f_mount(0, 0);

  return;
}


void fs_mount(void)
{
  if(Stat & STA_NOINIT)
  {
    if(disk_initialize(0) & STA_NOINIT)
    {
        return;
    }
  }

  f_mount(0, &fatfs);

  return;
}


unsigned int ini_searchentry(const char *filename, const char *entry)
{
  FRESULT res;
  unsigned int i, entry_len, found, rd;
  char c, buf[32];

  res = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ);
  if(res != FR_OK)
  {
    return 0;
  }

  entry_len = strlen(entry);
  i         = 0;
  found     = 0;
  do
  {
    res = f_read(&file, &c, 1, &rd);
    if((res != FR_OK) || (rd != 1))
    {
      break;
    }
    switch(c)
    {
      case '\r': //line end
      case '\n':
        i = 0;
        break;
      case '=':
        if(strncmp(buf, entry, entry_len) == 0)
        {
          found = file.fptr;
        }
        break;
      case '#': //comment
        if(i == 0)
        {
          while(1)
          {
            res = f_read(&file, &c, 1, &rd);
            if((res != FR_OK) || (rd != 1))
            {
              break;
            }
            else if((c == '\r') || (c == '\n'))
            {
              break;
            }
          }
          break;
        }
      default:
        if(c != ' ')
        {
          if(i<31)
          {
            buf[i++] = toupper(c);
            buf[i]   = 0;
          }
        }
        break;
    }
  }while(found == 0);

  f_close(&file);

  return found;
}


unsigned int ini_getentry(const char *filename, const char *entry, char *value, unsigned int len) //entry in upper case
{
  FRESULT res;
  unsigned int i, entry_len, found, rd;
  char c, *ptr;

  ptr  = value;
  *ptr = 0;

  found = ini_searchentry(filename, entry);
  if(found == 0)
  {
    return 1;
  }

  res = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ);
  if(res != FR_OK)
  {
    return 1;
  }
  res = f_lseek(&file, found);
  if(res != FR_OK)
  {
    return 1;
  }

  //read value
  if(len)
  {
    len--; //null at end
    while(len)
    {
      res = f_read(&file, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      else if((c == '\r') || (c == '\n'))
      {
        break;
      }
      else
      {
        *ptr++ = c;
        len--;
      }
    }
    *ptr = 0;
  }

  f_close(&file);

  //remove space at start and end
  strrmvspace(value, value);

  return 0;
}


unsigned int ini_setentry(const char *filename, const char *entry, const char *value)
{
  FRESULT res;
  unsigned int i, len, dif, found, rd;
  char c, buf[MAX_ADDR];

  found = ini_searchentry(filename, entry);
  if(found) //entry found
  {
    res = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if(res != FR_OK)
    {
      return 1;
    }
    res = f_lseek(&file, found);
    if(res != FR_OK)
    {
      return 1;
    }

    //calc current value size
    for(i=0; i<MAX_ADDR; i++)
    {
      res = f_read(&file, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      if((c == '\n') || (c == '\r'))
      {
        break;
      }
    }

    len = strlen(value); //new value len
    if(i == len) //same size
    {
      f_lseek(&file, found);
      f_puts(value, &file);
    }
    else if(i > len) //new value size is smaller
    {
      //write value to entry
      f_lseek(&file, found);
      f_puts(value, &file);
      //remove difference: old-new
      dif = i-len;
      for(i=file.fptr; (i+dif)<file.fsize; i++)
      {
        f_lseek(&file, i+dif);
        res = f_read(&file, &c, 1, &rd);
        if((res != FR_OK) || (rd != 1))
        {
          break;
        }
        f_lseek(&file, i);
        f_putc(c, &file);
      }
      f_truncate(&file);
    }
    else if(i < len) //new value size is bigger
    {
      //extend file: new-old
      dif = len-i;
      len = file.fsize-(file.fptr-1);
      for(i=file.fsize-1; len!=0; i--, len--)
      {
        f_lseek(&file, i);
        res = f_read(&file, &c, 1, &rd);
        if((res != FR_OK) || (rd != 1))
        {
          break;
        }
        f_lseek(&file, i+dif);
        f_putc(c, &file);
      }
      //write value to entry
      f_lseek(&file, found);
      f_puts(value, &file);
    }
    f_close(&file);
  }
  else
  {
    res = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if(res != FR_OK)
    {
      return 1;
    }
    res = f_lseek(&file, file.fsize);
    if(res != FR_OK)
    {
      return 1;
    }
    f_puts(entry, &file);
    f_puts("=", &file);
    f_puts(value, &file);
    f_puts("\r\n", &file);
    f_close(&file);
  }

  return 0;
}
