#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

// UART
volatile char     buffer_rx[8];
volatile uint8_t  tam_rx        = 0;
volatile bool     comando_listo = false;

// STOP asíncrono desde serie
volatile uint8_t  parar_pedido  = 0;

// prototipo usado en la ISR
static inline void parar_ya(void);

// iniciar UART a 9600 (
static void uart_iniciar(void){
	uint16_t ubrr = 103;                 // 16MHz, 9600 , U2X=0
	UCSR0A &= ~(1<<U2X0);
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
}
static void uart_tx(uint8_t c){ while(!(UCSR0A&(1<<UDRE0))); UDR0=c; }
static void uart_imprimir(const char *s){ while(*s) uart_tx((uint8_t)*s++); }

// Buzzer 1 (Timer1 OC1A en PB1/D9)
#define Z1_DDR   DDRB
#define Z1_PORT  PORTB
#define Z1_PIN   PB1

static inline void z1_encender(uint16_t hz){
	if(!hz){
		TCCR1A=0; TCCR1B=0;
		Z1_PORT &= ~(1<<Z1_PIN);
		return;
	}
	uint32_t ocr = (F_CPU)/(2UL*8UL*hz) - 1UL;   // prescaler=8
	if(ocr>65535UL) ocr=65535UL;
	Z1_DDR  |= (1<<Z1_PIN);
	TCCR1A = (1<<COM1A0);                        // toggle OC1A
	TCCR1B = (1<<WGM12)|(1<<CS11);               // CTC, presc=8
	OCR1A  = (uint16_t)ocr;
}
static inline void z1_apagar(void){
	TCCR1A=0; TCCR1B=0;
	Z1_PORT &= ~(1<<Z1_PIN);
}

// Buzzer 2 (Timer2 OC2A en PB3/D11)
#define Z2_DDR   DDRB
#define Z2_PORT  PORTB
#define Z2_PIN   PB3

static inline void z2_encender(uint16_t hz){
	if(!hz){
		TCCR2A=0; TCCR2B=0;
		Z2_PORT &= ~(1<<Z2_PIN);
		return;
	}
	// Timer2 (de 8 bits) prescalers: 1,8,32,64,128,256,1024
	const uint16_t presc_vals[] = {1,8,32,64,128,256,1024};
	const uint8_t  presc_bits[] = { (1<<CS20),
		(1<<CS21),
		(1<<CS21)|(1<<CS20),
		(1<<CS22),
		(1<<CS22)|(1<<CS20),
		(1<<CS22)|(1<<CS21),
	(1<<CS22)|(1<<CS21)|(1<<CS20) };
	uint8_t chosen = 0xFF;
	uint8_t ocr = 0;

	for(uint8_t i=0;i<7;i++){
		uint32_t tmp = (F_CPU)/(2UL*presc_vals[i]*(uint32_t)hz);
		if(tmp==0) continue;
		if(tmp>0) tmp -= 1U;
		if(tmp<=255U){
			chosen = i;
			ocr = (uint8_t)tmp;
			break;
		}
	}
	if(chosen==0xFF){ chosen = 6; ocr = 255; }   // caso extremo

	Z2_DDR  |= (1<<Z2_PIN);
	TCCR2A = (1<<COM2A0) | (1<<WGM21);          // toggle OC2A, CTC
	TCCR2B = presc_bits[chosen];
	OCR2A  = ocr;
}
static inline void z2_apagar(void){
	TCCR2A=0; TCCR2B=0;
	Z2_PORT &= ~(1<<Z2_PIN);
}

static inline void zumbadores_apagar_todos(void){
	z1_apagar();
	z2_apagar();
}

// STOP inmediato
static inline void parar_ya(void){
	parar_pedido = 1;
	zumbadores_apagar_todos();
}

