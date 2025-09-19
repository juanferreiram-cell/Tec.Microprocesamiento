#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

int main(void) {
	uint8_t led = 0x01;

	// Se configuran los pines D como salida y despues se apagan los LEDS
	
	DDRD  = 0xFF;    / /PD0–PD7 como salidas
	PORTD = 0x00;   // LEDs apagados

	while (1) {
		PORTD = led;          // Enciende el LED actual
		delay_ms(10);

		// Desplazamiento del led
		
		led <<= 1;
		if (led == 0) {
			led = 0x01;
		}
	}

	return 0;
}