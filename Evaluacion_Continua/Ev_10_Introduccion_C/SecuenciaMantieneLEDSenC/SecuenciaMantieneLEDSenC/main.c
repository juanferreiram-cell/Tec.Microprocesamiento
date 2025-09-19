#define F_CPU 16000000UL           // Frecuencia real del micro, 16 MHz
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRD = 0xFF;                   // PD0 a PD7 como salida
	const uint16_t T = 60;         // Tiempo por paso (en ms). Ajustar el valor 60 para ir más rápido o lento.

	while (1) {
		// Subida: 0 a 8
		for (int k = 0; k <= 8; k++) {
			
			// Usamos 16 bits en el shift para evitar overflow cuando k=8
			uint8_t mask = (k == 0) ? 0 : (uint8_t)(((uint16_t)1 << k) - 1);
			PORTD = mask;
			_delay_ms(T);
		}

		// Bajada: 7 a 0
		for (int k = 7; k >= 0; k--) {
			uint8_t mask = (k == 0) ? 0 : (uint8_t)(((uint16_t)1 << k) - 1);
			PORTD = mask;
			_delay_ms(T);
		}
	}
}
