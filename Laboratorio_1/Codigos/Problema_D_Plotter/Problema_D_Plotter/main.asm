.include "m328pdef.inc"

.org 0x0000
    rjmp Inicio

.equ F_CPU = 16000000
.equ baud  = 9600
.equ bps   = (F_CPU/16/baud) - 1


Inicio:
    ; Inicar stack
    ldi r16, HIGH(RAMEND)
    out SPH, r16
    ldi r16, LOW(RAMEND)
    out SPL, r16

    ; UART 9600
    ldi r16, LOW(bps)
    ldi r17, HIGH(bps)
    rcall iniciarUART

	; Definicion de los Pines PD como salida
    ldi     r20, (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)
    out     DDRD, r20                
    clr     r20
    out     PORTD, r20  
	
	    
	ldi     r21, 0
    sts     TCCR1A, r21               
    ldi     r21, (1<<CS12)           
    sts     TCCR1B, r21
    ldi     r21, 0
    sts     TCCR1C, r21               
    ldi     r21, 0
    sts     TIMSK1, r21               

        
    ldi     r17, HIGH(62932)
    sts     TCNT1H, r21
    ldi     r17, LOW(62932)
    sts     TCNT1L, r21
 
    ; Mensaje de bienvenida + Menu
    ldi ZH, high(msgInicio<<1)
    ldi ZL, low(msgInicio<<1)
    rcall enviarCadenaFlash

main_loop:
    rcall recibirCaracter               ; lee tecla en r16

    ; Switch: '1','2','3','T'/'t'
    cpi  r16, '1'
    breq caso_triangulo
    cpi  r16, '2'
    breq caso_circulo
    cpi  r16, '3'
    breq caso_cruz
    cpi  r16, 'T'
    breq caso_todas
    cpi  r16, 't'
    breq caso_todas

    rjmp caso_default

; Casos
caso_triangulo:
    ; Texto para Triangulo
    ldi ZH, high(txtTriangulo<<1)
    ldi ZL, low(txtTriangulo<<1)
    rcall enviarCadenaFlash
    ; Dibuja Triangulo
    rcall figura_triangulo
    rjmp fin_switch

caso_circulo:
	; Texto en USART para Circulo
    ldi ZH, high(txtCirculo<<1)
    ldi ZL, low(txtCirculo<<1)
    rcall enviarCadenaFlash ; envia el caracter correcto para la USART
	; Dibuja Circulo
    rcall figura_circulo
    rjmp fin_switch

caso_cruz:
	; Texto en USART para Cruz
    ldi ZH, high(txtCruz<<1)
    ldi ZL, low(txtCruz<<1)
    rcall enviarCadenaFlash
	; Dibuja Cruz
    rcall figura_cruz
    rjmp fin_switch

caso_todas:
	; Texto para todas las Figuras
    ldi ZH, high(txtTodas<<1)
    ldi ZL, low(txtTodas<<1)
    rcall enviarCadenaFlash 
	; Dibuja las 3 figuras
    rcall figuras_todas
	rjmp fin_switch

caso_default:
    rcall enviarCaracter          ; envia el caracter inv?lido en r16
    ldi  r16, 13
    rcall enviarCaracter
    ldi  r16, 10
    rcall enviarCaracter
	; Imprime la opcion invalida
    ldi ZH, high(msgOpcionInvalida<<1)
    ldi ZL, low(msgOpcionInvalida<<1)
    rcall enviarCadenaFlash
    rjmp fin_switch

fin_switch:
    rjmp main_loop


;r16 = dato a TX en enviarCaracter
;r16 <= dato RX en recibirCaracter

; Deja listo el puerto serie para poder enviar y recibir.
iniciarUART:
    sts UBRR0L, r16
    sts UBRR0H, r17
    ldi r16, (1<<RXEN0)|(1<<TXEN0)
    sts UCSR0B, r16
    ldi r16, (1<<UCSZ01)|(1<<UCSZ00)   
    sts UCSR0C, r16
    ret

; Env?a un byte por el puerto serie (espera hasta que se pueda y lo manda).
enviarCaracter:
    lds r17, UCSR0A
    sbrs r17, UDRE0
    rjmp enviarCaracter
    sts UDR0, r16
    ret

