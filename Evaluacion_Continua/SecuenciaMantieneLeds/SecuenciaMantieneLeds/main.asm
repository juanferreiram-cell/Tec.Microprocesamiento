.include "m328pdef.inc"

.cseg
.org 0x0000
    rjmp reset

reset:
    ; Defino de PD0 a PD7 como salidas
    ldi r16, 0xFF
    out DDRD, r16

    ; El Timer1 en modo normal con el prescaler en 256
  
    ldi r17, 0x00
    sts TCCR1A, r17
    ldi r17, (1<<CS12)      ; Prescaler 256
    sts TCCR1B, r17
    ldi r16, 0x00
    sts TCCR1C, r16
    ldi r17, 0x00           ; Sin interrupciones del Timer1
    sts TIMSK1, r17

    ; Carga inicial de TCNT1 = 3036 (1s con 16 MHz / 256 del prescaler)
    ldi r17, HIGH(3036)
    sts TCNT1H, r17
    ldi r17, LOW(3036)
    sts TCNT1L, r17

    sei                     
    rjmp loop

loop:
    ; Secuencia de subida

    ; Prende el LED 0
    ldi r16, 0x01
    out PORTD, r16
    call delay

    ; Prende el LED 1 y deja los anteriores prendidos
    ldi r16, 0x03
    out PORTD, r16
    call delay

    ; Prende el LED 2 y deja los anteriores prendidos
    ldi r16, 0x07
    out PORTD, r16
    call delay

    ; Prende el LED 3 y deja los anteriores prendidos
    ldi r16, 0x0F
    out PORTD, r16
    call delay

    ; Prende el LED 4 y deja los anteriores prendidos
    ldi r16, 0x1F
    out PORTD, r16
    call delay

    ; Prende el LED 5 y deja los anteriores prendidos
    ldi r16, 0x3F
    out PORTD, r16
    call delay

    ; Prende el LED 6 y deja los anteriores prendidos
    ldi r16, 0x7F
    out PORTD, r16
    call delay

    ; Prende el LED 7 y deja los anteriores prendidos
    ldi r16, 0xFF
    out PORTD, r16
    call delay

    ; Secuencia de bajada

    ; Prende hasta el LED 6 y deja los anteriores prendidos
    ldi r16, 0x7F
    out PORTD, r16
    call delay

    ; Prende hasta el LED 5 y deja los anteriores prendidos
    ldi r16, 0x3F
    out PORTD, r16
    call delay

    ; Prende hasta el LED 4 y deja los anteriores prendidos
    ldi r16, 0x1F
    out PORTD, r16
    call delay

    ; Prende hasta el LED 3 y deja los anteriores prendidos
    ldi r16, 0x0F
    out PORTD, r16
    call delay

    ; Prende hasta el LED 2 y deja los anteriores prendidos
    ldi r16, 0x07
    out PORTD, r16
    call delay

    ; Prende hasta el LED 1 y deja los anteriores prendidos
    ldi r16, 0x03
    out PORTD, r16
    call delay

    ; Prende solo el LED 0
    ldi r16, 0x01
    out PORTD, r16
    call delay

    ; Apaga el LED 0 (Están todos apagados)
    ldi r16, 0x00
    out PORTD, r16
    call delay

    rjmp loop

; Delay de 1s usando el Timer1
; Espera al overflow, limpia el flag y recarga la precarga en 3036.
delay:
    SBIS TIFR1, TOV1        ; Si TOV1=1 es que hubo overflow, salta a la siguiente instrucción
    RJMP delay              

    SBI  TIFR1, TOV1        ; Limpia el flag

    LDI  R16, HIGH(3036)    ; Precarga TCNT1 = 3036 (la parte alta)
    STS  TCNT1H, R16
    LDI  R16, LOW(3036)     ; Precarga TCNT1 = 3036 (la parte baja)
    STS  TCNT1L, R16

    RET             
