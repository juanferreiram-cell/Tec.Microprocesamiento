#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define BAUD        9600UL
#define UBRR_VALUE  (F_CPU/16/BAUD - 1)

static void uart_iniciar(void){
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)(UBRR_VALUE & 0xFF);
	UCSR0A = 0;                            
	UCSR0B = (1<<TXEN0);                    
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);    
}

// manda un caracter por UART
static void uart_tx(char c) { while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; }

static void adc_iniciar(void){
	ADMUX  = (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	DIDR0  = (1<<ADC0D);
}
// lectura unica
static uint16_t adc_leer_una(void){ ADCSRA |= (1<<ADSC); while (ADCSRA & (1<<ADSC)); return ADC; }
// promedio de lecturas
static uint16_t adc_leer_prom(uint8_t n){
	uint32_t suma = 0;
	for (uint8_t i=0; i<n; i++) suma += adc_leer_una();
	return (uint16_t)(suma / n);
}

// forma del teclado
static const char mapa_teclas[16] = {
	'1','2','3','A',
	'4','5','6','B',
	'7','8','9','C',
	'*','0','#','D'
};

// valores adc para cada caracter
static uint16_t niveles_adc[16] = {
	780,834,913,970,
	842,874,928,974,
	862,888,934,976,
	876,898,938,977
};

#define MARGEN          30   // margen del nivel adc para una tecla
#define CAMBIO_MINIMO    20   // distancia mínima desde reposo
#define LECTURAS_CONS     4   // lecturas consecutivas estables

// devuelve el indice del boton presionado
static int8_t indice_tecla_desde_adc(uint16_t v){
	int8_t mejor = -1; uint16_t mejor_err = 0xFFFF;
	for (uint8_t i=0; i<16; i++){
		uint16_t c = niveles_adc[i];
		uint16_t e = (v>c)? (v-c) : (c-v);
		if (e < mejor_err) { mejor_err = e; mejor = (int8_t)i; }
	}
	return (mejor_err <= MARGEN) ? mejor : -1;
}

int main(void){
	uart_iniciar();
	adc_iniciar();

	_delay_ms(200); 

	// chequeaa si no hay ningun boton presionado
	uint32_t acum = 0;
	for (uint8_t i=0; i<64; i++){ acum += adc_leer_una(); _delay_ms(2); }
	uint16_t reposo = (uint16_t)(acum / 64);

	int8_t  tecla_anterior = -1;
	uint16_t adc_prev      = reposo;
	uint8_t  estable       = 0;

	for(;;){
		uint16_t adc = adc_leer_prom(8);
		uint16_t dif = (adc>adc_prev)? (adc-adc_prev) : (adc_prev-adc);
		if (dif < 4) { if (estable < 255) estable++; } else { estable = 0; }
		adc_prev = adc;

		int8_t k = indice_tecla_desde_adc(adc);
		// si esta debajo del minimo no cuenta la letra
		uint16_t dreposo = (adc>reposo)? (adc-reposo) : (reposo-adc);
		if (dreposo < CAMBIO_MINIMO) k = -1;

		if (estable > LECTURAS_CONS){
			if (k >= 0 && tecla_anterior == -1)
			uart_tx(mapa_teclas[k]);     // imprime una vez por pulsacion
			tecla_anterior = k;
		}
		if (k < 0 && tecla_anterior != -1)
		tecla_anterior = -1;

		_delay_ms(5);
	}
}
