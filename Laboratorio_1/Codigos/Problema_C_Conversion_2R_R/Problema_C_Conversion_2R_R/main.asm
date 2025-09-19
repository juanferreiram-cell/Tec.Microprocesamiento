; Señal 7 (64 muestras) con DAC R-2R en PORTD usando Timer2
; Microcontrolador: ATmega328P y Frecuencia_CPU = 16 MHz
; Fout = 1 kHz (64 muestras que es fs = 64 kHz)
; Fs = F_CPU / (presc * (1 + OCR2A)) = 16x10^-6 / (8 * (1 + 30)) ? 64.516 kHz

.include "m328pdef.inc"

.cseg
.org 0x0000
    rjmp Inicio                  ; Reset vector
.org OC2Aaddr
    rjmp Interrupcion            ; TIMER2_COMPA

; Registros de trabajo
.def aux1  = r16
.def aux2  = r17
.def valor = r18                 ; Byte leído de tabla

.equ OCR2A_VAL = 30              ; Se varia segun la Fout buscada

Inicio:
    ; Stack
    ldi aux1, high(RAMEND)
    out SPH, aux1
    ldi aux1, low(RAMEND)
    out SPL, aux1

    ; GPIO: PORTD salida (8 bits al R-2R)
    ldi aux1, 0xFF
    out DDRD, aux1
    ldi aux1, 0x00
    out PORTD, aux1

    ; Timer2 en CTC
    ldi aux1, OCR2A_VAL
    sts OCR2A, aux1              ; Valor de comparación
    ldi aux1, (1<<WGM21)
    sts TCCR2A, aux1             ; CTC con OCR2A
    ldi aux1, (1<<OCF2A)
    out TIFR2, aux1              ; Limpiar OCF2AW1C)
    ldi aux1, (1<<OCIE2A)
    sts TIMSK2, aux1             ; Habilitar interrupción COMPA
    ldi aux1, (1<<CS21)
    sts TCCR2B, aux1             ; Prescaler = 8 

    ; Puntero de tabla en FLASH 
    ldi ZL, low(Tabla*2)
    ldi ZH, high(Tabla*2)

    sei                           ; Habilitar interrupciones

Bucle:
    rjmp Bucle                  

; Interrupción Timer2 COMPA
Interrupcion:
    
    lpm  valor, Z+                ; Leer siguiente muestra
    out  PORTD, valor             ; Salida a 2R-R

    ; Volver al inicio si Z llego al fin de tabla
    ldi  aux1,  low(TablaFin*2)
    ldi  aux2,  high(TablaFin*2)
    cp   ZL, aux1
    cpc  ZH, aux2
    brlo seguir
    ldi  ZL, low(Tabla*2)
    ldi  ZH, high(Tabla*2)
seguir:
    reti

; Tabla Señal 7 (64 muestras) Piramide
Tabla:
    .db 0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38
    .db 0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
    .db 0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xB8
    .db 0xC0,0xC8,0xD0,0xD8,0xE0,0xE8,0xF0,0xF8
    .db 0xFF,0xF7,0xEF,0xE7,0xDF,0xD7,0xCF,0xC7
    .db 0xBF,0xB7,0xAF,0xA7,0x9F,0x97,0x8F,0x87
    .db 0x7F,0x77,0x6F,0x67,0x5F,0x57,0x4F,0x47
    .db 0x3F,0x37,0x2F,0x27,0x1F,0x17,0x0F,0x07
TablaFin:
