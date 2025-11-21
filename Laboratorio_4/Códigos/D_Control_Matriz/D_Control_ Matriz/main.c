#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// Pines
#define PIN_MATRIZ PB0
#define PUERTO_MATRIZ   PORTB
#define DDR_MATRIZ    DDRB

#define PIN_BOTON    PD2
#define PUERTO_BOTON    PORTD
#define DDR_BOTON     DDRD
#define PINR_BOTON    PIND

// Matriz
#define ANCHO_MATRIZ  8
#define ALTO_MATRIZ 8
#define NUM_LEDS      (ANCHO_MATRIZ * ALTO_MATRIZ)

// Valores para cada direccion

#define AX_ARRIBA_MIN 8150
#define AX_ARRIBA_MAX 8500
#define AY_ARRIBA_MIN -1000
#define AY_ARRIBA_MAX 1000

#define AX_ABAJO_MIN -8150
#define AX_ABAJO_MAX -8500
#define AY_ABAJO_MIN -1000
#define AY_ABAJO_MAX 1000

#define AX_IZQ_MIN 1500
#define AX_IZQ_MAX 2000
#define AY_IZQ_MIN 50
#define AY_IZQ_MAX 200

#define AX_DER_MIN 1500
#define AX_DER_MAX 2000
#define AY_DER_MIN -200
#define AY_DER_MAX -50


// MPU6050
#define DIRECCION_MPU6050       0x68
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B

typedef struct {
	uint8_t g;  // La matriz usa grb
	uint8_t r;
	uint8_t b;
} Color_GRB;

// Colores
const Color_GRB colores[] = {
	{0, 255, 0},     // Rojo
	{255, 0, 0},     // Verde
	{0, 0, 255},     // Azul
	{255, 255, 0},     // Amarillo
	{255, 0, 255},     // Celeste
	{128, 255, 0},     // Naranja
};
#define NUM_COLORES 8

// Variables
volatile int8_t pos_x = 3;    // Posicion inicial cerca del centro
volatile int8_t pos_y = 3;
volatile uint8_t indice_color = 0;
uint8_t buffer_matriz[NUM_LEDS * 3];

void inicializar_sistema(void);
void inicializar_i2c(void);
void i2c_inicio(void);
void i2c_parar(void);
uint8_t i2c_escribir(uint8_t dato);
uint8_t i2c_leer_ack(void);
uint8_t i2c_leer_nack(void);
void inicializar_mpu6050(void);
uint8_t mpu6050_leer_aceleracion(int16_t *ax, int16_t *ay, int16_t *az);
void matriz_enviar_byte(uint8_t byte);
void actualizar_matriz(void);
void limpiar_matriz(void);
void establecer_pixel_matriz(uint8_t x, uint8_t y, Color_GRB color);
uint16_t xy_a_indice(uint8_t x, uint8_t y);
void actualizar_posicion_led(int16_t ax, int16_t ay);
void leer_boton(void);

// Main
int main(void) {
	inicializar_sistema();
	inicializar_i2c();
	_delay_ms(100);
	inicializar_mpu6050();
	_delay_ms(100);
	
	int16_t ax, ay, az;
	uint8_t contador_actualizacion = 0;
	
	// Muestra el LED inicial
	limpiar_matriz();
	establecer_pixel_matriz(pos_x, pos_y, colores[indice_color]);
	actualizar_matriz();
	
	while (1) {
		// Lee el acelerometro
		if (mpu6050_leer_aceleracion(&ax, &ay, &az)) {
			
			// actualiza la posicion
			contador_actualizacion++;
			if (contador_actualizacion >= 2) {
				contador_actualizacion = 0;
				actualizar_posicion_led(ax, ay);
			}
		}
		
		
		leer_boton();
		
		// Actualiza la matriz
		limpiar_matriz();
		establecer_pixel_matriz(pos_x, pos_y, colores[indice_color]);
		actualizar_matriz();
		
		_delay_ms(30);
	}
	
	return 0;
}

// Inicializacion
void inicializar_sistema(void) {
	// Pin de la matriz como salida
	DDR_MATRIZ |= (1 << PIN_MATRIZ);
	PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
	
	// Boton como entrada
	DDR_BOTON &= ~(1 << PIN_BOTON);
	PUERTO_BOTON |= (1 << PIN_BOTON);
	
	// Limpia el buffer
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		buffer_matriz[i] = 0;
	}
}

// Funciones I2C
void inicializar_i2c(void) {
	TWSR = 0x00;
	TWBR = 72;
	TWCR = (1 << TWEN);
}

