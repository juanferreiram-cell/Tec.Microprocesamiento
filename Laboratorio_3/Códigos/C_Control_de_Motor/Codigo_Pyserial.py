import serial, time, collections
import matplotlib.pyplot as plt

PORT = "COM3"
BAUD = 9600
NPTS = 300

def get_vals(line):
    try:
        ref,eje,pwm,sentido = line.split(",")
        return int(ref), int(eje), int(pwm), sentido
    except:
        return None

with serial.Serial(PORT, BAUD, timeout=1) as ser:
    time.sleep(2)
    ser.reset_input_buffer()

    hdr = ser.readline().decode(errors="ignore").strip()
    print(f"Encabezado: {hdr}")
    print("-" * 50)

    xs = collections.deque(maxlen=NPTS)
    refq = collections.deque(maxlen=NPTS)
    ejeq = collections.deque(maxlen=NPTS)
    pwmq = collections.deque(maxlen=NPTS)

    plt.ion()
    fig = plt.figure()
    ax1 = plt.gca()

    ln_ref, = ax1.plot([], [], label="Potenciometro", color='blue', linewidth=1)
    ln_eje, = ax1.plot([], [], label="Potenciometro Motor", color='green', linewidth=1)
    ln_pwm, = ax1.plot([], [], label="PWM", color='red', linewidth=1)
    ax1.set_ylim(-50, 1075)
    ax1.set_xlabel("muestras")
    ax1.set_ylabel("valor")
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    plt.title("Control de motor")

    contador = 0

    while True:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue
        
        contador += 1
        if contador % 10 == 0:
            print(line)
        
        if line.startswith("ref"):
            continue
            
        vals = get_vals(line)
        if not vals: 
            continue
        ref, eje, pwm, sentido = vals

        xs.append(len(xs))
        refq.append(ref)
        ejeq.append(eje)
        pwmq.append(pwm)

        ln_ref.set_data(range(len(refq)), list(refq))
        ln_eje.set_data(range(len(ejeq)), list(ejeq))
        ln_pwm.set_data(range(len(pwmq)), list(pwmq))
        ax1.set_xlim(0, max(50, len(xs)))
        plt.pause(0.001)