; Lee un byte que lleg? por el puerto serie (espera a que llegue y lo toma).
recibirCaracter:
    lds r17, UCSR0A
    sbrs r17, RXC0
    rjmp recibirCaracter
    lds r16, UDR0
    ret

; Env?a un texto guardado en la memoria de programa hasta encontrar el 0 final.
enviarCadenaFlash:
    lpm r16, Z+
    cpi r16, 0
    breq fin_enviarCadenaFlash
    rcall enviarCaracter
    rjmp enviarCadenaFlash
fin_enviarCadenaFlash:
    ret

figuras_todas:
		; Ubicacion del Selenoide para Dibujar
        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_20S_LARGO

		ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

		ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

		;TRIANGULO
		; Baja el selenoide
        ldi     r20, (1 <<PD2) 
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

		; Realizar el Dibujo del Triangulo
        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD6) | (1<<PD5)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

		; LEVANTA SELENOIDE
        ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_1S

		; SE MUEVE HACIA LA IZQUIERDA

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO
		

       ; BAJA SELENOIDE
        ldi     r20, (1<<PD2)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

       ;CIRCULO
        ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S
		
		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S
		
		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S
		
        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S
		
        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_12S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_12S

		;SUBE SELENOIDE
        ldi     r20, (1 <<PD3)
        out     PORTD, r20
        rcall   DELAY_1S

		; Me mueve para realizar la Cruz
		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO
		
       ; BAJA SELENOIDE
        ldi     r20, (1 <<PD2)
        out     PORTD, r20
        rcall   DELAY_1S


		; CRUZ
        ldi     r20, (1<<PD4) | (1<<PD7)
        out     PORTD, r20 
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD5) 
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD2)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD4) | (1<<PD6)
        out     PORTD, r20 
        rcall   DELAY_5S_LARGO

		ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_1S

        ret

figura_triangulo:
    ; POSICIONAMIENTO
        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_20S_LARGO

		ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

		
	;TRIANGULO
        ldi     r20, (1 <<PD2) 
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD6) | (1<<PD5)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

		; VUELVE AL ORIGEN
        ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_20S_LARGO

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_10S_LARGO

        ldi     r20, (1<<PD5)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO
    
    ret

figura_circulo:
    ;POSICION AL INICIO
        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_20S_LARGO

        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

       ; BAJA SELENOIDE
        ldi     r20, (1<<PD2)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

       ;CIRCULO
        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S
		
		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S
		
		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_4S

		ldi     r20, (1 <<PD4)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_12S

		ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S
		
        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S
		
        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD7)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_12S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_2S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

        ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_4S

        ldi     r20, (1 <<PD5)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1 <<PD6)
        out     PORTD, r20
        rcall   DELAY_12S
		
        ;SUBE SELENOIDE
        ldi     r20, (1 <<PD3)
        out     PORTD, r20
        rcall   DELAY_1S

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_20S_LARGO

        ldi     r20, (1<<PD5)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

    ret



figura_cruz:
     ; POSICIONAMIENTO
        ldi     r20, (1<<PD7)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD4)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO
		
		; CRUZ
        ldi     r20, (1<<PD2)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD4) | (1<<PD7)
        out     PORTD, r20 
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD5) 
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD2)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO

        ldi     r20, (1<<PD4) | (1<<PD6)
        out     PORTD, r20 
        rcall   DELAY_5S_LARGO

		; VUELVE AL ORIGEN
        ldi     r20, (1<<PD3)
        out     PORTD, r20
        rcall   DELAY_1S_LARGO

		ldi     r20, (1<<PD5) 
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

		ldi     r20, (1<<PD6)
        out     PORTD, r20
        rcall   DELAY_5S_LARGO

        ldi     r20, (1<<PD5)
        out     PORTD, r20
        rcall   DELAY_2S_LARGO
    ret



; Mensaje en USART
.cseg
.cseg
.cseg
msgInicio:
    .db "Bienvenido! Seleccione la figura a dibujar:", 13,10, \
        "1 - Triangulo", 13,10, \
        "2 - Circulo", 13,10, \
        "3 - Cruz", 13,10, \
        "T - Todas (1->2->3)", 13,10, 0, 0

