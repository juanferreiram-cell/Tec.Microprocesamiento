.include "m328pdef.inc"

.org 0x0000
    rjmp INICIO

.equ FILASB_OFF       = 0x3C   
.equ FILASC_OFF       = 0x0F   
.equ FRAMES_POR_PASO  = 10   // Velocidad a la que se produce el scroll, a menos numero mayor velocidad  
.equ NUMFIG           = 21


M:    ; M 
    .db 0x81, 0xc3, 0xa5, 0x99, 0x81, 0x81, 0x81, 0x81

E:     ; E
    .db 0x7f, 0x01, 0x01, 0x01, 0x1f, 0x01, 0x01, 0x7f

ESPACIO:     ; Espacio
    .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

G:    ; G
    .db 0x7f, 0x01, 0x01, 0x01, 0x79, 0x41, 0x41, 0x7f

U:      ; U
    .db 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7f

S:      ; S
    .db 0x7f, 0x01, 0x01, 0x01, 0x7f, 0x40, 0x40, 0x7f

T:      ; T
    .db 0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08

A:      ; A
    .db 0x7e, 0x81, 0x81, 0xff, 0x81, 0x81, 0x81, 0x81

ESPACIO1:     ; Espacio
    .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

C:      ; C
    .db 0x7f, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7f

O:      ; O
    .db 0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff

M1:      ; M
    .db 0x81, 0xc3, 0xa5, 0x99, 0x81, 0x81, 0x81, 0x81

E2:      ; E
    .db 0x7f, 0x01, 0x01, 0x01, 0x1f, 0x01, 0x01, 0x7f

R:      ; R
    .db 0x7f, 0x41, 0x41, 0x41, 0x7f, 0x11, 0x21, 0x41

ESPACIO2:     ; Espacio
    .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

A1:      ; A
    .db 0x7e, 0x81, 0x81, 0xff, 0x81, 0x81, 0x81, 0x81

S1:      ; S
    .db 0x7f, 0x01, 0x01, 0x01, 0x7f, 0x40, 0x40, 0x7f

A2:      ; A
    .db 0x7e, 0x81, 0x81, 0xff, 0x81, 0x81, 0x81, 0x81

D:      ; D
    .db 0x1f, 0x21, 0x41, 0x41, 0x41, 0x41, 0x21, 0x1f

O1:      ; O
    .db 0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff

ESPACIO3:     ; Espacio
    .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00



FIGTAB:
     .dw (M<<1), (E<<1), (ESPACIO<<1), (G<<1), (U<<1), (S<<1), (T<<1), (A<<1), (ESPACIO1<<1), (C<<1), (O<<1), (M1<<1), (E2<<1), (R<<1), (ESPACIO2<<1), (A1<<1), (S1<<1), (A2<<1), (D<<1), (O1<<1), (ESPACIO3<<1)


.def SHIFTX   = r23  
.def FRMCNT   = r24   
.def FIGIDX   = r19   
.def SHCNT    = r25   

INICIO:
    ldi  r16, HIGH(RAMEND)
    out  SPH, r16
    ldi  r16, LOW(RAMEND)
    out  SPL, r16
    clr  r1

    
    in   r16, DDRD
    ori  r16, 0b11111100
    out  DDRD, r16
    ldi  r16, 0b00111111
    out  DDRB, r16
    in   r16, DDRC
    ori  r16, 0b00001111
    out  DDRC, r16

    
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

    
    ldi  r16, (1<<WGM01)
    out  TCCR0A, r16
    ldi  r16, 249
    out  OCR0A, r16
    ldi  r16, (1<<CS01)|(1<<CS00)
    out  TCCR0B, r16

    
    clr  FIGIDX
    rcall CARGAR_FIGURA_Y_SIGUIENTE
    ldi  SHIFTX, 0
    ldi  FRMCNT, FRAMES_POR_PASO
    clr  r22


MAIN_LOOP:
    rcall APAGAR_FILAS   
    movw r30, r26
    mov  r18, r22
    add  ZL, r18
    adc  ZH, r1
    lpm  r20, Z

    
    movw r30, r28
    mov  r18, r22
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z

    
    mov  r18, SHIFTX

    
    cpi  r18, 8
    breq SOLO_B_CON_ESPACIO

    
    tst  r18
    breq A_LISTO

