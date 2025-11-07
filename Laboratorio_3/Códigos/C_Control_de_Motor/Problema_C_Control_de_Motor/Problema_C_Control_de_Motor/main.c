/*
 * Controlador PID simplificado para motor DC con realimentaci?n de posici?n
 * Este programa controla la posici?n de un motor usando un potenci?metro
 * como sensor de posici?n y otro como referencia
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

// Definici?n de pines para control de direcci?n del motor
#define IN1 PD2
#define IN2 PD3

// Constantes del sistema de control
static const uint16_t TOLERANCIA    = 15;   // Margen de error aceptable
static const uint8_t  PWM_MINIMO    = 150;  // Valor m?nimo de PWM para vencer fricci?n
static const uint8_t  PWM_MAXIMO    = 170;  // Valor m?ximo de PWM permitido

/*
 * Funci?n para enviar caracteres por UART
 * Se encarga del env?o de caracteres y convierte '\n' en '\r\n'
 */
static int uart_putchar(char c, FILE *stream){
	if (c == '\n'){ 
		while(!(UCSR0A & (1<<UDRE0)));  // Espera hasta que el buffer est? vac?o
		UDR0 = '\r';                    // Env?a retorno de carro
	}
	while(!(UCSR0A & (1<<UDRE0)));      // Espera hasta que el buffer est? vac?o
	UDR0 = c;                           // Env?a el caracter
	return 0;
}

// Configuraci?n del stream de salida est?ndar para UART
static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/*
 * Inicializa el m?dulo UART para comunicaci?n serial
 * Configura la velocidad de transmisi?n a 9600 baudios
 */
void iniciar_uart(uint32_t baud){
	uint16_t ubrr = (F_CPU/16/baud) - 1;    // Calcula el valor para el registro UBRR
	UBRR0H = (uint8_t)(ubrr>>8);            // Configura byte alto del divisor de baudios
	UBRR0L = (uint8_t)(ubrr);               // Configura byte bajo del divisor de baudios
	UCSR0B = (1<<TXEN0);                    // Habilita el transmisor
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);      // Configura 8 bits de datos, 1 bit de parada
	stdout = &uart_stdout;                  // Redirige la salida est?ndar a UART
}

/*
 * Configura el Timer1 para generar se?al PWM en el pin PB1
 * Modo Fast PWM de 8 bits, preescaler de 8
 */
void iniciar_pwm(void){
	DDRB |= (1 << PB1);                    // Configura PB1 como salida (OC1A)
	TCCR1A = (1 << COM1A1) | (1 << WGM10); // PWM no invertido, modo Fast PWM 8-bit
	TCCR1B = (1 << WGM12) | (1 << CS11);   // Modo Fast PWM, preescaler de 8
}

/*
 * Ajusta el ciclo de trabajo del PWM
 * valor: ciclo de trabajo entre 0 y 255
 */
void ajustar_pwm(uint8_t valor){
	OCR1A = valor;  // Establece el valor de comparaci?n para el PWM
}

/*
 * Inicializa los pines de control del motor
 * Configura IN1 e IN2 como salidas y las pone en estado bajo
 */
void iniciar_motor(void){
	DDRD |= (1 << IN1) | (1 << IN2);      // Configura pines como salidas
	PORTD &= ~((1 << IN1) | (1 << IN2));  // Apaga ambos pines (motor detenido)
}

/*
 * Hace girar el motor en sentido horario
 * Configura IN1 alto e IN2 bajo
 */
void motor_derecha(void){
	PORTD |=  (1 << IN1);   // Activa IN1
	PORTD &= ~(1 << IN2);   // Desactiva IN2
}

/*
 * Hace girar el motor en sentido antihorario
 * Configura IN1 bajo e IN2 alto
 */
void motor_izquierda(void){
	PORTD &= ~(1 << IN1);   // Desactiva IN1
	PORTD |=  (1 << IN2);   // Activa IN2
}

