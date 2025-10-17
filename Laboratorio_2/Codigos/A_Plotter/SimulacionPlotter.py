import argparse
import math
import re
from typing import List, Tuple, Optional

import matplotlib.pyplot as plt

# Mapeo de pines para el puerto PORTD (PD0 a PD7). Usamos solo un subconjunto.
PINS_PORTD = {
    'PD0': 0,
    'PD1': 1,
    'PD2': 2,  # lapicera baja
    'PD3': 3,  # lapicera arriba
    'PD4': 4,  # abajo ( -y )
    'PD5': 5,  # arriba ( +y )
    'PD6': 6,  # izquierda ( -x )
    'PD7': 7,  # derecha ( +x )
}

DELTA_MOVIMIENTO = {
    'PD7': ( 1,  0),  # derecha
    'PD6': (-1,  0),  # izquierda
    'PD5': ( 0,  1),  # arriba
    'PD4': ( 0, -1),  # abajo
}

# Expresión regular para capturar entradas como {(1 << PD6), 3} o con espacios/comentarios
ENTRADA_RE = re.compile(
    r"""\{\s*\(\s*1\s*<<\s*(PD[0-7])\s*\)\s*,\s*([0-9]+)\s*\}""",
    re.IGNORECASE
)

# También soporta máscaras OR compuestas entre llaves como {(1<<PD4)|(1<<PD7), 5}
ENTRADA_COMPUESTA_RE = re.compile(
    r"""\{\s*(?P<mask>[^,]+)\s*,\s*(?P<ticks>[0-9]+)\s*\}""",
    re.IGNORECASE
)

# Expresión regular para capturar un solo desplazamiento de bit
UN_SHIFT_PIN_RE = re.compile(r"""\(\s*1\s*<<\s*(PD[0-7])\s*\)""", re.IGNORECASE)


def parsear_entradas(texto: str) -> List[Tuple[List[str], int]]:
    """
    Analiza el texto de entradas y devuelve una lista de (pines, ticks).
    Los pines es una lista como ["PD6"] o ["PD4","PD7"] para diagonales.
    """
    # Eliminar comentarios estilo C/C++
    texto = re.sub(r"//.*?$", "", texto, flags=re.MULTILINE)
    texto = re.sub(r"/\*.*?\*/", "", texto, flags=re.DOTALL)

    entradas: List[Tuple[List[str], int]] = []
    for m in ENTRADA_COMPUESTA_RE.finditer(texto):
        expresion_mascara = m.group('mask')
        ticks = int(m.group('ticks'))
        pines = UN_SHIFT_PIN_RE.findall(expresion_mascara)
        pines = [p.upper() for p in pines]
        if not pines:
            # Intentar con la forma simple: {(1 << PDx), N}
            m2 = ENTRADA_RE.search(m.group(0))
            if m2:
                pines = [m2.group(1).upper()]
            else:
                continue
        entradas.append((pines, ticks))
    return entradas


def simular(entradas: List[Tuple[List[str], int]], inicio_xy=(0, 0), escala: float = 1.0):
    """
    Simula el movimiento y devuelve tres listas de líneas: caminos_dibujados, caminos_viajados, y el punto final.
    Cada camino es una lista de (x_lista, y_lista).
    """
    x, y = inicio_xy
    lapicera_abajo = False

    # Acumular puntos consecutivos para el segmento actual
    xs_actuales: List[float] = [x * escala]
    ys_actuales: List[float] = [y * escala]
    dibujar_actual = lapicera_abajo

    caminos_dibujados: List[Tuple[List[float], List[float]]] = []
    caminos_viajados: List[Tuple[List[float], List[float]]] = []

    def vaciar_segmento():
        nonlocal xs_actuales, ys_actuales, dibujar_actual, caminos_dibujados, caminos_viajados
        if len(xs_actuales) >= 2:
            if dibujar_actual:
                caminos_dibujados.append((xs_actuales, ys_actuales))
            else:
                caminos_viajados.append((xs_actuales, ys_actuales))
        # Empezar un nuevo segmento vacío (el llamante iniciará el punto de partida)
        xs_actuales = []
        ys_actuales = []

    longitud_total_dibujada = 0.0

    for pines, ticks in entradas:
        # Determinar el nuevo estado de la lapicera si se encuentra PD2/PD3 (nota: PORTD está completamente asignado por entrada)
        if 'PD2' in pines:
            # Lapicera baja (empezar a dibujar desde el siguiente movimiento)
            nuevo_estado_lapicera = True
        elif 'PD3' in pines:
            # Lapicera arriba
            nuevo_estado_lapicera = False
        else:
            nuevo_estado_lapicera = lapicera_abajo

        # Si el estado de la lapicera cambia, vaciar el segmento actual y empezar uno nuevo
        if nuevo_estado_lapicera != lapicera_abajo:
            vaciar_segmento()
            # Iniciar un nuevo segmento desde el punto actual con el estado actualizado de la lapicera
            lapicera_abajo = nuevo_estado_lapicera
            dibujar_actual = lapicera_abajo
            xs_actuales = [x * escala]
            ys_actuales = [y * escala]

        # Calcular el delta por tick desde los pines de movimiento
        dx = dy = 0
        for p in pines:
            if p in DELTA_MOVIMIENTO:
                ddx, ddy = DELTA_MOVIMIENTO[p]
                dx += ddx
                dy += ddy

        # Avanzar los ticks
        for _ in range(ticks):
            # Registrar desde el punto anterior al nuevo punto
            nuevo_x = x + dx
            nuevo_y = y + dy

            # Si estamos comenzando un nuevo segmento (sin puntos aún), inicializamos el punto actual
            if not xs_actuales:
                xs_actuales.append(x * escala)
                ys_actuales.append(y * escala)

            xs_actuales.append(nuevo_x * escala)
            ys_actuales.append(nuevo_y * escala)

            if lapicera_abajo:
                # Longitud de este pequeño paso (escalado)
                longitud_paso = math.hypot((nuevo_x - x) * escala, (nuevo_y - y) * escala)
                longitud_total_dibujada += longitud_paso

            x, y = nuevo_x, nuevo_y

    # Vaciar el último segmento
    vaciar_segmento()

    return caminos_dibujados, caminos_viajados, (x * escala, y * escala), longitud_total_dibujada


