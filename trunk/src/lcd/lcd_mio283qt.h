#ifndef _LCD_MIO283QT_H_
#define _LCD_MIO283QT_H_


#ifdef LCD_MIO283QT

//----- DEFINES -----
#define LCD_ID                         (0)
#define LCD_DATA                       ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER                   ((0x70)|(LCD_ID<<2))


//----- PROTOTYPES -----
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_area(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void                                   lcd_cursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);

#endif


#endif //_LCD_MIO283QT_H_
