#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/* ===== UART 9600 8N1 (Para debugging) ===== */
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

/* ===== I2C / TWI MASTER 100 kHz ===== */
static void i2c_init(void){ TWSR=0x00; TWBR=72; } // 100 kHz @ 16 MHz
static uint8_t i2c_start(uint8_t addr){
	// Enviar START
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	// Enviar SLA+W o SLA+R
	TWDR=addr;
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	uint8_t st=TWSR&0xF8;
	return (st==0x18 || st==0x40); // SLA+W ACK o SLA+R ACK
}
static void i2c_write(uint8_t d){
	// Enviar dato
	TWDR=d; TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}
static void i2c_stop(void){ TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO); }

// Función principal para enviar el frame de 4 bytes al esclavo
static void i2c_write_frame(uint8_t addr7, uint8_t b0,uint8_t b1,uint8_t b2,uint8_t b3){
	if(!i2c_start((addr7<<1)|0)) return; // Inicio + dirección W
	i2c_write(b0); i2c_write(b1); i2c_write(b2); i2c_write(b3); // Escribo los 4 bytes
	i2c_stop(); // Stop
}

/* ===== ADC (Lectura de sensores analógicos) ===== */
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

/* ===== Helpers (Mapeo de 10 bits a 8 bits) ===== */
static inline uint8_t map_u16_to_u8(uint16_t v, uint16_t in_min, uint16_t in_max){
	if(v<=in_min) return 0; if(v>=in_max) return 255;
	uint32_t num=(uint32_t)(v-in_min)*255UL; return (uint8_t)(num/(in_max-in_min));
}

/* ===== MAIN ===== */
int main(void){
	uart_init();
	uart_print("\r\n[MASTER] Iniciando I2C Master y ADC\r\n");

	i2c_init(); // Inicializo la comunicación I2C
	adc_init();

	uart_print("[MASTER] Listo. Leyendo sensores y enviando I2C.\r\n");

	for(;;){
		uint16_t pot = adc_read(0);      // Potenciómetro (A0)
		uint16_t ldr = adc_read(1);      // LDR (A1)

		// Mapeamos a los rangos de control (0-255 o 0-180)
		uint8_t buzzVal  = map_u16_to_u8(pot, 0,1023);   // Control de Buzzer
		uint8_t ledPWM   = map_u16_to_u8(ldr, 0,1023);   // Control de LED
		uint8_t servoAng = (uint8_t)((uint32_t)pot*180UL/1023UL); // Control de Servo

		uart_printf("[MASTER] pot=%4u ldr=%4u | buzz=%3u led=%3u servo=%3u\r\n",
		pot, ldr, buzzVal, ledPWM, servoAng);

		// Enviamos el frame al esclavo I2C (dirección 0x12)
		// Frame: [modo=0, buzzVal, ledPWM, servoAng]
		i2c_write_frame(0x12, 0, buzzVal, ledPWM, servoAng);

		_delay_ms(150);
	}
}