void i2c_inicio(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void i2c_parar(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	_delay_us(100);
}

uint8_t i2c_escribir(uint8_t dato) {
	TWDR = dato;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return (TWSR & 0xF8);
}

uint8_t i2c_leer_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

uint8_t i2c_leer_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// Funciones MPU6050
void inicializar_mpu6050(void) {
	i2c_inicio();
	i2c_escribir((DIRECCION_MPU6050 << 1) | 0);
	i2c_escribir(MPU6050_PWR_MGMT_1);
	i2c_escribir(0x00);
	i2c_parar();
	_delay_ms(100);
	
	// Se configura el rango del acelerometro
	i2c_inicio();
	i2c_escribir((DIRECCION_MPU6050 << 1) | 0);
	i2c_escribir(MPU6050_ACCEL_CONFIG);
	i2c_escribir(0x08);
	i2c_parar();
	_delay_ms(10);
}

uint8_t mpu6050_leer_aceleracion(int16_t *ax, int16_t *ay, int16_t *az) {
	uint8_t datos[6];
	
	i2c_inicio();
	if (i2c_escribir((DIRECCION_MPU6050 << 1) | 0) != 0x18) {
		i2c_parar();
		return 0;
	}
	if (i2c_escribir(MPU6050_ACCEL_XOUT_H) != 0x28) {
		i2c_parar();
		return 0;
	}
	
	i2c_inicio();
	i2c_escribir((DIRECCION_MPU6050 << 1) | 1);
	
	// Leer 6 bytes (X, Y, Z high y low)
	datos[0] = i2c_leer_ack();
	datos[1] = i2c_leer_ack();
	datos[2] = i2c_leer_ack();
	datos[3] = i2c_leer_ack();
	datos[4] = i2c_leer_ack();
	datos[5] = i2c_leer_nack();
	i2c_parar();
	
	*ax = (int16_t)((datos[0] << 8) | datos[1]);
	*ay = (int16_t)((datos[2] << 8) | datos[3]);
	*az = (int16_t)((datos[4] << 8) | datos[5]);
	
	return 1;
}

// Funciones matriz
void matriz_enviar_byte(uint8_t byte) {
	cli();  // deshabilita las interrupciones
	
	for (uint8_t bit = 0; bit < 8; bit++) {
		if (byte & 0x80) {
			PUERTO_MATRIZ |= (1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t"
			::
			);
			PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t"
			::
			);
			} else {
			PUERTO_MATRIZ |= (1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t"
			::
			);
			PUERTO_MATRIZ &= ~(1 << PIN_MATRIZ);
			__asm__ __volatile__(
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t"
			::
			);
		}
		byte <<= 1;
	}
	
	sei();  // Rehabilita las interrupciones
}

void actualizar_matriz(void) {
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		matriz_enviar_byte(buffer_matriz[i]);
	}
	
	_delay_us(60);
}

void limpiar_matriz(void) {
	for (uint16_t i = 0; i < NUM_LEDS * 3; i++) {
		buffer_matriz[i] = 0;
	}
}

void establecer_pixel_matriz(uint8_t x, uint8_t y, Color_GRB color) {
	if (x >= ANCHO_MATRIZ || y >= ALTO_MATRIZ) return;
	
	uint16_t indice = xy_a_indice(x, y);
	uint16_t posicion = indice * 3;
	
	buffer_matriz[posicion]     = color.g;
	buffer_matriz[posicion + 1] = color.r;
	buffer_matriz[posicion + 2] = color.b;
}

uint16_t xy_a_indice(uint8_t x, uint8_t y) {

	uint16_t indice = y * ANCHO_MATRIZ + x;
	
	return indice;
}

// Control
void actualizar_posicion_led(int16_t ax, int16_t ay) {
	#define UMBRAL_MOVIMIENTO 1200
	
	int8_t nueva_x = pos_x;
	int8_t nueva_y = pos_y;
	
	
	int16_t eje_x = ay;  // izquierda y derecha
	int16_t eje_y = -ax;  // arriba y abajo
	
	
	
	// Movimiento en X
	if (eje_x > UMBRAL_MOVIMIENTO) {
		nueva_x++;
		} else if (eje_x < -UMBRAL_MOVIMIENTO) {
		nueva_x--;
	}
	
	// Movimiento en Y
	if (eje_y > UMBRAL_MOVIMIENTO) {
		nueva_y++;
		} else if (eje_y < -UMBRAL_MOVIMIENTO) {
		nueva_y--;
	}
	
	// Limites de la matriz
	if (nueva_x < 0) nueva_x = 0;
	if (nueva_x >= ANCHO_MATRIZ) nueva_x = ANCHO_MATRIZ - 1;
	if (nueva_y < 0) nueva_y = 0;
	if (nueva_y >= ALTO_MATRIZ) nueva_y = ALTO_MATRIZ - 1;
	
	pos_x = nueva_x;
	pos_y = nueva_y;
}

void leer_boton(void) {
	static uint8_t ultimo_estado = 1;
	static uint8_t boton_presionado = 0;
	
	uint8_t estado_actual = (PINR_BOTON & (1 << PIN_BOTON)) ? 1 : 0;
	
	if (ultimo_estado == 1 && estado_actual == 0 && !boton_presionado) {
		_delay_ms(50);  // Debounce
		
		// Verifica si sigue apretado
		if (!(PINR_BOTON & (1 << PIN_BOTON))) {
			// Cambia de color
			indice_color++;
			if (indice_color >= NUM_COLORES) {
				indice_color = 0;
			}
			boton_presionado = 1;
		}
	}
	
	// Detecta cuando se deja de apretar el boton
	if (estado_actual == 1) {
		boton_presionado = 0;
	}
	
	ultimo_estado = estado_actual;
}