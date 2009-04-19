#ifndef _LCD_H_
#define _LCD_H_


//----- DEFINES -----
#define SMALLFONT                      (0)
#define SMALLFONT_NAME                 font1
#define SMALLFONT_START                FONT1_START
#define SMALLFONT_WIDTH                FONT1_WIDTH
#define SMALLFONT_HEIGHT               FONT1_HEIGHT
#define NORMALFONT                     (1)
#define NORMALFONT_NAME                font2
#define NORMALFONT_START               FONT2_START
#define NORMALFONT_WIDTH               FONT2_WIDTH
#define NORMALFONT_HEIGHT              FONT2_HEIGHT
#define TIMEFONT                       (2)
#define TIMEFONT_NAME                  font3
#define TIMEFONT_START                 FONT3_START
#define TIMEFONT_WIDTH                 FONT3_WIDTH
#define TIMEFONT_HEIGHT                FONT3_HEIGHT

#define LCD_WIDTH                      (176)
#define LCD_HEIGHT                     (132)
#define RGB(r,g,b)                     (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue
#define GET_RED(x)                     ((x>>8)&0xF8) //5 red
#define GET_GREEN(x)                   ((x>>3)&0xFC) //6 green
#define GET_BLUE(x)                    ((x<<3)&0xF8) //5 blue


//----- PROTOTYPES -----
void                                   lcd_img32(int x, unsigned int y, const unsigned char *img, unsigned int color, unsigned int bgcolor);
void                                   lcd_putlinebr(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor);
void                                   lcd_putline(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor);
unsigned int                           lcd_puts(unsigned int x, unsigned int y, const unsigned char *s, unsigned int font, unsigned int color, unsigned int bgcolor);
unsigned int                           lcd_putc(unsigned int x, unsigned int y, unsigned int c, unsigned int font, unsigned int color, unsigned int bgcolor);
void                                   lcd_rect(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_rectedge(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_line(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned int color);
void                                   lcd_pixel(unsigned int x, unsigned int y, unsigned int color);
void                                   lcd_clear(unsigned int color);
void                                   lcd_init(void);


#endif //_LCD_H_
