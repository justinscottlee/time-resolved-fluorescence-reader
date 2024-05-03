#ifndef TRF_LCD_H_
#define TRF_LCD_H_

void lcd_screen_init(void); //Must always be ran first
void lcd_send_msg(char *msg); //Sends message
void lcd_clear(void); //Clears entire screen
void lcd_reset_cursor(void); //Resets cursor to top left
void lcd_send_cmd(int x);
void lcd_setline1(); 
void lcd_setline2();

#endif