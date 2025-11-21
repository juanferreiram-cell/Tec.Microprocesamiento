#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/* UART 9600 8N1 (debug opcional) */
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

/* Pines */
#define PIN_LED     PD6   // OC0A -> LED
#define PIN_SERVO   PB1   // OC1A -> Servo
#define PIN_BUZZ    PD3   // D3 -> Buzzer pasivo
#define PIN_DBG     PD7   // actividad SPI

/* SPI Slave */
volatile uint8_t rxBuf[4];
volatile uint8_t rxIdx=0;
volatile uint8_t frameReady=0;

// ISR que recolecta el frame de 4 bytes
ISR(SPI_STC_vect){
	uint8_t b = SPDR;
	rxBuf[rxIdx++] = b;
	if(rxIdx>=4){ rxIdx=0; frameReady=1; }
	PIND = (1<<PIN_DBG); // toggle PD7
}

static void spi_slave_init(void){
	DDRB |= (1<<PB4);                      // MISO out
	DDRB &= ~((1<<PB5)|(1<<PB3)|(1<<PB2)); // SCK,MOSI,SS in
	SPCR = (1<<SPE)|(1<<SPIE);             // enable + IRQ
}

/* PWM LED (Timer0 OC0A) */
static void pwm_led_init(void){
	DDRD |= (1<<PIN_LED);
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00); // Fast PWM
	TCCR0B = (1<<CS01)|(1<<CS00); // Prescaler /64
	OCR0A = 0;
}
static inline void led_pwm(uint8_t v){ OCR0A = v; }

/* Servo (Timer1 OC1A) */
// Tick = 0.5 us. Periodo de 20 ms.
#define SERVO_MIN_US  600   // Rango 0° (ajustable)
#define SERVO_MAX_US  2400  // Rango 180° (ajustable)

static void servo_init(void){
	DDRB |= (1<<PIN_SERVO);
	TCCR1A = (1<<COM1A1)|(1<<WGM11);
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11); // /8
	ICR1 = 40000-1;              // 20 ms
	OCR1A = (1500*2);            // 1.5 ms (~centro)
}
static void servo_write_angle(uint8_t ang){
	if(ang>180) ang=180;
	// Mapeamos 0..180° a SERVO_MIN_US..SERVO_MAX_US
	uint16_t us = SERVO_MIN_US + (uint32_t)(SERVO_MAX_US - SERVO_MIN_US)*ang/180UL;
	// Seguridad de rango
	if(us < 400)  us = 400;
	if(us > 2600) us = 2600;
	OCR1A = us * 2; // El tick es 0.5 us
}

/* Buzzer pasivo en D3 (Timer2 CTC + ISR para generar la onda) */
static volatile uint8_t buzzer_on = 0;

// ISR que hace el toggle en el pin para generar la frecuencia
ISR(TIMER2_COMPA_vect){
	if(buzzer_on){
		PIND = (1<<PIN_BUZZ); // toggle PD3
		}else{
		PORTD &= ~(1<<PIN_BUZZ);
	}
}

static void buzzer_off(void){
	buzzer_on = 0;
	TIMSK2 &= ~(1<<OCIE2A);       // sin ISR
	TCCR2A = 0; TCCR2B = 0;
	PORTD &= ~(1<<PIN_BUZZ);
}

static void buzzer_set_freq(uint16_t hz){
	if(hz < 20){ buzzer_off(); return; }      // silencio
	const uint16_t presc_list[] = {1,8,32,64,128,256,1024};
	uint8_t best_ps_idx = 0;
	uint16_t best_ocr = 255;
	
	// Bucle para encontrar el mejor Prescaler y OCR
	for(uint8_t i=0;i<7;i++){
		uint32_t ocr = (F_CPU/(2UL*presc_list[i]*hz)) - 1UL;
		if(ocr <= 255){
			best_ps_idx = i;
			best_ocr = (uint16_t)ocr;
			break;
		}
	}
	TCCR2A = (1<<WGM21);  // CTC
	TCCR2B = 0;
	OCR2A  = (uint8_t)best_ocr;
	
	// Configuración del Prescaler
	switch(best_ps_idx){
		case 0: TCCR2B |= (1<<CS20); break;
		case 1: TCCR2B |= (1<<CS21); break;
		case 2: TCCR2B |= (1<<CS21)|(1<<CS20); break;
		case 3: TCCR2B |= (1<<CS22); break;
		case 4: TCCR2B |= (1<<CS22)|(1<<CS20); break;
		case 5: TCCR2B |= (1<<CS22)|(1<<CS21); break;
		default:TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);
	}
	TIMSK2 |= (1<<OCIE2A);    // habilita ISR
	buzzer_on = 1;
}

static void buzzer_init(void){
	DDRD |= (1<<PIN_BUZZ);
	PORTD &= ~(1<<PIN_BUZZ);
	buzzer_off();
}

/* Autotest */
static void self_test(void){
	// LED a tope 200 ms
	led_pwm(255); _delay_ms(200);
	led_pwm(40);

	// Servo barrido
	servo_write_angle(10);  _delay_ms(300);
	servo_write_angle(170); _delay_ms(300);
	servo_write_angle(90);

	// Beep corto
	buzzer_set_freq(1000); _delay_ms(200);
	buzzer_off();
}

/* Main */
int main(void){
	uart_init();
	uart_print("\r\n[SLAVE] Boot @9600 8N1 (LED/Servo/Buzzer)\r\n");

	DDRD |= (1<<PIN_DBG); PORTD &= ~(1<<PIN_DBG);

	pwm_led_init();
	servo_init();
	buzzer_init();
	spi_slave_init();
	sei();

	self_test();
	uart_print("[SLAVE] Self-test done. Waiting SPI...\r\n");

	uint32_t idle = 0;

	for(;;){
		if(frameReady){
			cli();
			uint8_t modo = rxBuf[0];
			uint8_t buzz = rxBuf[1];    // Frecuencia
			uint8_t lPWM = rxBuf[2];    // LED
			uint8_t ang  = rxBuf[3];    // Servo
			frameReady=0;
			sei();

			// Lógica del Buzzer
			if(buzz==0){
				buzzer_off();
				}else{
				// Mapeamos 0..255 a 200..3000 Hz
				uint16_t hz = 200 + ((uint32_t)buzz*2800UL)/255UL;
				buzzer_set_freq(hz);
			}

			led_pwm(lPWM);
			servo_write_angle(ang);

			idle = 0;
			uart_printf("[SLAVE] buzz=%u led=%u ang=%u\r\n", buzz, lPWM, ang);
			}else{
			_delay_ms(10);
			if(++idle >= 200){ // ~2 s sin frames
				// Modo inactivo
				buzzer_off();
				led_pwm(15);
				idle = 200;
			}
		}
	}
}