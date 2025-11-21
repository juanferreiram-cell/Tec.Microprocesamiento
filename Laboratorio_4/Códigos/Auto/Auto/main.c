#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// ===== PINES MOTORES =====
#define MOTOR_IZQ_IN1 PD4
#define MOTOR_IZQ_IN2 PB0
#define MOTOR_IZQ_PWM PD6

#define MOTOR_DER_IN1 PB1
#define MOTOR_DER_IN2 PB5
#define MOTOR_DER_PWM PD5

// ===== CONFIGURACION UART DEBUG =====
#define DEBUG_BAUD 9600
#define DEBUG_UBRR ((F_CPU/16/DEBUG_BAUD)-1)

// ===== VELOCIDADES PWM =====
#define VEL_AVANCE 200
#define VEL_GIRO 150

uint8_t velocidad_actual = VEL_AVANCE;

// ===== UART HARDWARE (Debug) =====
void Debug_Init(void) {
	UBRR0H = (unsigned char)(DEBUG_UBRR >> 8);
	UBRR0L = (unsigned char)DEBUG_UBRR;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
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

uint8_t Debug_Available(void) {
	return (UCSR0A & (1 << RXC0));
}

char Debug_Read(void) {
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

// ===== INICIALIZACION PWM =====
void init_pwm(void) {
	DDRD |= (1 << MOTOR_IZQ_PWM);
	DDRD |= (1 << MOTOR_DER_PWM);
	
	TCCR0A = (1 << WGM00) | (1 << WGM01);
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0B = (1 << CS01);
	
	OCR0A = 0;
	OCR0B = 0;
}

// ===== CONTROL DE MOTORES =====
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

// ===== FUNCIONES DE MOVIMIENTO =====
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

// ===== PROCESAMIENTO DE COMANDOS =====
void processCommand(char cmd) {
	switch(cmd) {
		case 'F':
		Debug_PrintString("ADELANTE\r\n");
		avanzar();
		break;
		
		case 'B':
		Debug_PrintString("ATRAS\r\n");
		retroceder();
		break;
		
		case 'L':
		Debug_PrintString("IZQUIERDA\r\n");
		girar_izquierda();
		break;
		
		case 'R':
		Debug_PrintString("DERECHA\r\n");
		girar_derecha();
		break;
		
		case 'S':
		Debug_PrintString("STOP\r\n");
		detener();
		break;
		
		case '0': velocidad_actual = 0; Debug_PrintString("VEL: 0\r\n"); break;
		case '1': velocidad_actual = 50; Debug_PrintString("VEL: 1\r\n"); break;
		case '2': velocidad_actual = 100; Debug_PrintString("VEL: 2\r\n"); break;
		case '3': velocidad_actual = 150; Debug_PrintString("VEL: 3\r\n"); break;
		case '4': velocidad_actual = 200; Debug_PrintString("VEL: 4\r\n"); break;
		case '5': velocidad_actual = 255; Debug_PrintString("VEL: 5\r\n"); break;
		
		default:
		break;
	}
}

// ===== GPIO =====
void GPIO_Init(void) {
	DDRD |= (1 << MOTOR_IZQ_IN1);
	DDRB |= (1 << MOTOR_IZQ_IN2);
	DDRB |= (1 << MOTOR_DER_IN1) | (1 << MOTOR_DER_IN2);
}

int main(void) {
	GPIO_Init();
	Debug_Init();
	init_pwm();
	
	_delay_ms(1000);
	
	Debug_PrintString("Robot Control Basico - Version 1\r\n");
	Debug_PrintString("Comandos: F B L R S 0-5\r\n");
	Debug_PrintString("Esperando comandos...\r\n\r\n");
	
	while (1) {
		if (Debug_Available()) {
			char cmd = Debug_Read();
			processCommand(cmd);
		}
	}
	
	return 0;
}