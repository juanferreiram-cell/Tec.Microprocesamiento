; Primer Progreso

.include "m328pdef.inc"

.cseg
.org 0x0000
    rjmp Inicio

; Registros
.def aux1  = r16
.def aux2  = r17
.def valor = r18

.equ OCR2A_VAL = 30   ; fs de 64.5 kHz, fout de 1 kHz

Inicio:
    ; Configurar stack
    ldi aux1, high(RAMEND)
    out SPH, aux1
    ldi aux1, low(RAMEND)
    out SPL, aux1

    ; PORTD salida para DAC 2R-R
    ldi aux1, 0xFF
    out DDRD, aux1
    ldi aux1, 0x00
    out PORTD, aux1

    ; Timer2 en modo CTC con prescaler en 8
    ldi aux1, (1<<WGM21)
    sts TCCR2A, aux1
    ldi aux1, (1<<CS21)
    sts TCCR2B, aux1
    ldi aux1, OCR2A_VAL
    sts OCR2A, aux1

    ldi ZL, low(Tabla*2)
    ldi ZH, high(Tabla*2)

Bucle:
EsperarTick:
    in  aux1, TIFR2
    sbrs aux1, OCF2A
    rjmp EsperarTick
    ; limpiar flag
    ldi aux1, (1<<OCF2A)
    out TIFR2, aux1

    ; Siguiente muestra
    lpm  valor, Z+
    out  PORTD, valor

    ; Reinicio de tabla.
    ldi  aux1,  low(TablaFin*2)
    ldi  aux2,  high(TablaFin*2)
    cp   ZL, aux1
    cpc  ZH, aux2
    brne Bucle
    ldi  ZL, low(Tabla*2)
    ldi  ZH, high(Tabla*2)
    rjmp Bucle

; Tabla de 64 muestras
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
