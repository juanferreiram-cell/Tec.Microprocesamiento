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

// ADC
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

// servo
#define SERVO_OC1A_PIN PB1
#define SERVO_LIMITE_TIMER     40000
#define SERVO_MIN_TICKS      1000
#define SERVO_MAX_TICKS      5000

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

// matriz LED 8x8
#define PIN_LED 3
#define ANCHO_MATRIZ 8
#define ALTO_MATRIZ 8
#define NUM_LEDS_MATRIZ (ANCHO_MATRIZ * ALTO_MATRIZ)

extern uint8_t leds_matriz[NUM_LEDS_MATRIZ][3];

static void matriz_enviar_bit(uint8_t valor_bit);
static void matriz_enviar_byte(uint8_t byte);
static void matriz_mostrar(void);
static void matriz_poner_led(uint8_t indice_led, uint8_t r, uint8_t g, uint8_t b);
static void matriz_llenar_todo(uint8_t r, uint8_t g, uint8_t b);
static void matriz_iniciar(void);
static void matriz_aplicar_color(color_t color);

typedef struct {
	const char *nombre;
	uint16_t bajo, alto; // rango ADC
	uint8_t  angulo;
	uint8_t  rgb[3];
} rango_t;

// rango de colores
static rango_t R[] = {
	[ROSA]     = {"ROSA",      580, 620,   0,  {255,  0,  80}},
	[ROJO]     = {"ROJO",      635, 650,  60,  {255,  0,   0}},
	[AMARILLO] = {"AMARILLO",  715, 745, 120,  {230,190,   0}},
	[VERDE]    = {"VERDE",     680, 700, 180,  {  0,255,   0}},
};

static inline color_t detectar(uint16_t v){
	for(color_t c=ROSA; c<=VERDE; c++)
	if(v>=R[c].bajo && v<=R[c].alto) return c;
	return NINGUN_COLOR;
}
static inline uint16_t punto_medio(color_t c){
	return (uint16_t)((R[c].bajo + R[c].alto)/2);
}

// funciones para la matriz de LEDs
uint8_t leds_matriz[NUM_LEDS_MATRIZ][3];

static void matriz_enviar_bit(uint8_t valor_bit){
	PORTD |= (1 << PIN_LED);
	
	if(valor_bit) {
		_delay_us(0.8);
		} else {
		_delay_us(0.4);
	}
	
	PORTD &= ~(1 << PIN_LED);
	_delay_us(0.45);
	}

static void matriz_enviar_byte(uint8_t byte){
	for(uint8_t i = 0; i < 8; i++){
		matriz_enviar_bit(byte & (1 << (7-i)));
	}
}

static void matriz_mostrar(void){
	cli();
	for (int i = 0; i < NUM_LEDS_MATRIZ; i++){
		matriz_enviar_byte(leds_matriz[i][1]); // G
		matriz_enviar_byte(leds_matriz[i][0]); // R
		matriz_enviar_byte(leds_matriz[i][2]); // B
	}
	sei();
	_delay_us(60);
}

static void matriz_poner_led(uint8_t indice_led, uint8_t r, uint8_t g, uint8_t b){
	if (indice_led >= NUM_LEDS_MATRIZ) return;
	leds_matriz[indice_led][0] = r;
	leds_matriz[indice_led][1] = g;
	leds_matriz[indice_led][2] = b;
}

static void matriz_llenar_todo(uint8_t r, uint8_t g, uint8_t b){
	for(int i = 0; i < NUM_LEDS_MATRIZ; i++){
		matriz_poner_led(i, r, g, b);
	}
}

static void matriz_iniciar(void){
	DDRD |= (1 << PIN_LED);
	PORTD &= ~(1 << PIN_LED);
	for(int i = 0; i < NUM_LEDS_MATRIZ; i++){
		matriz_poner_led(i, 0, 0, 0);
	}
	matriz_mostrar();
}

static void matriz_aplicar_color(color_t color){
	switch(color){
		case ROSA:
		matriz_llenar_todo(R[ROSA].rgb[0], R[ROSA].rgb[1], R[ROSA].rgb[2]);
		break;
		case ROJO:
		matriz_llenar_todo(R[ROJO].rgb[0], R[ROJO].rgb[1], R[ROJO].rgb[2]);
		break;
		case AMARILLO:
		matriz_llenar_todo(R[AMARILLO].rgb[0], R[AMARILLO].rgb[1], R[AMARILLO].rgb[2]);
		break;
		case VERDE:
		matriz_llenar_todo(R[VERDE].rgb[0], R[VERDE].rgb[1], R[VERDE].rgb[2]);
		break;
		default:
		break;
	}
	matriz_mostrar();
}

int main(void){
	cli();
	uart_iniciar();
	adc_iniciar();
	servo_iniciar();
	matriz_iniciar();
	sei();

	color_t actual = NINGUN_COLOR;
	uint8_t estable = 0, LEC_CONSEC = 3;

	printf("Esperando a detectar un color\n");

	while(1){
		uint16_t v = adc_promedio(CANAL_ADC_LDR, 32);
		color_t c = detectar(v);

		if(c == actual){
			estable = 0;
			} else if(c != NINGUN_COLOR){
			if(++estable >= LEC_CONSEC){
				actual = c; estable = 0;
				servo_angulo(R[c].angulo);
				matriz_aplicar_color(actual);
				uint16_t sp  = punto_medio(c);
				int16_t  dif = (int16_t)sp - (int16_t)v;
				printf("LDR=%u | Color=%s | PM=%u | Dif=%d\n", v, R[c].nombre, sp, dif);
			}
			} else {
			actual = NINGUN_COLOR;
			estable = 0;
		}

		_delay_ms(10);
	}
}