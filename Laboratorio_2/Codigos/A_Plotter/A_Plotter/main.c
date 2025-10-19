#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

static inline void delay_s(uint8_t s) {
	while (s--) _delay_ms(1000);
}

static inline void delay_ms_u16(uint16_t ms) {
	while (ms--) _delay_ms(1);
}

#define AUTOGEN_SCALE_MS_PER_S 50
#define AUTOGEN_DELAY(s) delay_ms_u16((uint16_t)((s) * AUTOGEN_SCALE_MS_PER_S))

#define BAUD 9600UL
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)

void uart_init(void) {
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)(UBRR_VALUE & 0xFF);
	UCSR0A = 0;
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void uart_putc(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void uart_print(const char *s) {
	while (*s) uart_putc(*s++);
}

char uart_getc(void) {
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void uart_flush(void) {
	// Leer y descartar todos los caracteres pendientes
	while (UCSR0A & (1<<RXC0)) {
		(void)UDR0;  // Leer y descartar
	}
}
typedef struct {
	uint8_t portd_value;
	uint8_t delay_units;  // Almacena el valor directo (1, 2, 3, etc.)
} Patron;

// Para secuencias con AUTOGEN_DELAY() (Circulo, Manzana)
static void ejecutar_patron_autogen(const Patron *seq, uint16_t len) {
	for (uint16_t i = 0; i < len; i++) {
		uint8_t portd_val = pgm_read_byte(&seq[i].portd_value);
		uint8_t delay_val = pgm_read_byte(&seq[i].delay_units);
		
		PORTD = portd_val;
		AUTOGEN_DELAY(delay_val);  // Delay con escala
	}
}


// Secuencia Cruz en FLASH (sin cambios)
static const Patron cruz_seq[] PROGMEM = {
	{(1 << PD7), 5},
	{(1 << PD4), 5},
	{(1 << PD2), 1},
	{(1 << PD4) | (1 << PD7), 5},
	{(1 << PD3), 1},
	{(1 << PD5), 5},
	{(1 << PD2), 1},
	{(1 << PD4) | (1 << PD6), 5},
	{(1 << PD3), 1}
};


static const Patron triangulo_seq[] PROGMEM = {
	{(1 << PD7), 5},
	{(1 << PD7), 5},
	{(1 << PD4), 2},
	{(1 << PD2), 2},
	{(1 << PD4), 5},
	{(1 << PD7), 5},
	{(1 << PD6) | (1 << PD5), 5},
	{(1 << PD3), 1},
	{(1 << PD6), 20},
	{(1 << PD6), 10},
	{(1 << PD5), 2}
};


static const Patron circulo_seq[] PROGMEM = {
	/* BAJA SELENOIDE */
	{(1 << PD2), 5},
	/* C?RCULO */
	{ (1 << PD7),  1 },
	{ (1 << PD4),  4 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  4 },
	{ (1 << PD4),  1 },
	{ (1 << PD7), 12 },


	{ (1 << PD5),  1 },
	{ (1 << PD7),  4 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  4 },
	{ (1 << PD7),  1 },
	{ (1 << PD5), 12 },



	{ (1 << PD6),  1 },
	{ (1 << PD5),  4 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  4 },
	{ (1 << PD5),  1 },
	{ (1 << PD6), 12 },



	{ (1 << PD4),  1 },
	{ (1 << PD6),  4 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  4 },
	{ (1 << PD6),  1 },
	{ (1 << PD4), 15 },
	
	/* SUBE SELENOIDE */
	{(1 << PD3), 1},
};
static const Patron Rana_seq[] PROGMEM = {
	{(1 << PD2), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD3), 1},
	
	{(1 << PD7), 39},
	{(1 << PD5), 20},
	{(1 << PD2), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 13},
	{(1 << PD4), 1},
	{(1 << PD6), 11},
	{(1 << PD3), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD2), 1},
	{(1 << PD7), 9},
	{(1 << PD5), 1},
	{(1 << PD7), 10},
	{(1 << PD5), 1},
	{(1 << PD7), 7},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 2},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 2},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 4},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 7},
	{(1 << PD7), 1},
	{(1 << PD4), 8},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 8},
	{(1 << PD4), 5},
	{(1 << PD6), 1},
	{(1 << PD4), 6},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD5), 2},
	{(1 << PD6), 1},
	{(1 << PD5), 3},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 1},
	{(1 << PD5), 7},
	{(1 << PD7), 1},
	{(1 << PD5), 5},
	{(1 << PD7), 1},
	{(1 << PD5), 4},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD3), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD3), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD3), 1},
	{(1 << PD6), 5},
	{(1 << PD4), 18},
	{(1 << PD2), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 4},
	{(1 << PD4), 1},
	{(1 << PD7), 10},
	{(1 << PD5), 1},
	{(1 << PD7), 5},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 5},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 2},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 3},
	{(1 << PD3), 2},
	{(1 << PD5), 10},
	{(1 << PD7), 5},
	{(1 << PD2), 1},




	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},

	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 1},

	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},

	{(1 << PD5), 3},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD3), 1},
	{(1 << PD3), 1},
};

