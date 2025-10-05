#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

// pines
#define TRIG_PORT  PORTB
#define TRIG_DDR   DDRB
#define TRIG_PIN   PB0      // D8

#define ECHO_PINR  PIND
#define ECHO_DDR   DDRD
#define ECHO_PIN   PD7      // D7

#define PWM_DDR    DDRD
#define PWM_PIN    PD6      // D6

// config autocal
#define muestras_ini   16   // cuantas junto al inicio
#define margen_ticks    1   // para no clavar en borde
#define recalc_ciclos 400   // cada tanto, recalibro

// 10 pasos de brillo
static const uint8_t tabla_pwm[10] = {13,38,64,89,115,140,166,191,217,242};

// init pwm 1 kHz en D6
static void pwm_ini(void){
	PWM_DDR |= (1<<PWM_PIN);
	TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0A1);  // fast pwm
	TCCR0B = (1<<CS01)|(1<<CS00);                // /64
	OCR0A = tabla_pwm[0];
}
static inline void pwm_set(uint8_t d){ OCR0A = d; }

//cuento tiempo (0.5us por tick)
static void t1_ini(void){
	TCCR1A = 0;
	TCCR1B = (1<<CS11);   // /8
	TCNT1 = 0;
}

// espero estado de pin con timeout
static bool esperar(volatile uint8_t *pin, uint8_t mask, uint8_t estado, uint16_t to){
	uint16_t t0 = TCNT1;
	while((((*pin)&mask)?1:0) != estado){
		if((uint16_t)(TCNT1 - t0) > to) return false;
	}
	return true;
}

// disparo y mido ECHO
static uint16_t leer_ticks(void){
	TRIG_PORT &= ~(1<<TRIG_PIN);   // bajo un rato
	_delay_us(200);

	TRIG_PORT |=  (1<<TRIG_PIN);   // pulso 10us
	_delay_us(10);
	TRIG_PORT &= ~(1<<TRIG_PIN);

	if(!esperar(&ECHO_PINR, (1<<ECHO_PIN), 1, 20000)) return 0xFFFF; // subida
	TCNT1 = 0;
	if(!esperar(&ECHO_PINR, (1<<ECHO_PIN), 0, 60000)) return 0xFFFF; // bajada

	return TCNT1;
}

// mapeo por autocal, ticks nivel del 0 al 9
static uint8_t nivel_auto(uint16_t ticks, uint16_t min_visto, uint16_t max_visto){
	if(ticks == 0xFFFF) return 0;                 // sin dato
	if(max_visto <= min_visto + 1) return 0;      // sin rango

	if(ticks < min_visto) ticks = min_visto;      // recorte
	if(ticks > max_visto) ticks = max_visto;

	uint32_t span = (uint32_t)(max_visto - min_visto);
	uint32_t pos  = (uint32_t)(ticks - min_visto);

	uint8_t base = (uint8_t)((pos * 10u) / (span + 1u));  // de 0 a 9, lejos
	if(base > 9) base = 9;
	return (uint8_t)(9u - base);                           // invertido
}

int main(void){
	// direcciones
	TRIG_DDR |=  (1<<TRIG_PIN);   // trig salida
	ECHO_DDR &= ~(1<<ECHO_PIN);   // echo entrada

	pwm_ini();
	t1_ini();

	// estado autocal
	uint16_t min_visto = 0xFFFF;
	uint16_t max_visto = 0;
	uint16_t ciclos = 0;
	uint16_t tomadas = 0;

	uint8_t duty_ult = tabla_pwm[0];

	while(1){
		// promedio 4 lecturas, >=60ms entre disparos
		uint8_t buenas = 0;
		uint32_t acum = 0;
		for(uint8_t i=0;i<4;i++){
			uint16_t t = leer_ticks();
			if(t != 0xFFFF){ acum += t; buenas++; }
			_delay_ms(15);
		}

		if(buenas){
			uint16_t ticks = (uint16_t)(acum / buenas);

			// arranque y recalc
			if(tomadas < muestras_ini || ciclos >= recalc_ciclos){
				if(ticks < min_visto) min_visto = ticks;
				if(ticks > max_visto) max_visto = ticks;
				tomadas++;
				if(ciclos >= recalc_ciclos){
					ciclos = 0;
					tomadas = 0;
					min_visto = 0xFFFF;
					max_visto = 0;
				}
				} else {
				// abro el rango si aparecen nuevos extremos
				if(ticks + margen_ticks < min_visto) min_visto = ticks;
				if(ticks > max_visto + margen_ticks) max_visto = ticks;
			}
			if(max_visto <= min_visto) max_visto = min_visto + 1;

			// saco nivel y aplico pwm
			uint8_t nivel = nivel_auto(ticks, min_visto, max_visto);
			duty_ult = tabla_pwm[nivel];
			pwm_set(duty_ult);
			} else {
			// sin medicion, dejo lo ultimo
			pwm_set(duty_ult);
		}

		ciclos++;
		_delay_ms(10);
	}
}
