import serial
import matplotlib.pyplot as plt
from collections import deque

# Configuração da serial
ser = serial.Serial("COM11", 115200, timeout=1)

# Número de pontos exibidos
N = 200

x = deque(maxlen=N)
y = deque(maxlen=N)
z = deque(maxlen=N)

plt.ion()
fig, ax = plt.subplots(figsize=(10, 5))

linha_x, = ax.plot([], [], label="X")
linha_y, = ax.plot([], [], label="Y")
linha_z, = ax.plot([], [], label="Z")

ax.set_xlabel("Amostras")
ax.set_ylabel("Aceleração (g)")
ax.set_title("Acelerômetro em Tempo Real")
ax.set_ylim(-2, 2)
ax.grid(True)
ax.legend()

while True:
    try:
        linha = ser.readline().decode().strip()

        if linha:
            ax_g, ay_g, az_g = map(float, linha.split(","))

            x.append(ax_g)
            y.append(ay_g)
            z.append(az_g)
            print(f"X={ax_g}, Y={ay_g}, Z={az_g}")
            print(f"Tamanho: {len(x)}")

            linha_x.set_data(range(len(x)), x)
            linha_y.set_data(range(len(y)), y)
            linha_z.set_data(range(len(z)), z)

            ax.set_xlim(0, len(x))

            fig.canvas.draw()
            fig.canvas.flush_events()

    except KeyboardInterrupt:
        break
    except Exception as e:
        print("Erro:", e)

ser.close()