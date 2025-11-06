#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include  "SPI.h"

#ifndef SPI_C
#define SPI_C

void spi_init(void) {
	DDRB |= (1<<PB2)|(1<<PB3)|(1<<PB5); // SS, MOSI, SCK salidas
	DDRB &= ~(1<<PB4); // MISO entrada
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = (1<<SPI2X); // fosc/8
}

uint8_t spi_transfer(uint8_t data) {
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

#endif
