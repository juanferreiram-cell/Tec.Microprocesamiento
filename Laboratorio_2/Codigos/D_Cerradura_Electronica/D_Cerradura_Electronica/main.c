#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/pgmspace.h>


#define LCD_ADDR        0x27
#define LCD_BACKLIGHT   0x08
#define EN              0x04
#define LCD_RS          0x01
#define LCD_COMMAND     0x00
#define LCD_DATA        LCD_RS

void i2c_init() {
	
	TWSR = 0;
	TWBR = (uint8_t)(((F_CPU / 100000UL) - 16) / 2);
	TWCR = (1 << TWEN);
}


void i2c_start() {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}


void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}


void i2c_stop() {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}


void lcd_i2c_write(uint8_t data, uint8_t mode) {
	uint8_t high = data & 0xF0;
	uint8_t low = (data << 4) & 0xF0;
	uint8_t data_arr[4];
	data_arr[0] = high | mode | LCD_BACKLIGHT | EN;
	data_arr[1] = high | mode | LCD_BACKLIGHT;
	data_arr[2] = low | mode | LCD_BACKLIGHT | EN;
	data_arr[3] = low | mode | LCD_BACKLIGHT;

	i2c_start();
	i2c_write(LCD_ADDR << 1);
	for (uint8_t i = 0; i < 4; i++) {
		i2c_write(data_arr[i]);
	}
	i2c_stop();
}

void lcd_i2c_init() {
	_delay_ms(50);

	i2c_start();
	i2c_write(LCD_ADDR << 1);


	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT | EN)); _delay_us(1);
	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT));      _delay_ms(5);

	
	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT | EN)); _delay_us(1);
	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT));      _delay_us(100);

	
	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT | EN)); _delay_us(1);
	i2c_write((0x30 | LCD_COMMAND | LCD_BACKLIGHT));      _delay_us(100);

	
	i2c_write((0x20 | LCD_COMMAND | LCD_BACKLIGHT | EN)); _delay_us(1);
	i2c_write((0x20 | LCD_COMMAND | LCD_BACKLIGHT));      _delay_us(100);

	i2c_stop();

	
	lcd_i2c_write(0x28, LCD_COMMAND);
	lcd_i2c_write(0x0C, LCD_COMMAND);
	lcd_i2c_write(0x06, LCD_COMMAND);
	lcd_i2c_write(0x01, LCD_COMMAND);
	_delay_ms(2);
}



// Forma de Trabajar con String pero ccon memoria Flash debido a que el otro wrtie consume RAM y no corre
void lcd_i2c_write_string_P(PGM_P p){
	char c;
	while ((c = pgm_read_byte(p++))) {
		lcd_i2c_write((uint8_t)c, LCD_DATA);
	}
}

char keypad[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

static void iniciar_keypad(void) {
	DDRD  |= 0x0F;
	PORTD |= 0x0F;

	DDRB  &= ~0x0F;
	PORTB |= 0x0F;
}

char Leer_keypad(void) {
	for (uint8_t row = 0; row < 4; row++) {

		
		uint8_t filas = ((~(1 << row)) & 0x0F);
		PORTD = (PORTD & 0xF0) | filas;

		_delay_us(5);

		
		uint8_t cols = (~PINB) & 0x0F;
		if (cols) {
			for (uint8_t col = 0; col < 4; col++) {
				if (cols & (1 << col)) {
					_delay_ms(25);
					if ( ((~PINB) & (1 << col)) ) {
						while ( ((~PINB) & (1 << col)) ) { _delay_ms(5); }
						PORTD = (PORTD & 0xF0) | 0x0F;
						return keypad[row][col];
					}
				}
			}
		}
	}
	PORTD = (PORTD & 0xF0) | 0x0F;
	return 0;
}


static void clave_4_digitos(void){
	uint8_t c = 0;
	while (c < 4) {
		char x = 0;
		do { 
			x = Leer_keypad(); 
			} while (x == 0);
		lcd_i2c_write((uint8_t)x, LCD_DATA);       
		c++;                      
	}
}


static void clave_6_digitos(void){
	uint8_t c = 0;
	while (c < 6) {
		char x = 0;
		do {
			x = Leer_keypad();
		} while (x == 0);
		lcd_i2c_write((uint8_t)x, LCD_DATA);
		c++;
	}
}

static inline void lcd_clear(void){
	lcd_i2c_write(0x01, LCD_COMMAND);
	_delay_ms(2);
}
static inline void lcd_linea2(void){
	lcd_i2c_write(0xC0, LCD_COMMAND);
}

static char Enter(void){
	char x = 0;
	do {
		x = Leer_keypad();
	} while (x == 0);
	return x;
}


static void cambiar_clave(void){
	lcd_clear();
	lcd_i2c_write_string_P(PSTR("Clave Vieja?"));
	lcd_linea2();
	clave_6_digitos(); // Se lee la EEPROM y si en la EEPROM hay una constrase?a de 4 digitos
	                   // se llama a clave_4, si hay una de 6 se llama a clave_6
					   // Ahora se llama a clave_6_digitos para probarla
	
}

static void poner_clave(void){
	lcd_clear();
	lcd_i2c_write_string_P(PSTR("Ingrese Clave"));
	lcd_linea2();
	clave_4_digitos(); // Lo mismo que en cambiar clave
}

// Menu donde se ejecuta la pantalla LCD 
// Se usa do while para que si se toca otra tecla que no sea '*' no se rompa el codigo
static void Menu(void){
	char x;

	
	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Bienvenido a"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("la Cerradura"));
		x = Enter();
	} while (x != '*');

	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Que desea hacer"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("con la clave?"));
		x = Enter();
	} while (x != '*');

	// 3) Menu de opciones: aceptar solo '1' o '2'
	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("1-Poner Clave"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("2-Cambiar Clave"));
		x = Enter();
	} while (x != '1' && x != '2');

	// 4) Ejecutar opci?n
	if (x == '1') {
		poner_clave();
		} else { // x == '2'
		cambiar_clave();
	}
}


int main(void)
{
	iniciar_keypad();
	i2c_init();
	lcd_i2c_init();
	Menu();
	while(1) {
	}
}