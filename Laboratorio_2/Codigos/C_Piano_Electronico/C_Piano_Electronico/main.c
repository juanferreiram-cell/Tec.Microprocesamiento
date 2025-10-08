#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

// UART 9600
static void uart_iniciar(void){
	uint16_t ubrr = 103;                // 16MHz, 9600, U2X=0
	UCSR0A &= ~(1<<U2X0);
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)(ubrr&0xFF);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B = (1<<TXEN0);
}
static void uart_tx(uint8_t c){ while(!(UCSR0A&(1<<UDRE0))); UDR0=c; }
static void uart_imprimir(const char *s){ while(*s) uart_tx((uint8_t)*s++); }

// Buzzer en OC1A (D9) 
#define BUZ_DDR   DDRB
#define BUZ_PORT  PORTB
#define BUZ_PIN   PB1

static inline void buz_on(uint16_t hz){
	if(!hz){ TCCR1A=0; TCCR1B=0; BUZ_PORT&=~(1<<BUZ_PIN); return; }
	uint32_t ocr = (16000000UL)/(2UL*8UL*hz) - 1UL;
	if(ocr>65535) ocr=65535;
	BUZ_DDR |= (1<<BUZ_PIN);
	TCCR1A = (1<<COM1A0);               // toggle
	TCCR1B = (1<<WGM12)|(1<<CS11);      // CTC, /8
	OCR1A  = (uint16_t)ocr;
}
static inline void buz_off(void){ TCCR1A=0; TCCR1B=0; BUZ_PORT&=~(1<<BUZ_PIN); }

// Botones: PC0 a PC5, PD2 a PD3 (a GND)
#define BTN0 PC0
#define BTN1 PC1
#define BTN2 PC2
#define BTN3 PC3
#define BTN4 PC4
#define BTN5 PC5
#define BTN6 PD2
#define BTN7 PD3

static inline void botones_ini(void){
	DDRC  &= ~((1<<BTN0)|(1<<BTN1)|(1<<BTN2)|(1<<BTN3)|(1<<BTN4)|(1<<BTN5));
	PORTC |=  ((1<<BTN0)|(1<<BTN1)|(1<<BTN2)|(1<<BTN3)|(1<<BTN4)|(1<<BTN5));
	DDRD  &= ~((1<<BTN6)|(1<<BTN7));
	PORTD |=  ((1<<BTN6)|(1<<BTN7));
}
static inline uint8_t boton_leer(void){
	if(!(PINC&(1<<BTN0))) return 0;
	if(!(PINC&(1<<BTN1))) return 1;
	if(!(PINC&(1<<BTN2))) return 2;
	if(!(PINC&(1<<BTN3))) return 3;
	if(!(PINC&(1<<BTN4))) return 4;
	if(!(PINC&(1<<BTN5))) return 5;
	if(!(PIND&(1<<BTN6))) return 6;
	if(!(PIND&(1<<BTN7))) return 7;
	return 255;
}

// Mapa de notas 
static const uint16_t NOTAS[8]={262,294,330,349,392,440,494,523};

int main(void){
	DDRB |= (1<<PB5);               // LED
	uart_iniciar();
	botones_ini();

	uart_imprimir("\r\nv0.1 Piano por botones. Presioná algo...\r\n");

	uint8_t prev=255;
	for(;;){
		PORTB ^= (1<<PB5); _delay_ms(40);  // latido
		uint8_t b = boton_leer();
		if(b!=prev){
			prev=b;
			if(b!=255){
				buz_on(NOTAS[b]);
				uart_imprimir("BTN "); uart_tx('0'+b); uart_imprimir("\r\n");
				}else{
				buz_off();
				uart_imprimir("BTN none\r\n");
			}
		}
	}
}
