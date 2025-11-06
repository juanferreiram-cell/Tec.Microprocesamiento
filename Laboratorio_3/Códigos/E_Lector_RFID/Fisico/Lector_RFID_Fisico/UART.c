#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "UART.h"

#ifndef UART_C
#define UART_C

void uart_init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

char uart_receive(void) {
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void uart_send(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void uart_print(const char *s) {
	while (*s) uart_send(*s++);
}

void uart_print_hex(uint8_t val) {
	char buf[6];
	sprintf(buf, "0x%02X", val);
	uart_print(buf);
}

void uart_print_hex_array(const uint8_t *arr, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		uart_print_hex(arr[i]);
		uart_send(' ');
	}
	uart_print("\r\n");
}

#endif