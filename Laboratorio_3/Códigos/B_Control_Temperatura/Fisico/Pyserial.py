import serial
import time
import collections
import matplotlib.pyplot as plt
import threading

PORT = "COM3"
BAUD = 9600
NPTS = 300

# Rango del punto medio fijo - inicialmente 23 a 30
mid_range_min = 23.0
mid_range_max = 30.0
mid_range_avg = 26.5

def get_vals(line):
    try:
        if "Temperatura:" in line:
            temp_part = line.split("Temperatura:")[1].split("|")[0].strip()
            temp_str = temp_part.replace("C", "").replace("°", "").strip()
            temp = float(temp_str)
            
            fan_state = 0
            if "Ventilador: BAJO" in line:
                fan_state = 85
            elif "Ventilador: MEDIO" in line:
                fan_state = 170
            elif "Ventilador: ALTO" in line:
                fan_state = 255
            
            heater_state = 250 if "Calefactor: ENCENDIDO" in line else 0
            
            return temp, fan_state, heater_state
        return None
    except Exception as e:
        return None

tempq = collections.deque(maxlen=NPTS)
fanq = collections.deque(maxlen=NPTS)
heaterq = collections.deque(maxlen=NPTS)
data_lock = threading.Lock()
running = True
en_reconfig = False

def leer_serial(ser):
    global running, en_reconfig
    contador = 0
    
    while running:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode(errors="ignore").strip()
                if not line:
                    continue
                
                if en_reconfig:
                    print(line)
                    continue
                
                # Mostrar líneas importantes
                if any(x in line for x in ["==", "[CMD]", "OK.", "HEATER:", "MID", "LOW", "MED", "HIGH", "CTRL", "Comandos", "[Ayuda]"]):
                    print(line)
                    continue
                
                contador += 1
                if contador % 10 == 0:
                    print(line)
                
                vals = get_vals(line)
                if vals:
                    temp, fan_state, heater_state = vals
                    with data_lock:
                        tempq.append(temp)
                        fanq.append(fan_state)
                        heaterq.append(heater_state)
        except Exception as e:
            pass
        time.sleep(0.01)

def enviar_comandos(ser):
    global running, en_reconfig, mid_range_min, mid_range_max, mid_range_avg
    print("COMANDOS:")
    print("  E - Encender calefactor | A - Apagar calefactor")
    print("  R - Reconfigurar rango  | S - Salir")
    print(f"  Rango actual del punto medio: [{mid_range_min}, {mid_range_max}] -> Temperatura ideal: {mid_range_avg}°C")
    
    while running:
        try:
            cmd = input(">> ").strip().upper()
            
            if cmd == 'S':
                running = False
                print("Saliendo")
                break
                
            elif cmd == 'E':
                ser.write(b'E')
                print("Calefactor Encendido")
                
            elif cmd == 'A':
                ser.write(b'A')
                print("Calefactor Apagado")
                
            elif cmd == 'R':
                en_reconfig = True
                ser.write(b'R')
                time.sleep(0.3)
                
                # Esperar mensaje "Ingrese MIN"
                esperando_min = True
                timeout = 100
                while esperando_min and timeout > 0:
                    time.sleep(0.1)
                    timeout -= 1
                    if timeout == 50:
                        esperando_min = False
                
                # Usuario ingresa MIN
                min_val = input()
                ser.write((min_val + '\n').encode())
                time.sleep(0.3)
                
                # Esperar mensaje "Ingrese MAX"
                time.sleep(0.5)
                
                # Usuario ingresa MAX
                max_val = input()
                ser.write((max_val + '\n').encode())
                time.sleep(0.8)
                
                # Actualizar el rango del punto medio
                try:
                    mid_range_min = float(min_val)
                    mid_range_max = float(max_val)
                    mid_range_avg = (mid_range_min + mid_range_max) / 2.0
                    print(f"Nuevo rango del punto medio: [{mid_range_min}, {mid_range_max}]")
                    print(f"Temperatura ideal actualizada: {mid_range_avg}°C")
                except:
                    print("Error: Valores inválidos para el rango")
                
                en_reconfig = False
                print("Reconfiguración completada\n")
                
            else:
                print("Comando inválido")
                
        except EOFError:
            break
        except Exception as e:
            print(f"Error: {e}")
            en_reconfig = False

def main():
    global running, mid_range_min, mid_range_max, mid_range_avg
    
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        time.sleep(2)
        ser.reset_input_buffer()
        time.sleep(0.5)

        print("Conectado al puerto serial")
        print(f"Rango del punto medio inicial: [{mid_range_min}, {mid_range_max}]")
        print(f"Temperatura ideal inicial: {mid_range_avg}°C")

        # Threads
        thread_lectura = threading.Thread(target=leer_serial, args=(ser,), daemon=True)
        thread_comandos = threading.Thread(target=enviar_comandos, args=(ser,), daemon=True)
        
        thread_lectura.start()
        thread_comandos.start()

        # Gráficas
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
        
        # Gráfica de temperatura
        ln_temp, = ax1.plot([], [], 'r-', linewidth=2, marker='o', markersize=2, label='Temperatura')
        ln_ideal, = ax1.plot([], [], 'g--', linewidth=3, label=f'Temperatura ideal ({mid_range_avg}°C)')
        
        ax1.set_ylim(0, 60)
        ax1.set_xlim(0, NPTS)
        ax1.set_xlabel("Muestras")
        ax1.set_ylabel("Temperatura (°C)")
        ax1.grid(True, alpha=0.3)
        ax1.set_title("Temperatura en Tiempo Real", fontweight='bold')
        ax1.legend(loc='upper right')
        
        # Gráfica de estados
        ln_fan, = ax2.plot([], [], 'b-', label="Ventilador", linewidth=2, marker='s', markersize=2)
        ln_heater, = ax2.plot([], [], 'r-', label="Calefactor", linewidth=2, marker='^', markersize=2)
        ax2.set_ylim(-10, 270)
        ax2.set_xlim(0, NPTS)
        ax2.set_xlabel("Muestras")
        ax2.set_ylabel("Estado")
        ax2.legend(loc='upper right')
        ax2.grid(True, alpha=0.3)
        ax2.set_title("Estados del Sistema", fontweight='bold')
        
        plt.tight_layout()
        plt.ion()
        plt.show()

        print("Gráfica iniciada")

        while running:
            with data_lock:
                n = len(tempq)
                if n > 0:
                    x = list(range(n))
                    
                    ln_temp.set_data(x, list(tempq))
                    
                    ln_ideal.set_data([0, n], [mid_range_avg, mid_range_avg])
                    ln_ideal.set_label(f'Temperatura ideal ({mid_range_avg:.1f}°C)')
                    
                    ln_fan.set_data(x, list(fanq))
                    ln_heater.set_data(x, list(heaterq))
                    
                    ax1.set_xlim(0, max(NPTS, n))
                    ax2.set_xlim(0, max(NPTS, n))
                    
                    ax1.legend(loc='upper right')
            
            fig.canvas.draw_idle()
            fig.canvas.flush_events()
            time.sleep(0.05)
                
    except KeyboardInterrupt:
        print("\n[Interrumpido]")
    except serial.SerialException as e:
        print(f"[ERROR] Puerto {PORT}: {e}")
    except Exception as e:
        print(f"[ERROR] Inesperado: {e}")
    finally:
        running = False
        try:
            ser.close()
        except:
            pass
        plt.close('all')

if __name__ == "__main__":
    main()