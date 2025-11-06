#define F_CPU 16000000UL
#define BAUD 9600
#define MI_UBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include "lcd_i2c.h"

// Pines
#define PUERTO_LED_ROJO   PORTD
#define DDR_LED_ROJO    DDRD
#define PIN_LED_ROJO    PD6

#define PUERTO_LED_VERDE  PORTD
#define DDR_LED_VERDE   DDRD
#define PIN_LED_VERDE   PD7

#define PUERTO_BOTON_BORRAR     PORTD
#define DDR_BOTON_BORRAR      DDRD
#define REG_PIN_BOTON_BORRAR  PIND
#define PIN_BOTON_BORRAR      PD4

#define PUERTO_BOTON_ACTUALIZAR     PORTD
#define DDR_BOTON_ACTUALIZAR      DDRD
#define REG_PIN_BOTON_ACTUALIZAR  PIND
#define PIN_BOTON_ACTUALIZAR      PD5

volatile char memoria_id_recibido[20];
volatile uint8_t indice_id_recibido = 0;
volatile uint8_t bandera_nuevo_id_recibido = 0;

char id_maestra_en_ram[20]; // ID de la tarjeta con acceso

void uart_inicializar(uint16_t ubrr);
void uart_transmitir_caracter(char data);
void uart_transmitir_cadena(const char* str);
void leer_id_desde_eeprom(void);
void guardar_id_en_eeprom(const char* id);
void procesar_id_recibido(void);
void modo_actualizar_id(void);
void modo_borrar_id(void);

// recepcion por UART para simular como si fuese el lector RFID
ISR(USART_RX_vect) {
	char c = UDR0;

	if (c == '\r' || c == '\n' || indice_id_recibido >= 19) {
		if (indice_id_recibido > 0) {
			memoria_id_recibido[indice_id_recibido] = '\0';
			bandera_nuevo_id_recibido = 1;
		}
		indice_id_recibido = 0;
		} else {
		memoria_id_recibido[indice_id_recibido] = c;
		indice_id_recibido++;
	}
}

int main(void) {
	// se configuran los pines
	DDR_LED_ROJO |= (1 << PIN_LED_ROJO);
	DDR_LED_VERDE |= (1 << PIN_LED_VERDE);

	DDR_BOTON_BORRAR &= ~(1 << PIN_BOTON_BORRAR);
	DDR_BOTON_ACTUALIZAR &= ~(1 << PIN_BOTON_ACTUALIZAR);
	PUERTO_BOTON_BORRAR |= (1 << PIN_BOTON_BORRAR);
	PUERTO_BOTON_ACTUALIZAR |= (1 << PIN_BOTON_ACTUALIZAR);

	uart_inicializar(MI_UBRR);
	lcd_init();
	sei();

	// Carga el ID de la tarjeta con acceso y da la bienvenida
	leer_id_desde_eeprom();
	uart_transmitir_cadena("Sistema RFID Iniciado\r\n");
	
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Bienvenido");
	lcd_gotoxy(0, 1);
	lcd_puts("Acerque tarjeta");

	while (1) {
		if (bandera_nuevo_id_recibido) {
			procesar_id_recibido();
			bandera_nuevo_id_recibido = 0;
		}

		if (!(REG_PIN_BOTON_BORRAR & (1 << PIN_BOTON_BORRAR))) {
			_delay_ms(50); // Debounce
			if (!(REG_PIN_BOTON_BORRAR & (1 << PIN_BOTON_BORRAR))) {
				modo_borrar_id();
				while (!(REG_PIN_BOTON_BORRAR & (1 << PIN_BOTON_BORRAR)));
			}
		}

		if (!(REG_PIN_BOTON_ACTUALIZAR & (1 << PIN_BOTON_ACTUALIZAR))) {
			_delay_ms(50); // Debounce
			if (!(REG_PIN_BOTON_ACTUALIZAR & (1 << PIN_BOTON_ACTUALIZAR))) {
				modo_actualizar_id();
				while (!(REG_PIN_BOTON_ACTUALIZAR & (1 << PIN_BOTON_ACTUALIZAR)));
			}
		}
	}
}


void procesar_id_recibido(void) {
	uart_transmitir_cadena("ID recibido: ");
	uart_transmitir_cadena((const char*)memoria_id_recibido);
	uart_transmitir_cadena("\r\n");

	if (strlen(id_maestra_en_ram) > 0 && strcmp((const char*)memoria_id_recibido, id_maestra_en_ram) == 0) {
		// ACCESO PERMITIDO
		uart_transmitir_cadena("Acceso PERMITIDO\r\n");
		lcd_clear();
		lcd_gotoxy(0, 0);
		lcd_puts("Acceso Permitido");
		PUERTO_LED_VERDE |= (1 << PIN_LED_VERDE);
		_delay_ms(2000);
		PUERTO_LED_VERDE &= ~(1 << PIN_LED_VERDE);
		} else {
		// ACCESO DENEGADO
		uart_transmitir_cadena("Acceso DENEGADO\r\n");
		lcd_clear();
		lcd_gotoxy(0, 0);
		lcd_puts("Acceso Denegado");
		PUERTO_LED_ROJO |= (1 << PIN_LED_ROJO);
		_delay_ms(2000);
		PUERTO_LED_ROJO &= ~(1 << PIN_LED_ROJO);
	}
	
	_delay_ms(500);
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Acerque tarjeta");
}

void modo_borrar_id(void) {
	uart_transmitir_cadena("MODO BORRADO: ID Eliminado.\r\n");
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Tarjeta Borrada");
	guardar_id_en_eeprom("");
	strcpy(id_maestra_en_ram, "");
	_delay_ms(1500);
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Acerque tarjeta");
}

void modo_actualizar_id(void) {
	uart_transmitir_cadena("MODO ACTUALIZAR: Acerque nueva tarjeta\r\n");
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Acerque nueva");
	lcd_gotoxy(0, 1);
	lcd_puts("tarjeta");

	bandera_nuevo_id_recibido = 0;
	while (bandera_nuevo_id_recibido == 0) {
		
	}

	guardar_id_en_eeprom((const char*)memoria_id_recibido);
	strcpy(id_maestra_en_ram, (const char*)memoria_id_recibido);

	uart_transmitir_cadena("Nuevo ID registrado: ");
	uart_transmitir_cadena(id_maestra_en_ram);
	uart_transmitir_cadena("\r\n");
	
	_delay_ms(2000);
	
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts("Acerque tarjeta");
}

// Funciones UART
void uart_inicializar(uint16_t ubrr) {
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmitir_caracter(char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void uart_transmitir_cadena(const char* str) {
	while (*str) {
		uart_transmitir_caracter(*str++);
	}
}

// Funciones de la EEPROM
void leer_id_desde_eeprom(void) {
	eeprom_read_block((void*)id_maestra_en_ram, (const void*)0, 20);
	id_maestra_en_ram[19] = '\0';
}

void guardar_id_en_eeprom(const char* id) {
	char memoria_temporal[20];
	strncpy(memoria_temporal, id, 19);
	memoria_temporal[19] = '\0';
	eeprom_write_block((const void*)memoria_temporal, (void*)0, 20);
}