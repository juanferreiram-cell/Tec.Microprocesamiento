#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define BTN_LENTO   PB0
#define BTN_RAPIDO  PB1

#define TIEMPO_NORMAL_MS   150UL
#define TIEMPO_MINIMO_MS    10UL
#define CAMBIO_MS           50UL
#define RETARDO_TROZO_MS     5U
#define DEBOUNCE_MS        40U

static void delay_ms_var(uint16_t ms) {
	while (ms--) _delay_ms(1);
}

static void esperar_y_leer_clicks(volatile uint32_t *p_delay_ms, uint32_t ms_total) {
	static uint8_t rapido_bloqueado = 0, lento_bloqueado = 0;
	static uint16_t rapido_estable = 0, lento_estable = 0;
	static uint8_t rapido_antes = 0, lento_antes = 0;

	while (ms_total > 0) {
		uint16_t chunk = (ms_total > RETARDO_TROZO_MS) ? RETARDO_TROZO_MS : ms_total;
		delay_ms_var(chunk);
		ms_total -= chunk;

		uint8_t pinb       = PINB;
		uint8_t rapido_ahora = !(pinb & (1 << BTN_RAPIDO));
		uint8_t lento_ahora  = !(pinb & (1 << BTN_LENTO));

		if (rapido_ahora && lento_ahora) {
			} else {
			// Debounce rápido
			if (rapido_ahora == rapido_antes) rapido_estable += chunk;
			else                            rapido_estable  = 0;
			rapido_antes = rapido_ahora;

			if (rapido_ahora && rapido_estable >= DEBOUNCE_MS && !rapido_bloqueado && !lento_ahora) {
				if (*p_delay_ms > TIEMPO_MINIMO_MS + CAMBIO_MS)
				*p_delay_ms -= CAMBIO_MS;
				else
				*p_delay_ms = TIEMPO_MINIMO_MS;
				rapido_bloqueado = 1;
			}
			if (!rapido_ahora && rapido_bloqueado && rapido_estable >= DEBOUNCE_MS)
			rapido_bloqueado = 0;

			// Debounce lento
			if (lento_ahora == lento_antes) lento_estable += chunk;
			else                         lento_estable  = 0;
			lento_antes = lento_ahora;

			if (lento_ahora && lento_estable >= DEBOUNCE_MS && !lento_bloqueado && !rapido_ahora) {
				*p_delay_ms += CAMBIO_MS;
				lento_bloqueado = 1;
			}
			if (!lento_ahora && lento_bloqueado && lento_estable >= DEBOUNCE_MS)
			lento_bloqueado = 0;
		}
	}
}

int main(void) {
	uint8_t led = 0x01;
	volatile uint32_t delay_ms = TIEMPO_NORMAL_MS;

	DDRD  = 0xFF;
	PORTD = 0x00;

	DDRB  &= ~((1 << BTN_LENTO) | (1 << BTN_RAPIDO));
	PORTB |=  ((1 << BTN_LENTO) | (1 << BTN_RAPIDO));

	while (1) {
		PORTD = led;
		esperar_y_leer_clicks(&delay_ms, delay_ms);
		PORTD = 0;
		led <<= 1;
		if (led == 0) led = 0x01;
	}

	return 0;
}
