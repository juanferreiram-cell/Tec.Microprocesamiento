#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define IN1 PD2
#define IN2 PD3

static const uint16_t TOLERANCIA = 15;
static const uint8_t PWM_MINIMO = 150;
static const uint8_t PWM_MAXIMO = 170;

void iniciar_pwm(void){
	DDRB |= (1 << PB1);
	TCCR1A = (1 << COM1A1) | (1 << WGM10);
	TCCR1B = (1 << WGM12) | (1 << CS11);
}

void ajustar_pwm(uint8_t valor){
	OCR1A = valor;
}

void iniciar_motor(void){
	DDRD |= (1 << IN1) | (1 << IN2);
	PORTD &= ~((1 << IN1) | (1 << IN2));
}

void motor_derecha(void){
	PORTD |= (1 << IN1);
	PORTD &= ~(1 << IN2);
}

void motor_izquierda(void){
	PORTD &= ~(1 << IN1);
	PORTD |= (1 << IN2);
}

void motor_parar(void){
	PORTD &= ~((1 << IN1) | (1 << IN2));
	ajustar_pwm(0);
}

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
	iniciar_motor();
	iniciar_pwm();
	
	while(1){
		uint16_t referencia = leer_adc(3);
		uint16_t posicion = leer_adc(4);
		int16_t error = (int16_t)referencia - (int16_t)posicion;
		
		uint8_t pwm_salida = 0;
		
		if (error > -((int16_t)TOLERANCIA) && error < (int16_t)TOLERANCIA){
			motor_parar();
			pwm_salida = 0;
			} else {
			if (error > 0){
				motor_izquierda();
				} else {
				motor_derecha();
			}
			
			uint16_t magnitud = (error < 0) ? (uint16_t)(-error) : (uint16_t)error;
			uint16_t duty = magnitud;
			
			if (duty < PWM_MINIMO) duty = PWM_MINIMO;
			if (duty > PWM_MAXIMO) duty = PWM_MAXIMO;
			
			pwm_salida = (uint8_t)duty;
			ajustar_pwm(pwm_salida);
		}
		
		_delay_ms(50);
	}
}