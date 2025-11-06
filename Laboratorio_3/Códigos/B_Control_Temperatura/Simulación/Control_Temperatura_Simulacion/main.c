#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

// Pines
#define PIN_CALEFACTOR PD3

// UART
#define BAUD 9600
#define VALOR_UBRR (F_CPU / 16 / BAUD - 1)


uint16_t T_CALOR_MAX = 22;


void uart_inicializar(void);
void uart_enviar_caracter(char caracter);
void uart_enviar_cadena(const char* cadena);

void adc_inicializar(void);
uint16_t adc_leer_canal(uint8_t canal);


int main(void) {
	char búfer_serial[128];
	float temperatura = 0.0;
	float voltaje = 0.0;
	uint16_t valor_adc = 0;
	uint16_t temp_entera_para_control = 0;

	const char* estado_calefactor;


	uart_inicializar();
	adc_inicializar();

	DDRD |= (1 << PIN_CALEFACTOR);
	PORTD &= ~(1 << PIN_CALEFACTOR);

	uart_enviar_cadena("Bienvenido al sistema de control de temperatura\r\n");

	while (1) {
		
		// Lee la temperatura
		valor_adc = adc_leer_canal(0);
		voltaje = (valor_adc / 1023.0) * 1.1;
		temperatura = voltaje * 100.0;
		temp_entera_para_control = (uint16_t)temperatura;

		if (temp_entera_para_control <= T_CALOR_MAX) {
			// Rango del calefactor
			PORTD |= (1 << PIN_CALEFACTOR);
			estado_calefactor = "PRENDIDO";
			} else {
			// Rango Normal
			PORTD &= ~(1 << PIN_CALEFACTOR);
			estado_calefactor = "APAGADO";
		}

		int temp_entera = (int)temperatura;
		int temp_decimal = (int)((temperatura - (float)temp_entera) * 10);
		if (temp_decimal < 0) temp_decimal = 0;
		if (temp_decimal > 9) temp_decimal = 9;

		sprintf(búfer_serial, "Temperatura: %d.%dC | Calefactor: %s\r\n",
		temp_entera,
		temp_decimal,
		estado_calefactor);
		
		uart_enviar_cadena(búfer_serial);

		_delay_ms(1000);
	}
}

void uart_inicializar(void) {
	UBRR0H = (unsigned char)(VALOR_UBRR >> 8);
	UBRR0L = (unsigned char)VALOR_UBRR;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
void uart_enviar_caracter(char caracter) {
	while ( !(UCSR0A & (1 << UDRE0)) );
	UDR0 = caracter;
}
void uart_enviar_cadena(const char* cadena) {
	while (*cadena) {
		uart_enviar_caracter(*cadena++);
	}
}

void adc_inicializar(void) {
	ADMUX = (1 << REFS1) | (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}
uint16_t adc_leer_canal(uint8_t canal) {
	canal &= 0x07;
	ADMUX = (ADMUX & 0xF8) | canal;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	return (ADC);
}