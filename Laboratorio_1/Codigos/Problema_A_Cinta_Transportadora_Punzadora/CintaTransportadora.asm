.include "m328pdef.inc"

.equ F_CPU     = 16000000
.equ BAUD      = 9600
.equ UBRR_VAL  = 103
.equ T_FIN_MS = 3000 ; Este es el tiempo que el sistema esta en terminado antes de arrancar de vuelta

; Pines de los motores en PORTD

.equ M1_A_BIT  = 3
.equ M1_B_BIT  = 5
.equ M3_A_BIT  = 4
.equ M3_B_BIT  = 0

; Pines de los LEDS en PORTC

.equ LED_ESPERA_BIT = 0
.equ LED_FUNCIONANDO_BIT  = 1
.equ LED_TERMINO_BIT = 2
.equ LED_LIGERA_BIT  = 3
.equ LED_MEDIANA_BIT  = 4
.equ LED_PESADA_BIT  = 5

; Tiempos

.equ T_M3_MEDIA = 370 ; Tiempo aproximado para que la palanca de la punzadora de media vuelta (esto hay veces que hay que cambiarlo porque no siempre es el mismo)

; Ligera
.equ L_AVANCE     = 3000
.equ L_PAUSA      = 2000
.equ L_PRESIONAR  = 2000
.equ L_VOLVER     = 3000

; Mediana
.equ M_AVANCE     = 4000
.equ M_PAUSA      = 2000
.equ M_PRESIONAR  = 3000
.equ M_VOLVER     = 4000

; Pesada
.equ P_AVANCE     = 5000
.equ P_PAUSA      = 3000
.equ P_PRESIONAR  = 4000
.equ P_VOLVER     = 5000


.dseg
tipo_carga:   .byte 1
piezas:       .byte 2
t_avance:     .byte 2
t_pausa:      .byte 2
t_mantener:   .byte 2
t_volver:     .byte 2

.cseg
.org 0x0000
    rjmp RESET

RESET:
    ; Stack pointer
    ldi  r16, high(RAMEND)
    out  SPH, r16
    ldi  r16, low(RAMEND)
    out  SPL, r16
    clr  r1

    ldi  r16, (1<<M1_A_BIT)|(1<<M3_A_BIT)|(1<<M1_B_BIT)
    in   r17, DDRD
    or   r17, r16
    out  DDRD, r17
    sbi  DDRB, M3_B_BIT

    ; Apaga los LEDS y prende el de espera

    ldi  r16, (1<<LED_ESPERA_BIT)|(1<<LED_FUNCIONANDO_BIT)|(1<<LED_TERMINO_BIT) \
             |(1<<LED_LIGERA_BIT)|(1<<LED_MEDIANA_BIT)|(1<<LED_PESADA_BIT)
    in   r17, DDRC
    or   r17, r16
    out  DDRC, r17
    rcall LEDS_APAGAR_TODOS
    sbi  PORTC, LED_ESPERA_BIT

    rcall M1_PARAR
    rcall M3_PARAR

; USART

    ldi  r16, high(UBRR_VAL)
    sts  UBRR0H, r16
    ldi  r16, low(UBRR_VAL)
    sts  UBRR0L, r16
    ldi  r16, (1<<RXEN0)|(1<<TXEN0)
    sts  UCSR0B, r16
    ldi  r16, (1<<UCSZ01)|(1<<UCSZ00)
    sts  UCSR0C, r16

; Timer1 a 1ms

    ldi  r16, 0x00
    sts  TCCR1A, r16
    ldi  r16, (1<<WGM12)|(1<<CS11)|(1<<CS10)
    sts  TCCR1B, r16
    ldi  r16, low(249)
    sts  OCR1AL, r16
    ldi  r16, high(249)
    sts  OCR1AH, r16

    ldi  ZL, low(msg_arriba<<1)
    ldi  ZH, high(msg_arriba<<1)
    rcall USART_MANDAR_CADENA

PRINCIPAL:
    rcall LEDS_ESPERANDO

    ; Espera a que la persona mande una "A"

    ldi  ZL, low(msg_pedir_A<<1)
    ldi  ZH, high(msg_pedir_A<<1)
    rcall USART_MANDAR_CADENA
ESPERA_A:
    rcall USART_RX
    cpi  r16, 'A'
    breq A_OK
    cpi  r16, 'a'
    brne ESPERA_A
