.include "m328pdef.inc"

.cseg
.org 0x0000
    rjmp reset

reset:
    ldi r16, HIGH(RAMEND)
    out SPH, r16
    ldi r16, LOW(RAMEND)
    out SPL, r16

    ; Declarar PB5 como salida 
    sbi DDRB, DDB5

    ; Declarar PD2 y PD3 como entradas con pull up
    cbi DDRD, DDD2
    cbi DDRD, DDD3
    sbi PORTD, PD2
    sbi PORTD, PD3

    ldi r17, 0

main:
    ; enciendo LED
    sbi PORTB, PB5
    rcall esperar

    ; apago LED
    cbi PORTB, PB5
    rcall esperar
    rjmp main

; espera según cada cuanto parpadea
esperar:
    ldi  r24, 60
    clr  r25
    mov  r30, r17
        
ajustar:
    cpi  r30, 0
    breq comenzar
    lsl  r24             
    rol  r25
    dec  r30
    rjmp ajustar

comenzar:
bucle:
    rcall ret50

    sbis PIND, PD2
    rjmp subir

    sbis PIND, PD3
    rjmp bajar

continuar:
    sbiw r24, 1         
    brne bucle
    ret

; Parpadear más rápido
subir:
    cpi  r17, 3
    breq soltar_subir
    inc  r17
    rcall deb15
soltar_subir:          
    sbis PIND, PD2
    rjmp soltar_subir
    rjmp continuar

; Parpadear más lento
bajar:
    cpi  r17, 0
    breq soltar_bajar
    dec  r17
    rcall deb15
soltar_bajar:          
    sbis PIND, PD3
    rjmp soltar_bajar
    rjmp continuar

; Parpadeo base
ret50:                
    push r18
    push r19
    ldi  r18, 10
    ldi  r19, 199
d50:
    dec  r19
    brne d50
    dec  r18
    brne d50
    pop  r19
    pop  r18
    ret

; Debounce para que evitar señales basura
deb15:             
    push r18
    push r19
    ldi  r18, 3
    ldi  r19, 90
db15:
    dec  r19
    brne db15
    dec  r18
    brne db15
    pop  r19
    pop  r18
    ret
