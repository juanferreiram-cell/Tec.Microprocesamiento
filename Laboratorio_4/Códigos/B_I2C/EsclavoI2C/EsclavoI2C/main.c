#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <avr/interrupt.h.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/* ===== UART 9600 8N1 (debug opcional) ===== */
static void uart_init(void){
	UBRR0H=0; UBRR0L=103; UCSR0A=0;
	UCSR0B=(1<<TXEN0);
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);
}
static void uart_putc(char c){ while(!(UCSR0A&(1<<UDRE0))); UDR0=c; }
static void uart_print(const char* s){ while(*s) uart_putc(*s++); }
static void uart_printf(const char* fmt, ...){
	char buf[96]; va_list ap; va_start(ap,fmt);
	vsnprintf(buf,sizeof(buf),fmt,ap); va_end(ap); uart_print(buf);
}

/* ===== Pines ===== */
#define PIN_LED     PD6   // OC0A -> LED
#define PIN_SERVO   PB1   // OC1A -> Servo
#define PIN_BUZZ    PD3   // D3 -> Buzzer pasivo
#define PIN_DBG     PD7   // debug actividad I2C

/* ===== I2C / TWI SLAVE (addr 0x12) ===== */
#define I2C_SLAVE_ADDR  0x12

volatile uint8_t rxBuf[4];
volatile uint8_t rxIdx=0;
volatile uint8_t frameReady=0;

static void i2c_slave_init(uint8_t addr7){
	TWAR = (addr7<<1);
	TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWIE)|(1<<TWINT);
}

// ISR que maneja la comunicación I2C y recolecta los 4 bytes
ISR(TWI_vect){
	uint8_t st = TWSR & 0xF8;
	switch(st){
		case 0x60: // Own SLA+W received
		case 0x68:
		rxIdx = 0;
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		PIND = (1<<PIN_DBG);
		break;

		case 0x80: // data received
		case 0x90:
		if(rxIdx < 4){
			rxBuf[rxIdx++] = TWDR;
			if(rxIdx==4){
				frameReady = 1;
			}
		}
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0x88: // data received, NACK
		case 0xA0: // STOP
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		default:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
	}
}

/* ===== PWM LED (Timer0 OC0A) ===== */
static void pwm_led_init(void){
	DDRD |= (1<<PIN_LED);
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS01)|(1<<CS00); // ~976 Hz
	OCR0A = 0;
}
static inline void led_pwm(uint8_t v){ OCR0A = v; }

/* ===== Servo (Timer1 OC1A) ===== */
#define SERVO_MIN_US  600
#define SERVO_MAX_US  2400
static void servo_init(void){
	DDRB |= (1<<PIN_SERVO);
	TCCR1A = (1<<COM1A1)|(1<<WGM11);
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11); // /8
	ICR1 = 40000-1;              // 20 ms
	OCR1A = 1500*2;              // centro
}
static void servo_write_angle(uint8_t ang){
	if(ang>180) ang=180;
	uint16_t us = SERVO_MIN_US + (uint32_t)(SERVO_MAX_US - SERVO_MIN_US)*ang/180UL;
	if(us<400) us=400; if(us>2600) us=2600;
	OCR1A = us*2;
}

/* ===== Buzzer pasivo con Timer2 CTC + ISR ===== */
static volatile uint8_t buzzer_on = 0;

ISR(TIMER2_COMPA_vect){
	// Toggle del pin para generar la onda
	if(buzzer_on) PIND = (1<<PIN_BUZZ);
	else          PORTD &= ~(1<<PIN_BUZZ);
}
static void buzzer_off(void){
	buzzer_on = 0;
	TIMSK2 &= ~(1<<OCIE2A);
	TCCR2A = 0; TCCR2B = 0;
	PORTD &= ~(1<<PIN_BUZZ);
}
static void buzzer_set_freq(uint16_t hz){
	if(hz < 20){ buzzer_off(); return; }
	const uint16_t presc_list[] = {1,8,32,64,128,256,1024};
	uint8_t ps=0; uint16_t ocr=255;
	for(uint8_t i=0;i<7;i++){
		// Buscamos el mejor Prescaler y OCR para la frecuencia
		uint32_t v=(F_CPU/(2UL*presc_list[i]*hz))-1UL;
		if(v<=255){ ps=i; ocr=(uint16_t)v; break; }
	}
	TCCR2A = (1<<WGM21);  TCCR2B = 0;
	OCR2A  = (uint8_t)ocr;
	// Seteo del Prescaler
	switch(ps){
		case 0: TCCR2B |= (1<<CS20); break;
		case 1: TCCR2B |= (1<<CS21); break;
		case 2: TCCR2B |= (1<<CS21)|(1<<CS20); break;
		case 3: TCCR2B |= (1<<CS22); break;
		case 4: TCCR2B |= (1<<CS22)|(1<<CS20); break;
		case 5: TCCR2B |= (1<<CS22)|(1<<CS21); break;
		default:TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20); break;
	}
	TIMSK2 |= (1<<OCIE2A);
	buzzer_on = 1;
}

/* ===== Autotest ===== */
static void self_test(void){
	// LED
	led_pwm(255); _delay_ms(200); led_pwm(40);
	// Servo
	servo_write_angle(10); _delay_ms(300);
	servo_write_angle(170); _delay_ms(300);
	servo_write_angle(90);
	// Beep
	buzzer_set_freq(1000); _delay_ms(200);
	buzzer_off();
}

/* ===== Main ===== */
int main(void){
	uart_init();
	uart_print("\r\n[SLAVE] Boot @9600 8N1 (I2C slave 0x12)\r\n");

	DDRD |= (1<<PIN_DBG); PORTD &= ~(1<<PIN_DBG);
	DDRD |= (1<<PIN_BUZZ); PORTD &= ~(1<<PIN_BUZZ);

	pwm_led_init();
	servo_init();
	// Se inicializa el buzzer con la función de apagado
	buzzer_off();
	i2c_slave_init(I2C_SLAVE_ADDR);
	sei();

	self_test();
	uart_print("[SLAVE] Self-test done. Waiting I2C frames...\r\n");

	uint32_t idle=0;

	for(;;){
		if(frameReady){
			cli();
			uint8_t modo = rxBuf[0];
			uint8_t buzz = rxBuf[1]; // Byte 1: Frecuencia
			uint8_t lPWM = rxBuf[2]; // Byte 2: LED PWM
			uint8_t ang  = rxBuf[3]; // Byte 3: Ángulo Servo
			frameReady=0;
			sei();

			// Lógica del Buzzer
			if(buzz==0) buzzer_off();
			else{
				// Mapeo 0..255 a 200..3000 Hz
				uint16_t hz = 200 + ((uint32_t)buzz*2800UL)/255UL;
				buzzer_set_freq(hz);
			}
			// Control de LED y Servo
			led_pwm(lPWM);
			servo_write_angle(ang);

			idle=0;
			uart_printf("[SLAVE] I2C rx: b=%u l=%u a=%u\r\n", buzz,lPWM,ang);
			}else{
			_delay_ms(10);
			if(++idle>=200){ // 2 segundos sin data
				// Modo inactivo/ahorro
				buzzer_off();
				led_pwm(15);
				idle=200;
			}
		}
	}
}