A_SHIFT:
    lsr  r20
    dec  r18
    brne A_SHIFT
A_LISTO:

    
    ldi  SHCNT, 8
    sub  SHCNT, SHIFTX
    tst  SHCNT
    breq B_LISTO
B_SHIFT:
    lsl  r0
    dec  SHCNT
    brne B_SHIFT
B_LISTO:

    
    lsl  r0              

    or   r20, r0
    rjmp COMPOSICION_COMPLETA

SOLO_B_CON_ESPACIO:
    mov  r20, r0
    lsl  r20             

COMPOSICION_COMPLETA:
    
    rcall PONER_COLUMNAS
    mov  r21, r22
    rcall ACTIVAR_FILA
    rcall ESPERAR_1MS

    
    inc  r22
    cpi  r22, 8
    brlo MAIN_LOOP

    
    clr  r22
    dec  FRMCNT
    brne MAIN_LOOP

  
    ldi  FRMCNT, FRAMES_POR_PASO
    inc  SHIFTX
    cpi  SHIFTX, 9           
    brlo MAIN_LOOP

    
    clr  SHIFTX
    inc  FIGIDX
    cpi  FIGIDX, NUMFIG
    brlo SIGUIENTE_OK
    clr  FIGIDX
SIGUIENTE_OK:
    rcall CARGAR_FIGURA_Y_SIGUIENTE
    rjmp MAIN_LOOP


CARGAR_FIGURA_Y_SIGUIENTE:
  
    ldi  ZL, low(FIGTAB<<1)
    ldi  ZH, high(FIGTAB<<1)
    mov  r18, FIGIDX
    lsl  r18
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z+
    mov  r26, r0
    lpm  r0, Z
    mov  r27, r0

    mov  r18, FIGIDX
    inc  r18
    cpi  r18, NUMFIG
    brlo IDX_OK
    clr  r18
IDX_OK:
    ldi  ZL, low(FIGTAB<<1)
    ldi  ZH, high(FIGTAB<<1)
    lsl  r18
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z+
    mov  r28, r0
    lpm  r0, Z
    mov  r29, r0
    ret

APAGAR_FILAS:
    in   r16, PORTB
    ori  r16, FILASB_OFF
    out  PORTB, r16
    in   r16, PORTC
    ori  r16, FILASC_OFF
    out  PORTC, r16
    ret

PONER_COLUMNAS:
    
    mov  r16, r20
    andi r16, 0x3F
    lsl  r16
    lsl  r16
    in   r17, PORTD
    andi r17, 0x03
    or   r17, r16
    out  PORTD, r17

    
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

ACTIVAR_FILA:
    push r18
    push r19
    push r20
    cpi  r21, 4
    brlt FILA_B

    
    subi r21, 4
    ldi  r18, FILASC_OFF
    ldi  r19, 0x01
    mov  r20, r21
DESPLAZAR_PC:
    tst  r20
    breq FIN_PC
    lsl  r19
    dec  r20
    rjmp DESPLAZAR_PC
FIN_PC:
    com  r19
    and  r18, r19
    in   r17, PORTC
    andi r17, 0b11110000
    or   r17, r18
    out  PORTC, r17
    rjmp FIN_FILA

FILA_B:
  
    ldi  r18, FILASB_OFF
    ldi  r19, 0x04
    mov  r20, r21
DESPLAZAR_PB:
    tst  r20
    breq FIN_PB
    lsl  r19
    dec  r20
    rjmp DESPLAZAR_PB
FIN_PB:
    com  r19
    and  r18, r19
    in   r17, PORTB
    andi r17, 0b11000011
    or   r17, r18
    out  PORTB, r17

FIN_FILA:
    pop  r20
    pop  r19
    pop  r18
    ret

ESPERAR_1MS:
    ldi  r16, (1<<OCF0A)
    out  TIFR0, r16
ESPERAR:
    in   r17, TIFR0
    sbrs r17, OCF0A
    rjmp ESPERAR
    ret