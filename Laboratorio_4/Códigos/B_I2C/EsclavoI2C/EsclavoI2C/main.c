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
	// Establece nuestra dirección de 7 bits (el bit 0 se usa para R/W)
	TWAR = (addr7<<1);
	// Habilita TWI, ACK automático, Interrupción (TWIE)
	TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWIE)|(1<<TWINT);
}

// ISR que maneja la comunicación I2C y recolecta los 4 bytes
ISR(TWI_vect){
	uint8_t st = TWSR & 0xF8;
	switch(st){
		case 0x60: // Own SLA+W recibido (Master quiere escribir)
		case 0x68: // Arbitraje perdido, pero nos seleccionaron
		rxIdx = 0; // Reinicio el contador de bytes
		// Prepara el hardware para recibir datos (esperamos un byte)
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		PIND = (1<<PIN_DBG); // Toggle del pin de debug
		break;

		case 0x80: // Dato recibido, ACK enviado
		if(rxIdx < 4){
			rxBuf[rxIdx++] = TWDR; // Guardo el byte
			if(rxIdx==4){
				frameReady = 1; // Ya tengo el frame completo
			}
		}
		// ACK next (listo para recibir el siguiente byte)
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0x88: // Dato recibido, NACK enviado (Master decidió parar)
		case 0xA0: // Condición de STOP o START repetido
		// Listo para recibir la próxima dirección
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		default: // Cualquier otro estado, nos recuperamos
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
	}
}

/* MAIN */
int main(void){
	uart_init();
	uart_print("\r\n[SLAVE] Iniciando TWI Slave (0x12)\r\n");

	// Inicialización de debug y pines de buzzer
	DDRD |= (1<<PIN_DBG); PORTD &= ~(1<<PIN_DBG);
	DDRD |= (1<<PIN_BUZZ); PORTD &= ~(1<<PIN_BUZZ);

	// Solo inicializo el I2C/TWI
	i2c_slave_init(I2C_SLAVE_ADDR);
	sei();

	uart_print("[SLAVE] Esperando frame I2C...\r\n");

	for(;;){
		if(frameReady){
			cli();
			uint8_t modo = rxBuf[0];
			frameReady=0;
			sei();

			// Solo confirmo la recepción
			uart_printf("[SLAVE] Frame I2C recibido. Byte 0: %u\r\n", modo);
			}else{
			_delay_ms(100);
		}
	}
}