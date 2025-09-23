; Codigo Problema B Matriz de Leds
; Autores: Lucas Elizalde, Juan Manuel Ferreira y Felipe Morrudo

.include "m328pdef.inc" 

.org 0x0000
    rjmp Inicio

.equ F_CPU = 16000000
.equ baud  = 9600
.equ bps   = (F_CPU/16/baud) - 1

.equ FILASB_OFF       = 0x3C   
.equ FILASC_OFF       = 0x0F   

; VELOCIDAD: numero de frames (barridos de 8 filas) por cada desplazamiento 
; de 1 columna en el scroll. Mayor valor = scroll ms lento.
.equ VELOCIDAD  = 10 
.equ NUMFIG     = 21     

; Instrucciones para la matriz LED
; Entrar al editor: [https://xantorohara.github.io/led-matrix-editor/]
; Dibujar la letra o figura que quieras.
; Copiar el patron hexadecimal que genera la pagina (ej.: 0x81C3A59981818181).
; Para usarlo en la matriz, separar en bytes y antepone 0x a CADA par de caracteres.
; Ejemplo: 0x81C3A59981818181 Å® 0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81
; Con eso ya se puede cargar y correr en la matriz.


M:         .db 0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81
E:         .db 0x7F, 0x01, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x7F
ESPACIO:   .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
G:         .db 0x7F, 0x01, 0x01, 0x01, 0x79, 0x41, 0x41, 0x7F
U:         .db 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7F
S:         .db 0x7F, 0x01, 0x01, 0x01, 0x7F, 0x40, 0x40, 0x7F
T:         .db 0x7F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
A:         .db 0x7E, 0x81, 0x81, 0xFF, 0x81, 0x81, 0x81, 0x81
ESPACIO1:  .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
C:         .db 0x7F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7F
O:         .db 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
M1:        .db 0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81
E2:        .db 0x7F, 0x01, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x7F
R:         .db 0x7F, 0x41, 0x41, 0x41, 0x7F, 0x11, 0x21, 0x41
ESPACIO2:  .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
A1:        .db 0x7E, 0x81, 0x81, 0xFF, 0x81, 0x81, 0x81, 0x81
S1:        .db 0x7F, 0x01, 0x01, 0x01, 0x7F, 0x40, 0x40, 0x7F
A2:        .db 0x7E, 0x81, 0x81, 0xFF, 0x81, 0x81, 0x81, 0x81
D:         .db 0x1F, 0x21, 0x41, 0x41, 0x41, 0x41, 0x21, 0x1F
O1:        .db 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
ESPACIO3:  .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

; Patrones de bits para cada figura
SONRISA: .db 0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C
TRISTE:  .db 0x3C,0x42,0xA5,0x81,0x99,0xA5,0x42,0x3C
CORAZON: .db 0x00,0x66,0xFF,0xFF,0xFF,0x7E,0x3C,0x18
ROMBO:   .db 0x18,0x3C,0x7E,0xFF,0xFF,0x7E,0x3C,0x18
ALIEN:   .db 0x3C,0x7E,0xDB,0xFF,0xFF,0x24,0x5A,0x81


; Tabla de punteros a caracteres para formar el mensaje "ME GUSTA COMER ASADO"
MENSAJE:
    .dw (M<<1),(E<<1),(ESPACIO<<1),(G<<1),(U<<1),(S<<1),(T<<1),(A<<1),(ESPACIO1<<1), \
        (C<<1),(O<<1),(M1<<1),(E2<<1),(R<<1),(ESPACIO2<<1),(A1<<1),(S1<<1),(A2<<1), \
        (D<<1),(O1<<1),(ESPACIO3<<1)


; Configuracion inicial del sistema: stack, UART, matriz y mensaje de bienvenida
Inicio:
    ; Configuracion del stack pointer
    ldi r16, HIGH(RAMEND)
    out SPH, r16
    ldi r16, LOW(RAMEND)
    out SPL, r16
    clr r1                  

    ; Inicializacon UART a 9600 baudios
    ldi r16, LOW(bps)
    ldi r17, HIGH(bps)
    rcall inicializarUART

    ; Configuracion de la matriz LED (GPIO + Timer0)
    rcall inicializarMatriz

    ; Mostrar menu de opciones por puerto serie
    ldi ZH, high(msgInicio<<1)
    ldi ZL, low(msgInicio<<1)
    rcall enviarCadena


; LOOP PRINCIPAL
; Bucle principal: lee comandos por UART y ejecuta la funci?n correspondiente
bucle_principal:
    rcall leerCaracter       ; espera un caracter en r16

    ; Evaluacion de opciones del menu
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
    rjmp caso_invalido

; CASOS DEL MENU PRINCIPAL

caso_1:
    ; Opcion 1: Mostrar mensaje scrolleando
    ldi ZH, high(txtUno<<1)
    ldi ZL, low(txtUno<<1)
    rcall enviarCadena
    rcall mostrarScrollMensaje
    rjmp fin_opcion

caso_2:
    ; Opcion 2: Mostrar cara feliz continuamente
    ldi ZH, high(txtDos<<1)
    ldi ZL, low(txtDos<<1)
    rcall enviarCadena
    ldi  ZL, low(SONRISA<<1)
    ldi  ZH, high(SONRISA<<1)
    rjmp mostrarSiempre

caso_3:
    ; Opcion 3: Mostrar cara triste continuamente
    ldi ZH, high(txtTres<<1)
    ldi ZL, low(txtTres<<1)
    rcall enviarCadena
    ldi  ZL, low(TRISTE<<1)
    ldi  ZH, high(TRISTE<<1)
    rjmp mostrarSiempre

caso_4:
    ; Opcion 4: Mostrar rombo continuamente
    ldi ZH, high(txtCuatro<<1)
    ldi ZL, low(txtCuatro<<1)
    rcall enviarCadena
    ldi  ZL, low(ROMBO<<1)
    ldi  ZH, high(ROMBO<<1)
    rjmp mostrarSiempre

caso_5:
    ; Opcion 5: Mostrar corazon continuamente
    ldi ZH, high(txtCinco<<1)
    ldi ZL, low(txtCinco<<1)
    rcall enviarCadena
    ldi  ZL, low(CORAZON<<1)
    ldi  ZH, high(CORAZON<<1)
    rjmp mostrarSiempre

caso_6:
    ; Opcion 6: Mostrar alien continuamente
    ldi ZH, high(txtSeis<<1)
    ldi ZL, low(txtSeis<<1)
    rcall enviarCadena
    ldi  ZL, low(ALIEN<<1)
    ldi  ZH, high(ALIEN<<1)
    rjmp mostrarSiempre

caso_7:
    ; Opcion 7: Secuencia animada de las 5 figuras
    ldi ZH, high(txtSiete<<1)
    ldi ZL, low(txtSiete<<1)
    rcall enviarCadena
    rjmp mostrarSecuenciaAnimada

caso_invalido:
    ; Echo del caracter no valido + nueva linea
    rcall enviarCaracter
    ldi  r16, 13
    rcall enviarCaracter
    ldi  r16, 10
    rcall enviarCaracter

fin_opcion:
    rjmp bucle_principal

; Inicializacion del UART
inicializarUART:
    sts UBRR0L, r16
    sts UBRR0H, r17
    ldi r16, (1<<RXEN0)|(1<<TXEN0)
    sts UCSR0B, r16
    ldi r16, (1<<UCSZ01)|(1<<UCSZ00)
    sts UCSR0C, r16
    ret

; Envio de un caracter: espera que el buffer esto libre y envia el byte en r16
enviarCaracter:
    lds r17, UCSR0A
    sbrs r17, UDRE0
    rjmp enviarCaracter
    sts UDR0, r16
    ret

; Lectura de un caracter: espera hasta recibir un byte y lo retorna en r16
leerCaracter:
    lds r17, UCSR0A
    sbrs r17, RXC0
    rjmp leerCaracter
    lds r16, UDR0
    ret

; Envio de cadena: transmite una cadena terminada en 0 desde la memoria Flash
enviarCadena:
    lpm r16, Z+
    cpi r16, 0
    breq fin_cadena
    rcall enviarCaracter
    rjmp enviarCadena
fin_cadena:
    ret

; Mensajes en el Monitor Serial
.cseg
msgInicio:
    .db "Bienvenido! Ingrese el numero para la accion que quiere realizar", 13, 10, \
        "1 - Mostrar el Mensaje", 13, 10, \
        "2 - Mostrar Cara Feliz", 13, 10, \
        "3 - Mostrar Cara Triste", 13, 10, \
        "4 - Mostrar Rombo", 13, 10, \
        "5 - Mostrar Corazon", 13, 10, \
        "6 - Mostrar Alien de Space Invaders", 13, 10, \
        "7 - Mostrar las 5 figuras cada 1 segundo", 13, 10, 0, 0   

txtUno:    .db "Has elegido MENSAJE",      13, 10, 0          
txtDos:    .db "Has elegido CARA FELIZ",      13, 10, 0, 0        
txtTres:   .db "Has elegido CARA TRISTE",     13, 10, 0, 0, 0       
txtCuatro: .db "Has elegido ROMBO",   13, 10, 0, 0, 0       
txtCinco:  .db "Has elegido CORAZON",    13, 10, 0          
txtSeis:   .db "Has elegido ALIEN DE SPACE INVADERS",     13, 10, 0, 0, 0       
txtSiete:  .db "Has elegido VER LAS 5 FIGURAS",    13, 10, 0


; Configuracion de pines GPIO para matriz 8x8 y Timer0 para temporizacion
inicializarMatriz:
    ; Configuracion de pines como salida (sin afectar PD0/PD1 del UART)
    in   r16, DDRD
    ori  r16, 0b11111100     
    out  DDRD, r16
    ldi  r16, 0b00111111      
    out  DDRB, r16
    in   r16, DDRC
    ori  r16, 0b00001111
    out  DDRC, r16

    ; Estado inicial: columnas en LOW, filas en HIGH (matriz apagada)
    in   r16, PORTD
    andi r16, 0b00000011      
    out  PORTD, r16
    in   r16, PORTB
    andi r16, 0b11000000      
    ori  r16, FILASB_OFF      
    out  PORTB, r16
    in   r16, PORTC
    andi r16, 0b11110000      
    ori  r16, FILASC_OFF     
    out  PORTC, r16

    ; Configuracion Timer0 en modo CTC para generar base de tiempo de ~1ms
    ldi  r16, (1<<WGM01)      
    out  TCCR0A, r16
    ldi  r16, 249            
    out  OCR0A, r16
    ldi  r16, (1<<CS01)|(1<<CS00)
    out  TCCR0B, r16
    ret


; Secuencia de 5 figuras mostradas 3 segundos cada una (una sola pasada)
mostrarSecuenciaCompleta:
    push r0
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r24
    push r25
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31

    ; Mostrar cada figura durante aproximadamente 3 segundos
    ldi  ZL, low(SONRISA<<1)
    ldi  ZH, high(SONRISA<<1)
    rcall mostrar3Segundos

    ldi  ZL, low(TRISTE<<1)
    ldi  ZH, high(TRISTE<<1)
    rcall mostrar3Segundos

    ldi  ZL, low(CORAZON<<1)
    ldi  ZH, high(CORAZON<<1)
    rcall mostrar3Segundos

    ldi  ZL, low(ROMBO<<1)
    ldi  ZH, high(ROMBO<<1)
    rcall mostrar3Segundos

    ldi  ZL, low(ALIEN<<1)
    ldi  ZH, high(ALIEN<<1)
    rcall mostrar3Segundos

    ; Restaurar registros
    pop  r31
    pop  r30
    pop  r29
    pop  r28
    pop  r27
    pop  r26
    pop  r25
    pop  r24
    pop  r22
    pop  r21
    pop  r20
    pop  r19
    pop  r18
    pop  r17
    pop  r16
    pop  r0
    ret

; Secuencia infinita de las 5 figuras (interrumpible por UART)
mostrarSecuenciaAnimada:
bucleSecuencia:
    ; Verificar si llego algun comando antes de cada figura
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla7a
    rjmp bucle_principal
noTecla7a:
    ldi  ZL, low(SONRISA<<1)
    ldi  ZH, high(SONRISA<<1)
    rcall mostrar3Segundos

    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla7b
    rjmp bucle_principal
noTecla7b:
    ldi  ZL, low(TRISTE<<1)
    ldi  ZH, high(TRISTE<<1)
    rcall mostrar3Segundos

    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla7c
    rjmp bucle_principal
noTecla7c:
    ldi  ZL, low(CORAZON<<1)
    ldi  ZH, high(CORAZON<<1)
    rcall mostrar3Segundos

    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla7d
    rjmp bucle_principal
noTecla7d:
    ldi  ZL, low(ROMBO<<1)
    ldi  ZH, high(ROMBO<<1)
    rcall mostrar3Segundos

    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla7e
    rjmp bucle_principal
noTecla7e:
    ldi  ZL, low(ALIEN<<1)
    ldi  ZH, high(ALIEN<<1)
    rcall mostrar3Segundos

    rjmp bucleSecuencia

; Mostrar figura durante aproximadamente 3 segundos con escaneo continuo
mostrar3Segundos:
   movw r26, r30             
    ldi  r25, 0x03            
    ldi  r24, 0xE8           
    clr  r22                 
bucle3s:
    rcall apagarFilas
    lpm  r20, Z+              
    rcall configurarColumnas
    mov  r21, r22           
    rcall activarFila
    rcall esperar1ms

    ; Verificar si llego comando por UART
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTecla3s
    rjmp bucle_principal
noTecla3s:
    inc  r22                 
    cpi  r22, 8
    brlo filaValida3s
    clr  r22                 
    movw r30, r26            
filaValida3s:
    sbiw r24, 1               
    brne bucle3s
    ret

; Display continuo de una figura (interrumpible por UART)
mostrarSiempre:
    movw r26, r30             
    clr  r22                  
bucleInfinito:
    rcall apagarFilas
    movw r30, r26             
    mov  r18, r22             
    add  ZL, r18
    adc  ZH, r1
    lpm  r20, Z              
    rcall configurarColumnas
    mov  r21, r22            
    rcall activarFila
    rcall esperar1ms

    ; Verificar si llego comando por UART
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noTeclaSiempre
    rjmp bucle_principal
noTeclaSiempre:
   
    inc  r22                
    cpi  r22, 8
    brlo bucleInfinito

    clr  r22                
    rjmp bucleInfinito



; Apagar todas las filas (poner en estado HIGH)
apagarFilas:
    in   r16, PORTB
    ori  r16, FILASB_OFF      ; filas B en HIGH
    out  PORTB, r16
    in   r16, PORTC
    ori  r16, FILASC_OFF      ; filas C en HIGH
    out  PORTC, r16
    ret

; Configurar el patron de columnas segun el valor en r20
configurarColumnas:
    ; Configurar columnas C1 a C6 en PD2 a PD7
    mov  r16, r20
    andi r16, 0x3F            
    lsl  r16               
    lsl  r16
    in   r17, PORTD
    andi r17, 0x03            ; preservar PD0,PD1 (UART)
    or   r17, r16
    out  PORTD, r17
    
    ; Configurar columnas C7 a C8 en PB0 a PB1
    mov  r16, r20
    andi r16, 0xC0          
    lsr  r16                 
    lsr  r16
    lsr  r16
    lsr  r16
    lsr  r16
    lsr  r16
    in   r17, PORTB
    andi r17, 0b11111100      
    or   r17, r16
    out  PORTB, r17
    ret

; Activar unicamente la fila especificada en r21 (poner en LOW)
activarFila:
    push r18
    push r19
    push r20
    
    cpi  r21, 4
    brlt filaEnPortoB

    subi r21, 4               
    ldi  r18, FILASC_OFF      
    ldi  r19, 0x01            
    mov  r20, r21
desplazarPC:
    tst  r20
    breq finDesplazarPC
    lsl  r19                 
    dec  r20
    rjmp desplazarPC
finDesplazarPC:
    com  r19                  
    and  r18, r19
    in   r17, PORTC
    andi r17, 0b11110000     
    or   r17, r18
    out  PORTC, r17
    rjmp finFila

filaEnPortoB:
    ; Filas F1 a F4 estan en PORTB (PB2 a PB5)
    ldi  r18, FILASB_OFF      
    ldi  r19, 0x04            
    mov  r20, r21
desplazarPB:
    tst  r20
    breq finDesplazarPB
    lsl  r19                  
    dec  r20
    rjmp desplazarPB
finDesplazarPB:
    com  r19                
    and  r18, r19
    in   r17, PORTB
    andi r17, 0b11000011     
    or   r17, r18
    out  PORTB, r17

finFila:
    pop  r20
    pop  r19
    pop  r18
    ret

; Generar retardo de 1ms usando Timer0 en modo CTC
esperar1ms:
    ldi  r16, (1<<OCF0A)     
    out  TIFR0, r16
esperarTick:
    in   r17, TIFR0
    sbrs r17, OCF0A          
    rjmp esperarTick
    ret


; FUNCION DE SCROLL DE TEXTO

; Algoritmo de scroll horizontal para mostrar el mensaje completo
; Combina dos letras consecutivas desplazandolas bit a bit
mostrarScrollMensaje:
    ; Preservar todos los registros utilizados
    push r0
    push r16
    push r17
    push r18
    push r19        
    push r20
    push r21
    push r22
    push r23        
    push r24        
    push r25
    push r26       
    push r27
    push r28        
    push r29
    push r30        
    push r31

    ; Estado inicial del algoritmo de scroll
    clr  r19                  ; comenzar con primera figura
    rcall cargarFiguraActualYSiguiente
    ldi  r23, 0               
    ldi  r24, VELOCIDAD       ; frames hasta proximo paso
    clr  r22                  

bucleScroll:
    ; Apagar todas las filas antes de actualizar
    rcall apagarFilas
    
    ; Obtener byte de la figura actual
    movw r30, r26            
    mov  r18, r22             
    add  ZL, r18
    adc  ZH, r1
    lpm  r20, Z              
    
    ; Obtener byte correspondiente de la figura siguiente
    movw r30, r28             
    mov  r18, r22
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z               

    ; Aplicar desplazamiento a la figura actual (hacia la derecha)
    mov  r18, r23
    cpi  r18, 8
    breq soloFiguraSiguiente

    tst  r18
    breq figuraActualLista
bucleDesplazarActual:
    lsr  r20                 
    dec  r18
    brne bucleDesplazarActual
figuraActualLista:

    ; Aplicar desplazamiento a la figura siguiente (hacia la izquierda)
    ldi  r25, 8
    sub  r25, r23            
    tst  r25
    breq siguienteLista
bucleDesplazarSiguiente:
    lsl  r0                  
    dec  r25
    brne bucleDesplazarSiguiente
siguienteLista:

    lsl  r0                   
    or   r20, r0              
    rjmp composicionTerminada

soloFiguraSiguiente:
    mov  r20, r0
    lsl  r20                 

composicionTerminada:
    ; Mostrar la fila compuesta en la matriz
    rcall configurarColumnas
    mov  r21, r22            
    rcall activarFila
    rcall esperar1ms

    ; Verificar si llego comando de interrupcion
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noInterrupcionScroll
    rjmp finalizarScroll
noInterrupcionScroll:

    ; Avanzar a la siguiente fila
    inc  r22
    cpi  r22, 8
    brlo bucleScroll

    ; Se completo un frame (todas las 8 filas)
    clr  r22                  
    dec  r24                  

    ; Verificar interrupcion tambien al final del frame
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp noInterrupcionFrame
    rjmp finalizarScroll
noInterrupcionFrame:

    brne bucleScroll          

    ; Momento de avanzar un paso en el scroll
    ldi  r24, VELOCIDAD       
    inc  r23                  
    cpi  r23, 9               
    brlo bucleScroll

    ; Termino el desplazamiento de la letra actual
    clr  r23                  
    inc  r19                  
    cpi  r19, NUMFIG          
    brlo siguienteFiguraOk
    clr  r19                 
siguienteFiguraOk:
    rcall cargarFiguraActualYSiguiente
    rjmp bucleScroll

; Salida limpia del scroll restaurando todos los registros
finalizarScroll:
    pop  r31
    pop  r30
    pop  r29
    pop  r28
    pop  r27
    pop  r26
    pop  r25
    pop  r24
    pop  r23
    pop  r22
    pop  r21
    pop  r20
    pop  r19
    pop  r18
    pop  r17
    pop  r16
    pop  r0
    ret

; Cargar punteros a la figura actual (X) y siguiente (Y) segun indice en r19
cargarFiguraActualYSiguiente:
    ; Cargar puntero a figura actual
    ldi  ZL, low(MENSAJE<<1)
    ldi  ZH, high(MENSAJE<<1)
    mov  r18, r19             
    lsl  r18                  
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z+              
    mov  r26, r0
    lpm  r0, Z                
    mov  r27, r0

    ; Calcular indice de figura siguiente (con wrap-around)
    mov  r18, r19
    inc  r18
    cpi  r18, NUMFIG
    brlo indiceValido
    clr  r18                  

indiceValido:
    ; Cargar puntero a figura siguiente
    ldi  ZL, low(MENSAJE<<1)
    ldi  ZH, high(MENSAJE<<1)
    lsl  r18                 
    add  ZL, r18
    adc  ZH, r1
    lpm  r0, Z+              
    mov  r28, r0
    lpm  r0, Z               
    mov  r29, r0
    ret