// UART RX
ISR(USART_RX_vect){
	uint8_t b = UDR0;

	if (b=='S' || b=='s'){
		parar_ya();
		tam_rx = 0;
		buffer_rx[0] = '\0';
		return;
	}
	if (b=='\r' || b=='\n'){
		if(tam_rx>0){
			buffer_rx[tam_rx]='\0';
			comando_listo=true;
		}
		return;
	}
	if (tam_rx < sizeof(buffer_rx)-1){
		buffer_rx[tam_rx++] = (char)b;
		buffer_rx[tam_rx] = '\0';
		}else{
		tam_rx=0; buffer_rx[0]='\0';
	}
	if (tam_rx==1){
		char c = buffer_rx[0];
		if (c=='h'||c=='H') comando_listo=true;
	}
	if (tam_rx==2) comando_listo=true;   // N#, C1, C2 sin Enter
}

// delay 1 ms consultable por STOP
static inline bool tick_1ms_cortable(void){
	if (parar_pedido) return true;
	_delay_ms(1);
	return parar_pedido;
}

// Helpers para botones
static inline void pulso_nota(uint16_t hz, uint16_t on_ms, uint16_t off_ms){
	if (parar_pedido) return;
	if (hz){
		z1_encender(hz);
		for(uint16_t i=0;i<(on_ms?on_ms:1);i++){ if(tick_1ms_cortable()) { z1_apagar(); return; } }
		z1_apagar();
		}else{
		for(uint16_t i=0;i<(on_ms?on_ms:1);i++){ if(tick_1ms_cortable()) return; }
	}
	if (off_ms && !parar_pedido){
		for(uint16_t i=0;i<off_ms;i++){ if(tick_1ms_cortable()) return; }
	}
}
static inline void tocar_nota(uint16_t hz, uint16_t dur_ms){
	if(!parar_pedido) pulso_nota(hz,dur_ms,dur_ms);
}

// Mapa de 8 notas para la botonera
static const uint16_t NOTAS_PIANO[8] = {262,294,330,349,392,440,494,523}; // DO RE MI FA SOL LA SI DO

// Botones
#define BTN0 PC0
#define BTN1 PC1
#define BTN2 PC2
#define BTN3 PC3
#define BTN4 PC4
#define BTN5 PC5
#define BTN6 PD2
#define BTN7 PD3

static inline void botones_iniciar(void){
	DDRC  &= ~((1<<BTN0)|(1<<BTN1)|(1<<BTN2)|(1<<BTN3)|(1<<BTN4)|(1<<BTN5));
	PORTC |=  ((1<<BTN0)|(1<<BTN1)|(1<<BTN2)|(1<<BTN3)|(1<<BTN4)|(1<<BTN5));
	DDRD  &= ~((1<<BTN6)|(1<<BTN7));
	PORTD |=  ((1<<BTN6)|(1<<BTN7));
}
static inline uint8_t leer_boton(void){
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

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))
#endif

// Macros de notas usadas
#ifndef A5
#define A5  880
#endif
#ifndef F5
#define F5  698
#endif
#ifndef D5
#define D5  587
#endif
#ifndef G5
#define G5  784
#endif
#ifndef E5
#define E5  659
#endif
#ifndef C5
#define C5  523
#endif
#ifndef C6
#define C6  1047
#endif
#ifndef Ab5
#define Ab5 932
#endif
#ifndef Ab4
#define Ab4 466
#endif
#ifndef G4
#define G4  392
#endif
#ifndef A4
#define A4  440
#endif
#ifndef Cb5
#define Cb5 554
#endif
#ifndef E4
#define E4  330
#endif
#ifndef B4
#define B4  494
#endif
#ifndef F4
#define F4  349
#endif
#ifndef D4
#define D4  294
#endif
#ifndef C4
#define C4  262
#endif
#ifndef Ab3
#define Ab3 233
#endif
#ifndef G3
#define G3  196
#endif
#ifndef F3
#define F3  175
#endif
#ifndef A3
#define A3  220
#endif
#ifndef E3
#define E3  165
#endif
#ifndef A2
#define A2  110
#endif
#ifndef Cb3
#define Cb3 139
#endif
#ifndef Cb4
#define Cb4 277
#endif
#ifndef Cb6
#define Cb6 1109
#endif
#ifndef Gb4
#define Gb4 415
#endif
#ifndef Gb5
#define Gb5 831
#endif
#ifndef F6
#define F6 1397
#endif
#ifndef E6
#define E6 1319
#endif
#ifndef Db6
#define Db6 1245
#endif
#ifndef Db5
#define Db5 622
#endif
#ifndef B5
#define B5 988
#endif
#ifndef Gb2
#define Gb2 104
#endif
#ifndef Gb3
#define Gb3 208
#endif
#ifndef Cb2
#define Cb2 69
#endif
#ifndef F2
#define F2 87
#endif
#ifndef E2
#define E2 82
#endif
#ifndef Fb2
#define Fb2 92
#endif
#ifndef Fb3
#define Fb3 185
#endif
#ifndef Ab2
#define Ab2 117
#endif
#ifndef C3
#define C3 131
#endif
#ifndef D3
#define D3 147
#endif
#ifndef Db3
#define Db3 156
#endif
#ifndef B3
#define B3 247
#endif
#ifndef Db4
#define Db4 311
#endif

