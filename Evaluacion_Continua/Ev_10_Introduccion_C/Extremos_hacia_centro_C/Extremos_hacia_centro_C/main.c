#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* ---------- ENTRADAS ---------- */
#define ENTRADA_LENTO    PB0    // mantener para LENTO 
#define ENTRADA_RAPIDO   PB1    // mantener para R?PIDO 

/* ---------- PAR?METROS DE TIEMPO AJUSTABLES ---------- */
#define RETARDO_INICIAL_MS        500UL   // retardo inicial (persiste)
#define RETARDO_MIN_MS              1UL    // piso pr?ctico
#define RETARDO_PASO_MS            20UL    // cambio por gtickh de autorepetici?n
#define RETARDO_REPETICION_MS     120UL    // cada cu?ntos ms aplica un PASO
#define RETARDO_TROZO_MS            5U     // granularidad de chequeo

// Retardo en ms (usa F_CPU = 8 MHz)
static void esperar_ms(uint16_t ms) {
    while (ms--) _delay_ms(1);
}

/* Espera 'ms_total' y permite ajustar *p_retardo_ms mientras espera */
static void esperar_y_ajustar(volatile uint32_t *p_retardo_ms, uint32_t ms_total) {
    static uint16_t acumulado_ms = 0;

    while (ms_total > 0) {
        uint16_t trozo = (ms_total > RETARDO_TROZO_MS) ? RETARDO_TROZO_MS : (uint16_t)ms_total;
        esperar_ms(trozo);
        ms_total -= trozo;

        uint8_t pinb   = PINB;
        uint8_t rapido = !(pinb & (1 << ENTRADA_RAPIDO)); // activos en bajo
        uint8_t lento  = !(pinb & (1 << ENTRADA_LENTO));
        uint8_t uno    = (rapido ^ lento);                // exactamente uno presionado

        if (uno) {
            acumulado_ms += trozo;
            if (acumulado_ms >= RETARDO_REPETICION_MS) {
                acumulado_ms = 0;

                if (rapido) {
                    // R?PIDO: bajar retardo con piso
                    if (*p_retardo_ms > RETARDO_MIN_MS + RETARDO_PASO_MS)
                        *p_retardo_ms -= RETARDO_PASO_MS;
                    else
                        *p_retardo_ms = RETARDO_MIN_MS;
                } else { // lento
                    // LENTO: subir retardo (sin techo)
                    *p_retardo_ms += RETARDO_PASO_MS;
                }
            }
        } else {
            acumulado_ms = 0; // solt?, ambos o ninguno: se mantiene
        }
    }
}

/* --------- SECUENCIA (8 pasos) ---------
   1) PD0|PD1
   2) +PD6|PD7
   3) +PD2|PD5
   4) +PD3|PD4 (todos)
   5) quitar PD3|PD4
   6) solo PD6|PD7 + extremos
   7) solo extremos (PD0|PD1)
   8) todos apagados
----------------------------------------- */
static const uint8_t patrones[] = {
    (1<<PD0) | (1<<PD1),                                              // Paso 1: encender extremos PD0 y PD1
    (1<<PD0) | (1<<PD1) | (1<<PD6) | (1<<PD7),                        // Paso 2: agregar PD6 y PD7
    (1<<PD0) | (1<<PD1) | (1<<PD2) | (1<<PD5) | (1<<PD6) | (1<<PD7),  // Paso 3: agregar PD2 y PD5
    0xFF,                                                             // Paso 4: encender todos (agregar PD3 y PD4)
    (1<<PD0) | (1<<PD1) | (1<<PD2) | (1<<PD5) | (1<<PD6) | (1<<PD7),  // Paso 5: apagar PD3 y PD4
    (1<<PD0) | (1<<PD1) | (1<<PD6) | (1<<PD7),                        // Paso 6: solo PD6 y PD7 + extremos PD0 y PD1
    (1<<PD0) | (1<<PD1),                                              // Paso 7: volver a solo extremos
    0x00                                                              // Paso 8: apagar todos
};

int main(void) {
    // LEDs en PORTD
    DDRD  = 0xFF;
    PORTD = 0x00;

    // Entradas PB0 (LENTO) y PB1 (R?PIDO) con resistencias pull-up
    DDRB  &= ~((1 << ENTRADA_LENTO) | (1 << ENTRADA_RAPIDO));
    PORTB |=  ((1 << ENTRADA_LENTO) | (1 << ENTRADA_RAPIDO));

    volatile uint32_t retardo_ms = RETARDO_INICIAL_MS;  // velocidad actual (persistente)
    const uint8_t N = sizeof(patrones) / sizeof(patrones[0]);
    uint8_t i = 0;

    while (1) {
        PORTD = patrones[i];
        esperar_y_ajustar(&retardo_ms, retardo_ms);  // ajustar mientras espera

        // siguiente paso de la secuencia
        i++;
        if (i >= N) i = 0;
    }
}
