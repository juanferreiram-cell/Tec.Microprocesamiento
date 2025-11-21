#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/* ===== UART 9600 8N1 ===== */
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
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	TWDR=addr;
	TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	uint8_t st=TWSR&0xF8;
	return (st==0x18 || st==0x40); // SLA+W ack o SLA+R ack
}
static void i2c_write(uint8_t d){
	TWDR=d; TWCR=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}
static void i2c_stop(void){ TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO); }

static void i2c_write_frame(uint8_t addr7, uint8_t b0,uint8_t b1,uint8_t b2,uint8_t b3){
	if(!i2c_start((addr7<<1)|0)) return;
	i2c_write(b0); i2c_write(b1); i2c_write(b2); i2c_write(b3);
	i2c_stop();
}

/* ===== ADC (AVcc) ===== */
static void adc_init(void){
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); // /64
}
static uint16_t adc_read(uint8_t ch){
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

/* ===== DHT11 en PD4 ===== */
#define DHT_PIN PD4
static bool dht11_read(uint8_t *t, uint8_t *h){
	uint8_t d[5]={0};
	DDRD |= (1<<DHT_PIN); PORTD &= ~(1<<DHT_PIN); _delay_ms(20);
	PORTD |= (1<<DHT_PIN); _delay_us(40);
	DDRD &= ~(1<<DHT_PIN); PORTD |= (1<<DHT_PIN);
	uint16_t to=0;
	while(PIND&(1<<DHT_PIN)){ if(++to>200) return false; _delay_us(1); }
	to=0; while(!(PIND&(1<<DHT_PIN))){ if(++to>200) return false; _delay_us(1); }
	to=0; while(PIND&(1<<DHT_PIN)){ if(++to>200) return false; _delay_us(1); }
	for(uint8_t i=0;i<40;i++){
		to=0; while(!(PIND&(1<<DHT_PIN))){ if(++to>200) return false; _delay_us(1); }
		uint16_t w=0; while(PIND&(1<<DHT_PIN)){ if(++w>255) break; _delay_us(1); }
		d[i/8]<<=1; if(w>40) d[i/8]|=1;
	}
	if((uint8_t)(d[0]+d[1]+d[2]+d[3])!=d[4]) return false;
	*h=d[0]; *t=d[2]; return true;
}

/* ===== Helpers ===== */
static inline uint8_t map_u16_to_u8(uint16_t v, uint16_t in_min, uint16_t in_max){
	if(v<=in_min) return 0; if(v>=in_max) return 255;
	uint32_t num=(uint32_t)(v-in_min)*255UL; return (uint8_t)(num/(in_max-in_min));
}

/* ===== LCD I²C — MAP2 fijo ===== */
#define LCD_ADDR            0x27    // si no se ve texto: 0x3F
#define LCD_BL_ACTIVE_LOW   0

static uint8_t lcd_i2c_addrW(void){ return (LCD_ADDR<<1)|0; }

#define P_RS 0
#define P_RW 1
#define P_EN 2
#define P_BL 3
#define P_D4 4
#define P_D5 5
#define P_D6 6
#define P_D7 7

static inline uint8_t lcd_build(uint8_t nib, uint8_t rs, uint8_t rw, uint8_t backlight_on){
	uint8_t b = 0;
	if(nib & 0x1) b |= (1<<P_D4);
	if(nib & 0x2) b |= (1<<P_D5);
	if(nib & 0x4) b |= (1<<P_D6);
	if(nib & 0x8) b |= (1<<P_D7);
	if(rs)        b |= (1<<P_RS);
	if(rw)        b |= (1<<P_RW);
	if(LCD_BL_ACTIVE_LOW){ if(backlight_on) b &= ~(1<<P_BL); else b |= (1<<P_BL); }
	else                   { if(backlight_on) b |=  (1<<P_BL); else b &= ~(1<<P_BL); }
	return b;
}
static void lcd_pulse_en(uint8_t base){
	i2c_write(base | (1<<P_EN));  _delay_us(1);
	i2c_write(base & ~(1<<P_EN)); _delay_us(40);
}
static void lcd_send_nibble(uint8_t nib, uint8_t rs, uint8_t backlight_on){
	if(!i2c_start(lcd_i2c_addrW())) return;
	uint8_t base = lcd_build(nib, rs, 0, backlight_on);
	lcd_pulse_en(base);
	i2c_stop();
}
static void lcd_send(uint8_t v, uint8_t rs){
	lcd_send_nibble((v>>4)&0x0F, rs, 1);
	lcd_send_nibble( v     &0x0F, rs, 1);
}
static void lcd_cmd(uint8_t c){ lcd_send(c,0); }
static void lcd_data(uint8_t d){ lcd_send(d,1); }
static void lcd_init_parallel(void){
	_delay_ms(40);
	lcd_send_nibble(0x03,0,1); _delay_ms(5);
	lcd_send_nibble(0x03,0,1); _delay_ms(5);
	lcd_send_nibble(0x03,0,1); _delay_ms(5);
	lcd_send_nibble(0x02,0,1); _delay_ms(5);
	lcd_cmd(0x28); lcd_cmd(0x0C); lcd_cmd(0x06);
	lcd_cmd(0x01); _delay_ms(3);
}
static void lcd_clear(void){ lcd_cmd(0x01); _delay_ms(3); }
static void lcd_goto_xy(uint8_t c, uint8_t r){ lcd_cmd(0x80 | (r?0x40:0x00) | c); }
static void lcd_print(const char* s){ while(*s) lcd_data(*s++); }
static void lcd_printf(const char* fmt,...){
	char b[32]; va_list ap; va_start(ap,fmt); vsnprintf(b,sizeof b,fmt,ap); va_end(ap); lcd_print(b);
}

/* ===== MAIN ===== */
int main(void){
	uart_init();
	uart_print("\r\n[MASTER] Boot @9600 8N1 (I2C master)\r\n");

	i2c_init();
	adc_init();

	/* DHT idle */
	DDRD &= ~(1<<DHT_PIN); PORTD |= (1<<DHT_PIN);

	/* LCD I²C */
	lcd_init_parallel();
	lcd_goto_xy(0,0); lcd_print("I2C Maestro");
	lcd_goto_xy(0,1); lcd_print("To Slave 0x12");

	uint8_t T=25,H=50;
	uint16_t tick=0, lcdTick=0;

	for(;;){
		uint16_t pot = adc_read(0);     // A0
		uint16_t ldr = adc_read(1);     // A1

		if(++tick>=10){ tick=0; uint8_t t,h; if(dht11_read(&t,&h)){T=t;H=h;} }

		// Mapas
		uint8_t buzzVal  = map_u16_to_u8(pot, 0,1023);   // 0..255
		uint8_t ledPWM   = map_u16_to_u8(ldr, 0,1023);   // 0..255
		uint8_t servoAng = (uint8_t)((uint32_t)pot*180UL/1023UL);

		// Log
		uart_printf("[MASTER] pot=%4u ldr=%4u | buzz=%3u led=%3u servo=%3u | T=%u H=%u\r\n",
		pot, ldr, buzzVal, ledPWM, servoAng, T, H);

		// Enviar frame al esclavo (addr 0x12)
		i2c_write_frame(0x12, 0, buzzVal, ledPWM, servoAng);

		// LCD cada ~600 ms
		if(++lcdTick >= 4){
			lcdTick=0;
			lcd_goto_xy(0,0); lcd_printf("BZ:%3u L:%3u   ", buzzVal, ledPWM);
			lcd_goto_xy(0,1); lcd_printf("T:%2u H:%2u S:%3u", T, H, servoAng);
		}

		_delay_ms(150);
	}
}