A_OK:
    rcall USART_TX
    ldi  r16, 13
    rcall USART_TX
    ldi  r16, 10
    rcall USART_TX

    ; Despues de la "A" el sistema esta funcionando
    rcall LEDS_FUNCIONANDO

    ; Se pide el tipo de carga a mandar
    ldi  ZL, low(msg_pedir_carga<<1)
    ldi  ZH, high(msg_pedir_carga<<1)
    rcall USART_MANDAR_CADENA
LEE_CARGA:
    rcall USART_RX
    mov  r18, r16
    cpi  r18, '1'
    breq CARGA_LIGERA
    cpi  r18, '2'
    breq CARGA_MEDIANA
    cpi  r18, '3'
    breq CARGA_PESADA
    rjmp LEE_CARGA

CARGA_LIGERA:
    ldi  r16, 1
    sts  tipo_carga, r16
    rcall LEDS_LIGERA
    rjmp CARGA_CONFIRMADA
CARGA_MEDIANA:
    ldi  r16, 2
    sts  tipo_carga, r16
    rcall LEDS_MEDIANA
    rjmp CARGA_CONFIRMADA
CARGA_PESADA:
    ldi  r16, 3
    sts  tipo_carga, r16
    rcall LEDS_PESADA

CARGA_CONFIRMADA:
    mov  r16, r18
    rcall USART_TX
    ldi  r16, 13
    rcall USART_TX
    ldi  r16, 10
    rcall USART_TX

    ; Se pide la cantidad de piezas a mandar
    ldi  ZL, low(msg_pedir_piezas_1dig<<1)
    ldi  ZH, high(msg_pedir_piezas_1dig<<1)
    rcall USART_MANDAR_CADENA

    rcall LEER_UN_DIGITO
    sts  piezas,   r18
    sts  piezas+1, r19
    rcall USART_FLUSH_RX

    ldi  ZL, low(msg_iniciando<<1)
    ldi  ZH, high(msg_iniciando<<1)
    rcall USART_MANDAR_CADENA

    rcall CARGAR_TIEMPOS
    lds  r26, piezas
    lds  r27, piezas+1

    tst  r26
    brne TIENE_P
    tst  r27
    brne TIENE_P
    rjmp FIN_CICLO
TIENE_P:

BUCLE_PIEZAS:
    ldi  ZL, low(msg_pieza<<1)
    ldi  ZH, high(msg_pieza<<1)
    rcall USART_MANDAR_CADENA
    mov  r16, r26
    subi r16, -'0'
    rcall USART_TX
    ldi  r16, 13
    rcall USART_TX
    ldi  r16, 10
    rcall USART_TX

    ; Alimentacion
    ldi  ZL, low(msg_alimentacion<<1)
    ldi  ZH, high(msg_alimentacion<<1)
    rcall USART_MANDAR_CADENA
    rcall M1_ADELANTE
    lds  r24, t_avance
    lds  r25, t_avance+1
    rcall DELAY_MS

    rcall M1_PARAR
    ldi  ZL, low(msg_pausa<<1)
    ldi  ZH, high(msg_pausa<<1)
    rcall USART_MANDAR_CADENA
    lds  r24, t_pausa
    lds  r25, t_pausa+1
    rcall DELAY_MS

    ; Punzado
    ldi  ZL, low(msg_bajar<<1)
    ldi  ZH, high(msg_bajar<<1)
    rcall USART_MANDAR_CADENA
    rcall M3_BAJAR
    ldi  r24, low(T_M3_MEDIA)
    ldi  r25, high(T_M3_MEDIA)
    rcall DELAY_MS
    rcall M3_PARAR

    ldi  ZL, low(msg_mantener<<1)
    ldi  ZH, high(msg_mantener<<1)
    rcall USART_MANDAR_CADENA
    lds  r24, t_mantener
    lds  r25, t_mantener+1
    rcall DELAY_MS

    ldi  ZL, low(msg_subir<<1)
    ldi  ZH, high(msg_subir<<1)
    rcall USART_MANDAR_CADENA
    rcall M3_SUBIR
    ldi  r24, low(T_M3_MEDIA)
    ldi  r25, high(T_M3_MEDIA)
    rcall DELAY_MS
    rcall M3_PARAR

    ; Descarga
    ldi  ZL, low(msg_descarga<<1)
    ldi  ZH, high(msg_descarga<<1)
    rcall USART_MANDAR_CADENA
    rcall M1_ATRAS
    lds  r24, t_volver
    lds  r25, t_volver+1
    rcall DELAY_MS
    rcall M1_PARAR

    sbiw r26, 1
    brne BUCLE_PIEZAS

