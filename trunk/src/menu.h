#ifndef _MENU_H_
#define _MENU_H_


//----- DEFINES -----
#define DEFAULT_BGCOLOR                RGB(  0,  0,  0) //background
#define DEFAULT_FGCOLOR                RGB(255,255, 50) //foreground
#define DEFAULT_SELCOLOR               RGB(255,255,255) //selection
#define DEFAULT_EDGECOLOR              RGB(120,120,100) //edges
#define DEFAULT_SBCOLOR                RGB(255,255,255) //standby

#define MENU_LINES                     (8)  //lines
#define MENU_LINEHEIGHT                (16) //pixel

#define SMALLFONT                      (0)
#define NORMALFONT                     (1)
#define TIMEFONT                       (2)

#define MENU_TOP                       (40)
#define MENU_TEXTX                     (60)
#define MENU_TEXTY                     (85)
#define MENU_MIDDLEITEM                ((LCD_WIDTH/2)-(32/2))
#define MENU_RIGHTITEM                 (LCD_WIDTH-32)

#define MENU_PLAY                      (0)
#define MENU_UPDATE                    (1)
#define MENU_ERROR                     (2)
#define MENU_BACK                      (3)
#define MENU_BACKTXT                   "<< back <<"

#define MENU_STATE_STOP                (0)
#define MENU_STATE_BUF                 (1)
#define MENU_STATE_PLAY                (2)



typedef struct
{
  const char *name;
  const unsigned char *img[3];
  void(*init)(void);                      //init routine
  unsigned int(*items)(void);             //get item count
  void(*get)(unsigned int item, char *s); //get item name
  unsigned int(*open)(unsigned int item); //open item (ret: 0=play, 1=update, 2=error, 3=back)
  void(*close)(void);                     //close item
  void(*service)(void);                   //service routine
} MAINMENUITEM;


//----- PROTOTYPES -----
unsigned int                           menu_openfile(char *file);
unsigned int                           menu_sw(void);
unsigned int                           menu_swlong(void);
void                                   menu_up(void);
void                                   menu_down(void);
void                                   menu_steps(int steps);
void                                   menu_service(unsigned int draw);
void                                   menu_popup(char *s);
void                                   menu_drawsub(unsigned int redraw);
void                                   menu_drawmain(unsigned int redraw);
void                                   menu_drawclock(void);
void                                   menu_drawdate(void);
void                                   menu_drawvol(void);
void                                   menu_drawstatus(void);
void                                   menu_setinfo(unsigned int status, const char *info);
void                                   menu_drawinfo(unsigned int redraw);
void                                   menu_drawbg(void);
void                                   menu_update(unsigned int redraw);
void                                   menu_setcolors(unsigned int bg, unsigned int fg, unsigned int sel, unsigned int edge);
void                                   menu_init(void);


#endif //_MENU_H_
