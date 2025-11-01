#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include <util/delay.h>

// Pines
#define PASO_X     PB3
#define SENTIDO_X  PB4
#define HABIL_X    PB5

#define PASO_Y     PC3
#define SENTIDO_Y  PC4
#define HABIL_Y    PC5

#define SOLENOIDE  PC0

// L?mites (PD2=A, PD3=D)
#define LIMIT_YA   PD2   // INT0
#define LIMIT_YD   PD3   // INT1

// Delays
#define T_SETUP_DIR   50    // tiempo entre configurar DIR y el primer pulso
#define T_PULSO_ALTO  300   // ~1.2 ms en alto
#define T_PULSO_BAJO  300   // ~1.2 ms en bajo

// Inicializacion
#define ciclo_delay(n)           _delay_loop_2((n))
#define poner_bit(REG,BIT)      (*(REG) |=  (1U<<(BIT)))
#define limpiar_bit(REG,BIT)    (*(REG) &= ~(1U<<(BIT)))

typedef struct {
	volatile uint8_t *port_dir;
	uint8_t           bit_dir;
	volatile uint8_t *port_step;
	uint8_t           bit_step;
} eje_t;

static const eje_t EJE_X = { &PORTB, SENTIDO_X, &PORTB, PASO_X };
static const eje_t EJE_Y = { &PORTC, SENTIDO_Y, &PORTC, PASO_Y };

// Estado de aborto por l?mites
static volatile uint8_t abort_flag = 0;

static inline void pluma_arriba(void);

static inline void abort_if_needed(void){
	if (!abort_flag) return;
	// Acciones seguras y detener
	pluma_arriba();
	limpiar_bit(&PORTB, HABIL_X);
	limpiar_bit(&PORTC, HABIL_Y);
	cli();
	while(1){} // detener programa
}

// Interrupciones de l?mite
ISR(INT0_vect){ abort_flag = 1; } // L?mite A (PD2)
ISR(INT1_vect){ abort_flag = 1; } // L?mite D (PD3)

static inline void sistema_init(void){
	// Configurar como salida
	DDRB |= (1<<PASO_X) | (1<<SENTIDO_X) | (1<<HABIL_X);
	DDRC |= (1<<PASO_Y) | (1<<SENTIDO_Y) | (1<<HABIL_Y) | (1<<SOLENOIDE);

	// Habilitar drivers (nivel alto)
	poner_bit(&PORTB, HABIL_X);
	poner_bit(&PORTC, HABIL_Y);

	// Pluma arriba por defecto
	poner_bit(&PORTC, SOLENOIDE);

	// Entradas de l?mite con pull-up
	DDRD  &= ~((1<<LIMIT_YA) | (1<<LIMIT_YD)); // entradas
	PORTD |=  (1<<LIMIT_YA) | (1<<LIMIT_YD);   // pull-ups

	// INT0/INT1 en flanco descendente (sensores activos en 0)
	EICRA = (1<<ISC01)|(0<<ISC00)   // INT0: falling edge
	| (1<<ISC11)|(0<<ISC10);  // INT1: falling edge
	EIFR  = (1<<INTF0)|(1<<INTF1);  // limpia flags pendientes
	EIMSK = (1<<INT0)|(1<<INT1);    // habilita interrupciones
	sei();
}

// Control de Motores paso a paso
static inline void run_steps(const eje_t *e, uint8_t sentido, uint16_t cantidad){
	abort_if_needed();

	if (sentido) poner_bit(e->port_dir, e->bit_dir);
	else         limpiar_bit(e->port_dir, e->bit_dir);

	ciclo_delay(T_SETUP_DIR);

	while (cantidad--){
		abort_if_needed();

		poner_bit(e->port_step, e->bit_step);
		ciclo_delay(T_PULSO_ALTO);
		limpiar_bit(e->port_step, e->bit_step);
		ciclo_delay(T_PULSO_BAJO);
	}
}

