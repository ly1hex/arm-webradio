#ifndef _LCD_H_
#define _LCD_H_


//----- DEFINES -----
#define LCD_WIDTH                      (176)
#define LCD_HEIGHT                     (132)
#define RGB(r,g,b)                     (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue


//----- PROTOTYPES -----
void                                   lcd_img32(int x, unsigned int y, const unsigned char *img, unsigned int color, unsigned int bg_color);
void                                   lcd_putline(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bg_color);
unsigned int                           lcd_puts(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bg_color);
unsigned int                           lcd_putc(unsigned int x, unsigned int y, unsigned int c, unsigned int font, unsigned int color, unsigned int bg_color);
void                                   lcd_rect(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_rectedge(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_line(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_pixel(unsigned int x, unsigned int y, unsigned int color);
void                                   lcd_clear(unsigned int color);
void                                   lcd_init(void);


#endif //_LCD_H_
