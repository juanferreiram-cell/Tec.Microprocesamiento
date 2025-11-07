#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

// Pines
#define CANAL_ADC_LM35    0
#define CALEFACTOR_DDR      DDRD
#define CALEFACTOR_PIN_BM (1<<PD5)
#define VENTILADOR_DDR        DDRB
#define VENTILADOR_PIN_BM     (1<<PB2)
#define VENTILADOR_INVERTIDO 0

static void uart_inicializar(uint32_t baudios){
	uint16_t ubrr = (F_CPU / 16 / baudios) - 1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}
static void uart_tx_caracter(char c){ while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; }
static void uart_tx_cadena(const char *cadena){ while(*cadena) uart_tx_caracter(*cadena++); }
static void uart_tx_entero16(uint16_t valor){
	char buffer[6]; uint8_t i=0; if(valor==0){ uart_tx_caracter('0'); return; }
	while(valor){ buffer[i++]='0'+(valor%10); valor/=10; } while(i) uart_tx_caracter(buffer[--i]);
}
static inline bool uart_rx_disponible(void){ return (UCSR0A & (1<<RXC0)); }
static inline uint8_t uart_rx_leer(void){ return UDR0; }
static void uart_rx_limpiar_buffer(void){ while (uart_rx_disponible()) (void)uart_rx_leer(); }


static uint16_t uart_leer_entero_flexible(void){
	uint16_t valor = 0;
	bool recibio_digito = false;
	uint16_t milis_inactividad = 0;

	for(;;){
		if (uart_rx_disponible()){
			uint8_t c = uart_rx_leer();
			if (c >= '0' && c <= '9'){
				recibio_digito = true;
				valor = (uint16_t)(valor*10 + (c-'0'));
				uart_tx_caracter(c);
				milis_inactividad = 0;
				} else if (c=='\r' || c=='\n' || c==' ' || c==',' || c==';' || c=='\t'){
				uart_tx_cadena("\r\n");
				break;
				} else {
				if (recibio_digito){ uart_tx_cadena("\r\n"); break; }
			}
			} else {
			if (recibio_digito){
				_delay_ms(1);
				if (++milis_inactividad >= 2000){ uart_tx_cadena("\r\n"); break; }
				} else {
				_delay_ms(1);
			}
		}
	}
	return valor;
}

