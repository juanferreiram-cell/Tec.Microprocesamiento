.include "m328pdef.inc"

.equ S1_ACTIVE_HIGH = 1
.equ S2_ACTIVE_HIGH = 1
.equ DEBOUNCE = 5

; Tiempos de la punzadora
.equ T_DESC_MS        = 450      ; Prende M2 para que de media vuelta la palanca de la punzadora y descienda
.equ T_PRESS_MS       = 2000     ; Apaga M2 para que presione por dos segundos
.equ T_ASC_MS         = 450      ; Prende M2 en el otro sentido para que ascienda

; Pines
.equ S1_BIT           = 7
.equ S2_BIT           = 2

.equ M1_A_PORT        = PORTD
.equ M1_A_DDR         = DDRD
.equ M1_A_BIT         = 3
.equ M1_B_PORT        = PORTD
.equ M1_B_DDR         = DDRD
.equ M1_B_BIT         = 5
.equ REVERSE_M1       = 1

.equ M2_A_PORT        = PORTD
.equ M2_A_DDR         = DDRD
.equ M2_A_BIT         = 6
.equ M2_B_PORT        = PORTB
.equ M2_B_DDR         = DDRB
.equ M2_B_BIT         = 1
.equ REVERSE_M2       = 0

.cseg
.org 0x0000
    rjmp RESET

RESET:
    ; Stack
    ldi  r16, HIGH(RAMEND)
    out  SPH, r16
    ldi  r16, LOW(RAMEND)
    out  SPL, r16

    ; M1 salidas y apagado
    sbi  M1_A_DDR, M1_A_BIT
    sbi  M1_B_DDR, M1_B_BIT
    cbi  M1_A_PORT, M1_A_BIT
    cbi  M1_B_PORT, M1_B_BIT

    ; M2 salidas y apagado
    sbi  M2_A_DDR, M2_A_BIT
    sbi  M2_B_DDR, M2_B_BIT
    cbi  M2_A_PORT, M2_A_BIT
    cbi  M2_B_PORT, M2_B_BIT

    ; Sensores como entrada (UNO-F5 acondiciona; sin pull-ups)
    cbi  DDRD, S1_BIT
    cbi  DDRD, S2_BIT
    cbi  PORTD, S1_BIT
    cbi  PORTD, S2_BIT

MAIN:
    ; El sensor detecta la pieza y empieza a avanzar la cinta
    rcall WAIT_S1_ACTIVE
    rcall M1_ON

    ; Llega al sensor dos y se apaga la cinta
    rcall WAIT_S2_ACTIVE
    rcall M1_OFF

    ; Se activa la punzadora
    rcall PUNCH_SEQUENCE

    ; La cinta va en sentido contrario hasta llevar la pieza al primer sensor
    rcall M1_ON_REV
    rcall WAIT_S1_ACTIVE
    rcall M1_OFF

    ; Espera a que saquen la pieza
    rcall WAIT_S1_INACTIVE

    rjmp MAIN

; Configuración de los motores

; Motor avanzando la cinta
M1_ON:
.if REVERSE_M1
    cbi  M1_A_PORT, M1_A_BIT    ; D3 = 0
    sbi  M1_B_PORT, M1_B_BIT    ; D5 = 1
.else
    sbi  M1_A_PORT, M1_A_BIT    ; D3 = 1
    cbi  M1_B_PORT, M1_B_BIT    ; D5 = 0
.endif
    ret

; Motor de la cinta en sentido contrario
M1_ON_REV:
.if REVERSE_M1
    sbi  M1_A_PORT, M1_A_BIT    ; D3 = 1
    cbi  M1_B_PORT, M1_B_BIT    ; D5 = 0
.else
    cbi  M1_A_PORT, M1_A_BIT    ; D3 = 0
    sbi  M1_B_PORT, M1_B_BIT    ; D5 = 1
.endif
    ret

M1_OFF:
    cbi  M1_A_PORT, M1_A_BIT
    cbi  M1_B_PORT, M1_B_BIT
    ret

