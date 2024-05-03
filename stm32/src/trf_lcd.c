#include "trf_lcd.h"

#include "stm32h7xx_hal.h"

extern UART_HandleTypeDef huart4;

uint8_t tx_buffer [50];

void lcd_screen_init(void) {
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x52;
	tx_buffer[2] = 40;
	lcd_send_cmd(3); //Set contrast of LCD

	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x53;
	tx_buffer[2] = 5;//Set LCD back-light
	lcd_send_cmd(3);
}

void lcd_send_msg(char *msg) {
    int i;
	for (i = 0; msg[i] != '\0'; i++)
	{
		tx_buffer[i] = msg[i];
	}
	lcd_send_cmd(i);
}

void lcd_clear(void) {
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x51;
	lcd_send_cmd(2);
}

void lcd_reset_cursor(void) {
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x46;
	lcd_send_cmd(2);
}

void lcd_setline1() {
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x45;
	tx_buffer[2] = 0;
	lcd_send_cmd(3); //Sets cursor to start of line 1
}

void lcd_setline2() {
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x45;
	tx_buffer[2] = 0x40;
	lcd_send_cmd(3); //Sets cursor to start of line 2
}


void lcd_send_cmd(int x) {
	for (int i = 0; i < x; i++) {
		HAL_UART_Transmit(&huart4, &tx_buffer[i], 1, 10);
	}
	HAL_Delay(5);
}