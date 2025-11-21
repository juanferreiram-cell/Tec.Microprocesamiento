#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define PIN_MATRIZ PB0
#define PUERTO_MATRIZ   PORTB
#define DDR_MATRIZ    DDRB

#define ANCHO_MATRIZ  8
#define ALTO_MATRIZ 8
#define NUM_LEDS      (ANCHO_MATRIZ * ALTO_MATRIZ)

typedef struct {
	uint8_t g;
	uint8_t r;
	uint8_t b;
} Color_GRB;

const Color_GRB colores[] = {
	{0, 255, 0},
	{255, 0, 0},
	{0, 0, 255},
	{255, 255, 0},
	{255, 0, 255},
	{128, 255, 0},
};
#define NUM_COLORES 8

volatile int8_t pos_x = 3;
volatile int8_t pos_y = 3;
volatile uint8_t indice_color = 0;
uint8_t buffer_matriz[NUM_LEDS * 3];

void inicializar_sistema(void);
void matriz_enviar_byte(uint8_t byte);
void actualizar_matriz(void);
void limpiar_matriz(void);
void establecer_pixel_matriz(uint8_t x, uint8_t y, Color_GRB color);
uint16_t xy_a_indice(uint8_t x, uint8_t y);

int main(void) {
	inicializar_sistema();
	_delay_ms(100);
	
	limpiar_matriz();
	establecer_pixel_matriz(pos_x, pos_y, colores[indice_color]);
	actualizar_matriz();
	
	while (1) {
		limpiar_matriz();
		establecer_pixel_matriz(pos_x, pos_y, colores[indice_color]);
		actualizar_matriz();
		
		_delay_ms(30);
	}
	
	return 0;
}

void inicializar_sistema(void) {
	DDR_MATRIZ |= (1 << PIN_MATRIZ);
	PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
	
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		buffer_matriz[i] = 0;
	}
}

void matriz_enviar_byte(uint8_t byte) {
	cli();
	
	for (uint8_t bit = 0; bit < 8; bit++) {
		if (byte & 0x80) {
			PUERTO_MATRIZ |= (1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t"
			::
			);
			PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t"
			::
			);
			} else {
			PUERTO_MATRIZ |= (1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t"
			::
			);
			PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t"
			::
			);
		}
		byte <<= 1;
	}
	
	sei();
}

void actualizar_matriz(void) {
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		matriz_enviar_byte(buffer_matriz[i]);
	}
	
	_delay_us(60);
}

void limpiar_matriz(void) {
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		buffer_matriz[i] = 0;
	}
}

void establecer_pixel_matriz(uint8_t x, uint8_t y, Color_GRB color) {
	if (x >= ANCHO_MATRIZ || y >= ALTO_MATRIZ) return;
	
	uint16_t indice = xy_a_indice(x, y);
	uint16_t posicion = indice * 3;
	
	buffer_matriz[posicion]     = color.g;
	buffer_matriz[posicion + 1] = color.r;
	buffer_matriz[posicion + 2] = color.b;
}

uint16_t xy_a_indice(uint8_t x, uint8_t y) {

	uint16_t indice = y * ANCHO_MATRIZ + x;
	
	return indice;
}