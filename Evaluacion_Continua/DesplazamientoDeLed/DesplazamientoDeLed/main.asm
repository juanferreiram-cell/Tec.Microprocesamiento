.include "m328pdef.inc"

.org 0x0000
    rjmp DECLARACIONES

DECLARACIONES:
    ; Stack Pointer
    ldi r16, high(RAMEND)
    out SPH, r16
    ldi r16, low(RAMEND)
    out SPL, r16

    ; Se declaran todos los LEDS como salida y se ponen en 0
    ldi r16, 0xFF
    out DDRD, r16
    clr r16
    out PORTD, r16

    ; Se configura el Timer1 en CTC para medio segundo
    ldi r16, 0
    sts TCCR1A, r16
    ldi r16, (1<<WGM12)|(1<<CS12)
    sts TCCR1B, r16
    ldi r16, high(31249)
    sts OCR1AH, r16
    ldi r16, low(31249)
    sts OCR1AL, r16

    ; Carga el LED en P0
    ldi r20, 0x01

	; Con lsl se va desplazando el LED
MAIN:
    out PORTD, r20
    rcall delay
    lsl r20
    brne MAIN
    ldi r20, 0x01
    rjmp MAIN

; Se pone medio segundo de delay entre encendido de los LEDs
DELAY:
    clr r16
    sts TCNT1H, r16
    sts TCNT1L, r16
    ldi r16, (1<<OCF1A)
    out TIFR1, r16

; Espera a que el timer ponga la bandera OCF1A en 1
ESPERA:
    in  r17, TIFR1
    sbrs r17, OCF1A
    rjmp ESPERA
    ldi r16, (1<<OCF1A)  ; Limpia la bandera
    out TIFR1, r16
    ret
