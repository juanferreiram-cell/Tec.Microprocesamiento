#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>

#include "UART.h"
#include "SPI.h"
#include "RC522.h"

// Distribución de pines:

// LEDs
#define LED_VERDE_DDR DDRD
#define LED_VERDE_PORT PORTD
#define LED_VERDE_PIN PD7

#define LED_ROJO_DDR DDRD
#define LED_ROJO_PORT PORTD
#define LED_ROJO_PIN PD6

// Botones
#define BOTON_BORRAR_DDR DDRD
#define BOTON_BORRAR_PORT PORTD
#define BOTON_BORRAR_PIN PIND
#define BOTON_BORRAR_BIT PD4

#define BOTON_REGISTRAR_DDR DDRD
#define BOTON_REGISTRAR_PORT PORTD
#define BOTON_REGISTRAR_PIN PIND
#define BOTON_REGISTRAR_BIT PD5

// Direcciones de la EEPROM
#define EEPROM_DIR_TARJETA_VALIDA 0x00
#define EEPROM_DIR_TARJETA_ID 0x01



// LCD I2C
#define LCD_DIRECCION 0x27 // Dirección I2C del LCD
#define LCD_LUZ_FONDO 0x08
#define LCD_HABILITAR 0x04
#define LCD_RW 0x02
#define LCD_RS 0x01


volatile uint8_t tarjeta_registrada_id[4] = {0, 0, 0, 0};
volatile uint8_t hay_tarjeta_registrada = 0; // 0 = no hay 1 = si hay
volatile uint8_t modo = 0; // 0=normal 1=registro 2=borrado


// funciones I2C
void I2C_Inicializar(void) {
	TWSR = 0x00;
	TWBR = 0x48;
	TWCR = (1 << TWEN);
}

