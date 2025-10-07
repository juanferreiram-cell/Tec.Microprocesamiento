// Configuracion de CPU y librerias basicas (delay, EEPROM, PROGMEM, booleanos)
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/pgmspace.h>   // para PSTR() y pgm_read_byte (strings en flash)
#include <avr/eeprom.h>     // para eeprom_read/update_*
#include <stdbool.h>

// Definicion de pines de salida para feedback (LEDs y buzzer en PORTC)
#define LED_VERDE PC0
#define LED_ROJO  PC1
#define BUZZER    PC2

// Direcciones y bits del modulo LCD con backpack I2C (PCF8574)
#define LCD_DIRECCION   0x27  // I2C del backpack (PCF8574)
#define LCD_BACKLIGHT   0x08  // bit del backlight en el PCF8574
#define EN              0x04  // Enable de la LCD
#define LCD_RS          0x01  // Register Select (0=cmd, 1=data)
#define LCD_COMMAND     0x00
#define LCD_DATA        LCD_RS

// Ubicacion y layout de la clave en EEPROM (LEN + digitos ASCII)
#define EE_BASE                 ((uint8_t*)0x000)
#define EE_DIRECCION_LARGO      (EE_BASE + 0)   // len: 4 o 6
#define EE_DIRECCION_D1         (EE_BASE + 1)   // digitos ASCII
#define EE_DIRECCION_D2         (EE_BASE + 2)
#define EE_DIRECCION_D3         (EE_BASE + 3)
#define EE_DIRECCION_D4         (EE_BASE + 4)
#define EE_DIRECCION_D5         (EE_BASE + 5)
#define EE_DIRECCION_D6         (EE_BASE + 6)

// Inicializa buzzer y LEDs como salidas y los apaga
static void init_buzzer_led(void){
	DDRC  |= (1<<LED_VERDE) | (1<<LED_ROJO) | (1<<BUZZER);
	PORTC &= ~((1<<LED_VERDE) | (1<<LED_ROJO) | (1<<BUZZER));
}

// Inicializa el bus I2C
void i2c_init() {
	TWSR = 0;
	TWBR = (uint8_t)(((F_CPU / 100000UL) - 16) / 2);
	TWCR = (1 << TWEN);  
}

