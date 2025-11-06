#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "RC522.h"

#ifndef RC522_C
#define RC522_C

void mfrc522_resetPinInit() {
	DDRB |= (1<<RST_PIN);
	// Realizar un reset hardware
	PORTB &= ~(1<<RST_PIN);
	_delay_ms(10);
	PORTB |= (1<<RST_PIN);
	_delay_ms(50);
}

void mfrc522_write(uint8_t reg, uint8_t value) {
	SS_LOW();
	spi_transfer((reg<<1) & 0x7E);
	spi_transfer(value);
	SS_HIGH();
}

uint8_t mfrc522_read(uint8_t reg) {
	uint8_t val;
	SS_LOW();
	spi_transfer(((reg<<1)&0x7E) | 0x80);
	val = spi_transfer(0x00);
	SS_HIGH();
	return val;
}

void mfrc522_setBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp = mfrc522_read(reg);
	mfrc522_write(reg, tmp | mask);
}

void mfrc522_clearBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp = mfrc522_read(reg);
	mfrc522_write(reg, tmp & (~mask));
}

void mfrc522_printRegister(const char* name, uint8_t reg) {
	uart_print(name);
	uart_print(": ");
	uart_print_hex(mfrc522_read(reg));
	uart_print("\r\n");
}


// ==== Funciones de depuracion ====
void mfrc522_reset() {
	uart_print("Soft Reset...\r\n");
	mfrc522_write(CommandReg, (1<<4));
	_delay_ms(50);
}

void mfrc522_init() {
	mfrc522_reset();

	uart_print("Configurando temporizadores y modulacion...\r\n");
	// Configuracion de temporizadores para 106 kbps
	mfrc522_write(TModeReg, 0x8D);     // 1000 1101 - Auto restart, timer starts
	mfrc522_write(TPrescalerReg, 0x3E); // 0011 1110 - Prescaler
	mfrc522_write(TReloadRegL, 30);     // Timer reload value
	mfrc522_write(TReloadRegH, 0);

	// Configuracion de la modulacion
	mfrc522_write(TxASKReg, 0x40);     // 100% ASK modulation
	mfrc522_write(ModeReg, 0x3D);      // CRC enabled, MSB first
	
	// Configurar ganancia del receptor
	mfrc522_write(RFCfgReg, 0x7F);     // Ganancia maxima (48dB)
	
	// Activar la antena
	mfrc522_write(TxControlReg, 0x83); // Antena ON, con control de ganancia
	_delay_ms(5); // Espera a que la antena estabilice
}


void mfrc522_debug_init() {
	mfrc522_reset();

	uart_print("Configurando temporizadores y modulacion...\r\n");
	// Configuracion de temporizadores para 106 kbps
	mfrc522_write(TModeReg, 0x8D);     // 1000 1101 - Auto restart, timer starts
	mfrc522_write(TPrescalerReg, 0x3E); // 0011 1110 - Prescaler
	mfrc522_write(TReloadRegL, 30);     // Timer reload value
	mfrc522_write(TReloadRegH, 0);

	// Configuracion de la modulacion
	mfrc522_write(TxASKReg, 0x40);     // 100% ASK modulation
	mfrc522_write(ModeReg, 0x3D);      // CRC enabled, MSB first
	
	// Configurar ganancia del receptor
	mfrc522_write(RFCfgReg, 0x7F);     // Ganancia maxima (48dB)
	
	// Activar la antena
	mfrc522_write(TxControlReg, 0x83); // Antena ON, con control de ganancia
	_delay_ms(5); // Espera a que la antena estabilice

	uart_print("Registros clave despues de init:\r\n");
	mfrc522_printRegister("VersionReg", VersionReg);
	mfrc522_printRegister("TxControlReg", TxControlReg);
	mfrc522_printRegister("TxASKReg", TxASKReg);
	mfrc522_printRegister("ModeReg", ModeReg);
	mfrc522_printRegister("TModeReg", TModeReg);
	mfrc522_printRegister("TPrescalerReg", TPrescalerReg);
	mfrc522_printRegister("TReloadRegH", TReloadRegH);
	mfrc522_printRegister("TReloadRegL", TReloadRegL);
	mfrc522_printRegister("RFCfgReg", RFCfgReg);
}