FIN_CICLO:
    rcall LEDS_APAGAR_CARGAS

    rcall LEDS_TERMINO
    ldi  ZL, low(msg_ciclo_final<<1)
    ldi  ZH, high(msg_ciclo_final<<1)
    rcall USART_MANDAR_CADENA

    ldi  r24, low(T_FIN_MS)
    ldi  r25, high(T_FIN_MS)
    rcall DELAY_MS

    rjmp PRINCIPAL


; Tiempos para cada carga

CARGAR_TIEMPOS:
    lds  r16, tipo_carga
    cpi  r16, 1
    breq CARGA_LIG
    cpi  r16, 2
    breq CARGA_MED

    ldi  r24, low(P_AVANCE)
    ldi  r25, high(P_AVANCE)
    sts  t_avance, r24
    sts  t_avance+1, r25
    ldi  r24, low(P_PAUSA)
    ldi  r25, high(P_PAUSA)
    sts  t_pausa, r24
    sts  t_pausa+1, r25
    ldi  r24, low(P_PRESIONAR)
    ldi  r25, high(P_PRESIONAR)
    sts  t_mantener, r24
    sts  t_mantener+1, r25
    ldi  r24, low(P_VOLVER)
    ldi  r25, high(P_VOLVER)
    sts  t_volver, r24
    sts  t_volver+1, r25
    ret

CARGA_MED:
    ldi  r24, low(M_AVANCE)
    ldi  r25, high(M_AVANCE)
    sts  t_avance, r24
    sts  t_avance+1, r25
    ldi  r24, low(M_PAUSA)
    ldi  r25, high(M_PAUSA)
    sts  t_pausa, r24
    sts  t_pausa+1, r25
    ldi  r24, low(M_PRESIONAR)
    ldi  r25, high(M_PRESIONAR)
    sts  t_mantener, r24
    sts  t_mantener+1, r25
    ldi  r24, low(M_VOLVER)
    ldi  r25, high(M_VOLVER)
    sts  t_volver, r24
    sts  t_volver+1, r25
    ret

CARGA_LIG:
    ldi  r24, low(L_AVANCE)
    ldi  r25, high(L_AVANCE)
    sts  t_avance, r24
    sts  t_avance+1, r25
    ldi  r24, low(L_PAUSA)
    ldi  r25, high(L_PAUSA)
    sts  t_pausa, r24
    sts  t_pausa+1, r25
    ldi  r24, low(L_PRESIONAR)
    ldi  r25, high(L_PRESIONAR)
    sts  t_mantener, r24
    sts  t_mantener+1, r25
    ldi  r24, low(L_VOLVER)
    ldi  r25, high(L_VOLVER)
    sts  t_volver, r24
    sts  t_volver+1, r25
    ret

; Motores

M1_ADELANTE:
    cbi PORTD, M1_A_BIT
    sbi PORTD, M1_B_BIT
    ret
M1_ATRAS:
    sbi PORTD, M1_A_BIT
    cbi PORTD, M1_B_BIT
    ret
M1_PARAR:
    cbi PORTD, M1_A_BIT
    cbi PORTD, M1_B_BIT
    ret

M3_BAJAR:
    sbi PORTD, M3_A_BIT
    cbi PORTB, M3_B_BIT
    ret
M3_SUBIR:
    cbi PORTD, M3_A_BIT
    sbi PORTB, M3_B_BIT
    ret
M3_PARAR:
    cbi PORTD, M3_A_BIT
    cbi PORTB, M3_B_BIT
    ret

; LEDs

LEDS_APAGAR_TODOS:
    in   r17, PORTC
    andi r17, ~((1<<LED_ESPERA_BIT)|(1<<LED_FUNCIONANDO_BIT)|(1<<LED_TERMINO_BIT) \
               |(1<<LED_LIGERA_BIT)|(1<<LED_MEDIANA_BIT)|(1<<LED_PESADA_BIT))
    out  PORTC, r17
    ret

LEDS_APAGAR_ESTADOS:              ; Apaga los leds que indican el estado (o sea, esperando, funcionando,ternmino)
    in   r17, PORTC
    andi r17, ~((1<<LED_ESPERA_BIT)|(1<<LED_FUNCIONANDO_BIT)|(1<<LED_TERMINO_BIT))
    out  PORTC, r17
    ret

