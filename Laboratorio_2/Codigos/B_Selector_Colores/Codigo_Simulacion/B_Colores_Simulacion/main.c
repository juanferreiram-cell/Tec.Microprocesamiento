#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// UART
#define VELOCIDAD_BAUDIO   9600
#define VALOR_UBRR         ((F_CPU/16/VELOCIDAD_BAUDIO)-1)

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

#define RGB_ANODO_COMUN 0
#define RGB_R_OC0A_PIN PD6
#define RGB_G_OC0B_PIN PD5
#define RGB_B_OC2A_PIN PB3

static void rgb_iniciar(void){
	DDRD |= (1<<RGB_R_OC0A_PIN)|(1<<RGB_G_OC0B_PIN);
	TCCR0A = (1<<COM0A1)|(1<<COM0B1)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS01)|(1<<CS00);
	DDRB |= (1<<RGB_B_OC2A_PIN);
	TCCR2A = (1<<COM2A1)|(1<<WGM21)|(1<<WGM20);
	TCCR2B = (1<<CS22);
}
static inline uint8_t nivel(uint8_t v){ return RGB_ANODO_COMUN ? (255 - v) : v; }
static inline void rgb_poner(uint8_t r, uint8_t g, uint8_t b){
	OCR0A = nivel(r);
	OCR0B = nivel(g);
	OCR2A = nivel(b);
}

// servo
#define SERVO_OC1A_PIN PB1
#define SERVO_LIMITE_TIMER     40000
#define SERVO_MIN_TICKS      2000
#define SERVO_MAX_TICKS      4000

static void servo_iniciar(void){
	DDRB  |= (1<<SERVO_OC1A_PIN);
	TCCR1A = (1<<COM1A1)|(1<<WGM11);
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11);
	ICR1   = SERVO_LIMITE_TIMER;
	OCR1A  = SERVO_MIN_TICKS;
}
static void servo_angulo(uint8_t a){
	if(a>180) a=180;
	uint16_t pulso = SERVO_MIN_TICKS +
	(uint32_t)(SERVO_MAX_TICKS - SERVO_MIN_TICKS)*a/180UL;
	OCR1A = pulso;
}

typedef enum { NINGUN_COLOR=0, ROSA, ROJO, AMARILLO, VERDE } color_t;

typedef struct {
	const char *nombre;
	uint16_t bajo, alto; // rango ADC
	uint8_t  angulo;
	uint8_t  rgb[3];
} rango_t;

// rango de colores
static rango_t R[] = {
	[ROSA]     = {"ROSA",      128, 172,   0,  {255,  0,  80}},
	[ROJO]  = {"ROJO",      210, 244,  60,  {255,  0,   0}},
	[AMARILLO] = {"AMARILLO",  274, 301, 120,  {255,255,   0}},
	[VERDE]    = {"VERDE",     326, 349, 180,  {  0,255,   0}},
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
	rgb_iniciar();
	servo_iniciar();
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
				servo_angulo(R[c].angulo);
				rgb_poner(R[c].rgb[0], R[c].rgb[1], R[c].rgb[2]);
				uint16_t sp  = punto_medio(c);
				int16_t  dif = (int16_t)sp - (int16_t)v;
				printf("LDR=%u | Color=%s | REF=%u | Dif=%d\n", v, R[c].nombre, sp, dif);
			}
			} else {
			rgb_poner(0,0,0);
			actual = NINGUN_COLOR;
			estable = 0;
		}

		_delay_ms(10);
	}
}
