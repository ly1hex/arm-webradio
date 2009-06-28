#ifndef _LCD_L2F50_H_
#define _LCD_L2F50_H_


//----- PROTOTYPES -----
#if defined(L2F50)
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y);
void                                   lcd_cursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);
#endif


#endif //_LCD_L2F50_H_
