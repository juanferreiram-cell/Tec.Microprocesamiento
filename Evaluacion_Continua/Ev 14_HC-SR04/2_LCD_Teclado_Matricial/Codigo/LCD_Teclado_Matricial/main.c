#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// Definici?n de pines LCD en PORTD
#define RS PD0
#define EN PD1
#define D4 PD4
#define D5 PD5
#define D6 PD6
#define D7 PD7

// Funci?n que permite inicializar la pantalla (configura pines del LCD como salidas)
void iniciar_lcd_pins() {
	DDRD |= (1 << RS) | (1 << EN) | (1 << D4) |
	(1 << D5) | (1 << D6) | (1 << D7);
}

// Enviar nibble a la LCD
void lcd_send_nibble(uint8_t nibble) {
	if (nibble & 0x01) PORTD |= (1 << D4);
	else PORTD &= ~(1 << D4);
	if (nibble & 0x02) PORTD |= (1 << D5);
	else PORTD &= ~(1 << D5);
	if (nibble & 0x04) PORTD |= (1 << D6);
	else PORTD &= ~(1 << D6);
	if (nibble & 0x08) PORTD |= (1 << D7);
	else PORTD &= ~(1 << D7);

	// Pulso en Enable
	PORTD |= (1 << EN);
	_delay_us(1);
	PORTD &= ~(1 << EN);
	_delay_us(100);
}

// Funci?n que env?a un comando de 8 bits al LCD
void lcd_send_command(uint8_t cmd) {
	PORTD &= ~(1 << RS);        // RS=0 ¨ modo comando
	lcd_send_nibble(cmd >> 4);  // mitad alta
	lcd_send_nibble(cmd & 0x0F);// mitad baja
}

// Funci?n que env?a un dato (car?cter) de 8 bits al LCD
void lcd_send_data(uint8_t data) {
	PORTD |= (1 << RS);         // RS=1 ¨ modo datos
	lcd_send_nibble(data >> 4); // mitad alta
	lcd_send_nibble(data & 0x0F);// mitad baja
}

// Funci?n que inicializa el LCD en modo 4 bits y configura display/entry mode
void iniciar_lcd() {
	_delay_ms(50);              // espera tras alimentaci?n
	PORTD &= ~(1 << RS);        // RS=0 ¨ comandos

	// Secuencia est?ndar para pasar a 4 bits
	lcd_send_nibble(0x03); _delay_ms(5);
	lcd_send_nibble(0x03); _delay_us(100);
	lcd_send_nibble(0x03); _delay_us(100);
	lcd_send_nibble(0x02);      // ahora en 4 bits

	// Configuraci?n del LCD
	lcd_send_command(0x28);     // Function set: 4 bits, 2 l?neas, font 5x8
	lcd_send_command(0x0C);     // Display ON, cursor OFF, blink OFF
	lcd_send_command(0x06);     // Entry mode: cursor incrementa a la derecha
	lcd_send_command(0x01);     // Clear display
	_delay_ms(2);               // tiempo de clear/home
}

// Matriz de caracteres para mapear filas/columnas del keypad
char keypad[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

// Funci?n que inicializa el keypad (filas PC0..PC3 salidas, columnas PB0..PB3 entradas con pull-up)
static void iniciar_keypad(void) {
	DDRC  |= 0x0F;  // filas como salida
	PORTC |= 0x0F;  // filas inicialmente en alto

	DDRB  &= ~0x0F; // columnas como entrada
	PORTB |= 0x0F;  // pull-ups habilitados en columnas
}

// Funci?n que lee una tecla del keypad (escaneo por filas, retorno del car?cter o 0 si no hay tecla)
char Leer_keypad(void) {
	for (uint8_t row = 0; row < 4; row++) {
		PORTC = ~(1 << row);    // activar 1 fila en 0 (resto en 1)
		_delay_us(5);           // peque?o tiempo de asentamiento

		for (uint8_t col = 0; col < 4; col++) {
			if (!(PINB & (1 << col))) { // activa en bajo (por pull-up)
				_delay_ms(15);          // antirrebote simple
				while (!(PINB & (1 << col))) {} // esperar liberaci?n
				return keypad[row][col]; // devolver car?cter mapeado
			}
		}
	}
	return 0; // sin tecla detectada
}

// Funci?n que espera una tecla v?lida y la escribe en el LCD
static void escribir(void){
	char x = 0;
	do {
		x = Leer_keypad();      // bloqueo hasta que haya tecla
	} while (x == 0);

	lcd_send_data(x);           // mostrar la tecla en la posici?n actual
}


// Funci?n principal: inicializa keypad y LCD y muestra continuamente las teclas presionadas
int main(void)
{
	iniciar_keypad();                           // configurar filas/columnas
	iniciar_lcd_pins();                            // configurar pines del LCD
	PORTD &= ~((1<<RS)|(1<<EN)|(1<<D4)|(1<<D5)|(1<<D6)|(1<<D7)); // limpiar l?neas
	iniciar_lcd();                                 // inicializar controlador LCD

	while(1) {
		escribir();                             // leer tecla y escribirla
	}
}
