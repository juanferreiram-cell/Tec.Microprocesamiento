.include "m328pdef.inc"   ; Incluir las definiciones del microcontrolador ATmega328P

.cseg
.org 0x0000               ; Dirección de arranque (reset)
rjmp reset                ; Salta a la rutina de inicio

reset:
    ; Definir todos los pines de PD0 a PD7 como salidas
    ldi r16, 0xFF         ; Carga 0xFF (11111111 en binario) en r16
    out DDRD, r16         ; Configura todos los pines de PD0 a PD7 como salidas

loop:
    ; Secuencia de encendido de LEDs
    ldi r16, (1 << PD0)      
    out PORTD, r16           
    call delay              

    ldi r16, (1 << PD0) | (1 << PD1)   
    out PORTD, r16                
    call delay                      

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7)
    out PORTD, r16
    call delay

    ; Secuencia de apagado de LEDs
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0) | (1 << PD1)
    out PORTD, r16
    call delay

    ldi r16, (1 << PD0)
    out PORTD, r16
    call delay

    ldi r16, 0x00             ; Apaga todos los LEDs
    out PORTD, r16
    call delay

    rjmp loop                 ; Vuelve al inicio del bucle

; Rutina de retardo
delay:
    ldi r25, 250              ; Carga un valor para el retardo
    ldi r26, 250
delay1:
    ldi r27, 250
delay2:
    dec r27                   ; Decrementa r27
    brne delay2               ; Si no es cero, repite
    dec r26                   ; Decrementa r26
    brne delay1               ; Si no es cero, repite
    dec r25                   ; Decrementa r25
    brne delay                ; Si no es cero, repite
    ret                       ; Retorna de la función
