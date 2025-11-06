#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

// Pines
#define PIN_CALEFACTOR PD3
#define PIN_VENTILADOR PD5

// Velocidades del ventilador
#define VELOCIDAD_APAGADO 0
#define VELOCIDAD_BAJA 85
#define VELOCIDAD_MEDIA 170
#define VELOCIDAD_ALTA 255

// Configuración del Puerto Serie (UART)
#define BAUD 9600
#define VALOR_UBRR (F_CPU / 16 / BAUD - 1)


// Rangos de temperatura
uint16_t T_CALOR_MAX = 22;
uint16_t T_MEDIO_MIN = 23;
uint16_t T_MEDIO_MAX = 30;
uint16_t T_BAJA_MAX  = 40;
uint16_t T_MEDIA_MAX = 50;




void uart_inicializar(void);
void uart_enviar_caracter(char caracter);
void uart_enviar_cadena(const char* cadena);

void adc_inicializar(void);
uint16_t adc_leer_canal(uint8_t canal);

void pwm_ventilador_inicializar(void);
void pwm_fijar_velocidad_ventilador(uint8_t velocidad);
void ventilador_apagar_forzado(void);


int main(void) {
	char búfer_serial[128];
	float temperatura = 0.0;
	float voltaje = 0.0;
	uint16_t valor_adc = 0;
	uint16_t temp_entera_para_control = 0;

	const char* estado_calefactor;
	const char* estado_ventilador;

	
	uart_inicializar();
	adc_inicializar();
	pwm_ventilador_inicializar();

	DDRD |= (1 << PIN_CALEFACTOR);
	
	PORTD &= ~(1 << PIN_CALEFACTOR); // Inicia calefactor apagado
	ventilador_apagar_forzado();     // Inicia ventilador apagado

	uart_enviar_cadena("Bienvenido al sistema de control\r\n");

	while (1) {
		
		valor_adc = adc_leer_canal(0);
		voltaje = (valor_adc / 1023.0) * 1.1;
		temperatura = voltaje * 100.0;
		temp_entera_para_control = (uint16_t)temperatura;

		
		if (temp_entera_para_control <= T_CALOR_MAX) {
			// Rango Calefactor
			PORTD |= (1 << PIN_CALEFACTOR);
			ventilador_apagar_forzado();
			
			estado_calefactor = "PRENDIDO";
			estado_ventilador = "APAGADO";

			} else if (temp_entera_para_control >= T_MEDIO_MIN && temp_entera_para_control <= T_MEDIO_MAX) {
			// Rango punto medio
			PORTD &= ~(1 << PIN_CALEFACTOR);
			ventilador_apagar_forzado();
			
			estado_calefactor = "APAGADO";
			estado_ventilador = "APAGADO";

			} else if (temp_entera_para_control > T_MEDIO_MAX && temp_entera_para_control <= T_BAJA_MAX) {
			// Rango Ventilador Bajo
			PORTD &= ~(1 << PIN_CALEFACTOR);
			pwm_fijar_velocidad_ventilador(VELOCIDAD_BAJA);
			
			estado_calefactor = "APAGADO";
			estado_ventilador = "BAJO";

			} else if (temp_entera_para_control > T_BAJA_MAX && temp_entera_para_control <= T_MEDIA_MAX) {
			// Rango Ventilador Medio
			PORTD &= ~(1 << PIN_CALEFACTOR);
			pwm_fijar_velocidad_ventilador(VELOCIDAD_MEDIA);

			estado_calefactor = "APAGADO";
			estado_ventilador = "MEDIO";

			} else { // (temp_entera_para_control > T_MEDIA_MAX)
			// Rango Ventilador Alto
			PORTD &= ~(1 << PIN_CALEFACTOR);
			pwm_fijar_velocidad_ventilador(VELOCIDAD_ALTA);

			estado_calefactor = "APAGADO";
			estado_ventilador = "ALTO";
		}

		int temp_entera = (int)temperatura;
		int temp_decimal = (int)((temperatura - (float)temp_entera) * 10);
		if (temp_decimal < 0) temp_decimal = 0;
		if (temp_decimal > 9) temp_decimal = 9;

		sprintf(búfer_serial, "Temperatura: %d.%dC | Ventilador: %s | Calefactor: %s\r\n",
		temp_entera,
		temp_decimal,
		estado_ventilador,
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

// Funciones ADC
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

// Funciones Ventilador
void pwm_ventilador_inicializar(void) {
	DDRD |= (1 << PIN_VENTILADOR);
	TCCR0A = (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << CS01) | (1 << CS00);
	OCR0B = 0;
	PORTD &= ~(1 << PIN_VENTILADOR);
}
void ventilador_apagar_forzado(void) {
	TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
	PORTD &= ~(1 << PIN_VENTILADOR);
}
void pwm_fijar_velocidad_ventilador(uint8_t velocidad) {
	TCCR0A |= (1 << COM0B1);
	TCCR0A &= ~(1 << COM0B0);
	OCR0B = velocidad;
}