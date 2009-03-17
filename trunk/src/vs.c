#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "third_party/lmi/inc/hw_types.h"
#include "third_party/lmi/inc/hw_memmap.h"
#include "third_party/lmi/driverlib/gpio.h"
#include "third_party/lmi/driverlib/interrupt.h"
#include "third_party/lmi/driverlib/ssi.h"
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "buffer.h"
#include "vs.h"


int vs_vol=0, vs_sbamp=0, vs_sbfreq=0, vs_stamp=0, vs_stfreq=0;
unsigned int vs_playing=0;

//VS1053: OGG Vorbis Decoder Patch 1.0
#define VS1053_PLUGINLEN (396)
#if (VS1053_PLUGINLEN != 0)
const unsigned short vs1053_plugin[VS1053_PLUGINLEN] =
{
  0x0007, 0x0001, 0x8300, 0x0006, 0x0184, 0xb080, 0x0024, 0x0007, /*    0 */
  0x9257, 0x3f00, 0x0024, 0x0006, 0x0017, 0x3f10, 0x0024, 0x3f00, /*    8 */
  0x0024, 0x0000, 0xf057, 0xf400, 0x55c0, 0x0000, 0x0817, 0xf400, /*   10 */
  0x57c0, 0x0000, 0x004d, 0x000a, 0x708f, 0x0000, 0xc3ce, 0x280f, /*   18 */
  0xe100, 0x0006, 0x2016, 0x0000, 0x028d, 0x280a, 0x71d5, 0x002c, /*   20 */
  0x9d40, 0x0000, 0xfa0d, 0x0039, 0x324f, 0x0000, 0xcd8e, 0x2920, /*   28 */
  0x41c0, 0x0000, 0x0024, 0x000a, 0x708f, 0x0000, 0xc3ce, 0x280a, /*   30 */
  0xcac0, 0x0000, 0x028d, 0x3613, 0x0024, 0x3e10, 0xb803, 0x3e14, /*   38 */
  0x3811, 0x3e11, 0x3805, 0x3e00, 0x3801, 0x0007, 0xc390, 0x0006, /*   40 */
  0xa011, 0x3010, 0x0444, 0x3050, 0x4405, 0x6458, 0x0302, 0xff94, /*   48 */
  0x4081, 0x0003, 0xffc5, 0x48b6, 0x0024, 0xff82, 0x0024, 0x42b2, /*   50 */
  0x0042, 0xb458, 0x0003, 0x4cd6, 0x9801, 0xf248, 0x1bc0, 0xb58a, /*   58 */
  0x0024, 0x6de6, 0x1804, 0x0006, 0x0010, 0x3810, 0x9bc5, 0x3800, /*   60 */
  0xc024, 0x36f4, 0x1811, 0x36f0, 0x9803, 0x283e, 0x2d80, 0x0fff, /*   68 */
  0xffc3, 0x003e, 0x2d4f, 0x2800, 0xdf00, 0x0000, 0xc6ce, 0x3413, /*   70 */
  0x0024, 0x2800, 0xcf85, 0xf400, 0x4510, 0x2800, 0xd340, 0x6894, /*   78 */
  0x13cc, 0x3000, 0x184c, 0x6090, 0x93cc, 0x38b0, 0x3812, 0x3004, /*   80 */
  0x4024, 0x0000, 0x0910, 0x3183, 0x0024, 0x3100, 0x4024, 0x6016, /*   88 */
  0x0024, 0x000c, 0x8012, 0x2800, 0xd291, 0xb884, 0x104c, 0x6894, /*   90 */
  0x3002, 0x2939, 0xb0c0, 0x3e10, 0x93cc, 0x4084, 0x9bd2, 0x4282, /*   98 */
  0x0024, 0x0000, 0x0041, 0x2800, 0xd545, 0x6212, 0x0024, 0x0000, /*   a0 */
  0x0040, 0x2800, 0xda45, 0x000c, 0x8390, 0x2a00, 0xddc0, 0x34c3, /*   a8 */
  0x0024, 0x3444, 0x0024, 0x3073, 0x0024, 0x3053, 0x0024, 0x3000, /*   b0 */
  0x0024, 0x6092, 0x098c, 0x0000, 0x0241, 0x2800, 0xddc5, 0x32a0, /*   b8 */
  0x0024, 0x6012, 0x0024, 0x0000, 0x0024, 0x2800, 0xddd5, 0x0000, /*   c0 */
  0x0024, 0x3613, 0x0024, 0x3001, 0x3844, 0x2920, 0x0580, 0x3009, /*   c8 */
  0x3852, 0xc090, 0x9bd2, 0x2800, 0xddc0, 0x3800, 0x1bc4, 0x000c, /*   d0 */
  0x4113, 0xb880, 0x2380, 0x3304, 0x4024, 0x3800, 0x05cc, 0xcc92, /*   d8 */
  0x05cc, 0x3910, 0x0024, 0x3910, 0x4024, 0x000c, 0x8110, 0x3910, /*   e0 */
  0x0024, 0x39f0, 0x4024, 0x3810, 0x0024, 0x38d0, 0x4024, 0x3810, /*   e8 */
  0x0024, 0x38f0, 0x4024, 0x34c3, 0x0024, 0x3444, 0x0024, 0x3073, /*   f0 */
  0x0024, 0x3063, 0x0024, 0x3000, 0x0024, 0x4080, 0x0024, 0x0000, /*   f8 */
  0x0024, 0x2839, 0x53d5, 0x4284, 0x0024, 0x3613, 0x0024, 0x2800, /*  100 */
  0xe105, 0x6898, 0xb804, 0x0000, 0x0084, 0x293b, 0x1cc0, 0x3613, /*  108 */
  0x0024, 0x000c, 0x8197, 0x0000, 0x0015, 0x408a, 0x9bc4, 0x3f15, /*  110 */
  0x4024, 0x2800, 0xe405, 0x4284, 0x3c15, 0x6590, 0x0024, 0x0000, /*  118 */
  0x0024, 0x2839, 0x53d5, 0x4284, 0x0024, 0x0000, 0x0024, 0x2800, /*  120 */
  0xce58, 0x458a, 0x0024, 0x2a39, 0x53c0, 0x3009, 0x3851, 0x3e14, /*  128 */
  0xf812, 0x3e12, 0xb817, 0x0006, 0xa057, 0x3e11, 0x9fd3, 0x0023, /*  130 */
  0xffd2, 0x3e01, 0x0024, 0x0006, 0x0011, 0x3111, 0x0024, 0x6498, /*  138 */
  0x07c6, 0x868c, 0x2444, 0x3901, 0x8e06, 0x0030, 0x0551, 0x3911, /*  140 */
  0x8e06, 0x3961, 0x9c44, 0xf400, 0x44c6, 0xd46c, 0x1bc4, 0x36f1, /*  148 */
  0xbc13, 0x2800, 0xef95, 0x36f2, 0x9817, 0x002b, 0xffd2, 0x3383, /*  150 */
  0x188c, 0x3e01, 0x8c06, 0x0006, 0xa097, 0x468c, 0xbc17, 0xf400, /*  158 */
  0x4197, 0x2800, 0xec84, 0x3713, 0x0024, 0x2800, 0xecc5, 0x37e3, /*  160 */
  0x0024, 0x3009, 0x2c17, 0x3383, 0x0024, 0x3009, 0x0c06, 0x468c, /*  168 */
  0x4197, 0x0006, 0xa052, 0x2800, 0xeec4, 0x3713, 0x2813, 0x2800, /*  170 */
  0xef05, 0x37e3, 0x0024, 0x3009, 0x2c17, 0x36f1, 0x8024, 0x36f2, /*  178 */
  0x9817, 0x36f4, 0xd812, 0x2100, 0x0000, 0x3904, 0x5bd1, 0x2a00, /*  180 */
  0xe50e, 0x000a, 0x0001, 0x0300
};
#endif

