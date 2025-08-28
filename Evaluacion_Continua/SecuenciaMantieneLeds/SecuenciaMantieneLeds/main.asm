.include "m328pdef.inc"

.cseg 
.org 0x0000
rjmp reset

reset:

ldi r16, (1 << PD0)   ; Defino el pin D0 como salida
out DDRD, r16             
ldi r17, (1 << PD1)   ; Defino el pin D1 como salida   
out DDRD, r17             
ldi r18, (1 << PD2)   ; Defino el pin D3 como salida   
out DDRD, r18             
ldi r19, (1 << PD3)   ; Defino el pin D4 como salida   
out DDRD, r19             
ldi r20, (1 << PD4)   ; Defino el pin D5 como salida   
out DDRD, r20             
ldi r21, (1 << PD5)   ; Defino el pin D6 como salida   
out DDRD, r21             
ldi r22, (1 << PD6)   ; Defino el pin D7 como salida 
out DDRD, r22          
ldi r23, (1 << PD7)   ; Defino el pin D8 como salida  
out DDRD, r23         

loop:
    ; Prende el LED 0 
    ldi r16, (1 << PD0)     
    out PORTD, r16           
    call delay         

	; Prende el LED 1 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1)   
    out PORTD, r16                
    call delay                     

    ; Prende el LED 2 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2)
    out PORTD, r16
    call delay

    ; Prende el LED 3 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3)
    out PORTD, r16
    call delay

    ; Prende el LED 4 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4)
    out PORTD, r16
    call delay

    ; Prende el LED 5 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5)
    out PORTD, r16
    call delay

    ; Prende el LED 6 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6)
    out PORTD, r16
    call delay

    ; Prende el LED 7 y deja los anteriores prendidos
    ldi r16, (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7)
    out PORTD, r16
    call delay

	rjmp loop 