void I2C_Iniciar(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void I2C_Detener(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	_delay_us(100);
}

void I2C_Escribir(uint8_t dato) {
	TWDR = dato;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// LCD
void LCD_EnviarNibble(uint8_t nibble, uint8_t rs) {
	uint8_t dato = nibble | LCD_LUZ_FONDO;
	if (rs) dato |= LCD_RS;
	
	I2C_Iniciar();
	I2C_Escribir(LCD_DIRECCION << 1);
	I2C_Escribir(dato | LCD_HABILITAR);
	_delay_us(1);
	I2C_Escribir(dato);
	_delay_us(50);
	I2C_Detener();
}

void LCD_EnviarByte(uint8_t dato, uint8_t rs) {
	LCD_EnviarNibble(dato & 0xF0, rs);
	LCD_EnviarNibble((dato << 4) & 0xF0, rs);
}

void LCD_Comando(uint8_t cmd) {
	LCD_EnviarByte(cmd, 0);
	_delay_ms(2);
}

void LCD_Dato(uint8_t dato) {
	LCD_EnviarByte(dato, 1);
}

void LCD_Inicializar(void) {
	_delay_ms(50);
	LCD_EnviarNibble(0x30, 0);
	_delay_ms(5);
	LCD_EnviarNibble(0x30, 0);
	_delay_us(150);
	LCD_EnviarNibble(0x30, 0);
	LCD_EnviarNibble(0x20, 0);
	
	LCD_Comando(0x28);
	LCD_Comando(0x0C);
	LCD_Comando(0x06);
	LCD_Comando(0x01);
	_delay_ms(2);
}

void LCD_Limpiar(void) {
	LCD_Comando(0x01);
	_delay_ms(2);
}

void LCD_FijarCursor(uint8_t col, uint8_t fila) {
	uint8_t addr = (fila == 0) ? 0x80 : 0xC0;
	LCD_Comando(addr + col);
}

void LCD_Imprimir(const char* cadena) {
	while (*cadena) {
		LCD_Dato(*cadena++);
	}
}

void LCD_ImprimirHex(uint8_t valor) {
	char hex[] = "0123456789ABCDEF";
	LCD_Dato(hex[valor >> 4]);
	LCD_Dato(hex[valor & 0x0F]);
}



// funciones de la EEPROM
void EEPROM_GuardarTarjeta(volatile uint8_t *id_tarjeta) {
	eeprom_update_byte((uint8_t*)EEPROM_DIR_TARJETA_VALIDA, 0xAA);
	for (uint8_t i = 0; i < 4; i++) {
		eeprom_update_byte((uint8_t*)(EEPROM_DIR_TARJETA_ID + i), id_tarjeta[i]);
	}
}

uint8_t EEPROM_CargarTarjeta(volatile uint8_t *id_tarjeta) {
	uint8_t valida = eeprom_read_byte((uint8_t*)EEPROM_DIR_TARJETA_VALIDA);
	if (valida == 0xAA) {
		for (uint8_t i = 0; i < 4; i++) {
			id_tarjeta[i] = eeprom_read_byte((uint8_t*)(EEPROM_DIR_TARJETA_ID + i));
		}
		return 1;
	}
	return 0;
}

void EEPROM_BorrarTarjeta(void) {
	eeprom_update_byte((uint8_t*)EEPROM_DIR_TARJETA_VALIDA, 0xFF);
}

// funciones LEDs
void LED_Inicializar(void) {
	LED_VERDE_DDR |= (1 << LED_VERDE_PIN);
	LED_ROJO_DDR |= (1 << LED_ROJO_PIN);
	LED_VERDE_PORT &= ~(1 << LED_VERDE_PIN);
	LED_ROJO_PORT &= ~(1 << LED_ROJO_PIN);
}

void LED_Verde_Encender(void) {
	LED_VERDE_PORT |= (1 << LED_VERDE_PIN);
}

void LED_Verde_Apagar(void) {
	LED_VERDE_PORT &= ~(1 << LED_VERDE_PIN);
}

void LED_Rojo_Encender(void) {
	LED_ROJO_PORT |= (1 << LED_ROJO_PIN);
}

void LED_Rojo_Apagar(void) {
	LED_ROJO_PORT &= ~(1 << LED_ROJO_PIN);
}

void LED_TodosApagar(void) {
	LED_Verde_Apagar();
	LED_Rojo_Apagar();
}

//funciones botones
void Botones_Inicializar(void) {
	
	BOTON_BORRAR_DDR &= ~(1 << BOTON_BORRAR_BIT);
	BOTON_REGISTRAR_DDR &= ~(1 << BOTON_REGISTRAR_BIT);
	
	BOTON_BORRAR_PORT |= (1 << BOTON_BORRAR_BIT);
	BOTON_REGISTRAR_PORT |= (1 << BOTON_REGISTRAR_BIT);
}

uint8_t Boton_Borrar_Presionado(void) {
	return !(BOTON_BORRAR_PIN & (1 << BOTON_BORRAR_BIT));
}

uint8_t Boton_Registrar_Presionado(void) {
	return !(BOTON_REGISTRAR_PIN & (1 << BOTON_REGISTRAR_BIT));
}

// Debounce
void EsperarSoltarBoton(void) {
	_delay_ms(50);
	while (Boton_Borrar_Presionado() || Boton_Registrar_Presionado()) {
		_delay_ms(10);
	}
	_delay_ms(50);
}


uint8_t CompararIDTarjeta(volatile uint8_t *id_a, uint8_t *id_b) {
	uint8_t a_vacio = 1;
	uint8_t b_vacio = 1;
	
	for (uint8_t i = 0; i < 4; i++) {
		if (id_a[i] != 0x00 && id_a[i] != 0xFF) a_vacio = 0;
		if (id_b[i] != 0x00 && id_b[i] != 0xFF) b_vacio = 0;
	}
	
	if (a_vacio || b_vacio) return 0;
	
	for (uint8_t i = 0; i < 4; i++) {
		if (id_a[i] != id_b[i]) return 0;
	}
	return 1;
}

void AccesoPermitido(void) {
	LED_TodosApagar();
	LED_Verde_Encender();
	
	LCD_Limpiar();
	LCD_FijarCursor(0, 0);
	LCD_Imprimir("ACCESO PERMITIDO");
	
	uart_print("ACCESO PERMITIDO\r\n");
	
	_delay_ms(3000);
	
	LED_Verde_Apagar();
}

void AccesoDenegado(void) {
	LED_TodosApagar();
	LED_Rojo_Encender();
	
	LCD_Limpiar();
	LCD_FijarCursor(0, 0);
	LCD_Imprimir("ACCESO DENEGADO");
	LCD_FijarCursor(0, 1);
	LCD_Imprimir("Tarjeta inválida");
	
	uart_print("ACCESO DENEGADO\r\n");
	
	for (uint8_t i = 0; i < 5; i++) {
		LED_Rojo_Encender();
		_delay_ms(200);
		LED_Rojo_Apagar();
		_delay_ms(200);
	}
}

void MostrarMensajeBienvenida(void) {
	LCD_Limpiar();
	LCD_FijarCursor(0, 0);
	LCD_Imprimir("SISTEMA RFID");
	LCD_FijarCursor(0, 1);
	LCD_Imprimir("Inicializando");
	_delay_ms(2000);
}

void MostrarMensajeListo(void) {
	LCD_Limpiar();
	LCD_FijarCursor(0, 0);
	if (hay_tarjeta_registrada) {
		LCD_Imprimir("SISTEMA LISTO");
		} else {
		LCD_Imprimir("SIN TARJETA");
	}
	LCD_FijarCursor(0, 1);
	LCD_Imprimir("Acerque tarjeta");
}

// MAIN
int main(void) {
	uint8_t num_serie[5];
	
	uart_init(103);
	I2C_Inicializar();
	LCD_Inicializar();
	spi_init();
	LED_Inicializar();
	Botones_Inicializar();
	
	
	// Inicializar RC522
	_delay_ms(100);
	mfrc522_resetPinInit();
	mfrc522_init();
	
	
	MostrarMensajeBienvenida();
	uart_print("\r\nBienvenido al sistema RFID\r\n");
	
	hay_tarjeta_registrada = EEPROM_CargarTarjeta(tarjeta_registrada_id);
	
	if (hay_tarjeta_registrada) {
		uart_print("Tarjeta registrada encontrada en EEPROM\r\n");
		} else {
		uart_print("No hay tarjeta registrada\r\n");
	}
	
	MostrarMensajeListo();
	uart_print("\r\nSistema listo. Esperando tarjetas\r\n\r\n");
	
	while (1) {
		
		memset(num_serie, 0, 5);
		mfrc522_standard(num_serie);
		
		// Si el primer byte no es 0, se detecto una tarjeta
		if (num_serie[0] != 0) {
			
			
			LCD_Limpiar();
			LCD_FijarCursor(0, 0);
			LCD_Imprimir("Tarjeta leída:");
			LCD_FijarCursor(0, 1);
			for (uint8_t i = 0; i < 4; i++) {
				LCD_ImprimirHex(num_serie[i]);
				if (i < 3) LCD_Imprimir(" ");
			}
			
			uart_print("\r\nTARJETA DETECTADA\r\n");
			uart_print("ID: ");
			for (uint8_t i = 0; i < 4; i++) {
				uart_print_hex(num_serie[i]);
				uart_print(" ");
			}
			uart_print("\r\n");
			
			_delay_ms(800);
			
			// Verificacion de acceso
			uart_print("\r\nVerificando acceso\r\n");
			
			if (hay_tarjeta_registrada == 0) {
				LCD_Limpiar();
				LCD_FijarCursor(0, 0);
				LCD_Imprimir("SIN TARJETA REG");
				LCD_FijarCursor(0, 1);
				LCD_Imprimir("Registre primero");
				
				uart_print("\r\nACCESO DENEGADO: NO HAY TARJETA REGISTRADA\r\n");
				
				LED_Rojo_Encender();
				_delay_ms(2500);
				LED_Rojo_Apagar();
				
				MostrarMensajeListo();
				_delay_ms(500);
				continue;
			}
			
			if (CompararIDTarjeta(tarjeta_registrada_id, num_serie)) {
				uart_print("ID correcta\r\n");
				AccesoPermitido();
				} else {
				uart_print("ID incorrecta\r\n");
				AccesoDenegado();
			}
			
			MostrarMensajeListo();
			_delay_ms(500);
		}
		
		_delay_ms(100);
	}
	
	return 0;
}