// TABLAS para C1 y C2
// C1 PISTA A Lose Yourself - Eminem
const uint16_t midiC1a[][3] PROGMEM = {
	{A5,474,26},{F5,474,26},{D5,474,26},{A5,474,26},{G5,474,26},{E5,474,26},{C5,474,26},{G5,474,26},
	{A5,474,26},{F5,474,26},{D5,949,1051},{C6,236,14},{Ab5,236,14},{A5,236,14},{G5,236,14},{A5,474,26},
	{F5,474,26},{D5,474,26},{A5,474,26},{G5,474,26},{E5,474,26},{C5,474,26},{G5,474,26},{A5,474,26},
	{F5,474,26},{D5,949,2051},{D5,474,26},{Ab4,474,26},{G4,474,26},{D5,474,26},{C5,949,551},{Ab4,236,14},
	{A4,236,14},{G4,1899,1101},{A4,474,26},{Ab4,474,26},{Cb5,1899,601},{E4,474,26},{A4,474,26},{B4,474,26},
	{Cb5,1899,14851},{F5,355,1145},{F5,355,1145},{F5,355,1145},{F5,355,1145},{F5,355,1145},{F5,355,1145},
	{F5,355,1145},{Ab4,355,395},{D5,712,38},{F5,712,788},{F5,712,38},{F5,712,38},{F5,712,788},{F5,712,38},
	{D5,712,38},{F5,712,788},{F5,712,38},{Ab4,1424,76},{G4,1424,76},{D5,712,38},{F5,712,788},{F5,712,38},
	{F5,712,38},{F5,712,788},{F5,712,38},{D5,712,38},{F5,712,788},{F5,712,38},{Ab4,1424,76},{G4,1424,76},
	{D5,712,38},{F5,712,788},{F5,712,38},{F5,712,38},{F5,712,38},{F5,355,20},{D5,355,20},{A5,177,10},
	{G5,177,10},{F5,177,10},{E5,177,10},{D5,712,38},{F5,712,788},{F5,355,20},{F5,355,20},{Ab4,1424,76},
	{G4,1068,57},{F4,177,10},{A4,177,10},{D5,712,38},{F5,712,788},{F5,712,38},{F5,712,38},{F5,712,38},
	{F5,355,20},{D5,355,20},{A5,177,10},{G5,177,10},{F5,177,10},{E5,177,10},{D5,712,38},{F5,712,788},
	{F5,355,20},{F5,355,20},{Ab4,1424,76},{G4,1068,57},{F4,177,10},{A4,177,10},{D5,712,38},{F5,712,788},
	{D5,712,38},{F5,712,38},{F5,712,38},{F5,355,20},{D5,355,20},{A5,177,10},{G5,177,10},{F5,177,10},
	{E5,177,10},{D5,712,38},{F5,712,38},{D5,712,38},{D5,712,38},{Ab4,1424,76},{G4,712,38},{G4,355,20},
	{F4,177,10},{A4,177,10},{D5,712,38},{F5,712,788},{D5,712,38},{F5,712,38},{F5,712,38},{F5,355,20},
	{D5,355,20},{A5,177,10},{G5,177,10},{F5,177,10},{E5,177,10},{D5,712,38},{F5,712,38},{D5,712,38},
	{D5,712,38},{Ab4,1424,76},{G4,1068,57},{A4,177,10},{F4,177,10},{D4,1424,0}
};

