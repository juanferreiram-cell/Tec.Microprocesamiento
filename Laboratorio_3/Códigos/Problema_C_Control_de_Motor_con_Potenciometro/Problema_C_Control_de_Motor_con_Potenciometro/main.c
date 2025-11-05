#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

void iniciar_adc(void){
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);
	DIDR0 = (1 << ADC3D) | (1 << ADC4D);
	
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
}

uint16_t leer_adc(uint8_t canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	_delay_us(5);
	
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	
	return ADC;
}

int main(void){
	iniciar_adc();
	
	while(1){
		uint16_t referencia = leer_adc(3);
		uint16_t posicion = leer_adc(4);
		
		_delay_ms(50);
	}
}