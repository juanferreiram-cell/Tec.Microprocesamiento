#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#ifndef RC522_H_
#define RC522_H_


#define SS_LOW()   (PORTB &= ~(1<<PB2))
#define SS_HIGH()  (PORTB |=  (1<<PB2))
#define RST_PIN PB1

// ==== Registros ====
#define CommandReg      0x01
#define CommIEnReg      0x02
#define CommIrqReg      0x04
#define DivIrqReg       0x05
#define ErrorReg        0x06
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define ModeReg         0x11
#define TxModeReg       0x12
#define RxModeReg       0x13
#define TxControlReg    0x14
#define TxASKReg        0x15
#define RFCfgReg        0x26
#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReloadRegH     0x2C
#define TReloadRegL     0x2D
#define VersionReg      0x37

#define PCD_IDLE        0x00
#define PCD_TRANSCEIVE  0x0C

#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93


void mfrc522_resetPinInit();

void mfrc522_write(uint8_t reg, uint8_t value);

uint8_t mfrc522_read(uint8_t reg);

void mfrc522_setBitMask(uint8_t reg, uint8_t mask);

void mfrc522_clearBitMask(uint8_t reg, uint8_t mask);

void mfrc522_printRegister(const char* name, uint8_t reg);

void mfrc522_reset();

void mfrc522_init();

void mfrc522_debug_init();

void mfrc522_standard(uint8_t *card_uid);

#endif