//VS1033: no plugin
#define VS1033_PLUGINLEN (0)
#if (VS1033_PLUGINLEN != 0)
const unsigned short vs1033_plugin[VS1033_PLUGINLEN] =
{
}
#endif


void vs_plugin(unsigned int vs)
{
  int i, len=0;
  unsigned short addr, n, val;
  const unsigned short *plugin;

  if(vs == 1033)       //vs1033
  {
#if (VS1033_PLUGINLEN != 0)
    plugin = vs1033_plugin;
    len    = VS1033_PLUGINLEN;
    DEBUGOUT("VS: load VS1033 plugin\n");
#endif
  }
  else if(vs == 1053) //vs1053
  {
#if (VS1053_PLUGINLEN != 0)
    plugin = vs1053_plugin;
    len    = VS1053_PLUGINLEN;
    DEBUGOUT("VS: load VS1053 plugin\n");
#endif
  }

  for(i=0; i < len;)
  {
    addr = plugin[i++];
    n    = plugin[i++];
    if(n & 0x8000U) //RLE run, replicate n samples
    {
      n  &= 0x7FFF;
      val = plugin[i++];
      while(n--) 
      {
        vs_write_reg(addr, val);
      }
    }
    else //copy run, copy n sample
    {
      while(n--)
      {
        val = plugin[i++];
        vs_write_reg(addr, val);
      }
    }
  }

  return;
}


