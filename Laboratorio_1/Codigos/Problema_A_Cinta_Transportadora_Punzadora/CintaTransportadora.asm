.include "m328pdef.inc"

.equ S1_BIT           = 7
.equ S2_BIT           = 2
.equ S1_ACTIVE_HIGH   = 1
.equ S2_ACTIVE_HIGH   = 1
.equ DEBOUNCE_COUNT   = 5 

; Salidas del motor de la cinta
.equ M1_A_PORT = PORTD
.equ M1_A_DDR  = DDRD
.equ M1_A_BIT  = 3

.equ M1_B_PORT = PORTD
.equ M1_B_DDR  = DDRD
.equ M1_B_BIT  = 5

.cseg
.org 0x0000
    rjmp RESET

RESET:
    ; Stack
    ldi  r16, HIGH(RAMEND)
    out  SPH, r16
    ldi  r16, LOW(RAMEND)
    out  SPL, r16

    ; Salidas del motor de la cinta
    sbi  M1_A_DDR, M1_A_BIT
    sbi  M1_B_DDR, M1_B_BIT
    ; Motor APAGADO
    cbi  M1_A_PORT, M1_A_BIT
    cbi  M1_B_PORT, M1_B_BIT

    ; Se ponen los sensores como entrada
    cbi  DDRD, S1_BIT
    cbi  DDRD, S2_BIT
    cbi  PORTD, S1_BIT
    cbi  PORTD, S2_BIT


MAIN:
    ; Esperar S1 -> ON
    rcall WAIT_S1_ACTIVE
    rcall MOTOR_ON

    ; Mantener hasta S2 -> OFF
    rcall WAIT_S2_ACTIVE
    rcall MOTOR_OFF

    rjmp MAIN


; Prendido y apagado del motor de la cinta
MOTOR_ON:
    cbi  M1_A_PORT, M1_A_BIT    ; D3 = 0
    sbi  M1_B_PORT, M1_B_BIT    ; D5 = 1
    ret

MOTOR_OFF:
    cbi  M1_A_PORT, M1_A_BIT    ; D3 = 0
    cbi  M1_B_PORT, M1_B_BIT    ; D5 = 0
    ret

; Detección de las piezas con sensores
WAIT_S1_ACTIVE:
    clr  r18
W1_LOOP:
    in   r16, PIND
.if S1_ACTIVE_HIGH
    sbrs r16, S1_BIT
    rjmp W1_RST
.else
    sbrc r16, S1_BIT
    rjmp W1_RST
.endif
    inc  r18
    cpi  r18, DEBOUNCE_COUNT
    brlo W1_DLY
    ret
W1_RST:
    clr  r18
W1_DLY:
    rcall DELAY_MS_5
    rjmp W1_LOOP

WAIT_S2_ACTIVE:
    clr  r18
W2_LOOP:
    in   r16, PIND
.if S2_ACTIVE_HIGH
    sbrs r16, S2_BIT
    rjmp W2_RST
.else
    sbrc r16, S2_BIT
    rjmp W2_RST
.endif
    inc  r18
    cpi  r18, DEBOUNCE_COUNT
    brlo W2_DLY
    ret
W2_RST:
    clr  r18
W2_DLY:
    rcall DELAY_MS_5
    rjmp W2_LOOP

DELAY_1S:
    ldi  r23, 200
D1S_L:
    rcall DELAY_MS_5
    dec  r23
    brne D1S_L
    ret

DELAY_MS_5:
    ldi  r20, 5
D5_L:
    rcall DELAY_MS_1
    dec  r20
    brne D5_L
    ret

DELAY_MS_1:
    ldi  r21, 100
D1_L1:
    ldi  r22, 53
D1_L2:
    dec  r22
    brne D1_L2
    dec  r21
    brne D1_L1
    ret
