#ifndef _LCD_MIO283QT_H_
#define _LCD_MIO283QT_H_


//----- DEFINES -----
#define LCD_ID                         (0)
#define LCD_DATA                       ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER                   ((0x70)|(LCD_ID<<2))


//----- PROTOTYPES -----
#if defined(MIO283QT)
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y);
void                                   lcd_cursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);
#endif


#endif //_LCD_MIO283QT_H_