static void Cruz(void) {
	PORTD = (1 << PD7); delay_s(8);
	PORTD = (1 << PD4); delay_s(3);
	PORTD = (1 << PD2); delay_s(2);
	PORTD = (1 << PD4) | (1<<PD7); delay_s(5);
	PORTD = (1<< PD3); delay_s(2);
	PORTD = (1<< PD5); delay_s(5);
	PORTD = (1<< PD2); delay_s(2);
	PORTD = (1<< PD4) | (1<<PD6); delay_s(5);
	PORTD = (1<< PD3); delay_s(1);
}

static void Triangulo(void){
	PORTD = (1 << PD7);
	delay_s(16);
	PORTD = (1 << PD4);
	delay_s(2);;

	PORTD = (1 << PD2); delay_s(2);
	PORTD = (1 << PD4); delay_s(5);
	PORTD = (1 << PD7); delay_s(5);
	PORTD = (1 << PD6) | (1 << PD5); delay_s(5);

	PORTD = (1 << PD3); delay_s(1);
}


static void Circulo(void) {
	PORTD = (1 << PD6);
	delay_s(7);
	PORTD = (1 << PD4);
	delay_s(3);
	ejecutar_patron_autogen(circulo_seq, sizeof(circulo_seq) / sizeof(Patron));
	
}

static void Rana(void) {
	PORTD = (1 << PD6);
	delay_s(8);
	ejecutar_patron_autogen(Rana_seq, sizeof(Rana_seq) / sizeof(Patron));
}
//BIEN
static void PosicionamientoManzana(void){
	PORTD = (1 << PD7);
	delay_s(16);
	PORTD = (1 << PD4);
	delay_s(2);
}
static void Manzana(void) {
	PosicionamientoManzana();
	PORTD = (1 << PD2); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(4);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(6);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(4);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(12);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(6);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(7);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(6);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(16);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(17);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(5);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(7);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(9);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(7);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(6);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(25);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(12);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(10);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD3); AUTOGEN_DELAY(1);
}


typedef void (*FuncionDibujo)(void);

void ejecutar_con_uart_desactivado(FuncionDibujo funcion) {
	_delay_ms(200);     // Esperar a que se envíen los mensajes pendientes
	UCSR0B = 0;         // Desactivar UART completamente
	UCSR0A = 0;         // Limpiar flags del UART
	funcion();          // Ejecutar la función de dibujo
	uart_init();        // Reactivar UART
}

int main(void) {
	DDRD |= (1<<PD7) | (1<<PD6) | (1<<PD5) | (1<<PD4) | (1<<PD3) | (1<<PD2);
	uart_init();
	uart_print("Bienvenido al Menu para Plotter\r\n");
	uart_print("Que quiere elegir?\r\n");
	uart_print("1 - Triangulo\r\n");
	uart_print("2 - Circulo\r\n");
	uart_print("3 - Cruz\r\n");
	uart_print("4 - Manzana\r\n");
	uart_print("5 - Rana\r\n");
	uart_print(">>");
	
	while(1) {
		char c = uart_getc();
		if (c == '\r' || c == '\n') {
			continue;
		}
		
		switch (c) {
			case '1':
			uart_print("Se ha elegido Triangulo\r\n");
			ejecutar_con_uart_desactivado(Triangulo);
			uart_print("Se ha Terminado Triangulo\r\n");
			uart_print("> ");
			break;
			case '2':
			uart_print("Se ha elegido Circulo\r\n");
			ejecutar_con_uart_desactivado(Circulo);
			uart_print("Se ha Terminado Circulo\r\n");
			uart_print("> ");
			break;
			case '3':
			uart_print("Se ha elegido Cruz\r\n");
			ejecutar_con_uart_desactivado(Cruz);
			uart_print("Se ha Terminado Cruz\r\n");
			uart_print("> ");
			break;
			case '4':
			uart_print("Se ha elegido Manzana\r\n");
			ejecutar_con_uart_desactivado(Manzana);
			uart_print("Se ha Terminado Manzana\r\n");
			uart_print("> ");
			break;
			case '5':
			uart_print("Se ha elegido Rana\r\n");
			ejecutar_con_uart_desactivado(Rana);
			uart_print("Se ha Terminado Rana\r\n");
			uart_print("> ");
			break;
			default:
			uart_print("Comando incorrecto\r\n");
			uart_print("> ");
			break;
		}
	}
}