// Genera condicion START en I2C y espera a que se complete
void i2c_start() {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// Escribe un byte en I2C
void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// Genera condicion STOP en I2C
void i2c_stop() {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Envia un byte al LCD via PCF8574 en modo 4 bits
void lcd_i2c_write(uint8_t data, uint8_t mode) {
	uint8_t high = data & 0xF0;
	uint8_t low  = (data << 4) & 0xF0;
	uint8_t data_arr[4];

	data_arr[0] = high | mode | LCD_BACKLIGHT | EN;
	data_arr[1] = high | mode | LCD_BACKLIGHT;
	data_arr[2] = low  | mode | LCD_BACKLIGHT | EN;
	data_arr[3] = low  | mode | LCD_BACKLIGHT;

	i2c_start();
	i2c_write(LCD_DIRECCION << 1); 
	for (uint8_t i = 0; i < 4; i++) i2c_write(data_arr[i]);
	i2c_stop();
}

// Inicializa la LCD en modo 4 bits siguiendo la secuencia oficial
void lcd_i2c_init() {
	_delay_ms(50);

	i2c_start();
	i2c_write(LCD_DIRECCION << 1);


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

// Escribe un string almacenado en FLASH (PROGMEM) caracter por caracter
void lcd_i2c_write_string_P(PGM_P p){
	char c;
	while ((c = pgm_read_byte(p++))) {
		lcd_i2c_write((uint8_t)c, LCD_DATA);
	}
}

// Limpia la pantalla de la LCD
static inline void lcd_clear(void){
	lcd_i2c_write(0x01, LCD_COMMAND);
	_delay_ms(2);
}

// Mueve el cursor al inicio de la segunda linea
static inline void lcd_linea2(void){
	lcd_i2c_write(0xC0, LCD_COMMAND);
}

// Mapa logico de teclas del keypad 4x4
char keypad[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

// Configura lineas del keypad: filas (PD0 a  PD3) como salidas, columnas (PB0 a B3) como entradas con pull-up
void iniciar_keypad(void) {
	DDRD  |= 0x0F;   // filas como salida
	PORTD |= 0x0F;   // filas inicialmente en alto

	DDRB  &= ~0x0F;  // columnas como entrada
	PORTB |= 0x0F;   // pull-ups en columnas
}

// Escanea el keypad por filas; devuelve el caracter de la tecla presionada (bloquea hasta soltar)
char Leer_keypad(void) {
	for (uint8_t row = 0; row < 4; row++) {
		// Baja solo la fila 'row' y deja el resto en alto sin tocar PD4..PD7
		uint8_t filas = ((~(1 << row)) & 0x0F);
		PORTD = (PORTD & 0xF0) | filas;

		_delay_us(5); // tiempo de establecimiento

		// Columna activa es bajo (por pull-up)
		uint8_t cols = (~PINB) & 0x0F;
		if (cols) {
			for (uint8_t col = 0; col < 4; col++) {
				if (cols & (1 << col)) {
					_delay_ms(25); // debounce
					if ((~PINB) & (1 << col)) {
						while ((~PINB) & (1 << col)) {
							 _delay_ms(5); 
							 } // espera soltar
						PORTD = (PORTD & 0xF0) | 0x0F; // restaura filas
						return keypad[row][col];
					}
				}
			}
		}
	}
	PORTD = (PORTD & 0xF0) | 0x0F; // restaura filas al terminar
	return 0;
}

// Lee el largo de la clave desde EEPROM (4 o 6)
static inline uint8_t ee_get_largo(void) {
	return eeprom_read_byte(EE_DIRECCION_LARGO);
}

// Escribe/actualiza el largo de la clave en EEPROM
static inline void ee_set_largo(uint8_t largo) {
	eeprom_update_byte(EE_DIRECCION_LARGO, largo);
}

// Lee un digito i de la clave desde EEPROM
static inline uint8_t ee_get_digit(uint8_t i) {
	return eeprom_read_byte(EE_BASE + 1 + i);    
}

// Escribe/actualiza un digito i en EEPROM
static inline void ee_set_digit(uint8_t i, uint8_t v) {
	eeprom_update_byte(EE_BASE + 1 + i, v);
}

// Lee 'largo' teclas del keypad y las muestra en LCD
static void leer_clave(uint8_t largo, char *buf) {
	for (uint8_t i = 0; i < largo; i++) {
		char x;
		do { x = Leer_keypad(); } while (x == 0);
		buf[i] = x;
		lcd_i2c_write((uint8_t)x, LCD_DATA);
	}
}

// Pide nueva clave por keypad y la guarda en EEPROM 
// Actualiza LEN y limpia sobrantes si len=4
static void escribir_clave(uint8_t largo) {
	ee_set_largo(largo);
	for (uint8_t i = 0; i < largo; i++) {
		char x;
		do { x = Leer_keypad(); } while (x == 0);
		lcd_i2c_write((uint8_t)x, LCD_DATA);
		ee_set_digit(i, (uint8_t)x);
	}
	if (largo == 4) { ee_set_digit(4, 0xFF); ee_set_digit(5, 0xFF); }
}

// Compara un buffer en RAM contra la clave guardada en EEPROM
static bool clave_coincide(uint8_t len, const char *buf) {
	for (uint8_t i = 0; i < len; i++) {
		if ((uint8_t)buf[i] != ee_get_digit(i)) return false;
	}
	return true;
}

// Lee una tecla de forma bloqueante
static char Enter(void){
	char x = 0;
	do { 
		x = Leer_keypad(); 
		} while (x == 0);
	return x;
}

// Flujo para cambiar la clave: valida la vieja (3 intentos), elige largo (4/6), escribe y da feedback
static void cambiar_clave(void){
	uint8_t l = ee_get_largo();  // 4 o 6
	uint8_t c = 0;               // intentos
	uint8_t x = 0;
	char ingresada[6];

	do{
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Clave Vieja?"));
		lcd_linea2();
		leer_clave(l, ingresada);

		if (clave_coincide(l, ingresada)) {
			lcd_clear();
			lcd_i2c_write_string_P(PSTR("Clave Correcta"));
			PORTC &= ~(1<<LED_ROJO);
			PORTC |=  (1<<LED_VERDE);
			_delay_ms(1000);
			PORTC &= ~(1<<LED_VERDE);

			// Elegir largo de la nueva clave
			do {
				lcd_clear();
				lcd_i2c_write_string_P(PSTR("Clave nueva"));
				lcd_linea2();
				lcd_i2c_write_string_P(PSTR("4 o 6 digitos"));
				x = Enter();
			} while (x != '4' && x != '6');

			lcd_clear();
			lcd_i2c_write_string_P(PSTR("Clave nueva:"));
			lcd_linea2();
			if (x == '4') { escribir_clave(4); }
			else          { escribir_clave(6); }

			// Feedback final
			lcd_clear();
			lcd_i2c_write_string_P(PSTR("Clave"));
			lcd_linea2();
			lcd_i2c_write_string_P(PSTR("Actualizada"));
			_delay_ms(1000);
			return;
			 // volver al menu
			} 
			else { // Error de clave vieja
				lcd_clear();
				lcd_i2c_write_string_P(PSTR("Clave Vieja"));
				lcd_linea2();
				lcd_i2c_write_string_P(PSTR("Incorrecta"));
				PORTC &= ~(1<<LED_VERDE);
				PORTC |=  (1<<LED_ROJO);
				_delay_ms(1000);
				c++;
		}
	} while (c < 3);

	// 3 fallos activa alarma/bloqueo con buzzer
	if (c >= 3) {
		PORTC |= (1<<BUZZER);
		_delay_ms(2500);
		PORTC &= ~(1<<LED_VERDE);
		PORTC &= ~(1<<BUZZER);
		PORTC &= ~(1<<LED_ROJO);
	}
}

// Flujo para ingresar la clave actual y validarla (hasta 3 intentos)
static void poner_clave(void) {
	uint8_t l = ee_get_largo(); // 4 o 6
	uint8_t c = 0;
	char ingresada[6];

	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Ingrese Clave"));
		lcd_linea2();

		leer_clave(l, ingresada);

		if (clave_coincide(l, ingresada)) {
			lcd_clear();
			lcd_i2c_write_string_P(PSTR("Clave Correcta"));
			PORTC &= ~(1<<LED_ROJO);
			PORTC |=  (1<<LED_VERDE);
			_delay_ms(2500);
			PORTC &= ~(1<<LED_VERDE);
			PORTC &= ~(1<<BUZZER);
			PORTC &= ~(1<<LED_ROJO);
			return;   // fin del flujo
			} else {
			lcd_clear();
			lcd_i2c_write_string_P(PSTR("Clave Incorrecta"));
			PORTC &= ~(1<<LED_VERDE);
			PORTC |=  (1<<LED_ROJO);
			_delay_ms(1000);
			c++;
		}
	} while (c < 3);

	// 3 intentos fallidos suena buzzer
	if (c >= 3) {
		 PORTC |= (1<<BUZZER);
		 _delay_ms(2500);
		 PORTC &= ~(1<<LED_VERDE);
		 PORTC &= ~(1<<BUZZER);
		 PORTC &= ~(1<<LED_ROJO);
		 }
}

// Interfaz de usuario por LCD/Keypad:
uint8_t Menu(void){
	char x;

	// Bienvenida; continuar cuando se presione '*', cumple la funcion de "Enter"
	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Bienvenido a"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("la Cerradura"));
		x = Enter();
	} while (x != '*');

	// Pregunta general; continuar con '*'
	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("Que desea hacer"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("con la clave?"));
		x = Enter();
	} while (x != '*');

	// menu de opciones; aceptar solo '1' o '2'
	do {
		lcd_clear();
		lcd_i2c_write_string_P(PSTR("1-Poner Clave"));
		lcd_linea2();
		lcd_i2c_write_string_P(PSTR("2-Cambiar Clave"));
		x = Enter();
	} while (x != '1' && x != '2');

	if (x == '1') {
		poner_clave();
		return 1;   // no repetir menu
		} else {
		cambiar_clave();
		return 1;   // volver al menu
	}
}

// Inicializa la EEPROM con una clave por defecto si esta vacia
static void clave_si_eeprom_vacia(void){
	uint8_t len = ee_get_largo();          // lee 0x000
	if (len != 4 && len != 6) {            // EEPROM no inicializada
		// LEN=4, digitos '0''0''0''0', resto 0xFF
		eeprom_update_block(
		(const void*)(uint8_t[]){ 4,'0','0','0','0',0xFF,0xFF },
		EE_BASE, 7
		);
	}
}

// Punto de entrada: inicializa Pantalla, Keypad y I2C, asegura EEPROM y corre el menu principal
int main(void)
{
	iniciar_keypad();
	init_buzzer_led();
	i2c_init();
	lcd_i2c_init();
	clave_si_eeprom_vacia();

	// Repetir el Menu solo cuando cambiar_clave termina con exito (devuelve 1)
	while ( Menu() ) { /* loop de menu */ }

	// Si se eligio "Poner Clave" y termino, quedarse aqui
	while(1) { }
}
