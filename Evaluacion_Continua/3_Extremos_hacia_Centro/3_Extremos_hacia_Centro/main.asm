;
; 3_Extremos_hacia_Centro.asm
;
; Created: 28/8/2025
; Author : Lucas Elizalde, Juan Manuel Ferreira, Felipe Morrudo
;


.include "m328pdef.inc"

.cseg
.org    0x0000
rjmp    DECLARAR_VARIABLES

; Se declararan los pines en el R16 y el Timer el R17
DECLARAR_VARIABLES:
       
	   ; Stack Pointer
        ldi     r16, high(RAMEND)
        out     SPH, r16
        ldi     r16, low(RAMEND)
        out     SPL, r16

    ; Declaracion de pines del 0 al 8 del Arduino
        ldi     r16, (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7)
        out     DDRD, r16
		ldi     r16, (1<<DDB0) | (1<<DDB1)                            
        out     DDRB, r16
	
	; Declaracion del Timer
		LDI R17,0
        STS TCCR1A,R17
        LDI R17,(1<<CS12)
        STS TCCR1B,R17
        LDI R16,0
        STS TCCR1C,R17
        LDI R17,0
        STS TIMSK1,R17
        LDI R17,HIGH(3036)
        STS TCNT1H,R17
        LDI R17,LOW(3036)
        STS TCNT1L,R17
        SEI
		rjmp    MAIN

; Main principal del codigo con funcionamiento del mismo

MAIN:
		ldi r16, (1<<PORTB0) | (1<<PORTB1)
        out PORTB, r16     
		call DELAY
		call APAGARLEDS
		call DELAY     
		ldi r16, (1<<PORTD7)   |(1<<PORTD6)
        out PORTD, r16
		call DELAY
		call APAGARLEDS
		call DELAY
		ldi r16, (1<<PORTD2) |(1<<PORTD5)
        out PORTD, r16          
		call DELAY
		call APAGARLEDS
		call DELAY
		ldi r16, (1<<PORTD3) |(1<<PORTD4)
        out PORTD, r16
		call DELAY
		call APAGARLEDS
		call DELAY
		ldi r16, (1<<PORTD2) |(1<<PORTD5)
        out PORTD, r16          
		call DELAY
		call APAGARLEDS
		call DELAY
		ldi r16, (1<<PORTD7) |(1<<PORTD6)
        out PORTD, r16
		call DELAY
		call APAGARLEDS
		call DELAY
		ldi r16, (1<<PORTB0) | (1<<PORTB1)
        out PORTD, r16
		call DELAY
		call APAGARLEDS
		call DELAY
		rjmp MAIN


; Funcion que apaga los LEDS
APAGARLEDS:
            ldi     r16, 0        
			out     PORTD, r16
			out     PORTB, r16

; Funcion para el Delay
DELAY:
        SBIS TIFR1,TOV1
        RJMP DELAY
        SBI TIFR1,TOV1
        LDI R16,HIGH(1500)
        STS TCNT1H,R16
        LDI R16,LOW(1500)
        STS TCNT1L,R16
        RET