LEDS_ESPERANDO:
    rcall LEDS_APAGAR_ESTADOS
    sbi  PORTC, LED_ESPERA_BIT
    ret

LEDS_FUNCIONANDO:
    rcall LEDS_APAGAR_ESTADOS
    sbi  PORTC, LED_FUNCIONANDO_BIT
    ret

LEDS_TERMINO:
    rcall LEDS_APAGAR_ESTADOS
    sbi  PORTC, LED_TERMINO_BIT
    ret

LEDS_APAGAR_CARGAS:              ; Apaga los leds que indican la carga (ligera, mediana y pesada)
    in   r17, PORTC
    andi r17, ~((1<<LED_LIGERA_BIT)|(1<<LED_MEDIANA_BIT)|(1<<LED_PESADA_BIT))
    out  PORTC, r17
    ret

LEDS_LIGERA:
    rcall LEDS_APAGAR_CARGAS
    sbi  PORTC, LED_LIGERA_BIT
    ret
LEDS_MEDIANA:
    rcall LEDS_APAGAR_CARGAS
    sbi  PORTC, LED_MEDIANA_BIT
    ret
LEDS_PESADA:
    rcall LEDS_APAGAR_CARGAS
    sbi  PORTC, LED_PESADA_BIT
    ret

; Delays

DELAY_MS:
    tst  r24
    brne DLY_LOOP
    tst  r25
    breq DLY_FIN
DLY_LOOP:
    ldi  r16, 0
    sts  TCNT1H, r16
    sts  TCNT1L, r16
    ldi  r16, (1<<OCF1A)
    out  TIFR1, r16
DLY_ESPERAR:
    in   r17, TIFR1
    sbrs r17, OCF1A
    rjmp DLY_ESPERAR
    subi r24, 1
    sbci r25, 0
    brne DLY_LOOP
DLY_FIN:
    ret


; USART

USART_TX:
ESPERAR_UDRE:
    lds  r17, UCSR0A
    sbrs r17, UDRE0
    rjmp ESPERAR_UDRE
    sts  UDR0, r16
    ret

USART_RX:
ESPERAR_RXC:
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp ESPERAR_RXC
    lds  r16, UDR0
    ret

USART_FLUSH_RX:
FLUSH_LOOP:
    lds  r17, UCSR0A
    sbrs r17, RXC0
    ret
    lds  r16, UDR0
    rjmp FLUSH_LOOP

USART_MANDAR_CADENA:
    lpm  r16, Z+
    tst  r16
    breq FIN_CADENA
    rcall USART_TX
    rjmp USART_MANDAR_CADENA
FIN_CADENA:
    ret

; Leer el digito para la cantidad de piezas a mandar

LEER_UN_DIGITO:
LEE_D_BUCLE:
    rcall USART_RX
    cpi  r16, '0'
    brlo LEE_D_BUCLE
    cpi  r16, '9'+1
    brsh LEE_D_BUCLE
    rcall USART_TX
    ldi  r19, 0
    subi r16, '0'
    mov  r18, r16
    ldi  r16, 13
    rcall USART_TX
    ldi  r16, 10
    rcall USART_TX
    ret

; Mensajes
msg_arriba:        .db "Cinta transportadora con punzadora",13,10,0,0
msg_pedir_A:       .db "En espera, enviar 'A' para inicializar",13,10,0,0
msg_pedir_carga:   .db "Carga: 1=Ligera  2=Media  3=Pesada",13,10,0,0
msg_pedir_piezas_1dig: .db "Cantidad de piezas: ",13,10,0,0
msg_iniciando:     .db "Iniciando ciclo...",13,10,0,0
msg_pieza:         .db "Pieza restante: ",13,10,0,0
msg_alimentacion:  .db "Alimentacion (cinta avanzando)",13,10,0,0
msg_pausa:         .db "Pieza posicionada",13,10,0
msg_bajar:         .db "Punzon: Bajando",13,10,0
msg_mantener:      .db "Presionando",13,10,0
msg_subir:         .db "Punzon: Subiendo",13,10,0,0
msg_descarga:      .db "Descarga (cinta retrocediendo)",13,10,0,0
msg_listo:         .db "Proceso finalizado.",13,10,0
msg_ciclo_final:   .db "Ciclo finalizado",13,10,0,0