msgOpcionInvalida:
    .db "Opcion invalida. Use 1, 2, 3 o T.", 13,10, 0

txtTriangulo: .db "Dibujando TRIANGULO...", 13,10, 0, 0
txtCirculo:   .db "Dibujando CIRCULO...",   13,10, 0, 0
txtCruz:      .db "Dibujando CRUZ...",      13,10, 0
txtTodas:     .db "Dibujando TODAS las figuras...", 13,10, 0, 0

; Se definen 2 tipos de delays los cortos y los largos que son los que contienen _LARGO
; Si bien se definen pines que no se usan, se dejaron por si se quiere modificar el tama?o de las figuras poniendo esos delays
DELAY_1S:
        ldi     r18, 1
D1_LOOP:
               ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D1_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D1_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D1_LOOP
        ret

DELAY_1S_LARGO:
        ldi     r18, 1
D1_LARGO_LOOP:
               ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D1_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D1_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D1_LOOP
        ret

DELAY_2S:
        ldi     r18, 2
D2_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D2_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D2_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D2_LOOP
        ret

DELAY_3S:
        ldi     r18, 3
D3_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D3_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D3_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D3_LOOP
        ret

DELAY_5S:
        ldi     r18, 5
D5_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D5_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D5_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D5_LOOP
        ret

DELAY_6S:
        ldi     r18, 6
D6_LOOP:
               ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D6_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D6_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D6_LOOP
        ret

DELAY_7S:
        ldi     r18, 7
D7_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D7_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D7_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D7_LOOP
        ret

DELAY_10S:
        ldi     r18, 10
D10_LOOP:
               ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D10_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D10_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D10_LOOP
        ret

DELAY_4S:
        ldi     r18, 4
D4S_LOOP:
                ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D4S_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D4S_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D4S_LOOP
        ret

DELAY_12S:
        ldi     r18, 12
D12S_LOOP:
        ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D12S_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D12S_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D12S_LOOP
        ret

DELAY_15S:
        ldi     r18, 15
D15_LOOP:
        ldi     r16, HIGH(62932)
        sts     TCNT1H, r16
        ldi     r16, LOW(62932)
        sts     TCNT1L, r16

        sts     TCNT1L, r16
D15_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D15_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D15_LOOP
        ret

DELAY_2S_LARGO:
        ldi     r18, 24
D2_LARGO_LOOP:
        ldi     r16, HIGH(9286)          
        sts     TCNT1H, r16
        ldi     r16, LOW(9286)
        sts     TCNT1L, r16
        sts     TCNT1L, r16
D2_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D2_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D2_LARGO_LOOP
        ret

DELAY_5S_LARGO:
        ldi     r18, 60                   
D5_LARGO_LOOP:
       
        ldi     r16, HIGH(9286)          
        sts     TCNT1H, r16
        ldi     r16, LOW(9286)
        sts     TCNT1L, r16
D5_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D5_WAIT
        sbi     TIFR1, TOV1               
        dec     r18
        brne    D5_LARGO_LOOP
        ret


DELAY_15S_LARGO:
        ldi     r18, 180
D15_LARGO_LOOP:
        ldi     r16, HIGH(9286)          
        sts     TCNT1H, r16
        ldi     r16, LOW(9286)
        sts     TCNT1L, r16
D15_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D15_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D15_LARGO_LOOP
        ret

DELAY_10S_LARGO:
        ldi     r18, 90
D10_LARGO_LOOP:
        ldi     r16, HIGH(9286)          
        sts     TCNT1H, r16
        ldi     r16, LOW(9286)
        sts     TCNT1L, r16
D10_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D15_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D10_LARGO_LOOP
        ret

DELAY_20S_LARGO:
        ldi     r18, 255
D20_LARGO_LOOP:
        ldi     r16, HIGH(9286)          
        sts     TCNT1H, r16
        ldi     r16, LOW(9286)
        sts     TCNT1L, r16
D20_LARGO_WAIT:
        sbis    TIFR1, TOV1
        rjmp    D15_WAIT
        sbi     TIFR1, TOV1
        dec     r18
        brne    D20_LARGO_LOOP
        ret