.include "m328pdef.inc"

.org    0x0000
        rjmp    RESET

RESET:

        ; Declaración del Stack Pointer
        ldi     r16, high(RAMEND)
        out     SPH, r16
        ldi     r16, low(RAMEND)
        out     SPL, r16

        ; Se definen los pines del PD2 al PD7 como salidas
        ldi     r16, (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)
        out     DDRD, r16
        clr     r16
        out     PORTD, r16

        ; Configuración del Timer1: modo normal, prescaler 256
        ldi     r17, 0
        sts     TCCR1A, r17               ; Modo normal
        ldi     r17, (1<<CS12)            ; Prescaler 256
        sts     TCCR1B, r17
        ldi     r17, 0
        sts     TCCR1C, r17
        ldi     r17, 0
        sts     TIMSK1, r17               ; Sin interrupciones (Polling)

        ; Configuración del Timer1 para base de 1/3 s: 65536 - (62500/3) ? 44703
        ldi     r17, HIGH(60328)
        sts     TCNT1H, r17
        ldi     r17, LOW(60328)
        sts     TCNT1L, r17

        rjmp    MAIN

MAIN:
        ; Establece la posición inicial (PD7 encendido)
        ldi     r16, (1 <<PD7)
        out     PORTD, r16
        rcall   DELAY_INICIO1

        ; Establece la posición en PD4
        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_INICIO

        ; Baja el solenoide (PD2 encendido)
        ldi     r16, (1 <<PD2)
        out     PORTD, r16
        rcall   DELAY_1S

        ; Ciclo de encendido y apagado de pines para formar el patrón
        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_12S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_4S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_2S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_2S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_2S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_2S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_2S

        ldi     r16, (1 <<PD6)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_1S

        ldi     r16, (1 <<PD7)
        out     PORTD, r16
        rcall   DELAY_4S

        ; Sube el solenoide (PD3 encendido)
        ldi     r16, (1 <<PD3)
        out     PORTD, r16
        rcall   DELAY_1S


; Delays

DELAY_1S:
        ldi     r18, 1
D1_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
D1_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D1_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D1_LOOP
        ret

DELAY_2S:
        ldi     r18, 2
D2_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
D2_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D2_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D2_LOOP
        ret

DELAY_4S:
        ldi     r18, 4
D4S_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
D4S_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D4S_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D4S_LOOP
        ret

DELAY_12S:
        ldi     r18, 12
D12S_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
D12S_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D12S_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D12S_LOOP
        ret

DELAY_INICIO:
        ldi     r18, 80
DINICIO_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
DINICIO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    DINICIO_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    DINICIO_LOOP
        ret

DELAY_INICIO1:
        ldi     r18, 100
DINICIO1_LOOP:
        ldi     r16, HIGH(60328)
        sts     TCNT1H, r16
        ldi     r16, LOW(60328)
        sts     TCNT1L, r16
DINICIO1_WAIT:
        sbis    TIFR1, TOV1
        rjmp    DINICIO1_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    DINICIO1_LOOP
        ret
