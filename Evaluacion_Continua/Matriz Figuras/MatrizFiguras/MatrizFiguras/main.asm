; Columnas: C1 a C6= PD2 a PD7, C7=PB0, C8=PB1
; Filas:    F1 a F4=PB2 a PB5, F5 a F8=PC0 a PC3 

.include "m328pdef.inc"

.org 0x0000
    rjmp INICIO

.equ FILASB_OFF = 0x3C   ; PB2 a PB5
.equ FILASC_OFF = 0x0F   ; PC0 a PC3

; Figura segun los bytes encendidos
SONRISA:
    .db 0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C
TRISTE:
    .db 0x3C,0x42,0xA5,0x81,0x99,0xA5,0x42,0x3C
CORAZON:
    .db 0x00,0x66,0xFF,0xFF,0xFF,0x7E,0x3C,0x18
ROMBO:
    .db 0x18,0x3C,0x7E,0xFF,0xFF,0x7E,0x3C,0x18
ALIEN:
    .db 0x3C,0x7E,0xDB,0xFF,0xFF,0x24,0x5A,0x81

; r20 = patr?n de columnas
; r21 = ?ndice de fila activa
; r22 = contador de fila
; r24:r25 = contador de 3s

INICIO:
    ; Stack
    ldi  r16, HIGH(RAMEND)
    out  SPH, r16
    ldi  r16, LOW(RAMEND)
    out  SPL, r16
    clr  r1

    ; GPIO: columnas salida
    in   r16, DDRD
    ori  r16, 0b11111100      ; PD2 a PD7
    out  DDRD, r16
    ldi  r16, 0b00111111      ; PB0 a PB5 (C7, C8 y F1..F4)
    out  DDRB, r16
    in   r16, DDRC
    ori  r16, 0b00001111      ; PC0 a PC3 (F5 a F8)
    out  DDRC, r16

    ; Columnas en LOW, filas en HIGH
    in   r16, PORTD
    andi r16, 0b00000011
    out  PORTD, r16
    in   r16, PORTB
    andi r16, 0b11000000
    ori  r16, FILASB_OFF
    out  PORTB, r16
    in   r16, PORTC
    andi r16, 0b11110000
    ori  r16, FILASC_OFF
    out  PORTC, r16

    ; 1 ms de timer
    ldi  r16, (1<<WGM01)
    out  TCCR0A, r16
    ldi  r16, 249             ; 16 MHz / 64 = 250 kHz -> 1 ms
    out  OCR0A, r16
    ldi  r16, (1<<CS01)|(1<<CS00)
    out  TCCR0B, r16

BUCLE:
    ; Cara Feliz
    ldi  ZL, low(SONRISA<<1)
    ldi  ZH, high(SONRISA<<1)
    rcall MOSTRAR_3S
    ; Triste
    ldi  ZL, low(TRISTE<<1)
    ldi  ZH, high(TRISTE<<1)
    rcall MOSTRAR_3S
    ; Coraz?n
    ldi  ZL, low(CORAZON<<1)
    ldi  ZH, high(CORAZON<<1)
    rcall MOSTRAR_3S
    ; Rombo
    ldi  ZL, low(ROMBO<<1)
    ldi  ZH, high(ROMBO<<1)
    rcall MOSTRAR_3S
    ; Alien de Space Invader 
    ldi  ZL, low(ALIEN<<1)
    ldi  ZH, high(ALIEN<<1)
    rcall MOSTRAR_3S

    rjmp BUCLE

; Muestra la figura apuntada por Z durante 3s
MOSTRAR_3S:
    movw r26, r30             ; X = base de la figura
    ldi  r25, 0x0B            ; 0x0BB8 = 3000
    ldi  r24, 0xB8
    clr  r22                  ; Fila = 0
MOSTRAR_L:
    rcall APAGAR_FILAS
    lpm  r20, Z+              ; Patr?n de la fila actual
    rcall PONER_COLUMNAS
    mov  r21, r22
    rcall ACTIVAR_FILA
    rcall ESPERAR_1MS

    inc  r22
    cpi  r22, 8
    brlo FilaOK
    clr  r22
    movw r30, r26             ; Reiniciar puntero
FilaOK:
    sbiw r24, 1
    brne MOSTRAR_L
    ret

; Apaga todas las Filas
APAGAR_FILAS:
    in   r16, PORTB
    ori  r16, FILASB_OFF
    out  PORTB, r16
    in   r16, PORTC
    ori  r16, FILASC_OFF
    out  PORTC, r16
    ret

; Carga las columnas
PONER_COLUMNAS:
    ; PD2 a PD7
    mov  r16, r20
    andi r16, 0x3F
    lsl  r16
    lsl  r16
    in   r17, PORTD
    andi r17, 0x03
    or   r17, r16
    out  PORTD, r17
    ; PB0 a PB1
    mov  r16, r20
    andi r16, 0xC0
    lsr  r16
    lsr  r16
    lsr  r16
    lsr  r16
    lsr  r16
    lsr  r16
    in   r17, PORTB
    andi r17, 0b11111100
    or   r17, r16
    out  PORTB, r17
    ret

; Activa las filas
ACTIVAR_FILA:
    push r18
    push r19
    push r20

    cpi  r21, 4
    brlt FilaB

    ; F5 a F8 en PORTC
    subi r21, 4
    ldi  r18, FILASC_OFF      
    ldi  r19, 0x01            
    mov  r20, r21
DESPLAZAR_PC:
    tst  r20
    breq FIN_DESPLAZAR_PC
    lsl  r19
    dec  r20
    rjmp DESPLAZAR_PC
FIN_DESPLAZAR_PC:
    com  r19                  ; dejar 0 en la fila elegida
    and  r18, r19
    in   r17, PORTC
    andi r17, 0b11110000
    or   r17, r18
    out  PORTC, r17
    rjmp FilaEnd

FilaB:
    ; F1 a F4 en PORTB
    ldi  r18, FILASB_OFF     
    ldi  r19, 0x04            ; PB2
    mov  r20, r21
DESPLAZAR_PB:
    tst  r20
    breq FIN_DESPLAZAR_PB
    lsl  r19
    dec  r20
    rjmp DESPLAZAR_PB
FIN_DESPLAZAR_PB:
    com  r19
    and  r18, r19
    in   r17, PORTB
    andi r17, 0b11000011
    or   r17, r18
    out  PORTB, r17

FilaEnd:
    pop  r20
    pop  r19
    pop  r18
    ret

; Espera 1 ms con el Timer0 en CTC 
ESPERAR_1MS:
    ldi  r16, (1<<OCF0A)
    out  TIFR0, r16
W1:
    in   r17, TIFR0
    sbrs r17, OCF0A
    rjmp W1
    ret
