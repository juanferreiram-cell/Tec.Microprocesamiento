#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// UART
#define VELOCIDAD_BAUDIO 9600
#define VALOR_UBRR ((F_CPU/16/VELOCIDAD_BAUDIO)-1)

static int uart_enviar_caracter(char c, FILE *s){
	if(c=='\n') uart_enviar_caracter('\r', s);
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
	return 0;
}
FILE uart_salida = FDEV_SETUP_STREAM(uart_enviar_caracter, NULL, _FDEV_SETUP_WRITE);

static inline void uart_iniciar(void){
	UBRR0H = (uint8_t)(VALOR_UBRR>>8);
	UBRR0L = (uint8_t)VALOR_UBRR;
	UCSR0B = (1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	stdout  = &uart_salida;
}

#define CANAL_ADC_LDR 0

static void adc_iniciar(void){
	ADMUX  = (1<<REFS0) | (CANAL_ADC_LDR & 0x0F);
	ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	_delay_ms(2);
}
static uint16_t adc_leer(uint8_t canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}
static uint16_t adc_promedio(uint8_t canal, uint8_t n){
	uint32_t suma = 0;
	for(uint8_t i=0;i<n;i++) suma += adc_leer(canal);
	return (uint16_t)(suma/n);
}

typedef enum { NINGUN_COLOR=0, ROSA, ROJO, AMARILLO, VERDE } color_t;

typedef struct {
	const char *nombre;
	uint16_t bajo, alto;   // rango ADC
} rango_t;

// rango de colores
static rango_t R[] = {
	[ROSA]      = {"ROSA",     128, 172},
	[ROJO]      = {"ROJO",     210, 244},
	[AMARILLO]  = {"AMARILLO", 274, 301},
	[VERDE]     = {"VERDE",    326, 349},
};

static inline color_t detectar(uint16_t v){
	for(color_t c=ROSA; c<=VERDE; c++)
	if(v>=R[c].bajo && v<=R[c].alto) return c;
	return NINGUN_COLOR;
}
static inline uint16_t punto_medio(color_t c){
	return (uint16_t)((R[c].bajo + R[c].alto)/2);
}

int main(void){
	cli();
	uart_iniciar();
	adc_iniciar();
	sei();

	color_t actual = NINGUN_COLOR;
	uint8_t estable = 0, LEC_CONSEC = 3;

	while(1){
		uint16_t v = adc_promedio(CANAL_ADC_LDR, 32);
		color_t c = detectar(v);

		if(c == actual){
			estable = 0;
			} else if(c != NINGUN_COLOR){
			if(++estable >= LEC_CONSEC){
				actual = c; estable = 0;
				uint16_t ref = punto_medio(c);
				int16_t  dif = (int16_t)ref - (int16_t)v;
				printf("LDR=%u | Color=%s | REF=%u | Dif=%d\n", v, R[c].nombre, ref, dif);
			}
			} else {
			actual = NINGUN_COLOR;
			estable = 0;
		}

		_delay_ms(10);
	}
}
