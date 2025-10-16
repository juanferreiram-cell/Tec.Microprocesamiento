#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

// Definicion de Pines para Leds
#define LED1 PB0
#define LED2 PB1
#define LED3 PB2
#define LED4 PB3
#define LED5 PB4

// Variables
volatile uint8_t sleep_mode_counter = 1;
volatile uint8_t seconds_counter = 0;
volatile uint8_t wdt_counter = 0;

// Configurar Watchdog Timer para interrupci?n cada 1 segundo
void setup_watchdog(void) {
	cli();
	
	// Resetear watchdog
	wdt_reset();
	
	// Habilitar cambio de configuracion
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	
	// Configurar para 1 segundo e interrupcion
	WDTCSR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1);
	
	sei();
}

// ISR del Watchdog (se ejecuta cada 1 segundo)
ISR(WDT_vect) {
	wdt_counter++;
	seconds_counter++;
}

void leds_on(void) {
	PORTB |= 0x1F;
}

void leds_off(void) {
	PORTB &= ~0x1F;
}

void enter_sleep_mode(uint8_t mode) {
	set_sleep_mode(SLEEP_MODE_IDLE); // Por defecto
	
	switch(mode) {
		case 1: // IDLE MODE
		set_sleep_mode(SLEEP_MODE_IDLE);
		break;
		
		case 2: // POWER-DOWN MODE
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		break;
		
		case 3: // POWER-SAVE MODE
		set_sleep_mode(SLEEP_MODE_PWR_SAVE);
		break;
	}
	
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}

int main(void) {
	// Configurar LEDs
	DDRB = 0xFF;
	PORTB = 0x00;
	
	// Indicador de inicio
	for(uint8_t i = 0; i < 5; i++) {
		PORTB = 0x1F;
		_delay_ms(100);
		PORTB = 0x00;
		_delay_ms(100);
	}
	
	setup_watchdog();
	
	_delay_ms(1000);
	
	while(1) {
		uint8_t current_mode = sleep_mode_counter;
		
		// Modo de parpadeos para indiciar cambios de leds
		for(uint8_t i = 0; i < current_mode; i++) {
			PORTB = 0x1F;
			_delay_ms(150);
			PORTB = 0x00;
			_delay_ms(150);
		}
		_delay_ms(1000);
		
		// Prende los LEDs 10 segundos
		leds_on();
		seconds_counter = 0;
		
		while(seconds_counter < 10) {
			
		}
		
		// Apaga los LEDs 30 segundos y entra en uno de los modos sleep
		leds_off();
		seconds_counter = 0;
		
		while(seconds_counter < 30) {
			wdt_reset();
			enter_sleep_mode(current_mode);
		}
		
		// Avanza entre los modos sleep
		sleep_mode_counter++;
		if(sleep_mode_counter > 3) {
			sleep_mode_counter = 1;
		}
		
		PORTB = 0x1F;
		_delay_ms(500);
		PORTB = 0x00;
		_delay_ms(500);
	}
	
	return 0;
}