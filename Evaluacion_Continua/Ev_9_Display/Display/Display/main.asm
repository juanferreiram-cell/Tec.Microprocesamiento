.include "m328pdef.inc"

.org 0x0000
    rjmp inicio

.equ TABLA_B_DIR  = 0x0100
.equ TABLA_D_DIR  = 0x0110

.equ MASCARA_B     = 0b00011000
.equ MASCARA_D     = 0b01111100
.equ INV_MASCARA_B = 0b11100111
.equ INV_MASCARA_D = 0b10000011

inicializar_puertos:
    in      r20, DDRB
    ori     r20, MASCARA_B
    out     DDRB, r20
    in      r20, DDRD
    ori     r20, MASCARA_D
    out     DDRD, r20
    rcall   cargar_tablas
    ret

pausa_5ms:
    ldi     r22, 65
d5_externo:
    ldi     r23, 255
d5_interno:
    dec     r23
    brne    d5_interno
    dec     r22
    brne    d5_externo
    ret

pausa_200ms:
    ldi     r24, 255
d200_bucle:
    rcall   pausa_5ms
    dec     r24
    brne    d200_bucle
    ret

obtener_numero:
    mov     r21, r16
    andi    r21, 0x0F
    ret

mostrar_numero:
    ldi     r28, LOW(TABLA_B_DIR)
    ldi     r29, HIGH(TABLA_B_DIR)
    add     r28, r21
    adc     r29, r1
    ld      r22, Y
    ldi     r28, LOW(TABLA_D_DIR)
    ldi     r29, HIGH(TABLA_D_DIR)
    add     r28, r21
    adc     r29, r1
    ld      r23, Y
    in      r25, PORTB
    andi    r25, INV_MASCARA_B
    or      r25, r22
    out     PORTB, r25
    in      r25, PORTD
    andi    r25, INV_MASCARA_D
    or      r25, r23
    out     PORTD, r25
    ret

cargar_tablas:
    ldi     r28, LOW(TABLA_B_DIR)
    ldi     r29, HIGH(TABLA_B_DIR)
    ldi     r20, 0b00011000
    st      Y+, r20
    ldi     r20, 0b00001000
    st      Y+, r20
    ldi     r20, 0b00011000
    st      Y+, r20
    ldi     r20, 0b00011000
    st      Y+, r20
    ldi     r20, 0b00001000
    st      Y+, r20
    ldi     r20, 0b00010000
    st      Y+, r20
    ldi     r20, 0b00010000
    st      Y+, r20
    ldi     r20, 0b00011000
    st      Y+, r20
    ldi     r20, 0b00011000
    st      Y+, r20
    ldi     r20, 0b00011000
    st      Y+, r20

    ldi     r28, LOW(TABLA_D_DIR)
    ldi     r29, HIGH(TABLA_D_DIR)
    ldi     r20, 0b01111000
    st      Y+, r20
    ldi     r20, 0b01000000
    st      Y+, r20
    ldi     r20, 0b00110100
    st      Y+, r20
    ldi     r20, 0b01100100
    st      Y+, r20
    ldi     r20, 0b01001100
    st      Y+, r20
    ldi     r20, 0b01101100
    st      Y+, r20
    ldi     r20, 0b01111100
    st      Y+, r20
    ldi     r20, 0b01000000
    st      Y+, r20
    ldi     r20, 0b01111100
    st      Y+, r20
    ldi     r20, 0b01101100
    st      Y+, r20
    ret

inicio:
    clr     r1
    rcall   inicializar_puertos
    clr     r16

bucle:
    rcall   obtener_numero
    rcall   mostrar_numero
    rcall   pausa_200ms
    inc     r16
    cpi     r16, 10
    brlo    bucle
    clr     r16
    rjmp    bucle