void mfrc522_debug_REQA() {
	uint8_t req[1] = {PICC_REQIDL};
	uint8_t buffer[16];
	uint8_t bufferLength = sizeof(buffer);
	uint8_t backBits = 0;
	uint8_t status;

	// Preparar registro de bits y FIFO
	mfrc522_write(BitFramingReg, 0x07); // 7 bits para REQA
	mfrc522_write(CommIrqReg, 0x7F);    // Limpiar IRQ
	mfrc522_write(FIFOLevelReg, 0x80);  // Limpiar FIFO

	uart_print("\r\n=== Enviando REQA ===\r\n");
	uart_print("CommIrqReg antes: "); mfrc522_printRegister("", CommIrqReg);
	uart_print("FIFOLevelReg antes: "); mfrc522_printRegister("", FIFOLevelReg);

	// Escribir FIFO
	for (uint8_t i=0; i<1; i++) {
		mfrc522_write(FIFODataReg, req[i]);
		uart_print("FIFODataReg[escrito]: "); uart_print_hex(req[i]); uart_print("\r\n");
	}

	// Iniciar transaccion
	mfrc522_write(CommandReg, PCD_TRANSCEIVE);
	mfrc522_setBitMask(BitFramingReg, 0x80); // StartSend

	// Esperar IRQ (RxIRq o Timeout)
	uint16_t count = 1000; // Reducir el tiempo de espera
	uint8_t irq;
	do {
		irq = mfrc522_read(CommIrqReg);
		count--;
	} while (!(irq & 0x30) && count); // RxIRq o IdleIRq

	mfrc522_clearBitMask(BitFramingReg, 0x80);

	uart_print("CommIrqReg despu s: "); mfrc522_printRegister("", CommIrqReg);
	uart_print("FIFOLevelReg despu s: "); mfrc522_printRegister("", FIFOLevelReg);
	uart_print("ErrorReg: "); mfrc522_printRegister("", ErrorReg);

	if (count == 0) {
		uart_print("Timeout REQA, tarjeta no detectada\r\n");
		} else {
		uart_print("IRQ activada, leer datos FIFO\r\n");
		uint8_t fifoLevel = mfrc522_read(FIFOLevelReg);
		uart_print("FIFOLevelReg: "); uart_print_hex(fifoLevel); uart_print("\r\n");
		for (uint8_t i=0; i<fifoLevel; i++) {
			uint8_t val = mfrc522_read(FIFODataReg);
			uart_print("FIFODataReg[leido]: "); uart_print_hex(val); uart_print("\r\n");
			if (i < bufferLength) {
				buffer[i] = val;
			}
		}
		
		// Si hay datos, intentar leer el UID
		if (fifoLevel > 0) {
			uart_print("Tarjeta detectada! Intentando leer UID...\r\n");
			
			// Anticollision
			mfrc522_write(BitFramingReg, 0x00);
			mfrc522_write(CommIrqReg, 0x7F);
			mfrc522_write(FIFOLevelReg, 0x80);
			
			// Escribir comando Anticollision en FIFO
			mfrc522_write(FIFODataReg, PICC_ANTICOLL);
			mfrc522_write(FIFODataReg, 0x20); // NVB - Number of Valid Bits
			
			// Iniciar transaccion
			mfrc522_write(CommandReg, PCD_TRANSCEIVE);
			mfrc522_setBitMask(BitFramingReg, 0x80); // StartSend
			
			// Esperar IRQ
			count = 1000;
			do {
				irq = mfrc522_read(CommIrqReg);
				count--;
			} while (!(irq & 0x30) && count);
			
			mfrc522_clearBitMask(BitFramingReg, 0x80);
			
			if (count == 0) {
				uart_print("Timeout Anticollision\r\n");
				} else {
				fifoLevel = mfrc522_read(FIFOLevelReg);
				uart_print("UID leido: ");
				for (uint8_t i=0; i<fifoLevel; i++) {
					uint8_t val = mfrc522_read(FIFODataReg);
					uart_print_hex(val);
					uart_print(" ");
				}
				uart_print("\r\n");
			}
		}
	}
}

// REEMPLAZA LA FUNCIÓN mfrc522_standard EN RC522.c CON ESTA:

