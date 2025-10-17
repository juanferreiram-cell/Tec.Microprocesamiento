import cv2
import numpy as np
import argparse
from pathlib import Path

# Parámetros ajustables
ANCHO_OBJETIVO, ALTO_OBJETIVO = 180, 120

# Parámetros para suavizado
TAMANO_KER_GAUSS = (5, 5)  
SIGMA_GAUSS = 1.2

# Parámetros para morfología 
KER_CIERRE = (3, 3)
ITER_CIERRE = 2

# Simplificación de contornos (cuanto más pequeño, más curvado)
EPSILON_RELATIVO = 0.008   # 0.006–0.010 recomendado
AREA_MINIMA_CONTORNO = 120  # Para descartar ruidos pequeños

# Parámetros para re-muestreo del contorno
PASO_REMUESTREO = 1          # 1–2 recomendado para curvas suaves

# Incluir detalles internos (huecos dentro de la figura)
INCLUIR_HUECOS = True

# Si el salto entre dos puntos consecutivos es mayor que N píxeles, los partimos
SALTO_MAXIMO = 3   # 2–4 suele ir bien

# Mapeo de pines para controlar la lapicera
PIN_DERECHA = "PD7"
PIN_IZQUIERDA = "PD6"
PIN_SUBE = "PD5"  # y- (sube)
PIN_BAJA = "PD4"  # y+ (baja)
PIN_LAPICERA_SUBE = "PD3"
PIN_LAPICERA_BAJA = "PD2"

# Ajuste de tiempos para la lapicera
TIEMPO_CAMBIO_LAPICERA = 1       # delay corto al subir o bajar lapicera
EJE_PRIMERO_VIAJE = "x"    # Viaje entre contornos, elige "x" o "y"


def centrar_en_lienzo(imagen_b, ancho_objetivo, alto_objetivo):
    """Escala la imagen manteniendo la relación de aspecto y la centra en el lienzo objetivo."""
    h, w = imagen_b.shape
    escala = min(ancho_objetivo / w, alto_objetivo / h)
    nuevo_ancho, nuevo_alto = max(1, int(round(w * escala))), max(1, int(round(h * escala)))
    imagen_redimensionada = cv2.resize(imagen_b, (nuevo_ancho, nuevo_alto), interpolation=cv2.INTER_NEAREST)
    lienzo = np.zeros((alto_objetivo, ancho_objetivo), dtype=np.uint8)
    y0 = (alto_objetivo - nuevo_alto) // 2
    x0 = (ancho_objetivo - nuevo_ancho) // 2
    lienzo[y0:y0+nuevo_alto, x0:x0+nuevo_ancho] = imagen_redimensionada
    return lienzo