int vs_treblefreq(void)
{
  return vs_stfreq*1000;
}


void vs_settreblefreq(int freq) //1000 - 15000Hz
{
  freq /= 1000;

  if(freq < 1) //< 1
  {
    freq = 1;
  }
  else if(freq > 15) //> 15
  {
    freq = 15;
  }
  vs_stfreq = freq;
  vs_write_bass();

  return;
}


int vs_trebleamp(void)
{
  return vs_stamp;
}


void vs_settrebleamp(int amp) //-8 - 7dB
{
  if(amp < -8) //< -8
  {
    amp = -8;
  }
  else if(amp > 7) //> 7
  {
    amp = 15;
  }
  vs_stamp = amp;
  vs_write_bass();

  return;
}


int vs_bassfreq(void)
{
  return vs_sbfreq*10;
}


void vs_setbassfreq(int freq) //20 - 150Hz
{
  freq /= 10;

  if(freq < 2) //< 2
  {
    freq = 2;
  }
  else if(freq > 15) //> 15
  {
    freq = 15;
  }
  vs_sbfreq = freq;
  vs_write_bass();

  return;
}


int vs_bassamp(void)
{
  return vs_sbamp;
}


void vs_setbassamp(int amp) //0 - 15dB
{
  if(amp < 0) //< 0
  {
    amp = 0;
  }
  else if(amp > 15) //> 15
  {
    amp = 15;
  }
  vs_sbamp = amp;
  vs_write_bass();

  return;
}


int vs_volume(void)
{
  return vs_vol;
}


void vs_setvolume(int vol) //0 - 100%
{
  if(vol <= 0) //<= 0
  {
    vs_write_reg(VS_VOL, 0xFFFF); //analog power off
  }
  else if(vol > 100) //> 100
  {
    vs_vol = 100;
    if(vs_playing)
    {
      vs_write_volume();
    }
  }
  else //1 - 99
  {
    vs_vol = vol;
    if(vs_playing)
    {
      vs_write_volume();
    }
  }

  return;
}


unsigned int vs_request(void)
{
  return VS_DREQ_READ(); //1=ready, 0=buf full
}


void vs_requesthandler(void)
{
  unsigned int len, tail;

  GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);

  len = vsbuf_len();
  if(len != 0)
  {
    if(len > 32)
    {
      len = 32;
    }
    vs_ssi_writewait(); //transmit fifo full?
    VS_DCS_DISABLE();
    tail = vsbuf_tail;
    VS_DCS_ENABLE();
    for(; len!=0; len--)
    {
      vs_ssi_write(vsbuf.b8[tail++]);
      if(tail >= VS_BUFSIZE)
      {
        tail = 0;
      }
    }
    vsbuf_tail = tail;
  }
  else
  {
    vs_pause();
  }

  return;
}


void vs_data(unsigned int c)
{
  VS_DCS_ENABLE();

  vs_write(c);

  VS_DCS_DISABLE();

  return;
}


void vs_write_bass(void)
{
  vs_write_reg(VS_BASS, ((vs_stamp&0x0f)<<12)|((vs_stfreq&0x0f)<<8)|((vs_sbamp&0x0f)<<4)|((vs_sbfreq&0x0f)<<0));

  return;
}


void vs_write_volume(void)
{
  int vol;

  vol = 100-vs_vol;
  vs_write_reg(VS_VOL, (vol<<8)|(vol<<0));

  return;
}


unsigned long vs_read_ram(unsigned int addr)
{
  unsigned long ret;

  vs_write_reg(VS_WRAMADDR, addr);
  ret  = vs_read_reg(VS_WRAM)<<16;
  ret |= vs_read_reg(VS_WRAM);

  return ret;
}


void vs_write_reg(unsigned int reg, unsigned int data)
{
  unsigned int timeout;

  IntMasterDisable();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_ENABLE();
  vs_write(VS_WRITE);
  vs_write(reg);
  vs_write(data>>8);
  vs_write(data);
  VS_CS_DISABLE();

  //execution -> DREQ low
  switch(reg)
  {
    case VS_MODE:   timeout = 30000; break;
    case VS_STATUS: timeout =   200; break;
    case VS_BASS:   timeout =   200; break;
    case VS_VOL:    timeout =   200; break;
    default:        timeout =  1000; break;
  }
  for(; timeout!=0; timeout--)
  {
    if(vs_request())
    {
      break;
    }
  }

  IntMasterEnable();

  return;
}