// ADC
static void adc_inicializar(void){
	ADMUX  = (1<<REFS0) | (CANAL_ADC_LM35 & 0x0F);
	ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
static uint16_t adc_leer_canal(uint8_t canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}

static void calefactor_pwm_inicializar(void){
	CALEFACTOR_DDR |= CALEFACTOR_PIN_BM;
	TCCR0A = (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS01) | (1<<CS00);
	OCR0B  = 0;
}
static inline void calefactor_fijar_pwm(uint8_t ciclo_trabajo){ OCR0B = ciclo_trabajo; }

static void ventilador_pwm_inicializar(void){
	VENTILADOR_DDR |= VENTILADOR_PIN_BM;
	TCCR1A = (1<<WGM10);
	TCCR1B = (1<<WGM12);
	#if VENTILADOR_INVERTIDO
	TCCR1A |= (1<<COM1B1) | (1<<COM1B0);
	#else
	TCCR1A |= (1<<COM1B1);
	#endif
	TCCR1B |= (1<<CS11) | (1<<CS10);
	OCR1B = 0;
}
static inline void ventilador_fijar_pwm(uint8_t ciclo_trabajo){
	#if VENTILADOR_INVERTIDO
	OCR1B = (uint8_t)(255 - ciclo_trabajo);
	#else
	OCR1B = ciclo_trabajo;
	#endif
}

// PWM
#define VENTILADOR_CICLO_BAJO      85
#define VENTILADOR_CICLO_MEDIO     170
#define VENTILADOR_CICLO_ALTO      255
#define CALEFACTOR_CICLO_APAGADO   0
#define CALEFACTOR_CICLO_ENCENDIDO 250

// LM35
static uint16_t lm35_leer_celsius_x10(void){
	uint16_t crudo = adc_leer_canal(CANAL_ADC_LM35);
	uint32_t temp_decimas = (uint32_t)crudo * 488U;
	return (uint16_t)(temp_decimas / 100U);
}
static uint16_t lm35_leer_celsius_entero(void){
	uint16_t crudo = adc_leer_canal(CANAL_ADC_LM35);
	return (uint16_t)(((uint32_t)crudo * 125U) >> 8);
}

typedef enum { CALEFACTOR_AUTO = 0, CALEFACTOR_MANUAL } modo_calefactor_t;
typedef enum { VENT_APAGADO=0, VENT_BAJO, VENT_MEDIO, VENT_ALTO } accion_ventilador_t;

static volatile modo_calefactor_t g_modo_calefactor = CALEFACTOR_AUTO;
static volatile uint8_t           g_ciclo_manual_calefactor = CALEFACTOR_CICLO_APAGADO;

// rangos
static uint16_t T_CALOR_MAX = 22;
static uint16_t T_MEDIO_MIN = 23;
static uint16_t T_MEDIO_MAX = 30;
static uint16_t T_BAJO_MAX  = 40;
static uint16_t T_MED_MAX   = 50;

// Constantes para recalcular
#define ANCHO_BANDA_BAJA  10
#define ANCHO_BANDA_MEDIA 10

// recalcula los rangos a partir del nuevo punto medio
static void recalcular_desde_medio(uint16_t medio_min, uint16_t medio_max){
	if (medio_min == 0) medio_min = 1;
	if (medio_max < medio_min) medio_max = medio_min;

	T_MEDIO_MIN = medio_min;
	T_MEDIO_MAX = medio_max;
	T_CALOR_MAX = (medio_min > 0) ? (uint16_t)(medio_min - 1) : 0;
	T_BAJO_MAX = (uint16_t)(medio_max + ANCHO_BANDA_BAJA);
	T_MED_MAX = (uint16_t)(T_BAJO_MAX + ANCHO_BANDA_MEDIA);
}

// Control ventilador
static accion_ventilador_t aplicar_control_ventilador(uint16_t temp_celsius){
	if (temp_celsius <= T_CALOR_MAX){ ventilador_fijar_pwm(0); return VENT_APAGADO; }
	else if (temp_celsius <= T_MEDIO_MAX){ ventilador_fijar_pwm(0); return VENT_APAGADO; }
	else if (temp_celsius <= T_BAJO_MAX){ ventilador_fijar_pwm(VENTILADOR_CICLO_BAJO);  return VENT_BAJO;  }
	else if (temp_celsius <= T_MED_MAX){ ventilador_fijar_pwm(VENTILADOR_CICLO_MEDIO); return VENT_MEDIO; }
	else {                                ventilador_fijar_pwm(VENTILADOR_CICLO_ALTO);  return VENT_ALTO;  }
}

static void mostrar_rango_actual(uint16_t temp_celsius){
	if (temp_celsius <= T_CALOR_MAX){
		uart_tx_cadena("Rango: 0-"); uart_tx_entero16(T_CALOR_MAX);
		} else if (temp_celsius <= T_MEDIO_MAX){
		uart_tx_cadena("Rango: "); uart_tx_entero16(T_MEDIO_MIN); uart_tx_caracter('-'); uart_tx_entero16(T_MEDIO_MAX);
		} else if (temp_celsius <= T_BAJO_MAX){
		uart_tx_cadena("Rango: "); uart_tx_entero16((uint16_t)(T_MEDIO_MAX+1)); uart_tx_caracter('-'); uart_tx_entero16(T_BAJO_MAX);
		} else if (temp_celsius <= T_MED_MAX){
		uart_tx_cadena("Rango: "); uart_tx_entero16((uint16_t)(T_BAJO_MAX+1)); uart_tx_caracter('-'); uart_tx_entero16(T_MED_MAX);
		} else {
		uart_tx_cadena("Rango: >= "); uart_tx_entero16((uint16_t)(T_MED_MAX+1));
	}
}

static void log_por_uart(uint16_t temp_decimas, accion_ventilador_t accion_vent, uint16_t temp_celsius){
	uart_tx_cadena("Temperatura: ");
	uint16_t ent = temp_decimas / 10, dec = temp_decimas % 10;
	uart_tx_entero16(ent); uart_tx_caracter('.'); uart_tx_entero16(dec);
	uart_tx_cadena("°C | Ventilador: ");
	switch(accion_vent){
		case VENT_APAGADO: uart_tx_cadena("APAGADO"); break;
		case VENT_BAJO:    uart_tx_cadena("BAJO");    break;
		case VENT_MEDIO:   uart_tx_cadena("MEDIO");   break;
		case VENT_ALTO:    uart_tx_cadena("ALTO");    break;
	}
	uart_tx_cadena(" | Calefactor: ");
	
	if (OCR0B > 0) {
		uart_tx_cadena("ENCENDIDO");
		} else {
		uart_tx_cadena("APAGADO");
	}
	
	uart_tx_cadena(" | ");
	mostrar_rango_actual(temp_celsius);
	uart_tx_cadena("\r\n");
}


// comando para reconfigurar el punto medio
static void comando_reconfigurar_rango(void){
	uart_tx_cadena("\r\n Reconfigurar Rango Medio \r\n");
	uart_rx_limpiar_buffer();

	uart_tx_cadena("Ingrese minimo del nuevo rango: ");
	uint16_t min_medio = uart_leer_entero_flexible();

	uart_tx_cadena("Ingrese maximo del nuevo rango: ");
	uint16_t max_medio = uart_leer_entero_flexible();

	recalcular_desde_medio(min_medio, max_medio);

	uart_tx_cadena("Nuevo rango aplicado.\r\n");
}

static void procesar_uart(void){
	while (uart_rx_disponible()){
		uint8_t c = uart_rx_leer();
		if (c=='E'){
			g_modo_calefactor = CALEFACTOR_MANUAL;
			g_ciclo_manual_calefactor = CALEFACTOR_CICLO_ENCENDIDO;
			calefactor_fijar_pwm(g_ciclo_manual_calefactor);
			uart_tx_cadena("Calefactor ENCENDIDO\r\n");
			} else if (c=='A'){
			g_modo_calefactor = CALEFACTOR_MANUAL;
			g_ciclo_manual_calefactor = CALEFACTOR_CICLO_APAGADO;
			calefactor_fijar_pwm(g_ciclo_manual_calefactor);
			uart_tx_cadena("Calefactor APAGADO\r\n");
			} else if (c=='R'){
			comando_reconfigurar_rango();
			} else {
			uart_tx_cadena("[Ayuda] Comandos: E=Calefactor ENCENDIDO, A=Calefactor APAGADO, R=Rango medio\r\n");
		}
	}
}


int main(void){
	uart_inicializar(9600);
	adc_inicializar();
	calefactor_pwm_inicializar();
	ventilador_pwm_inicializar();

	uart_tx_cadena("Sistema de control de temperatura iniciado.\r\n");
	uart_tx_cadena("Comandos: 'E' Calefactor ENCENDIDO, 'A' Calefactor APAGADO, 'R' cambiar punto medio.\r\n");

	while(1){
		procesar_uart();

		// Lee la temperatura
		uint16_t temp_c_entero  = lm35_leer_celsius_entero();
		uint16_t temp_c_decimas = lm35_leer_celsius_x10();

		// Ventilador
		accion_ventilador_t accion_v = aplicar_control_ventilador(temp_c_entero);

		// Calefactor
		if (g_modo_calefactor == CALEFACTOR_MANUAL){
			calefactor_fijar_pwm(g_ciclo_manual_calefactor);
			} else {
			if (temp_c_entero <= T_CALOR_MAX) {
				calefactor_fijar_pwm(CALEFACTOR_CICLO_ENCENDIDO);
				} else {
				calefactor_fijar_pwm(CALEFACTOR_CICLO_APAGADO);
			}
		}

		log_por_uart(temp_c_decimas, accion_v, temp_c_entero);

		_delay_ms(1000);
	}
}