def simplificar_y_remuestrear(contorno, epsilon_relativo, paso_remuestreo):
    """Simplifica el contorno con approxPolyDP y re-muestrea a pasos según paso_remuestreo."""
    perimetro = cv2.arcLength(contorno, True)
    epsilon = max(1e-6, epsilon_relativo * perimetro)
    contorno_simplificado = cv2.approxPolyDP(contorno, epsilon, True).reshape(-1, 2)

    if len(contorno_simplificado) < 2:
        return []

    contorno_denso = []
    for i in range(len(contorno_simplificado)):
        x0, y0 = contorno_simplificado[i]
        x1, y1 = contorno_simplificado[(i + 1) % len(contorno_simplificado)]
        dx, dy = x1 - x0, y1 - y0
        distancia = int(max(1, round(np.hypot(dx, dy))))
        pasos = max(1, distancia // max(1, paso_remuestreo))
        for t in range(pasos):
            u = t / pasos
            x = int(round(x0 + u * dx))
            y = int(round(y0 + u * dy))
            if not contorno_denso or (x, y) != contorno_denso[-1]:
                contorno_denso.append((x, y))
    return contorno_denso


def dividir_saltos_largos(puntos, salto_maximo):
    """Divide los puntos en sub-rutas cuando los saltos entre puntos son grandes."""
    if not puntos:
        return []
    rutas = []
    ruta_actual = [puntos[0]]
    for p in puntos[1:]:
        if abs(p[0] - ruta_actual[-1][0]) > salto_maximo or abs(p[1] - ruta_actual[-1][1]) > salto_maximo:
            if len(ruta_actual) >= 2:
                rutas.append(ruta_actual)
            ruta_actual = [p]
        else:
            ruta_actual.append(p)
    if len(ruta_actual) >= 2:
        rutas.append(ruta_actual)
    return rutas


#  BRESENHAM / MOVIMIENTOS 
def _agregar_movimiento(movimientos, pin, cnt):
    """Agrega un movimiento de un pin (hacia arriba, abajo, etc.)."""
    if cnt <= 0:
        return
    if movimientos and movimientos[-1][0] == pin:
        movimientos[-1] = (pin, movimientos[-1][1] + cnt)
    else:
        movimientos.append((pin, cnt))


def _paso_unitario(dx, dy):
    """Devuelve el pin correspondiente al paso unitario según dx,dy."""
    if dx == 1:  return PIN_DERECHA
    if dx == -1: return PIN_IZQUIERDA
    if dy == 1:  return PIN_BAJA   # y+ hacia abajo
    if dy == -1: return PIN_SUBE
    return None


def segmento_bresenham(p0, p1):
    """Genera la secuencia de pasos unitarios (dx,dy) entre dos puntos usando el algoritmo de Bresenham."""
    x0, y0 = p0
    x1, y1 = p1
    dx = abs(x1 - x0)
    dy = -abs(y1 - y0)
    sx = 1 if x0 < x1 else -1 if x0 > x1 else 0
    sy = 1 if y0 < y1 else -1 if y0 > y1 else 0
    err = dx + dy

    pasos = []
    while True:
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        movido = False
        if e2 >= dy:
            err += dy
            x0 += sx
            pasos.append((sx, 0))
            movido = True
        if e2 <= dx:
            err += dx
            y0 += sy
            pasos.append((0, sy))
            movido = True
        if not movido:
            break
    return pasos


def puntos_a_movimientos_bresenham(puntos):
    """Convierte una lista de puntos en movimientos cardinales agrupados con el algoritmo de Bresenham."""
    movimientos = []
    if not puntos:
        return movimientos
    for i in range(1, len(puntos)):
        pasos_segmento = segmento_bresenham(puntos[i-1], puntos[i])
        pin_actual, contar = None, 0
        for (dx, dy) in pasos_segmento:
            pin = _paso_unitario(dx, dy)
            if pin is None:
                continue
            if pin == pin_actual:
                contar += 1
            else:
                if pin_actual is not None and contar > 0:
                    _agregar_movimiento(movimientos, pin_actual, contar)
                pin_actual, contar = pin, 1
        if pin_actual is not None and contar > 0:
            _agregar_movimiento(movimientos, pin_actual, contar)
    return movimientos


def viajes_entre_puntos(pos_actual, pos_objetivo, eje_primero="x"):
    """Genera los movimientos de viaje (pen-up) entre dos puntos sin diagonales."""
    if pos_actual is None:
        return [], pos_objetivo
    (x0, y0), (x1, y1) = pos_actual, pos_objetivo
    dx, dy = x1 - x0, y1 - y0
    secuencia = []
    def agregar(pin, cnt):
        if cnt <= 0: return
        if secuencia and secuencia[-1][0] == pin:
            secuencia[-1] = (pin, secuencia[-1][1] + cnt)
        else:
            secuencia.append((pin, cnt))
    if eje_primero == "x":
        if dx > 0:  agregar(PIN_DERECHA, dx)
        elif dx < 0: agregar(PIN_IZQUIERDA, -dx)
        if dy > 0:  agregar(PIN_BAJA,  dy)
        elif dy < 0: agregar(PIN_SUBE,   -dy)
    else:
        if dy > 0:  agregar(PIN_BAJA,  dy)
        elif dy < 0: agregar(PIN_SUBE,   -dy)
        if dx > 0:  agregar(PIN_DERECHA, dx)
        elif dx < 0: agregar(PIN_IZQUIERDA, -dx)
    return secuencia, pos_objetivo

def emitir_funcion_c(camino, ruta_salida):
    """Genera la función C 'Figura_Autogen' con los movimientos de dibujo."""
    with open(ruta_salida, "w", encoding="utf-8") as f:
        f.write("// Generado por img_to_plotter_cgen_autogen.py\n")
        f.write("// Requiere en tu firmware:\n")
        f.write("//   static inline void delay_ms_u16(uint16_t ms){ while(ms--) _delay_ms(1);} \n")
        f.write("//   #define AUTOGEN_SCALE_MS_PER_S 50\n")
        f.write("//   #define AUTOGEN_DELAY(s) delay_ms_u16((uint16_t)((s) * AUTOGEN_SCALE_MS_PER_S))\n")
        f.write("// Pines: PD2(bajar), PD3(subir), PD4(down), PD5(up), PD6(left), PD7(right)\n\n")
        f.write("static void Figura_Autogen(void) {\n")
        # Seguridad: lapicera arriba
        f.write(f"    PORTD = (1 << {PIN_LAPICERA_SUBE}); AUTOGEN_DELAY({TIEMPO_CAMBIO_LAPICERA});\n\n")

        pos_actual = None
        for camino_sub in camino:
            if not camino_sub:
                continue
            # Viaje al inicio del subtrazo (lapicera arriba)
            inicio = camino_sub[0]
            viaje, pos_actual = viajes_entre_puntos(pos_actual, inicio, eje_primero=EJE_PRIMERO_VIAJE)
            for pin, cnt in viaje:
                f.write(f"    PORTD = (1 << {pin}); AUTOGEN_DELAY({cnt});\n")

            # Lapicera abajo
            f.write(f"    PORTD = (1 << {PIN_LAPICERA_BAJA}); AUTOGEN_DELAY({TIEMPO_CAMBIO_LAPICERA});\n")

            # Movimientos de dibujo (Bresenham intercalados)
            movimientos = puntos_a_movimientos_bresenham(camino_sub)
            for pin, cnt in movimientos:
                f.write(f"    PORTD = (1 << {pin}); AUTOGEN_DELAY({cnt});\n")

            # Lapicera arriba al final del subtrazo
            f.write(f"    PORTD = (1 << {PIN_LAPICERA_SUBE}); AUTOGEN_DELAY({TIEMPO_CAMBIO_LAPICERA});\n\n")
            pos_actual = camino_sub[-1]

        # Asegurar lapicera arriba al final
        f.write(f"    PORTD = (1 << {PIN_LAPICERA_SUBE}); AUTOGEN_DELAY({TIEMPO_CAMBIO_LAPICERA});\n")
        f.write("}\n")

def main():
    ap = argparse.ArgumentParser(description="Convierte imagen a C para plotter (AUTOGEN_DELAY).")
    ap.add_argument("-i", "--entrada",  default="entrada.png", help="Imagen de entrada (png/jpg)")
    ap.add_argument("-o", "--salida", default="Figura_Autogen.c", help="Archivo .c de salida")
    args = ap.parse_args()

    ruta_entrada = Path(args.entrada)
    if not ruta_entrada.exists():
        raise FileNotFoundError(f"No se encontró {ruta_entrada}. Coloca 'entrada.png' junto al script o usa -i.")

    # Lectura y binarización
    imagen  = cv2.imread(str(ruta_entrada), cv2.IMREAD_COLOR)
    if imagen is None:
        raise RuntimeError(f"No pude leer la imagen: {ruta_entrada}")
    gris = cv2.cvtColor(imagen, cv2.COLOR_BGR2GRAY)
    gris = cv2.GaussianBlur(gris, TAMANO_KER_GAUSS, SIGMA_GAUSS)

    # Línea negra en blanco → invertimos al umbral para trabajar en "blanco"
    _, bw = cv2.threshold(gris, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

    # Morfología para cerrar huecos pequeños
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, KER_CIERRE)
    bw = cv2.morphologyEx(bw, cv2.MORPH_CLOSE, kernel, iterations=ITER_CIERRE)

    # Centrar/escala a canvas destino
    bw = centrar_en_lienzo(bw, ANCHO_OBJETIVO, ALTO_OBJETIVO)

    # Contornos (externos + internos)
    modo = cv2.RETR_TREE if INCLUIR_HUECOS else cv2.RETR_EXTERNAL
    contornos, jerarquia = cv2.findContours(bw, modo, cv2.CHAIN_APPROX_NONE)

    # Ordenar contornos por área (formas grandes primero)
    contornos = sorted(contornos, key=cv2.contourArea, reverse=True)

    rutas = []
    for cnt in contornos:
        if cv2.contourArea(cnt) < AREA_MINIMA_CONTORNO:
            continue
        pts = simplificar_y_remuestrear(cnt, EPSILON_RELATIVO, PASO_REMUESTREO)
        if not pts:
            continue
        # Dividir en subtrazos si hay saltos largos
        subrutas = dividir_saltos_largos(pts, SALTO_MAXIMO)
        for sp in subrutas:
            if sp:
                rutas.append(sp)

    if not rutas:
        raise RuntimeError("No se detectaron contornos útiles. Revisa umbral/morfología/parámetros.")

    emitir_funcion_c(rutas, args.salida)
    print(f"OK → Generado: {args.salida} con {len(rutas)} trazo(s).")

if __name__ == "__main__":
    main()
