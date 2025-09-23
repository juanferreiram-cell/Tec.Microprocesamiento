.include "m328pdef.inc"

.org    0x0000
        rjmp    RESET

RESET:

        ; Declaracion del Stack Pointer
        ldi     r16, high(RAMEND)
        out     SPH, r16
        ldi     r16, low(RAMEND)
        out     SPL, r16

		; Se definen los pines del PD2 al PD7 como salidas
        ldi     r16, (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)
        out     DDRD, r16
        clr     r16
        out     PORTD, r16

        ldi     r17, 0
        sts     TCCR1A, r17
        ldi     r17, (1<<CS12)
        sts     TCCR1B, r17
        ldi     r17, 0
        sts     TCCR1C, r17
        ldi     r17, 0
        sts     TIMSK1, r17

        ldi     r17, HIGH(3036)
        sts     TCNT1H, r17
        ldi     r17, LOW(3036)
        sts     TCNT1L, r17

        rjmp    MAIN

MAIN:
        ldi     r16, (1 <<PD7)
        out     PORTD, r16
        rcall   DELAY_10S

	    ldi     r16, (1<<PD4)
        out     PORTD, r16
        rcall   DELAY_2S
		

	    ldi     r16, (1 <<PD2) 
        out     PORTD, r16
        rcall   DELAY_2S

	    ldi     r16, (1 <<PD4)
        out     PORTD, r16
        rcall   DELAY_4_9S

	    ldi     r16, (1<<PD3)
        out     PORTD, r16
        rcall   DELAY_2S

	    ldi     r16, (1<<PD2) 
        out     PORTD, r16
        rcall   DELAY_2S

	    ldi     r16, (1<<PD7)
        out     PORTD, r16
        rcall   DELAY_4_9S

	    ldi     r16, (1<<PD3)
        out     PORTD, r16
        rcall   DELAY_2S

	    ldi     r16, (1<<PD2) 
        out     PORTD, r16
        rcall   DELAY_2S

	    ldi     r16, (1<<PD6) | (1<<PD5)
        out     PORTD, r16
        rcall   DELAY_5S

	    ldi     r16, (1<<PD3)
        out     PORTD, r16
        rcall   DELAY_10S



DELAY_2S:
        ldi     r18, 1
D2_LOOP:
        ldi     r16, HIGH(3036)
        sts     TCNT1H, r16
        ldi     r16, LOW(3036)
        sts     TCNT1L, r16
D2_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D2_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D2_LOOP
        ret

DELAY_15S:
        ldi     r18, 15
D15_LOOP:
        ldi     r16, HIGH(3036)
        sts     TCNT1H, r16
        ldi     r16, LOW(3036)
        sts     TCNT1L, r16
D15_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D15_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D15_LOOP
        ret

DELAY_10S:
        ldi     r18, 10
D10_LOOP:
        ldi     r16, HIGH(3036)
        sts     TCNT1H, r16
        ldi     r16, LOW(3036)
        sts     TCNT1L, r16
D10_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D10_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D10_LOOP
        ret

DELAY_1_4S:
        ldi     r16, HIGH(3036)
        sts     TCNT1H, r16
        ldi     r16, LOW(3036)
        sts     TCNT1L, r16
D14_WAIT1:
        sbis    TIFR1, TOV1
        rjmp    D14_WAIT1
        sbi     TIFR1, TOV1

        ldi     r16, HIGH(40536)
        sts     TCNT1H, r16
        ldi     r16, LOW(40536)
        sts     TCNT1L, r16
D14_WAIT2:
        sbis    TIFR1, TOV1
        rjmp    D14_WAIT2
        sbi     TIFR1, TOV1

        ret

DELAY_6S:
        ldi     r18, 6
D6_LOOP:
        ldi     r16, HIGH(3036)
        sts     TCNT1H, r16
        ldi     r16, LOW(3036)
        sts     TCNT1L, r16
D6_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D6_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D6_LOOP
        ret


DELAY_4_9S:
        ldi     r18, 4
D49_LOOP:
        ldi     r16, HIGH(60848)          
        sts     TCNT1H, r16
        ldi     r16, LOW(60848)
        sts     TCNT1L, r16
D49_WAIT1:
        sbis    TIFR1, TOV1
        rjmp    D49_WAIT1
        sbi     TIFR1, TOV1
        dec     r18
        brne    D49_LOOP
        ldi     r16, HIGH(60848)       
        sts     TCNT1H, r16
        ldi     r16, LOW(60848)
        sts     TCNT1L, r16
D49_WAIT2:
        sbis    TIFR1, TOV1
        rjmp    D49_WAIT2
        sbi     TIFR1, TOV1
        ret

DELAY_5S:
        ldi     r18, 5
D5_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D5_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D5_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D5_LOOP
        ret