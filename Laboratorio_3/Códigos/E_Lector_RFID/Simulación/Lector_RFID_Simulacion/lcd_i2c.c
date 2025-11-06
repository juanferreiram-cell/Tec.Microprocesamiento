#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "lcd_i2c.h"
#include <util/delay.h>

#define SCL_CLOCK 100000L
#define I2C_WRITE_ADDR (LCD_I2C_ADDRESS << 1)


void i2c_init(void) {
	TWSR = 0;
	TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2;
}

static void i2c_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

static void i2c_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

static void lcd_i2c_write(uint8_t data, uint8_t rs) {
	uint8_t high_nibble = data & 0xF0;
	uint8_t low_nibble = (data << 4) & 0xF0;

	uint8_t rs_bit = (rs) ? (1 << 0) : 0;

	// Manda el nibble alto
	i2c_write(high_nibble | rs_bit | (1 << 3));
	i2c_write(high_nibble | rs_bit | (1 << 2) | (1 << 3));
	_delay_us(1);
	i2c_write(high_nibble | rs_bit | (1 << 3));
	_delay_us(50);
	
	// Manda el nibble bajo
	i2c_write(low_nibble | rs_bit | (1 << 3));
	i2c_write(low_nibble | rs_bit | (1 << 2) | (1 << 3));
	_delay_us(1);
	i2c_write(low_nibble | rs_bit | (1 << 3));
	_delay_us(50);
}

void lcd_send_cmd(char cmd) {
	i2c_start();
	i2c_write(I2C_WRITE_ADDR);
	lcd_i2c_write(cmd, 0);
	i2c_stop();
	_delay_us(50);
}

// Envía un caracter a la LCD
void lcd_send_data(char data) {
	i2c_start();
	i2c_write(I2C_WRITE_ADDR);
	lcd_i2c_write(data, 1);
	i2c_stop();
	_delay_us(50);
}



void lcd_init(void) {
	i2c_init();
	
	// Secuencia para inicializar
	_delay_ms(50);
	lcd_send_cmd(0x03);
	_delay_ms(5);
	lcd_send_cmd(0x03);
	_delay_us(100);
	lcd_send_cmd(0x03);
	_delay_ms(2);
	lcd_send_cmd(0x02); // Lo pone en 4 Bits
	_delay_ms(2);

	// Configuracion final
	lcd_send_cmd(0x28);
	_delay_ms(1);
	lcd_send_cmd(0x0C);
	_delay_ms(1);
	lcd_send_cmd(0x06);
	_delay_ms(1);
	lcd_clear();
	_delay_ms(2);
}

void lcd_clear(void) {
	lcd_send_cmd(0x01);
	_delay_ms(5);
}

void lcd_gotoxy(uint8_t x, uint8_t y) {
	uint8_t address = (y == 0) ? 0x80 : 0xC0;
	address += x;
	lcd_send_cmd(address);
}

void lcd_puts(const char *s) {
	while (*s) {
		lcd_send_data(*s++);
	}
}