/*
 * Detiene el motor
 * Pone ambos pines en bajo y ajusta PWM a cero
 */
void motor_parar(void){
	PORTD &= ~((1 << IN1) | (1 << IN2));  // Desactiva ambos pines de control
	ajustar_pwm(0);                       // Apaga la se?al PWM
}

/*
 * Inicializa el conversor anal?gico-digital (ADC)
 * Configura referencia AVcc, preescaler de 128
 */
void iniciar_adc(void){
	ADMUX  = (1 << REFS0);                              // Referencia AVcc
	ADCSRA = (1 << ADEN) | (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Habilita ADC, preescaler 128
	DIDR0  = (1 << ADC0D) | (1 << ADC1D);               // Deshabilita entradas digitales en ADC0 y ADC1
	ADCSRA |= (1 << ADSC);                              // Inicia primera conversi?n
	while(ADCSRA & (1 << ADSC));                        // Espera a que termine la conversi?n
}

/*
 * Lee un valor del ADC en el canal especificado
 * canal: n?mero de canal ADC a leer (0-7)
 * retorna: valor digital de 10 bits (0-1023)
 */
uint16_t leer_adc(uint8_t canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);  // Selecciona canal manteniendo configuraci?n
	_delay_us(5);                             // Espera estabilizaci?n
	ADCSRA |= (1 << ADSC);                    // Inicia conversi?n
	while(ADCSRA & (1 << ADSC));              // Espera a que termine la conversi?n
	return ADC;                               // Retorna el resultado
}

/*
 * Funci?n principal del programa
 * Implementa el lazo de control para posicionar el motor
 */
int main(void){
	// Inicializaci?n de perif?ricos
	iniciar_uart(9600);      // Comunicaci?n serial a 9600 baudios
	iniciar_adc();           // Conversor anal?gico-digital
	iniciar_motor();         // Control del motor
	iniciar_pwm();           // Generaci?n de PWM
	
	// Encabezado para datos de depuraci?n
	printf("ref,eje,pwm,sentido\n");
	
	// Lazo principal de control
	while(1){
		// Lectura de sensores
		uint16_t referencia = leer_adc(3);  // Lee potenci?metro de referencia (canal 3)
		uint16_t posicion   = leer_adc(4);  // Lee potenci?metro de posici?n (canal 4)
		
		// C?lculo del error
		int16_t error = (int16_t)referencia - (int16_t)posicion;
		
		// Variables para el control
		uint8_t pwm_salida = 0;
		const char *direccion = "STOP";
		
		// Verifica si el error est? dentro de la tolerancia
		if (error > -((int16_t)TOLERANCIA) && error < (int16_t)TOLERANCIA){
			// Error dentro de tolerancia - detiene el motor
			motor_parar();
			pwm_salida = 0;
			direccion = "STOP";
		} else {
			// Error fuera de tolerancia - controla direcci?n
			if (error > 0){
				// Error positivo - gira a la izquierda
				motor_izquierda();
				direccion = "IZQ";
			} else {
				// Error negativo - gira a la derecha  
				motor_derecha();
				direccion = "DER";
			}
			
			// Calcula magnitud del error (valor absoluto)
			uint16_t magnitud = (error < 0) ? (uint16_t)(-error) : (uint16_t)error;
			uint16_t duty = magnitud;
			
			// Limita el valor de PWM entre m?nimo y m?ximo
			if (duty < PWM_MINIMO) duty = PWM_MINIMO;
			if (duty > PWM_MAXIMO) duty = PWM_MAXIMO;
			pwm_salida = (uint8_t)duty;
			
			// Aplica el valor de PWM calculado
			ajustar_pwm(pwm_salida);
		}
		
		// Env?a datos por puerto serial para monitoreo
		printf("%u,%u,%u,%s\n", referencia, posicion, pwm_salida, direccion);
		
		// Peque?a pausa para estabilidad
		_delay_ms(50);
	}
}