void mfrc522_standard(uint8_t *card_uid) {
	uint8_t req[1] = {PICC_REQIDL};
	uint8_t buffer[16];
	uint8_t bufferLength = sizeof(buffer);
	
	// --- SOLUCIÓN: Limpiar el búfer de salida al inicio ---
	// Usamos 5 bytes, que es el tamaño de serNum[] en main.c
	memset(card_uid, 0, 5);

	// Preparar registro de bits y FIFO
	mfrc522_write(BitFramingReg, 0x07); // 7 bits para REQA
	mfrc522_write(CommIrqReg, 0x7F);    // Limpiar IRQ
	mfrc522_write(FIFOLevelReg, 0x80);  // Limpiar FIFO

	// Escribir FIFO
	for (uint8_t i=0; i<1; i++) {
		mfrc522_write(FIFODataReg, req[i]);
	}

	// Iniciar transaccion
	mfrc522_write(CommandReg, PCD_TRANSCEIVE);
	mfrc522_setBitMask(BitFramingReg, 0x80); // StartSend

	// Esperar IRQ (RxIRq o Timeout)
	uint16_t count = 1000;
	uint8_t irq;
	do {
		irq = mfrc522_read(CommIrqReg);
		count--;
	} while (!(irq & 0x30) && count); // RxIRq o IdleIRq

	mfrc522_clearBitMask(BitFramingReg, 0x80);

	if (count == 0) {
		// Timeout en REQA. card_uid ya está en 0. Salir.
		return;
	}
	
	// --- REQA Tuvo éxito ---
	uint8_t fifoLevel = mfrc522_read(FIFOLevelReg);
	for (uint8_t i=0; i<fifoLevel; i++) {
		uint8_t val = mfrc522_read(FIFODataReg);
		if (i < bufferLength) {
			buffer[i] = val; // Guardar ATQA (tipo de tarjeta) en buffer local
		}
	}
	
	if (fifoLevel > 0) {
		// Tarjeta detectada, intentar Anticollision
		
		mfrc522_write(BitFramingReg, 0x00);
		mfrc522_write(CommIrqReg, 0x7F);
		mfrc522_write(FIFOLevelReg, 0x80);
		
		mfrc522_write(FIFODataReg, PICC_ANTICOLL);
		mfrc522_write(FIFODataReg, 0x20); // NVB
		
		mfrc522_write(CommandReg, PCD_TRANSCEIVE);
		mfrc522_setBitMask(BitFramingReg, 0x80);
		
		count = 1000;
		do {
			irq = mfrc522_read(CommIrqReg);
			count--;
		} while (!(irq & 0x30) && count);
		
		mfrc522_clearBitMask(BitFramingReg, 0x80);
		
		if (count == 0) {
			// Timeout en Anticollision. card_uid ya está en 0. Salir.
			return;
		}
		
		// --- Anticollision Tuvo Éxito ---
		fifoLevel = mfrc522_read(FIFOLevelReg);
		
		// --- SOLUCIÓN: Implementar validación de Checksum (BCC) ---
		// El lector debe devolver 5 bytes (4 de ID + 1 de Checksum)
		if (fifoLevel != 5) {
			// Error, no se recibieron 5 bytes. card_uid ya está en 0. Salir.
			return;
		}
		
		uint8_t bcc_calculado = 0;
		
		// Leer los 4 bytes del ID
		for (uint8_t i=0; i<4; i++) {
			uint8_t val = mfrc522_read(FIFODataReg);
			card_uid[i] = val;      // Guardar el byte del ID
			bcc_calculado ^= val;   // Calcular el checksum
		}
		
		// Leer el 5to byte (el Checksum)
		uint8_t bcc_leido = mfrc522_read(FIFODataReg);

		if (bcc_calculado != bcc_leido) {
			// El checksum es incorrecto! Es una mala lectura.
			// Borrar el buffer y salir.
			memset(card_uid, 0, 5);
			return;
		}
		
		// ¡ÉXITO! card_uid[0-3] tiene un ID válido.
		// card_uid[4] sigue siendo 0 (del primer memset).
		// La comprobación 'if (serNum[0] != 0)' en main.c funcionará.
	}
}

#endif