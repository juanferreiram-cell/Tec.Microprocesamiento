#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#ifndef UART_H
#define UART_H

#define UART_TIMEOUT_MS 500

void uart_init(unsigned int ubrr);

char uart_receive(void);

void uart_send(char c);

void uart_print(const char *s);

void uart_print_hex(uint8_t val);

void uart_print_hex_array(const uint8_t *arr, uint8_t len);

#endif