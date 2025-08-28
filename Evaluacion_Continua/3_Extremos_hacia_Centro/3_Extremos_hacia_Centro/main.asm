;
; 3_Extremos_hacia_Centro.asm
;
; Created: 28/8/2025 14:49:51
; Author : juanm
;


; Replace with your application code

        .include "m328pdef.inc"

        .cseg
        .org    0x0000
        rjmp    INICIO

DECLARAR_VARIABLES:
        ; --- Stack ---
        ldi     r16, high(RAMEND)
        out     SPH, r16
        ldi     r16, low(RAMEND)
        out     SPL, r16

        ; --- LED en PB5 como salida ---
        ldi     r16, (1<<DDD0) | (1<<DDD1) | (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7)
        out     DDRD, r16

MAIN:
		ldi r16, (1<<PORTD0) |(1<<PORTD7)
        out PORTD, r16          ; Escribe en el registro de salida
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD1) |(1<<PORTD6)
        out PORTD, r16
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD2) |(1<<PORTD5)
        out PORTD, r16          ; Escribe en el registro de salida
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD3) |(1<<PORTD4)
        out PORTD, r16
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD2) |(1<<PORTD5)
        out PORTD, r16          ; Escribe en el registro de salida
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD1) |(1<<PORTD6)
        out PORTD, r16
		rjmp APAGARLEDS
		ldi r16, (1<<PORTD0) |(1<<PORTD7)
        out PORTD, r16          ; Escribe en el registro de salida
		rjmp APAGARLEDS
		rjmp MAIN

APAGARLEDS:
            ldi     r16, 0            ; Todos los pines de PORTD en bajo
			out     PORTD, r16




