.include "m328pdef.inc"

.org 0x0000
    rjmp Inicio

.equ F_CPU = 16000000
.equ baud  = 9600
.equ bps   = (F_CPU/16/baud) - 1

Inicio:
    ldi r16, HIGH(RAMEND)
    out SPH, r16
    ldi r16, LOW(RAMEND)
    out SPL, r16

    ldi r16, LOW(bps)
    ldi r17, HIGH(bps)
    rcall initUART

    ldi ZH, high(msgInicio<<1)
    ldi ZL, low(msgInicio<<1)
    rcall puts

main_loop:
    rcall getc
    cpi  r16, '1'
    breq caso_1
    cpi  r16, '2'
    breq caso_2
    cpi  r16, '3'
    breq caso_3
    cpi  r16, '4'
    breq caso_4
    cpi  r16, '5'
    breq caso_5
    cpi  r16, '6'
    breq caso_6
    cpi  r16, '7'
    breq caso_7
    rjmp caso_default

caso_1:
    ldi ZH, high(txtUno<<1)
    ldi ZL, low(txtUno<<1)
    rcall puts
    rjmp fin_switch

caso_2:
    ldi ZH, high(txtDos<<1)
    ldi ZL, low(txtDos<<1)
    rcall puts
    rjmp fin_switch

caso_3:
    ldi ZH, high(txtTres<<1)
    ldi ZL, low(txtTres<<1)
    rcall puts
    rjmp fin_switch

caso_4:
    ldi ZH, high(txtCuatro<<1)
    ldi ZL, low(txtCuatro<<1)
    rcall puts
    rjmp fin_switch

caso_5:
    ldi ZH, high(txtCinco<<1)
    ldi ZL, low(txtCinco<<1)
    rcall puts
    rjmp fin_switch

caso_6:
    ldi ZH, high(txtSeis<<1)
    ldi ZL, low(txtSeis<<1)
    rcall puts
    rjmp fin_switch

caso_7:
    ldi ZH, high(txtSiete<<1)
    ldi ZL, low(txtSiete<<1)
    rcall puts
    rjmp fin_switch

caso_default:
    
    rcall putc
    ldi  r16, 13
    rcall putc
    ldi  r16, 10
    rcall putc

fin_switch:
    rjmp main_loop


initUART:
    sts UBRR0L, r16
    sts UBRR0H, r17
    ldi r16, (1<<RXEN0)|(1<<TXEN0)
    sts UCSR0B, r16
    ldi r16, (1<<UCSZ01)|(1<<UCSZ00) 
    sts UCSR0C, r16
    ret

putc:
    lds r17, UCSR0A
    sbrs r17, UDRE0
    rjmp putc
    sts UDR0, r16
    ret

getc:
    lds r17, UCSR0A
    sbrs r17, RXC0
    rjmp getc
    lds r16, UDR0
    ret

puts:
    lpm r16, Z+
    cpi r16, 0
    breq fin_puts
    rcall putc
    rjmp puts
fin_puts:
    ret

.cseg
; Mensajes en el monitor Serial
msgInicio:
    .db "Bienvenido! Ingrese el numero para la accion que quiere realizar", 13, 10, \
        "1 - Mostrar el Mensaje", 13, 10, \
        "2 - Mostrar Cara Feliz", 13, 10, \
        "3 - Mostrar Cara Triste", 13, 10, \
        "4 - Mostrar Rombo", 13, 10, \
        "5 - Mostrar Corazon", 13, 10, \
        "6 - Mostrar Alien de Space Invaders", 13, 10, \
        "7 - Mostrar las 5 figuras con un intervalo", 13, 10, 0, 0   

txtUno:    .db "Has elegido UNO",      13, 10, 0          
txtDos:    .db "Has elegido DOS",      13, 10, 0          
txtTres:   .db "Has elegido TRES",     13, 10, 0, 0       
txtCuatro: .db "Has elegido CUATRO",   13, 10, 0, 0       
txtCinco:  .db "Has elegido CINCO",    13, 10, 0          
txtSeis:   .db "Has elegido SEIS",     13, 10, 0, 0       
txtSiete:  .db "Has elegido SIETE",  13, 10, 0
