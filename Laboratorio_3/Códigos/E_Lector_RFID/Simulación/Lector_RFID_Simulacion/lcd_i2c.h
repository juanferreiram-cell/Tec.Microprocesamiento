#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <avr/io.h>
#define LCD_I2C_ADDRESS 0x20


void i2c_init(void);
void lcd_init(void);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_clear(void);
void lcd_gotoxy(uint8_t x, uint8_t y);
void lcd_puts(const char *s);

#endif