def renderizar_caminos(caminos_dibujados, caminos_viajados, salida_png: Optional[str] = None, salida_svg: Optional[str] = None, mostrar=False):
    fig, ax = plt.subplots(figsize=(6, 6))

    # Dibujar el viaje (lapicera arriba) como punteado
    for xs, ys in caminos_viajados:
        ax.plot(xs, ys, linestyle='--')

    # Dibujar el trazado con lapicera abajo como sólido
    for xs, ys in caminos_dibujados:
        ax.plot(xs, ys, linestyle='-')

    ax.set_aspect('equal', adjustable='box')
    ax.set_xlabel('X (pasos)')
    ax.set_ylabel('Y (pasos)')
    ax.grid(True)

    if salida_png:
        fig.savefig(salida_png, dpi=160, bbox_inches='tight')
    if salida_svg:
        fig.savefig(salida_svg, bbox_inches='tight')
    if mostrar:
        plt.show()
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser(description="Simula y grafica una secuencia de PORTD para un plotter AVR.")
    src = ap.add_mutualmente_exclusivos(required=True)
    src.add_argument('--input', type=str, help='Ruta al archivo de texto que contiene las entradas {(1<<PDx), N}.')
    src.add_argument('--text', type=str, help='Texto de las entradas pasado directamente en la línea de comando (cítalo).')
    ap.add_argument('--start', type=str, default='0,0', help='Coordenada de inicio "x,y" (por defecto 0,0).')
    ap.add_argument('--scale', type=float, default=1.0, help='Escala visual de los pasos (por defecto 1.0).')
    ap.add_argument('--out', type=str, default='plot.png', help='Ruta de salida PNG.')
    ap.add_argument('--svg', type=str, default=None, help='Ruta opcional de salida SVG.')
    ap.add_argument('--show', action='store_true', help='Mostrar la gráfica interactiva.')
    args = ap.parse_args()

    if args.input:
        with open(args.input, 'r', encoding='utf-8') as f:
            texto = f.read()
    else:
        texto = args.text

    try:
        x0, y0 = map(float, args.start.split(','))
    except Exception:
        raise SystemExit('Start inválido. Usa "x,y". Ejemplo: --start 0,0')

    entradas = parsear_entradas(texto)
    if not entradas:
        raise SystemExit('No se encontraron entradas válidas. Asegúrate de que las líneas se vean como: {(1 << PD6), 3}, y que los comentarios sean // o /* */')

    caminos_dibujados, caminos_viajados, (xf, yf), L = simular(entradas, inicio_xy=(x0, y0), escala=args.scale)

    print(f'Se leyeron {len(entradas)} entradas.')
    print(f'Coordenada final: ({xf:.3f}, {yf:.3f})')
    print(f'Longitud total dibujada: {L:.3f} (en unidades de pasos escalados)')
    renderizar_caminos(caminos_dibujados, caminos_viajados, salida_png=args.out, salida_svg=args.svg, mostrar=args.show)
    print(f'PNG guardado en: {args.out}')
    if args.svg:
        print(f'SVG guardado en: {args.svg}')


if __name__ == '__main__':
    main()
