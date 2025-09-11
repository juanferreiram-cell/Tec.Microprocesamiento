.include "m328pdef.inc"
.equ DISPLAY_ANODO = 0

.org 0x0000
    rjmp inicio

; LUT en SRAM
.equ LUTB_ADDR  = 0x0100      ; PORTB: segmento a y b (PB4,PB3)
.equ LUTD_ADDR  = 0x0110      ; PORTD: del c al g (PD6 al PD2)

; Botones
.equ BTN_START  = 0           ; PC0 (A0)
.equ BTN_STOP   = 1           ; PC1 (A1)

; Máscaras de pines usados
.equ MASKB     = 0b00011000   ; PB4 (a), PB3 (b)
.equ MASKD     = 0b01111100   ; del PD6 al PD2 (c al g)
.equ INV_MASKB = 0b11100111   
.equ INV_MASKD = 0b10000011   

; Registros
; r1  : SIEMPRE 0
; r16 : contador
; r18 : debounce
; r19 : lectura PINC
; r20 : trabajo general
; r21 : dígito de 0 al 9
; r22 : patrón PORTB
; r23 : patrón PORTD
; r24 : delay (200ms)
; r25 : tmp para RMW de puertos
; r28 y r29 (Y): puntero


;  INICIALIZACIÓN

inicializar_puertos:
    ; PB4 y PB3 como salida
    in      r20, DDRB
    ori     r20, MASKB
    out     DDRB, r20

    ; del PD6 a PD2 como salida
    in      r20, DDRD
    ori     r20, MASKD
    out     DDRD, r20

    ; Botones con pull-up en PC0 y PC1
    clr     r20
    out     DDRC, r20
    ldi     r20, (1<<BTN_START) | (1<<BTN_STOP)
    out     PORTC, r20

    rcall   cargar_tabla_segmentos
    ret


;  Debounce

esperar_pulsacion:
espera_press_0:
    in      r19, PINC
    and     r19, r18
    brne    espera_press_0
    rcall   pausa_5ms
    in      r19, PINC
    and     r19, r18
    brne    espera_press_0
    ret

esperar_soltado:
espera_release_1:
    in      r19, PINC
    and     r19, r18
    breq    espera_release_1
    rcall   pausa_5ms
    in      r19, PINC
    and     r19, r18
    breq    espera_release_1
    ret


;  Delays (16 MHz)

pausa_5ms:
    ldi     r22, 65
d5ms_outer:
    ldi     r23, 255
d5ms_inner:
    dec     r23
    brne    d5ms_inner
    dec     r22
    brne    d5ms_outer
    ret

; Timer para el paso entre segundos debido a que 200ms era muy rapido se modifico para que sea mas lento
pausa_200ms:
    ldi     r24, 255
d200ms_loop:
    rcall   pausa_5ms
    dec     r24
    brne    d200ms_loop
    ret

;  DISPLAY 7 SEGMENTOS

obtener_digito:
    mov     r20, r16
    andi    r20, 0x0F
    mov     r21, r20
    ret

; Escribe dígito r21
mostrar_digito:
    ldi     r28, LOW(LUTB_ADDR)
    ldi     r29, HIGH(LUTB_ADDR)
    add     r28, r21
    adc     r29, r1
    ld      r22, Y             ; PB4 al PB3

    ldi     r28, LOW(LUTD_ADDR)
    ldi     r29, HIGH(LUTD_ADDR)
    add     r28, r21
    adc     r29, r1
    ld      r23, Y             ; PD6 al PD2

.if DISPLAY_ANODO
    ; invertir SOLO los bits usados (para ánodo común)
    ldi     r25, MASKB
    eor     r22, r25
    ldi     r25, MASKD
    eor     r23, r25
.endif

    in      r25, PORTB
    andi    r25, INV_MASKB
    or      r25, r22
    out     PORTB, r25

    in      r25, PORTD
    andi    r25, INV_MASKD
    or      r25, r23
    out     PORTD, r25
    ret

; Carga las tablas de segmentos en SRAM
cargar_tabla_segmentos:
    
; PORTB (a y b)
    ldi     r28, LOW(LUTB_ADDR)
    ldi     r29, HIGH(LUTB_ADDR)
    ldi     r20, 0b00011000    ; 0: a b
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

    ; PORTD (c al g)
    ldi     r28, LOW(LUTD_ADDR)
    ldi     r29, HIGH(LUTD_ADDR)
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

;  PROGRAMA PRINCIPAL

inicio:
    ; Stack
    clr     r1
    rcall   inicializar_puertos
    clr     r16                  
    rjmp    menu_principal

menu_principal:
    ldi     r18, (1<<BTN_START)
    rcall   esperar_pulsacion
    rcall   esperar_soltado

contador_activo:
    rcall   obtener_digito
    rcall   mostrar_digito

    ; STOP
    in      r20, PINC
    sbrc    r20, BTN_STOP
    rjmp    seguir_contando
    ldi     r18, (1<<BTN_STOP)
    rcall   esperar_pulsacion
    rcall   esperar_soltado
    rjmp    menu_principal        

seguir_contando:
    rcall   pausa_200ms
    inc     r16
    cpi     r16, 10
    brlo    contador_activo
    clr     r16
    rjmp    contador_activo
