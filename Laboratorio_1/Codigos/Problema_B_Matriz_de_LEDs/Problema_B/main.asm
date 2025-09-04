;
; Problema_B.asm
;
; Created: 4/9/2025 14:59:12
; Author : Lucas Elizalde, Juan Manuel Ferreira y Felipe Morrudo
;

start:
    ldi  r16, LOW(RAMEND)
    out  SPL, r16
    ldi  r16, HIGH(RAMEND)
    out  SPH, r16