// C1 PISTA B Lose Yourself - Eminem
const uint16_t midiC1b[][3] PROGMEM = {
	{D4,474,26},{F4,474,26},{A4,474,26},{F4,474,26},{C4,474,26},{E4,474,26},{G4,474,26},{E4,474,26},
	{Ab3,474,26},{D4,474,26},{F4,949,51},{Ab3,474,26},{D4,474,26},{F4,949,51},{D4,474,26},{F4,474,26},
	{A4,474,26},{F4,474,26},{C4,474,26},{E4,474,26},{G4,474,26},{E4,474,26},{Ab3,474,26},{D4,474,26},
	{F4,949,51},{Ab3,474,26},{D4,474,26},{F4,949,51},{G3,474,26},{Ab3,474,26},{D4,474,26},{Ab3,474,26},
	{F3,474,26},{A3,474,26},{C4,474,26},{A3,474,26},{E3,3799,201},{A2,474,26},{Cb3,474,26},{E3,474,26},
	{A3,474,26},{Cb4,474,5526},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},{D3,355,20},
	{D3,355,20},{D3,177,10},{D3,177,10},{D3,1424,0}
};

// C2 PISTA A Spyder Dance - Undertale
const uint16_t midiC2a[][3] PROGMEM = {
	{F5,247,14},{C5,247,14},{Gb4,247,14},{F4,247,145},{B4,123,8},{Ab4,247,14},{Ab4,123,8},{Gb4,123,8},{E4,123,8},{F4,123,138},
	{C5,123,8},{Ab4,123,8},{Gb4,123,8},{Ab4,123,8},{C5,123,8},{E4,123,8},{F4,123,8},{Gb4,123,8},{F4,123,8},{E4,123,8},
	{F5,123,138},{Db5,123,8},{C5,61,4},{Ab4,61,4},{Gb4,123,8},{F4,247,14},{Cb4,247,14},{G4,247,14},{Cb4,247,14},{Gb4,247,14},
	{Cb4,247,14},{Ab4,247,14},{Cb4,247,14},{C5,247,14},{Ab4,247,14},{F5,247,14},{C5,247,14},{E5,247,14},{Cb5,247,14},{C5,247,275},
	{F5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{F5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{Cb5,123,8},{B5,123,8},
	{Ab5,123,8},{B5,123,8},{Db5,123,8},{C6,123,8},{G5,123,8},{Ab5,123,8},{F5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},
	{G5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{Gb5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{Ab5,123,8},{Cb6,123,8},
	{C6,123,8},{Cb6,123,8},{Cb5,123,8},{Gb5,123,8},{G5,123,8},{Gb5,123,8},{Db5,123,8},{Ab5,123,8},{Gb5,123,8},{Ab5,123,8},
	{Db5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{Db5,123,8},{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C5,247,14},{Ab4,247,14},
	{F5,247,14},{C5,247,14},{E5,247,14},{Cb5,247,14},{C5,495,27},{F5,247,14},{C5,247,14},{Gb4,247,14},{F4,247,145},{B4,123,8},
	{Ab4,247,14},{Ab4,123,8},{Gb4,123,8},{E4,123,8},{F4,123,138},{C5,123,8},{Ab4,123,8},{Gb4,123,8},{Ab4,123,8},{C5,123,8},
	{E4,123,8},{F4,123,8},{Gb4,123,8},{F4,123,8},{E4,123,8},{F5,123,138},{Db5,123,8},{C5,61,4},{Ab4,61,4},{Gb4,123,8},
	{F4,247,14},{Cb4,247,14},{G4,247,14},{Cb4,247,14},{Gb4,247,14},{Cb4,247,14},{Ab4,247,14},{Cb4,247,14},{C5,247,14},{Ab4,247,14},
	{F5,247,14},{C5,247,14},{E5,247,14},{Cb5,247,14},{C5,247,275},{F5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{F5,123,8},
	{C6,123,8},{Ab5,123,8},{C6,123,8},{Cb5,123,8},{B5,123,8},{Ab5,123,8},{B5,123,8},{Db5,123,8},{C6,123,8},{G5,123,8},{Ab5,123,8},
	{F5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{G5,123,8},{C6,123,8},{Ab5,123,8},{C6,123,8},{Gb5,123,8},{C6,123,8},{Ab5,123,8},
	{C6,123,8},{Ab5,123,8},{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C6,247,14},{Ab5,247,14},{F6,247,14},{C6,247,14},{E6,247,14},
	{Cb6,247,14},{C6,123,8},{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C6,247,14},{Gb5,247,14},{F5,247,14},{C5,247,14},{Gb5,247,14},
	{F5,123,8},{G5,371,282},{Gb5,247,14},{F5,123,8},{G5,371,282},{Gb5,247,14},{F5,123,8},{G5,247,14},{Cb6,123,8},{C6,123,8},
	{Cb6,123,8},{C6,247,14},{Gb5,247,14},{F5,247,14},{C5,247,14},{Gb5,247,14},{Ab5,123,8},{G5,371,282},{Gb5,247,14},{Ab5,123,8},
	{G5,371,282},{Gb5,247,14},{Ab5,123,8},{C6,247,14},{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C6,247,14},{Gb5,247,14},{F5,247,14},
	{C5,247,14},{Gb5,247,14},{F5,123,8},{G5,371,282},{Gb5,247,14},{F5,123,8},{G5,371,282},{Gb5,247,14},{F5,123,8},{G5,247,14},
	{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C6,247,14},{F6,247,14},{Ab5,247,14},{Gb5,247,14},{Ab5,123,8},{Gb5,247,14},{Ab5,371,21},
	{Gb5,123,8},{Ab5,123,8},{B5,123,8},{Ab5,123,8},{Gb5,247,14},{E5,247,14},{C5,247,14},{Cb5,129,132},{E5,129,132},{F5,495,27},
	{C6,247,14},{Gb5,247,14},{F5,247,14},{Ab4,123,8},{C5,123,8},{Gb5,247,14},{F5,247,14},{Cb5,247,14},{Gb4,247,14},{Cb5,123,8},
	{Gb4,123,8},{Gb5,123,8},{Gb4,123,8},{G5,123,8},{Gb5,123,8},{F5,247,14},{E5,123,8},{F5,123,8},{E5,123,8},{F5,123,8},{G5,123,8},
	{E5,123,8},{F5,123,8},{G5,123,8},{F5,123,8},{C5,123,8},{F5,123,8},{G5,123,8},{Gb5,123,8},{G5,123,8},{Gb5,123,8},{Ab5,123,8},
	{C6,123,8},{Ab5,123,8},{Gb5,123,8},{G5,123,8},{Gb5,123,8},{G5,123,8},{F5,123,8},{G5,123,8},{F5,123,8},{Gb5,123,8},{Db6,123,8},
	{F5,123,8},{Cb6,123,8},{Db6,123,8},{C6,123,8},{Cb6,123,8},{Ab5,123,8},{C6,123,8},{Gb5,123,8},{Ab5,123,8},{C6,495,27},{C6,247,14},
	{Gb5,247,14},{F5,247,14},{C5,247,14},{Gb5,247,14},{F5,123,8},{G5,371,282},{Gb5,247,14},{F5,123,8},{G5,371,282},{Gb5,247,14},
	{F5,123,8},{G5,247,14},{Cb6,123,8},{C6,123,8},{Cb6,123,8},{C6,247,14},{F6,247,14},{Ab5,247,14},{Gb5,247,14},{Ab5,123,8},
	{Gb5,247,14},{Ab5,371,21},{Gb5,123,8},{Ab5,123,8},{B5,123,8},{Ab5,123,8},{Gb5,247,14},{E5,247,14},{C5,247,14},{D5,129,132},
	{E5,129,132},{F5,1486,0}
};

// C2 PISTA B Spyder Dance - Undertale
const uint16_t midiC2b[][3] PROGMEM = {
	{F3,123,8},{F3,123,8},{C4,123,8},{F3,123,8},{F3,123,8},{F3,123,8},{Db3,123,8},{Db3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{C3,123,8},{C3,123,8},{C3,123,8},{C3,123,8},
	{F3,123,8},{F3,123,8},{F3,123,8},{F3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{Gb3,123,8},{Gb3,123,8},{Gb3,123,8},{Gb3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},
	{Cb3,123,8},{Cb3,123,8},{Ab3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Ab3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{F3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Gb3,123,8},{Cb3,123,8},
	{C3,247,536},{C3,247,14},{C3,247,145},{C3,247,14},{C3,123,8},{D3,123,8},{E3,123,8},{F4,247,14},{C4,247,14},{Gb3,247,14},{F3,247,145},{B3,123,8},{Ab3,247,14},{Ab3,123,8},{Gb3,123,8},{E3,123,8},
	{F3,123,138},{C4,123,8},{Ab3,123,8},{Gb3,123,8},{Ab3,123,8},{C4,123,8},{E3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{E3,123,8},{F4,123,138},{Db4,123,8},{C4,61,4},{Ab3,61,4},{Gb3,123,8},
	{F3,247,275},{G3,247,275},{Gb3,247,275},{Ab3,247,275},{C3,247,14},{Ab2,247,14},{F3,247,14},{C3,247,14},{E3,247,14},{Cb3,247,14},{C3,123,8},{C3,123,8},{D3,123,8},{E3,123,8},{F3,123,8},{F3,123,8},
	{C4,123,8},{F3,123,8},{F3,123,8},{F3,123,8},{Db3,123,8},{Db3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{C3,123,8},{C3,123,8},{C3,123,8},{C3,123,8},{F3,123,8},{F3,123,8},
	{F3,123,8},{F3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{Gb3,123,8},{Gb3,123,8},{Gb3,123,8},{Gb3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{G3,123,8},{Cb3,123,8},{Cb3,123,8},
	{Ab3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Ab3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{F3,123,8},{Cb3,123,8},{Cb3,123,8},{Cb3,123,8},{Gb3,123,8},{Cb3,123,8},{C3,247,536},{C3,247,14},
	{C3,247,145},{C3,247,14},{C3,123,8},{D3,123,8},{E3,123,8},{F4,247,14},{C4,247,14},{Gb3,247,14},{F3,247,145},{B3,123,8},{Ab3,247,14},{Ab3,123,8},{Gb3,123,8},{E3,123,8},{F3,123,138},{C4,123,8},
	{Ab3,123,8},{Gb3,123,8},{Ab3,123,8},{C4,123,8},{E3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{E3,123,8},{F4,123,138},{Db4,123,8},{C4,61,4},{Ab3,61,4},{Gb3,123,8},{F3,247,275},{G3,247,275},
	{Gb3,247,275},{Ab3,247,275},{F2,247,14},{C3,247,14},{F3,247,14},{C3,247,14},{E3,247,14},{Cb3,247,14},{C3,123,8},{Cb3,123,8},{C3,123,8},{Cb3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},
	{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Ab2,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Ab2,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},
	{Cb3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},
	{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Ab2,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Ab2,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},
	{Cb3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},
	{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Fb2,123,8},{Cb3,123,8},{D3,123,8},{Cb3,123,8},{Fb2,123,8},{Cb3,123,8},{D3,123,8},{Cb3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},
	{C3,123,8},{E3,123,8},{Fb3,123,8},{E3,123,8},{Cb3,123,8},{Gb3,123,8},{C3,123,8},{G3,123,8},{F3,495,27},{F2,123,8},{C3,123,8},
	{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Ab2,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Ab2,123,8},
	{F3,123,8},{G3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},
	{C3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},
	{C3,123,8},{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Ab2,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Ab2,123,8},{F3,123,8},
	{G3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{Cb3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{C3,123,8},
	{F3,123,8},{Gb3,123,8},{F3,123,8},{C3,123,8},{F3,123,8},{G3,123,8},{F3,123,8},{F3,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},
	{F2,123,8},{C3,123,8},{F3,123,8},{C3,123,8},{Fb2,123,8},{Cb3,123,8},{D3,123,8},{Cb3,123,8},{Fb2,123,8},{Cb3,123,8},{D3,123,8},
	{Cb3,123,8},{Cb3,123,8},{F3,123,8},{Gb3,123,8},{F3,123,8},{C3,123,8},{E3,123,8},{Fb3,123,8},{E3,123,8},{Cb3,123,8},{Gb3,123,8},
	{C3,123,8},{G3,123,8},{F3,495,27},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},
	{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},
	{C5,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},{C5,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},{C5,123,8},{F4,123,8},{C5,123,8},
	{Cb5,123,8},{C5,123,8},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{Cb4,123,8},
	{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{Cb4,123,8},{Gb4,123,8},{Ab4,123,8},{Gb4,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},{C5,123,8},
	{F4,123,8},{C5,123,8},{Cb5,123,8},{C5,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},{C5,123,8},{F4,123,8},{C5,123,8},{Cb5,123,8},
	{C5,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},
	{Ab2,123,8},{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},{F2,123,8},
	{C3,123,8},{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},
	{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},
	{Gb2,123,8},{Cb2,123,8},{Gb2,123,8},{Ab2,123,8},{Gb2,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},
	{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},{F2,123,8},{C3,123,8},{Cb3,123,8},{C3,123,8},{C3,247,14},
	{C3,247,14},{E3,123,8},{C3,247,14},{Ab2,123,8},{C3,371,21},{E3,61,4},{F3,61,4},{G3,495,27},{F2,247,797},{Ab2,247,797},{Cb3,247,797},
	{C3,247,275},{C3,247,14},{E2,247,14},{F2,247,797},{Ab2,247,797},{Cb3,247,275},{C3,247,275},{C3,247,14},{E2,247,14},{F2,1486,0}
};

// Scheduler de dos pistas en paralelo (Buzzer 1 + Buzzer 2)
typedef struct {
	const uint16_t (*tbl)[3];
	uint16_t len, idx, rem_ms;
	uint8_t  fase;       // 0=fin, 1=ON, 2=OFF
	uint16_t f_actual;
	uint8_t  sonando;
} pista_t;

static inline void pista_iniciar(pista_t *p, const uint16_t (*tabla)[3], uint16_t n){
	p->tbl=tabla; p->len=n; p->idx=0; p->rem_ms=0; p->fase=0; p->f_actual=0; p->sonando=0;
	if(n==0) return;
	uint16_t f  = pgm_read_word(&tabla[0][0]);
	uint16_t on = pgm_read_word(&tabla[0][1]);
	p->f_actual = f;
	p->rem_ms   = (on?on:1);
	p->fase     = (on?1:2);
}

static inline void pista_tick(pista_t *p, uint8_t which_z){
	if(p->fase==0) return;
	if(p->rem_ms>0) p->rem_ms--;

	if(p->fase==1){ // ON
		if(!p->sonando){
			if(which_z==1) z1_encender(p->f_actual); else z2_encender(p->f_actual);
			p->sonando=1;
		}
		if(p->rem_ms==0){
			if(which_z==1) z1_apagar(); else z2_apagar();
			p->sonando=0;
			uint16_t off = pgm_read_word(&p->tbl[p->idx][2]);
			p->rem_ms = off;
			p->fase   = 2;
		}
		}else{          // OFF
		if(p->sonando){ if(which_z==1) z1_apagar(); else z2_apagar(); p->sonando=0; }
		if(p->rem_ms==0){
			p->idx++;
			if(p->idx>=p->len){ p->fase=0; return; }
			uint16_t f  = pgm_read_word(&p->tbl[p->idx][0]);
			uint16_t on = pgm_read_word(&p->tbl[p->idx][1]);
			p->f_actual=f;
			p->rem_ms=(on?on:1);
			p->fase=(on?1:2);
		}
	}
}

// Wrappers de canciones
static void reproducir_C1_dual(void){
	pista_t p1, p2;
	pista_iniciar(&p1, midiC1a, (uint16_t)ARRAY_LEN(midiC1a));
	pista_iniciar(&p2, midiC1b, (uint16_t)ARRAY_LEN(midiC1b));
	while(!parar_pedido && (p1.fase||p2.fase)){
		pista_tick(&p1,1);
		pista_tick(&p2,2);
		if(tick_1ms_cortable()) break;
	}
	zumbadores_apagar_todos();
}

static void reproducir_C2_dual(void){
	pista_t p1, p2;
	pista_iniciar(&p1, midiC2a, (uint16_t)ARRAY_LEN(midiC2a));
	pista_iniciar(&p2, midiC2b, (uint16_t)ARRAY_LEN(midiC2b));
	while(!parar_pedido && (p1.fase||p2.fase)){
		pista_tick(&p1,1);
		pista_tick(&p2,2);
		if(tick_1ms_cortable()) break;
	}
	zumbadores_apagar_todos();
}

// Menú
static void imprimir_banner(void){
	uart_imprimir("\r\n=== Piano UART + Botones ===\r\n");
	uart_imprimir("Comandos: N0..N7 | C1 (Lose Yourself - Eminem ) | C2 (Spyder Dance - Undertale) | S (Stop) | H\r\n> ");
}

// Modo piano
volatile uint8_t modo_piano = 1;

int main(void){
	DDRB |= (1<<PB5);                    // LED estado

	uart_iniciar();
	sei();
	botones_iniciar();
	imprimir_banner();

	uint8_t ultimo_boton = 255;

	for(;;){
		PORTB ^= (1<<PB5);
		_delay_ms(30);                   // latido

		if (comando_listo){
			comando_listo=false;
			char c0=buffer_rx[0], c1=buffer_rx[1];
			if(c0>='a'&&c0<='z') c0-=32;

			if(c0=='N' && c1>='0' && c1<='7'){
				parar_pedido = 0;
				uint8_t idx=(uint8_t)(c1-'0');
				z1_encender(NOTAS_PIANO[idx]);
				for(uint16_t i=0;i<220;i++){ if(tick_1ms_cortable()) break; }
				z1_apagar();
				uart_imprimir("-> Nota N"); uart_tx(c1);
				uart_imprimir(" OK\r\n> ");

				}else if(c0=='C' && c1=='1'){
				modo_piano = 0; parar_pedido = 0;
				zumbadores_apagar_todos();
				uart_imprimir("-> C1 (doble pista)\r\n");
				reproducir_C1_dual();
				zumbadores_apagar_todos();
				uart_imprimir(parar_pedido ? "\r\n-> C1 cancelada\r\n> " : "\r\n-> Fin C1\r\n> ");
				modo_piano = 1; ultimo_boton = 255; parar_pedido = 0;

				}else if(c0=='C' && c1=='2'){
				modo_piano = 0; parar_pedido = 0;
				zumbadores_apagar_todos();
				uart_imprimir("-> C2 (doble pista)\r\n");
				reproducir_C2_dual();
				zumbadores_apagar_todos();
				uart_imprimir(parar_pedido ? "\r\n-> C2 cancelada\r\n> " : "\r\n-> Fin C2\r\n> ");
				modo_piano = 1; ultimo_boton = 255; parar_pedido = 0;

				}else if(c0=='H'){
				imprimir_banner();

				}else{
				uart_imprimir("OK. Usa N0..N7, C1, C2, S o H.\r\n> ");
			}
			tam_rx=0; buffer_rx[0]='\0';
		}

		if(modo_piano==1){
			uint8_t idx=leer_boton();
			if(idx!=ultimo_boton){
				ultimo_boton=idx;
				if(idx!=255){
					z1_encender(NOTAS_PIANO[idx]);
					uart_imprimir("BTN "); uart_tx('0'+idx);
					uart_imprimir(" PRES\r\n> ");
					}else{
					z1_apagar();
					uart_imprimir("BTN none\r\n> ");
				}
			}
		}

		if (parar_pedido){
			zumbadores_apagar_todos();
			uart_imprimir("\r\n-> STOP\r\n");
			imprimir_banner();
		}
	}
}
