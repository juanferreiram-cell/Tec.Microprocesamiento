// Código de la Secuencia que Mantiene el LED anterior

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// Entradas
#define ENTRADA_DISMINUIR   PB0    // Bajar velocidad
#define ENTRADA_AUMENTAR    PB1    // Aumentar velocidad

// Parametros
#define RETARDO_INICIAL_MS       500UL
#define RETARDO_MIN_MS             1UL
#define RETARDO_PASO_MS           20UL
#define RETARDO_REPETICION_MS    120UL
#define RETARDO_TROZO_MS           5U

static void esperar_ms(uint16_t ms) {
	while (ms--) _delay_ms(1);
}

// Espera a ms_total y mientras tanto lee los botones y ajusta el p_delay_ms
static void esperar_y_ajustar(volatile uint32_t *p_retardo_ms, uint32_t ms_total) {
	static uint16_t acumulado_ms = 0;

	while (ms_total > 0) {
		uint16_t trozo = (ms_total > RETARDO_TROZO_MS) ? RETARDO_TROZO_MS : (uint16_t)ms_total;
		esperar_ms(trozo);
		ms_total -= trozo;

		uint8_t pinb = PINB;
		uint8_t aumentar   = !(pinb & (1 << ENTRADA_AUMENTAR));  // Activo en bajo
		uint8_t disminuir  = !(pinb & (1 << ENTRADA_DISMINUIR));
		uint8_t uno        = (aumentar ^ disminuir);

		if (uno) {
			acumulado_ms += trozo;
			if (acumulado_ms >= RETARDO_REPETICION_MS) {
				acumulado_ms = 0;
				if (aumentar) {
					if (*p_retardo_ms > RETARDO_MIN_MS + RETARDO_PASO_MS)
					*p_retardo_ms -= RETARDO_PASO_MS;
					else
					*p_retardo_ms = RETARDO_MIN_MS;
					} else {
					*p_retardo_ms += RETARDO_PASO_MS;  // Sin límite
				}
			}
			} else {
			acumulado_ms = 0; // Al soltar se mantiene el valor actual
		}
	}
}

int main(void) {
	uint8_t mascara_salida = 0x00;
	uint8_t bajando        = 0;          // 0=Sube, 1=Baja
	int8_t  indice_bit     = 7;          // bit para apagar al bajar
	volatile uint32_t retardo_ms = RETARDO_INICIAL_MS;

	DDRD  = 0xFF;              // PORTD como salida
	PORTD = 0x00;

	DDRB  &= ~((1 << ENTRADA_DISMINUIR) | (1 << ENTRADA_AUMENTAR));  // Entradas
	PORTB |=  ((1 << ENTRADA_DISMINUIR) | (1 << ENTRADA_AUMENTAR));  // Pull ups

	while (1) {
		// Llena de 0 a 7 y luego vacía de 7 a 0 
		if (!bajando) {
			if (mascara_salida == 0x00) mascara_salida = 0x01;
			else if (mascara_salida != 0xFF) mascara_salida = (mascara_salida << 1) | 0x01;
			if (mascara_salida == 0xFF) { bajando = 1; indice_bit = 7; }
			} else {
			mascara_salida &= (uint8_t)~(1 << indice_bit);
			if (indice_bit > 0) indice_bit--;
			else { bajando = 0; mascara_salida = 0x00; }
		}

		PORTD = mascara_salida;
		esperar_y_ajustar(&retardo_ms, retardo_ms);
	}
}