; Motor de la punzadora
M2_DOWN:
.if REVERSE_M2
    cbi  M2_A_PORT, M2_A_BIT
    sbi  M2_B_PORT, M2_B_BIT
.else
    sbi  M2_A_PORT, M2_A_BIT
    cbi  M2_B_PORT, M2_B_BIT
.endif
    ret

M2_UP:
.if REVERSE_M2
    sbi  M2_A_PORT, M2_A_BIT
    cbi  M2_B_PORT, M2_B_BIT
.else
    cbi  M2_A_PORT, M2_A_BIT
    sbi  M2_B_PORT, M2_B_BIT
.endif
    ret

M2_OFF:
    cbi  M2_A_PORT, M2_A_BIT
    cbi  M2_B_PORT, M2_B_BIT
    ret

; Pasos de la punzadora

PUNCH_SEQUENCE:
    ; Baja
    rcall M2_DOWN
    ldi  r24, LOW(T_DESC_MS)
    ldi  r25, HIGH(T_DESC_MS)
    rcall DELAY_MS
    rcall M2_OFF

    ; Presiona
    ldi  r24, LOW(T_PRESS_MS)
    ldi  r25, HIGH(T_PRESS_MS)
    rcall DELAY_MS

    ; Asciende
    rcall M2_UP
    ldi  r24, LOW(T_ASC_MS)
    ldi  r25, HIGH(T_ASC_MS)
    rcall DELAY_MS
    rcall M2_OFF
    ret

; Antirebote

; Sensor 1 Activo estable
WAIT_S1_ACTIVE:
    clr  r18
W1A_LOOP:
    in   r16, PIND
.if S1_ACTIVE_HIGH
    sbrs r16, S1_BIT
    rjmp W1A_RST
.else
    sbrc r16, S1_BIT
    rjmp W1A_RST
.endif
    inc  r18
    cpi  r18, DEBOUNCE
    brlo W1A_DLY
    ret
W1A_RST:
    clr  r18
W1A_DLY:
    rcall DELAY_MS_5
    rjmp W1A_LOOP

; Sensor 1 Inactivo estable
WAIT_S1_INACTIVE:
    clr  r18
W1I_LOOP:
    in   r16, PIND
.if S1_ACTIVE_HIGH
    sbrs r16, S1_BIT
    rjmp W1I_GOOD
    rjmp W1I_RST
W1I_GOOD:
    inc  r18
    cpi  r18, DEBOUNCE_COUNT
    brlo W1I_DLY
    ret
W1I_RST:
    clr  r18
W1I_DLY:
    rcall DELAY_MS_5
    rjmp W1I_LOOP
.else
    sbrs r16, S1_BIT
    rjmp W1I_RST
    inc  r18
    cpi  r18, DEBOUNCE_COUNT
    brlo W1I_DLY2
    ret
W1I_RST:
    clr  r18
W1I_DLY2:
    rcall DELAY_MS_5
    rjmp W1I_LOOP
.endif

; Segundo sensor estable
WAIT_S2_ACTIVE:
    clr  r18
W2A_LOOP:
    in   r16, PIND
.if S2_ACTIVE_HIGH
    sbrs r16, S2_BIT
    rjmp W2A_RST
.else
    sbrc r16, S2_BIT
    rjmp W2A_RST
.endif
    inc  r18
    cpi  r18, DEBOUNCE_COUNT
    brlo W2A_DLY
    ret
W2A_RST:
    clr  r18
W2A_DLY:
    rcall DELAY_MS_5
    rjmp W2A_LOOP

; DELAYS

DELAY_MS:
    mov  r20, r24
    or   r20, r25
    breq DM_DONE
DM_LOOP:
    rcall DELAY_MS_1
    sbiw r24, 1
    brne DM_LOOP
DM_DONE:
    ret

DELAY_S:
    mov  r25, r24
DS_LOOP:
    rcall DELAY_1S
    dec  r25
    brne DS_LOOP
    ret

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
