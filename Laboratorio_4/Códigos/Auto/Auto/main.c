#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SENSOR_IZQ PD2
#define SENSOR_DER PD7

#define MOTOR_IZQ_IN1 PD4
#define MOTOR_IZQ_IN2 PB0
#define MOTOR_IZQ_PWM PD6

#define MOTOR_DER_IN1 PB1
#define MOTOR_DER_IN2 PB5
#define MOTOR_DER_PWM PD5

#define METEGOL_IN1 PC0
#define METEGOL_IN2 PC1
#define METEGOL_PWM PB4

#define HC05_RX_PIN PB2
#define HC05_TX_PIN PB3

#define DEBUG_BAUD 9600
#define DEBUG_UBRR ((F_CPU/16/DEBUG_BAUD)-1)

#define HC05_BAUD 38400
#define HC05_BIT_DELAY 26

#define VEL_AVANCE 200
#define VEL_GIRO 150

#define LEER_IZQ() ((PIND & (1 << SENSOR_IZQ)) >> SENSOR_IZQ)
#define LEER_DER() ((PIND & (1 << SENSOR_DER)) >> SENSOR_DER)

char receivedCommand = 0;
uint8_t velocidad_actual = VEL_AVANCE;

void Debug_Init(void) {
	UBRR0H = (unsigned char)(DEBUG_UBRR >> 8);
	UBRR0L = (unsigned char)DEBUG_UBRR;
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Debug_Print(unsigned char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void Debug_PrintString(const char* str) {
	while (*str) {
		Debug_Print(*str++);
	}
}

void Debug_PrintHex(uint8_t num) {
	const char hexDigits[] = "0123456789ABCDEF";
	Debug_Print(hexDigits[(num >> 4) & 0x0F]);
	Debug_Print(hexDigits[num & 0x0F]);
}

void HC05_Init(void) {
	DDRB |= (1 << HC05_TX_PIN);
	PORTB |= (1 << HC05_TX_PIN);
	
	DDRB &= ~(1 << HC05_RX_PIN);
	PORTB |= (1 << HC05_RX_PIN);
}

void HC05_Write(unsigned char data) {
	cli();
	
	PORTB &= ~(1 << HC05_TX_PIN);
	_delay_us(HC05_BIT_DELAY);
	
	for (uint8_t i = 0; i < 8; i++) {
		if (data & 0x01) {
			PORTB |= (1 << HC05_TX_PIN);
			} else {
			PORTB &= ~(1 << HC05_TX_PIN);
		}
		data >>= 1;
		_delay_us(HC05_BIT_DELAY);
	}
	
	PORTB |= (1 << HC05_TX_PIN);
	_delay_us(HC05_BIT_DELAY);
	
	sei();
}

void HC05_WriteString(const char* str) {
	while (*str) {
		HC05_Write(*str++);
	}
}

uint8_t HC05_Read(char* data) {
	uint16_t timeout = 0;
	
	while ((PINB & (1 << HC05_RX_PIN)) && timeout < 50000) {
		timeout++;
	}
	
	if (timeout >= 50000) {
		return 0;
	}
	
	_delay_us(HC05_BIT_DELAY / 2);
	
	if (PINB & (1 << HC05_RX_PIN)) {
		return 0;
	}
	
	uint8_t byte = 0;
	
	for (uint8_t i = 0; i < 8; i++) {
		_delay_us(HC05_BIT_DELAY);
		byte >>= 1;
		if (PINB & (1 << HC05_RX_PIN)) {
			byte |= 0x80;
		}
	}
	
	_delay_us(HC05_BIT_DELAY);
	
	if (!(PINB & (1 << HC05_RX_PIN))) {
		return 0;
	}
	
	*data = byte;
	return 1;
}

void init_pwm(void) {
	DDRD |= (1 << MOTOR_IZQ_PWM);
	DDRD |= (1 << MOTOR_DER_PWM);
	
	TCCR0A = (1 << WGM00) | (1 << WGM01);
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0B = (1 << CS01);
	
	OCR0A = 0;
	OCR0B = 0;
}

void set_motor_izq(int16_t velocidad) {
	if (velocidad > 0) {
		PORTD |= (1 << MOTOR_IZQ_IN1);
		PORTB &= ~(1 << MOTOR_IZQ_IN2);
		OCR0A = (velocidad > 255) ? 255 : velocidad;
	}
	else if (velocidad < 0) {
		PORTD &= ~(1 << MOTOR_IZQ_IN1);
		PORTB |= (1 << MOTOR_IZQ_IN2);
		OCR0A = (velocidad < -255) ? 255 : -velocidad;
	}
	else {
		PORTD |= (1 << MOTOR_IZQ_IN1);
		PORTB |= (1 << MOTOR_IZQ_IN2);
		OCR0A = 255;
	}
}

void set_motor_der(int16_t velocidad) {
	if (velocidad > 0) {
		PORTB |= (1 << MOTOR_DER_IN1);
		PORTB &= ~(1 << MOTOR_DER_IN2);
		OCR0B = (velocidad > 255) ? 255 : velocidad;
	}
	else if (velocidad < 0) {
		PORTB &= ~(1 << MOTOR_DER_IN1);
		PORTB |= (1 << MOTOR_DER_IN2);
		OCR0B = (velocidad < -255) ? 255 : -velocidad;
	}
	else {
		PORTB |= (1 << MOTOR_DER_IN1);
		PORTB |= (1 << MOTOR_DER_IN2);
		OCR0B = 255;
	}
}

void coast_motor_izq(void) {
	PORTD &= ~(1 << MOTOR_IZQ_IN1);
	PORTB &= ~(1 << MOTOR_IZQ_IN2);
	OCR0A = 0;
}

void coast_motor_der(void) {
	PORTB &= ~((1 << MOTOR_DER_IN1) | (1 << MOTOR_DER_IN2));
	OCR0B = 0;
}

void metegol_girar(uint8_t velocidad, uint16_t duracion_ms) {
	Debug_PrintString("Metegol: Girando...\r\n");
	
	PORTC |= (1 << METEGOL_IN1);
	PORTC &= ~(1 << METEGOL_IN2);
	
	uint16_t ciclos = duracion_ms;
	uint16_t tiempo_on = (velocidad * 1000UL) / 255;
	uint16_t tiempo_off = 1000 - tiempo_on;
	
	for (uint16_t i = 0; i < ciclos; i++) {
		if (tiempo_on > 0) {
			PORTB |= (1 << METEGOL_PWM);
			for (uint16_t j = 0; j < tiempo_on; j++) {
				_delay_us(1);
			}
		}
		
		if (tiempo_off > 0) {
			PORTB &= ~(1 << METEGOL_PWM);
			for (uint16_t j = 0; j < tiempo_off; j++) {
				_delay_us(1);
			}
		}
	}
	
	PORTC |= (1 << METEGOL_IN1);
	PORTC |= (1 << METEGOL_IN2);
	PORTB |= (1 << METEGOL_PWM);
	_delay_ms(50);
	
	PORTC &= ~((1 << METEGOL_IN1) | (1 << METEGOL_IN2));
	PORTB &= ~(1 << METEGOL_PWM);
	
	Debug_PrintString("Metegol: Detenido\r\n");
}

void detener(void) {
	set_motor_izq(0);
	set_motor_der(0);
}

void avanzar(void) {
	set_motor_izq(velocidad_actual);
	set_motor_der(velocidad_actual);
}

void retroceder(void) {
	set_motor_izq(-velocidad_actual);
	set_motor_der(-velocidad_actual);
}

void girar_izquierda(void) {
	set_motor_izq(-VEL_GIRO);
	set_motor_der(VEL_GIRO);
}

void girar_derecha(void) {
	set_motor_izq(VEL_GIRO);
	set_motor_der(-VEL_GIRO);
}

void diagonal_adelante_izq(void) {
	coast_motor_izq();
	set_motor_der(velocidad_actual);
}

void diagonal_adelante_der(void) {
	set_motor_izq(velocidad_actual);
	coast_motor_der();
}

void diagonal_atras_izq(void) {
	set_motor_izq(-velocidad_actual);
	coast_motor_der();
}

void diagonal_atras_der(void) {
	coast_motor_izq();
	set_motor_der(-velocidad_actual);
}

uint8_t puede_moverse(char comando) {
	uint8_t izq = LEER_IZQ();
	uint8_t der = LEER_DER();
	
	if (!izq && !der) {
		return 1;
	}
	
	if (izq && der) {
		if (comando == 'B' || comando == 'S') {
			return 1;
		}
		Debug_PrintString("BLOQUEADO: Ambos sensores detectan linea\r\n");
		HC05_WriteString("BLOQ:BORDE\r\n");
		return 0;
	}
	
	if (izq && !der) {
		if (comando == 'L' || comando == 'Q' || comando == 'Z') {
			Debug_PrintString("BLOQUEADO: Sensor izquierdo activo\r\n");
			HC05_WriteString("BLOQ:IZQ\r\n");
			return 0;
		}
		return 1;
	}
	
	if (!izq && der) {
		if (comando == 'R' || comando == 'E' || comando == 'C') {
			Debug_PrintString("BLOQUEADO: Sensor derecho activo\r\n");
			HC05_WriteString("BLOQ:DER\r\n");
			return 0;
		}
		return 1;
	}
	
	return 1;
}

void processCommand(char cmd) {
	Debug_PrintString("CMD: '");
	Debug_Print(cmd);
	Debug_PrintString("' [0x");
	Debug_PrintHex(cmd);
	Debug_PrintString("] ");
	
	if (!puede_moverse(cmd)) {
		detener();
		return;
	}
	
	switch(cmd) {
		case 'F':
		Debug_PrintString("ADELANTE\r\n");
		avanzar();
		HC05_WriteString("OK:F\r\n");
		break;
		
		case 'B':
		Debug_PrintString("ATRAS\r\n");
		retroceder();
		HC05_WriteString("OK:B\r\n");
		break;
		
		case 'L':
		Debug_PrintString("IZQUIERDA\r\n");
		girar_izquierda();
		HC05_WriteString("OK:L\r\n");
		break;
		
		case 'R':
		Debug_PrintString("DERECHA\r\n");
		girar_derecha();
		HC05_WriteString("OK:R\r\n");
		break;
		
		case 'S':
		Debug_PrintString("STOP\r\n");
		detener();
		HC05_WriteString("OK:S\r\n");
		break;
		
		case 'Q':
		Debug_PrintString("DIAG A-I\r\n");
		diagonal_adelante_izq();
		HC05_WriteString("OK:Q\r\n");
		break;
		
		case 'E':
		Debug_PrintString("DIAG A-D\r\n");
		diagonal_adelante_der();
		HC05_WriteString("OK:E\r\n");
		break;
		
		case 'Z':
		Debug_PrintString("DIAG B-I\r\n");
		diagonal_atras_izq();
		HC05_WriteString("OK:Z\r\n");
		break;
		
		case 'C':
		Debug_PrintString("DIAG B-D\r\n");
		diagonal_atras_der();
		HC05_WriteString("OK:C\r\n");
		break;
		
		case 'X':
		Debug_PrintString("METEGOL SUAVE\r\n");
		HC05_WriteString("OK:X-SUAVE\r\n");
		metegol_girar(255, 100);
		break;
		
		case 'Y':
		Debug_PrintString("METEGOL MAXIMO\r\n");
		HC05_WriteString("OK:Y-MAX\r\n");
		metegol_girar(255, 500);
		break;
		
		case '0': velocidad_actual = 0; Debug_PrintString("VEL: 0\r\n"); HC05_WriteString("OK:0\r\n"); break;
		case '1': velocidad_actual = 50; Debug_PrintString("VEL: 1\r\n"); HC05_WriteString("OK:1\r\n"); break;
		case '2': velocidad_actual = 100; Debug_PrintString("VEL: 2\r\n"); HC05_WriteString("OK:2\r\n"); break;
		case '3': velocidad_actual = 150; Debug_PrintString("VEL: 3\r\n"); HC05_WriteString("OK:3\r\n"); break;
		case '4': velocidad_actual = 200; Debug_PrintString("VEL: 4\r\n"); HC05_WriteString("OK:4\r\n"); break;
		case '5': velocidad_actual = 255; Debug_PrintString("VEL: 5\r\n"); HC05_WriteString("OK:5\r\n"); break;
		
		case '\r':
		case '\n':
		case 0x00:
		case 0xFF:
		break;
		
		default:
		Debug_PrintString("DESCONOCIDO\r\n");
		break;
	}
}

void GPIO_Init(void) {
	DDRD |= (1 << MOTOR_IZQ_IN1);
	DDRB |= (1 << MOTOR_IZQ_IN2);
	
	DDRB |= (1 << MOTOR_DER_IN1) | (1 << MOTOR_DER_IN2);
	
	DDRC |= (1 << METEGOL_IN1) | (1 << METEGOL_IN2);
	DDRB |= (1 << METEGOL_PWM);
	PORTC &= ~((1 << METEGOL_IN1) | (1 << METEGOL_IN2));
	PORTB &= ~(1 << METEGOL_PWM);
	
	DDRD &= ~((1 << SENSOR_IZQ) | (1 << SENSOR_DER));
	PORTD |= (1 << SENSOR_IZQ) | (1 << SENSOR_DER);
}

int main(void) {
	GPIO_Init();
	Debug_Init();
	HC05_Init();
	init_pwm();
	
	_delay_ms(1000);
	
	Debug_PrintString("Sistema BLE con Sensores de Linea\r\n");
	Debug_PrintString("HC-05 Baudrate: 38400\r\n");
	
	HC05_WriteString("Sistema listo - Sensores activos\r\n");
	
	Debug_PrintString("Esperando comandos...\r\n\r\n");
	
	while (1) {
		if (HC05_Read(&receivedCommand)) {
			processCommand(receivedCommand);
			_delay_ms(10);
		}
	}
	
	return 0;
}