#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>


#ifndef SPI_H
#define SPI_H

void spi_init(void);

uint8_t spi_transfer(uint8_t data);

#endif
