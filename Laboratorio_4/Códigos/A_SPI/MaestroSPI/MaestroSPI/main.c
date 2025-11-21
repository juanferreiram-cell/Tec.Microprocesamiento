#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

// Configuración básica para poder ver el estado por el terminal.
/* UART 9600 8N1 */
static void uart_init(void){
	UBRR0H = 0; UBRR0L = 103;
	UCSR0A = 0;
	UCSR0B = (1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}
static void uart_putc(char c){ while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; }
static void uart_print(const char* s){ while(*s) uart_putc(*s++); }
static void uart_printf(const char* fmt, ...){
	char buf[96]; va_list ap; va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
	uart_print(buf);
}

/* SPI MASTER (Configuración de pines) */
static void spi_master_init(void){
	// SCK, MOSI, SS como salidas
	DDRB |= (1<<PB5)|(1<<PB3)|(1<<PB2);
	DDRB &= ~(1<<PB4); // MISO como entrada
	PORTB |= (1<<PB2); // SS en alto (chip deseleccionado)
	// Habilitar SPI, Modo Master, Velocidad fosc/64
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
}
static inline uint8_t spi_txrx(uint8_t b){ SPDR=b; while(!(SPSR&(1<<SPIF))); return SPDR; }

// Envía 4 bytes y maneja la línea SS (Slave Select)
static void spi_send4(uint8_t b0,uint8_t b1,uint8_t b2,uint8_t b3){
	PORTB &= ~(1<<PB2); // SS a bajo
	spi_txrx(b0); spi_txrx(b1); spi_txrx(b2); spi_txrx(b3);
	PORTB |=  (1<<PB2); // SS a alto
}

/* ADC (Lectura de Sensores Analógicos) */
static void adc_init(void){
	ADMUX = (1<<REFS0); // Referencia AVcc
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); // Habilitar y Prescaler /64
}
static uint16_t adc_read(uint8_t ch){
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F); // Seleccionar canal
	ADCSRA |= (1<<ADSC); // Iniciar conversión
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

// Función simple de mapeo (no es crítica ahora, pero es útil)
static inline uint8_t map_u16_to_u8(uint16_t v, uint16_t in_min, uint16_t in_max){
	if(v<=in_min) return 0; if(v>=in_max) return 255;
	uint32_t num=(uint32_t)(v-in_min)*255UL; return (uint8_t)(num/(in_max-in_min));
}

/* MAIN */
int main(void){
	uart_init();
	uart_print("\r\n[MASTER] Iniciando SPI y ADC\r\n");

	spi_master_init();
	adc_init();

	uart_print("[MASTER] Listo. Leyendo potenciometros y enviando.\r\n");

	for(;;){
		uint16_t pot = adc_read(0);      // Leo el Potenciómetro (A0)
		uint16_t ldr = adc_read(1);      // Leo el LDR (A1)

		// Mapeo los valores del ADC (0-1023) a 8 bits (0-255)
		uint8_t buzzVal = map_u16_to_u8(pot, 0,1023);
		uint8_t pwmLED  = map_u16_to_u8(ldr, 0,1023);
		uint8_t servoAng= (uint8_t)((uint32_t)pot*180UL/1023UL);

		uart_printf("[MASTER] pot=%4u ldr=%4u | buzz=%3u led=%3u servo=%3u\r\n",
		pot, ldr, buzzVal, pwmLED, servoAng);

		// Envío el frame: [modo=0, buzzVal, pwmLED, servoAng]
		spi_send4(0, buzzVal, pwmLED, servoAng);

		_delay_ms(150);
	}
}