#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

// Configuraciones
#define LEDS_W   8
#define LEDS_H   8
#define NUM_LEDS (LEDS_W * LEDS_H)

#define WS_PIN_PORT  PORTD
#define WS_PIN_DDR   DDRD
#define WS_PIN_BIT   PD2      // DATA WS2812 en PD2

#define LATCH_US     300      // reset/latch WS2812
#define SERPENTINE   0        // mapeo lineal

// Buffer GRB para WS2812
static uint8_t leds[NUM_LEDS * 3];

// PRNG simple
static uint32_t rng_state = 0xC0FFEEu;
static inline uint32_t xorshift32(void){
	uint32_t x = rng_state;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return rng_state = x;
}

// Salidas deterministas a 2 ciclos (SBI/CBI)
static inline void ws_hi(void){
	__asm__ __volatile__("sbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTD)), "I" (WS_PIN_BIT));
}
static inline void ws_lo(void){
	__asm__ __volatile__("cbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTD)), "I" (WS_PIN_BIT));
}

// Timings WS2812 (800 kHz)
#if (F_CPU == 16000000UL)
#define CYCLES_1H  9
#define CYCLES_1L 11
#define CYCLES_0H  4
#define CYCLES_0L 16
#elif (F_CPU == 8000000UL)
#define CYCLES_1H  6
#define CYCLES_1L  4
#define CYCLES_0H  3
#define CYCLES_0L  7
#else
#error "F_CPU debe ser 16 MHz u 8 MHz para este driver WS2812."
#endif

static inline void delay_cycles(uint8_t n){
	if (n==0) return;
	__builtin_avr_delay_cycles((unsigned long)n);
}

// Envío de un byte a WS2812
static inline void ws_send_byte(uint8_t b){
	for (uint8_t i = 0; i < 8; i++){
		if (b & 0x80){
			ws_hi(); delay_cycles(CYCLES_1H);
			ws_lo(); delay_cycles(CYCLES_1L);
			} else {
			ws_hi(); delay_cycles(CYCLES_0H);
			ws_lo(); delay_cycles(CYCLES_0L);
		}
		b <<= 1;
	}
}

// Envío del frame completo y latch
static void ws_show(const uint8_t *grb, uint16_t nleds){
	uint8_t s = SREG; __asm__ __volatile__("cli" ::: "memory");
	for (uint16_t i = 0; i < nleds * 3; i++){
		ws_send_byte(grb[i]);
	}
	SREG = s;
	_delay_us(LATCH_US);
}

// Mapeo de coordenadas a índice lineal
static inline uint16_t index_xy(uint8_t x, uint8_t y){
	#if SERPENTINE
	return (y & 1) ? (y * LEDS_W + (LEDS_W - 1 - x)) : (y * LEDS_W + x);
	#else
	return y * LEDS_W + x;
	#endif
}

// Seteo de un pixel (GRB)
static inline void set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b){
	if (x >= LEDS_W || y >= LEDS_H) return;
	uint16_t idx = index_xy(x, y) * 3;
	leds[idx + 0] = g;
	leds[idx + 1] = r;
	leds[idx + 2] = b;
}

// Limpieza del buffer
static inline void clear_all(void){
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++){
		leds[i] = 0;
	}
}

// Main 
int main(void){
	// Configuro DATA en PD2 como salida y lo dejo LOW
	WS_PIN_DDR  |=  (1 << WS_PIN_BIT);
	WS_PIN_PORT &= ~(1 << WS_PIN_BIT);

	// Limpio la matriz y envío un par de frames
	clear_all();
	for (uint8_t k = 0; k < 2; k++){
		ws_show(leds, NUM_LEDS);
	}

	// Fijo una posición visible y cambio el color cada 500 ms
	uint8_t x = 3, y = 3;
	while (1){
		uint8_t r = (uint8_t)(xorshift32() & 0xFF);
		uint8_t g = (uint8_t)((xorshift32() >> 8) & 0xFF);
		uint8_t b = (uint8_t)((xorshift32() >> 16) & 0xFF);
		if (r < 8 && g < 8 && b < 8) r = 64;

		clear_all();
		set_pixel(x, y, r, g, b);
		ws_show(leds, NUM_LEDS);

		_delay_ms(500);
	}
}