unsigned int vs_read_reg(unsigned int reg)
{
  unsigned int ret, timeout;

  IntMasterDisable();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_ENABLE();
  vs_write(VS_READ);
  vs_write(reg);
  ret  = vs_read()<<8;
  ret |= vs_read();
  VS_CS_DISABLE();

  //execution -> DREQ low
  switch(reg)
  {
    case VS_MODE:   timeout = 30000; break;
    case VS_STATUS: timeout =   200; break;
    case VS_BASS:   timeout =   200; break;
    case VS_VOL:    timeout =   200; break;
    default:        timeout =  1000; break;
  }
  for(; timeout!=0; timeout--)
  {
    if(vs_request())
    {
      break;
    }
  }

  IntMasterEnable();

  return ret;
}


void vs_write(unsigned int c)
{
  vs_ssi_readwrite(c);

  return;
}


unsigned int vs_read(void)
{
  return vs_ssi_readwrite(0xff);
}


void vs_pause(void)
{
  GPIOPinIntDisable(GPIO_PORTA_BASE, GPIO_PIN_1); //disable dreq irq
  GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);

  return;
}


void vs_play(void)
{
  if(vs_request() && vs_playing)
  {
    GPIOPinIntEnable(GPIO_PORTA_BASE, GPIO_PIN_1); //enable dreq irq
  }

  return;
}


void vs_stopstream(void)
{
  unsigned int i, timeout;

  vs_pause();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_DISABLE();

  //cancel playback
  vs_write_reg(VS_MODE, SM_SDINEW | SM_CANCEL);
  for(timeout=100; (timeout!=0) && (vs_read_reg(VS_MODE) & SM_CANCEL); timeout--)
  {
    VS_DCS_ENABLE();
    for(i=32; i!=0; i--)
    {
      vs_ssi_write(vsbuf_getc());
    }
    vs_ssi_writewait();
    VS_DCS_DISABLE();
  }

  //flush buffer
  VS_DCS_ENABLE();
  for(i=2052; i!=0; i--)
  {
    vs_ssi_write(0x00);
  }
  vs_ssi_writewait();
  VS_DCS_DISABLE();

  //check status -> soft reset
  if(vs_read_reg(VS_HDAT0) || vs_read_reg(VS_HDAT1))
  {
    vs_write_reg(VS_MODE, SM_SDINEW | SM_RESET);
    DEBUGOUT("VS: soft reset\n");
  }

  return;
}


void vs_stop(void)
{
  DEBUGOUT("VS: stop\n");

  vs_playing = 0;

  USB_OFF();

  vs_pause();
  vs_setvolume(0);
  vs_stopstream();

  vsbuf_init();

  return;
}


void vs_start(void)
{
  DEBUGOUT("VS: start\n");

  vs_playing = 1;

  USB_ON();

  vs_pause();
  vs_setvolume(vs_vol);

  vsbuf_init();

  return;
}


void vs_init(unsigned int vs) //vs = 1033 or 1053
{
  DEBUGOUT("VS: init (%i)\n", vs);

  if((vs != 1033) && (vs != 1053))
  {
    vs = DEFAULT_VS;
  }

  vs_playing = 0;

  vsbuf_init();

  //ssi speed down
  vs_ssi_speed(2000000); //2 MHz

  //hard reset
  VS_CS_DISABLE();
  VS_DCS_DISABLE();
  VS_RST_ENABLE();
  delay_ms(5);
  VS_RST_DISABLE();
  delay_ms(5);

  //set registers
  vs_write_reg(VS_MODE, SM_SDINEW);
  vs_write_reg(VS_CLOCKF, 0x1800+SC_MUL_4X); //x4 = 48 MHz

  //load plugin
  vs_plugin(vs);

  //set volume, bass, treble
  vs_setvolume(DEFAULT_VOLUME);
  vs_setbassfreq(DEFAULT_BASSFREQ);
  vs_setbassamp(DEFAULT_BASSAMP);
  vs_settreblefreq(DEFAULT_TREBLEFREQ); 
  vs_settrebleamp(DEFAULT_TREBLEAMP);
  vs_setvolume(0); //0 -> analog power off

  //ssi speed up
  vs_ssi_speed(0); //0 = default speed

  //init pin interrupt
  GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_1, GPIO_HIGH_LEVEL);
  GPIOPortIntRegister(GPIO_PORTA_BASE, vs_requesthandler);

  return;
}