// Movimiento de los ejes
void eje_x(uint8_t dir, uint16_t pasos){
	abort_if_needed();
	run_steps(&EJE_X, dir, pasos);
}

void eje_y(uint8_t dir, uint16_t pasos){
	abort_if_needed();
	run_steps(&EJE_Y, dir, pasos);
}

// Movimiento combinado X+Y para diagonales

// Modo para diagonales 45?
enum {
	D45_ARRIBA_IZQ  = 0,
	D45_ARRIBA_DER  = 1,
	D45_ABAJO_IZQ   = 2,
	D45_ABAJO_DER   = 3
};

// Diagonal 45?
void d45(uint8_t modo, uint16_t pasos){
	abort_if_needed();

	uint8_t dirx = 0, diry = 0;

	switch(modo){
		case D45_ARRIBA_IZQ:  diry = 0; dirx = 0; break;
		case D45_ARRIBA_DER:  diry = 0; dirx = 1; break;
		case D45_ABAJO_IZQ:   diry = 1; dirx = 0; break;
		default:              diry = 1; dirx = 1; break; // D45_ABAJO_DER
	}

	// fijar sentidos
	if (dirx) poner_bit(EJE_X.port_dir, EJE_X.bit_dir);
	else      limpiar_bit(EJE_X.port_dir, EJE_X.bit_dir);

	if (diry) poner_bit(EJE_Y.port_dir, EJE_Y.bit_dir);
	else      limpiar_bit(EJE_Y.port_dir, EJE_Y.bit_dir);

	ciclo_delay(T_SETUP_DIR);

	// pulsos simult?neos
	while (pasos--){
		abort_if_needed();

		poner_bit(EJE_X.port_step, EJE_X.bit_step);
		poner_bit(EJE_Y.port_step, EJE_Y.bit_step);
		ciclo_delay(T_PULSO_ALTO);
		limpiar_bit(EJE_X.port_step, EJE_X.bit_step);
		limpiar_bit(EJE_Y.port_step, EJE_Y.bit_step);
		ciclo_delay(T_PULSO_BAJO);
	}
}

// Movimiento XY con cualquier pendiente
void mover_xy(uint8_t dirx, uint8_t diry, uint16_t pasos_x, uint16_t pasos_y){
	abort_if_needed();

	if (dirx) poner_bit(EJE_X.port_dir, EJE_X.bit_dir);
	else      limpiar_bit(EJE_X.port_dir, EJE_X.bit_dir);

	if (diry) poner_bit(EJE_Y.port_dir, EJE_Y.bit_dir);
	else      limpiar_bit(EJE_Y.port_dir, EJE_Y.bit_dir);

	ciclo_delay(T_SETUP_DIR);

	uint16_t maxp = (pasos_x > pasos_y) ? pasos_x : pasos_y;
	uint32_t accx = 0, accy = 0;

	for(uint16_t i=0; i<maxp; ++i){
		abort_if_needed();

		uint8_t px = 0, py = 0;

		accx += pasos_x; if (accx >= maxp){ accx -= maxp; px = 1; }
		accy += pasos_y; if (accy >= maxp){ accy -= maxp; py = 1; }

		if (px) poner_bit(EJE_X.port_step, EJE_X.bit_step);
		if (py) poner_bit(EJE_Y.port_step, EJE_Y.bit_step);
		ciclo_delay(T_PULSO_ALTO);
		if (px) limpiar_bit(EJE_X.port_step, EJE_X.bit_step);
		if (py) limpiar_bit(EJE_Y.port_step, EJE_Y.bit_step);
		ciclo_delay(T_PULSO_BAJO);
	}
}

// Movimiento de selenoide
static inline void pluma_abajo(void){
	abort_if_needed();
	limpiar_bit(&PORTC, SOLENOIDE);
	_delay_ms(100);
}

static inline void pluma_arriba(void){
	poner_bit(&PORTC, SOLENOIDE);
	_delay_ms(100);
	abort_if_needed();
}


