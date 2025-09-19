#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DELAY_MS 100

int main(void)
{
	// Configuraci?n de pines: D0?D7 como salida
	DDRD = (1 << DDD0) | (1 << DDD1) | (1 << DDD2) | (1 << DDD3) |
	(1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);

	// Asegurar todos apagados al inicio
	PORTD = 0x00;

	while (1)
	{
		// Paso 1: solo extremos (PD0 y PD1)
		PORTD = (1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 2: agregar D6, D7
		PORTD = (1 << PD6) | (1 << PD7) | (1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 3: agregar D2, D5
		PORTD = (1 << PD2) | (1 << PD5) | (1 << PD6) | (1 << PD7) |
		(1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 4: agregar D3, D4 (todos encendidos)
		PORTD = (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) |
		(1 << PD6) | (1 << PD7) | (1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 5: volver a quitar D3, D4
		PORTD = (1 << PD2) | (1 << PD5) | (1 << PD6) | (1 << PD7) |
		(1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 6: solo D6, D7 + extremos
		PORTD = (1 << PD6) | (1 << PD7) | (1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Paso 7: solo extremos (PD0, PD1)
		PORTD = (1 << PD0) | (1 << PD1);
		_delay_ms(DELAY_MS);

		// Apagar todos
		PORTD = 0x00;
		_delay_ms(DELAY_MS);
	}
}