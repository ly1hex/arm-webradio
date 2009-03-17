#ifndef _LCD_LS020_H_
#define _LCD_LS020_H_


//----- PROTOTYPES -----
#if defined(LS020)
inline void                            lcd_draw(unsigned int color);
inline void                            lcd_drawstop(void);
inline void                            lcd_drawstart(void);
inline void                            lcd_area(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y);
inline void                            lcd_cursor(unsigned int x, unsigned int y);
inline void                            lcd_cmd(unsigned int reg, unsigned int param);
inline void                            lcd_data(unsigned int c);
void                                   lcd_reset(void);
#endif


#endif //_LCD_LS020_H_
